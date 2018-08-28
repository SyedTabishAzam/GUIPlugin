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
#include "SimulationBatcher.h"

#include "Constants.h"
#include "Sim.h"
#include "SimulationRun.h"

#include <sig_path.h>
#include <sig_print.h>
#include <sig_signal_hdlrs.h>
#include <sig_timer.h>

#include <sqxOSSystem.h>

#include <stage_msg.h>
#include <stage_icd.h>
#include <stage_icd_xdr.h>
#include <stagePath.h>
#include <msg/stage_msg_exp.h>
#include <msg/StExpMsgHandler.h>

#include <cassert>
#include <cstdio>
#include <algorithm>

//=============================================================================
// L O C A L   C L A S S E S
//=============================================================================
class NullListener : public SimulationBatcher::ExecutionListener
{
public:
   static NullListener s_Instance;
   sqxVoid vOnStepChanged(SimulationBatcher::ExecutionStep) {}
};
NullListener NullListener::s_Instance;

//=============================================================================
// L O C A L   V A R I A B L E S
//=============================================================================
static SimulationBatcher* s_pInstance = SQX_NULL;
static const int s_MaxSleepTimeInMs = 100;

//=============================================================================
// L O C A L   F U N C T I O N S
//=============================================================================
sqxVoid vSleepWithoutInterrupts(int a_Milliseconds)
{
   sig_signal_hdlrs_block_io();
   sig_timer_msec_sleep(a_Milliseconds);
   sig_signal_hdlrs_unblock_io();
}

//=============================================================================
// P U B L I C   M E T H O D S
//=============================================================================
SimulationBatcher& SimulationBatcher::s_rGetInstance()
{
   if (s_pInstance == SQX_NULL) s_pInstance = new SimulationBatcher();
   return *s_pInstance;
}

sqxBool SimulationBatcher::Execute(const SimulationRun& a_rRun, sqxUInt a_ID)
{
   // This method and its callees supports asynchronous cancellation.
   // Therefore, every time we wait for an event, the cancel execution
   // flag should be checked once in a while.
   CancelExecutionFlag = SQX_FALSE;

   Sim& _rSim = Sim::s_rGetInstance();

   pListener->vOnStepChanged(ExecutionStep_CheckingStopConditions);
   if (a_rRun.stopConditions == StopConditions_None || CancelExecutionFlag)
      return SQX_FALSE;

   // Wait for connection to the SIM
   pListener->vOnStepChanged(ExecutionStep_ConnectingToSim);

   if (_rSim.GetState() == Sim::State_CommsNotWorking)
      return SQX_FALSE;

   // For each iteration of this run...
  
 

      
      // Create temporary batch file with SIM commands specific to this run
      pListener->vOnStepChanged(ExecutionStep_WritingCommandFile);

      /* Open batch file */
      char file_name[256];
      sprintf(file_name, "%s%cGUIPlugin%c%s",
         stage_get_ansi_path(STAGE_DIR),
         SIG_PATH_SEP, SIG_PATH_SEP, SIM_CMD_FILE);

	  if (!WriteCommandsFile(a_rRun, a_ID, file_name, a_rRun.InitPublisherPath.c_str(), a_rRun.CommandSubscriberPath.c_str()) || CancelExecutionFlag)
         return SQX_FALSE;

      // Load the scenario
      pListener->vOnStepChanged(ExecutionStep_LoadingScenario);
      if (!LoadScenario(a_rRun) || CancelExecutionFlag)
         return SQX_FALSE;

      // Start the loaded the scenario
      pListener->vOnStepChanged(ExecutionStep_StartingScenario);
      if (!StartLoadedScenario(a_rRun) || CancelExecutionFlag)
         return SQX_FALSE;

      pListener->vOnStepChanged(ExecutionStep_RunningScenario);
     

      // Wait for the sim to stop
      while (_rSim.GetState() == Sim::State_ScenarioRunning)
      {
         if (CancelExecutionFlag)
         {
            _rSim.vSendStopScenario();
            return SQX_FALSE;
         }

         vSleepWithoutInterrupts(s_MaxSleepTimeInMs);
      }
   

   return SQX_TRUE;
}

sqxVoid SimulationBatcher::vSetListener(ExecutionListener* a_pListener)
{
   // Ensure the value is never null
   pListener = a_pListener == SQX_NULL
      ? &NullListener::s_Instance : a_pListener;
}

sqxVoid SimulationBatcher::vSetCancelExecutionFlag()
{
   CancelExecutionFlag = SQX_TRUE;
}

//=============================================================================
// C O N S T R U C T O R S   A N D   D E S T R U C T O R S
//=============================================================================
SimulationBatcher::SimulationBatcher()
   : pListener(&NullListener::s_Instance),
   CancelExecutionFlag(SQX_FALSE)
{}

//=============================================================================
// P R I V A T E   M E T H O D S
//=============================================================================
sqxBool SimulationBatcher::WriteCommandsFile(const SimulationRun& a_rRun,
	sqxUInt a_RunID, const sqxChar* a_pFilePath, const wchar_t* pubpart, const wchar_t* subpart)
{
   FILE* _pFile = fopen(a_pFilePath, "w");
   if (_pFile == NULL)
   {
      sig_print("%s:%d: Error: Cannot open file %s for writing !\n",
         __FILE__, __LINE__, a_pFilePath);
      return false;
   }

   
   fprintf(_pFile, "RUN %d\n", (int)a_RunID);
   fwprintf(_pFile, L"PUBLISHER %ls\n", pubpart);
   fwprintf(_pFile, L"SUBSCRIBER %ls\n", subpart);

   // Add the stop conditions that are enabled.
   if (a_rRun.stopConditions & StopConditions_ElapsedTime)
   {
      fprintf(_pFile, "%s %f\n", STOP_CONDITION_TIME_ELAPSED,
         static_cast<float>(a_rRun.StopTimeInSeconds));
   }

   // Store the SIM command lines for the current run
   for (size_t i = 0 ; i < a_rRun.SimParams.size(); i++)
   {
      const std::string& _rParam = a_rRun.SimParams[i];
      fputs(_rParam.c_str(), _pFile);
      
      // Ensure a newline is written after the command.
      if (_rParam[_rParam.length() - 1] != '\n')
         fputs("\n", _pFile);
   }

   fclose(_pFile);
   return true;
}

sqxBool SimulationBatcher::LoadScenario(const SimulationRun& a_rRun)
{
   Sim& _rSim = Sim::s_rGetInstance();

   
	_rSim.vSendOpenScenario(
		a_rRun.DatabaseFilePath.c_str(),
		a_rRun.ScenarioFilePath.c_str());
   
   
   while (_rSim.GetLastScenarioLoadResult() == Sim::Result_Pending)
   {
      if (CancelExecutionFlag) {
         return SQX_FALSE;
      }

      vSleepWithoutInterrupts(s_MaxSleepTimeInMs);
   }

   return _rSim.GetLastScenarioLoadResult() == Sim::Result_Success;
}

sqxBool SimulationBatcher::StartLoadedScenario(const SimulationRun& a_rRun)
{
   Sim& _rSim = Sim::s_rGetInstance();

   _rSim.vSendStartScenario();

   while (_rSim.GetLastScenarioStartResult() == Sim::Result_Pending)
   {
      if (CancelExecutionFlag) {
         return SQX_FALSE;
      }

      vSleepWithoutInterrupts(s_MaxSleepTimeInMs);
   }

   return _rSim.GetLastScenarioStartResult() == Sim::Result_Success;
}
