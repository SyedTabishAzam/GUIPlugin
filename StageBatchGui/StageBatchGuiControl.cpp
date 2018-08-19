/*##########################################################################
//
// AVIS IMPORTANT: ce logiciel, incluant le code source, contient de l’information
// confidentielle et/ou propriété de Presagis Canada inc. et/ou ses filiales et
// compagnies affiliées (« Presagis ») et est protégé par les lois sur le droit
// d’auteur. L’utilisation de ce logiciel est sujet aux termes d’une licence de
// Presagis.
// Malgré toute disposition contraire, l'utilisateur titulaire
// d'une licence peut utiliser les exemples de code pour générer de nouvelles
// applications.
// © Presagis Canada Inc., 1995-2011. Tous droits réservés.
//
// IMPORTANT NOTICE:  This software program (“Software”) and/or  the Source
// code is a confidential and/or proprietary product of Presagis Canada Inc.
// and/or its subsidiaries and affiliated companies (“Presagis”) and is
// protected by copyright laws.  The use of the Software is subject to the
// terms of the Presagis Software License Agreement.
// Notwithstanding anything to the contrary, the
// licensed end-user is allowed to use the sample source provided with the
// Software to generate new applications.
// © Presagis Canada Inc., 1995-2011. All Rights Reserved.
//
*###########################################################################*/

//=============================================================================
// I N C L U D E S
//=============================================================================
#include "stdafx.h"

#include "StageBatchGuiControl.h"
#include "StageBatchGuiDlg.h"

#include <Sim.h>
#include <SimulationBatcher.h>

#include <sqxOSSystem.h>

#include <sig_path.h>
#include <sig_print.h>

#include <afxwin.h>
#include <windows.h>

#include <cassert>

//=============================================================================
// L O C A L   T Y P E   D E C L A R A T I O N S
//=============================================================================
/// @brief Encapsulates parameters used to initialize the worker thread.
struct WorkerThreadStartParams
{
   /// @brief The StageBatchGuiControl instance to which the thread belongs.
   StageBatchGuiControl* pInstance;

   /// @brief The index of the simulation run to be executed,
   /// or -1 to execute all of them.
   sqxInt RunIndex;
};

//=============================================================================
// C O N S T R U C T O R S   A N D   D E S T R U C T O R S
//=============================================================================
StageBatchGuiControl::StageBatchGuiControl()
   : pDialog(SQX_NULL), pWorkerThread(SQX_NULL), RunBeingExecutedIndex(-1)
{
   pDialog = new CStageBatchGuiDlg(*this);
}

StageBatchGuiControl::~StageBatchGuiControl()
{
   // Ensure the thread is stopped before deleting the dialog,
   // or it may lead to race conditions with the worker thread trying
   // to output message lines to the UI.
   vCancelExecution();
   delete pDialog;
}

//=============================================================================
// P U B L I C   M E T H O D S
//=============================================================================
CStageBatchGuiDlg* StageBatchGuiControl::pGetDialog() const
{
   return pDialog;
}

sqxVoid StageBatchGuiControl::vDoModal()
{
   pDialog->DoModal();
}

sqxUInt StageBatchGuiControl::GetRunCount() const
{
   return static_cast<sqxUInt>(Runs.size());
}

SimulationRun* StageBatchGuiControl::pGetRunAt(sqxUInt a_Index)
{
   return Runs[static_cast<size_t>(a_Index)];
}

sqxVoid StageBatchGuiControl::vRemoveRunAt(sqxUInt a_Index)
{
   assert(!IsExecuting());

   Runs.erase(Runs.begin() + a_Index);
}

sqxVoid StageBatchGuiControl::vMoveRun(sqxUInt a_SourceIndex, sqxUInt a_DestIndex)
{
   assert(!IsExecuting());

   SimulationRun* _pRun = Runs[a_SourceIndex];
   Runs.erase(Runs.begin() + a_SourceIndex);
   Runs.insert(Runs.begin() + a_DestIndex, _pRun);
}

sqxVoid StageBatchGuiControl::vAddRun()
{
   assert(!IsExecuting());

   char _Number[12];
   itoa(static_cast<int>(Runs.size()) + 1, _Number, 10);

   SimulationRun* _pRun = new SimulationRun();

   _pRun->Name = L"Run #";
   _pRun->Name += g_pOSSystem->AnsiToUnicode(_Number);
   _pRun->DatabaseFilePath = L"S:\\Presagis\\Suite16\\STAGE\\data\\sm_db\\default.xml";
   _pRun->ScenarioFilePath = L"S:\\Presagis\\Suite16\\STAGE\\data\\sm_db\\default\\scenario\\GUIScenerio.scenario";
   Runs.push_back(_pRun);
}

sqxBool StageBatchGuiControl::IsExecuting() const
{
   // The worker thread is executing if it has been created and has not
   // yet completed.
   return pWorkerThread != SQX_NULL
      && ::WaitForSingleObject(pWorkerThread->m_hThread, 0) != WAIT_OBJECT_0;
}

sqxVoid StageBatchGuiControl::vBeginExecution(sqxUInt a_RunIndex)
{
   assert(!IsExecuting());

   // Delete the previous worker thread, if any.
   delete pWorkerThread;
   pWorkerThread = SQX_NULL;

   CancelExecutionFlag = SQX_FALSE;

   // The parameters structure will be deleted by the worker thread.
   WorkerThreadStartParams* _pParams = new WorkerThreadStartParams();
   _pParams->pInstance = this;
   _pParams->RunIndex = a_RunIndex;

   // The worker thread is created in suspended state to be able to set the
   // auto delete flag without race conditions (the thread could have ended
   // be deleted before the flag is set).
   pWorkerThread = AfxBeginThread(StaticThreadProc, _pParams, 0, 0, CREATE_SUSPENDED);
   pWorkerThread->m_bAutoDelete = FALSE;
   pWorkerThread->ResumeThread();
}

sqxVoid StageBatchGuiControl::vBeginExecutionCancellation()
{
   if (!IsExecuting()) return;

   CancelExecutionFlag = SQX_TRUE;
   SimulationBatcher::s_rGetInstance().vSetCancelExecutionFlag();
}

sqxVoid StageBatchGuiControl::vCancelExecution()
{
   if (pWorkerThread == SQX_NULL) return;

   CancelExecutionFlag = SQX_TRUE;
   ::WaitForSingleObject(pWorkerThread->m_hThread, INFINITE);
   delete pWorkerThread;
   pWorkerThread = SQX_NULL;
}

//=============================================================================
// P R I V A T E   M E T H O D S
//=============================================================================
sqxVoid StageBatchGuiControl::vOnStepChanged(
   SimulationBatcher::ExecutionStep a_Step, sqxUInt a_ExecutionIndex)
{
   SimulationRun& _rRun = *Runs[RunBeingExecutedIndex];
   CString _TempString;

   // Warning: This is called on the worker thread, synchronization
   // is needed to access MFC objects on the UI thread.
   switch (a_Step)
   {
   case SimulationBatcher::ExecutionStep_ConnectingToSim:
      pDialog->vAppendOutputLine(L"Connecting to SIM...");
      break;

   case SimulationBatcher::ExecutionStep_StartingIteration:
      _TempString.Format(L"=> Iteration %d:",
         static_cast<int>(a_ExecutionIndex));
      pDialog->vAppendOutputLine(_TempString.GetString());
      break;
      
   case SimulationBatcher::ExecutionStep_Delaying:
      _TempString.Format(L"Delaying for %f seconds...",
         static_cast<float>(_rRun.DelayInSeconds));
      pDialog->vAppendOutputLine(_TempString.GetString());
      break;
      
   case SimulationBatcher::ExecutionStep_LoadingScenario:
      _TempString.Format(L"%s scenario '%s'...",
         _rRun.IsSnapshotRestore ? L"Restoring" : L"Opening",
         sig_wbasename(_rRun.ScenarioFilePath.c_str()));
      pDialog->vAppendOutputLine(_TempString.GetString());
      break;
      
   case SimulationBatcher::ExecutionStep_StartingScenario:
      _TempString.Format(L"Starting scenario...");
      pDialog->vAppendOutputLine(_TempString.GetString());
      break;
      
   case SimulationBatcher::ExecutionStep_RunningScenario:
      pDialog->vAppendOutputLine(L"Waiting for stop condition...");
      break;
   }
}

UINT StageBatchGuiControl::StaticThreadProc(LPVOID pParam)
{
   WorkerThreadStartParams* _pParams
      = static_cast<WorkerThreadStartParams*>(pParam);

   // Store the initialization parameters and delete the structure
   // to prevent leaks.
   StageBatchGuiControl* _pInstance = _pParams->pInstance;
   sqxInt _RunIndex = _pParams->RunIndex;
   delete _pParams;

   return _pInstance->ThreadProc(_RunIndex) ? 0 : 1;
}

sqxBool StageBatchGuiControl::ThreadProc(sqxInt a_RunToExecuteIndex)
{
   // Determine the range of simulation runs to be executed
   // (either a single one or all of them).
   sqxInt _FirstRunIndex, _EndRunIndex;
   if (a_RunToExecuteIndex == -1)
   {
      _FirstRunIndex = 0;
      _EndRunIndex = static_cast<sqxInt>(Runs.size());
   }
   else
   {
      _FirstRunIndex = a_RunToExecuteIndex;
      _EndRunIndex = a_RunToExecuteIndex + 1;
   }

   // Initialize the simulation batcher.
   SimulationBatcher& _rBatcher = SimulationBatcher::s_rGetInstance();
   _rBatcher.vSetListener(this);
   
   // Execute every run
   for (RunBeingExecutedIndex = _FirstRunIndex;
      RunBeingExecutedIndex < _EndRunIndex;
      ++RunBeingExecutedIndex)
   {
      if (CancelExecutionFlag) break;

      SimulationRun& _rRun = *Runs[RunBeingExecutedIndex];

      CString _Message;
      _Message.Format(L"======== %ls ========", _rRun.Name.c_str());
      pDialog->vAppendOutputLine(_Message.GetString());

      sqxBool _RunSucceeded = _rBatcher.Execute(_rRun,
         static_cast<sqxUInt>(RunBeingExecutedIndex));
      
      pDialog->vAppendOutputLine(
         _RunSucceeded ? L"Run succeeded" : L"Run failed");
      pDialog->vAppendOutputLine(L"");
   }

   pDialog->vAppendOutputLine(CancelExecutionFlag
      ? L"Execution cancelled." : L"All runs completed!");

   RunBeingExecutedIndex = -1;

   return SQX_TRUE;
}
