#pragma once
#include "ExamInfoDlg.h"
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
	void	InitData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog(); 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CExamInfoDlg*> m_vecExamInfoDlg;
	EXAM_LIST	m_lExamList;
	CRect		m_rtExamList;		//显示考试列表的区域
	CComboBoxExt	m_comboSubject;		//科目下拉列表
	CComboBoxExt	m_comboGrade;		//年级下拉列表
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeComboSubject();
	afx_msg void OnCbnSelchangeComboGrade();
};
