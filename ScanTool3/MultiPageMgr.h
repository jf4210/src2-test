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
	//�޸�ҳ��, ҳ���1��ʼ��ע�ⲻ�������Ѿ����ڵ�ҳ��
	//ͬʱ��Ҫ�޸ĵ�ǰҳ���ѡ���⡢ѡ�����ҳ����Ϣ
	bool ModifyPicPagination(pST_PicInfo pPic, int nNewPage);
	//�޸�ͼƬ��׼��֤�š�ҳ��󣬽���ͼƬ�ϲ�
	bool ModifyPic(pST_PicInfo pPic, pPAPERSINFO pPapers, int nNewPage, std::string strZKZH);
	//����Ƿ���Խ����޸�ҳ�룬�����ǰ�Ծ��Ѿ����ڴ�ҳ�룬�������޸ģ�
	//���磺������1��2��3��4��4ҳͼƬ������ͼƬ(ҳ��3��4)��ӽ���������ҳ��3��4�޸ĳ�ҳ��1��2ʱ��3��4��Ӧ��ѡ���⡢ѡ������Ϣ��ҳ����Ϣ���Ӧ�޸ģ�
	//����ֽ���ҳ�޸�����ҳ�룬��ԭ����ҳ��1��2��Ӧ��omr��ѡ������ϢҲ�ᱻ�޸�
	bool ChkModifyPagination(pST_PicInfo pPic, int nNewPage);
	//���µ�ǰ�Ծ��Omr��ѡ������Ϣ
	void UpdateOmrInfo(pST_PaperInfo pPaper);
	//����Ծ���ɨ���Ծ�����й����µĿ����Ծ����
	pST_PaperInfo GetNewPaperFromScanPaper(pST_SCAN_PAPER pScanPaper, pPAPERSINFO pPapers, void* pNotifyDlg);
	//�Ծ���Ϸ��Լ��
	void ChkPapersValid(pPAPERSINFO pPapers);
	void ChkPaperValid(pST_PaperInfo pPaper, pMODEL pModel);
private:
	std::string _strLog;
	pMODEL _pModel;
};

