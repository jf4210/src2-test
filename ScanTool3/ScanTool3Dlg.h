
// ScanTool2Dlg.h : 头文件
//

#pragma once
#include "TcpClient.h"
#include "CompressThread.h"
#include "RecognizeThread.h"
#include "SendFileThread.h"
#include "ExamInfoMgrDlg.h"
#include "ScanMgrDlg.h"
#include "ScanThread.h"
#include "BmpButton.h"
#include "ModifyZkzhDlg.h"

// CScanTool2Dlg 对话框
class CScanTool3Dlg : public CDialogEx
{
// 构造
public:
	CScanTool3Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SCANTOOL2_DIALOG };


	void	SwitchDlg(int nDlg, int nChildID = 1);		//切换窗口显示
	void	SwitchModifyZkzkDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL);
	bool	HandleModel();				//处理模板，通过子窗口处理模板
	void	DumpReleaseTwain();			//在系统崩溃时释放twain资源
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void InitThreads();
	void ReleaseThreads();
	void InitCtrlPositon();
	void ReleaseData();
	void ReleaseDlg();

	void InitUI();

	LRESULT	MsgCmdDlModel(WPARAM wParam, LPARAM lParam);
	LRESULT MsgCmdGetBmk(WPARAM wParam, LPARAM lParam);
private:
	Poco::Thread*		m_pRecogThread;		//识别线程对象数组
	std::vector<CRecognizeThread*> m_vecRecogThreadObj;		//存储识别线程对象
	Poco::Thread*		m_SendFileThread;
	CSendFileThread*	m_pSendFileObj;
	Poco::Thread*		m_TcpCmdThread;
	CTcpClient*			m_pTcpCmdObj;
	Poco::Thread*		m_pCompressThread;
	CCompressThread*	m_pCompressObj;
// 	Poco::Thread*		m_pScanThread;
// 	CScanThread*		m_pScanThreadObj;
//	CScanThread			m_scanThread;

	//++窗口指针
	CExamInfoMgrDlg*	m_pExamInfoMgrDlg;
	CScanMgrDlg*		m_pScanMgrDlg;
	CModifyZkzhDlg*		m_pModifyZkzhDlg;
	//--


	CBitmap		m_bmpBkg;
	CBitmap		m_bmpTitle;
	CBmpButton	m_bmpBtnMin;
	CBmpButton	m_bmpBtnExit;
	CBmpButton	m_bmpBtnUserPic;	//用户头像按钮

	CString		m_strTitle;
	CString		m_strVersion;
	CString		m_strUserName;		//显示的用户名
	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontTitle;			//标题字体
	CFont			m_fontVersion;			//版本字体
	CFont			m_fontUserName;			//用户名字体
	COLORREF		m_colorStatus;			//状态栏字体颜色
	void	SetFontSize(int nSize);
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
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnMin();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
