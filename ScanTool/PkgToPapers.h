/*
*
*	��һ��Pkg�Ծ���ָ���һ��Papers�����
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
	
	//++��ѧ����ϢS1��S2...����
// 	bool SortByPaper(const pST_PaperInfo& x, const pST_PaperInfo& y);
// 	bool SortbyNumASC(const std::string& x, const std::string& y);
	//--
};

