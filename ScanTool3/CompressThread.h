#pragma once
#include "global.h"
#include "ZipObj.h"

class CCompressThread : public Poco::Runnable
{
public:
	CCompressThread(void* pDlg);
	~CCompressThread();

	virtual void run();

	void HandleTask(pCOMPRESSTASK pTask);
	void GetFileData(std::string strFilePath, int& nExamID, int& nSubjectID);

	void setDlg(void * pDlg);
private:
	void* m_pDlg;
};

