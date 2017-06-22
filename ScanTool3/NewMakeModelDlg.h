#pragma once
#include "global.h"
#include "BmpButton.h"
#include "ComboBoxExt.h"
#include "ScanModelPaperDlg.h"
#include "NewModelDlg.h"

#include "MakeModelDlg.h"
// CMakeModelDlg �Ի���

class CNewMakeModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewMakeModelDlg)

public:
	CNewMakeModelDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CNewMakeModelDlg();

// �Ի�������
	enum { IDD = IDD_NEWMAKEMODELDLG };

public:
	CScanThread		m_scanThread;
	std::vector<CString> m_vecScanSrc;
	pTW_IDENTITY GetScanSrc(int nIndex);
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	void	InitExamData();
	pMODEL  LoadSubjectModel(pEXAM_SUBJECT pSubModel);

	//----------------------
	TwainApp*		_pTWAINApp;
//	std::vector<CString> m_vecScanSrc;
	void	InitScanner();

	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);
	//----------------------
	CString			m_strScanPicPath;		//ɨ����ɨ���ͼ��·������Ҫ���Ͽ�Ŀ����

	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ

	CBmpButton		m_bmpBtnScan;
	CBmpButton		m_bmpBtnUpload;
	CBmpButton		m_bmpBtnSave;
	CBmpButton		m_bmpBtnDown;
	CComboBoxExt	m_comboSubject;		//CComboBoxExt

	CRect			m_rtChild;
	CNewModelDlg*	m_pNewModelDlg;
	CMakeModelDlg*	m_pMakeModelDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnScanpaper();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnNewmakemodel();
};
