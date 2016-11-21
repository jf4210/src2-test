#pragma once
#include "global.h"
#include "Net_Cmd_Protocol.h"

#include "ComboBoxExt.h"
// CGetModelDlg 对话框

#define DEFAULT_RECVBUFF	1024*1024*2

class CGetModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CGetModelDlg)

public:
	CGetModelDlg(CString strIP, int nPort, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CGetModelDlg();

// 对话框数据
	enum { IDD = IDD_GETMODELDLG };

public:
	CProgressCtrl	m_progress;
	CComboBoxExt	m_comboExamName;
	CComboBoxExt	m_comboSubject;

	int			m_nExamID;
	CString		m_strExamName;
	CString		m_strExamTypeName;
	CString		m_strGradeName;
	int			m_SubjectID;
	CString		m_strSubjectName;
	CString		m_strScanModelName;


	Poco::Net::StreamSocket m_ss;
	int			RecvData();
	int			RecvFile(pST_DOWN_MODEL pModelInfo);
	void		InitUI();
private:
	CString		m_strServerIP;
	int			m_nServerPort;

	int			m_nRecvLen;
	int			m_nWantLen; 
	char		m_szRecvBuff[1024];
	char*		m_pFileRecv;

	CToolTipCtrl m_Tip;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	BOOL	PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboExamname();
	afx_msg void OnCbnSelchangeComboSubjectname();
	afx_msg void OnBnClickedBtnDown();
	afx_msg void OnBnClickedBtnExit();
	afx_msg void OnBnClickedBtnRefreshexam();
	afx_msg void OnBnClickedBtnCreatemodel();
};
