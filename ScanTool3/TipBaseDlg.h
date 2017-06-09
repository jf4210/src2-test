#pragma once
class CTipBaseDlg : public CDialog
{
public:
	CTipBaseDlg(UINT nIDTemplate, CWnd* pParent = NULL);
	~CTipBaseDlg();


	CString m_sCaption;
public:
	void	DrawBorder(CDC* pDC, CRect rect);
protected:
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()

};

