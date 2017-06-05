#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ctrl/skinscrollwnd.h"
#include "BmpButton.h"

// CScanRecordDlg �Ի���

class CScanBmkRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanBmkRecordDlg)

public:
	CScanBmkRecordDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanBmkRecordDlg();

// �Ի�������
	enum { IDD = IDD_SCANBMKRECORDDLG };

	void	UpDateInfo();
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);

	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ

	CBmpButton		m_bmpBtnExport;
	CXListCtrl		m_lcBmk;
	HBITMAP			m_bitmap_scrollbar;
	
	CString			m_strGmkCount;		//������������
	CString			m_strScanCount;		//��ɨ����
	CString			m_strUnScanCount;	//δɨ����
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMClickListBmk(NMHDR *pNMHDR, LRESULT *pResult);
};
