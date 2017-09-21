#pragma once
#include "global.h"

class CPapersMgr
{
public:
	CPapersMgr();
	~CPapersMgr();

	bool	SavePapers(pPAPERSINFO pPapers);
	std::string AddPapersCompress(pPAPERSINFO pPapers);
	void	setCurrSavePath(std::string strPath);

private:
	bool WriteJsonFile(pPAPERSINFO pPapers);

	std::string _strCurrSavePath;	//gb2312
};

