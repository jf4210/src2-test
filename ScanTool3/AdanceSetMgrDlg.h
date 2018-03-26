#pragma once
#include "BmpButton.h"
#include "TipBaseDlg.h"
#include "ScanParamSetDlg.h"
#include "RecogParamSetDlg.h"
#include "SystemSetDlg.h"

class AdvanceParam
{
public:
	AdvanceParam()
	{
		nCurrentZkzhSensitivity = 2;
		nCurrentOmrSensitivity = 5;
		nDefZkzhSensitivity = 2;
		nDefOmrSensitivity = 5;
		nDefFixThreshold = 150;
		nDefGrayThreshold = 150;
		nDefQkWjThreshold = 150;
		nDefZkzhThreshold = 180;
		nDefOmrThreshold = 180;
		nDefCharacterConfidence = 60;
		nDefPersentFix = 80;
		nDefPersentGray = 70;
		nDefPersentQkWj = 70;
		nDefPersentZkzh = 100;
		nDefPersentOmr = 100;

		nChkLostCorner = 0;
	}
public:
	int nCurrentZkzhSensitivity;
	int nCurrentOmrSensitivity;
	int nDefZkzhSensitivity;
	int nDefOmrSensitivity;

	int nCurrentFixThreshold;
	int nCurrentGrayThreshold;
	int nCurrentQkWjThreshold;
	int nCurrentZkzhThreshold;
	int nCurrentOmrThreshold;
	int nDefFixThreshold;
	int nDefGrayThreshold;
	int nDefQkWjThreshold;
	int nDefZkzhThreshold;
	int nDefOmrThreshold;

	int nPersentFix;
	int nPersentGray;
	int nPersentQkWj;
	int nPersentZkzh;
	int nPersentOmr;
	int nDefPersentFix;
	int nDefPersentGray;
	int nDefPersentQkWj;
	int nDefPersentZkzh;
	int nDefPersentOmr;

	int nCharacterAnchorPoint;		//���ֶ������
	int nDefCharacterAnchorPoint;	//Ĭ�����ֶ������
	int nCharacterConfidence;		//����ʶ��Ҫ������ķ�ֵ
	int nDefCharacterConfidence;	//Ĭ������ʶ�����ķ�ֵ

	int			nScanDpi;			//ɨ��DPI
	int			nScanPaperSize;		//1:A4, 2:A3, 3:����
	int			nScanType;			//1:�Ҷ�ɨ��, 2: ��ɫɨ��
	int			nAutoCut;			//ɨ�裺�Ƿ��Զ��ü�

	int			nUseWordAnchorPoint;//ʹ��������Ϊ����
	int			nUsePagination;		//ʹ��ҳ�룬��Զ�ҳ�Ծ�����

	int	nChkLostCorner;		//ȱ�Ǽ��
};

class CAdanceSetMgrDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CAdanceSetMgrDlg)

public:
	CAdanceSetMgrDlg(pMODEL	pModel, AdvanceParam stSensitiveParam, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAdanceSetMgrDlg();

// �Ի�������
	enum { IDD = IDD_ADANCESETMGRDLG };

	void	InitUI();
	void	InitCtrlPosition();

	CTabCtrl		m_tabParamMgr;
	CBmpButton		m_bmpBtnClose;
	std::vector<CBaseTabDlg*>	m_vecTabDlg;
	int				m_nCurrTabSel;

	AdvanceParam _stSensitiveParam;
private:
	pMODEL		m_pModel;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
	afx_msg void OnTcnSelchangeTabParam(NMHDR *pNMHDR, LRESULT *pResult);
};
