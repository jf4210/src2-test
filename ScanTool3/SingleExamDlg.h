#pragma once
#include "global.h"


// CSingleExamDlg �Ի���
typedef enum _SubBtnID_
{

};

class CSingleExamDlg : public CDialog
{
	DECLARE_DYNAMIC(CSingleExamDlg)

public:
	CSingleExamDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSingleExamDlg();

// �Ի�������
	enum { IDD = IDD_SINGLEEXAMDLG };

	int		m_nMaxSubsRow;		//һ�������ʾ�Ŀ�Ŀ��ť����
	int		m_nSubjectBtnH;		//��Ŀ��ť�߶�

	void	SetExamInfo(pEXAMINFO pExamInfo);

private:
	void	InitData();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	void	ReleaseData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()


private:
	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ

//	pEXAM_SUBJECT	_pSubjectInfo;
	pEXAMINFO		_pExamInfo;
	std::vector<CButton*> m_vecBtn;			//��̬������ť���洢��ť����
	CString		_strExamName;		//��������
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
