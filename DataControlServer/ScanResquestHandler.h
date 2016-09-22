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

	int modelHandle(pSCAN_REQ_TASK pTask, Poco::JSON::Object::Ptr object);

	pMODEL CreateModel(Poco::JSON::Object::Ptr object, int nExamID, int nSubjectID, std::vector<std::vector<int>>& vecSheets);
	bool GetPdf(Poco::JSON::Object::Ptr object, std::string& strSavePath);
	bool Pdf2Jpg(std::string& strModelPath, std::vector<std::vector<int>>& vecSheets);
	bool InitModelRecog(pMODEL pModel);
	bool SaveModel(pMODEL pModel, std::string& strModelPath);
	int ZipModel(pMODEL pModel, std::string& strModelPath);
};

