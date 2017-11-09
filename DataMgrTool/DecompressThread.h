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


	void SearchExtractFile(pPAPERSINFO pPapers, std::string strPath);	//���������Ѿ���ѹ��pkg�Ծ��
	bool SearchDirPaper(pPAPERSINFO pPapers, std::string strPath);		//������ͨ�ļ���gb2312�����ڲ���jpg�ļ������Ծ���б���
	void SearchDirPapers(std::string strPath);

//	bool calcStatistics(pPAPERSINFO pPapers);
	Poco::Event eExit;
private:
	void* m_pDlg;
};

