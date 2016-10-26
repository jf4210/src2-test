#pragma once  
#include "stdafx.h"

#include <windows.h>  
#include <imagehlp.h>  
#include <stdlib.h>  

#include "ScanTool.h"
#include "ScanToolDlg.h"
#include "GuideDlg.h"

#pragma comment(lib, "dbghelp.lib")  


inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)
{
	if (pModuleName == 0)
	{
		return FALSE;
	}

	WCHAR szFileName[_MAX_FNAME] = L"";
	_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);
	if (wcsicmp(szFileName, L"ntdll") == 0)
		return TRUE;


	return FALSE;
}



inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,
									  const PMINIDUMP_CALLBACK_INPUT   pInput,
									  PMINIDUMP_CALLBACK_OUTPUT        pOutput)
{
	if (pInput == 0 || pOutput == 0)
		return FALSE;

	switch (pInput->CallbackType)
	{
		case ModuleCallback:
			if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
				if (!IsDataSectionNeeded(pInput->Module.FullPath))
					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
			
		case IncludeModuleCallback:
		case IncludeThreadCallback:
		case ThreadCallback:
		case ThreadExCallback:
			return TRUE;



		default:;
	}
	
	return FALSE;
}



//创建Dump文件  

inline void CreateMiniDump(EXCEPTION_POINTERS* pep, LPCTSTR strFileName)
{
	HANDLE hFile = CreateFileW(strFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{

		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId = GetCurrentThreadId();

		mdei.ExceptionPointers = pep;

		mdei.ClientPointers = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;

		mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;

		mci.CallbackParam = 0;

		MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)0x0000ffff;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mdei, NULL, &mci);

		CloseHandle(hFile);
	}
}



LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}



BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibraryW(_T("kernel32.dll"));

	if (hKernel32 == NULL)
		return FALSE;



	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");

	if (pOrgEntry == NULL)
		return FALSE;

	
	unsigned char newJump[100];
	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far  


	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;

	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;


	newJump[0] = 0xE9;  // JMP absolute  

	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));

	SIZE_T bytesWritten;

	BOOL bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);

	return bRet;
}





LONG WINAPI UnhandledExceptionFilterEx(struct _EXCEPTION_POINTERS *pException)
{
	TCHAR szMbsFile[MAX_PATH] = { 0 };

	::GetModuleFileNameW(NULL, szMbsFile, MAX_PATH);

	TCHAR* pFind = _tcsrchr(szMbsFile, '\\');

	if (pFind)
	{
		*(pFind + 1) = 0;
		_tcscat(szMbsFile, _T("CreateMiniDump.dmp"));
		CreateMiniDump(pException, szMbsFile);
	}
	

	//++让主窗口关闭扫描仪连接
#ifdef SHOW_GUIDEDLG
	((CGuideDlg*)AfxGetMainWnd())->m_pScanDlg->ReleaseScan();
	((CGuideDlg*)AfxGetMainWnd())->m_pScanDlg->ReleaseUploadFileList();
#else
	((CScanToolDlg*)AfxGetMainWnd())->ReleaseTwain();
	((CScanToolDlg*)AfxGetMainWnd())->ReleaseUploadFileList();
#endif
	//--

	// TODO: MiniDumpWriteDump  
	FatalAppExit(-1, _T("Fatal Error"));

	return EXCEPTION_CONTINUE_SEARCH;
}



//运行异常处理  

void RunCrashHandler()
{
	SetUnhandledExceptionFilter(UnhandledExceptionFilterEx);

	PreventSetUnhandledExceptionFilter();
}