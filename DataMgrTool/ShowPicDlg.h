#pragma once
#include "global.h"
#include "DMTDef.h"
#include "PicShow.h"
#include "BmpButton.h"
// CShowPicDlg 对话框

//#define MSG_SHOW_OMR_TH	(WM_USER + 0x30)	//显示题号对应的Omr框

class CShowPicDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPicDlg)

public:
	CShowPicDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowPicDlg();

// 对话框数据
	enum { IDD = IDD_SHOWPICDLG };

	void	setShowPaper(pST_PaperInfo pPaper);
	void	UpdateUI();
	void	setShowModel(int nModel);	//显示模式，1--使用tab控件显示，2--使用按钮控件代替tab分页
	void	setRotate(int nDirection);	//设置旋转方向，1:针对原始图像需要进行的旋转，正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
	void	showTmpPic(cv::Mat& matPic, cv::Point pt = cv::Point(0, 0), float fShowPer = 1.0, int nDirection = -1);
	void	showPaperOmrTh(pST_PaperInfo pPaper, int nTh);	//显示Omr题号对应的选项框
private:
	CTabCtrl		m_tabPicShowCtrl;		//图片显示控件
	int				m_nShowModel;		//显示模式，1--使用tab控件显示，2--使用按钮控件代替tab分页

	std::vector<CBmpButton*> m_vecBtn;	//m_nShowModel == 2 时有效
	std::vector<CPicShow*>	m_vecPicShow;	//存储图片显示窗口指针，有多个模板图片时，对应到不同的tab控件页面
	int						m_nCurrTabSel;	//当前Tab控件选择的页面
	CPicShow*				m_pCurrentPicShow;		//当前图片显示控件
	int						m_nModelPicNums;		//模板图片数，即一份模板有多少图片，对应多少试卷

	pST_PaperInfo			m_pCurrPaper;

	void	InitUI();
	void	InitCtrlPosition();
	void	PaintRecognisedRect(pST_PaperInfo pPaper, int nTH = 0);

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
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
