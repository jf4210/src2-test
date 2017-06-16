// ShowPicDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ShowPicDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"

// CShowPicDlg 对话框


using namespace cv;

IMPLEMENT_DYNAMIC(CShowPicDlg, CDialog)

CShowPicDlg::CShowPicDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowPicDlg::IDD, pParent)
	, m_nModelPicNums(1), m_pCurrPaper(NULL), m_nShowModel(2), m_nCurrTabSel(0)
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


// CShowPicDlg 消息处理程序
void CShowPicDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 2;	//上边的间隔
	const int nBottomGap = 2;	//下边的间隔
	const int nLeftGap = 2;		//左边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	int nGap = 5;

	if (m_nShowModel == 1)
	{
		if (GetDlgItem(IDC_TAB_PicShow)->GetSafeHwnd())
		{
			GetDlgItem(IDC_TAB_PicShow)->MoveWindow(nLeftGap, nTopGap, cx - nLeftGap - nRightGap, cy - nTopGap - nBottomGap);
		}
	}
	else
	{
		int nBtnW = 50;
		int nBtnH = 25;
		int nCurrLeft = nLeftGap;
		int nCurrTop = nTopGap;
		for (int i = 0; i < m_vecBtn.size(); i++)
		{
			nCurrLeft = nCurrLeft + i * (nBtnW + 1);
			if (m_vecBtn[i]->GetSafeHwnd())
			{
				m_vecBtn[i]->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
			}
		}
		CRect rtPic;
		rtPic.top = nCurrTop + nBtnH + 2;
		rtPic.left = nLeftGap;
		rtPic.right = cx - nRightGap;
		rtPic.bottom = cy - nBottomGap;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
	Invalidate();
}

void CShowPicDlg::InitUI()
{
	if (m_nShowModel == 1)
	{
		for (int i = 0; i < m_vecBtn.size(); i++)
		{
			CButton* pBtn = m_vecBtn[i];
			SAFE_RELEASE(pBtn);
			m_vecBtn[i] = NULL;
		}
		m_vecBtn.clear();

// 		if (_pModel_)
// 		{
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
//		}
		m_tabPicShowCtrl.DeleteAllItems();

		if (_pModel_)
			m_nModelPicNums = _pModel_->nPicNum;
		else
			m_nModelPicNums = _nPicNum4Ty_;

		USES_CONVERSION;
		CRect rtTab;
		m_tabPicShowCtrl.GetClientRect(&rtTab);
		for (int i = 0; i < m_nModelPicNums; i++)
		{
			char szTabHeadName[20] = { 0 };
			sprintf_s(szTabHeadName, "第%d页", i + 1);

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
		m_nCurrTabSel = 0;

		if (m_tabPicShowCtrl.GetSafeHwnd())
		{
			CRect rtTab;
			m_tabPicShowCtrl.GetClientRect(&rtTab);
			int nTabHead_H = 24;		//tab控件头的高度
			CRect rtPic = rtTab;
			rtPic.top = rtPic.top + nTabHead_H;
			rtPic.left += 2;
			rtPic.right -= 4;
			rtPic.bottom -= 4;
			for (int i = 0; i < m_vecPicShow.size(); i++)
				m_vecPicShow[i]->MoveWindow(&rtPic);
		}
	}
	else
	{
		m_tabPicShowCtrl.ShowWindow(SW_HIDE);
// 		if (_pModel_)
// 		{
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
//		}
		
		if (_pModel_)
			m_nModelPicNums = _pModel_->nPicNum;
		else
			m_nModelPicNums = _nPicNum4Ty_;

		for (int i = 0; i < m_vecBtn.size(); i++)
		{
			CButton* pBtn = m_vecBtn[i];
			SAFE_RELEASE(pBtn);
			m_vecBtn[i] = NULL;
		}
		m_vecBtn.clear();

		USES_CONVERSION;
		for (int i = 0; i < m_nModelPicNums; i++)
		{
			char szTabHeadName[20] = { 0 };
			sprintf_s(szTabHeadName, "第%d页", i + 1);

			CBmpButton* pNewButton = new CBmpButton();// 也可以定义为类的成员变量。
			pNewButton->SetStateBitmap(IDB_RecordDlg_Btn_Over, IDB_RecordDlg_Btn, IDB_RecordDlg_Btn_Hover, 0, IDB_RecordDlg_Btn);
			CRect rcButton(10, 10, 50, 30); // 按钮在对话框中的位置。
			pNewButton->Create(A2T(szTabHeadName), 0, rcButton, this, (i + 1) * 100);	//设置索引从101开始
			pNewButton->ShowWindow(SW_SHOW);
			m_vecBtn.push_back(pNewButton);
			
			CPicShow* pPicShow = new CPicShow(this);
			pPicShow->Create(CPicShow::IDD, this);	//pNewButton
			pPicShow->ShowWindow(SW_HIDE);
			m_vecPicShow.push_back(pPicShow);
		}
		m_vecBtn[0]->CheckBtn(TRUE);

		if (m_vecPicShow.size())
		{
			m_vecPicShow[0]->ShowWindow(SW_SHOW);
			m_pCurrentPicShow = m_vecPicShow[0];
		}
		m_nCurrTabSel = 0;
	}
	InitCtrlPosition();
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

	if (m_nShowModel == 2)
	{
		for (int i = 0; i < m_vecBtn.size(); i++)
		{
			if (i == 0)
				m_vecBtn[i]->CheckBtn(TRUE);
			else
				m_vecBtn[i]->CheckBtn(FALSE);
		}
	}

	m_pCurrentPicShow = m_vecPicShow[0];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != 0)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}

void CShowPicDlg::UpdateUI()
{
	InitUI();
}

void CShowPicDlg::setShowModel(int nModel)
{
	m_nShowModel = nModel;
	InitUI();
}

void CShowPicDlg::setRotate(int nDirection)
{
	m_vecPicShow[m_nCurrTabSel]->SetRotateDir(nDirection);
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

		RECTLIST::iterator itNormal = (*itPic)->lNormalRect.begin();													//显示识别正常的点
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
			RECTLIST::iterator itSelRoi = pPaper->pModel->vecPaperModel[i]->lSelFixRoi.begin();													//显示识别定点的选择区
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

		RECTLIST::iterator itPicFix = (*itPic)->lFix.begin();														//显示识别出来的定点
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
			RECTLIST::iterator itFixRect = pPaper->pModel->vecPaperModel[i]->lFix.begin();								//显示模板上的定点对应到此试卷上的新定点
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

			//打印OMR、SN位置
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
		if (i >= m_nModelPicNums)
			break;
		m_vecPicShow[i]->ShowPic(tmp);
	}
}

LRESULT CShowPicDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_COMMAND == message)
	{
		//++扫描中禁止切换
		if (_nScanStatus_ == 1)
		{
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "扫描中，请稍后...");
			dlg.DoModal();
			return FALSE;
		}
		//--
		USES_CONVERSION;
		WORD wID = LOWORD(wParam);
		for (int i = 0; i < m_vecBtn.size(); i++)
		{
			if (wID == (i + 1) * 100)
			{
				m_vecBtn[i]->CheckBtn(TRUE);
				m_vecPicShow[i]->ShowWindow(SW_SHOW);
				m_nCurrTabSel = i;
			}
			else
			{
				m_vecBtn[i]->CheckBtn(FALSE);
				m_vecPicShow[i]->ShowWindow(SW_HIDE);
			}
		}
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
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

	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		CButton* pBtn = m_vecBtn[i];
		SAFE_RELEASE(pBtn);
		m_vecBtn[i] = NULL;
	}
	m_vecBtn.clear();
}

void CShowPicDlg::showTmpPic(cv::Mat& matPic)
{
	if (m_pCurrentPicShow)
		m_pCurrentPicShow->ShowPic(matPic);
}
