
// ScanToolStart.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CScanToolStartApp: 
// �йش����ʵ�֣������ ScanToolStart.cpp
//

class CScanToolStartApp : public CWinApp
{
public:
	CScanToolStartApp();

// ��д
public:
	virtual BOOL InitInstance();

	BOOL FirstInstance();
// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CScanToolStartApp theApp;