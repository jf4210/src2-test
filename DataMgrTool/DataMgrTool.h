
// DataMgrTool.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CDataMgrToolApp: 
// �йش����ʵ�֣������ DataMgrTool.cpp
//

class CDataMgrToolApp : public CWinApp
{
public:
	CDataMgrToolApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CDataMgrToolApp theApp;