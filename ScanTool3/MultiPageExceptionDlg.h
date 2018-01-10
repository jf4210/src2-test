#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "VagueSearchDlg.h"
#include "ShowPicDlg.h"


// CMultiPageExceptionDlg �Ի���

class CMultiPageExceptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CMultiPageExceptionDlg)

public:
	CMultiPageExceptionDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMultiPageExceptionDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MULTIPAGEEXCEPTIONDLG };
#endif

	CXListCtrl		m_lcIssuePaper;				//�쳣���Ծ��б�
	CXListCtrl		m_lcIssuePics;				//�Ծ��ͼƬ�б�

	pMODEL			m_pModel;				//ɨ���Ծ�ʱ��У��ģ��
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentPaperID;
	int				m_nCurrentPicID;
	
	CVagueSearchDlg* m_pVagueSearchDlg;	//ģ����������
	CShowPicDlg*	m_pShowPicDlg;		//ͼƬ��ʾ����

	COLORREF		crPaperOldText, crPaperOldBackground;
	COLORREF		crPicOldText, crPicOldBackground;

	CString			m_strPicZKZH;
	int				m_nPicPagination;		//ͼƬ��ҳ��
public:
	void	SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg);
	void	ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo);
private:
	void	InitData();
	void	InitUI();
	void	InitCtrlPosition();
	void	ShowPaperDetail(pST_PaperInfo pPaper);
	void	ShowPicDetail(pST_PicInfo pPic, bool bShowPic = false);
	void	SetPicInfo(pST_PicInfo pPic);
	void	ShowPaperByItem(int nItem);

	BOOL	PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListPics(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListPics(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
