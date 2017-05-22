// ShowPicDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ShowPicDlg.h"
#include "afxdialogex.h"


// CShowPicDlg �Ի���


using namespace cv;

IMPLEMENT_DYNAMIC(CShowPicDlg, CDialog)

CShowPicDlg::CShowPicDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowPicDlg::IDD, pParent)
	, m_nModelPicNums(1), m_pCurrPaper(NULL)
{

}

CShowPicDlg::~CShowPicDlg()
{
}

void CShowPicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_PicShow, m_tabPicShowCtrl);
}


BOOL CShowPicDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (_pModel_) m_nModelPicNums = _pModel_->nPicNum;
	InitUI();

	return TRUE;
}

BOOL CShowPicDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CShowPicDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PicShow, &CShowPicDlg::OnTcnSelchangeTabPicshow)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CShowPicDlg ��Ϣ�������
void CShowPicDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 2;	//�ϱߵļ��
	const int nBottomGap = 2;	//�±ߵļ��
	const int nLeftGap = 2;		//��ߵĿհ׼��
	const int nRightGap = 2;	//�ұߵĿհ׼��
	int nGap = 5;

	if (GetDlgItem(IDC_TAB_PicShow)->GetSafeHwnd())
	{
		GetDlgItem(IDC_TAB_PicShow)->MoveWindow(nLeftGap, nTopGap, cx - nLeftGap - nRightGap, cy - nTopGap - nBottomGap);
	}
}

void CShowPicDlg::InitUI()
{
	if (_pModel_)
	{
		std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
		for (; itPic != m_vecPicShow.end();)
		{
			CPicShow* pModelPicShow = *itPic;
			if (pModelPicShow)
			{
				delete pModelPicShow;
				pModelPicShow = NULL;
			}
			itPic = m_vecPicShow.erase(itPic);
		}
	}
	m_tabPicShowCtrl.DeleteAllItems();

	USES_CONVERSION;
	CRect rtTab;
	m_tabPicShowCtrl.GetClientRect(&rtTab);
	for (int i = 0; i < m_nModelPicNums; i++)
	{
		char szTabHeadName[20] = { 0 };
		sprintf_s(szTabHeadName, "��%dҳ", i + 1);

		m_tabPicShowCtrl.InsertItem(i, A2T(szTabHeadName));

		CPicShow* pPicShow = new CPicShow(this);
		pPicShow->Create(CPicShow::IDD, &m_tabPicShowCtrl);
		pPicShow->ShowWindow(SW_HIDE);
		pPicShow->MoveWindow(&rtTab);
		m_vecPicShow.push_back(pPicShow);
	}
	m_tabPicShowCtrl.SetCurSel(0);
	if (m_vecPicShow.size())
	{
		m_vecPicShow[0]->ShowWindow(SW_SHOW);
		m_pCurrentPicShow = m_vecPicShow[0];
	}

	if (m_tabPicShowCtrl.GetSafeHwnd())
	{
		CRect rtTab;
		m_tabPicShowCtrl.GetClientRect(&rtTab);
		int nTabHead_H = 24;		//tab�ؼ�ͷ�ĸ߶�
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
}

void CShowPicDlg::OnTcnSelchangeTabPicshow(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	int nIndex = m_tabPicShowCtrl.GetCurSel();
	m_nCurrTabSel = nIndex;

	m_pCurrentPicShow = m_vecPicShow[nIndex];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != nIndex)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}

void CShowPicDlg::setShowPaper(pST_PaperInfo pPaper)
{
	m_pCurrPaper = pPaper;

	PaintRecognisedRect(m_pCurrPaper);

	m_pCurrentPicShow = m_vecPicShow[0];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != 0)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}

void CShowPicDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

BOOL CShowPicDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

HBRUSH CShowPicDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_TAB_PicShow)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

void CShowPicDlg::PaintRecognisedRect(pST_PaperInfo pPaper)
{
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		Mat matSrc = imread((*itPic)->strPicPath);
#ifdef PIC_RECTIFY_TEST
		Mat dst;
		Mat rotMat;
		PicRectify(matSrc, dst, rotMat);
		Mat matImg;
		if (dst.channels() == 1)
			cvtColor(dst, matImg, CV_GRAY2BGR);
		else
			matImg = dst;
		// #ifdef WarpAffine_TEST
		// 		cv::Mat	inverseMat(2, 3, CV_32FC1);
		// 		PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
		// #endif
#else
		Mat matImg = matSrc;
#endif

#ifdef WarpAffine_TEST
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		if (pPaper->pModel)
			PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
#endif

#ifdef Test_ShowOriPosition
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		GetInverseMat((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
#endif
		Mat tmp = matImg;	// matSrc.clone();
		Mat tmp2 = matImg.clone();

		if (pPaper->pModel)
		{
			RECTLIST::iterator itHTracker = pPaper->pModel->vecPaperModel[i]->lSelHTracker.begin();
			for (int j = 0; itHTracker != pPaper->pModel->vecPaperModel[i]->lSelHTracker.end(); itHTracker++, j++)
			{
				cv::Rect rt = (*itHTracker).rt;

				rectangle(tmp, rt, CV_RGB(25, 200, 20), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			RECTLIST::iterator itVTracker = pPaper->pModel->vecPaperModel[i]->lSelVTracker.begin();
			for (int j = 0; itVTracker != pPaper->pModel->vecPaperModel[i]->lSelVTracker.end(); itVTracker++, j++)
			{
				cv::Rect rt = (*itVTracker).rt;

				rectangle(tmp, rt, CV_RGB(25, 200, 20), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}

		RECTLIST::iterator itNormal = (*itPic)->lNormalRect.begin();													//��ʾʶ�������ĵ�
		for (int j = 0; itNormal != (*itPic)->lNormalRect.end(); itNormal++, j++)
		{
			cv::Rect rt = (*itNormal).rt;

			char szCP[20] = { 0 };
			if (itNormal->eCPType == SN || itNormal->eCPType == OMR)
			{
				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			else
			{
				rectangle(tmp, rt, CV_RGB(50, 255, 55), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
		if (pPaper->pModel)
		{
			RECTLIST::iterator itSelRoi = pPaper->pModel->vecPaperModel[i]->lSelFixRoi.begin();													//��ʾʶ�𶨵��ѡ����
			for (int j = 0; itSelRoi != pPaper->pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++, j++)
			{
				cv::Rect rt = (*itSelRoi).rt;

				char szCP[20] = { 0 };
				// 				sprintf_s(szCP, "FIX%d", j);
				// 				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, rt, CV_RGB(0, 0, 255), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}

		RECTLIST::iterator itPicFix = (*itPic)->lFix.begin();														//��ʾʶ������Ķ���
		for (int j = 0; itPicFix != (*itPic)->lFix.end(); itPicFix++, j++)
		{
			cv::Rect rt = (*itPicFix).rt;

			char szCP[20] = { 0 };
			sprintf_s(szCP, "R_F%d", j);
			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp, rt, CV_RGB(0, 255, 0), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
		if (pPaper->pModel)
		{
			RECTLIST::iterator itFixRect = pPaper->pModel->vecPaperModel[i]->lFix.begin();								//��ʾģ���ϵĶ����Ӧ�����Ծ��ϵ��¶���
			for (int j = 0; itFixRect != pPaper->pModel->vecPaperModel[i]->lFix.end(); itFixRect++, j++)
			{
				cv::Rect rt = (*itFixRect).rt;

				char szCP[20] = { 0 };
				sprintf_s(szCP, "M_F%d", j);
				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			// 		RECTLIST::iterator itHRect = pPaper->pModel->vecPaperModel[i].lH_Head.begin();
			// 		for (int j = 0; itHRect != pPaper->pModel->vecPaperModel[i].lH_Head.end(); itHRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itHRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "H%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(55, 0, 255), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itVRect = pPaper->pModel->vecPaperModel[i].lV_Head.begin();
			// 		for (int j = 0; itVRect != pPaper->pModel->vecPaperModel[i].lV_Head.end(); itVRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itVRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "V%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(55, 0, 255), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itABRect = pPaper->pModel->vecPaperModel[i].lABModel.begin();
			// 		for (int j = 0; itABRect != pPaper->pModel->vecPaperModel[i].lABModel.end(); itABRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itABRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "AB%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itCourseRect = pPaper->pModel->vecPaperModel[i].lCourse.begin();
			// 		for (int j = 0; itCourseRect != pPaper->pModel->vecPaperModel[i].lCourse.end(); itCourseRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itCourseRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "C%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itQKRect = pPaper->pModel->vecPaperModel[i].lQK_CP.begin();
			// 		for (int j = 0; itQKRect != pPaper->pModel->vecPaperModel[i].lQK_CP.end(); itQKRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itQKRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "QK%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itGrayRect = pPaper->pModel->vecPaperModel[i].lGray.begin();
			// 		for (int j = 0; itGrayRect != pPaper->pModel->vecPaperModel[i].lGray.end(); itGrayRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itGrayRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "G%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itWhiteRect = pPaper->pModel->vecPaperModel[i].lWhite.begin();
			// 		for (int j = 0; itWhiteRect != pPaper->pModel->vecPaperModel[i].lWhite.end(); itWhiteRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itWhiteRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "W%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}

			//��ӡOMR��SNλ��
#ifdef PaintOmrSnRect
			// 			SNLIST::iterator itSN = pPaper->pModel->vecPaperModel[i]->lSNInfo.begin();
			// 			for (; itSN != pPaper->pModel->vecPaperModel[i]->lSNInfo.end(); itSN++)
			// 			{
			// 				pSN_ITEM pSnItem = *itSN;
			// 				RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
			// 				for (; itSnItem != pSnItem->lSN.end(); itSnItem++)
			// 				{
			// 					cv::Rect rt = (*itSnItem).rt;
			// 	#ifdef Test_ShowOriPosition
			// 
			// 					GetPosition2(inverseMat, rt, rt);
			// 	#else
			// 					GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
			// 	#endif
			// 					rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 					rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 				}
			// 			}
			// 
			// 			OMRLIST::iterator itOmr = pPaper->pModel->vecPaperModel[i]->lOMR2.begin();
			// 			for (; itOmr != pPaper->pModel->vecPaperModel[i]->lOMR2.end(); itOmr++)
			// 			{
			// 				pOMR_QUESTION pOmrQuestion = &(*itOmr);
			// 				RECTLIST::iterator itOmrItem = pOmrQuestion->lSelAnswer.begin();
			// 				for (; itOmrItem != pOmrQuestion->lSelAnswer.end(); itOmrItem++)
			// 				{
			// 					cv::Rect rt = (*itOmrItem).rt;
			// 	#ifdef Test_ShowOriPosition
			// 
			// 					GetPosition2(inverseMat, rt, rt);
			// 	#else
			// 					GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
			// 	#endif
			// 					rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 					rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 				}
			// 			}
#endif
		}

		addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
		m_vecPicShow[i]->ShowPic(tmp);
	}
}

void CShowPicDlg::OnDestroy()
{
	CDialog::OnDestroy();

	std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
	for (; itPic != m_vecPicShow.end();)
	{
		CPicShow* pModelPicShow = *itPic;
		SAFE_RELEASE(pModelPicShow);
		itPic = m_vecPicShow.erase(itPic);
	}
}
