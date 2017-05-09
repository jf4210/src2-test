#pragma once
#include "ExamInfoDlg.h"
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
	void	InitData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog(); 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CExamInfoDlg*> m_vecExamInfoDlg;
	EXAM_LIST	m_lExamList;
	CRect		m_rtExamList;		//��ʾ�����б������
	CComboBoxExt	m_comboSubject;		//��Ŀ�����б�
	CComboBoxExt	m_comboGrade;		//�꼶�����б�
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeComboSubject();
	afx_msg void OnCbnSelchangeComboGrade();
};
