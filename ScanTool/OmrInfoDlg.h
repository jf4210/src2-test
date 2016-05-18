#pragma once
#include "CV_picture.h"
#include "global.h"
// COmrInfoDlg �Ի���

class COmrInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(COmrInfoDlg)

public:
	COmrInfoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~COmrInfoDlg();

// �Ի�������
	enum { IDD = IDD_OMRINFODLG };
public:
	CV_picture  m_picOmrShow;	//Ч��ͼ����
	cv::Mat		m_src_img;
private:
	void		InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
