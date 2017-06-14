
// ScanTool2Dlg.h : ͷ�ļ�
//

#pragma once
#include "TcpClient.h"
#include "CompressThread.h"
#include "RecognizeThread.h"
#include "SendFileThread.h"
#include "ExamInfoMgrDlg.h"
#include "ScanMgrDlg.h"
#include "ScanThread.h"
#include "BmpButton.h"
#include "ModifyZkzhDlg.h"

// CScanTool2Dlg �Ի���
class CScanTool3Dlg : public CDialogEx
{
// ����
public:
	CScanTool3Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SCANTOOL2_DIALOG };


	void	SwitchDlg(int nDlg, int nChildID = 1);		//�л�������ʾ
	void	SwitchModifyZkzkDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL);
	bool	HandleModel();				//����ģ�壬ͨ���Ӵ��ڴ���ģ��
	void	DumpReleaseTwain();			//��ϵͳ����ʱ�ͷ�twain��Դ
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	void InitThreads();
	void ReleaseThreads();
	void InitCtrlPositon();
	void ReleaseData();
	void ReleaseDlg();

	void InitUI();

	LRESULT	MsgCmdDlModel(WPARAM wParam, LPARAM lParam);
	LRESULT MsgCmdGetBmk(WPARAM wParam, LPARAM lParam);
private:
	Poco::Thread*		m_pRecogThread;		//ʶ���̶߳�������
	std::vector<CRecognizeThread*> m_vecRecogThreadObj;		//�洢ʶ���̶߳���
	Poco::Thread*		m_SendFileThread;
	CSendFileThread*	m_pSendFileObj;
	Poco::Thread*		m_TcpCmdThread;
	CTcpClient*			m_pTcpCmdObj;
	Poco::Thread*		m_pCompressThread;
	CCompressThread*	m_pCompressObj;
// 	Poco::Thread*		m_pScanThread;
// 	CScanThread*		m_pScanThreadObj;
//	CScanThread			m_scanThread;

	//++����ָ��
	CExamInfoMgrDlg*	m_pExamInfoMgrDlg;
	CScanMgrDlg*		m_pScanMgrDlg;
	CModifyZkzhDlg*		m_pModifyZkzhDlg;
	//--


	CBitmap		m_bmpBkg;
	CBitmap		m_bmpTitle;
	CBmpButton	m_bmpBtnMin;
	CBmpButton	m_bmpBtnExit;
	CBmpButton	m_bmpBtnUserPic;	//�û�ͷ��ť

	CString		m_strTitle;
	CString		m_strVersion;
	CString		m_strUserName;		//��ʾ���û���
	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontTitle;			//��������
	CFont			m_fontVersion;			//�汾����
	CFont			m_fontUserName;			//�û�������
	COLORREF		m_colorStatus;			//״̬��������ɫ
	void	SetFontSize(int nSize);
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnMin();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
