#pragma once
#include "global.h"
#include "PicShow.h"

// CShowPicDlg �Ի���

class CShowPicDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPicDlg)

public:
	CShowPicDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShowPicDlg();

// �Ի�������
	enum { IDD = IDD_SHOWPICDLG };

	void	setShowPaper(pST_PaperInfo pPaper);
	void	UpdateUI();
private:
	CTabCtrl		m_tabPicShowCtrl;		//ͼƬ��ʾ�ؼ�

	std::vector<CPicShow*>	m_vecPicShow;	//�洢ͼƬ��ʾ����ָ�룬�ж��ģ��ͼƬʱ����Ӧ����ͬ��tab�ؼ�ҳ��
	int						m_nCurrTabSel;	//��ǰTab�ؼ�ѡ���ҳ��
	CPicShow*				m_pCurrentPicShow;		//��ǰͼƬ��ʾ�ؼ�
	int						m_nModelPicNums;		//ģ��ͼƬ������һ��ģ���ж���ͼƬ����Ӧ�����Ծ�

	pST_PaperInfo			m_pCurrPaper;

	void	InitUI();
	void	InitCtrlPosition();
	void	PaintRecognisedRect(pST_PaperInfo pPaper);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchangeTabPicshow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
};
