#pragma once
#include "CV_picture.h"
#include "global.h"

// CSNInfoSetDlg 对话框

class CSNInfoSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CSNInfoSetDlg)

public:
	CSNInfoSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSNInfoSetDlg();

// 对话框数据
	enum { IDD = IDD_SNINFOSETDLG };
public:
	CV_picture  m_picSNShow;	//效果图像区
	cv::Mat		m_src_img;
	
	CButton		m_chkUseBarCode;
	CButton		m_radioSN_H;
	CButton		m_radioSN_V;

	CButton		m_radioADD_Z;
	CButton		m_radioADD_F;
	int			m_nCurrentSNVal;	//根据选项、方向组合构成的4位二进制的值，标识4中情况
	int			m_nZkzhType;		//准考证号识别类型: 1 - OMR，2 - 条码or二维码
	void		InitType(int nType);
	void		ShowUI(int nSnVal);
private:
	int			m_nSNSel;		//SN选择	0-横向，1-纵向
	int			m_nOptionSel;	//题号方向选择	0-从左到右，1-从右到左

	bool		m_bShowFist;

	void		InitCtrlPosition();
	void		ShowSNPic();
	void		InitUI();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedRadioSnH();
	afx_msg void OnBnClickedRadioSnV();
	afx_msg void OnBnClickedRadioOption1();
	afx_msg void OnBnClickedRadioOption2();
	afx_msg void OnBnClickedCheckUseBarcode();
};
