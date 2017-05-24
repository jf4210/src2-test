#pragma once

class CIconWnd : public CStatic
{
public:
	CIconWnd();
	virtual ~CIconWnd();


public:
	void SetIcon( HICON icon );

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:

	HICON	m_icon;
};

/////////////////////////////////////////////////////////////////////////////
// CIconEdit window

class CIconEdit : public CEdit
{
// Construction/destruction
public:
	CIconEdit();
	virtual ~CIconEdit();

// Operations
public:
	void SetIcon(HICON icon);
	void SetIcon(UINT iconres);

protected:
	virtual void PreSubclassWindow();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam); // Maps to WM_SETFONT
	DECLARE_MESSAGE_MAP()

private:
	void		Prepare();

// Attributes
	CIconWnd	m_icon;
	HICON		m_internalIcon;
public:
	//afx_msg void OnPaint();
//	afx_msg void OnPaint();
//	afx_msg void OnEnSetfocus();
};
