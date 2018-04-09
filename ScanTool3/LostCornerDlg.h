#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "VagueSearchDlg.h"
#include "ShowPicDlg.h"


// CLostCornerDlg �Ի���

class CLostCornerDlg : public CDialog
{
	DECLARE_DYNAMIC(CLostCornerDlg)

public:
	CLostCornerDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CLostCornerDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOSTCORNERDLG };
#endif

	CXListCtrl		m_lcLostCornerPaper;				//ȱ�ǵ��Ծ��б�

	pMODEL			m_pModel;				//ɨ���Ծ�ʱ��У��ģ��
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentPaperID;
	int				m_nCurrentPicID;

	CVagueSearchDlg* m_pVagueSearchDlg;	//ģ����������
	CShowPicDlg*	m_pShowPicDlg;		//ͼƬ��ʾ����

	COLORREF		crPaperOldText, crPaperOldBackground;
	COLORREF		crPicOldText, crPicOldBackground;
public:
	void	SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg);
	void	ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo);
private:
	void	InitData();
	void	InitUI();
	void	InitCtrlPosition();

	void	ShowPaperDetail(pST_PaperInfo pPaper);

	BOOL	PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNMDblclkListLostcorner(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListLostcorner(NMHDR *pNMHDR, LRESULT *pResult);
};
