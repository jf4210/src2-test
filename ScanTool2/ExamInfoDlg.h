#pragma once
#include "global.h"

// CExamInfoDlg �Ի���

class CExamInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CExamInfoDlg)

public:
	CExamInfoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CExamInfoDlg();

// �Ի�������
	enum { IDD = IDD_EXAMINFODLG };

	void	SetExamInfo(pEXAMINFO pExamInfo, pEXAM_SUBJECT pSubjectInfo);
	void	SetUploadPapers(int nCount);

private:
	void	InitData();
	void	InitCtrlPosition();
	void	DrawBorder(CDC *pDC);	//���Ʊ߿���
	void	SetFontSize(int nSize);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ

	pEXAM_SUBJECT _pSubjectInfo;
	pEXAMINFO	_pExamInfo;
	CString		_strExamName;		//��������
	CString		_strSubject;		//��Ŀ��Ϣ
	CString		_strMakeModel;		//����ģ��
	CString		_strScan;			//ɨ��
	CString		_strUpLoadProcess;	//�ϴ�����

	bool		_bMouseInDlg;		//����ڵ�ǰ��������
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnStnClickedStaticScan();
};
