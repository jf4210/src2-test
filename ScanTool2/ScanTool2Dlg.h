
// ScanTool2Dlg.h : 头文件
//

#pragma once
#include "TcpClient.h"
#include "CompressThread.h"
#include "RecognizeThread.h"
#include "ExamInfoMgrDlg.h"
#include "ScanMgrDlg.h"
#include "ScanThread.h"

// CScanTool2Dlg 对话框
class CScanTool2Dlg : public CDialogEx
{
// 构造
public:
	CScanTool2Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SCANTOOL2_DIALOG };


	void	SwitchDlg(int nDlg);		//切换窗口显示
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void InitThreads();
	void ReleaseThreads();
	void InitCtrlPositon();
	void ReleaseData();
	void ReleaseDlg();

	void InitUI();
private:
	Poco::Thread*		m_pRecogThread;		//识别线程对象数组
	std::vector<CRecognizeThread*> m_vecRecogThreadObj;		//存储识别线程对象
	Poco::Thread*		m_TcpCmdThread;
	CTcpClient*			m_pTcpCmdObj;
	Poco::Thread*		m_pCompressThread;
	CCompressThread*	m_pCompressObj;
// 	Poco::Thread*		m_pScanThread;
// 	CScanThread*		m_pScanThreadObj;
	CScanThread			m_scanThread;

	//++窗口指针
	CExamInfoMgrDlg*	m_pExamInfoMgrDlg;
	CScanMgrDlg*		m_pScanMgrDlg;
	//--
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
