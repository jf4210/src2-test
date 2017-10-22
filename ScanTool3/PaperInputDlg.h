#pragma once
#include "global.h"
#include "PicShow.h"
#include "MyCodeConvert.h"
#include "FileUpLoad.h"
#include "ComboBoxExt.h"
#include "XListCtrl.h"
#include "StudentMgr.h"
#include "AnswerShowDlg.h"
// CPaperInputDlg �Ի���

class CPaperInputDlg : public CDialog
{
	DECLARE_DYNAMIC(CPaperInputDlg)

public:
	CPaperInputDlg(pMODEL pModel, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CPaperInputDlg();

// �Ի�������
	enum { IDD = IDD_PAPERINPUTDLG };

public:
	CXListCtrl	m_lPapersCtrl;		//�Ծ���б�
	CXListCtrl	m_lPaperCtrl;		//�Ծ���е��Ծ��б�
//	CXListCtrl	m_lIssuePaperCtrl;	//�����Ծ��б�
	CComboBoxExt	m_comboModel;		//ģ�������б�
	CButton		m_btnBroswer;		//�����ť
	
	CShowPicDlg* m_pShowPicDlg;
	CAnswerShowDlg* m_pAnswerShowDlg;

	CString		m_strPapersPath;	//�Ծ���ļ���·��
	int			m_nModelPicNums;	//ģ��ͼƬ������һ��ģ���ж���ͼƬ����Ӧ�����Ծ�

	CString		m_strModelName;		//���Ծ������ģ�������
	CString		m_strPapersName;	//���Ծ��������
	CString		m_strPapersDesc;	//���Ծ����������Ϣ
	
	pMODEL		m_pOldModel;				//��ģ����Ϣ���л�combox��ʱ���ܰ����ͷ�
	pMODEL		m_pModel;					//ģ����Ϣ

	int				m_nCurrItemPapers;		//�Ծ���б�ǰѡ�����
	int				m_ncomboCurrentSel;		//�����б�ǰѡ����

	int				m_nCurrItemPaperList;	//��ǰ�Ծ��б�ѡ�е���
	std::vector<std::string> m_vecCHzkzh;	//�غŵ�׼��֤�ţ���ͬһ���Ծ�ʱ���м�鵱ǰ�Ծ���������غŵ��Ծ���ֹ���ֵ�1���Ծ�������������غŵģ���ǰ���غŵ��Ծ��޷��������⣩

	void	ReShowPapers();
	void	ReInitData(pMODEL pModel);

	void	SetStatusShowInfo(CString strMsg, BOOL bWarn = FALSE);	//����״̬����ʾ����Ϣ
private:
	void	InitUI();
	void	InitCtrlPosition(); 
	void	InitParam();

	void	SeachModel();
// 	bool	UnZipModel(CString strZipPath);
// 	pMODEL	LoadModelFile(CString strModelPath);			//����ģ���ļ�

// 	int		CheckOrientation4Fix(cv::Mat& matSrc, int n);	//����ģʽ�µķ���
// 	int		CheckOrientation4Head(cv::Mat& matSrc, int n);	//ͬ��ͷģʽ�µķ���
// 	int		CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan);	//���ͼƬ����

// 	void	PaintRecognisedRect(pST_PaperInfo pPaper);		//����ʶ��ľ���
// 	void	PaintIssueRect(pST_PaperInfo pPaper);			//������ʶ��������������λ��

	COLORREF		crOldText, crOldBackground;
	void	SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//�����б������ʾ
	void	UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//ȡ���б������ʾ

	void	ShowPaperByItem(int nItem);
	void	ShowPapers(pPAPERSINFO pPapers);
	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);

	void	InitTmpSubjectBmk();
	STUDENT_LIST		m_lBmkStudent;

	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ
	pST_PaperInfo	m_pCurrentShowPaper;
	pPAPERSINFO		m_pCurrentPapers;

	CStudentMgr*	m_pStudentMgr;			//������������
	
	void	ShowRectByPoint(cv::Point pt, pST_PaperInfo pPaper);
	LRESULT RoiLBtnDown(WPARAM wParam, LPARAM lParam);		//���������µ�֪ͨ
	int		GetRectInfoByPoint(cv::Point pt, pST_PicInfo pPic, RECTINFO*& pRc);

	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);		//׼��֤��ʶ�����ʱ��֪ͨ

	void	SetFontSize(int nSize);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnBroswer();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnCbnSelchangeComboModellist();
	afx_msg void OnNMDblclkListPapers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnSave();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLvnKeydownListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNMHoverListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListPapers(NMHDR *pNMHDR, LRESULT *pResult);
};
