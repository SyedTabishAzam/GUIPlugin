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

#ifndef STAGE_BATCH_LIB_SIMULATION_BATCHER_H
#define STAGE_BATCH_LIB_SIMULATION_BATCHER_H

//=============================================================================
// I N C L U D E S
//=============================================================================
#include "Export.h"

#include "sqxTypes.h"

//=============================================================================
// F O R W A R D   D E C L A R A T I O N S
//=============================================================================
struct SimulationRun;

//=============================================================================
// T Y P E   D E C L A R A T I O N S
//=============================================================================
/// @brief Handles the execution of simulation runs.
///
/// @remarks This class is intended to be used by a single thread with the
/// exception of the vSetCancelExecutionFlag() method that can be used by
/// any thread to signal an Execute() call to cancel itself.
class STAGE_BATCH_LIB_SPEC SimulationBatcher
{
public:
   //==========================================================================
   // N E S T E D   T Y P E   D E C L A R A T I O N S
   //==========================================================================
   /// @brief Represents a step in the process of executing a simulation run.
   /// This is used to report status updates and failure stages.
   enum ExecutionStep
   {
      ExecutionStep_None,
      ExecutionStep_CheckingStopConditions,
      ExecutionStep_ConnectingToSim,
      ExecutionStep_StartingIteration,
      ExecutionStep_Delaying,
      ExecutionStep_WritingCommandFile,
      ExecutionStep_LoadingScenario,
      ExecutionStep_StartingScenario,
      ExecutionStep_RunningScenario,
   };

   /// @brief Interface for classes which track the progress of
   /// simulation run executions. Typical use case is displaying
   /// progress info to the user.
   class ExecutionListener
   {
   public:
      /// @brief Invoked when the current step in the execution
      /// process changes.
      ///
      /// @param a_Step The execution step that is reached.
      /// @param a_IterationIndex The iteration index for runs
      /// that must be executed multiple times.
      virtual sqxVoid vOnStepChanged(ExecutionStep a_Step,
         sqxUInt a_IterationIndex) = 0;
   };

   //==========================================================================
   // P U B L I C   M E T H O D S
   //==========================================================================
   /// @brief Gets the singleton instance of this class.
   static SimulationBatcher& s_rGetInstance();

   /// @brief Executes a simulation run.
   ///
   /// @param a_rRun The run to be executed.
   /// @param a_ID The identifier of the run, used to create the results filename.
   ///
   /// @retval SQX_TRUE for success, SQX_FALSE for failure.
   sqxBool Execute(const SimulationRun& a_rRun, sqxUInt a_ID);

   /// @brief Sets the execution listener.
   sqxVoid vSetListener(ExecutionListener* a_pListener);

   /// @brief Marks an execution that is in progress as being cancelled.
   ///
   /// @remarks This method is thread-safe
   /// It should be called by another thread while Execute() is running.
   /// The execution is not cancelled when this method returns, but rather
   /// scheduled to be cancelled.
   sqxVoid vSetCancelExecutionFlag();

private:
   //==========================================================================
   // C O N S T R U C T O R S   A N D   D E S T R U C T O R S
   //==========================================================================
   SimulationBatcher();

   //==========================================================================
   // P R I V A T E   M E T H O D S
   //==========================================================================
   /// @brief Sends a command to the sim to open or restore the scenario
   /// for a given simulation run.
   sqxBool LoadScenario(const SimulationRun& a_rRun);

   /// @brief Starts the loaded scenario.
   sqxBool StartLoadedScenario(const SimulationRun& a_rRun);

   /// @brief Writes the simulation command file for the given run.
   static sqxBool WriteCommandsFile(const SimulationRun& a_rRun,
	   sqxUInt a_RunID, sqxUInt a_IterationID, const sqxChar* a_pFilePath, const wchar_t*, const wchar_t*);

   //==========================================================================
   // P R I V A T E   F I E L D S
   //==========================================================================
   /// @brief The current execution progress listener.
   /// To simplify internal logic, this is never null.
   /// Instead, it is set to a null object when unused.
   ExecutionListener* pListener;

   /// @brief A flag set by another thread than the one executing Execute
   /// to indicate that execution should be cancelled. This is polled in
   /// Execute.
   sqxBool CancelExecutionFlag;
};

#endif
