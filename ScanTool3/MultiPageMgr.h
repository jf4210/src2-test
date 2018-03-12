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
	
	//修改图片的准考证号，再进行图片合并
	bool ModifyPicZkzh(pST_PicInfo pPic, pPAPERSINFO pPapers, std::string strZKZH);
	//修改页码, 页码从1开始，注意不能设置已经存在的页码
	//修改页码后需要根据新页码重新识别
	bool ModifyPicPagination(pST_PicInfo pPic, int nNewPage);
	//检查试卷袋每个考生的图片，并重命名图片
	void ReNamePicInPapers(pPAPERSINFO pPapers);

private:
	void ChkPaperValid(pST_PaperInfo pPaper, pMODEL pModel);
	//检查是否可以进行修改页码，允许页码重复，不允许页码超过模板总页数
	bool ChkModifyPagination(pST_PicInfo pPic, int nNewPage);

	
	//更新当前试卷的Omr、选做题信息
	void UpdateOmrInfo(pST_PaperInfo pPaper);
	//添加试卷，从扫描试卷对象中构建一份含有基本信息的新考生试卷对象(没有图片的答案等已识别信息)
	pST_PaperInfo GetBaseNewPaperFromScanPaper(pST_SCAN_PAPER pScanPaper, pPAPERSINFO pPapers, pST_PaperInfo pCurrentPaper, std::string strSN);
	//将扫描试卷信息合并到新试卷中
	void MergeScanPaperToDstPaper(pST_SCAN_PAPER pScanPaper, pST_PaperInfo pCurrentPaper, pST_PaperInfo pDstPaper);
private:
	std::string _strLog;
	pMODEL _pModel;
};

