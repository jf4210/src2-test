#pragma once
#include "global.h"
class CMultiPageMgr
{
public:
	CMultiPageMgr(pMODEL pModel);
	~CMultiPageMgr();

	std::string GetLog();
	//将一张试卷合并到另一张试卷, 原试卷的图片完全合并到目标试卷
	void MergePaper(pST_PaperInfo pSrcPaper, pST_PaperInfo pDstPaper);
	//合并图片到新试卷，试卷的准考证号不能为空，原试卷的部分图片合并到新试卷
	void MergePic(pST_PaperInfo pSrcPaper, pST_PicInfo pSrcPic, pST_PaperInfo pDstPaper);
	//修改页码, 页码从1开始，注意不能设置已经存在的页码
	//同时需要修改当前页面的选择题、选做题的页码信息
	bool ModifyPicPagination(pST_PicInfo pPic, int nNewPage);
	//修改图片的准考证号、页码后，进行图片合并
	bool ModifyPic(pST_PicInfo pPic, pPAPERSINFO pPapers, int nNewPage, std::string strZKZH);
	//检查是否可以进行修改页码，如果当前试卷已经存在此页码，则不允许修改，
	//例如：考生有1、2、3、4共4页图片，当新图片(页码3、4)添加进来，或者页码3、4修改成页码1、2时，3、4对应的选择题、选做题信息的页码信息会对应修改，
	//如果又将此页修改其他页码，则原来的页码1、2对应的omr、选做题信息也会被修改
	bool ChkModifyPagination(pST_PicInfo pPic, int nNewPage);
	//更新当前试卷的Omr、选做题信息
	void UpdateOmrInfo(pST_PaperInfo pPaper);
	//添加试卷，从扫描试卷对象中构建新的考生试卷对象
	pST_PaperInfo GetNewPaperFromScanPaper(pST_SCAN_PAPER pScanPaper, pPAPERSINFO pPapers, void* pNotifyDlg);
	//试卷袋合法性检测
	void ChkPapersValid(pPAPERSINFO pPapers);
	void ChkPaperValid(pST_PaperInfo pPaper, pMODEL pModel);
private:
	std::string _strLog;
	pMODEL _pModel;
};

