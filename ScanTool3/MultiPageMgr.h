#pragma once
#include "global.h"
class CMultiPageMgr
{
public:
	CMultiPageMgr();
	~CMultiPageMgr();

	std::string GetLog();
	//将一张试卷合并到另一张试卷
	void MergePaper(pST_PaperInfo pSrcPaper, pST_PaperInfo pDstPaper);
	//修改页码, 页码从1开始
	void ModifyPicPagination(pST_PicInfo pPic, int nNewPage);
private:
	std::string _strLog;
};

