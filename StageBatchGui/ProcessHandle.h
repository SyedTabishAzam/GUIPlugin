#pragma once
#include <stdio.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <cstring>
#include <atlstr.h>
#include "Constants.h"
#include <minwindef.h>

using namespace std;

class ProcessHandle
{
	public:
	ProcessHandle();
	void KillProcess(string);
	bool StartProcess(DWORD &,string);

};
