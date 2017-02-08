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
	void HandleOmrTask(pSEND_HTTP_TASK pTask);	//�ύomr��zkzh��ѡ������Ϣ
	inline bool checkPicAddr(std::string& strPicAddr, pPAPERS_DETAIL pPapers, pPIC_DETAIL pPic);	// ���ͼ���ַ�Ƿ����ظ�

	bool MovePkg(pPAPERS_DETAIL pPapers, std::string& strMovePath);		//���ύomr��zkzh��ѡ������Ϣ����ʧ��ʱ�������Ծ���ƶ���ָ��Ŀ¼�������뱸��Ŀ¼
};

