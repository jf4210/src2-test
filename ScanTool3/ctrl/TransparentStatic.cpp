// TransparentStatic.cpp : implementation file
//

#include "stdafx.h"
#include "TransparentStatic.h"
//#include "TerminalPlatform.h"

// CTransparentStatic

extern CFont					g_fontBase;
IMPLEMENT_DYNAMIC(CTransparentStatic, CStatic)
CTransparentStatic::CTransparentStatic()
{
	color=RGB(0,0,0);
	m_bLeftAlign = FALSE;
	m_bCenterAlign = FALSE;
	m_bMultiLine=FALSE;
   // m_font = NULL;
	//m_font = new CFont;
	//if(m_font)
	//{
		LOGFONT	logfont;
		g_fontBase.GetLogFont(&logfont);
		m_font.CreateFontIndirect(&logfont);
	//}
}

CTransparentStatic::~CTransparentStatic()
{
// 	if(m_font)
// 		delete m_font;
}


BEGIN_MESSAGE_MAP(CTransparentStatic, CStatic)
	//{{AFX_MSG_MAP(CTransparentStatic)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
   ON_MESSAGE(WM_SETTEXT,OnSetText)
   ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CTransparentStatic message handlers

LRESULT CTransparentStatic::OnSetText(WPARAM wParam,LPARAM lParam)
{
   LRESULT Result = Default();
   CRect Rect;
   GetWindowRect(&Rect);
   GetParent()->ScreenToClient(&Rect);
   GetParent()->InvalidateRect(&Rect);
   GetParent()->UpdateWindow();
   return Result;
}

void  CTransparentStatic::SetTitleFont()
{
	//if(m_font)
	{
		m_font.DeleteObject();
		LOGFONT	logfont;
		g_fontBase.GetLogFont(&logfont);
		if(logfont.lfHeight > 0)
			logfont.lfHeight +=4;
		else
			logfont.lfHeight -=4;
		logfont.lfWeight = FW_BOLD;
		m_font.CreateFontIndirect(&logfont);
	}
}

void  CTransparentStatic::SetLittleFont()
{
	//if(m_font)
	{
		m_font.DeleteObject();
		LOGFONT	logfont;
		g_fontBase.GetLogFont(&logfont);
		if(logfont.lfHeight > 0)
			logfont.lfHeight -=1;
		else
			logfont.lfHeight +=1;
		m_font.CreateFontIndirect(&logfont);
	}
}

void  CTransparentStatic::SetGroupTextFont()
{
	//if(m_font)
	{
		m_font.DeleteObject();
		LOGFONT	logfont;
		g_fontBase.GetLogFont(&logfont);
		if(logfont.lfHeight > 0)
			logfont.lfHeight +=1;
		else
			logfont.lfHeight -=1;
		logfont.lfWeight = FW_SEMIBOLD;
		m_font.CreateFontIndirect(&logfont);
	}
}

void CTransparentStatic::SetLeftAlign()
{
	m_bLeftAlign = TRUE;
} 

void CTransparentStatic::SetCenterAlign()
{
	m_bCenterAlign = TRUE;
} 

void CTransparentStatic::UnSetLeftAlign()
{
	m_bLeftAlign = FALSE;
} 

void CTransparentStatic::SetMultiLine()
{
	m_bMultiLine = TRUE;
}


void CTransparentStatic::SetShowFont(CFont& font)
{
	m_font.DeleteObject();
//	m_font = font;
	LOGFONT	logfont;
	font.GetLogFont(&logfont);
	m_font.CreateFontIndirect(&logfont);
}

HBRUSH CTransparentStatic::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
 //  pDC->SetBkColor(bkcolor);
   pDC->SetBkMode(TRANSPARENT);
   return (HBRUSH)GetStockObject(NULL_BRUSH);
}

void CTransparentStatic::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect client_rect;
	GetClientRect(client_rect);

	CString szText;
	GetWindowText(szText);

	CFont *pFont,*pOldFont;
	pFont=&m_font /*GetFont()*/;
	pOldFont = dc.SelectObject(pFont);


	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(color);
	//dc.SetBkColor(bkcolor);
	
// 	CBrush br(bkcolor);
// 	dc.FillRect(client_rect,&br);

	int nLines = 1;
	int nSingleCount = 0;
	LOGFONT	logfont;
	if (m_font.GetSafeHandle())
	{
		m_font.GetLogFont(&logfont);
		int nCharW = logfont.lfWidth;
		nSingleCount = client_rect.Width() / nCharW - 1;
		int nLen = szText.GetLength()/* * sizeof(TCHAR)*/;
		nLines = nLen / nSingleCount + 1;
		if (nLines > 1)
		{
			m_bMultiLine = TRUE;
		}
		else
		{
			m_bMultiLine = FALSE;
		}
	}

	DWORD dwFormat;
	if (m_bLeftAlign)		//m_bLeftAlign
		dwFormat = DT_LEFT | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;
	else if (m_bCenterAlign)	//m_bCenterAlign
		dwFormat = DT_CENTER | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;
	else
		dwFormat = DT_RIGHT | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;

	if(!m_bMultiLine)
	{
		::DrawTextEx (dc.m_hDC,szText.GetBuffer (0),szText.GetLength(),&client_rect,dwFormat,NULL);
		szText.ReleaseBuffer();
	}
	else
	{
#if 1
		LOGFONT	logfont;
		if(m_font.GetSafeHandle())
		{
// 			for (int i = 0; i < nLines; i++)
// 			{
// 				szText.Insert(nSingleCount * (i + 1) / sizeof(TCHAR), _T("\r\n"));
// 			}
			for (int i = 1; i < nLines; i++)
			{
				szText.Insert(nSingleCount * i / sizeof(TCHAR) - 1, _T("\r\n"));
			}
			dc.DrawText(szText, client_rect, DT_WORDBREAK | DT_CENTER | DT_VCENTER);	//DT_WORDBREAK | DT_LEFT /*| DT_VCENTER*//*DT_WORDBREAK*/
//			dc.DrawText(szText, client_rect, DT_WORDBREAK | DT_VCENTER | DT_VCENTER);	//DT_WORDBREAK | DT_LEFT /*| DT_VCENTER*//*DT_WORDBREAK*/
		}
		else
			dc.DrawText(szText, client_rect, DT_WORDBREAK | DT_CENTER | DT_VCENTER/*DT_WORDBREAK*/);
#else
		dc.DrawText(szText, client_rect, DT_WORDBREAK | DT_CENTER | DT_VCENTER/*DT_WORDBREAK*/);
#endif
	}

	dc.SelectObject(pOldFont);
	//pOldFont->DeleteObject();
	//delete pOldFont;
}

void CTransparentStatic::SetTextColor(COLORREF cr)
{
	color=cr;
}

