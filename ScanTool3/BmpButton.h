#if !defined(AFX_BmpButton_H__B8ED554A_B363_45AA_8A39_F1C207E05A89__INCLUDED_)
#define AFX_BmpButton_H__B8ED554A_B363_45AA_8A39_F1C207E05A89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// XBmpButton.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CBmpButton window
class CBmpButton : public CButton
{
// Construction
public:
	CBmpButton(BOOL p_bCutWnd = FALSE);

// Attributes
public:

	//设置背景位图
	void SetStateBitmap(
		CBitmap *p_pUncheckedBmpNormal, CBitmap *p_pUncheckedBmpSelect,
		CBitmap *p_pBmpUncheckedOver, CBitmap *p_pBmpDisable,
		CBitmap *p_pCheckedBmpNormal = NULL, CBitmap *p_pCheckedBmpSelect = NULL,
		CBitmap *p_pBmpCheckedOver = NULL);
	
	void SetStateBitmap(
		LPCTSTR lpUnNormal,				LPCTSTR lpUnSelect,
		LPCTSTR lpUnOver,				LPCTSTR lpDisabel,
		LPCTSTR lpChNormal=NULL,		LPCTSTR lpChSelect=NULL,
		LPCTSTR lpChNormalOver=NULL );

 	//设置背景位图
 	void SetStateBitmap(
 		UINT nBmpUncheckedNormalID,		UINT nBmpUncheckedSelectID,
 		UINT nBmpUncheckedOverID,		UINT nBmpDisableID=0,
		UINT nBmpCheckedNormalID=0,		UINT nBmpCheckedSelectID=0,
 		UINT nBmpCheckedOverID=0 );

	void	XEReflashBitmap();



	BOOL BtnIsChecked(){ return m_bBtnChecked; }
	void CheckBtn(BOOL p_bCheck);

	void ShowBtnText(BOOL p_bShow) { m_bShowText=p_bShow; }
	void ShowBtnText(BOOL p_bShow, BYTE bAlign) { m_bShowText = p_bShow; m_Align=bAlign; }
	void SetBtnTextColor(COLORREF p_NormalColor, COLORREF p_HoverColor, COLORREF p_SelectColor, COLORREF p_DisableColor);
	void MoveBtn(int x, int y);

	//字体显示
	void ShowBmpBtnText( CDC* pDC, const CRect& rc );
	void SetBmpBtnText( LPCTSTR szText );
	void SetBtnFont( CFont & Font );
	void SetBtnFont( CFont & NormalFont, CFont & HoverFont, CFont & SelectFont, CFont & DisableFont );
	void SetTextRect( RECT rcText );
	void SetTextXOffset( const int nXOffset, const int nYOffset=0);
	
	void SetTips(CString strTips);
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBmpButton)
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBmpButton();

	// Generated message map functions
protected:
	CBitmap *m_pBmpUncheckedNormal;
	CBitmap *m_pBmpUncheckedSelect;
	CBitmap *m_pBmpUncheckedOver;
	CBitmap *m_pBmpCheckedNormal;
	CBitmap *m_pBmpCheckedSelect;
	CBitmap *m_pBmpCheckedOver;
	CBitmap *m_pBmpDisable;

	UINT m_nBmpUncheckedNormalID;
	UINT m_nBmpUncheckedSelectID;
	UINT m_nBmpUncheckedOverID;
	UINT m_nBmpDisableID;
	UINT m_nBmpCheckedNormalID;
	UINT m_nBmpCheckedSelectID;
	UINT m_nBmpCheckedOverID;

	
	BOOL m_bBtnChecked;
	BOOL m_bCutWnd;
	BOOL m_bShowText;

	COLORREF m_NormalTextColor;
	COLORREF m_HoverTextColor;
	COLORREF m_SelectTextColor;
	COLORREF m_DisableTextColor;

	//清除所有位图
	void ReleaseBmp();

	void SetWndRng();

	//{{AFX_MSG(CBmpButton)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
		
public:
	void SetCheckBtnTextColor(COLORREF p_CheckColor);

private:
	BOOL m_bMouseIsOver;
	BOOL m_bBmpIsCreateBySelf;//位图是否由自己创建标志

	// 
	CFont* m_pFont;

	// 
	CFont m_NormalFont;
	CFont m_HoverFont;
	CFont m_SelectFont;
	CFont m_DisableFont; 

	BOOL	m_bUseDefaultFont;
	CRect	m_rcText;
	BOOL	m_bUseDefaultrc;

	//字体对齐方式
	BYTE	m_Align;
	int		m_nXOffset;

	//end
	CToolTipCtrl		m_toolTip;
public:
	// 设置默认风格，nStyle 0-2,3个文字按钮，1-3,4个文字按钮
	void SetDefaultButton(int nStyle);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_XBMPBUTTON_H__B8ED554A_B363_45AA_8A39_F1C207E05A89__INCLUDED_)
