
// DataMgrToolDlg.h : 头文件
//

#pragma once
#include "global.h"
#include "DecompressThread.h"


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
	CMFCEditBrowseCtrl m_mfcEdit_Decrypt;

	CString		m_strRarPath;
	CString		m_strDecompressPath;
	CString		m_strEncryptPath;
	CString		m_strDecryptPath;

	CEdit		m_edit_Msg;
	CString		m_strMsg;

private:
	Poco::Thread* m_pDecompressThread;
	std::vector<CDecompressThread*> m_vecDecompressThreadObj;

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
};
