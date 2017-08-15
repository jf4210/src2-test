#pragma once

#include "global.h"
#include "PicShow.h"
#include "XListCtrl.h"
#include "StudentMgr.h"
#include "VagueSearchDlg.h"
#include "ShowPicDlg.h"
//#include "TipBaseDlg.h"
// CModifyZkzhDlg 对话框

#define	NewListModelTest	//测试

class CModifyZkzhDlg : public CDialog
{
	DECLARE_DYNAMIC(CModifyZkzhDlg)

public:
	CModifyZkzhDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModifyZkzhDlg();

// 对话框数据
	enum { IDD = IDD_MODIFYZKZHDLG };

	CXListCtrl		m_lcZkzh;				//需要修改的准考证号列表

	pMODEL			m_pModel;				//扫描试卷时的校验模板
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentSelItem;		//当前选择的项

	CString			m_strCurZkzh;
	COLORREF		crOldText, crOldBackground;
	CBmpButton		m_bmpBtnReturn;

	STUDENT_LIST	m_lBmkStudent;

	void			ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL);
private:
//	std::vector<std::string> m_vecCHzkzh;	//重号的准考证号，在同一袋试卷时进行检查当前试卷袋中所有重号的试卷（防止出现第1份试卷正常后面出现重号的，在前面重号的试卷无法检测的问题）
	CStudentMgr*	m_pStudentMgr;
	pST_PaperInfo	m_pShowPaper;		//默认显示的试卷
	CVagueSearchDlg* m_pVagueSearchDlg;	//模糊搜索窗口
	CShowPicDlg*	m_pShowPicDlg;		//图片显示窗口
private:
	void	InitUI();
	void	InitTab();
	void	InitCtrlPosition();
	void	InitData();
	bool	ReleaseData();

	bool	VagueSearch(int nItem);		//对某项进行模糊查找

	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);

	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);
	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);
	void	LeftRotate();
	void	RightRotate();
	void	PicsExchange();		//图片调换，第一页与第二页调换顺序，只有在模板图片为2页时有用
	
 	void	ShowPaperByItem(int nItem);
	void	SetZkzhStatus();	//设置准考证号的扫描状态

// 	void	ShowPaperZkzhPosition(pST_PaperInfo pPaper);
//	void	PrintRecogRect(int nPic, pST_PaperInfo pPaper, pST_PicInfo pPic, cv::Mat& matImg);		//打印所有模板上的矩形位置
	BOOL	PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	afx_msg LRESULT OnEditEnd(WPARAM, LPARAM);
	afx_msg LRESULT OnLBtnDownEdit(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMDblclkListZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnNMDblclkListZkzhsearchresult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnBack();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
