#pragma once
#include "DMTDef.h"


class CDecompressThread : public Poco::Runnable
{
public:
	CDecompressThread(void* pDlg);
	~CDecompressThread();

	virtual void run();

	void HandleTask(pDECOMPRESSTASK pTask);
	bool GetFileData(std::string strFilePath, pPAPERSINFO pPapers);
	bool GetFileData2(std::string strFilePath, pPAPERSINFO pPapers);


	void SearchExtractFile(pPAPERSINFO pPapers, std::string strPath);	//搜索的是已经解压的pkg试卷包
	bool SearchDirPaper(pPAPERSINFO pPapers, std::string strPath);		//搜索普通文件夹gb2312，将内部的jpg文件放入试卷袋列表中
	void SearchDirPapers(std::string strPath);

//	bool calcStatistics(pPAPERSINFO pPapers);
	Poco::Event eExit;
private:
	void* m_pDlg;
};

