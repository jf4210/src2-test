#pragma once
#include "global.h"
#include "DMTDef.h"
#include "PicShow.h"
#include "BmpButton.h"
// CShowPicDlg �Ի���

//#define MSG_SHOW_OMR_TH	(WM_USER + 0x30)	//��ʾ��Ŷ�Ӧ��Omr��

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
	void	setShowModel(int nModel);	//��ʾģʽ��1--ʹ��tab�ؼ���ʾ��2--ʹ�ð�ť�ؼ�����tab��ҳ
	void	setRotate(int nDirection);	//������ת����1:���ԭʼͼ����Ҫ���е���ת�����򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180
	void	showTmpPic(cv::Mat& matPic, cv::Point pt = cv::Point(0, 0), float fShowPer = 1.0, int nDirection = -1);
	void	showPaperOmrTh(pST_PaperInfo pPaper, int nTh);	//��ʾOmr��Ŷ�Ӧ��ѡ���
private:
	CTabCtrl		m_tabPicShowCtrl;		//ͼƬ��ʾ�ؼ�
	int				m_nShowModel;		//��ʾģʽ��1--ʹ��tab�ؼ���ʾ��2--ʹ�ð�ť�ؼ�����tab��ҳ

	std::vector<CBmpButton*> m_vecBtn;	//m_nShowModel == 2 ʱ��Ч
	std::vector<CPicShow*>	m_vecPicShow;	//�洢ͼƬ��ʾ����ָ�룬�ж��ģ��ͼƬʱ����Ӧ����ͬ��tab�ؼ�ҳ��
	int						m_nCurrTabSel;	//��ǰTab�ؼ�ѡ���ҳ��
	CPicShow*				m_pCurrentPicShow;		//��ǰͼƬ��ʾ�ؼ�
	int						m_nModelPicNums;		//ģ��ͼƬ������һ��ģ���ж���ͼƬ����Ӧ�����Ծ�

	pST_PaperInfo			m_pCurrPaper;

	void	InitUI();
	void	InitCtrlPosition();
	void	PaintRecognisedRect(pST_PaperInfo pPaper, int nTH = 0);

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
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
