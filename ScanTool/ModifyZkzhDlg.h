#pragma once

#include "global.h"
#include "PicShow.h"
#include "XListCtrl.h"
#include "StudentMgr.h"
// CModifyZkzhDlg �Ի���

class CModifyZkzhDlg : public CDialog
{
	DECLARE_DYNAMIC(CModifyZkzhDlg)

public:
	CModifyZkzhDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CModifyZkzhDlg();

// �Ի�������
	enum { IDD = IDD_MODIFYZKZHDLG };

	CXListCtrl		m_lcZkzh;				//��Ҫ�޸ĵ�׼��֤���б�
	CTabCtrl		m_tabPicShowCtrl;		//ͼƬ��ʾ�ؼ�

	std::vector<CPicShow*>	m_vecPicShow;	//�洢ͼƬ��ʾ����ָ�룬�ж��ģ��ͼƬʱ����Ӧ����ͬ��tab�ؼ�ҳ��
	int						m_nCurrTabSel;	//��ǰTab�ؼ�ѡ���ҳ��
	CPicShow*				m_pCurrentPicShow;		//��ǰͼƬ��ʾ�ؼ�
	int						m_nModelPicNums;		//ģ��ͼƬ������һ��ģ���ж���ͼƬ����Ӧ�����Ծ�

	pMODEL			m_pModel;				//ɨ���Ծ�ʱ��У��ģ��
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentSelItem;		//��ǰѡ�����

	CString			m_strCurZkzh;
	COLORREF		crOldText, crOldBackground;

	int				m_nSearchType;		//�������ͣ�1-������������2-��׼��֤��
	CString			m_strSearchKey;		//�����ؼ���
	CListCtrl		m_lcBmk;			//�������б�ؼ�
private:
	CStudentMgr*	m_pStudentMgr;
private:
	void	InitUI();
	void	InitTab();
	void	InitCtrlPosition();
	void	InitData();

	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);
	void	LeftRotate();
	void	RightRotate();

	void	ShowPaperByItem(int nItem);
	void	ShowPaperZkzhPosition(pST_PaperInfo pPaper);
	BOOL	PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	afx_msg LRESULT OnEditEnd(WPARAM, LPARAM);
	afx_msg LRESULT OnLBtnDownEdit(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchangeTabZkzhpic(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedRadioSearchzkzh();
	afx_msg void OnBnClickedRadioSearchname();
	afx_msg void OnBnClickedBtnSearch();
	afx_msg void OnNMDblclkListZkzhsearchresult(NMHDR *pNMHDR, LRESULT *pResult);
};
