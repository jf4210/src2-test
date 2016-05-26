// GetModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanToolDlg.h"
#include "GetModelDlg.h"
#include "afxdialogex.h"
//#include "Net_Cmd_Protocol.h"

// CGetModelDlg 对话框

IMPLEMENT_DYNAMIC(CGetModelDlg, CDialog)

CGetModelDlg::CGetModelDlg(CString strIP, int nPort, CWnd* pParent /*=NULL*/)
: CDialog(CGetModelDlg::IDD, pParent)
, m_strExamTypeName(_T("")), m_strGradeName(_T("")), m_SubjectID(0), m_nExamID(0), m_strScanModelName(_T(""))
, m_strServerIP(strIP)
, m_nServerPort(nPort)
, m_nRecvLen(0)
, m_nWantLen(0)
{

}

CGetModelDlg::~CGetModelDlg()
{
}

void CGetModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ExamID, m_nExamID);
	DDX_Text(pDX, IDC_EDIT_ExamTypeName, m_strExamTypeName);
	DDX_Text(pDX, IDC_EDIT_GradeName, m_strGradeName);
	DDX_Text(pDX, IDC_EDIT_SubjectID, m_SubjectID);
	DDX_Text(pDX, IDC_EDIT_ScanModelName, m_strScanModelName);
	DDX_Control(pDX, IDC_COMBO_ExamName, m_comboExamName);
	DDX_Control(pDX, IDC_COMBO_SubjectName, m_comboSubject);
	DDX_Control(pDX, IDC_PROGRESS_DOWN, m_progress);
}


BEGIN_MESSAGE_MAP(CGetModelDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_ExamName, &CGetModelDlg::OnCbnSelchangeComboExamname)
	ON_CBN_SELCHANGE(IDC_COMBO_SubjectName, &CGetModelDlg::OnCbnSelchangeComboSubjectname)
	ON_BN_CLICKED(IDC_BTN_DOWN, &CGetModelDlg::OnBnClickedBtnDown)
END_MESSAGE_MAP()

BOOL CGetModelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;

	if (g_lExamList.size() == 0)
		return TRUE;

	EXAM_LIST::iterator itExam = g_lExamList.begin();
	for (; itExam != g_lExamList.end(); itExam++)
	{
		CString strName = A2T(itExam->strExamName.c_str());
		m_comboExamName.AddString(strName);
		int nCount = m_comboExamName.GetCount();
		m_comboExamName.SetItemData(nCount - 1, (DWORD_PTR)&(*itExam));
	}
	m_comboExamName.SetCurSel(0);

	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemData(0);
	if (pExamInfo)
	{
		m_comboSubject.ResetContent();
		SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
		for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
		{
			EXAM_SUBJECT* pSubject = &(*itSub);
			CString strSubjectName = A2T(itSub->strSubjName.c_str());
			m_comboSubject.AddString(strSubjectName);
			int nCount = m_comboSubject.GetCount();
			m_comboSubject.SetItemData(nCount - 1, (DWORD_PTR)pSubject);

			if (i == 0)
			{
				m_SubjectID = itSub->nSubjID;
				m_strScanModelName = A2T(itSub->strModelName.c_str());
			}
		}
		m_comboSubject.SetCurSel(0);

		m_nExamID = pExamInfo->nExamID;
		m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
		m_strGradeName = pExamInfo->strGradeName.c_str();
	}

	UpdateData(FALSE);
	return TRUE;
}


// CGetModelDlg 消息处理程序


void CGetModelDlg::OnCbnSelchangeComboExamname()
{
	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemData(n);
	if (!pExamInfo)
		return;

	USES_CONVERSION;
	m_comboSubject.ResetContent();
	SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
	for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
	{
		EXAM_SUBJECT* pSubject = &(*itSub);
		CString strSubjectName = A2T(itSub->strSubjName.c_str());
		m_comboSubject.AddString(strSubjectName);
		int nCount = m_comboSubject.GetCount();
		m_comboSubject.SetItemData(nCount - 1, (DWORD_PTR)pSubject);

		if (i == 0)
		{
			m_SubjectID = itSub->nSubjID;
		}
	}
	m_comboSubject.SetCurSel(0);

	m_nExamID = pExamInfo->nExamID;
	m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
	m_strGradeName = pExamInfo->strGradeName.c_str();
	UpdateData(FALSE);
}


void CGetModelDlg::OnCbnSelchangeComboSubjectname()
{
	int n2 = m_comboSubject.GetCurSel();

	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemData(n);
	if (!pExamInfo)
		return;

	SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
	for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
	{
		if (i == n2)
		{
			m_SubjectID = itSub->nSubjID;
		}
	}

	UpdateData(FALSE);
}

void CGetModelDlg::OnBnClickedBtnDown()
{
	USES_CONVERSION;
	Poco::Net::SocketAddress sa(T2A(m_strServerIP), m_nServerPort);
	m_ss.close();

	try
	{
		Poco::Timespan ts(5, 0);
		m_ss.connect(sa);
		m_ss.setReceiveTimeout(ts);

		CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();

		//先查本地列表，如果没有则请求，如果有，计算crc，和服务器不同则下载
		USES_CONVERSION;
		CString modelPath = g_strCurrentPath + _T("Model");
		modelPath = modelPath + _T("\\") + m_strScanModelName;
		std::string strModelPath = T2A(modelPath);

#if 1	//test data
		m_nExamID = 11;
		m_SubjectID = 12;
		m_strScanModelName = _T("11_12.mod");
#endif

		ST_DOWN_MODEL stModelInfo;
		ZeroMemory(&stModelInfo, sizeof(ST_DOWN_MODEL));
		stModelInfo.nExamID = m_nExamID;
		stModelInfo.nSubjectID = m_SubjectID;
		sprintf_s(stModelInfo.szUserNo, "%s", T2A(pDlg->m_strUserName));
		sprintf_s(stModelInfo.szModelName, "%s", T2A(m_strScanModelName));

		Poco::File fileModel(strModelPath);
		if (fileModel.exists())
		{
			std::string strMd5 = calcFileMd5(strModelPath);
			strncpy(stModelInfo.szMD5, strMd5.c_str(), strMd5.length());
		}


		ST_CMD_HEADER stHead;
		stHead.usCmd = USER_NEED_DOWN_MODEL;
		stHead.uPackSize = sizeof(ST_DOWN_MODEL);
		

		char szSendBuf[1024] = { 0 };
		memcpy(szSendBuf, (char*)&stHead, HEAD_SIZE);
		memcpy(szSendBuf + HEAD_SIZE, (char*)&stModelInfo, sizeof(ST_DOWN_MODEL));
		m_ss.sendBytes(szSendBuf, HEAD_SIZE + stHead.uPackSize);

		int nResult = RecvData();
		if (nResult == 0)
		{
			AfxMessageBox(_T("接收模板数据失败"));
			memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
			m_nRecvLen = 0;
			m_nWantLen = 0;
			return;
		}
		else if (nResult == 1)
		{
			pST_DOWN_MODEL pstModelInfo = (pST_DOWN_MODEL)(m_szRecvBuff + HEAD_SIZE);

			ST_CMD_HEADER stHead2;
			stHead2.usCmd = USER_DOWN_MODEL;
			stHead2.uPackSize = sizeof(ST_DOWN_MODEL);

			char szSendBuf[1024] = { 0 };
			memcpy(szSendBuf, (char*)&stHead2, HEAD_SIZE);
			memcpy(szSendBuf + HEAD_SIZE, (char*)&stModelInfo, sizeof(ST_DOWN_MODEL));
			m_ss.sendBytes(szSendBuf, HEAD_SIZE + stHead2.uPackSize);

			int nRecvResult = RecvFile(&stModelInfo);
			if (nRecvResult)
				AfxMessageBox(_T("下载成功"));
		}

	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "连接服务器失败，Detail: " + exc.displayText();
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		AfxMessageBox(_T("登录失败"));
		OnCancel();
	}
}

int CGetModelDlg::RecvData()
{
	int nCount = 0;
	int nBaseLen = HEAD_SIZE;
	try
	{
		while (!g_nExitFlag)
		{
			int nLen;
			if (m_nRecvLen < nBaseLen)
			{
				m_nWantLen = nBaseLen - m_nRecvLen;
				nLen = m_ss.receiveBytes(m_szRecvBuff + m_nRecvLen, m_nWantLen);
				if (nLen > 0)
				{
					m_nRecvLen += nLen;
					if (m_nRecvLen == nBaseLen)
					{
						if (nCount == 0)
						{
							ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
							nBaseLen += pstHead->uPackSize;
						}
						nCount++;
					}
				}
				else if (nLen == 0)
				{
					TRACE("the peer has closed.\n");
					return 0;
				}
				else
					Poco::Thread::sleep(1);
			}
			else
				break;
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "接收数据异常 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		return 0;
	}
	

	int nResult = 0;
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
	if (pstHead->usCmd == USER_RESPONSE_NEEDDOWN)
	{
		switch (pstHead->usResult)
		{
		case RESULT_DOWNMODEL_OK:
			{
				pST_DOWN_MODEL pstModelInfo = (pST_DOWN_MODEL)(m_szRecvBuff + HEAD_SIZE);
				nResult = 1;
			}
			break;
		case RESULT_DOWNMODEL_FAIL:
			{
				AfxMessageBox(_T("服务器此科目模板不存在"));
				nResult = 2;
			}
			break;
		case RESULT_DOWNMODEL_NONEED:
			{
				AfxMessageBox(_T("本地存在此文件，不需要下载"));
				nResult = 3;
			}
			break;
		}
	}
	return nResult;
}

int CGetModelDlg::RecvFile(pST_DOWN_MODEL pModelInfo)
{
	Poco::Timespan ts(60, 0);
	m_ss.setReceiveTimeout(ts);
	memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
	m_nRecvLen = 0;
	m_nWantLen = 0;

	int nCount = 0;
	int nBaseLen = HEAD_SIZE;
	try
	{
		while (!g_nExitFlag)
		{
			int nLen;
			if (m_nRecvLen < nBaseLen)
			{
				m_nWantLen = nBaseLen - m_nRecvLen;
				nLen = m_ss.receiveBytes(m_szRecvBuff + m_nRecvLen, m_nWantLen);
				if (nLen > 0)
				{
					m_nRecvLen += nLen;
					TRACE("get len: %d\n", nLen);
					if (m_nRecvLen == nBaseLen)
					{
						if (nCount == 0)
						{
							ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
							nBaseLen += pstHead->uPackSize;
						}
						nCount++;
					}
				}
				else if (nLen == 0)
				{
					TRACE("the peer has closed.\n");
					return 0;
				}
				else
					Poco::Thread::sleep(1);
			}
			else
				break;
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "接收模板文件异常 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		return 0;
	}

	USES_CONVERSION;
	int nResult = 0;
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
	if (pstHead->usCmd == USER_RESPONSE_DOWNMODEL)
	{
		switch (pstHead->usResult)
		{
			case RESULT_DOWNMODEL_RECV:
			{
				nResult = 1;

// 				char	*szFileBuff = new char[pstHead->uPackSize];
// 				strncpy(szFileBuff, m_szRecvBuff + HEAD_SIZE, pstHead->uPackSize);
// 				std::string strData = m_szRecvBuff + HEAD_SIZE;

				//覆盖本地文件
				std::string strModelPath = T2A(g_strCurrentPath);
				strModelPath.append("Model\\");
				strModelPath.append(pModelInfo->szModelName);
				Poco::File fileModel(strModelPath);
				if (fileModel.exists())
					fileModel.remove();

				ofstream out(strModelPath);
				std::stringstream buffer;
				buffer.write(m_szRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				int n = buffer.str().length();
				out << buffer.str();
				out.close();

//				SAFE_RELEASE(szFileBuff);
			}
			break;
		}
	}
	return nResult;
}
