#pragma once
#include "ShowPicDlg.h"
#include "AnswerShowDlg.h"
#include "XListCtrl.h"

// CShowPapersDlg �Ի���

class CShowPapersDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPapersDlg)

public:
	CShowPapersDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShowPapersDlg();

// �Ի�������
	enum { IDD = IDD_SHOWPAPERSDLG };

	CXListCtrl	m_listPaper;
	void setShowPapers(pPAPERSINFO pPapers);
	void ShowPapers(pPAPERSINFO pPapers);
	void ShowPaper(pST_PaperInfo pPaper);
	void ShowOmrTh(pST_PaperInfo pPaper, int nTh);	//��ʾ���
private:
	void InitUI();
	void InitCtrlPosition();
	void	LeftRotate();
	void	RightRotate();
	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);

	void	SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//�����б������ʾ
	void	UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//ȡ���б������ʾ
	COLORREF		crOldText, crOldBackground;

	int		m_nPaperShowType;	//��ʾ���ͣ�0-����ʾ��1-�޻���, 2-��ʾ���ɣ�3-��ʾ�޻��ɺͻ��ɣ�4-��ʾ�գ�5-��ʾ�޻��ɺͿգ�6-��ʾ���ɺͿ�, 7-��ʾ����
	void ShowPaperListByType(int nType);	//��ʾ���ͣ�0-����ʾ��1-�޻���, 2-��ʾ���ɣ�3-��ʾ�޻��ɺͻ��ɣ�4-��ʾ�գ�5-��ʾ�޻��ɺͿգ�6-��ʾ���ɺͿ�, 7-��ʾ����


	CButton		m_btnChkNormal;
	CButton		m_btnChkDoubt;
	CButton		m_btnChkNull;
	void	setCheckStatus(int nStatus);
	int		getCheckStatus();

	CShowPicDlg* m_pShowPicDlg;
	CAnswerShowDlg* m_pAnswerShowDlg;
	CEdit			m_edit_OmrRecogInfo;
	CString			m_strMsg;

	int			m_nCurrItemPaperList;
	pPAPERSINFO m_pPapers;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMDblclkListPapers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownListPapers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListPapers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedChkNormal();
	afx_msg void OnBnClickedChkDoubt();
	afx_msg void OnBnClickedChkNull();
};
