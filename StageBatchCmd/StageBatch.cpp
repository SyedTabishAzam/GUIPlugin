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
#include "Constants.h"
#include "Parser.h"
#include "Sim.h"
#include "SimulationBatcher.h"
#include "SimulationRun.h"

#include <sig_path.h>
#include <sig_print.h>
#include <sig_timer.h>
#include <util/stagePath.h>

#include <vector>

//=============================================================================
// L O C A L   T Y P E S
//=============================================================================
/// @brief Listens to events while simulation runs are executing 
/// to generate console feedback to the user.
class Listener : public SimulationBatcher::ExecutionListener
{
public:
   SimulationRun* pCurrentRun;

   sqxVoid vOnStepChanged(SimulationBatcher::ExecutionStep a_Step)
   {
      switch (a_Step)
      {
      case SimulationBatcher::ExecutionStep_ConnectingToSim:
         sig_print("Connecting to the SIM...\n");
         break;

         
      case SimulationBatcher::ExecutionStep_LoadingScenario:
         sig_wprint(L"%ls scenario %ls...\n", L"Opening",
            sig_wbasename(pCurrentRun->ScenarioFilePath.c_str()));
         break;
         
      case SimulationBatcher::ExecutionStep_StartingScenario:
         sig_print("Starting scenario...\n");
         break;
         
      case SimulationBatcher::ExecutionStep_RunningScenario:
         sig_print("Running scenario...\n");
         break;
      }
   }
};
static Listener s_ListenerInstance;

//#############################################################################
//                    L O C A L    F U N C T I O N S
//#############################################################################

//=============================================================================
// E X P O R T E D   F U N C T I O N S
//=============================================================================
/*-------------------------------------------------------------*/
/* main()                                                      */
/*                                                             */
/* Executes STAGE in batch mode                                */
/*                                                             */
/* Return value: none                                          */
/*-------------------------------------------------------------*/
/// @brief Entry point of the command-line STAGE batch mode utility.
///
/// @param argc The number of command line arguments.
/// @param argv The array of command line arguments.
/// If empty, the default commands file is used.
/// Otherwise, the first parameter is interpreted as a path
/// to the commands file to be used.
///
/// @retval 0 on success, 1 on failure.
int main(int argc, char *argv[])
{
   sig_timer_startup();
   stage_path_init();

   if (Sim::s_rGetInstance().GetState() == Sim::State_CommsNotWorking)
   {
      sig_print("Failed to initialize SIM message handlers, cannot run.\n");
      return 1;
   }
   
   // Parse batch file
   const char* _pCommandsFilePath = (argc > 1) ? argv[1] : "stage.commands";
   std::vector<SimulationRun*> _Runs;

   if (ParseBatchFile(_pCommandsFilePath, _Runs))
   {
      sqxUInt _TotalRunCount = static_cast<sqxUInt>(_Runs.size());


      
      SimulationBatcher& _rBatcher = SimulationBatcher::s_rGetInstance();
      _rBatcher.vSetListener(&s_ListenerInstance);
      for (size_t i = 0; i < _Runs.size(); ++i)
      {
         SimulationRun* _pRun = _Runs[i];
         sig_wprint(L"======== Run %d: %s ========\n",
            i + 1, _pRun->Name.c_str());

         s_ListenerInstance.pCurrentRun = _pRun;
         if (!_rBatcher.Execute(*_pRun, static_cast<sqxInt>(i + 1)))
            sig_print("Execution step failed, run aborted.\n");

         sig_print("\n");
      }
   }
   
   sig_timer_cleanup();

   return 0;
}

