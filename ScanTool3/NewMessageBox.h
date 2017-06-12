#pragma once
#include "resource.h"
#include "BmpButton.h"
#include "ctrl/TransparentStatic.h"
#include "TipBaseDlg.h"
#include <string>

// CTestDlg �Ի���

class CNewMessageBox : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CNewMessageBox)

public:
	CNewMessageBox(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CNewMessageBox();

// �Ի�������
	enum { IDD = IDD_NEWMESSAGEBOXDLG };

	int		m_nResult;
	void	setShowInfo(int nType, int nBtns, std::string strMsg);		//nType=1 ��ʾ�ޱ�������Ϣ��2-��ʾ��ģ����Ϣ
private:
	int		m_nBtn;				//��ʾ������ť
	int		m_nShowType;		//nType=1 ��ʾ�ޱ�������Ϣ��2-��ʾ��ģ����Ϣ
	CBmpButton		m_bmpBtnOK;
	CBmpButton		m_bmpBtnClose;
	CTransparentStatic	m_staticMsgShow;

	CString			m_strMsgShow;
	CBitmap			m_bmpBk;

	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	CFont			m_fontBtn;				//��ť����

	void	SetFontSize(int nSize);
	void	InitUI();
	void	InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
