// PicShow.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "PicShow.h"
#include "afxdialogex.h"


const int nScrollH_H = 12;
const int nScrollV_W = 12;

using namespace cv;
using namespace std;
// CPicShow 对话框

IMPLEMENT_DYNAMIC(CPicShow, CDialog)

CPicShow::CPicShow(CWnd* pParent /*=NULL*/)
	: CDialog(CPicShow::IDD, pParent)
	, m_bShowScrolH(TRUE), m_bShowScrolV(TRUE), m_iX(0), m_iY(0), m_fScale(1.0), m_nDirection(1), m_nRotateTimes(0)
{

}

CPicShow::~CPicShow()
{
}

void CPicShow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PicShow, m_picShow);
	DDX_Control(pDX, IDC_SCROLLBAR_H, m_scrollBarH);
	DDX_Control(pDX, IDC_SCROLLBAR_V, m_scrollBarV);
}


BEGIN_MESSAGE_MAP(CPicShow, CDialog)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_MESSAGE(WM_CV_MBtnWheel, &CPicShow::MBtnWheel)
	ON_MESSAGE(WM_CV_picture, &CPicShow::CvPaint)
	ON_MESSAGE(WM_CV_LBTNUP, &CPicShow::RoiLbtnUp)
	ON_MESSAGE(WM_CV_LBTNDOWN, &CPicShow::RoiLbtnDown)
	ON_MESSAGE(WM_CV_RBTNUP, &CPicShow::RoiRbtnUp)
	ON_MESSAGE(WM_CV_HTrackerChange, &CPicShow::HTrackerChange)
	ON_MESSAGE(WM_CV_VTrackerChange, &CPicShow::VTrackerChange)
	ON_MESSAGE(WM_CV_SNTrackerChange, &CPicShow::SNTrackerChange)
	ON_MESSAGE(WM_CV_ZGTTrackerChange, &CPicShow::ZgtTrackerChange)
	ON_MESSAGE(WM_CV_MBtnDown, &CPicShow::MBtnDown)
	ON_MESSAGE(WM_CV_MBtnUp, &CPicShow::MBtnUp)
	ON_MESSAGE(WM_CV_ShiftDown, &CPicShow::ShiftKeyDown)
	ON_MESSAGE(WM_CV_ShiftUp, &CPicShow::ShiftKeyUp)
END_MESSAGE_MAP()

BOOL CPicShow::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_picShow.OnInit(1);

	m_scrollBarV.ShowScrollBar(m_bShowScrolV);
	m_scrollBarH.ShowScrollBar(m_bShowScrolH);

	InitCtrlPosition();

	return TRUE;
}

// CPicShow 消息处理程序
void CPicShow::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);


#if 1
	InitCtrlPosition();
#else
	if (m_picShow.GetSafeHwnd())
	{
		if (!m_bShowScrolH && !m_bShowScrolV)
			m_picShow.MoveWindow(0, 0, cx, cy);
		else if (!m_bShowScrolH)
			m_picShow.MoveWindow(0, 0, cx - nScrollV_W - 1, cy);
		else if (!m_bShowScrolV)
			m_picShow.MoveWindow(0, 0, cx, cy - nScrollH_H - 1);
		else
			m_picShow.MoveWindow(0, 0, cx - nScrollV_W - 1, cy - nScrollH_H - 1);
	}

	if (m_bShowScrolH && m_scrollBarH.GetSafeHwnd())
		m_scrollBarH.MoveWindow(0, cy - nScrollH_H, cx, cy);
	if (m_bShowScrolV && m_scrollBarV.GetSafeHwnd())
		m_scrollBarV.MoveWindow(cx - nScrollV_W, 0, cx, cy);
#endif
}

void CPicShow::InitCtrlPosition()
{
	CRect rt;
	GetClientRect(&rt);
	const int nGap = 1;

	if (m_picShow.GetSafeHwnd())
	{
		if (!m_bShowScrolH && !m_bShowScrolV)
			m_picShow.MoveWindow(0, 0, rt.Width(), rt.Height());
		else if (!m_bShowScrolH)
			m_picShow.MoveWindow(0, 0, rt.Width() - nScrollV_W - nGap, rt.Height());
		else if (!m_bShowScrolV)
			m_picShow.MoveWindow(0, 0, rt.Width(), rt.Height() - nScrollH_H - nGap);
		else
			m_picShow.MoveWindow(0, 0, rt.Width() - nScrollV_W - nGap, rt.Height() - nScrollH_H - nGap);
	}

	if (m_bShowScrolH && m_scrollBarH.GetSafeHwnd())
		m_scrollBarH.MoveWindow(0, rt.Height() - nScrollH_H, rt.Width() - 12, nScrollH_H);	//-12为防止两个滚动条重叠
	if (m_bShowScrolV && m_scrollBarV.GetSafeHwnd())
		m_scrollBarV.MoveWindow(rt.Width() - nScrollV_W, 0, nScrollV_W, rt.Height() - 12);
}

BOOL CPicShow::PreTranslateMessage(MSG* pMsg)
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

void CPicShow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int pos;
	pos = m_scrollBarV.GetScrollPos();

	switch (nSBCode)
	{
	case SB_LINEUP:
		pos -= 1;
		break;
	case SB_LINEDOWN:
		pos += 1;
		break;
	case SB_PAGEUP:
		pos -= 10;
		break;
	case SB_PAGEDOWN:
		pos += 10;
		break;
	case SB_TOP:
		pos = 0;
		break;
	case SB_BOTTOM:
		pos = m_picHeight;
		break;
	case SB_THUMBPOSITION:
		pos = nPos;
		break;
	case SB_THUMBTRACK:
		pos = nPos;
		break;
	default:
		return;
	}
	if (pos<0)
		pos = 0;
	else if (pos > m_picHeight)
		pos = m_picHeight;
// 	else if (pos + m_client.Height()>m_picHeight - 1)
// 		pos = m_picHeight - 1 - m_client.Height();
	m_scrollBarV.SetScrollPos(pos, TRUE);
	m_iY = pos;


//	cv::Point pt(m_iX * m_fScale, m_iY * m_fScale);
	cv::Point pt(m_iX * 1, m_iY * 1);
//	TRACE("OnVScroll, pt(%d, %d), 缩放后:(%d,%d)\n", m_iX, m_iY, m_iX * m_fScale, m_iY * m_fScale);

//	m_picShow.ShowImage_rect(m_src_img, pt);
	m_picShow.ShowImage_Rect_roi(m_src_img, pt);
}

void CPicShow::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int pos;
	pos = m_scrollBarH.GetScrollPos();

	switch (nSBCode)
	{
	case SB_LINEUP:
		pos -= 1;
		break;
	case SB_LINEDOWN:
		pos += 1;
		break;
	case SB_PAGEUP:
		pos -= 10;
		break;
	case SB_PAGEDOWN:
		pos += 10;
		break;
	case SB_TOP:
		pos = 0;
		break;
	case SB_BOTTOM:
		pos = m_picWidth;
		break;
	case SB_THUMBPOSITION:
		pos = nPos;
		break;
	case SB_THUMBTRACK:
		pos = nPos;
		break;
	default:
		TRACE("nSBCode = %d, nPos = %d\n", nSBCode, nPos);
		return;
	}
	if (pos<0)
		pos = 0;
	else if (pos > m_picWidth)
		pos = m_picWidth;
// 	else if (pos + m_client.Width()>m_picWidth - 1)
// 		pos = m_picWidth - 1 - m_client.Width();
	m_scrollBarH.SetScrollPos(pos, TRUE);
	m_iX = pos;

//	cv::Point pt(m_iX * m_fScale, m_iY * m_fScale);
	cv::Point pt(m_iX * 1, m_iY * 1);
//	TRACE("OnHScroll, pt(%d, %d), 缩放后:(%d,%d), m_fScale = %f.\n", m_iX, m_iY, m_iX * m_fScale, m_iY * m_fScale, m_fScale);

//	m_picShow.ShowImage_rect(m_src_img, pt);
	m_picShow.ShowImage_Rect_roi(m_src_img, pt);
}

void CPicShow::ShowPic(cv::Mat& imgMat, cv::Point pt /*= cv::Point(0,0)*/, float fShowPer /*= 1.0*/, int nDirection /*= -1*/)
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

	m_picWidth  = imgMat.cols;
	m_picHeight = imgMat.rows;
#if 1
	if (m_fScale == 1)
	{
		float fScale1 = (float)m_picWidth / (float)m_client.Width();
		float fScale2 = (float)m_picHeight / (float)m_client.Height();
		m_fScale = fScale1 > fScale2 ? fScale2 : fScale1;
		TRACE("***********原始比例: m_fScale = %f\n", m_fScale);
	}	

	m_fScale *= fShowPer;
	int nPageH, nPageV;
	if (m_picWidth < m_client.Width())
		nPageH = m_picWidth;
	else
		nPageH = m_client.Width() * m_fScale;
	if (m_picHeight < m_client.Height())
		nPageV = m_picHeight;
	else
		nPageV = m_client.Height() * m_fScale;

	int nWidthMax = m_picWidth;
	int nHeightMax = m_picHeight;

	if (pt.x < 0)
		pt.x = 0;
	if (pt.y < 0)
		pt.y = 0;

	if (m_client.Width() > m_picWidth)
	{
		pt.x = (m_client.Width() - m_picWidth) / 2;
	}
	else
	{
		if (pt.x + m_client.Width() > nWidthMax)
			pt.x = nWidthMax - m_client.Width();
	}
	if (m_client.Height() > m_picHeight)
	{
		pt.y = (m_client.Height() - m_picHeight) / 2;
	}
	else
	{
		if (pt.y + m_client.Height() > nHeightMax)
			pt.y = nHeightMax - m_client.Height();
	}	

	if (pt.x == 0 && pt.y == 0)
	{
		pt.x = m_iX;
		pt.y = m_iY;
	}
	else
	{
		m_iX = pt.x;
		m_iY = pt.y;
	}

	if (m_picWidth >= m_client.Width())
	{
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_ALL;
		info.nMin = 0;
		info.nMax = nWidthMax;
		info.nPage = nPageH;
		info.nPos = pt.x;
		m_scrollBarH.SetScrollInfo(&info);
	}

	if (m_picHeight >= m_client.Height())
	{
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_ALL;
		info.nMin = 0;
		info.nMax = nHeightMax;
		info.nPage = nPageV;
		info.nPos = pt.y;
		m_scrollBarV.SetScrollInfo(&info);
	}
#else
	int nPageH, nPageV;
	if (m_picWidth < m_client.Width())
		nPageH = m_picWidth;
	else
		nPageH = m_client.Width();
	if (m_picHeight < m_client.Height())
		nPageV = m_picHeight;
	else
		nPageV = m_client.Height();

	int nWidthMax = m_picWidth;
	int nHeightMax = m_picHeight;
	
	if (pt.x < 0)
		pt.x = 0;
	if (pt.y < 0)
		pt.y = 0;
	if (pt.x + m_client.Width() > nWidthMax)
		pt.x = nWidthMax - m_client.Width();
	if (pt.y + m_client.Height() > nHeightMax)
		pt.y = nHeightMax - m_client.Height();

	if (pt.x == 0 && pt.y == 0)
	{
		pt.x = m_iX;
		pt.y = m_iY;
	}
	else
	{
		m_iX = pt.x;
		m_iY = pt.y;
	}

	if (m_picWidth >= m_client.Width())
	{
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_ALL;
		info.nMin = 0;
		info.nMax = nWidthMax;
		info.nPage = nPageH;
		info.nPos = pt.x;
		m_scrollBarH.SetScrollInfo(&info);
	}

	if (m_picHeight >= m_client.Height())
	{
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_ALL;
		info.nMin = 0;
		info.nMax = nHeightMax;
		info.nPage = nPageV;
		info.nPos = pt.y;
		m_scrollBarV.SetScrollInfo(&info);
	}
#endif
	m_ptBase = pt;
	m_src_img = imgMat;
//	cv::Point pt(0, 0);
	m_picShow.ShowImage_rect(m_src_img, pt, m_fScale);
	m_picShow.Invalidate();
}

void CPicShow::RotateImg2(cv::Mat& imgMat, int nDirection /*= 1*/)
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

void CPicShow::RotateImg(int nDirection)
{
	switch (nDirection)
	{
		case 1:	break;
		case 2:
		{
				  Mat dst;
				  transpose(m_src_img, dst);	//左旋90，镜像 
				  flip(dst, m_src_img, 1);		//右旋90，模板图像需要左旋90，原图即需要右旋90
		}
			break;
		case 3:
		{
				  Mat dst;
				  transpose(m_src_img, dst);	//左旋90，镜像 
				  flip(dst, m_src_img, 0);		//左旋90，模板图像需要右旋90，原图即需要左旋90				 
		}
			break;
		case 4:
		{
				  Mat dst;
				  transpose(m_src_img, dst);	//左旋90，镜像 
				  Mat dst2;
				  flip(dst, dst2, 1);
				  Mat dst5;
				  transpose(dst2, dst5);
				  flip(dst5, m_src_img, 1);	//右旋180
		}
			break;
		default: break;
	}
//	m_picShow.ShowImage_rect(m_src_img, m_ptBase, m_fScale);
	ShowPic(m_src_img, m_ptBase);
}

void CPicShow::SetRotateDir(int nDirection)
{
	SetRotateDir(m_src_img, nDirection);

// 	if (nDirection == 3)
// 		++m_nRotateTimes;
// 	if (nDirection == 2)
// 		--m_nRotateTimes;
// 	m_nRotateTimes = m_nRotateTimes < 0 ? m_nRotateTimes + 4 : m_nRotateTimes;
// 	switch (m_nRotateTimes % 4)
// 	{
// 		case 0:
// 			m_nDirection = 1;
// 			break;
// 		case 1:
// 			m_nDirection = 3;
// 			break;
// 		case 2:
// 			m_nDirection = 4;
// 			break;
// 		case 3:
// 			m_nDirection = 2;
// 			break;
// 	}
// 	ShowPic(m_src_img, m_ptBase, 1.0, nDirection);
}

void CPicShow::SetRotateDir(cv::Mat& imgMat, int nDirection)
{
	if (nDirection == 3)
		++m_nRotateTimes;
	if (nDirection == 2)
		--m_nRotateTimes;
	m_nRotateTimes = m_nRotateTimes < 0 ? m_nRotateTimes + 4 : m_nRotateTimes;
	switch (m_nRotateTimes % 4)
	{
		case 0:
			m_nDirection = 1;
			break;
		case 1:
			m_nDirection = 3;
			break;
		case 2:
			m_nDirection = 4;
			break;
		case 3:
			m_nDirection = 2;
			break;
	}
	ShowPic(imgMat, m_ptBase, 1.0, nDirection);
}

LRESULT CPicShow::CvPaint(WPARAM wParam, LPARAM lParam)
{
	Mat* pShowMat = (Mat*)(lParam);
	m_picShow.ShowImage_roi(*pShowMat, 0);

	return TRUE;
}

LRESULT CPicShow::RoiLbtnUp(WPARAM wParam, LPARAM lParam)
{
	Rect* pRt = (Rect*)(wParam);
	Mat*  pShowMat = (Mat*)(lParam);

// 	m_dst_img = m_src_img(*pRt);
// 	m_picShow.ShowImage(m_dst_img, 0);

//	m_picShow.ShowImage(*pShowMat, 0);
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_LBTNUP, wParam, lParam);
	return TRUE;
}

LRESULT CPicShow::RoiLbtnDown(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_LBTNDOWN, wParam, lParam);
	return TRUE;
}

LRESULT CPicShow::MBtnWheel(WPARAM wParam, LPARAM lParam)
{
	float fScale = *(float*)wParam;
	cv::Rect rcRoi = *(cv::Rect*)lParam;
	m_fScale = fScale;

	int nMaxH, nMaxV;
	nMaxH = m_picWidth;
	nMaxV = m_picHeight;
// 	nMaxH = m_picWidth / fScale;
// 	nMaxV = m_picHeight / fScale;
// 	if (nMaxH < m_client.Width())
// 		nMaxH = m_client.Width();
// 	if (nMaxV < m_client.Height())
// 		nMaxV = m_client.Height();

	int nX = m_scrollBarH.GetScrollPos();
	int nY = m_scrollBarV.GetScrollPos();
// 	m_iX = nX / fScale;
// 	m_iY = nY / fScale;
	m_iX = rcRoi.tl().x;
	m_iY = rcRoi.tl().y;
	if (m_iX > nMaxH)
		m_iX = nMaxH;
	if (m_iY > nMaxV)
		m_iY = nMaxV;
	
	SCROLLINFO infoH;
	infoH.cbSize = sizeof(SCROLLINFO);
	infoH.fMask = SIF_ALL;
	infoH.nMin = 0;
	infoH.nMax = nMaxH;
	infoH.nPage = rcRoi.width;//m_client.Width();
	infoH.nPos = m_iX;
	m_scrollBarH.SetScrollInfo(&infoH);

	SCROLLINFO infoV;
	infoV.cbSize = sizeof(SCROLLINFO);
	infoV.fMask = SIF_ALL;
	infoV.nMin = 0;
	infoV.nMax = nMaxV;
	infoV.nPage = rcRoi.height;// m_client.Height();
	infoV.nPos = m_iY;
	m_scrollBarV.SetScrollInfo(&infoV);

//	TRACE("MBtnWheel, fScale = %f, 滚动前起始位置(%d,%d), 滚动后位置(%d,%d),滚动范围(%d,%d)\n", fScale, nX, nY, m_iX, m_iY, nMaxH, nMaxV);

	return TRUE;
}

LRESULT CPicShow::RoiRbtnUp(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_RBTNUP, wParam, lParam);
	return TRUE;
}

void CPicShow::ReInit()
{
	m_picShow.OnInit(1);

	m_scrollBarV.ShowScrollBar(m_bShowScrolV);
	m_scrollBarH.ShowScrollBar(m_bShowScrolH);
	Invalidate();
}

void CPicShow::SetShowTracker(bool bShowH, bool bShowV, bool bShowSN, bool bShowZgt)
{
	m_picShow.SetShowRectTracker(bShowH, bShowV, bShowSN, bShowZgt);
}

LRESULT CPicShow::HTrackerChange(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_HTrackerChange, wParam, lParam);
	return TRUE;
}

LRESULT CPicShow::VTrackerChange(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_VTrackerChange, wParam, lParam);
	return TRUE;
}

LRESULT CPicShow::SNTrackerChange(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_SNTrackerChange, wParam, lParam);
	return TRUE;
}

LRESULT CPicShow::ZgtTrackerChange(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_ZGTTrackerChange, wParam, lParam);
	return TRUE;
}

LRESULT CPicShow::MBtnDown(WPARAM wParam, LPARAM lParam)
{
	cv::Point2f pt = *(cv::Point2f*)(wParam);
	m_iX = static_cast<int>(pt.x);
	m_iY = static_cast<int>(pt.y);
	return TRUE;
}

LRESULT CPicShow::MBtnUp(WPARAM wParam, LPARAM lParam)
{
	cv::Point2f pt = *(cv::Point2f*)(wParam);
	m_iX = static_cast<int>(pt.x);
	m_iY = static_cast<int>(pt.y);
	return TRUE;
}

LRESULT CPicShow::ShiftKeyDown(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_ShiftDown, wParam, lParam);
	return TRUE;
}

LRESULT CPicShow::ShiftKeyUp(WPARAM wParam, LPARAM lParam)
{
	::SendMessageA(this->GetParent()->GetParent()->m_hWnd, WM_CV_ShiftUp, wParam, lParam);
	return TRUE;
}

