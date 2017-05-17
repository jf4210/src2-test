#pragma once
//#include "ExamInfoDlg.h"
#include "SingleExamDlg.h"
#include "ComboBoxExt.h"

// CExamInfoMgrDlg �Ի���

class CExamInfoMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CExamInfoMgrDlg)

public:
	CExamInfoMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CExamInfoMgrDlg();

// �Ի�������
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog(); 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CSingleExamDlg*> m_vecExamInfoDlg;
	EXAM_LIST	m_lExamList;
	CRect		m_rtExamList;		//��ʾ�����б������
	CComboBoxExt	m_comboSubject;		//��Ŀ�����б�
	CComboBoxExt	m_comboGrade;		//�꼶�����б�

	int		m_nMaxShowExamListItem;		//��ǰ������������ʾ�Ŀ����б�����
	int		m_nAllExamListItems;		//��ǰ���������£�������Ҫ��ʾ�Ŀ�������
	int		m_nCurrStartShowExamListItem;	//��ǰ��ʾ���ǿ����б��еĵڼ���
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
