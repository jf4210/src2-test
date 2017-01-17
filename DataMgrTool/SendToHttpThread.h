#pragma once
#include "DMTDef.h"

class CSendToHttpThread : public Poco::Runnable
{
public:
	CSendToHttpThread(void* pDlg);
	~CSendToHttpThread();

	virtual void run();

	bool ParseResult(std::string& strInput, pSEND_HTTP_TASK pTask);
	bool doRequest(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::URI& uri, pSEND_HTTP_TASK pTask);
	
	std::string calcMd5(std::string& strInfo);
	void checkTaskStatus(pSEND_HTTP_TASK pTask, int nState = 0);		//�������������������ύ׼��֤�š�OMR��ѡ���⣬��������˾Ϳ���ɾ���򲿷��Ծ����
	void HandleOmrTask(pSEND_HTTP_TASK pTask);	//�ύomr��zkzh��ѡ������Ϣ
//	inline bool checkPicAddr(std::string& strPicAddr, pPAPERS_DETAIL pPapers, pPIC_DETAIL pPic);	// ���ͼ���ַ�Ƿ����ظ�

private:
	void* m_pDlg;
};

