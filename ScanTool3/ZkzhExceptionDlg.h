#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "StudentMgr.h"
#include "VagueSearchDlg.h"
#include "ShowPicDlg.h"


// CZkzhExceptionDlg �Ի���

class CZkzhExceptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CZkzhExceptionDlg)

public:
	CZkzhExceptionDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CZkzhExceptionDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ZKZHEXCEPTIONDLG };
#endif

	CXListCtrl		m_lcZkzh;				//��Ҫ�޸ĵ�׼��֤���б�
	COLORREF		crOldText, crOldBackground;

	pMODEL			m_pModel;				//ɨ���Ծ�ʱ��У��ģ��
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentSelItem;		//��ǰѡ�����

	void	SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg);
	void	ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL);
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	InitData();
	void	ReleaseData();
	BOOL	PreTranslateMessage(MSG* pMsg);
	std::string GetDetailInfo(pST_PaperInfo pPaper);

	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);
	void	SetZkzhStatus();	//����׼��֤�ŵ�ɨ��״̬
	void	ShowPaperByItem(int nItem);
	bool	VagueSearch(int nItem);		//��ĳ�����ģ������

	STUDENT_LIST	m_lBmkStudent;

	CStudentMgr*	m_pStudentMgr;
	pST_PaperInfo	m_pDefShowPaper;		//Ĭ����ʾ���Ծ�
	CVagueSearchDlg* m_pVagueSearchDlg;	//ģ����������
	CShowPicDlg*	m_pShowPicDlg;		//ͼƬ��ʾ����
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	afx_msg LRESULT OnEditEnd(WPARAM, LPARAM);
	afx_msg LRESULT OnLBtnDownEdit(WPARAM, LPARAM);
	afx_msg LRESULT MsgVagueSearchResult(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMDblclkListZkzhexception(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListZkzhexception(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
};
