
// DataMgrToolDlg.h : ͷ�ļ�
//

#pragma once
#include "global.h"
#include "DecompressThread.h"
#include "RecognizeThread.h"
#include "CompressThread.h"
#include "SendToHttpThread.h"
#include "SearchThread.h"

#define TIMER_UPDATE_STARTBAR	1000

// CDataMgrToolDlg �Ի���
class CDataMgrToolDlg : public CDialogEx
{
// ����
public:
	CDataMgrToolDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DATAMGRTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	CMFCEditBrowseCtrl m_mfcEdit_RAR;
	CMFCEditBrowseCtrl m_mfcEdit_Decompress;

	CMFCEditBrowseCtrl m_mfcEdit_Encrypt;
	CMFCEditBrowseCtrl m_mfcEdit_RePkg;
	CMFCEditBrowseCtrl m_mfcEdit_RePkg_SavePath;

	CString		m_strRarPath;
	CString		m_strDecompressPath;
	CString		m_strEncryptPath;
	CString		m_strRePkgPath;
	CString		m_strRePkg_SavePath;

	CEdit		m_edit_Msg;
	CString		m_strMsg;


	CMFCEditBrowseCtrl m_mfcEdit_PkgDir;
	CMFCEditBrowseCtrl m_mfcEdit_RecogDir;
	CMFCEditBrowseCtrl m_mfcEdit_ModelPath;
	CMFCEditBrowseCtrl m_mfcEdit_ModelPath_showPapersDlg;
	CMFCEditBrowseCtrl m_mfcEdit_PkgPath_showPapersDlg;
	CString		m_strPkgPath;
	CString		m_strRecogPath;
	CString		m_strModelPath;
	CString		m_strModelPath_showPapersDlg;
	CString		m_strPkgPath_showPapersDlg;

	CString		m_strWatchPaper_PapersDir;		//�鿴�Ծ�--�Ծ������·��
	CString		m_strWatchPaper_PaperInfo;		//�鿴�Ծ�--�Ծ���Ϣ����ʽ(�Ծ������:�Ծ��ţ�12345:S1)

	CStatusBarCtrl	m_statusBar;
	CStatusBar		m_wndStatusBar;

	void	showMsg(CString& strMsg);
	void	showPapers(pPAPERSINFO pPapers);

	LRESULT MsgRecogErr(WPARAM wParam, LPARAM lParam);
	LRESULT MsgRecogComplete (WPARAM wParam, LPARAM lParam);
	LRESULT MsgSendResultState(WPARAM wParam, LPARAM lParam);
private:
	Poco::Thread* m_pDecompressThread;
	std::vector<CDecompressThread*> m_vecDecompressThreadObj;

	Poco::Thread* m_pCompressThread;
	std::vector<CCompressThread*> m_vecCompressThreadObj;

	Poco::Thread*	m_pRecogThread;
	std::vector<CRecognizeThread*> m_vecRecogThreadObj;


	Poco::Thread* m_pSearchPkgThread;
	CSearchThread* m_pSearchPkgObj;

#ifdef Test_SendRecogResult
	Poco::Thread* m_pHttpThread;
	CSendToHttpThread* m_pHttpObj;
#endif

	void InitParam();
	void InitConfig();
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMfcbuttonDecompress();
	afx_msg void OnBnClickedMfcbuttonDecrypt();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnRecogpkg();
	afx_msg void OnBnClickedBtnRerecogpkg();
	afx_msg void OnBnClickedBtnStudentanswer();
	afx_msg void OnBnClickedBtnStatistics();
	afx_msg void OnBnClickedBtnStatisticsresult();
	afx_msg void OnBnClickedBtnClearstatistics();
	afx_msg void OnBnClickedChkReadparam();
	afx_msg void OnBnClickedBtnLoadparam();
	afx_msg void OnBnClickedMfcbuttonRepkg();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnDecompresslook();
	afx_msg void OnBnClickedBtnWatchpic();
	afx_msg void OnBnClickedBtnWatchpapers();
};
