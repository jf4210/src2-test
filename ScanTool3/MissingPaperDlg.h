#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"
#include "XListCtrl.h"
#include "StudentMgr.h"

// CMissingPaperDlg �Ի���

class CMissingPaperDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CMissingPaperDlg)

public:
	CMissingPaperDlg(pPAPERSINFO pPapers, pMODEL pModel, CStudentMgr* pMgr, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMissingPaperDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MISSINGPAPERDLG };
#endif


	CXListCtrl		m_lcMissingZkzh;				//ȱʧ��׼��֤���б�
	CXListCtrl		m_lcKC;							//ɨ���Ծ������Ŀ����б�
	CXListCtrl		m_lcMissingZkzh_Scaned;			//ȱʧ��׼��֤���б�(��ɨ)
	CXListCtrl		m_lcScaned;						//��ǰ��ɨ�Ծ�

	CBmpButton		m_bmpBtnClose;
	CBmpButton		m_bmpBtnSubmit;

	CString		m_strKD;	//����
	CString		m_strKC;	//����
	CString		m_strZW;	//��λ
	CString		m_strKcScaned;	//����ɨ����Ծ�

private:
	CStudentMgr*	m_pStudentMgr;
	pPAPERSINFO		m_pPapers;
	pMODEL			m_pModel;
	int				m_nCurrentSelItem;
	COLORREF		crOldText, crOldBackground;

	STUDENT_LIST	m_lAllStudent;

	CFont	fontStatus1;
	CFont	fontStatus2;
	CFont	fontStatus3;
	CFont	fontStatus4;
	CFont	fontStatus5;
	CFont	fontStatus6;
	void	SetFontSize();

	void	InitUI();
	void	InitCtrlPosition();
	void	InitData();
	void	showStudentInfo(int nItem);
	BOOL	PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMHoverListMissingpaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListMissingpaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnSubmit();
	afx_msg void OnNMDblclkListKc(NMHDR *pNMHDR, LRESULT *pResult);
};
