#pragma once
#include "global.h"
class CMultiPageMgr
{
public:
	CMultiPageMgr();
	~CMultiPageMgr();

	std::string GetLog();
	//��һ���Ծ�ϲ�����һ���Ծ�
	void MergePaper(pST_PaperInfo pSrcPaper, pST_PaperInfo pDstPaper);
	//�޸�ҳ��, ҳ���1��ʼ
	void ModifyPicPagination(pST_PicInfo pPic, int nNewPage);
private:
	std::string _strLog;
};

