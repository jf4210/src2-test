#pragma once
#include <vector>
#include "global.h"

// CModelInfoDlg �Ի���
// typedef struct tagModelPicPath
// {
// 	CString strName;
// 	CString strPath;
// }MODELPICPATH;

class CModelInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CModelInfoDlg)

public:
	CModelInfoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CModelInfoDlg();

// �Ի�������
	enum { IDD = IDD_MODELINFODLG };

public:
	int			m_nPaperNum;
	bool		m_bABPaperModel;		//AB��ģʽ
	bool		m_bHasHead;				//�Ƿ���ͬ��ͷ
	std::vector<MODELPICPATH> m_vecPath;

	CListCtrl	m_listPath;
private:
	int			m_nCurrentItem;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedRadioNotabpaper();
	afx_msg void OnBnClickedRadioAbpaper();
	afx_msg void OnBnClickedRadioNohead();
	afx_msg void OnBnClickedRadioHashead();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnNMClickListPic(NMHDR *pNMHDR, LRESULT *pResult);
};
