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

#include "stdafx.h"

#include "StageBatchGui.h"
#include "StageBatchGuiControl.h"
#include "StageBatchGuiDlg.h"


#include <Sim.h>

#include <sig_timer.h>
#include <util/stagePath.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CStageBatchGuiApp

BEGIN_MESSAGE_MAP(CStageBatchGuiApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CStageBatchGuiApp construction

CStageBatchGuiApp::CStageBatchGuiApp()
{
	// Place all significant initialization in InitInstance
}


// The one and only CStageBatchGuiApp object

CStageBatchGuiApp theApp;


// CStageBatchGuiApp initialization

BOOL CStageBatchGuiApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

   // Custom startup code
   sig_timer_startup();
   stage_path_init();

   if (Sim::s_rGetInstance().GetState() == Sim::State_CommsNotWorking)
   {
      ::MessageBoxW(NULL,
         L"Failed to setup SIM message handling, executing simulation runs will fail.",
         L"Error", MB_OK | MB_ICONERROR);
   }

   StageBatchGuiControl Control;
	m_pMainWnd = Control.pGetDialog();
   Control.vDoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
