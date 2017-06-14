#pragma once
#include "StudentMgr.h"
#include "ctrl/XPGroupBox.h"
#include "ctrl/skinscrollwnd.h"
#include "BmpButton.h"

// CVagueSearchDlg �Ի���

class CVagueSearchDlg : public CDialog
{
	DECLARE_DYNAMIC(CVagueSearchDlg)

public:
	CVagueSearchDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CVagueSearchDlg();

// �Ի�������
	enum { IDD = IDD_VAGUESEARCHDLG };

	void setExamInfo(CStudentMgr* pMgr, pMODEL pModel);
	bool vagueSearch(pST_PaperInfo pPaper);
private:
	CStudentMgr*	m_pStudentMgr;
	pMODEL			m_pModel;				//ɨ���Ծ�ʱ��У��ģ��

	int				m_nSearchType;		//�������ͣ�1-������������2-��׼��֤��
	CString			m_strSearchKey;		//�����ؼ���
	CListCtrl		m_lcBmk;			//�������б�ؼ�
	CXPGroupBox		m_GroupStatic;
	CBmpButton		m_bmpBtnSearch;

	HBITMAP			m_bitmap_scrollbar;
private:
	void	InitUI();
	void	InitCtrlPosition();
	
	void	SetZkzhScaned(std::string strZkzh);		//����һ�������Ѿ�ɨ���ʶ
	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedRadioSearchname();
	afx_msg void OnBnClickedRadioSearchzkzh();
	afx_msg void OnBnClickedBtnSearch();
	afx_msg void OnNMDblclkListZkzhsearchresult(NMHDR *pNMHDR, LRESULT *pResult);
};
