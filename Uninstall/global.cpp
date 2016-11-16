#include "stdafx.h"
#include "global.h"
#include <Tlhelp32.h>




BOOL CheckProcessExist(CString &str, int& nProcessID)
{
	BOOL bResult;
	CString strTemp, strProcessName;
	HANDLE hSnapshot;               //内存进程的“快照”句柄      
	PROCESSENTRY32 ProcessEntry;    //描述进程的结构   
	//输入要结束的进程名称    
	strProcessName = str/*m_strProcessName*/;
	strProcessName.MakeLower();
	//返回内存所有进程的快照。参数为TH32CS_SNAPPROCESS取有的进程,忽略参数2；    
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	//获取要的进程名称对应的所有进程ID    
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	bResult = Process32First(hSnapshot, &ProcessEntry);//获取第一个进程 
	int processcount = 0;
	while (bResult)
	{
		//判断是否为要结束的进程    
		strTemp.Format(_T("%s"), ProcessEntry.szExeFile);
		strTemp.MakeLower();
		if (strTemp == strProcessName)
		{
			nProcessID = ProcessEntry.th32ProcessID;
			processcount++;
			return TRUE;
		}
		//获取下一个进程    
		bResult = Process32Next(hSnapshot, &ProcessEntry);
	}
	return FALSE;
}

bool EnableDebugPrivilege()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return   FALSE;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		CloseHandle(hToken);
		return false;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		CloseHandle(hToken);
		return false;
	}
	return true;
}

bool UpPrivilege()
{
	HANDLE hToken;                             //   handle   to   process   token   
	TOKEN_PRIVILEGES tkp;               //   pointer   to   token   structure   
	bool result = OpenProcessToken(GetCurrentProcess(),
								   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
								   &hToken);
	if (!result)         //打开进程错误   
		return result;
	result = LookupPrivilegeValue(NULL,
								  SE_DEBUG_NAME,
								  &tkp.Privileges[0].Luid);
	if (!result)         //查看进程权限错误   
		return   result;
	tkp.PrivilegeCount = 1;     //   one   privilege   to   set   
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	result = AdjustTokenPrivileges(hToken,
								   FALSE,
								   &tkp,
								   sizeof(TOKEN_PRIVILEGES),
								   (PTOKEN_PRIVILEGES)NULL,
								   (PDWORD)NULL);
	return   result;
}

BOOL KillProcess(CString& str)
{
	HANDLE hProcess = 0;
	DWORD dwExitCode = 0;
	int nProcessID = 0;
	if (CheckProcessExist(str, nProcessID))
	{
		hProcess = ::OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_CREATE_THREAD, FALSE, nProcessID);
		
		LPVOID Param = VirtualAllocEx(hProcess, NULL, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		WriteProcessMemory(hProcess, Param, (LPVOID)&dwExitCode, sizeof(DWORD), NULL);

		HANDLE hThread = CreateRemoteThread(hProcess,
											NULL,
											NULL,
											(LPTHREAD_START_ROUTINE)ExitProcess,
											Param,
											NULL,
											NULL);
		return TRUE;
	}

	return FALSE;
}








