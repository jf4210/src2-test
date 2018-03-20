#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ScanReminderDlg.h"
#include "ShowPicDlg.h"
#include "BmpButton.h"
#include "ctrl/skinscrollwnd.h"
#include "StudentMgr.h"
//#include "ModifyZkzhDlg.h"

// CScanProcessDlg �Ի���

class CScanProcessDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanProcessDlg)

public:
	CScanProcessDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanProcessDlg();

// �Ի�������
	enum { IDD = IDD_SCANPROCESSDLG };

	void	AddPaper(int nID, pST_PaperInfo pPaper);
	void	ResetPicList();
	void	InitShow();
	void	UpdateChildInfo(bool bScanDone = false);
	void	ScanCompleted();
	void	SetStatusShow(int nType, CString strShowInfo, bool bWarn = false);	//1--ɨ��״̬��Ϣ��2--�����Ծ����Ϣ
	void	UpdateExamBmk();		//����ǰ�Ծ����ɨ����Ϣ���µ����Ե��ܱ�������
	void	ReShowCurrPapers();
	void	ShowSinglePic(cv::Mat& matPic);
	void	InitTmpSubjectBmk();		//��ʼ����ʱ������
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	EnableBtn(BOOL bEnable);
	bool	WriteJsonFile();
	void	SetFontSize();

	void	ShowPapers(pPAPERSINFO pPapers);
	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);

	void TestData(bool bReset);	//�������ݣ�����ɾ

	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);		//׼��֤��ʶ�����ʱ��֪ͨ
private:
	CXListCtrl	m_lcPicture;	//CXListCtrl
	CRect		m_rtChildDlg;	//�ӿؼ�λ��
	CScanReminderDlg*	m_pReminderDlg;	//ɨ����ʾ�򴰿�
	CShowPicDlg*		m_pShowPicDlg;
	CStudentMgr*		m_pStudentMgr;
//	CModifyZkzhDlg*		m_pModifyZkzhDlg;

	STUDENT_LIST		m_lBmkStudent;

	CFont			m_fontBtn1;				//��ť����
	CFont			m_fontBtn2;
	CBmpButton		m_bmpBtnScanProcess;
	CBmpButton		m_bmpBtnScanAgain;
	CBmpButton		m_bmpBtnSave;
	HBITMAP			m_bitmap_scrollbar;

	int				m_nCurrentScanCount;	//��ǰɨ����Ҫɨ���Ծ�����
	std::string		m_strCurrPicSavePath;		//gb2312
	std::vector<std::string> m_vecCHzkzh;	//�غŵ�׼��֤�ţ���ͬһ���Ծ�ʱ���м�鵱ǰ�Ծ���������غŵ��Ծ���ֹ���ֵ�1���Ծ�������������غŵģ���ǰ���غŵ��Ծ��޷��������⣩
	//std::vector<std::string> m_vecKC;		//��¼��ǰ�Ծ���������Ծ�Ŀ����б������Ƕ���������Ծ�һ��ɨ��
	std::set<std::string> m_setKC;			//��¼��ǰ�Ծ���������Ծ�Ŀ����б������Ƕ���������Ծ�һ��ɨ��
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnScanagain();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnDestroy();
	afx_msg void OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnScanprocess();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
