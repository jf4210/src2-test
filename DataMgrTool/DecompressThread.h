#pragma once
#include "DMTDef.h"


class CDecompressThread : public Poco::Runnable
{
public:
	CDecompressThread(void* pDlg);
	~CDecompressThread();

	virtual void run();

	void HandleTask(pDECOMPRESSTASK pTask);
	void GetFileData(std::string strFilePath, int& nExamID, int& nSubjectID);
	bool GetFileData(std::string strFilePath, pPAPERSINFO pPapers);


	void SearchExtractFile(pPAPERSINFO pPapers, std::string strPath);
private:
	void* m_pDlg;
};

