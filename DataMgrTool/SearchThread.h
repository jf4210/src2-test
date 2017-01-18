#pragma once
#include "DMTDef.h"

class CSearchThread :
	public Poco::Runnable
{
public:
	CSearchThread(void* pDlg);
	~CSearchThread();

	virtual void run();
	void HandleTask(pST_SEARCH pTask);

private:
	void* m_pDlg;
};

