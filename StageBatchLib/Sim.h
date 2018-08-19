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

#ifndef STAGE_BATCH_LIB_SIM_H
#define STAGE_BATCH_LIB_SIM_H

//=============================================================================
// I N C L U D E S
//=============================================================================
#include "Export.h"

#include "sqxTypes.h"

//=============================================================================
// T Y P E   D E C L A R A T I O N S
//=============================================================================
/// @brief Encapsulates communication logic with the SIM process.
class STAGE_BATCH_LIB_SPEC Sim
{
public:
   //==========================================================================
   // N E S T E D   T Y P E   D E C L A R A T I O N S
   //==========================================================================
   /// @brief Describes the current state of the simulation process.
   enum State
   {
      /// @brief Indicates that the SIM state is unknown because
      /// message handlers could not be initialized.
      State_CommsNotWorking,

      /// @brief Indicates that the connection to the SIM has not yet
      /// been established.
      State_NotConnected,

      /// @brief Indicates that the SIM is available and idle.
      State_Idle,

      /// @brief Indicates that the SIM has loaded a scenario but hasn't
      /// started it yet.
      State_ScenarioLoaded,

      /// @brief Indicates that the SIM is running a scenario.
      State_ScenarioRunning
   };

   /// @brief Describes the result of an asynchronous SIM request.
   enum Result
   {
      /// @brief Indicates that the result of the operation is not yet known.
      Result_Pending,

      /// @brief Indicates that the operation was successful.
      Result_Success,

      /// @brief Indicates that the operation failed.
      Result_Failure
   };

   //==========================================================================
   // P U B L I C   M E T H O D S
   //==========================================================================
   /// @brief Gets the singleton class instance.
   static Sim& s_rGetInstance();

   /// @brief Gets the last known state of the SIM process.
   State GetState() const;

   /// @brief Sends a message to the SIM so it opens a given scenario.
   sqxVoid vSendOpenScenario(const sqxWChar* a_pDatabasePath,
      const sqxWChar* a_pScenarioPath);

   /// @brief Sends a message to the SIM so it restores a given snapshot.
   sqxVoid vSendRestoreScenario(const sqxWChar* a_pSnapshotPath);

   /// @brief Gets the result of the last scenario open or restore operation.
   Result GetLastScenarioLoadResult() const;

   /// @brief Sends a message to the SIM so it starts an opened scenario.
   sqxVoid vSendStartScenario();

   /// @brief Gets the result of the last scenario start operation.
   Result GetLastScenarioStartResult() const;

   /// @brief Sends a message to the SIM so it stops the current scenario.
   sqxVoid vSendStopScenario();

   /// @brief Sends a message to the SIM so it takes a snapshot of the scenario.
   sqxVoid vSendTakeSnapshot();

   /// @brief Gets the last known simulation time, in seconds.
   sqxDouble GetTimeInSeconds() const;

private:
   //==========================================================================
   // C O N S T R U C T O R S   A N D   D E S T R U C T O R S
   //==========================================================================
   Sim();

   //==========================================================================
   // P R I V A T E   M E T H O D S
   //==========================================================================
   /// @brief Initializes the SIM message handlers.
   sqxBool InitializeMessageHandlers();

   static void StoreExportedStatusData(void* a_pData, int a_DataSize);

   static void HandleRTCMessage(int a_Subtype, int a_Status, int a_DataSize,
      void* a_pData, unsigned int a_SenderMachine, unsigned short a_SenderPort);

   //==========================================================================
   // P R I V A T E   F I E L D S
   //==========================================================================
   /// @brief The last known state of the sim process.
   mutable State StateValue;

   /// @brief The result of the last scenario open or restore.
   Result LastScenarioLoadResult;

   /// @brief The result of the last scenario start.
   Result LastScenarioStartResult;

   /// @brief The current simulation time, in seconds.
   sqxDouble TimeInSeconds;
};

#endif
