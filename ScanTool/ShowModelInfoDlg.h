#pragma once
#include "global.h"

// CShowModelInfoDlg 对话框

class CShowModelInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowModelInfoDlg)

public:
	CShowModelInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowModelInfoDlg();

// 对话框数据
	enum { IDD = IDD_SHOWMODELINFODLG };

public:
	void ShowModelInfo(pMODEL pModel, int nFlag = 0);

private:
	void InitUI();
	void InitCtrlPosition();
	void SetFontSize(int nNameSize, int nDesSize);

	CString			m_strModelName;
	CString			m_strModelDesc;

	int				m_nNameSize;			//字体大小
	int				m_nDesSize;
	CFont			m_fontStatus_Name;		//字体
	CFont			m_fontStatus_Des;		//字体
	COLORREF		m_colorStatus;			//字体颜色
	CToolTipCtrl	m_toolTip;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
