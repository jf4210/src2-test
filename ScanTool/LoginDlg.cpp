// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
//#include "global.h"
#include "Net_Cmd_Protocol.h"

// CLoginDlg 对话框

IMPLEMENT_DYNAMIC(CLoginDlg, CDialog)

CLoginDlg::CLoginDlg(CString strIP, int nPort, CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
	, m_strUserName(_T("18520883118"))
	, m_strPwd(_T("123456"))
	, m_strServerIP(strIP)
	, m_nServerPort(nPort)
	, m_nRecvLen(0)
	, m_nWantLen(0)
	, m_bLogin(false)
{
	ZeroMemory(m_szRecvBuff, 2048);
}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_UserName, m_strUserName);
	DDX_Text(pDX, IDC_EDIT_Pwd, m_strPwd);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_Login, &CLoginDlg::OnBnClickedBtnLogin)
END_MESSAGE_MAP()


// CLoginDlg 消息处理程序


BOOL CLoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	UpdateData(FALSE);

	return TRUE;
}


void CLoginDlg::OnBnClickedBtnLogin()
{
	UpdateData(TRUE);
	USES_CONVERSION;

	Poco::Net::SocketAddress sa(T2A(m_strServerIP), m_nServerPort);
	try
	{
		Poco::Timespan ts(10, 0);
		m_ss.connect(sa);
		m_ss.setReceiveTimeout(ts);

		ST_CMD_HEADER stHead;
		stHead.usCmd = USER_LOGIN;
		stHead.uPackSize = sizeof(ST_LOGIN_INFO);
		ST_LOGIN_INFO stLogin;
		ZeroMemory(&stLogin, sizeof(ST_LOGIN_INFO));
		strcpy(stLogin.szUserNo, T2A(m_strUserName));
		strcpy(stLogin.szPWD, T2A(m_strPwd));

		char szSendBuf[1024] = { 0 };
		memcpy(szSendBuf, (char*)&stHead, HEAD_SIZE);
		memcpy(szSendBuf + HEAD_SIZE, (char*)&stLogin, sizeof(ST_LOGIN_INFO));
		m_ss.sendBytes(szSendBuf, HEAD_SIZE + stHead.uPackSize);

		CString strResult = _T("");
		if (RecvData(strResult))
		{
			if (!GetExamInfo())
				AfxMessageBox(_T("登录成功，获取考试信息失败"));
			OnOK();
		}
		else
		{
			if (strResult != _T(""))
				AfxMessageBox(_T("登录失败: ") + strResult);
			else
				AfxMessageBox(_T("登录失败"));
			OnCancel();
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

int CLoginDlg::RecvData(CString& strResultInfo)
{
	int nCount = 0;
	int nBaseLen = HEAD_SIZE;
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
	
	int nResult = 0;
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
	if (pstHead->usCmd == USER_RESPONSE_LOGIN)
	{
		char szData[300] = { 0 };
		switch (pstHead->usResult)
		{
		case RESULT_SUCCESS:
			m_bLogin = true;
			nResult = 1;
			strncpy(szData, m_szRecvBuff + HEAD_SIZE, pstHead->uPackSize);
			m_strEzs = szData;
			break;
		case RESULT_LOGIN_FAIL:
			m_bLogin = false;
			nResult = 0;
			strncpy(szData, m_szRecvBuff + HEAD_SIZE, pstHead->uPackSize);
			strResultInfo = szData;
			break;
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_EXAMINFO)
	{
		switch (pstHead->usResult)
		{
			case RESULT_EXAMINFO_SUCCESS:
			{
				nResult = 1;
				char szExamData[1024 * 10] = { 0 };
				strncpy(szExamData, m_szRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				std::string strExamData = szExamData;

				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result;
				try
				{
					result = parser.parse(szExamData);
					Poco::JSON::Object::Ptr examObj = result.extract<Poco::JSON::Object::Ptr>();

					Poco::JSON::Array::Ptr arryObj = examObj->getArray("exams");

					for (int i = 0; i < arryObj->size(); i++)
					{
						Poco::JSON::Object::Ptr objExamInfo = arryObj->getObject(i);
						EXAMINFO examInfo;
						examInfo.nExamID = objExamInfo->get("id").convert<int>();
						examInfo.strExamName = CMyCodeConvert::Utf8ToGb2312(objExamInfo->get("name").convert<std::string>());

						if (!objExamInfo->isNull("examType"))
						{
							Poco::JSON::Object::Ptr objExamType = objExamInfo->getObject("examType");
							if (objExamType->has("name"))
								examInfo.strExamTypeName = CMyCodeConvert::Utf8ToGb2312(objExamType->get("name").convert<std::string>());
						}
						if (!objExamInfo->isNull("grade"))
						{
							Poco::JSON::Object::Ptr objGrade = objExamInfo->getObject("grade");
							if (objGrade->has("id"))
								examInfo.nExamGrade = objGrade->get("id").convert<int>();
							if (objGrade->has("name"))
								examInfo.strGradeName = CMyCodeConvert::Utf8ToGb2312(objGrade->get("name").convert<std::string>());
						}
						examInfo.nExamState = objExamInfo->get("state").convert<int>();

						Poco::JSON::Array::Ptr arrySubjects = objExamInfo->getArray("examSubjects");
						for (int j = 0; j < arrySubjects->size(); j++)
						{
							Poco::JSON::Object::Ptr objSubject = arrySubjects->getObject(j);
							EXAM_SUBJECT subjectInfo;
							subjectInfo.nSubjID = objSubject->get("id").convert<int>();
							subjectInfo.nSubjCode = objSubject->get("code").convert<int>();
							subjectInfo.strSubjName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("name").convert<std::string>());
//							subjectInfo.strModelName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("scanTemplateName").convert<std::string>());
							examInfo.lSubjects.push_back(subjectInfo);
						}
						g_lExamList.push_back(examInfo);
					}
				}
				catch (Poco::JSON::JSONException& jsone)
				{
					std::string strErrInfo;
					strErrInfo.append("Error when parse json: ");
					strErrInfo.append(jsone.message() + "\tData:" + strExamData);
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());
				}
				catch (Poco::Exception& exc)
				{
					std::string strErrInfo;
					strErrInfo.append("Error: ");
					strErrInfo.append(exc.message() + "\tData:");
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());
				}
				catch (...)
				{
					std::string strErrInfo;
					strErrInfo.append("Unknown error.\tData:" + strExamData);
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());
				}
			}
			break;
		}
	}
	return nResult;
}

int CLoginDlg::GetExamInfo()
{
	USES_CONVERSION;
	if (!m_bLogin)
		return 0;

	int nResult = 0;
	ZeroMemory(m_szRecvBuff, sizeof(m_szRecvBuff));
	m_nRecvLen = 0;
	m_nWantLen = 0;

	ST_CMD_HEADER stHead;
	stHead.usCmd = USER_GETEXAMINFO;
	stHead.uPackSize = sizeof(ST_EXAM_INFO);
	ST_EXAM_INFO stExamInfo;
	ZeroMemory(&stExamInfo, sizeof(ST_EXAM_INFO));
	strcpy(stExamInfo.szEzs, T2A(m_strEzs));

	char szSendBuf[1024] = { 0 };
	memcpy(szSendBuf, (char*)&stHead, HEAD_SIZE);
	memcpy(szSendBuf + HEAD_SIZE, (char*)&stExamInfo, sizeof(ST_EXAM_INFO));
	m_ss.sendBytes(szSendBuf, HEAD_SIZE + stHead.uPackSize);

	CString strResult2 = _T("");
	if (RecvData(strResult2))
	{
		nResult = 1;
	}

	return nResult;
}
