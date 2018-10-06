// PicDlg.cpp : 实现文件
//

#include "..\stdafx.h"
//#include "ScanTool3.h"
#include "..\Resource.h"
#include "PicDlg.h"
#include "afxdialogex.h"
#include "..\PicShow.h"


using namespace cv;
using namespace std;
// CPicDlg 对话框

IMPLEMENT_DYNAMIC(CPicDlg, CDialog)

CPicDlg::CPicDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPicDlg::IDD, pParent)
	, m_iX(0), m_iY(0), m_fScale(1.0), m_nDirection(1), m_nRotateTimes(0)
{

}

CPicDlg::~CPicDlg()
{
}

void CPicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PicShow, m_picShow);
}


BOOL CPicDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPicDlg, CDialog)
	ON_WM_SIZE()
	ON_MESSAGE(WM_CV_MBtnWheel, &CPicDlg::MBtnWheel)
	ON_MESSAGE(WM_CV_picture, &CPicDlg::CvPaint)
	ON_MESSAGE(WM_CV_LBTNUP, &CPicDlg::RoiLbtnUp)
	ON_MESSAGE(WM_CV_LBTNDOWN, &CPicDlg::RoiLbtnDown)
	ON_MESSAGE(WM_CV_RBTNUP, &CPicDlg::RoiRbtnUp)
	ON_MESSAGE(WM_CV_HTrackerChange, &CPicDlg::HTrackerChange)
	ON_MESSAGE(WM_CV_VTrackerChange, &CPicDlg::VTrackerChange)
	ON_MESSAGE(WM_CV_SNTrackerChange, &CPicDlg::SNTrackerChange)
	ON_MESSAGE(WM_CV_ZGTTrackerChange, &CPicDlg::ZgtTrackerChange)
	ON_MESSAGE(WM_CV_MBtnDown, &CPicDlg::MBtnDown)
	ON_MESSAGE(WM_CV_MBtnUp, &CPicDlg::MBtnUp)
	ON_MESSAGE(WM_CV_ShiftDown, &CPicDlg::ShiftKeyDown)
	ON_MESSAGE(WM_CV_ShiftUp, &CPicDlg::ShiftKeyUp)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CPicDlg 消息处理程序

BOOL CPicDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_picShow.OnInit(1);

	InitCtrlPosition();

	return TRUE;
}

void CPicDlg::InitCtrlPosition()
{
	CRect rt;
	GetClientRect(&rt);
	const int nGap = 1;

	if (m_picShow.GetSafeHwnd())
	{
		m_picShow.MoveWindow(0, 0, rt.Width(), rt.Height());
	}
}

void CPicDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CPicDlg::ShowPic(cv::Mat& imgMat, cv::Point pt /*= cv::Point(0, 0)*/, float fShowPer /*= 1.0*/, int nDirection /*= -1*/)
{
	CRect rcDlg;
	this->GetClientRect(&rcDlg);

	CRect rcPic;
	m_picShow.GetClientRect(&m_client);

	int nDirect;
	if (nDirection == -1)		//默认情况下显示原始图像方向
		nDirect = m_nDirection;
	else
		nDirect = nDirection;
	RotateImg2(imgMat, nDirect);

	m_picWidth = imgMat.cols;
	m_picHeight = imgMat.rows;

// 	if (m_fScale == 1)
// 	{
// 		float fScale1 = (float)m_picWidth / (float)m_client.Width();
// 		float fScale2 = (float)m_picHeight / (float)m_client.Height();
// 		m_fScale = fScale1 > fScale2 ? fScale2 : fScale1;
// 		TRACE("***********原始比例: m_fScale = %f\n", m_fScale);
// 	}
// 
// 	m_fScale *= fShowPer;

// 	int nPageH, nPageV;
// 	if (m_picWidth < m_client.Width())
// 		nPageH = m_picWidth;
// 	else
// 		nPageH = m_client.Width() * m_fScale;
// 	if (m_picHeight < m_client.Height())
// 		nPageV = m_picHeight;
// 	else
// 		nPageV = m_client.Height() * m_fScale;

// 	int nWidthMax = m_picWidth;
// 	int nHeightMax = m_picHeight;

// 	if (pt.x < 0)
// 		pt.x = 0;
// 	if (pt.y < 0)
// 		pt.y = 0;
// 
// 	if (m_client.Width() > m_picWidth)
// 	{
// 		pt.x = (m_client.Width() - m_picWidth) / 2;
// 	}
// 	else
// 	{
// 		if (pt.x + m_client.Width() > nWidthMax)
// 			pt.x = nWidthMax - m_client.Width();
// 	}
// 	if (m_client.Height() > m_picHeight)
// 	{
// 		pt.y = (m_client.Height() - m_picHeight) / 2;
// 	}
// 	else
// 	{
// 		if (pt.y + m_client.Height() > nHeightMax)
// 			pt.y = nHeightMax - m_client.Height();
// 	}

// 	if (pt.x == 0 && pt.y == 0)
// 	{
// 		pt.x = m_iX;
// 		pt.y = m_iY;
// 	}
// 	else
// 	{
// 		m_iX = pt.x;
// 		m_iY = pt.y;
// 	}

// 	if (m_picWidth >= m_client.Width())
// 	{
// 		SCROLLINFO info;
// 		info.cbSize = sizeof(SCROLLINFO);
// 		info.fMask = SIF_ALL;
// 		info.nMin = 0;
// 		info.nMax = nWidthMax;
// 		info.nPage = nPageH;
// 		info.nPos = pt.x;
// 		m_scrollBarH.SetScrollInfo(&info);
// 	}
// 
// 	if (m_picHeight >= m_client.Height())
// 	{
// 		SCROLLINFO info;
// 		info.cbSize = sizeof(SCROLLINFO);
// 		info.fMask = SIF_ALL;
// 		info.nMin = 0;
// 		info.nMax = nHeightMax;
// 		info.nPage = nPageV;
// 		info.nPos = pt.y;
// 		m_scrollBarV.SetScrollInfo(&info);
// 	}
	m_ptBase = pt;
	m_src_img = imgMat;
	//	cv::Point pt(0, 0);
	//TRACE("CPicDlg::ShowPic--> 显示起点(%d,%d)\n", pt.x, pt.y);
	m_picShow.ShowImage_rect(m_src_img, pt, fShowPer);
	m_picShow.Invalidate();
}

void CPicDlg::RotateImg2(cv::Mat& imgMat, int nDirection /*= 1*/)
{
	switch (nDirection)
	{
		case 1:	break;
		case 2:
			{
				Mat dst;
				transpose(imgMat, dst);	//左旋90，镜像 
				flip(dst, imgMat, 1);		//右旋90，模板图像需要左旋90，原图即需要右旋90
			}
			break;
		case 3:
			{
				Mat dst;
				transpose(imgMat, dst);	//左旋90，镜像 
				flip(dst, imgMat, 0);		//左旋90，模板图像需要右旋90，原图即需要左旋90				 
			}
			break;
		case 4:
			{
				Mat dst;
				transpose(imgMat, dst);	//左旋90，镜像 
				Mat dst2;
				flip(dst, dst2, 1);
				Mat dst5;
				transpose(dst2, dst5);
				flip(dst5, imgMat, 1);	//右旋180
			}
			break;
		default: break;
	}
}


LRESULT CPicDlg::CvPaint(WPARAM wParam, LPARAM lParam)
{
	Mat* pShowMat = (Mat*)(lParam);
	m_picShow.ShowImage_roi(*pShowMat, 0);

	return TRUE;
}

LRESULT CPicDlg::RoiLbtnUp(WPARAM wParam, LPARAM lParam)
{
	Rect* pRt = (Rect*)(wParam);
	Mat*  pShowMat = (Mat*)(lParam);

	// 	m_dst_img = m_src_img(*pRt);
	// 	m_picShow.ShowImage(m_dst_img, 0);

	//	m_picShow.ShowImage(*pShowMat, 0);
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_LBTNUP, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::RoiLbtnDown(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_LBTNDOWN, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::MBtnWheel(WPARAM wParam, LPARAM lParam)
{
	float fScale = *(float*)wParam;
	m_fScale = fScale;
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_MBtnWheel, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::RoiRbtnUp(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_RBTNUP, wParam, lParam);
	return TRUE;
}

void CPicDlg::ReInit()
{
	m_picShow.OnInit(1);

	Invalidate();
}

void CPicDlg::SetShowTracker(bool bShowH, bool bShowV, bool bShowSN, bool bShowZgt)
{
	m_picShow.SetShowRectTracker(bShowH, bShowV, bShowSN, bShowZgt);
}

LRESULT CPicDlg::HTrackerChange(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_HTrackerChange, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::VTrackerChange(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_VTrackerChange, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::SNTrackerChange(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_SNTrackerChange, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::ZgtTrackerChange(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_ZGTTrackerChange, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::MBtnDown(WPARAM wParam, LPARAM lParam)
{
// 	cv::Point2f pt = *(cv::Point2f*)(wParam);
// 	m_iX = static_cast<int>(pt.x);
// 	m_iY = static_cast<int>(pt.y);
	::SendMessage(this->GetParent()->m_hWnd, WM_CV_MBtnDown, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::MBtnUp(WPARAM wParam, LPARAM lParam)
{
// 	cv::Point2f pt = *(cv::Point2f*)(wParam);
// 	m_iX = static_cast<int>(pt.x);
// 	m_iY = static_cast<int>(pt.y);
	::SendMessage(this->GetParent()->m_hWnd, WM_CV_MBtnUp, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::ShiftKeyDown(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_ShiftDown, wParam, lParam);
	return TRUE;
}

LRESULT CPicDlg::ShiftKeyUp(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->m_hWnd, WM_CV_ShiftUp, wParam, lParam);
	return TRUE;
}

void CPicDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CPicShow* pDlg = (CPicShow*)GetParent();
	pDlg->OnHScroll(nSBCode, nPos, pScrollBar);

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CPicDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CPicShow* pDlg = (CPicShow*)GetParent();
	pDlg->OnVScroll(nSBCode, nPos, pScrollBar);

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
