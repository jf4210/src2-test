#pragma once
#include <vector>
#include "TipBaseDlg.h"
#include "ModelInfoDlg.h"
#include "BmpButton.h"

// CCreateModelDlg �Ի���
// typedef struct tagPath
// {
// 	CString strName;
// 	CString strPath;
// }MODELPATH;

class CCreateModelDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CCreateModelDlg)

public:
	CCreateModelDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCreateModelDlg();

// �Ի�������
	enum { IDD = IDD_CREATEMODELDLG };

	int		m_nSearchType;	//1-��ɨ���ǻ�ȡͼ��2-�ӱ���ѡ��ͼƬ��3-�ӷ���������ͼ��
	CString			m_strScanSavePath;
	std::vector<MODELPICPATH> m_vecPath;


	CBmpButton		m_bmpBtnClose;
	CFont	fontStatus;
	void	SetFontSize();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
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
