#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ScanReminderDlg.h"
#include "ShowPicDlg.h"

// CScanProcessDlg �Ի���

class CScanProcessDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanProcessDlg)

public:
	CScanProcessDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanProcessDlg();

// �Ի�������
	enum { IDD = IDD_SCANPROCESSDLG };

	void	AddPaper(int nID, pST_PaperInfo pPaper);
	void	ResetPicList();
	void	InitShow();
	void UpdateChildInfo(bool bScanDone = false);
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	EnableBtn(BOOL bEnable);
	void	WriteJsonFile();
private:
	CXListCtrl	m_lcPicture;
	CRect		m_rtChildDlg;	//�ӿؼ�λ��
	CScanReminderDlg* m_pReminderDlg;	//ɨ����ʾ�򴰿�
	CShowPicDlg*	m_pShowPicDlg;

	int				m_nCurrentScanCount;	//��ǰɨ����Ҫɨ���Ծ�����
	std::string		m_strCurrPicSavePath;		//gb2312
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnScanagain();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnDestroy();
	afx_msg void OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnScanprocess();
};
