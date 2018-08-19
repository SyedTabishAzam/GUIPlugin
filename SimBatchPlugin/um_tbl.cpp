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

/***************************************************************/
/* M O D U L E    P U R P O S E                                */
/*							       */
/*[ Defines the tables for the 30 hz execution of the          */
/*  user-defined modules                                      ]*/
/*							       */
/***************************************************************/

/***************************************************************/
/* I N C L U D E D   F I L E S                                 */
/***************************************************************/

#include <sig_rtc.h>
#include <sqx_plugin.h>
#include <sim/sim_tbl.h>

#include <plugins/sqxPlugin.h>
#include <sim/sim_tbl.h>

/*-------------------*/
/* USER MODULES HERE */
/*-------------------*/
#include "um_batch.h"

/***************************************************************/
/* L O C A L   C O N S T A N T S                               */
/***************************************************************/

/***************************************************************/
/* L O C A L   M A C R O S                                     */
/***************************************************************/

#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

/***************************************************************/
/* L O C A L   D A T A    S T R U C T U R E S                  */
/***************************************************************/

using namespace std;


class UserSimPlugin : public sqxPlugin {

public:
	UserSimPlugin() : sqxPlugin(L"UserModulePlugin", 1.0) {}

	sqxStatus Initialize(sqxPluginContextData* a_pInLoadData)
	{
		FUNC_ADD_INIT(um_batch_init);
		FUNC_ADD_CTRL(um_batch_ctrl);
		FUNC_ADD_ENTRY(um_batch_exec, 1, 1);  // define the rate and band here
		FUNC_ADD_DAEMON(um_tmpl_daemon);
		FUNC_ADD_BACKGROUND(um_tmpl_background);
		//FUNC_ADD_DAEMON();
		
		return SQX_SUCCESS;
	}
	sqxVoid vCleanUp(sqxPluginContextData* a_pInLoadData) {}

};

/***************************************************************/
/* E X T E R N A L    F U N C T I O N S                        */
/***************************************************************/

/***************************************************************/
/* L O C A L    F U N C T I O N S                              */
/***************************************************************/

/***************************************************************/
/* E X T E R N A L    D A T A                                  */
/***************************************************************/

/***************************************************************/
/* L O C A L   D A T A                                         */
/***************************************************************/

//Func_Init utbl_init[] = {
//   FUNC_INIT(um_batch_init), /* User initialization function */
//   END_OF_TABLE
//};
//
//Func_Ctrl utbl_ctrl[] = {
//   FUNC_CTRL(um_batch_ctrl), /* batch control function */
//   END_OF_TABLE
//};
//
//Func_Entry utbl_exec[] = {
//   FUNC_ENTRY(um_batch_exec, 1, 1),
//   END_OF_TABLE
//};
//
//Func_Daemon utbl_dmon[] = {
//   END_OF_TABLE
//};
//
///* Create a static user dispatch table */
//static Rtc_Dispatch_Table utbl = {
//      utbl_init,
//      utbl_ctrl,
//      utbl_exec,
//      utbl_dmon
//};
//
///***************************************************************/
///* L O C A L   F U N C T I O N S   C O D E   S E C T I O N     */
///***************************************************************/
//
///*
//* Add the user table to the sim table
//*/
//static sqxStatus InitTbl(void* contextdata)
//{
//   (void)contextdata;
//   sim_tbl_add_user_tbl(&utbl);
//   return SQX_SUCCESS;
//}

/***************************************************************/
/* P U B L I C   F U N C T I O N S   C O D E   S E C T I O N   */
/***************************************************************/

/*
* This is the function loaded by the plugin server
*/
//DLL_EXPORT sqx_plugin* sqxPluginLoad() {
//
//   static sqx_plugin plugin;
//   static sqxWChar name[10] = L"um_batch";
//   plugin.aName = name;
//   plugin.Priority = 1.0;
//   plugin.pInitFunc = InitTbl;
//   plugin.pCleanupFunc = NULL;  /* none needed */
//   return &plugin;
//}



extern "C" DLL_EXPORT sqxPlugin* sqxPluginLoad() {
	sqxPlugin* _Plugin = new UserSimPlugin();
	return _Plugin;
}







