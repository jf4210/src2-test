
// EasyTntGuardProcess.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CEasyTntGuardProcessApp: 
// �йش����ʵ�֣������ EasyTntGuardProcess.cpp
//

class CEasyTntGuardProcessApp : public CWinApp
{
public:
	CEasyTntGuardProcessApp();

// ��д
public:
	virtual BOOL InitInstance();
	
	BOOL FirstInstance();
// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CEasyTntGuardProcessApp theApp;