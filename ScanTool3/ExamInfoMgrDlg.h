#pragma once
#include "SingleExamDlg.h"
#include "ComboBoxExt.h"

// CExamInfoMgrDlg 对话框

class CExamInfoMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CExamInfoMgrDlg)

public:
	CExamInfoMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CExamInfoMgrDlg();

// 对话框数据
	enum { IDD = IDD_EXAMINFOMGRDLG };

	void	ShowExamList(EXAM_LIST lExam, int nStartShow);
	void	ReleaseDlgData();
	void	InitCtrlPosition();
	void	InitShowData();
	void	ReleaseData();
private:
	void	InitSearchData();
	void	GetSearchResultExamList();
	void	GetAllShowPaperCount();
	int		GetStartExamIndex(int n);		//获得显示第n页时，起始考试在考试列表中的索引
	void	DrawBorder(CDC *pDC);	//绘制边框线
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog(); 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CSingleExamDlg*>	m_vecExamInfoDlg;
	std::vector<CLinkCtrl *>		m_vecBtnIndex;
	EXAM_LIST	m_lExamList;
	CRect		m_rtExamList;		//显示考试列表的区域
	CComboBoxExt	m_comboSubject;		//科目下拉列表
	CComboBoxExt	m_comboGrade;		//年级下拉列表

	int		m_nMaxShowExamListItem;		//当前窗口最大可以显示的考试列表行数
	int		m_nAllExamListItems;		//当前搜索条件下，所有需要显示的考试数量
	int		m_nShowPapersCount;			//当前搜索条件下，总共有多少也可以显示
	int		m_nCurrShowPaper;			//当前显示第几页

	CString m_strShowCurrPaper;			//显示当前页数信息

	//++具体考试信息子窗口
	int		m_nMaxSubsRow;		//一行最多显示的科目按钮数量
	int		m_nSubjectBtnH;		//科目按钮高度
	int		m_nDlgMinH;			//窗口最小高度
	//--
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeComboSubject();
	afx_msg void OnCbnSelchangeComboGrade();
	afx_msg void OnBnClickedBtnFirst();
	afx_msg void OnBnClickedBtnLast();
	afx_msg void OnBnClickedBtnUp();
	afx_msg void OnBnClickedBtnDown();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
