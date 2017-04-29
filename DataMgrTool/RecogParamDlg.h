#pragma once
#include "DMTDef.h"

// CRecogParamDlg 对话框

class CRecogParamDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecogParamDlg)

public:
	CRecogParamDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRecogParamDlg();

// 对话框数据
	enum { IDD = IDD_RECOGPARAMDLG };

public:
	int		m_nChkSN;
	int		m_nChkOmr;
	int		m_nChkElectOmr;
	int		m_nNoRecogVal;		//试卷袋不识别阀值：omr怀疑 + omr空 + SN空总数大于此阀值才进行重识别
	CString		m_strEzsAddr;

	int		m_nHandleResult;	//0-使用压缩包方式，1-直接发送结果给ezs，2-不发送结果也不压缩试卷包，测试模式，看识别率
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChkSn();
	afx_msg void OnBnClickedChkOmr();
	afx_msg void OnBnClickedChkElecomr();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadioCompresspkg();
	afx_msg void OnBnClickedRadioSendezs();
	afx_msg void OnBnClickedRadioNocompress();
};
