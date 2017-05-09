
// ScanTool2Dlg.h : ͷ�ļ�
//

#pragma once
#include "TcpClient.h"
#include "CompressThread.h"
#include "RecognizeThread.h"
#include "ExamInfoMgrDlg.h"

// CScanTool2Dlg �Ի���
class CScanTool2Dlg : public CDialogEx
{
// ����
public:
	CScanTool2Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SCANTOOL2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	void InitThreads();
	void ReleaseThreads();
	void InitCtrlPositon();
	void ReleaseData();
	void ReleaseDlg();

	void InitUI();

private:
	Poco::Thread*		m_pRecogThread;		//ʶ���̶߳�������
	std::vector<CRecognizeThread*> m_vecRecogThreadObj;		//�洢ʶ���̶߳���
	Poco::Thread*		m_TcpCmdThread;
	CTcpClient*			m_pTcpCmdObj;
	Poco::Thread*		m_pCompressThread;
	CCompressThread*	m_pCompressObj;

	//++����ָ��
	CExamInfoMgrDlg*	m_pExamInfoMgrDlg;
	//--
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
};
