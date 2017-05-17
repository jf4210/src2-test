#pragma once
#include "global.h"

// CExamInfoDlg 对话框

class CExamInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CExamInfoDlg)

public:
	CExamInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CExamInfoDlg();

// 对话框数据
	enum { IDD = IDD_EXAMINFODLG };

	void	SetExamInfo(pEXAMINFO pExamInfo, pEXAM_SUBJECT pSubjectInfo);
	void	SetUploadPapers(int nCount);

private:
	void	InitData();
	void	InitCtrlPosition();
	void	DrawBorder(CDC *pDC);	//绘制边框线
	void	SetFontSize(int nSize);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

	pEXAM_SUBJECT _pSubjectInfo;
	pEXAMINFO	_pExamInfo;
	CString		_strExamName;		//考试名称
	CString		_strSubject;		//科目信息
	CString		_strMakeModel;		//制作模板
	CString		_strScan;			//扫描
	CString		_strUpLoadProcess;	//上传进度

	bool		_bMouseInDlg;		//鼠标在当前窗口区域
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnStnClickedStaticScan();
};
