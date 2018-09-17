#include "ProcessHandle.h"

ProcessHandle::ProcessHandle()
{

}

void ProcessHandle::KillProcess(string proc)
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
		if (proc == "PUBLISHER")
			value = _tcsicmp(ProcessEntry.szExeFile, _T("InitialPublisher.exe"));
		if (proc == "SUBSCRIBER")
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

bool ProcessHandle::StartProcess(DWORD & exitCode,string proc)
{
	KillProcess(proc);


	//CString cmdLine = "\"S:\\Presagis\\Suite16\\STAGE\\GUIPlugin\\subscriber\\publisher.exe\" -DCPSConfigFile \"S:\\Presagis\\Suite16\\STAGE\\GUIPlugin\\subscriber\\rtps.ini\"";
	
	CString currentDir;
	string STAGE_DIR = getenv("STAGE_DIR");
	string maker = STAGE_DIR + "\\GUIPlugin\\" + "\\EXECUTABLES\\";
	currentDir = maker.c_str();
	if (proc == "PUBLISHER")
	{
		maker = "\"" + maker + STAGE_PUBLISHER + "\"";
	}
	if (proc == "SUBSCRIBER")
	{
		maker = "\"" + maker + STAGE_SUBSCRIBER + "\"";
	}
	
	//maker = maker + " -DCPSConfigFile rtps.ini";

	CString cmdLine = maker.c_str();
	PROCESS_INFORMATION processInformation = { 0 };
	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(startupInfo);
	int nStrBuffer = cmdLine.GetLength() + 50;


	// Create the process

	BOOL result = CreateProcess(NULL, cmdLine.GetBuffer(nStrBuffer),
		NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
		NULL, currentDir, &startupInfo, &processInformation);
	cmdLine.ReleaseBuffer();


	if (!result)
	{
		return false;
	}
	else
	{
		if (proc == "PUBLISHER")
		{
			WaitForSingleObject(processInformation.hProcess, 0);
		}
		if (proc == "SUBSCRIBER")
		{
			WaitForSingleObject(processInformation.hProcess, 0);
		}

		// Successfully created the process.  Wait for it to finish.

		// Get the exit code.
		result = GetExitCodeProcess(processInformation.hProcess, &exitCode);
		
		printf("status code %d \n", exitCode);

		// Close the handles.
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);

		if (!result)
		{
			// Could not get exit code.
			if (proc == "PUBLISHER" && exitCode == 259)
				KillProcess(proc);
			return false;
		}


		// We succeeded.
		return true;
	}

}