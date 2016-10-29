#pragma once
#include "DMTDef.h"

class CCompressThread :
	public Poco::Runnable
{
public:CCompressThread(void* pDlg);
	   ~CCompressThread();

	   virtual void run();

	   void HandleTask(pCOMPRESSTASK pTask);
	   void setDlg(void * pDlg);
private:
	void* m_pDlg;
};

