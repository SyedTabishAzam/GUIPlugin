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
#include "Resource.h"

#include <SimulationRun.h>

#include <sqxTypes.h>

#include <afxwin.h>
#include <windows.h>

//=============================================================================
// F O R W A R D   D E C L A R A T I O N S
//=============================================================================
class StageBatchGuiControl;

//=============================================================================
// T Y P E   D E C L A R A T I O N S
//=============================================================================
/// @brief The main application dialog which allows the user to create
/// simulation runs, configure them and execute them.
class CStageBatchGuiDlg : public CDialog
{
public:
   //==========================================================================
   // C O N S T R U C T O R S   A N D   D E S T R U C T O R S
   //==========================================================================
	explicit CStageBatchGuiDlg(StageBatchGuiControl& a_rControl);

   //==========================================================================
   // M F C   G E N E R A T E D   C O D E
   //==========================================================================
   // Dialog Data
	enum { IDD = IDD_STAGEBATCHGUI_DIALOG};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void DeleteEntityData();
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
   //==========================================================================
   // P U B L I C   M E T H O D S
   //==========================================================================
   virtual BOOL PreTranslateMessage(MSG* pMsg);

   /// @brief Adds a line to the output text box.
   ///
   /// @param a_pLine The line of text to be appended.
   ///
   /// @remarks This method is thread-safe.
   sqxVoid vAppendOutputLine(const sqxWChar* a_pLine);

private:
   //==========================================================================
   // P R I V A T E   M E T H O D S
   //==========================================================================
   /// @brief Clears all controls used to edit the runs.
   sqxVoid vResetRunEditControls();

   /// @brief Enables or disables all controls used to edit the runs.
   sqxVoid vEnableRunEditControls(sqxBool a_Enable);

   /// @brief Enables or disables all controls used to edit the run list.
   sqxVoid vEnableRunListControls(sqxBool a_Enable);

   /// @brief Sets the run editing controls to the values in a SimulationRun.
   sqxVoid vUpdateControlsFromRun(const SimulationRun& a_rRun);

   /// @brief Sets the values in a SimulationRun from the editing controls.
   sqxVoid vUpdateRunFromControls(SimulationRun& a_rRun);

   /// @brief Displays a browse dialog to open a file.
   ///
   /// @param a_rPathEdit The associated edit control that stores the path.
   /// @param a_pFilter The file type filter.
   sqxVoid vBrowsePath(CEdit& a_rPathEdit, const sqxWChar* a_pFilter);

   /// @brief Disables the UI and executes simulation runs
   /// This is a blocking call; it does not return until all runs have executed.
   sqxVoid vExecuteRuns(sqxBool a_All);

   void KillProcess();
   //==========================================================================
   // E V E N T   H A N D L E R S
   //==========================================================================
   afx_msg void OnSelectedRunChanged();
   afx_msg void OnAddRunButtonClicked();
   afx_msg void OnRemoveRunButtonClicked();
   afx_msg void OnMoveUpButtonClicked();
   afx_msg void OnMoveDownButtonClicked();
   afx_msg void OnRunNameChanged();
   afx_msg void OnBrowseDatabaseClicked();
   afx_msg void OnBrowseScenarioClicked();
   afx_msg void OnStopTimeCheckStateChanged();

   afx_msg void OnBnClickedPublisherBrowse();
   afx_msg void OnBnClickedSubscriberBrowse();
   afx_msg void OnRunSelectedButtonClicked();
   afx_msg void OnRunAllButtonClicked();
   afx_msg void OnCancelRunButtonClicked();
   afx_msg LRESULT OnAppendOutputLine(WPARAM wParam, LPARAM lParam);

   //==========================================================================
   // P R I V A T E   F I E L D S
   //==========================================================================
   StageBatchGuiControl& rControl;
   HWND WindowHandle;
   sqxInt SelectedRunIndex;
   
   CToolTipCtrl ToolTip;
   CEdit NameEdit;
   CEdit DatabaseEdit;
   CEdit ScenarioEdit;
   CEdit PublisherPathEdit;
   CEdit SubscriberPathEdit;
   CButton StopTimeCheck;
   CEdit StopTimeEdit;
   CListBox RunsListBox;
   CEdit OutputEdit;
   CButton CancelRunButton;

   


	
};
