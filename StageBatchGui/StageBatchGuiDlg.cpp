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
#include "ProcessHandle.h"
#include <sqxOSSystem.h>
#include <fstream>
#include <sig_function.h>
#include <tlhelp32.h>
#include <string>
#include <sstream>
#include <Constants.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
using namespace std;
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
   IDC_RUN_SELECTED_BUTTON,
   IDC_STOP_TIME_CHECK,
   IDC_STOP_TIME_EDIT,
   
   IDC_PUBLISHERPATH_EDIT,
   IDC_PUBLISHER_BROWSE,
   IDC_SUBSCRIBERPATH_EDIT,
   IDC_SUBSCRIBER_BROWSE

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
   
};

static const int s_RunListControlCount =
   sizeof(s_RunListControlIDs) / sizeof(s_RunListControlIDs[0]);

static const int WM_APPEND_OUTPUT_LINE = WM_USER + 1;
static const int WM_APPEND_SERVER_LINE = WM_USER + 2;
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

sqxVoid CStageBatchGuiDlg::vAppendServerLine(const sqxWChar* a_pLine)
{
	::SendMessageW(WindowHandle, WM_APPEND_SERVER_LINE,
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

   DDX_Control(pDX, IDC_STOP_TIME_CHECK, StopTimeCheck);
   DDX_Control(pDX, IDC_STOP_TIME_EDIT, StopTimeEdit);
   DDX_Control(pDX, IDC_RUN_LIST, RunsListBox);
   DDX_Control(pDX, IDC_LIST2, ConnectionsCCC);
   DDX_Control(pDX, IDC_LIST1, ConnectionsDef);
   DDX_Control(pDX, IDC_LIST3, ConnectionsAtt);
   DDX_Control(pDX, IDC_OUTPUT_EDIT, OutputEdit);
   DDX_Control(pDX, IDC_OUTPUT_EDIT2, ServerEdit);
   DDX_Control(pDX, IDC_CANCEL_RUN_BUTTON, CancelRunButton);
   DDX_Control(pDX, IDC_PUBLISHERPATH_EDIT, PublisherPathEdit);
   DDX_Control(pDX, IDC_SUBSCRIBERPATH_EDIT, SubscriberPathEdit);
}

BEGIN_MESSAGE_MAP(CStageBatchGuiDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
   ON_LBN_SELCHANGE(IDC_RUN_LIST, &CStageBatchGuiDlg::OnSelectedRunChanged)
   ON_BN_CLICKED(IDC_ADD_RUN_BUTTON, &CStageBatchGuiDlg::OnAddRunButtonClicked)
 
 
   ON_BN_CLICKED(IDC_REMOVE_RUN_BUTTON, &CStageBatchGuiDlg::OnRemoveRunButtonClicked)
   ON_BN_CLICKED(IDC_DATABASE_BROWSE_BUTTON, &CStageBatchGuiDlg::OnBrowseDatabaseClicked)
   ON_BN_CLICKED(IDC_SCENARIO_BROWSE_BUTTON, &CStageBatchGuiDlg::OnBrowseScenarioClicked)
   ON_BN_CLICKED(IDC_MOVE_RUN_UP_BUTTON, &CStageBatchGuiDlg::OnMoveUpButtonClicked)
   ON_BN_CLICKED(IDC_MOVE_RUN_DOWN_BUTTON, &CStageBatchGuiDlg::OnMoveDownButtonClicked)
   ON_BN_CLICKED(IDC_STOP_TIME_CHECK, &CStageBatchGuiDlg::OnStopTimeCheckStateChanged)
  
   ON_BN_CLICKED(IDC_RUN_SELECTED_BUTTON, &CStageBatchGuiDlg::OnRunSelectedButtonClicked)
 
   ON_MESSAGE(WM_APPEND_OUTPUT_LINE, OnAppendOutputLine)
   ON_MESSAGE(WM_APPEND_SERVER_LINE, OnAppendServerLine)
   ON_BN_CLICKED(IDC_CANCEL_RUN_BUTTON, &CStageBatchGuiDlg::OnCancelRunButtonClicked)
   ON_BN_CLICKED(IDC_PUBLISHER_BROWSE, &CStageBatchGuiDlg::OnBnClickedPublisherBrowse)
   ON_BN_CLICKED(IDC_SUBSCRIBER_BROWSE, &CStageBatchGuiDlg::OnBnClickedSubscriberBrowse)
   ON_LBN_SELCHANGE(IDC_LIST3, &CStageBatchGuiDlg::OnLbnSelchangeList3)
   ON_EN_CHANGE(IDC_OUTPUT_EDIT, &CStageBatchGuiDlg::OnEnChangeOutputEdit)
   ON_BN_CLICKED(IDC_BUTTON1, &CStageBatchGuiDlg::OnBnClickedButton1)
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
	  ToolTip.AddTool(&PublisherPathEdit,
		  L"The path to the publisher path to be used. "
		  L"This is the exe file used to publish initial scenerio data.");
	  ToolTip.AddTool(&SubscriberPathEdit,
		  L"The path to the subscriber to be used. "
		  L"This is the listener used to get commands from another commander (publisher).");
   
      ToolTip.AddTool(&StopTimeEdit,
         L"The simulation time after which the simulation run should stop. "
         L"As the simulation runs in asynchronous mode (as fast as possible), "
         L"the actual execution time can be less than this value.");
  


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
   PublisherPathEdit.SetWindowTextW(a_rRun.InitPublisherPath.c_str());
   SubscriberPathEdit.SetWindowTextW(a_rRun.CommandSubscriberPath.c_str());
   


   // Stop Conditions
   StopTimeCheck.SetCheck((a_rRun.stopConditions & StopConditions_ElapsedTime)
      ? BST_CHECKED : BST_UNCHECKED);

   char _TimeBuffer[9] = "\0";
   float stopTime = 36000;
   sig_time_to_str(stopTime, _TimeBuffer);
   StopTimeEdit.SetWindowTextW(g_pOSSystem->AnsiToUnicode(_TimeBuffer).c_str());
 
   StopTimeEdit.EnableWindow(TRUE);
   StopTimeCheck.SetCheck(TRUE);
  
   
  
}

void CStageBatchGuiDlg::vUpdateRunFromControls(SimulationRun& a_rRun)
{
   CString _TempString;

   // General
   GetEditTextW(NameEdit, a_rRun.Name);
   GetEditTextW(DatabaseEdit, a_rRun.DatabaseFilePath);
   GetEditTextW(ScenarioEdit, a_rRun.ScenarioFilePath);
   GetEditTextW(PublisherPathEdit, a_rRun.InitPublisherPath);
   GetEditTextW(SubscriberPathEdit, a_rRun.CommandSubscriberPath);


   // Startup
   

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
   ServerEdit.SetWindowTextW(L"");
   CancelRunButton.EnableWindow(TRUE);
   
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
   
   
   vEnableRunEditControls(SQX_TRUE);
   vEnableRunListControls(SQX_TRUE);

   if (_pSelectedRun != SQX_NULL)
   {
      // Re-update the controls so conditionnally enabled fields are
      // reverted to their proper state
      vUpdateControlsFromRun(*_pSelectedRun);
   }
}

void CStageBatchGuiDlg::KillProcess()
{
	HANDLE hProcessSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

	PROCESSENTRY32 ProcessEntry = { 0 };
	ProcessEntry.dwSize = sizeof(ProcessEntry);

	BOOL Return = FALSE;
	Label:Return = Process32First(hProcessSnapShot, &ProcessEntry);

	if (!Return)
	{
		goto Label;
	}

	do
	{
		int value = -1;
		value = _tcsicmp(ProcessEntry.szExeFile, _T("CommandSubscriber.exe"));
		//replace the taskmgr.exe to the process u want to remove.
		if (value == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, ProcessEntry.th32ProcessID);
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}

	} while (Process32Next(hProcessSnapShot, &ProcessEntry));

	CloseHandle(hProcessSnapShot);
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



void CStageBatchGuiDlg::OnRunSelectedButtonClicked()
{
	vExecuteRuns(SQX_FALSE);
	DeleteEntityData();
}



void CStageBatchGuiDlg::OnCancelRunButtonClicked()
{
	CString message;
	message.Format(L"Warning! Canceling simulation from 'Stage Engine' might cause sync problems with 'Player Engine'. It is recommended to cancel the simulation through 'Player Engine'. Do you still want to continue?");

	// Show the message box with a question mark icon
	
	const int result = MessageBox(message, L"Warning", MB_ICONWARNING | MB_YESNO);
   // Simulation runs are being executed or the button would have been disabled


	switch (result)
	{
	case IDYES:
		CancelRunButton.EnableWindow(FALSE);
		rControl.vBeginExecutionCancellation();
		KillProcess();
		break;
	case IDNO:
		break;
	}
   
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

LRESULT CStageBatchGuiDlg::OnAppendServerLine(WPARAM wParam, LPARAM)
{
	// Append text to the end of the output window
	int _Length = ServerEdit.GetWindowTextLength();
	ServerEdit.SetSel(_Length, _Length);
	ServerEdit.ReplaceSel(reinterpret_cast<const sqxWChar*>(wParam));

	// Append a line feed
	_Length = ServerEdit.GetWindowTextLength();
	ServerEdit.SetSel(_Length, _Length);
	ServerEdit.ReplaceSel(_T("\r\n"));

	return 0;
}


void CStageBatchGuiDlg::OnBnClickedPublisherBrowse()
{
	vBrowsePath(PublisherPathEdit, L"Publisher file\0*.exe\0");
}


void CStageBatchGuiDlg::OnBnClickedSubscriberBrowse()
{
	vBrowsePath(SubscriberPathEdit, L"Subscriber file\0*.exe\0");
}


void CStageBatchGuiDlg::OnLbnSelchangeList3()
{
	// TODO: Add your control notification handler code here
}


void CStageBatchGuiDlg::OnEnChangeOutputEdit()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CStageBatchGuiDlg::OnBnClickedButton1()
{
	vStartServer();

}

sqxVoid CStageBatchGuiDlg::vStartServer()
{
	ProcessHandle *pHandle = new ProcessHandle();
	
	CWinThread *m_pPointerToThread;
	DWORD exitCode;
	if (pHandle->StartProcess(exitCode,"PUBLISHER"))
	{
		if (pHandle->StartProcess(exitCode,"SUBSCRIBER"))
		{
			m_pPointerToThread = AfxBeginThread(ServerBegin, static_cast<LPVOID>(this));
		}
	}


	if (!m_pPointerToThread)
		vAppendServerLine(L"Server failed to start");
	else
	{
		vAppendServerLine(L"Server Started");
	}

}





UINT CStageBatchGuiDlg::ServerBegin(LPVOID param)
{
	CStageBatchGuiDlg *m_pPointerToDialog = static_cast<CStageBatchGuiDlg*>(param);

	// call a function: CSpecifiedDialog::ABC() in the same dialog
	m_pPointerToDialog->UpdateConnections();

	

	return 0;
}

sqxVoid CStageBatchGuiDlg::UpdateConnections()
{
	while (true)
	{
		Sleep(3000);
		CheckCCC();
	}
}

sqxBool CStageBatchGuiDlg::CheckAttackers()
{
	return SQX_TRUE;
}

sqxBool CStageBatchGuiDlg::CheckCCC()
{

	string STAGE_DIR = getenv("STAGE_DIR");
	string fileName = STAGE_DIR + "\\GUIPlugin\\" + SERVER_CONNECTION;

	fstream fs;
	fs.open(fileName, std::fstream::in );

	if (fs.fail())
	{
		vAppendServerLine(L"Waiting for connections..");
	}
	else
	{
		
		int currentCrisis = 0;
		int currentDefenders = 0;
		int currentAttackers = 0;
		string line;
		while (getline(fs, line))
		{
			

			CString str(line.c_str());

			if (line.find("CCC") != std::string::npos)
			{
				currentCrisis++;
				
			}
			if (line.find("ATTACKER") != std::string::npos)
			{
				currentAttackers++;
				
			}
			if (line.find("DEFENDER") != std::string::npos)
			{
				currentDefenders++;
				
			}
			
		}
		UpdateConnections(totalCrisisConnection, currentCrisis, "CCC");
		UpdateConnections(totalAttackers, currentAttackers, "ATTACKER");
		UpdateConnections(totalDefenders, currentDefenders, "DEFENDER");
		
		
		
		
		



		fs.close();
		
	}
	

	
	
	
	
	return SQX_FALSE;
}

sqxVoid CStageBatchGuiDlg::UpdateConnections(int & initial, int final, std::string type)
{
	while (initial != final)
	{
		if (initial < final)
		{
			initial++;
			char buffer[50];
			sprintf(buffer, "%s %d joined the server", type, initial);
			CString str(buffer);
			vAppendServerLine(str);

			sprintf(buffer, "%s %d", type, initial);
			CString str2(buffer);

			if (type == "CCC")
				ConnectionsCCC.AddString(str2);
			else if (type == "DEFENDER")
				ConnectionsDef.AddString(str2);
			else if (type == "ATTACKER")
				ConnectionsAtt.AddString(str2);
			
		}
		else if (initial > final)
		{
			char buffer[50];
			sprintf(buffer, "%s %d left the server", type, initial);
			CString str(buffer);
			vAppendServerLine(str);

			if (type == "CCC")
				ConnectionsCCC.DeleteString(static_cast<UINT>(final));
			else if (type == "DEFENDER")
				ConnectionsDef.DeleteString(static_cast<UINT>(final));
			else if (type == "ATTACKER")
				ConnectionsAtt.DeleteString(static_cast<UINT>(final));
			initial--;
		}
	}
	/*if (initial < final)
	{

		
		
		
		
	}
	else if (initial > final)
	{

		
		
	}
	initial = final;*/
}



sqxBool CStageBatchGuiDlg::CheckDefenders()
{
	return SQX_TRUE;
}

