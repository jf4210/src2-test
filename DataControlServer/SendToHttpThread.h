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
	void checkTaskStatus(pPAPERS_DETAIL pPapers);		//检查任务完成情况，针对提交准考证号、OMR、选做题，处理完成了就可以删除或部分试卷袋了
	void HandleOmrTask(pSEND_HTTP_TASK pTask);	//提交omr、zkzh、选做题信息
	inline bool checkPicAddr(std::string& strPicAddr, pPAPERS_DETAIL pPapers, pPIC_DETAIL pPic);	// 检测图像地址是否有重复

	bool MovePkg(pPAPERS_DETAIL pPapers, std::string& strMovePath);		//在提交omr、zkzh、选做题信息出现失败时，将此试卷包移动到指定目录，不进入备份目录
};

