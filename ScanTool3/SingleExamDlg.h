#pragma once
#include "global.h"


// CSingleExamDlg 对话框
typedef enum _SubBtnID_
{

};

class CSingleExamDlg : public CDialog
{
	DECLARE_DYNAMIC(CSingleExamDlg)

public:
	CSingleExamDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSingleExamDlg();

// 对话框数据
	enum { IDD = IDD_SINGLEEXAMDLG };

	int		m_nMaxSubsRow;		//一行最多显示的科目按钮数量
	int		m_nSubjectBtnH;		//科目按钮高度

	void	SetExamInfo(pEXAMINFO pExamInfo);

private:
	void	InitData();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	void	ReleaseData();
	void	DrawBorder(CDC *pDC);	//绘制边框线
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()


private:
	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

//	pEXAM_SUBJECT	_pSubjectInfo;
	pEXAMINFO		_pExamInfo;
	std::vector<CButton*> m_vecBtn;			//动态创建按钮，存储按钮对象
	CString		_strExamName;		//考试名称
	CString		_strExamTime;		//考试时间
	CString		_strExamType;
	CString		_strExamGrade;


	bool		_bMouseInDlg;		//鼠标在当前窗口区域
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnScanprocesses();
	afx_msg void OnBnClickedBtnMakescanmodel();
};
