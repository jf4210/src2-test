#pragma once
#include"ZkzhExceptionDlg.h"
#include "MultiPageExceptionDlg.h"


// CZkzhShowMgrDlg 对话框

class CZkzhShowMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CZkzhShowMgrDlg)

public:
	CZkzhShowMgrDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CZkzhShowMgrDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ZKZHSHOWMGRDLG };
#endif

	CZkzhExceptionDlg*	m_pZkzhExceptionDlg;
	CMultiPageExceptionDlg*	m_pMultiPageExceptionDlg;

	void	ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL);
	//子页面调用主页面进行重新初始化数据
	//子页面中修改试卷袋信息，可能导致其他窗口中的列表保存的试卷指针信息改变，必须通过主tab来重新初始化所有子页面
	void	ReInitDataFromChildDlg(pMODEL pModel, pPAPERSINFO pPapersInfo);
	void	SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg);
	bool	ReleaseData();
private:
	std::vector<CBmpButton*> m_vecBtn;

	pPAPERSINFO		m_pPapers;
	pMODEL			m_pModel;				//扫描试卷时的校验模板
	CStudentMgr*	m_pStudentMgr;
	pST_PaperInfo	m_pDefShowPaper;		//默认显示的试卷
	CVagueSearchDlg* m_pVagueSearchDlg;	//模糊搜索窗口
	CShowPicDlg*	m_pShowPicDlg;		//图片显示窗口
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	InitData();
	BOOL	PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
};
