
// ScanToolStart.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "ScanToolStart.h"
#include "ScanToolStartDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SYS_BASE_NAME _T("YKLX-ScanTool")

// CScanToolStartApp

BEGIN_MESSAGE_MAP(CScanToolStartApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CScanToolStartApp ����

CScanToolStartApp::CScanToolStartApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CScanToolStartApp ����

CScanToolStartApp theApp;


// CScanToolStartApp ��ʼ��

BOOL CScanToolStartApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	if (!FirstInstance())
		return FALSE;
	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO:  Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("EasyTNT"));

	CScanToolStartDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

BOOL CScanToolStartApp::FirstInstance()
{
	TCHAR strTitle[MAX_PATH] = { 0 };
	HWND hwnd = NULL;
	HWND AfterHwnd = NULL;
	while (true)
	{
		hwnd = ::FindWindowEx(NULL, AfterHwnd, _T("#32770"), NULL);
		if (!hwnd)
			break;
		else
		{
			if (::GetWindowText(hwnd, strTitle, MAX_PATH))
			{
				if (StrStr(strTitle, SYS_BASE_NAME) != 0)
				{
					//�ҵ����ں�Ĳ���
					AfxMessageBox(_T("��⵽�Ѿ���ɨ�蹤�������У�"));
					GetLastActivePopup(hwnd);
					if (IsIconic(hwnd))
						ShowWindow(hwnd, SW_RESTORE);
					SetForegroundWindow(hwnd);
					return FALSE;
				}
			}
		}
		AfterHwnd = hwnd;
	}
	return TRUE;
}

