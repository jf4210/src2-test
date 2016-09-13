#pragma once
#include"DCSDef.h"
#include "modelInfo.h"

class CScanResquestHandler : public Poco::Runnable
{
public:
	CScanResquestHandler();
	~CScanResquestHandler();

	virtual void run();
	void	HandleTask(pSCAN_REQ_TASK pTask);

	bool ParseResult(std::string& strInput, pSCAN_REQ_TASK pTask);

	pMODEL CreateModel(Poco::JSON::Object::Ptr object);
	bool GetPdf(Poco::JSON::Object::Ptr object, std::string strSavePath);
};

