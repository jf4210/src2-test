#pragma once
#include "global.h"

// CMultiPlatform4TYDlg �Ի���

class CMultiPlatform4TYDlg : public CDialog
{
	DECLARE_DYNAMIC(CMultiPlatform4TYDlg)

public:
	CMultiPlatform4TYDlg(VEC_PLATFORM_TY& vecPlatform, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMultiPlatform4TYDlg();

// �Ի�������
	enum { IDD = IDD_MULTIPLATFORM4TYDLG };

public:
	CListCtrl		m_lcPlatform;
	int				m_nStatusSize;			//�����С
	CFont			m_fontStatus;			//����
	COLORREF		m_colorStatus;			//������ɫ

	void			SetFontSize(int nSize);

	void	InitData();

	pST_PLATFORMINFO	_pPlatform;
	VEC_PLATFORM_TY&	_vecPlatform;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMDblclkListPlatform(NMHDR *pNMHDR, LRESULT *pResult);
};
