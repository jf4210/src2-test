#pragma once
#include <vector>
#include "TipBaseDlg.h"
#include "ModelInfoDlg.h"
#include "BmpButton.h"

// CCreateModelDlg 对话框
// typedef struct tagPath
// {
// 	CString strName;
// 	CString strPath;
// }MODELPATH;

class CCreateModelDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CCreateModelDlg)

public:
	CCreateModelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCreateModelDlg();

// 对话框数据
	enum { IDD = IDD_CREATEMODELDLG };

	int		m_nSearchType;	//1-从扫描仪获取图像，2-从本地选择图片，3-从服务器下载图像
	CString			m_strScanSavePath;
	std::vector<MODELPICPATH> m_vecPath;


	CBmpButton		m_bmpBtnClose;
	CFont	fontStatus;
	void	SetFontSize();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedRadioFromscanner();
	afx_msg void OnBnClickedRadioFromlocalfile();
	afx_msg void OnBnClickedRadioFromserver();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStaticFromserver();
	afx_msg void OnStnClickedStaticFromscanner();
	afx_msg void OnStnClickedStaticFromlocalfile();
	afx_msg void OnBnClickedBtnClose();
};
