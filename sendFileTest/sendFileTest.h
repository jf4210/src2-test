
// sendFileTest.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CsendFileTestApp: 
// �йش����ʵ�֣������ sendFileTest.cpp
//

class CsendFileTestApp : public CWinApp
{
public:
	CsendFileTestApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CsendFileTestApp theApp;