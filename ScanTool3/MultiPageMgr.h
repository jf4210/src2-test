#pragma once
#include "global.h"
class CMultiPageMgr
{
public:
	CMultiPageMgr(pMODEL pModel);
	~CMultiPageMgr();

	std::string GetLog();
	//��һ���Ծ�ϲ�����һ���Ծ�, ԭ�Ծ��ͼƬ��ȫ�ϲ���Ŀ���Ծ�
	void MergePaper(pST_PaperInfo pSrcPaper, pST_PaperInfo pDstPaper);
	//�ϲ�ͼƬ�����Ծ��Ծ��׼��֤�Ų���Ϊ�գ�ԭ�Ծ�Ĳ���ͼƬ�ϲ������Ծ�
	void MergePic(pST_PaperInfo pSrcPaper, pST_PicInfo pSrcPic, pST_PaperInfo pDstPaper);
	
	//�޸�ͼƬ��׼��֤�ţ��ٽ���ͼƬ�ϲ�
	bool ModifyPicZkzh(pST_PicInfo pPic, pPAPERSINFO pPapers, std::string strZKZH);
	//�޸�ҳ��, ҳ���1��ʼ��ע�ⲻ�������Ѿ����ڵ�ҳ��
	//�޸�ҳ�����Ҫ������ҳ������ʶ��
	bool ModifyPicPagination(pST_PicInfo pPic, int nNewPage);
	//����Ծ��ÿ��������ͼƬ����������ͼƬ
	void ReNamePicInPapers(pPAPERSINFO pPapers);

private:
	void ChkPaperValid(pST_PaperInfo pPaper, pMODEL pModel);
	//����Ƿ���Խ����޸�ҳ�룬����ҳ���ظ���������ҳ�볬��ģ����ҳ��
	bool ChkModifyPagination(pST_PicInfo pPic, int nNewPage);

	
	//���µ�ǰ�Ծ��Omr��ѡ������Ϣ
	void UpdateOmrInfo(pST_PaperInfo pPaper);
	//����Ծ���ɨ���Ծ�����й���һ�ݺ��л�����Ϣ���¿����Ծ����(û��ͼƬ�Ĵ𰸵���ʶ����Ϣ)
	pST_PaperInfo GetBaseNewPaperFromScanPaper(pST_SCAN_PAPER pScanPaper, pPAPERSINFO pPapers, pST_PaperInfo pCurrentPaper, std::string strSN);
	//��ɨ���Ծ���Ϣ�ϲ������Ծ���
	void MergeScanPaperToDstPaper(pST_SCAN_PAPER pScanPaper, pST_PaperInfo pCurrentPaper, pST_PaperInfo pDstPaper);
private:
	std::string _strLog;
	pMODEL _pModel;
};

