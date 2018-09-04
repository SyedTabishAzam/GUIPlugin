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

#pragma once

//=============================================================================
// I N C L U D E S
//=============================================================================
#include <SimulationBatcher.h>
#include <SimulationRun.h>

#include <sqxTypes.h>

#include <vector>

//=============================================================================
// F O R W A R D   D E C L A R A T I O N S
//=============================================================================
class CStageBatchGuiDlg;
class CWinThread;

//=============================================================================
// T Y P E   D E C L A R A T I O N S
//=============================================================================
/// @brief Controller of the application, manages the logic behind the UI,
/// including the execution of simulation runs.
///
/// @remarks This classes uses a second thread of execution to execute
/// simulation runs without blocking the UI thread. Therefore, care must
/// be taken to respect thread safety.
class StageBatchGuiControl : private SimulationBatcher::ExecutionListener
{
public:
   //==========================================================================
   // C O N S T R U C T O R S   A N D   D E S T R U C T O R S
   //==========================================================================
   StageBatchGuiControl();
   ~StageBatchGuiControl();

   //==========================================================================
   // P U B L I C   M E T H O D S
   //==========================================================================
   /// @brief Gets the MFC UI dialog associated with this controller.
   CStageBatchGuiDlg* pGetDialog() const;

   /// @brief Shows the UI dialog and enters a message loop until the dialog
   /// closes.
   sqxVoid vDoModal();

   /// @brief Gets the number of simulation runs.
   sqxUInt GetRunCount() const;

   /// @brief Gets a simulation run by its index.
   const SimulationRun* pGetRunAt(sqxUInt a_Index) const;

   /// @brief Gets a simulation run by its index.
   ///
   /// @remarks The run should not be modified while IsExecuting returns true.
   SimulationRun* pGetRunAt(sqxUInt a_Index);
   
   /// @brief Removes a simulation run by its index.
   ///
   /// @remarks This should not be called while IsExecuting returns true.
   sqxVoid vRemoveRunAt(sqxUInt a_Index);
   
   /// @brief Moves a simulation run to another index.
   ///
   /// @remarks This should not be called while IsExecuting returns true.
   sqxVoid vMoveRun(sqxUInt a_SourceIndex, sqxUInt a_DestIndex);
   
   /// @brief Adds a new simulation run at the end of the list.
   ///
   /// @remarks This should not be called while IsExecuting returns true.
   sqxVoid vAddRun();

   /// @brief Tests if simulation runs are being executed on a worker thread.
   ///
   /// @remarks It is only safe to check if this value is false, because even
   /// though it can return true, the thread might stop before any actions
   /// that depend on the thread activity are executed (race condition).
   sqxBool IsExecuting() const;

   /// @brief Begins the asynchronous execution of simulation runs in a worker
   /// thread. This should only be called if IsExecuting returns false.
   ///
   /// @param a_RunIndex The index of the simulation run to execute,
   /// or -1 to execute all of them.
   sqxVoid vBeginExecution(sqxUInt a_RunIndex);

   static UINT ServerBegin(LPVOID);
   sqxVoid vStartServer();
   /// @brief Sends a message to the worker thread to indicate that the
   /// execution of simulation runs should be cancelled. IsExecuting can
   /// then be polled to know when the thread has stopped.
   sqxVoid vBeginExecutionCancellation();

   /// @brief Cancels any threaded simulation run executions and blocks
   /// until the worker thread stops.
   sqxVoid vCancelExecution();

private:
   //==========================================================================
   // P R I V A T E   M E T H O D S
   //==========================================================================
   /// @brief Simulation batcher progress callback used to display
   /// progress messages to the user.
   sqxVoid vOnStepChanged(SimulationBatcher::ExecutionStep a_Step);

   /// @brief Static worker thread entry point.
   static UINT StaticThreadProc(LPVOID pParam);

   /// @brief Worker thread "main" method.
   ///
   /// @param a_RunToExecuteIndex The index of the simulation run to be executed,
   /// or -1 to execute them all.
   ///
   /// @retval A value indicating the execution success.
   sqxBool ThreadProc(sqxInt a_RunToExecuteIndex);

   sqxVoid UpdateConnections();
   //==========================================================================
   // P R I V A T E   F I E L D S
   //==========================================================================
   /// @brief MFC UI dialog, never null.
   CStageBatchGuiDlg* pDialog;

   /// @brief The worker thread used to execute simulation runs without
   /// blocking the UI.
   CWinThread* pWorkerThread;

   /// @brief Accessed only by the worker thread to indicate the run
   /// currently being executed. This is used by vOnStepChanged to display
   /// status information.
   sqxInt RunBeingExecutedIndex;

   /// @brief A flag set by the main thread to inform the worker thread
   /// to cancel the execution of simulation runs.
   sqxBool CancelExecutionFlag;

   /// @brief The simulation runs.
   ///
   /// @remarks This is read and written by the main thread, and only
   /// read by the worker threads. No locks synchronize its access, so
   /// the vector or its items should not be written to while the worker
   /// thread is running.
   std::vector<SimulationRun*> Runs;
};
