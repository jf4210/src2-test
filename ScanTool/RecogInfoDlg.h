#pragma once
#include "global.h"

// CRecogInfoDlg 对话框

class CRecogInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecogInfoDlg)

public:
	CRecogInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRecogInfoDlg();

// 对话框数据
	enum { IDD = IDD_RECOGINFODLG };

public:
	pRECTINFO	m_pCurRectInfo;
	CString		m_strCPTypeName;		//校验点的类型名
	int			m_nThresholdVal;		//校验点的识别阀值
	float		m_fThresholdValPercent;	//校验点达到阀值的比例

	void		ShowDetailRectInfo(pRECTINFO	pCurRectInfo);

private:
	void		InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSaverecoginfo();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
