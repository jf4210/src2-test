#pragma once
#include "DMTDef.h"
class CRecogResultMgr
{
public:
	CRecogResultMgr(pPAPERSINFO pPapers);
	~CRecogResultMgr();

	bool _bHasElectOmr;
	int nExamId;
	int nSubjuctId;
	std::string strPkgName;
	std::string GetSnResult();
	std::string GetOmrResult();
	std::string GetElectOmrResult();
private:
	std::string calcMd5(std::string& strInfo);
	void  GetRecogResult();
private:
	pPAPERSINFO _pPapers;
	std::string _strSnResult;
	std::string _strOmrResult;
	std::string _strElectOmrResult;
};

