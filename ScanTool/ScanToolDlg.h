
// ScanToolDlg.h : ͷ�ļ�
//

#pragma once
#include "global.h"
#include "PicShow.h"
#include "MakeModelDlg.h"
#include "RecognizeThread.h"
#include "PaperInputDlg.h"
#include "TwainCpp.h"
#include "DIB.h"
#include "bmp2ipl.h"
#include "ScanCtrlDlg.h"
#include "PapersInfoSaveDlg.h"
#include "SendFileThread.h"
#include "TcpClient.h"
#include "ShowModelInfoDlg.h"
#include "ScanerInfoDlg.h"
#include "CompressThread.h"
#include "ScanThread.h"
#include "ModifyZkzhDlg.h"
//#include "XListCtrl.h"
// CScanToolDlg �Ի���

#define TIMER_CheckRecogComplete	200

class CScanToolDlg : public CDialogEx, public CTwain
{
// ����
public:
	CScanToolDlg(pMODEL pModel, CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SCANTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	CListCtrl		m_lcPicture;			//ͼƬ�б�ؼ�
	CListCtrl		m_lcPaper;				//��ɨ�Ծ��б�ؼ�
	CComboBox		m_comboModel;			//ģ�������б�ؼ�
	CTabCtrl		m_tabPicShowCtrl;		//ͼƬ��ʾ�ؼ�

	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ

	CShowModelInfoDlg* m_pShowModelInfoDlg;
	CScanerInfoDlg*		m_pShowScannerInfoDlg;

	pMODEL			m_pModel;				//ɨ���Ծ�ʱ��У��ģ��
	MODELLIST		m_lModel;				//�Ѿ����ص�ģ���б�

	int				m_nCurrItemPaperList;	//��ǰ�Ծ��б�ѡ�е���
	int				m_ncomboCurrentSel;		//�����б�ǰѡ����


	Poco::Thread* m_pCompressThread;
	CCompressThread* m_pCompressObj;

	Poco::Thread*	m_pRecogThread;
	std::vector<CRecognizeThread*> m_vecRecogThreadObj;
	Poco::Thread*	m_SendFileThread;
	CSendFileThread* m_pSendFileObj;
	Poco::Thread*	m_TcpCmdThread;
	CTcpClient*		m_pTcpCmdObj;

	std::vector<CPicShow*>	m_vecPicShow;	//�洢ͼƬ��ʾ����ָ�룬�ж��ģ��ͼƬʱ����Ӧ����ͬ��tab�ؼ�ҳ��
	int						m_nCurrTabSel;	//��ǰTab�ؼ�ѡ���ҳ��
	CPicShow*				m_pCurrentPicShow;		//��ǰͼƬ��ʾ�ؼ�
	int						m_nModelPicNums;		//ģ��ͼƬ������һ��ģ���ж���ͼƬ����Ӧ�����Ծ�

	int				m_nScanCount;			//�Ѿ�ɨ�����ɵ�ͼƬ��
	std::string		m_strCurrPicSavePath;	//��ǰɨ��ͼ�񱣴�λ��
	pPAPERSINFO		m_pPapersInfo;			//��ǰɨ��Ŀ�����Ϣ
	pST_PaperInfo	m_pPaper;				//��ǰɨ���ѧ����Ϣ
	pST_PaperInfo	m_pCurrentShowPaper;	//��ǰ��ʾ��ѧ����Ϣ ��m_pCurrentPicShowһ��

	int				m_nDuplex;				//��ǰɨ���ǵ���ɨ��(0)����˫��ɨ��(1)
	int				m_nScanStatus;			//��ǰɨ��״̬, 0-δɨ�裬1-ɨ���У�2-ɨ���쳣��3-ɨ�����

	std::string		m_strCmdServerIP;
	int				m_nCmdPort;

	int				m_nCurrentScanCount;	//��ǰɨ����Ҫɨ���Ծ�����

	BOOL			m_bLogin;
	CString			m_strUserName;
	CString			m_strNickName;
	CString			m_strPwd;
	CString			m_strEzs;
	int				m_nTeacherId;
	int				m_nUserId;
	BOOL			m_bF1Enable;
	BOOL			m_bF2Enable;
	BOOL			m_bLastPkgSaveOK;		//ɨ�����һ���Ծ��Ƿ񱣴�ɹ�
	bool			m_bModifySN;			//ɨ����ɺ��Ƿ�δʶ���׼��֤�Ž����˹�����
public:
	void	InitUI();
	void	InitTab();
	void	InitConfig();
	void	InitCtrlPosition();
	void	InitParam();
	void	InitFileUpLoadList();
	void	InitCompressList();

	void	InitShow(pMODEL pModel);
	void	InitScan();
	void	ReleaseScan();

	void	ReleaseUploadFileList();
	BOOL	StartGuardProcess();

	void	SetFontSize(int nSize);
	void	SetStatusShowInfo(CString strMsg, BOOL bWarn = FALSE);	//����״̬����ʾ����Ϣ

	void	SearchModel();
//	bool	UnZipModel(CString strZipPath);
//	pMODEL	LoadModelFile(CString strModelPath);			//����ģ���ļ�

	void	ShowPaperByItem(int nItem);
	void	ShowRectByPoint(cv::Point pt, pST_PaperInfo pPaper);
	LRESULT RoiLBtnDown(WPARAM wParam, LPARAM lParam);		//���������µ�֪ͨ
	LRESULT MsgRecogErr(WPARAM wParam, LPARAM lParam);
	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);		//׼��֤��ʶ�����ʱ��֪ͨ
	void	PaintRecognisedRect(pST_PaperInfo pPaper);
	int		PaintIssueRect(pST_PaperInfo pPaper);

	int		GetRectInfoByPoint(cv::Point pt, pST_PicInfo pPic, RECTINFO*& pRc);

	BOOL	ScanSrcInit();
// 	BOOL	InitTwain(HWND hWnd);
// 
// 	HWND	m_hMessageWnd;
// 	HINSTANCE m_hTwainDLL;
// 	TW_IDENTITY m_AppId;

	BOOL m_bTwainInit;

	CArray<TW_IDENTITY, TW_IDENTITY> m_scanSourceArry;
	void CopyImage(HANDLE hBitmap, TW_IMAGEINFO& info);
	void SetImage(HANDLE hBitmap, int bits);
	void ScanDone(int nStatus);

	int CheckOrientation(cv::Mat& matSrc, int n);
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnBnClickedBtnScanmodule();
	afx_msg void OnCbnSelchangeComboModel();
	afx_msg void OnBnClickedBtnInputpaper();
	afx_msg void OnTcnSelchangeTabPicshow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
	afx_msg void OnNMDblclkListPicture(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnUploadpapers();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnModelmgr();
	afx_msg void OnNMHoverListPicture(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownListPicture(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnUploadmgr();
	afx_msg void OnBnClickedBtnScanall();
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnReback();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnTimer(UINT nIDEvent);
};
