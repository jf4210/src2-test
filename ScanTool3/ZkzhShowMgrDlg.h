#pragma once
#include"ZkzhExceptionDlg.h"
#include "MultiPageExceptionDlg.h"


// CZkzhShowMgrDlg �Ի���

class CZkzhShowMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CZkzhShowMgrDlg)

public:
	CZkzhShowMgrDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CZkzhShowMgrDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ZKZHSHOWMGRDLG };
#endif

	CZkzhExceptionDlg*	m_pZkzhExceptionDlg;
	CMultiPageExceptionDlg*	m_pMultiPageExceptionDlg;

	void	ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL);
	//��ҳ�������ҳ��������³�ʼ������
	//��ҳ�����޸��Ծ����Ϣ�����ܵ������������е��б�����Ծ�ָ����Ϣ�ı䣬����ͨ����tab�����³�ʼ��������ҳ��
	void	ReInitDataFromChildDlg(pMODEL pModel, pPAPERSINFO pPapersInfo);
	void	SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg);
	bool	ReleaseData();
private:
	std::vector<CBmpButton*> m_vecBtn;

	pPAPERSINFO		m_pPapers;
	pMODEL			m_pModel;				//ɨ���Ծ�ʱ��У��ģ��
	CStudentMgr*	m_pStudentMgr;
	pST_PaperInfo	m_pDefShowPaper;		//Ĭ����ʾ���Ծ�
	CVagueSearchDlg* m_pVagueSearchDlg;	//ģ����������
	CShowPicDlg*	m_pShowPicDlg;		//ͼƬ��ʾ����
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	InitData();
	BOOL	PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
};
