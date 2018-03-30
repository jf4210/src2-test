#pragma once
#include "global.h"

class CPapersMgr
{
public:
	CPapersMgr();
	~CPapersMgr();

	bool	SavePapers(pPAPERSINFO pPapers);
	bool AddPapersCompress(pPAPERSINFO pPapers, std::string& strResult);
	void	setCurrSavePath(std::string strPath);
	void	setExamInfo(pEXAMINFO pCurrExam, pMODEL pModel);

private:
	bool WriteJsonFile(pPAPERSINFO pPapers);

	pEXAMINFO	_pCurrExam;
	pMODEL		_pModel;
	std::string _strCurrSavePath;	//gb2312
};

