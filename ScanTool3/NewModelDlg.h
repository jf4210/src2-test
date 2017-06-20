#pragma once


// CNewModelDlg 对话框

class CNewModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewModelDlg)

public:
	CNewModelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNewModelDlg();

// 对话框数据
	enum { IDD = IDD_NEWMODELDLG };
private:
	void	InitTreeData();
	void	InitUI();
	void	InitCtrlPosition();


	CImageList  mTreeImageList;			// 树形菜单的位图列表
	CBitmap		mBmpRoot;				// 父节点位图
	CBitmap		mBmpLeaf;				// 节点位图
	CTreeCtrl	m_treeChkPoint;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
