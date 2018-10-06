#pragma once
#include "CV_picture.h"


// CPicDlg 对话框

class CPicDlg : public CDialog
{
	DECLARE_DYNAMIC(CPicDlg)

public:
	CPicDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPicDlg();

// 对话框数据
	enum { IDD = IDD_PICDLG };

public:
	CV_picture	m_picShow;//picture control 控件变量

	cv::Mat		m_src_img;
	cv::Mat		m_dst_img;

	CRect m_client;//picture control大小
	int m_picWidth;    //载入图片的宽
	int m_picHeight;   //载入图片的高

	int m_iShowWidth;   //要显示的宽
	int m_iShowHeight;   //要显示的高

	int m_iX;    //图较大时要显示的X坐标
	int m_iY;    //图较大时要显示的Y坐标
	float m_fScale;		//当前图像的缩放比

	void ShowPic(cv::Mat& imgMat, cv::Point pt = cv::Point(0, 0), float fShowPer = 1.0, int nDirection = -1);

	void ReInit();
	void SetShowTracker(bool bShowH, bool bShowV, bool bShowSN, bool bShowZgt);		//显示橡皮筋类
private:
	cv::Point	m_ptBase;
	int		m_nDirection;		//图像默认显示方向，1:针对原始图像需要进行的旋转，正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
	int		m_nRotateTimes;		//记录往左往右旋转次数
	void RotateImg2(cv::Mat& imgMat, int nDirection = 1);
	void InitCtrlPosition();

	LRESULT CvPaint(WPARAM wParam, LPARAM lParam);
	LRESULT RoiLbtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT RoiLbtnDown(WPARAM wParam, LPARAM lParam);
	LRESULT RoiRbtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT MBtnWheel(WPARAM wParam, LPARAM lParam);
	LRESULT MBtnDown(WPARAM wParam, LPARAM lParam);
	LRESULT MBtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT HTrackerChange(WPARAM wParam, LPARAM lParam);
	LRESULT VTrackerChange(WPARAM wParam, LPARAM lParam);
	LRESULT SNTrackerChange(WPARAM wParam, LPARAM lParam);
	LRESULT ZgtTrackerChange(WPARAM wParam, LPARAM lParam);

	LRESULT ShiftKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT ShiftKeyUp(WPARAM wParam, LPARAM lParam);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
