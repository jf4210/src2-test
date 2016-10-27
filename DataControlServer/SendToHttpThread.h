#pragma once
#include "DCSDef.h"

class CSendToHttpThread : public Poco::Runnable
{
public:
	CSendToHttpThread();
	~CSendToHttpThread();

	virtual void run();

	bool ParseResult(std::string& strInput, pSEND_HTTP_TASK pTask);
	bool doRequest(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::URI& uri, pSEND_HTTP_TASK pTask);
	
	bool GenerateResult(pPAPERS_DETAIL pPapers, pSEND_HTTP_TASK pTask);
	std::string calcMd5(std::string& strInfo);
	void checkTaskStatus(pPAPERS_DETAIL pPapers);		//�������������������ύ׼��֤�š�OMR��ѡ���⣬��������˾Ϳ���ɾ���򲿷��Ծ����
	inline bool checkPicAddr(std::string& strPicAddr, pPAPERS_DETAIL pPapers, pPIC_DETAIL pPic);	// ���ͼ���ַ�Ƿ����ظ�
};

