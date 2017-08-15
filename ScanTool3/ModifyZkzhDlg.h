#pragma once

#include "global.h"
#include "PicShow.h"
#include "XListCtrl.h"
#include "StudentMgr.h"
#include "VagueSearchDlg.h"
#include "ShowPicDlg.h"
//#include "TipBaseDlg.h"
// CModifyZkzhDlg �Ի���

#define	NewListModelTest	//����

class CModifyZkzhDlg : public CDialog
{
	DECLARE_DYNAMIC(CModifyZkzhDlg)

public:
	CModifyZkzhDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CModifyZkzhDlg();

// �Ի�������
	enum { IDD = IDD_MODIFYZKZHDLG };

	CXListCtrl		m_lcZkzh;				//��Ҫ�޸ĵ�׼��֤���б�

	pMODEL			m_pModel;				//ɨ���Ծ�ʱ��У��ģ��
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentSelItem;		//��ǰѡ�����

	CString			m_strCurZkzh;
	COLORREF		crOldText, crOldBackground;
	CBmpButton		m_bmpBtnReturn;

	STUDENT_LIST	m_lBmkStudent;

	void			ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL);
private:
//	std::vector<std::string> m_vecCHzkzh;	//�غŵ�׼��֤�ţ���ͬһ���Ծ�ʱ���м�鵱ǰ�Ծ���������غŵ��Ծ���ֹ���ֵ�1���Ծ�������������غŵģ���ǰ���غŵ��Ծ��޷��������⣩
	CStudentMgr*	m_pStudentMgr;
	pST_PaperInfo	m_pShowPaper;		//Ĭ����ʾ���Ծ�
	CVagueSearchDlg* m_pVagueSearchDlg;	//ģ����������
	CShowPicDlg*	m_pShowPicDlg;		//ͼƬ��ʾ����
private:
	void	InitUI();
	void	InitTab();
	void	InitCtrlPosition();
	void	InitData();
	bool	ReleaseData();

	bool	VagueSearch(int nItem);		//��ĳ�����ģ������

	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);

	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);
	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);
	void	LeftRotate();
	void	RightRotate();
	void	PicsExchange();		//ͼƬ��������һҳ��ڶ�ҳ����˳��ֻ����ģ��ͼƬΪ2ҳʱ����
	
 	void	ShowPaperByItem(int nItem);
	void	SetZkzhStatus();	//����׼��֤�ŵ�ɨ��״̬

// 	void	ShowPaperZkzhPosition(pST_PaperInfo pPaper);
//	void	PrintRecogRect(int nPic, pST_PaperInfo pPaper, pST_PicInfo pPic, cv::Mat& matImg);		//��ӡ����ģ���ϵľ���λ��
	BOOL	PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
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
