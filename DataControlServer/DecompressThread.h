#pragma once
#include "DCSDef.h"


class CDecompressThread : public Poco::Runnable
{
public:
	CDecompressThread();
	~CDecompressThread();

	virtual void run();

	void HandleTask(pDECOMPRESSTASK pTask);
	bool GetFileData(std::string strFilePath, pPAPERS_DETAIL pPapers, pDECOMPRESSTASK pTask);

	void SearchExtractFile(pPAPERS_DETAIL pPapers, std::string strPath);
	void UploadModelPic(pPAPERS_DETAIL pPapers, std::string strPath);
};

