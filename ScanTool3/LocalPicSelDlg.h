#pragma once
#include <vector>
#include "TipBaseDlg.h"
#include "ModelInfoDlg.h"

// CLocalPicSelDlg 对话框

class CLocalPicSelDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CLocalPicSelDlg)

public:
	CLocalPicSelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLocalPicSelDlg();

// 对话框数据
	enum { IDD = IDD_LOCALPICSELDLG };

	std::vector<MODELPATH> m_vecPath;

	CListCtrl	m_listPath;
private:
	int			m_nCurrentItem;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClickListPic(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnBnClickedBtnOk();
};
