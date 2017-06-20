#pragma once


// CNewModelDlg �Ի���

class CNewModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewModelDlg)

public:
	CNewModelDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CNewModelDlg();

// �Ի�������
	enum { IDD = IDD_NEWMODELDLG };
private:
	void	InitTreeData();
	void	InitUI();
	void	InitCtrlPosition();


	CImageList  mTreeImageList;			// ���β˵���λͼ�б�
	CBitmap		mBmpRoot;				// ���ڵ�λͼ
	CBitmap		mBmpLeaf;				// �ڵ�λͼ
	CTreeCtrl	m_treeChkPoint;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
