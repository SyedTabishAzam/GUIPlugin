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

#ifndef STAGE_BATCH_LIB_SIMULATION_RUN_H
#define STAGE_BATCH_LIB_SIMULATION_RUN_H

//=============================================================================
// I N C L U D E S
//=============================================================================
#include "Export.h"

#include <sqxTypes.h>

#include <string>
#include <vector>

//=============================================================================
// T Y P E   D E C L A R A T I O N S
//=============================================================================
/// @brief Flags indicating the conditions to stop a simulation run.
enum StopConditions
{
   /// @brief Indicates that no stop conditions are defined.
   StopConditions_None = 0,

   /// @brief Indicates that the run should stop once a fixed amount of time elapses.
   StopConditions_ElapsedTime = 1 << 0,

   /// @brief Indicates that the run should stop once an entity dies.
   StopConditions_DeadEntity = 1 << 1,

   /// @brief Indicates that the run should stop once there are fewer
   /// than certain counts of red or blue platforms.
   StopConditions_MinPlatforms = 1 << 2,
};

/// @brief Enables the |= operator between StopConditions enumerants.
inline StopConditions& operator|=(StopConditions& a, StopConditions b)
{
   a = (StopConditions)((int)a | (int)b);
   return a;
}

/// @brief Enables the | operator between StopConditions enumerants.
inline StopConditions operator|(StopConditions a, StopConditions b)
{
   return (StopConditions)((int)a | (int)b);
}

/// @brief Encapsulates the parameters needed for a simulation run.
struct STAGE_BATCH_LIB_SPEC SimulationRun
{
public:
   //==========================================================================
   // C O N S T R U C T O R S   A N D   D E S T R U C T O R S
   //==========================================================================
   /// @brief Zero-initializes a new simulation run instance.
   SimulationRun();

   //==========================================================================
   // P U B L I C   F I E L D S
   //==========================================================================
   /// @brief The name of this run.
   std::wstring Name;

   /// @brief The path to the database file.
   std::wstring DatabaseFilePath;

   /// @brief The path to the scenario or snapshot file.
   std::wstring ScenarioFilePath;

   /// @brief The path to the initial publisher
   std::wstring InitPublisherPath;

   /// @brief The path to command subscriber
   std::wstring CommandSubscriberPath;

   /// @brief A flag indicating if the run implies restoring a snapshot.
  

   /// @brief Flags indicating which stop conditions are used.
   StopConditions stopConditions;

   /// @brief The simulation time at which to stop this run, in seconds.
   /// This is only used if StopConditions_ElapsedTime is set.
   sqxDouble StopTimeInSeconds;

   /// @brief The name of the entity whose death causes the run to stop.
   /// This is only used if StopConditions_DeadEntity is set.
   //std::wstring StopEntityName;

  

   /// @brief The simulation times at which to take snapshots, in seconds.
 

   /// @brief Additional parameters to be passed to the SIM plugin.
   std::vector<std::string> SimParams;
};

#endif
