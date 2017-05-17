#pragma once
//#include "ExamInfoDlg.h"
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
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog(); 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CSingleExamDlg*> m_vecExamInfoDlg;
	EXAM_LIST	m_lExamList;
	CRect		m_rtExamList;		//显示考试列表的区域
	CComboBoxExt	m_comboSubject;		//科目下拉列表
	CComboBoxExt	m_comboGrade;		//年级下拉列表

	int		m_nMaxShowExamListItem;		//当前窗口最大可以显示的考试列表行数
	int		m_nAllExamListItems;		//当前搜索条件下，所有需要显示的考试数量
	int		m_nCurrStartShowExamListItem;	//当前显示的是考试列表中的第几项
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
};
