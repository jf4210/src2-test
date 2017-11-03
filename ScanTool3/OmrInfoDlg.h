#pragma once
#include "CV_picture.h"
#include "global.h"
// COmrInfoDlg 对话框

class COmrInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(COmrInfoDlg)

public:
	COmrInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~COmrInfoDlg();

// 对话框数据
	enum { IDD = IDD_OMRINFODLG };
public:
	CV_picture  m_picOmrShow;	//效果图像区
	cv::Mat		m_src_img;

	CButton		m_radioTX_S;
	CButton		m_radioTX_M;
	CButton		m_radioTX_P;	//判断题

	CButton		m_radioTH_H;
	CButton		m_radioTH_V;

	CButton		m_radioXX_H;
	CButton		m_radioXX_V;

	CButton		m_radioDirectZX;
	CButton		m_radioDirectFX;
	
	int			m_nSingle;		//题型，0-单选，1-多选，2-判断
	int			m_nCurrentOmrVal;	//根据题号、选项、方向组合构成的6位二进制的值，标识8中情况		//显示的假坐标下的值
	void		ShowUI(int nOmrVal, int nSingleFlag);
private:
	int			m_nTHSel;		//题号选择	0-横向，1-纵向
	int			m_nXXSel;		//选项选择	0-正序，1-逆序
	int			m_nDirectSel;	//方向选择	0-横向，1-纵向

	bool		m_bShowFist;

	void		InitCtrlPosition();
	void		ShowOmrPic();
	void		InitUI();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedRadioThH();
	afx_msg void OnBnClickedRadioThV();
	afx_msg void OnBnClickedRadioXxH();
	afx_msg void OnBnClickedRadioXxV();
	afx_msg void OnBnClickedRadioDirectZx();
	afx_msg void OnBnClickedRadioDirectFx();
	afx_msg void OnBnClickedRadioTxSingle();
	afx_msg void OnBnClickedRadioTxMulti();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnStnClickedStaticThH();
	afx_msg void OnStnClickedStaticThV();
	afx_msg void OnStnClickedStaticXxH();
	afx_msg void OnStnClickedStaticXxV();
	afx_msg void OnStnClickedStaticDirectZx();
	afx_msg void OnStnClickedStaticDirectFx();
	afx_msg void OnStnClickedStaticTxSingle();
	afx_msg void OnStnClickedStaticTxMulti();
	afx_msg void OnStnClickedStaticTxPd();
	afx_msg void OnBnClickedRadioTxPd();
};
