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
#include "Sim.h"

#include <sig_path.h>
#include <sig_print.h>
#include <sig_signal_hdlrs.h>
#include <sig_timer.h>

#include <sqxOSSystem.h>

#include <stage_msg.h>
#include <stage_icd.h>
#include <stage_icd_xdr.h>
#include <msg/stage_msg_exp.h>
#include <msg/StExpMsgHandler.h>

#include <cassert>
#include <string>

using namespace std;

//=============================================================================
// L O C A L   V A R I A B L E S
//=============================================================================
static Sim* s_pInstance = SQX_NULL;

//=============================================================================
// L O C A L   F U N C T I O N S
//=============================================================================
/// @brief Sets a result if its current value is Result_Pending.
inline sqxVoid vSetSuccessIfPending(Sim::Result& a_rResult, bool a_Value)
{
   if (a_rResult == Sim::Result_Pending)
      a_rResult = a_Value ? Sim::Result_Success : Sim::Result_Failure;
}

//=============================================================================
//
//=============================================================================
sqxVoid ThreadFunc(sqxVoid* a_pVoid)
{
   static bool _first = true;
   while (1) {
      g_pOSSystem->MSleep(500);
      stage_msg_exp_process_msg(SQX_TRUE);
   }
}

//=============================================================================
// P U B L I C   M E T H O D S
//=============================================================================
Sim& Sim::s_rGetInstance()
{
   if (s_pInstance == SQX_NULL) s_pInstance = new Sim();
   return *s_pInstance;
}

Sim::State Sim::GetState() const
{
   if (stage_msg_exp_is_connected_to_sim())
   {
      if (StateValue == State_NotConnected)
         StateValue = State_Idle;
   }
   else
   {
      if (StateValue > State_NotConnected)
         StateValue = State_NotConnected;
   }

   return StateValue;
}

sqxVoid Sim::vSendOpenScenario(const sqxWChar* a_pDatabasePath,
      const sqxWChar* a_pScenarioPath)
{
   LastScenarioLoadResult = Result_Pending;

   ui_sim_open_scenario(
      const_cast<sqxWChar*>(a_pScenarioPath),
      const_cast<sqxWChar*>(a_pDatabasePath));
}

sqxVoid Sim::vSendRestoreScenario(const sqxWChar* a_pSnapshotPath)
{
   LastScenarioLoadResult = Result_Pending;

   std::wstring _DirName = sig_wdirname(a_pSnapshotPath);
   std::wstring _BaseName = sig_wbasename(a_pSnapshotPath);

   ui_restore(
      const_cast<sqxWChar*>(_DirName.c_str()),
      const_cast<sqxWChar*>(_BaseName.c_str()));
}

Sim::Result Sim::GetLastScenarioLoadResult() const
{
   return LastScenarioLoadResult;
}

sqxVoid Sim::vSendStartScenario()
{
   LastScenarioStartResult = Result_Pending;
   ui_start_sim();
}

Sim::Result Sim::GetLastScenarioStartResult() const
{
   return LastScenarioStartResult;
}

sqxVoid Sim::vSendStopScenario()
{
   ui_stop_sim();
}

sqxVoid Sim::vSendTakeSnapshot()
{
   ui_snapshot(NULL);
}

sqxDouble Sim::GetTimeInSeconds() const
{
   return TimeInSeconds;
}

//=============================================================================
// C O N S T R U C T O R S   A N D   D E S T R U C T O R S
//=============================================================================
Sim::Sim()
   : LastScenarioLoadResult(Result_Pending),
   LastScenarioStartResult(Result_Pending),
   TimeInSeconds(0)
{
   StateValue = InitializeMessageHandlers()
      ? State_NotConnected : State_CommsNotWorking;

   StateValue = State_Idle;
}

//=============================================================================
// P R I V A T E   M E T H O D S
//=============================================================================
sqxBool Sim::InitializeMessageHandlers()
{
   // Create the connection with the DE (for sending)
   if (ui_init(STAGE_MSG_USER_CLASS_BEGIN) == -1
      || ui_init_connections(NULL) == -1)
   {
      sig_print("%s:%d: Could not create link to SIM\n",
         __FILE__, __LINE__);
      return SQX_FALSE;
   }

   if (stage_msg_exp_init())
   {
      sig_print("%s:%d: Error:Failed to initialize stage_msg_exp\n", 
         __FILE__, __LINE__);      
      return SQX_FALSE;
   }

   //Create a thread
   g_pOSSystem->CreateThread(ThreadFunc, SQX_NULL);

   // Install a handler to receive RTC messages sent by the DE
   if (stage_msg_exp_set_handler(MSG_DE_RTC, HandleRTCMessage))
   {
      sig_print("%s:%d: Could not install DE RTC message handler\n", 
         __FILE__, __LINE__);
      return SQX_FALSE;
   }

   if (!StExpMsgHandlerInit())
   {
      sig_print("%s:%d: Could not initialize exporter\n",
         __FILE__, __LINE__);
      return SQX_FALSE;
   }

   if (!StExpMsgHandlerRegisterStatusFunc(EXP_STATUS, "stage", StoreExportedStatusData)
      || !StExpMsgHandlerRegisterXDRFunc(EXP_STATUS, "stage",
         (StExpMsgXDRDecodeFunc)ui_xdr_export_exp_status))
   {
      sig_print("%s:%d: Could not register exported message handler\n", 
         __FILE__, __LINE__);
      return SQX_FALSE;
   }

   return SQX_TRUE;
}

void Sim::StoreExportedStatusData(void*, int)
{
   Sim& _rInstance = Sim::s_rGetInstance();
   _rInstance.TimeInSeconds = StExpMsgHandlerGetSimTime();
   vSetSuccessIfPending(_rInstance.LastScenarioLoadResult, true);

   if (_rInstance.StateValue < State_ScenarioLoaded)
      _rInstance.StateValue = State_ScenarioLoaded;
}

void Sim::HandleRTCMessage(int a_Subtype, int a_Status,
   int a_DataSize, void* a_pData, unsigned int, unsigned short)
{
   Sim& _rInstance = Sim::s_rGetInstance();

   if (a_Subtype == RC_DE_CURRENT_SIM_STATE)
   {
      Stage_XDR _XDR;
      stage_xdr_start_decode(&_XDR, a_DataSize, a_pData);
      Sim_Current_State _SimState;      
      ui_xdr_sim_rtc_state(&_XDR, &_SimState);

      switch (_SimState.rtc_mode)
      {
      case READY_TO_RUN:
         vSetSuccessIfPending(_rInstance.LastScenarioLoadResult, true);
         _rInstance.StateValue = State_ScenarioLoaded;
         break;

      case RUNNING_STATE:
         vSetSuccessIfPending(_rInstance.LastScenarioStartResult, true);
         _rInstance.StateValue = State_ScenarioRunning;
         break;

      case INITIALIZING_STATE:
         vSetSuccessIfPending(_rInstance.LastScenarioStartResult, false);
         _rInstance.StateValue = State_Idle;
         _rInstance.TimeInSeconds = 0;
         break;

      case IDLE_STATE:
         vSetSuccessIfPending(_rInstance.LastScenarioStartResult, false);
         _rInstance.StateValue = State_Idle;
         _rInstance.TimeInSeconds = 0;
         break;

      default:
         {
         }
      }
   }
   else if (a_Subtype == RC_DE_SIM_RESTORE_MODE)
   {
      Stage_Restore_Mode _RestoreMode = static_cast<Stage_Restore_Mode>(a_Status);
      switch (_RestoreMode)
      {
      case STAGE_RESTORE_WAITING_FOR_RUN:
         vSetSuccessIfPending(_rInstance.LastScenarioLoadResult, true);
         _rInstance.StateValue = State_ScenarioLoaded;
         break;

      default:
         {
         }
      }
   }
}
