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
private:
	void		InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
