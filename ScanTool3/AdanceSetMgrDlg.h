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

	int nCharacterAnchorPoint;		//文字定点个数
	int nDefCharacterAnchorPoint;	//默认文字定点个数
	int nCharacterConfidence;		//文字识别要求的信心阀值
	int nDefCharacterConfidence;	//默认文字识别信心阀值

	int			nScanDpi;			//扫描DPI
	int			nScanPaperSize;		//1:A4, 2:A3, 3:定制
	int			nScanType;			//1:灰度扫描, 2: 彩色扫描
	int			nAutoCut;			//扫描：是否自动裁剪

	int			nUseWordAnchorPoint;//使用文字作为定点
	int			nUsePagination;		//使用页码，针对多页试卷的情况

	int	nChkLostCorner;		//缺角检测
};

class CAdanceSetMgrDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CAdanceSetMgrDlg)

public:
	CAdanceSetMgrDlg(pMODEL	pModel, AdvanceParam stSensitiveParam, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAdanceSetMgrDlg();

// 对话框数据
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
	afx_msg void OnTcnSelchangeTabParam(NMHDR *pNMHDR, LRESULT *pResult);
};
