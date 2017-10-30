#pragma once
#include "XListCtrl.h"
#include "DMTDef.h"

// CAnswerShowDlg �Ի���

class CAnswerShowDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAnswerShowDlg)

public:
	CAnswerShowDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAnswerShowDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANSWERSHOWDLG };
#endif


	CXListCtrl	m_lAnswerListCtrl;		//�Ծ���б�
	void	InitData(pST_PaperInfo pPaper);
	void	InitModel(pMODEL pModel);
	void	SetShowType(int nType);	//��ʾ���ͣ�0-����ʾ��1-�޻���, 2-��ʾ���ɣ�3-��ʾ�޻��ɺͻ��ɣ�4-��ʾ�գ�5-��ʾ�޻��ɺͿգ�6-��ʾ���ɺͿ�, 7-��ʾ����
private:
	void	InitCtrlPosition();
	void	InitUI();

	void	SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//�����б������ʾ
	void	UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//ȡ���б������ʾ
	COLORREF		crOldText, crOldBackground;

	CButton		m_btnChkRight;
	CButton		m_btnChkDoubt;
	CButton		m_btnChkNull;
	void	setCheckStatus(int nStatus);
	int		getCheckStatus();

	int		m_nCurrentItem;
	pMODEL	m_pModel;
	int		m_nShowType;	//��ʾ���ͣ�0-����ʾ��1-�޻���, 2-��ʾ���ɣ�3-��ʾ�޻��ɺͻ��ɣ�4-��ʾ�գ�5-��ʾ�޻��ɺͿգ�6-��ʾ���ɺͿ�, 7-��ʾ����
	pST_PaperInfo m_pShowPaper;

//	CDialog*	m_pShowPicDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMHoverListAnswer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListAnswer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedChkRight();
	afx_msg void OnBnClickedChkDoubt();
	afx_msg void OnBnClickedChkNull();
};
