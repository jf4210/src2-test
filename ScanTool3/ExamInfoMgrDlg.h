#pragma once
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
	void	GetAllShowPaperCount();
	int		GetStartExamIndex(int n);		//�����ʾ��nҳʱ����ʼ�����ڿ����б��е�����
	void	DrawBorder(CDC *pDC);	//���Ʊ߿���
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog(); 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CSingleExamDlg*>	m_vecExamInfoDlg;
	std::vector<CLinkCtrl *>		m_vecBtnIndex;
	EXAM_LIST	m_lExamList;
	CRect		m_rtExamList;		//��ʾ�����б������
	CComboBoxExt	m_comboSubject;		//��Ŀ�����б�
	CComboBoxExt	m_comboGrade;		//�꼶�����б�

	int		m_nMaxShowExamListItem;		//��ǰ������������ʾ�Ŀ����б�����
	int		m_nAllExamListItems;		//��ǰ���������£�������Ҫ��ʾ�Ŀ�������
	int		m_nShowPapersCount;			//��ǰ���������£��ܹ��ж���Ҳ������ʾ
	int		m_nCurrShowPaper;			//��ǰ��ʾ�ڼ�ҳ

	CString m_strShowCurrPaper;			//��ʾ��ǰҳ����Ϣ

	//++���忼����Ϣ�Ӵ���
	int		m_nMaxSubsRow;		//һ�������ʾ�Ŀ�Ŀ��ť����
	int		m_nSubjectBtnH;		//��Ŀ��ť�߶�
	int		m_nDlgMinH;			//������С�߶�
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
