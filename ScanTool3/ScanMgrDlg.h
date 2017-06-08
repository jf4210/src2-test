#pragma once
#include "global.h"
#include "ScanDlg.h"
#include "WaitDownloadDlg.h"
#include "ScanProcessDlg.h"
#include "ScanThread.h"
#include "ScanRecordMgrDlg.h"
#include "BmpButton.h"
#include "StudentMgr.h"

// CScanMgrDlg 对话框
#define TIMER_CheckRecogComplete	200
#define MSG_Compress_Done		(WM_USER + 202)

class CScanMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanMgrDlg)

public:
	CScanMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanMgrDlg();

// 对话框数据
	enum { IDD = IDD_SCANMGRDLG };

	CScanThread		m_scanThread;

	CScanRecordMgrDlg*	m_pScanRecordMgrDlg;

	void	InitExamData();
	bool	SearchModel();		//遍历所有模板
	bool	DownLoadModel();	//下载模板
	int		GetBmkInfo();		//下载报名库
	void	ShowChildDlg(int n);
	void	ResetChildDlg();	//子窗口重置
	void	UpdateChildDlgInfo();	//更新子窗口信息
	pTW_IDENTITY GetScanSrc(int nIndex);	//获取扫描源信息
	void*	GetScanMainDlg();	//获取扫描的主界面窗口指针
	void	SetReturnDlg(int nFlag = 2);		//设置扫描记录窗口中的返回按钮的返回界面，点返回按钮时，是返回那个窗口，目前有两个入口，一是试卷管理窗口，二是扫描过程窗口，这两个地方都可以进入到当前窗口，需要记录返回的位置   1--返回试卷管理窗口，2--返回扫描过程窗口
	LRESULT	MsgCompressResult(WPARAM wParam, LPARAM lParam);		//压缩线程操作结果
private:
	void	InitUI();
	void	InitScanner();
	void	InitCtrlPosition();
	void	InitChildDlg();
	void	ReleaseDlg();
	void	SetFontSize(int nSize);
	void	DrawBorder(CDC *pDC);	//绘制边框线

	bool	chkChangeExamLegal();	//检查切换考试或者科目时的合法性，是否允许切换
	bool	getCurrSubjectBmk();	//获取当前考试科目的报名库，从考试总报名库中提取

	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);

//	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);		//准考证号识别完成时的通知
private:
	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

	CRect		m_rtChildDlg;		//显示子窗口的区域
	CWaitDownloadDlg*	m_pWaitDownloadDlg;
	CScanDlg*			m_pScanDlg;
	CScanProcessDlg*	m_pScanProcessDlg;
//	CScanRecordMgrDlg*	m_pScanRecordMgrDlg;

	CString				m_strExamName;
	CComboBoxExt		m_comboSubject;
//	CComboBox		m_comboSubject;
//	CStudentMgr*		m_pStudentMgr;

	TwainApp*		_pTWAINApp;
	std::vector<CString> m_vecScanSrc;

	CBmpButton		m_bmpBtnChangeExam;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnDatacheck();
	afx_msg void OnBnClickedBtnScanrecord();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedBtnChangeexam();
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCbnSelchangeComboSubject();
};
