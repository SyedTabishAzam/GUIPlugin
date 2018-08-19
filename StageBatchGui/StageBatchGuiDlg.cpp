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
#include "stdafx.h"

#include "StageBatchGuiDlg.h"
#include "StageBatchGuiControl.h"

#include <sqxOSSystem.h>

#include <sig_function.h>

#include <string>
#include <sstream>
#include <Constants.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static bool missionActivated = false;

//=============================================================================
// C O N S T A N T S
//=============================================================================
// The IDs of the controls used to edit run settings.
// This is used in the enabling/disabling logic.



static const int s_RunEditControlIDs[] =
{
   IDC_NAME_EDIT,
   IDC_DATABASE_EDIT,
   IDC_DATABASE_BROWSE_BUTTON,
   IDC_SCENARIO_EDIT,
   IDC_SCENARIO_BROWSE_BUTTON,
   IDC_ITERATIONS_EDIT,
   IDC_SNAPSHOT_TIMES_EDIT,
   IDC_DELAY_EDIT,
   IDC_SEED_EDIT,
   IDC_STOP_TIME_CHECK,
   IDC_STOP_TIME_EDIT,
   IDC_STOP_ENTITY_CHECK,
   IDC_STOP_ENTITY_EDIT,
   IDC_MIN_PLATFORM_COUNTS_CHECK,
   IDC_MIN_RED_PLATFORM_COUNT_EDIT,
   IDC_MIN_BLUE_PLATFORM_COUNT_EDIT
};

static const int s_RunEditControlCount =
   sizeof(s_RunEditControlIDs) / sizeof(s_RunEditControlIDs[0]);

// The IDs of the controls used to edit the run list.
// This is used in the enabling/disabling logic.
static const int s_RunListControlIDs[] =
{
   IDC_REMOVE_RUN_BUTTON,
   IDC_ADD_RUN_BUTTON,
   IDC_MOVE_RUN_UP_BUTTON,
   IDC_MOVE_RUN_DOWN_BUTTON,
   IDC_RUN_SELECTED_BUTTON,
   IDC_RUN_ALL_BUTTON
};

static const int s_RunListControlCount =
   sizeof(s_RunListControlIDs) / sizeof(s_RunListControlIDs[0]);

static const int WM_APPEND_OUTPUT_LINE = WM_USER + 1;

//=============================================================================
// L O C A L   F U N C T I O N S
//=============================================================================
template<typename TDest, typename TSource>
inline TDest lexical_cast(const TSource& a_rSource, TDest a_Default = TDest())
{
   std::wstringstream _StringStream;
   _StringStream << a_rSource;
   TDest _Result;
   _StringStream >> _Result;
   return _StringStream.fail() || _StringStream.bad() ? a_Default :_Result;
}

inline sqxVoid GetEditTextW(CEdit& a_rEdit, std::wstring& a_rString)
{
   a_rString.resize(static_cast<std::wstring::size_type>(a_rEdit.GetWindowTextLengthW()));
   a_rEdit.GetWindowTextW(&a_rString[0], static_cast<int>(a_rString.length()) + 1);
}

#define TO_WSTR(x) lexical_cast<std::wstring>(x).c_str()

//=============================================================================
// C O N S T R U C T O R S   A N D   D E S T R U C T O R S
//=============================================================================
CStageBatchGuiDlg::CStageBatchGuiDlg(StageBatchGuiControl& a_rControl)
	: CDialog(CStageBatchGuiDlg::IDD, NULL),
   rControl(a_rControl), WindowHandle(NULL), SelectedRunIndex(LB_ERR)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//=============================================================================
// P U B L I C   M E T H O D S
//=============================================================================
BOOL CStageBatchGuiDlg::PreTranslateMessage(MSG* pMsg)
{
  ToolTip.RelayEvent(pMsg);

  return CDialog::PreTranslateMessage(pMsg);
}

sqxVoid CStageBatchGuiDlg::vAppendOutputLine(const sqxWChar* a_pLine)
{
   ::SendMessageW(WindowHandle, WM_APPEND_OUTPUT_LINE,
      reinterpret_cast<WPARAM>(a_pLine), 0);
}

//=============================================================================
// M F C   G E N E R A T E D   C O D E
//=============================================================================
void CStageBatchGuiDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_NAME_EDIT, NameEdit);
   DDX_Control(pDX, IDC_DATABASE_EDIT, DatabaseEdit);
   DDX_Control(pDX, IDC_SCENARIO_EDIT, ScenarioEdit);
   DDX_Control(pDX, IDC_ITERATIONS_EDIT, IterationsEdit);
   DDX_Control(pDX, IDC_DELAY_EDIT, DelayEdit);
   DDX_Control(pDX, IDC_SEED_EDIT, SeedEdit);
   DDX_Control(pDX, IDC_STOP_TIME_CHECK, StopTimeCheck);
   DDX_Control(pDX, IDC_STOP_TIME_EDIT, StopTimeEdit);
   DDX_Control(pDX, IDC_STOP_ENTITY_CHECK, StopEntityCheck);
   DDX_Control(pDX, IDC_STOP_ENTITY_EDIT, StopEntityEdit);
   DDX_Control(pDX, IDC_RUN_LIST, RunsListBox);
   DDX_Control(pDX, IDC_OUTPUT_EDIT, OutputEdit);
   DDX_Control(pDX, IDC_CANCEL_RUN_BUTTON, CancelRunButton);
   DDX_Control(pDX, IDC_ACTIVATE_MISSION, ActivateMissionButton);
   DDX_Control(pDX, IDC_DEAC_MISSION, DeactivateButton);
   DDX_Control(pDX, IDC_ENTITY_MISSION, MissionEntityEdit);
   DDX_Control(pDX, IDC_SNAPSHOT_TIMES_EDIT, SnapshotTimesEdit);
   DDX_Control(pDX, IDC_MIN_PLATFORM_COUNTS_CHECK, MinPlatformCountsCheck);
   DDX_Control(pDX, IDC_MIN_RED_PLATFORM_COUNT_EDIT, MinRedPlatformCountEdit);
   DDX_Control(pDX, IDC_MIN_BLUE_PLATFORM_COUNT_EDIT, MinBluePlatformCountEdit);
}

BEGIN_MESSAGE_MAP(CStageBatchGuiDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
   ON_LBN_SELCHANGE(IDC_RUN_LIST, &CStageBatchGuiDlg::OnSelectedRunChanged)
   ON_BN_CLICKED(IDC_ADD_RUN_BUTTON, &CStageBatchGuiDlg::OnAddRunButtonClicked)
   ON_EN_CHANGE(IDC_NAME_EDIT, &CStageBatchGuiDlg::OnRunNameChanged)
   ON_BN_CLICKED(IDC_REMOVE_RUN_BUTTON, &CStageBatchGuiDlg::OnRemoveRunButtonClicked)
   ON_BN_CLICKED(IDC_DATABASE_BROWSE_BUTTON, &CStageBatchGuiDlg::OnBrowseDatabaseClicked)
   ON_BN_CLICKED(IDC_SCENARIO_BROWSE_BUTTON, &CStageBatchGuiDlg::OnBrowseScenarioClicked)
   ON_BN_CLICKED(IDC_MOVE_RUN_UP_BUTTON, &CStageBatchGuiDlg::OnMoveUpButtonClicked)
   ON_BN_CLICKED(IDC_MOVE_RUN_DOWN_BUTTON, &CStageBatchGuiDlg::OnMoveDownButtonClicked)
   ON_BN_CLICKED(IDC_STOP_TIME_CHECK, &CStageBatchGuiDlg::OnStopTimeCheckStateChanged)
   ON_BN_CLICKED(IDC_STOP_ENTITY_CHECK, &CStageBatchGuiDlg::OnStopEntityCheckStateChanged)
   ON_BN_CLICKED(IDC_RUN_SELECTED_BUTTON, &CStageBatchGuiDlg::OnRunSelectedButtonClicked)
   ON_BN_CLICKED(IDC_RUN_ALL_BUTTON, &CStageBatchGuiDlg::OnRunAllButtonClicked)
   ON_MESSAGE(WM_APPEND_OUTPUT_LINE, OnAppendOutputLine)
   ON_BN_CLICKED(IDC_CANCEL_RUN_BUTTON, &CStageBatchGuiDlg::OnCancelRunButtonClicked)
   ON_BN_CLICKED(IDC_MIN_PLATFORM_COUNTS_CHECK, &CStageBatchGuiDlg::OnMinPlatformCountsCheckStateChanged)
   ON_BN_CLICKED(IDC_ACTIVATE_MISSION, &CStageBatchGuiDlg::OnBnClickedActivateMission)
END_MESSAGE_MAP()


BOOL CStageBatchGuiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

   // No run is initially selected
   vEnableRunEditControls(FALSE);
   WindowHandle = GetSafeHwnd();

   if (ToolTip.Create(this))
	{
      ToolTip.AddTool(&NameEdit,
         L"The name of this simulation run, "
         L"this value has no impact on the execution.");
      ToolTip.AddTool(&DatabaseEdit,
         L"The path to the Scenario Manager database. "
         L"This can be only a filename if the database is in the standard folder.");
      ToolTip.AddTool(&ScenarioEdit,
         L"The path to the scenario or snapshot file to be used. "
         L"This can be only a filename for scenarios under the 'scenario' folder of their databases.");
      ToolTip.AddTool(&IterationsEdit,
         L"The number of times to execute this simulation run.");
      ToolTip.AddTool(&SnapshotTimesEdit,
         L"The times, in seconds and separated by spaces, "
         L"at which simulation snapshots should be taken.");
      ToolTip.AddTool(&DelayEdit,
         L"The time, in seconds, to wait before every iteration of this simulation run.");
      ToolTip.AddTool(&SeedEdit,
         L"The random number generator seed used for every iteration. "
         L"A value of zero will use a random seed.");
      ToolTip.AddTool(&StopTimeEdit,
         L"The simulation time after which the simulation run should stop. "
         L"As the simulation runs in asynchronous mode (as fast as possible), "
         L"the actual execution time can be less than this value.");
      ToolTip.AddTool(&StopEntityEdit,
         L"The name of the entity whose death causes the simulation run to stop.");
      ToolTip.AddTool(&MinRedPlatformCountEdit,
         L"The minimum number of red platforms below which the simulation should stop.");
      ToolTip.AddTool(&MinBluePlatformCountEdit,
         L"The minimum number of blue platforms below which the simulation should stop.");
      ToolTip.Activate(TRUE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CStageBatchGuiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CStageBatchGuiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//=============================================================================
// P R I V A T E   M E T H O D S
//=============================================================================
void CStageBatchGuiDlg::vResetRunEditControls()
{
   for (int i = 0; i < s_RunEditControlCount; ++i)
   {
      int _ControlID = s_RunEditControlIDs[i];
      CWnd* _pControl = GetDlgItem(_ControlID);
      if (_pControl == NULL) continue;

      _pControl->EnableWindow(FALSE);

      CEdit* _pEdit = dynamic_cast<CEdit*>(_pControl);
      if (_pEdit != SQX_NULL) _pEdit->SetWindowTextW(L"");
      
      CButton* _pButton = dynamic_cast<CButton*>(_pControl);
      if (_pButton != SQX_NULL) _pButton->SetCheck(BST_UNCHECKED);
   }
}

void CStageBatchGuiDlg::vEnableRunEditControls(sqxBool a_Enable)
{
   for (int i = 0; i < s_RunEditControlCount; ++i)
   {
      int _ControlID = s_RunEditControlIDs[i];
      CWnd* _pControl = GetDlgItem(_ControlID);
      if (_pControl != NULL) _pControl->EnableWindow(a_Enable ? TRUE : FALSE);
   }
}

void CStageBatchGuiDlg::vEnableRunListControls(sqxBool a_Enable)
{
   for (int i = 0; i < s_RunListControlCount; ++i)
   {
      int _ControlID = s_RunListControlIDs[i];
      CWnd* _pControl = GetDlgItem(_ControlID);
      if (_pControl != NULL) _pControl->EnableWindow(a_Enable ? TRUE : FALSE);
   }
}

void CStageBatchGuiDlg::vUpdateControlsFromRun(const SimulationRun& a_rRun)
{
   // General
   NameEdit.SetWindowTextW(a_rRun.Name.c_str());
   DatabaseEdit.SetWindowTextW(a_rRun.DatabaseFilePath.c_str());
   ScenarioEdit.SetWindowTextW(a_rRun.ScenarioFilePath.c_str());
   IterationsEdit.SetWindowTextW(TO_WSTR(a_rRun.IterationCount));

   // Convert the snapshot times to a string
   std::wstring _SnapshotTimesString;
   for (size_t i = 0; i < a_rRun.SnapshotTimesInSeconds.size(); ++i)
   {
      if (i > 0) _SnapshotTimesString += L" ";

      sqxDouble _SnapshotTimeInSeconds = a_rRun.SnapshotTimesInSeconds[i];
      _SnapshotTimesString += TO_WSTR(_SnapshotTimeInSeconds);
   }
   SnapshotTimesEdit.SetWindowTextW(_SnapshotTimesString.c_str());

   // Startup
   DelayEdit.SetWindowTextW(TO_WSTR(a_rRun.DelayInSeconds));
   SeedEdit.SetWindowTextW(TO_WSTR(a_rRun.Seed));

   // Stop Conditions
   StopTimeCheck.SetCheck((a_rRun.stopConditions & StopConditions_ElapsedTime)
      ? BST_CHECKED : BST_UNCHECKED);
   char _TimeBuffer[9] = "\0";
   float stopTime = 36000;
   sig_time_to_str(stopTime, _TimeBuffer);
   StopTimeEdit.SetWindowTextW(g_pOSSystem->AnsiToUnicode(_TimeBuffer).c_str());
 
   StopTimeEdit.EnableWindow((a_rRun.stopConditions & StopConditions_ElapsedTime)
      ? TRUE : FALSE);

   StopEntityCheck.SetCheck((a_rRun.stopConditions & StopConditions_DeadEntity)
      ? BST_CHECKED : BST_UNCHECKED);
   StopEntityEdit.SetWindowTextW(a_rRun.StopEntityName.c_str());
   StopEntityEdit.EnableWindow((a_rRun.stopConditions & StopConditions_DeadEntity)
      ? TRUE : FALSE);
   
   MinPlatformCountsCheck.SetCheck((a_rRun.stopConditions & StopConditions_MinPlatforms)
      ? BST_CHECKED : BST_UNCHECKED);
   MinRedPlatformCountEdit.SetWindowTextW(TO_WSTR(a_rRun.MinRedPlatformCount));
   MinRedPlatformCountEdit.EnableWindow((a_rRun.stopConditions & StopConditions_MinPlatforms)
      ? TRUE : FALSE);
   MinBluePlatformCountEdit.SetWindowTextW(TO_WSTR(a_rRun.MinBluePlatformCount));
   MinBluePlatformCountEdit.EnableWindow((a_rRun.stopConditions & StopConditions_MinPlatforms)
      ? TRUE : FALSE);
}

void CStageBatchGuiDlg::vUpdateRunFromControls(SimulationRun& a_rRun)
{
   CString _TempString;

   // General
   GetEditTextW(NameEdit, a_rRun.Name);
   GetEditTextW(DatabaseEdit, a_rRun.DatabaseFilePath);
   GetEditTextW(ScenarioEdit, a_rRun.ScenarioFilePath);
   
   // Consider it a snapshot restore if it has the snapshot file extension
   std::wstring::size_type _DotIndex = a_rRun.ScenarioFilePath.find_last_of('.');
   a_rRun.IsSnapshotRestore = _DotIndex != std::wstring::npos
      && a_rRun.ScenarioFilePath.substr(_DotIndex + 1) == L"snb";

   IterationsEdit.GetWindowTextW(_TempString);
   a_rRun.IterationCount = lexical_cast<sqxUInt>(_TempString.GetString(), 0);

   // Parse the snapshot times.
   SnapshotTimesEdit.GetWindowTextW(_TempString);
   std::wstringstream _SnapshotTimesStringStream;
   _SnapshotTimesStringStream << _TempString.GetString();
   a_rRun.SnapshotTimesInSeconds.clear();
   while (!_SnapshotTimesStringStream.eof())
   {
      sqxDouble _SnapshotTimeInSeconds;
      _SnapshotTimesStringStream >> _SnapshotTimeInSeconds;
      if (_SnapshotTimesStringStream.fail()
         || _SnapshotTimesStringStream.bad())
      {
         // Parsing error, stop
         break;
      }

      a_rRun.SnapshotTimesInSeconds.push_back(_SnapshotTimeInSeconds);
   }

   // Startup
   DelayEdit.GetWindowTextW(_TempString);
   a_rRun.DelayInSeconds = lexical_cast<sqxDouble>(_TempString.GetString(), 0);
   
   SeedEdit.GetWindowTextW(_TempString);
   a_rRun.Seed = lexical_cast<sqxUInt>(_TempString.GetString(), 0);

   // Stop Conditions
   int _StopConditions = StopConditions_None;

   if (StopTimeCheck.GetCheck() == BST_CHECKED)
      _StopConditions |= StopConditions_ElapsedTime;

   StopTimeEdit.GetWindowTextW(_TempString);
   float _StopTimeInSeconds;
   std::string _StopTimeString = g_pOSSystem->UnicodeToAnsi(_TempString.GetString());
   if (sig_str_to_time(&_StopTimeInSeconds, &_StopTimeString[0]) == 0)
   {
      a_rRun.StopTimeInSeconds = _StopTimeInSeconds;
   }

   if (StopEntityCheck.GetCheck() == BST_CHECKED)
      _StopConditions |= StopConditions_DeadEntity;

   StopEntityEdit.GetWindowTextW(_TempString);
   a_rRun.StopEntityName = _TempString.GetString();
   
   if (MinPlatformCountsCheck.GetCheck() == BST_CHECKED)
      _StopConditions |= StopConditions_MinPlatforms;
   
   MinBluePlatformCountEdit.GetWindowTextW(_TempString);
   a_rRun.MinBluePlatformCount = lexical_cast<sqxUInt>(_TempString.GetString(), 0);
   MinRedPlatformCountEdit.GetWindowTextW(_TempString);
   a_rRun.MinRedPlatformCount = lexical_cast<sqxUInt>(_TempString.GetString(), 0);

   a_rRun.stopConditions = static_cast<StopConditions>(_StopConditions);
}

sqxVoid CStageBatchGuiDlg::vBrowsePath(CEdit& a_rPathEdit, const sqxWChar* a_pFilter)
{
   CString _FilePath;
   a_rPathEdit.GetWindowTextW(_FilePath);

   // File dialogs must be created every time, they cannot be recycled.
   CFileDialog _Dialog(TRUE);
   OPENFILENAME& _rOFN = _Dialog.GetOFN();
   _rOFN.lpstrFile = _FilePath.GetBuffer(_MAX_PATH + 1);
   _rOFN.lpstrFilter = a_pFilter;
   if (_Dialog.DoModal() == IDOK)
   {
      a_rPathEdit.SetWindowTextW(_rOFN.lpstrFile);
   }
}

sqxVoid CStageBatchGuiDlg::vExecuteRuns(sqxBool a_All)
{
   if (rControl.GetRunCount() == 0) return;

   // "Commit" any changes in the controls to the SimulationRun object.
   SimulationRun* _pSelectedRun;
   if (SelectedRunIndex == LB_ERR)
   {
      if (!a_All) return;
      _pSelectedRun = SQX_NULL;
   }
   else
   {
      _pSelectedRun = rControl.pGetRunAt(SelectedRunIndex);
      vUpdateRunFromControls(*_pSelectedRun);
   }

   // Disable edit controls
   vEnableRunEditControls(SQX_FALSE);
   vEnableRunListControls(SQX_FALSE);
   OutputEdit.SetWindowTextW(L"");
   CancelRunButton.EnableWindow(TRUE);
   ActivateMissionButton.EnableWindow(TRUE);
   MissionEntityEdit.EnableWindow(TRUE);
   // Begin the asynchronous execution
   rControl.vBeginExecution(a_All ? -1 : SelectedRunIndex);

   // Wait for the run to complete, with a nested message loop to stay responsive.
   // Taken from http://msdn.microsoft.com/en-us/library/3dy7kd92.aspx
   
   while (rControl.IsExecuting()) 
   { 
      MSG msg;
      while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
      {
         if (!AfxGetThread()->PumpMessage())
         { 
            PostQuitMessage(0); 
            return;
         } 
      } 

      // let MFC do its idle processing
      LONG lIdle = 0;
      while (AfxGetApp()->OnIdle(lIdle)) lIdle++;
   }

   // Re-enable disabled controls
   CancelRunButton.EnableWindow(FALSE);
   ActivateMissionButton.EnableWindow(FALSE);
   MissionEntityEdit.EnableWindow(FALSE);
   vEnableRunEditControls(SQX_TRUE);
   vEnableRunListControls(SQX_TRUE);

   if (_pSelectedRun != SQX_NULL)
   {
      // Re-update the controls so conditionnally enabled fields are
      // reverted to their proper state
      vUpdateControlsFromRun(*_pSelectedRun);
   }
}

//=============================================================================
// E V E N T   H A N D L E R S
//=============================================================================
void CStageBatchGuiDlg::OnSelectedRunChanged()
{
   int _NewSelectionIndex = RunsListBox.GetCurSel();
   if (_NewSelectionIndex == SelectedRunIndex) return;

   if (SelectedRunIndex != LB_ERR)
      vUpdateRunFromControls(*rControl.pGetRunAt(SelectedRunIndex));

   SelectedRunIndex = _NewSelectionIndex;

   vEnableRunEditControls(SelectedRunIndex != LB_ERR);
   if (SelectedRunIndex != LB_ERR)
      vUpdateControlsFromRun(*rControl.pGetRunAt(SelectedRunIndex));
}

void CStageBatchGuiDlg::OnAddRunButtonClicked()
{
   rControl.vAddRun();

   SimulationRun& _rNewRun = *rControl.pGetRunAt(rControl.GetRunCount() - 1);
   if (SelectedRunIndex != LB_ERR)
   {
      // If there is a selection, make the new run a clone of the current 
      // one, except for the name.
      SimulationRun& _rSelectedRun = *rControl.pGetRunAt(SelectedRunIndex);
      vUpdateRunFromControls(_rSelectedRun);

      std::wstring _NewRunName = _rNewRun.Name;
      _rNewRun = _rSelectedRun;
      _rNewRun.Name = _NewRunName;
   }

   RunsListBox.AddString(_rNewRun.Name.c_str());
   
}

void CStageBatchGuiDlg::OnRemoveRunButtonClicked()
{
   if (SelectedRunIndex == LB_ERR) return;

   rControl.vRemoveRunAt(SelectedRunIndex);
   RunsListBox.DeleteString(static_cast<UINT>(SelectedRunIndex));

   if (SelectedRunIndex >= static_cast<sqxInt>(rControl.GetRunCount()))
      SelectedRunIndex = static_cast<sqxInt>(rControl.GetRunCount()) - 1;

   if (SelectedRunIndex == -1)
   {
      SelectedRunIndex = LB_ERR;
      vResetRunEditControls();
   }
   else
   {
      vUpdateControlsFromRun(*rControl.pGetRunAt(SelectedRunIndex));
   }
   
   RunsListBox.SetCurSel(SelectedRunIndex);
}

void CStageBatchGuiDlg::OnRunNameChanged()
{
   if (SelectedRunIndex == LB_ERR) return;

   CString _NewName;
   NameEdit.GetWindowTextW(_NewName);

   // Update the name in the listbox
   RunsListBox.DeleteString(SelectedRunIndex);
   RunsListBox.InsertString(SelectedRunIndex, _NewName);
   RunsListBox.SetCurSel(SelectedRunIndex);
}

void CStageBatchGuiDlg::OnBrowseDatabaseClicked()
{
   vBrowsePath(DatabaseEdit, L"Database Files\0*.xml\0");
}

void CStageBatchGuiDlg::OnBrowseScenarioClicked()
{
   vBrowsePath(ScenarioEdit, L"Scenario and Snapshot Files\0*.scenario;*.snb\0");
}

void CStageBatchGuiDlg::OnMoveUpButtonClicked()
{
   if (SelectedRunIndex == LB_ERR || SelectedRunIndex == 0) return;

   rControl.vMoveRun(SelectedRunIndex, SelectedRunIndex - 1);
   --SelectedRunIndex;

   SimulationRun* _pRun = rControl.pGetRunAt(SelectedRunIndex);
   RunsListBox.DeleteString(SelectedRunIndex + 1);
   RunsListBox.InsertString(SelectedRunIndex, _pRun->Name.c_str());
   RunsListBox.SetCurSel(SelectedRunIndex);
}

void CStageBatchGuiDlg::OnMoveDownButtonClicked()
{
   if (SelectedRunIndex == LB_ERR
      || static_cast<sqxUInt>(SelectedRunIndex + 1) == rControl.GetRunCount())
      return;

   rControl.vMoveRun(SelectedRunIndex, SelectedRunIndex + 1);
   ++SelectedRunIndex;

   SimulationRun* _pRun = rControl.pGetRunAt(SelectedRunIndex);
   RunsListBox.DeleteString(SelectedRunIndex - 1);
   RunsListBox.InsertString(SelectedRunIndex, _pRun->Name.c_str());
   RunsListBox.SetCurSel(SelectedRunIndex);
}

void CStageBatchGuiDlg::OnStopTimeCheckStateChanged()
{
   StopTimeEdit.EnableWindow(
      StopTimeCheck.GetCheck() == BST_CHECKED ? TRUE : FALSE);
}

void CStageBatchGuiDlg::OnStopEntityCheckStateChanged()
{
   StopEntityEdit.EnableWindow(
      StopEntityCheck.GetCheck() == BST_CHECKED ? TRUE : FALSE);
}

void CStageBatchGuiDlg::OnMinPlatformCountsCheckStateChanged()
{
   BOOL _Enabled = MinPlatformCountsCheck.GetCheck() == BST_CHECKED ? TRUE : FALSE;
   MinRedPlatformCountEdit.EnableWindow(_Enabled);
   MinBluePlatformCountEdit.EnableWindow(_Enabled);
}

void CStageBatchGuiDlg::OnRunSelectedButtonClicked()
{
	vExecuteRuns(SQX_FALSE);
	
	
	MissionEntityEdit.EnableWindow(TRUE);

	/* Open batch file */
	char file_name[256];
	sprintf(file_name, "S:\\presagis\\stage\\GUIPlugin\\%s", SIM_MISSION_FILE);
	
	FILE* _pFile = fopen(SIM_MISSION_FILE, "w");
	if (_pFile == NULL)
	{
		

	}
	else
	{
		fprintf(_pFile, "FALSE %s\n", " ");
	}


	fclose(_pFile);

}

void CStageBatchGuiDlg::OnRunAllButtonClicked()
{
   vExecuteRuns(SQX_TRUE);
}

void CStageBatchGuiDlg::OnCancelRunButtonClicked()
{
   // Simulation runs are being executed or the button would have been disabled
	
   CancelRunButton.EnableWindow(FALSE);
   rControl.vBeginExecutionCancellation();

   DeleteEntityData();
}

void CStageBatchGuiDlg::DeleteEntityData()
{
	char file_name[256];
	sprintf(file_name, "S:\\presagis\\stage\\GUIPlugin\\%s", SIM_MISSION_FILE);

	FILE* _pFile = fopen(SIM_MISSION_FILE, "w");
	if (_pFile == NULL)
	{


	}
	else
	{
		
	}


	fclose(_pFile);
}
LRESULT CStageBatchGuiDlg::OnAppendOutputLine(WPARAM wParam, LPARAM)
{
   // Append text to the end of the output window
   int _Length = OutputEdit.GetWindowTextLength();
   OutputEdit.SetSel(_Length, _Length);
   OutputEdit.ReplaceSel(reinterpret_cast<const sqxWChar*>(wParam));

   // Append a line feed
   _Length = OutputEdit.GetWindowTextLength();
   OutputEdit.SetSel(_Length, _Length);
   OutputEdit.ReplaceSel(_T("\r\n"));

   return 0;
}


void CStageBatchGuiDlg::OnBnClickedActivateMission()
{
	// TODO: Add your control notification handler code here
	
	
	std::wstring entityName;
	GetEditTextW(MissionEntityEdit, entityName);
	
	
	char ent[25] ;

	int i = 0;
	for (;;)
	{
		ent[i] = (wchar_t)entityName[i];
		if (entityName[i] == '\0') break;
		++i;
	}

	/* Open batch file */
	char file_name[256];


	sprintf(file_name, "%s%cGUIPlugin%c%s",
		"S:\\presagis\\stage",
		'\\', '\\', SIM_MISSION_FILE);
	FILE* _pFile = fopen(SIM_MISSION_FILE, "w");
	if (_pFile == NULL) 
	{
		DeactivateButton.EnableWindow(FALSE);
		ActivateMissionButton.EnableWindow(TRUE);
		
	}
	else
	{
		DeactivateButton.EnableWindow(TRUE);
		ActivateMissionButton.EnableWindow(FALSE);
		fprintf(_pFile, "TRUE %s \n", ent);
	}
	

	fclose(_pFile);

}

void store_entity_name(char* buf_ptr)
{
	
}


