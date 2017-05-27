// XBmpButton.cpp : implementation file
//

#include "stdafx.h"
#include "BmpButton.h"
//

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBmpButton
extern CFont	g_fontBase;	

CBmpButton::CBmpButton(BOOL p_bCutWnd)
	: m_pBmpUncheckedNormal(NULL)
	, m_pBmpUncheckedSelect(NULL)
	, m_pBmpUncheckedOver(NULL)
	, m_pBmpCheckedNormal(NULL)
	, m_pBmpCheckedSelect(NULL)
	, m_pBmpCheckedOver(NULL)
	, m_pBmpDisable(NULL)
	, m_bMouseIsOver(FALSE)
	, m_bBtnChecked(FALSE)
	, m_bBmpIsCreateBySelf(FALSE)
	, m_bCutWnd(p_bCutWnd)
	, m_bShowText(TRUE)
	, m_Align(DT_CENTER)
{
#if 0
	m_NormalTextColor = GetSysColor(COLOR_BTNTEXT);
	m_HoverTextColor = GetSysColor(COLOR_BTNHIGHLIGHT);
	m_SelectTextColor = GetSysColor(COLOR_BTNHIGHLIGHT);
#endif 

	m_NormalTextColor = RGB(0,0,0);
	m_HoverTextColor = RGB(0,0,0);
	m_SelectTextColor = RGB(0,0,0);

	m_DisableTextColor = GetSysColor(COLOR_GRAYTEXT);

	m_bUseDefaultrc = TRUE;
	m_bUseDefaultFont = TRUE;


	m_nBmpUncheckedNormalID	=0;
	m_nBmpUncheckedSelectID	=0;
	m_nBmpUncheckedOverID	=0;
	m_nBmpDisableID			=0;	
	m_nBmpCheckedNormalID	=0;
	m_nBmpCheckedSelectID	=0;
	m_nBmpCheckedOverID		=0;

	
}

CBmpButton::~CBmpButton()
{
	if (m_bBmpIsCreateBySelf) 
		ReleaseBmp();
}




void CBmpButton::PreSubclassWindow()
{
	CButton::PreSubclassWindow();
	
	ModifyStyle(0, BS_OWNERDRAW);
}

BEGIN_MESSAGE_MAP(CBmpButton, CButton)
	//{{AFX_MSG_MAP(CBmpButton)
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)   
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)   
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBmpButton message handlers
void CBmpButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC); 
	pDC->SetBkMode(TRANSPARENT); 

	CRect rcBtnRect(lpDrawItemStruct->rcItem);

	CBitmap *pBmp = NULL;
	
	const UINT nState = lpDrawItemStruct->itemState;
	if (nState & ODS_SELECTED) 
	{ 
		if (m_bBtnChecked && NULL != m_pBmpCheckedSelect)
			pBmp = m_pBmpCheckedSelect;
		else
			pBmp = m_pBmpUncheckedSelect ? m_pBmpUncheckedSelect : m_pBmpUncheckedNormal;
			
		pDC->SetTextColor(m_SelectTextColor);

		if(!m_bUseDefaultFont)
			m_pFont = &m_SelectFont;
	} 
	else if (nState & ODS_DISABLED)
	{
		pBmp = m_pBmpDisable ? m_pBmpDisable : m_pBmpUncheckedNormal;
			
		pDC->SetTextColor(m_DisableTextColor);

		if(!m_bUseDefaultFont)
			m_pFont = &m_DisableFont;
	} 
	else if (m_bMouseIsOver)
	{
		if (m_bBtnChecked && NULL != m_pBmpCheckedOver)
			pBmp = m_pBmpCheckedOver;
		else
			pBmp = m_pBmpUncheckedOver ? m_pBmpUncheckedOver : m_pBmpUncheckedNormal;
			
		pDC->SetTextColor(m_HoverTextColor);
		
		if(!m_bUseDefaultFont)
			m_pFont = &m_HoverFont;
	} 
	else
	{
		if (m_bBtnChecked && NULL != m_pBmpCheckedNormal)
			pBmp = m_pBmpCheckedNormal;
		else
			pBmp = m_pBmpUncheckedNormal;
			
		pDC->SetTextColor(m_NormalTextColor);

		if(!m_bUseDefaultFont)
			m_pFont = &m_NormalFont;
	}

	if (pBmp)
	{
		CBrush BmpBrush(pBmp);
		CDC *pMemDC = new CDC;
		pMemDC->CreateCompatibleDC(pDC);
		HGDIOBJ hOldObject=NULL;
		HBITMAP hbk= (HBITMAP)pBmp->m_hObject;
		BITMAP BmpInfo;
		pBmp->GetBitmap(&BmpInfo);
		if (NULL != hbk)
		{
			hOldObject = pMemDC->SelectObject(hbk);
			pDC->SetStretchBltMode(COLORONCOLOR);
			pDC->StretchBlt(rcBtnRect.left, rcBtnRect.top, rcBtnRect.Width(),rcBtnRect.Height(), pMemDC,
				0, 0,BmpInfo.bmWidth, BmpInfo.bmHeight ,SRCCOPY);
			pMemDC->SelectObject(hOldObject);
		}

		delete pMemDC;
	}
	ShowBmpBtnText(pDC, rcBtnRect);	

	CWnd *pWnd = this->GetParent();
	GetWindowRect(&rcBtnRect);
	pWnd->ScreenToClient(rcBtnRect);
	MoveWindow(rcBtnRect);      //调整按钮大小以适应图片
}

void CBmpButton::ShowBmpBtnText(CDC* pDC, const CRect& rc)
{
	if ( m_bShowText )
	{
		CString strBtnText;
		GetWindowText(strBtnText);
		
		if ( !strBtnText.IsEmpty() )
		{
			//是否使用默认字体
			CFont* poldfont = NULL;
			if ( m_bUseDefaultFont )
			{
				poldfont = pDC->SelectObject(&g_fontBase);	
			}
			else
			{
				poldfont = pDC->SelectObject(m_pFont);	
			}
			
			//是否使用默认方式显示字体
			//默认是居中，
			if ( m_bUseDefaultrc )
			{
				CRect rcBtnRect = rc;
// 				m_rcText = rc;
				GetClientRect(rcBtnRect);
				GetClientRect(m_rcText);

				LOGFONT lf;
				if (m_bUseDefaultFont)
				{
					g_fontBase.GetLogFont(&lf);
				}
				else
				{
					m_pFont->GetLogFont(&lf);
				}
				int nH = abs(lf.lfHeight);
				//纵向居中显示 
				m_rcText.top = m_rcText.bottom/2 - nH / 2;	//7
				//m_rcText.bottom = rcBtnRect.bottom / 2 + 10;
			}

			
			pDC->DrawText( strBtnText, -1, m_rcText, m_Align|DT_WORDBREAK|DT_VCENTER );
			
			if(poldfont)
				pDC->SelectObject(poldfont);
		}
	}
}



BOOL CBmpButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (!m_bMouseIsOver)
	{
		TRACKMOUSEEVENT   tme;   
		tme.cbSize = sizeof(tme);   
		tme.hwndTrack = GetSafeHwnd();  
		tme.dwFlags = TME_HOVER | TME_LEAVE;  
		tme.dwHoverTime = HOVER_DEFAULT;
		_TrackMouseEvent(&tme);

		m_bMouseIsOver = TRUE;

		Invalidate();
	}
	
	return CButton::OnSetCursor(pWnd, nHitTest, message);
}
	
LRESULT CBmpButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_bMouseIsOver = FALSE;

	Invalidate();

	return 0;
}
	
LRESULT CBmpButton::OnMouseHover(WPARAM wParam, LPARAM lParam)
{

	return 0;
}

void CBmpButton::OnSize(UINT nType, int cx, int cy) 
{
	CButton::OnSize(nType, cx, cy);
	
	if (m_bCutWnd) 
		SetWndRng();
}
	
//清除所有位图
void CBmpButton::ReleaseBmp()
{
	if(m_pBmpUncheckedNormal){ delete m_pBmpUncheckedNormal; m_pBmpUncheckedNormal = NULL; }
	if(m_pBmpUncheckedSelect){ delete m_pBmpUncheckedSelect; m_pBmpUncheckedSelect = NULL; }
	if(m_pBmpUncheckedOver){ delete m_pBmpUncheckedOver; m_pBmpUncheckedOver = NULL; }

	if(m_pBmpCheckedNormal){ delete m_pBmpCheckedNormal; m_pBmpCheckedNormal = NULL; }
	if(m_pBmpCheckedSelect){ delete m_pBmpCheckedSelect; m_pBmpCheckedSelect = NULL; }
	if(m_pBmpCheckedOver){ delete m_pBmpCheckedOver; m_pBmpCheckedOver = NULL; }

	if(m_pBmpDisable){ delete m_pBmpDisable; m_pBmpDisable = NULL; }
	
}

//设置背景位图，设置的位图资源由调用者负责释放
void CBmpButton::SetStateBitmap(CBitmap *p_pUncheckedBmpNormal, CBitmap *p_pUncheckedBmpSelect,
								 CBitmap *p_pBmpUncheckedOver, CBitmap *p_pBmpDisable,
								 CBitmap *p_pCheckedBmpNormal, CBitmap *p_pCheckedBmpSelect,
								 CBitmap *p_pBmpCheckedOver)
{
	ASSERT(p_pUncheckedBmpNormal);

	m_pBmpUncheckedNormal = p_pUncheckedBmpNormal;
	m_pBmpUncheckedSelect = p_pUncheckedBmpSelect;
	m_pBmpUncheckedOver = p_pBmpUncheckedOver;
	
	m_pBmpCheckedNormal = p_pCheckedBmpNormal;
	m_pBmpCheckedSelect = p_pCheckedBmpSelect;
	m_pBmpCheckedOver = p_pBmpCheckedOver;
	
	m_pBmpDisable = p_pBmpDisable;

	m_bBmpIsCreateBySelf = FALSE;
	
	///////////////////////////////
	BITMAP BmpInfo;
	p_pUncheckedBmpNormal->GetBitmap(&BmpInfo);

	CRect rcBtnRect;
	CWnd *pWnd = this->GetParent();
	GetWindowRect(&rcBtnRect);
	pWnd->ScreenToClient(rcBtnRect);

	rcBtnRect.right = rcBtnRect.left + BmpInfo.bmWidth;
	rcBtnRect.bottom = rcBtnRect.top + BmpInfo.bmHeight;


	MoveWindow(rcBtnRect);
}

//设置背景位图
void CBmpButton::SetStateBitmap(
					UINT nBmpUncheckedNormalID,		UINT nBmpUncheckedSelectID,
					UINT nBmpUncheckedOverID,		UINT nBmpDisableID,
					UINT nBmpCheckedNormalID,		UINT nBmpCheckedSelectID,
					UINT nBmpCheckedOverID )
		
{
	//若位图资源是由自己创建则先释放它们
	if (m_bBmpIsCreateBySelf) 
		ReleaseBmp();
	
	ASSERT(0!=nBmpUncheckedNormalID);
	
	m_pBmpUncheckedNormal = new CBitmap;
	m_pBmpUncheckedNormal->LoadBitmap(nBmpUncheckedNormalID);
	
	if (0!=nBmpUncheckedSelectID)
	{
		m_pBmpUncheckedSelect = new CBitmap;
		m_pBmpUncheckedSelect->LoadBitmap(nBmpUncheckedSelectID);
	}
	
	if (0!=nBmpUncheckedOverID)
	{
		m_pBmpUncheckedOver = new CBitmap;
		m_pBmpUncheckedOver->LoadBitmap(nBmpUncheckedOverID);
	}
	
	if (0!=nBmpCheckedNormalID)
	{
		m_pBmpCheckedNormal = new CBitmap;
		m_pBmpCheckedNormal->LoadBitmap(nBmpCheckedNormalID);
	}
	
	if (0!=nBmpCheckedSelectID)
	{
		m_pBmpCheckedSelect = new CBitmap;
		m_pBmpCheckedSelect->LoadBitmap(nBmpCheckedSelectID);
	}
		
	if (0!=nBmpCheckedOverID)
	{
		m_pBmpCheckedOver = new CBitmap;
		m_pBmpCheckedOver->LoadBitmap(nBmpCheckedOverID);
	}
	
	if (0!=nBmpDisableID)
	{
		m_pBmpDisable = new CBitmap;
		m_pBmpDisable->LoadBitmap(nBmpDisableID);
	}
	
	m_bBmpIsCreateBySelf = TRUE;
	
	///////////////////////////////
// 	BITMAP BmpInfo;
// 	m_pBmpUncheckedNormal->GetBitmap(&BmpInfo);
// 	
// 	CRect rcBtnRect;
// 	GetWindowRect(rcBtnRect);
// 	
// 	rcBtnRect.right = rcBtnRect.left + BmpInfo.bmWidth;
// 	rcBtnRect.bottom = rcBtnRect.top + BmpInfo.bmHeight;
// 	
// 	MoveWindow(rcBtnRect);
}


void CBmpButton::SetStateBitmap(
								  LPCTSTR lpUnNormal,LPCTSTR lpUnSelect,
								  LPCTSTR lpUnOver,LPCTSTR lpDisabel,
								  LPCTSTR lpChNormal, LPCTSTR lpChSelect,
								  LPCTSTR lpChNormalOver )
{
	CBitmap *pBmpUncheckedNormal	= new CBitmap;
	CBitmap *pBmpUncheckedSelect	= new CBitmap;
	CBitmap *pBmpUncheckedOver		= new CBitmap;
	CBitmap *pBmpCheckedNormal		= new CBitmap;
	CBitmap *pBmpCheckedSelect		= new CBitmap;
	CBitmap *pBmpCheckedOver		= new CBitmap;
	CBitmap *pBmpDisable			= new CBitmap;
	
	HBITMAP hBmpUnNormal = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),lpUnNormal,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
	pBmpUncheckedNormal->Attach(hBmpUnNormal);
	
	HBITMAP hBmpUnSelect = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),lpUnSelect,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
	pBmpUncheckedSelect->Attach(hBmpUnSelect);
	
	HBITMAP hBmpUnOver = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),lpUnOver,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
	pBmpUncheckedOver->Attach(hBmpUnOver);
	

	if (lpDisabel != lpUnNormal)
	{
		HBITMAP hBmpDisable = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),lpDisabel,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
		if (hBmpDisable)
			pBmpDisable->Attach(hBmpDisable);
	}
	else
	{
		pBmpDisable->Attach(hBmpUnNormal);
	}

	if (lpChNormal == lpUnSelect || lpChSelect == lpUnSelect)
	{
		pBmpCheckedNormal->Attach(hBmpUnSelect);
		pBmpCheckedSelect->Attach(hBmpUnSelect);
		pBmpCheckedOver->Attach(hBmpUnOver);
	}
	else
	{
		if (lpChNormal)
		{
			HBITMAP hBmpChNormal = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),lpChNormal,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
			pBmpCheckedNormal->Attach(hBmpChNormal);
		}
		if (lpChSelect != NULL )
		{
			HBITMAP hBmpChSelect = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),lpChSelect,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
			if (hBmpChSelect)
				pBmpCheckedSelect->Attach(hBmpChSelect);
		}
		if (lpChNormalOver != NULL)
		{
			HBITMAP hBmpChOver = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),lpChNormalOver,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
			if (hBmpChOver)
				pBmpCheckedOver->Attach(hBmpChOver);
		}

	}
	
	SetStateBitmap(pBmpUncheckedNormal,pBmpUncheckedSelect,pBmpUncheckedOver,pBmpDisable,pBmpCheckedNormal,pBmpCheckedSelect,pBmpCheckedOver);
	
	m_bBmpIsCreateBySelf = TRUE;
}

void CBmpButton::SetBtnTextColor(COLORREF p_NormalColor, COLORREF p_HoverColor, COLORREF p_SelectColor, COLORREF p_DisableColor)
{
	m_NormalTextColor = p_NormalColor;
	m_HoverTextColor = p_HoverColor;
	m_SelectTextColor = p_SelectColor;
	m_DisableTextColor = p_DisableColor;
}
	
void CBmpButton::CheckBtn(BOOL p_bCheck)
{
	if (m_bBtnChecked == p_bCheck) return;

	m_bBtnChecked = p_bCheck; 

	Invalidate();
}

void CBmpButton::MoveBtn(int x, int y)
{
	if (NULL == m_pBmpUncheckedNormal) return;
	
	BITMAP BmpInfo;
	m_pBmpUncheckedNormal->GetBitmap(&BmpInfo);
	
	MoveWindow(x, y, BmpInfo.bmWidth, BmpInfo.bmHeight);
}	

void CBmpButton::SetWndRng()
{
	if (NULL == m_pBmpUncheckedNormal) return;

	CRect rcDlgRect;
	GetWindowRect(rcDlgRect);

	BITMAP BmpInfo;
	m_pBmpUncheckedNormal->GetBitmap(&BmpInfo);

	MoveWindow(rcDlgRect.left, rcDlgRect.top, BmpInfo.bmWidth, BmpInfo.bmHeight);

	const int iBitWidth = BmpInfo.bmBitsPixel / 8;
	
	if(iBitWidth == 4)
	{
		//有alpha通道，则裁窗口形状

		CRgn rgn;
		rgn.CreateRectRgn(0, 0, BmpInfo.bmWidth, BmpInfo.bmHeight);

		int iBitSize = BmpInfo.bmWidth * BmpInfo.bmHeight * iBitWidth;
		BYTE *pBits = new BYTE[iBitSize];

		m_pBmpUncheckedNormal->GetBitmapBits(iBitSize, pBits);

		for (int y = 0; y < BmpInfo.bmHeight; y++)
		{
			int iRow = y * BmpInfo.bmWidth;

			for (int x = 0; x < BmpInfo.bmWidth; x++)
			{
				int iPos = (iRow + x) * iBitWidth;

				if (pBits[iPos + 3] == 0)//检测alpha是否为透明
				{
					CRgn rgnCut;
					rgnCut.CreateRectRgn(x, y, x+1, y+1);

					rgn.CombineRgn(&rgn, &rgnCut, RGN_XOR);
				}
			}
		}
		
		SetWindowRgn(rgn, FALSE);

		delete []pBits;
	}
}


void CBmpButton::SetBmpBtnText( LPCTSTR szText )
{
	ASSERT(NULL!=szText);
	m_bShowText = TRUE;
	SetWindowText(szText);
}


void CBmpButton::SetBtnFont( CFont & Font )
{
	ASSERT(NULL!=Font.m_hObject);
	SetBtnFont( Font, Font, Font, Font );
}

void CBmpButton::SetBtnFont( CFont & NormalFont, CFont & HoverFont, CFont & SelectFont, CFont & DisableFont )
{
	ASSERT(NULL != NormalFont.m_hObject);
	ASSERT(NULL != HoverFont.m_hObject);
	ASSERT(NULL != SelectFont.m_hObject);
	ASSERT(NULL != DisableFont.m_hObject);
	
	LOGFONT LogFont;
	
	NormalFont.GetLogFont(&LogFont);
	m_NormalFont.DeleteObject();
	m_NormalFont.CreateFontIndirect(&LogFont);

	HoverFont.GetLogFont(&LogFont);	
	m_HoverFont.DeleteObject();
	m_HoverFont.CreateFontIndirect(&LogFont);

	SelectFont.GetLogFont(&LogFont);
	m_SelectFont.DeleteObject();
	m_SelectFont.CreateFontIndirect(&LogFont);

	DisableFont.GetLogFont(&LogFont);		
	m_DisableFont.DeleteObject();
	m_DisableFont.CreateFontIndirect(&LogFont);
	
	m_bUseDefaultFont = FALSE;
	
}

void CBmpButton::SetTextRect( RECT rcText )
{
	m_rcText = rcText;
	m_bUseDefaultrc = FALSE;
	m_bShowText = TRUE;
}

void CBmpButton::SetTextXOffset( const int nXOffset, const int nYOffset)
{
	GetClientRect(m_rcText);

	m_rcText.left = nXOffset;
	m_rcText.top = nYOffset;
	m_bUseDefaultrc = FALSE;
	m_bShowText = TRUE;
}

void CBmpButton::SetTips(CString strTips)
{
// 	m_TipCtrl.Create(this);
// 	m_TipCtrl.Activate(TRUE);
// 	 m_TipCtrl.AddTool(this,strTips,NULL,0);
}	

void CBmpButton::SetCheckBtnTextColor(COLORREF p_CheckColor)
{
// 	m_bSetCheckcolor = TRUE;
// 	m_colCheckText = p_CheckColor;
}


void CBmpButton::XEReflashBitmap()
{
// 	//若位图资源是由自己创建则先释放它们
// 	if (m_bBmpIsCreateBySelf) 
// 		ReleaseBmp();
// 	
// 	ASSERT(0!=m_nBmpUncheckedNormalID);
// 	
// 	m_pBmpUncheckedNormal = new CBitmap;
// 	m_pBmpUncheckedNormal->Attach(XELoadBitmap(m_nBmpUncheckedNormalID));
// 	
// 	if (0!=m_nBmpUncheckedSelectID)
// 	{
// 		m_pBmpUncheckedSelect = new CBitmap;
// 		m_pBmpUncheckedSelect->Attach(XELoadBitmap(m_nBmpUncheckedSelectID));
// 	}
// 	
// 	if (0!=m_nBmpUncheckedOverID)
// 	{
// 		m_pBmpUncheckedOver = new CBitmap;
// 		m_pBmpUncheckedOver->Attach(XELoadBitmap(m_nBmpUncheckedOverID));
// 	}
// 	
// 	if (0!=m_nBmpCheckedNormalID)
// 	{
// 		m_pBmpCheckedNormal = new CBitmap;
// 		m_pBmpCheckedNormal->Attach(XELoadBitmap(m_nBmpCheckedNormalID));
// 	}
// 	
// 	if (0!=m_nBmpCheckedSelectID)
// 	{
// 		m_pBmpCheckedSelect = new CBitmap;
// 		m_pBmpCheckedSelect->Attach(XELoadBitmap(m_nBmpCheckedSelectID));
// 	}
// 	
// 	
// 	if (0!=m_nBmpCheckedOverID)
// 	{
// 		m_pBmpCheckedOver = new CBitmap;
// 		m_pBmpCheckedOver->Attach(XELoadBitmap(m_nBmpCheckedOverID));
// 	}
// 	
// 	if (0!=m_nBmpDisableID)
// 	{
// 		m_pBmpDisable = new CBitmap;
// 		m_pBmpDisable->Attach(XELoadBitmap(m_nBmpDisableID));
// 	}
// 	
// 	m_bBmpIsCreateBySelf = TRUE;
}



// 设置默认风格，nStyle 0-2,3个文字按钮，1-4个文字按钮
void CBmpButton::SetDefaultButton(int nStyle)
{
	if (nStyle==0)
	{
		SetStateBitmap(_T("skin\\btndefault1normal.bmp"), _T("skin\\btndefault1Click.bmp"), _T("skin\\btndefault1over.bmp"), _T("skin\\btndefault1normal.bmp"));
	}
	else if (nStyle==1)
	{
		SetStateBitmap(_T("skin\\btndefault2normal.bmp"), _T("skin\\btndefault2click.bmp"), _T("skin\\btndefault2over.bmp"), _T("skin\\btndefault2normal.bmp"));
	}
	else if (nStyle==2)
	{
		SetStateBitmap(_T("skin\\btndefault3normal.bmp"), _T("skin\\btndefault3click.bmp"), _T("skin\\btndefault3over.bmp"), _T("skin\\btndefault3normal.bmp"));
	}
}
