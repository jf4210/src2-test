
// DataMgrToolDlg.h : 头文件
//

#pragma once
#include "global.h"
#include "DecompressThread.h"
#include "RecognizeThread.h"
#include "CompressThread.h"

// CDataMgrToolDlg 对话框
class CDataMgrToolDlg : public CDialogEx
{
// 构造
public:
	CDataMgrToolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_DATAMGRTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

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
	CString		m_strPkgPath;
	CString		m_strRecogPath;
	CString		m_strModelPath;

	CStatusBarCtrl	m_statusBar;

	void	showMsg(CString& strMsg);

	LRESULT MsgRecogErr(WPARAM wParam, LPARAM lParam);
	LRESULT MsgRecogComplete (WPARAM wParam, LPARAM lParam);
private:
	Poco::Thread* m_pDecompressThread;
	std::vector<CDecompressThread*> m_vecDecompressThreadObj;

	Poco::Thread*	m_pRecogThread;
	std::vector<CRecognizeThread*> m_vecRecogThreadObj;


	Poco::Thread* m_pCompressThread;
	CCompressThread* m_pCompressObj;

	void InitParam();
	void InitConfig();
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
};
