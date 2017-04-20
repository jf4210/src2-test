#pragma once
#include "ShowModelInfoDlg.h"
#include "MakeModelDlg.h"
// CScanModleMgrDlg 对话框

typedef struct _ModelFile
{
	std::string strModelName;		//gb2312
	std::string strModifyTime;
}ST_MODELFILE;

class CScanModleMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanModleMgrDlg)

public:
	CScanModleMgrDlg(pMODEL pModel, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanModleMgrDlg();

// 对话框数据
	enum { IDD = IDD_SCANMODLEMGRDLG };

public:
	pMODEL				m_pModel;
private:
	CShowModelInfoDlg*	m_pShowModelInfoDlg;
	CListCtrl			m_ModelListCtrl;
	pMODEL				m_pOldModel;
	std::vector<pMODEL>	m_vecModel;
	std::list<ST_MODELFILE> m_lModelFile;	//模板文件列表，按修改时间排序
	int					m_nCurModelItem;
	CString				m_strCurModelName;
	CToolTipCtrl		m_ListTip;
	CMakeModelDlg*		m_pMakeModelDlg;
private:
	int	 GetBmkInfo();
	void InitUI();
	void InitCtrlPosition();
	void setUploadModelInfo(std::string& strName, CString& strModelPath, int nExamId, int nSubjectId, std::string& strElectOmrInfo);					//设置上传模板的信息
	BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnBnClickedBtnDlmodel();
	afx_msg void OnNMDblclkListModel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnDelmodel();
	afx_msg void OnBnClickedBtnAddmodel();
	afx_msg void OnNMHoverListModel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnMakemodel();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBtnuploadmodel();
	afx_msg void OnLvnHotTrackListModel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
