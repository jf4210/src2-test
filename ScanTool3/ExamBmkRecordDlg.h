#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ctrl/skinscrollwnd.h"
#include "BmpButton.h"
#include "ComboBoxExt.h"

// CScanRecordDlg �Ի���

class CExamBmkRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CExamBmkRecordDlg)

public:
	CExamBmkRecordDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CExamBmkRecordDlg();

// �Ի�������
	enum { IDD = IDD_EXAMBMKRECORDDLG };

	void	ResetBmkList();
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);

	void	GetBmkSearchResult();

	CBmpButton		m_bmpBtnExport;
	CXListCtrl		m_lcBmk;
	HBITMAP			m_bitmap_scrollbar;
	
	CComboBoxExt	m_comboSubject;		//��Ŀ�����б�
	CComboBoxExt	m_comboScanStatus;	//ɨ��״̬����

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangeComboBmkSubject();
	afx_msg void OnCbnSelchangeComboBmkScanstatus();
	afx_msg void OnBnClickedBtnExambmkExportscan();
};
