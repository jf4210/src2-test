#pragma once
#include "CV_picture.h"


// CPicDlg �Ի���

class CPicDlg : public CDialog
{
	DECLARE_DYNAMIC(CPicDlg)

public:
	CPicDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CPicDlg();

// �Ի�������
	enum { IDD = IDD_PICDLG };

public:
	CV_picture	m_picShow;//picture control �ؼ�����

	cv::Mat		m_src_img;
	cv::Mat		m_dst_img;

	CRect m_client;//picture control��С
	int m_picWidth;    //����ͼƬ�Ŀ�
	int m_picHeight;   //����ͼƬ�ĸ�

	int m_iShowWidth;   //Ҫ��ʾ�Ŀ�
	int m_iShowHeight;   //Ҫ��ʾ�ĸ�

	int m_iX;    //ͼ�ϴ�ʱҪ��ʾ��X����
	int m_iY;    //ͼ�ϴ�ʱҪ��ʾ��Y����
	float m_fScale;		//��ǰͼ������ű�

	void ShowPic(cv::Mat& imgMat, cv::Point pt = cv::Point(0, 0), float fShowPer = 1.0, int nDirection = -1);

	void ReInit();
	void SetShowTracker(bool bShowH, bool bShowV, bool bShowSN, bool bShowZgt);		//��ʾ��Ƥ����
private:
	cv::Point	m_ptBase;
	int		m_nDirection;		//ͼ��Ĭ����ʾ����1:���ԭʼͼ����Ҫ���е���ת�����򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180
	int		m_nRotateTimes;		//��¼����������ת����
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
