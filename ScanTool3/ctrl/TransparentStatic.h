/************************************************************************/
/*
function£ºÍ¸±³¾°×Ö·û´®Àà
author:	zhenglei
date£º    2012/7/12
*/
/************************************************************************/

#ifndef TRANSTATIC_H
#define TRANSTATIC_H
// CTransparentStatic
#pragma once

class CTransparentStatic : public CStatic
{
	DECLARE_DYNAMIC(CTransparentStatic)

public:
	void SetTextColor(COLORREF cr);
	CTransparentStatic();
	virtual ~CTransparentStatic();
	void     SetLittleFont();
	void     SetTitleFont();
	void     SetGroupTextFont();
	void     SetLeftAlign();
	void     SetCenterAlign();
	void     UnSetLeftAlign();
	void     SetMultiLine();
	void     SetBackColor(COLORREF cr){bkcolor = cr;};
	void	 SetShowFont(CFont& font);

	COLORREF color;
	COLORREF bkcolor;
	BOOL     m_bMultiLine;
	CFont    m_font;
	BOOL     m_bLeftAlign;
	BOOL     m_bCenterAlign;

protected:
		//{{AFX_MSG(CTransparentStatic)
	afx_msg void OnPaint();
	//}}AFX_MSG
   afx_msg LRESULT OnSetText(WPARAM,LPARAM);
   afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
   DECLARE_MESSAGE_MAP()
};

#endif

