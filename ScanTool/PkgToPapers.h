/*
*
*	从一个Pkg试卷包恢复成一个Papers类对象
*
*
*
*
*/
#pragma once
#include "ZipObj.h"
#include "global.h"

class CPkgToPapers
{
public:
	CPkgToPapers();
	~CPkgToPapers();

	pPAPERSINFO Pkg2Papers(std::string strPkgPath);
private:
	void SearchExtractFile(pPAPERSINFO pPapers, std::string strPath);
	bool GetFileData(std::string strFilePath, pPAPERSINFO pPapers);
};

