// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
//#include "global.h"
#include "Net_Cmd_Protocol.h"
#include "MultiPlatform4TYDlg.h"

// CLoginDlg 对话框

IMPLEMENT_DYNAMIC(CLoginDlg, CDialog)

CLoginDlg::CLoginDlg(CString strIP, int nPort, CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
	, m_strUserName(_T(""))
	, m_strPwd(_T(""))
	, m_strUser(_T(""))
	, m_strNickName(_T(""))
	, m_strPersonId(_T(""))
	, m_strServerIP(strIP)
	, m_nServerPort(nPort)
	, m_nRecvLen(0)
	, m_nWantLen(0)
	, m_bLogin(false)
	, m_pRecvBuff(NULL)
	, m_strEzs(_T(""))
	, m_nTeacherId(-1)
	, m_nUserId(-1)
{
	ZeroMemory(m_szRecvBuff, 1024);
}

CLoginDlg::~CLoginDlg()
{
#ifdef TO_WHTY
	VEC_PLATFORM_TY::iterator it = _vecPlatformList.begin();
	for (; it != _vecPlatformList.end();)
	{
		pST_PLATFORMINFO p = *it;
		it = _vecPlatformList.erase(it);
		SAFE_RELEASE(p);
	}
#endif
	SAFE_RELEASE_ARRY(m_pRecvBuff);
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

	char* ret;
	ret = new char[255];
	ret[0] = '\0';
	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", ret) == 0)
	{
		m_strUserName = ret;
	}
	SAFE_RELEASE_ARRY(ret);

#ifdef Test_Data
	m_strPwd = _T("12345678");
#endif

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
		int nResult = RecvData(strResult);
		if (nResult == 1)
		{
			GetExamInfo();
//			GetBmkInfo();
			WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));
			AfxMessageBox(_T("登录成功"));	//登录成功，获取考试信息失败
			CDialog::OnOK();
		}
		else if (nResult == 2)
		{
			//天喻版本，弹出平台列表，选择一个平台
		#ifdef TO_WHTY
			CMultiPlatform4TYDlg dlg(_vecPlatformList);
			dlg.DoModal();
			WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));

			pST_PLATFORMINFO pPlatform = dlg._pPlatform;
			//重新发送登录请求
			LoginPlatform4TY(pPlatform);
			CString strResult2 = _T("");
			int nResult2 = RecvData(strResult2);
			if (nResult2 == 1)
			{
				GetExamInfo();
				WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));
				AfxMessageBox(_T("登录成功"));	//登录成功，获取考试信息失败
				CDialog::OnOK();
			}
			else
			{
				WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));
				if (strResult2 != _T(""))
					AfxMessageBox(_T("登录失败: ") + strResult2);
				else
					AfxMessageBox(_T("登录失败"));
				CDialog::OnCancel();
			}
		#endif
		}
		else
		{
			WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));
			if (strResult != _T(""))
				AfxMessageBox(_T("登录失败: ") + strResult);
			else
				AfxMessageBox(_T("登录失败"));
			CDialog::OnCancel();
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

#if 1
	if (!m_pRecvBuff)
	{
		m_pRecvBuff = new char[1024 + HEAD_SIZE];
		nRecvBuffSize = 1024 + HEAD_SIZE;
	}
	while (!g_nExitFlag)
	{
		int nLen;
		if (m_nRecvLen < nBaseLen)
		{
			m_nWantLen = nBaseLen - m_nRecvLen;
			nLen = m_ss.receiveBytes(m_pRecvBuff + m_nRecvLen, m_nWantLen);
			if (nLen > 0)
			{
				m_nRecvLen += nLen;
				if (m_nRecvLen == nBaseLen)
				{
					if (nCount == 0)	//收到头信息
					{
						ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_pRecvBuff;
						nBaseLen += pstHead->uPackSize;
						if (pstHead->uPackSize == 0)
							break;
						else
						{
							int n = strlen(m_pRecvBuff);
							int n2 = sizeof(m_pRecvBuff);
							if (pstHead->uPackSize > 1024)
							{
								char* pOld = m_pRecvBuff;
								m_pRecvBuff = new char[pstHead->uPackSize + HEAD_SIZE + 1];
								m_pRecvBuff[pstHead->uPackSize + HEAD_SIZE] = '\0';
								nRecvBuffSize = pstHead->uPackSize + HEAD_SIZE + 1;
								memcpy(m_pRecvBuff, pOld, m_nRecvLen);
								SAFE_RELEASE_ARRY(pOld);
							}
						}
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
#else
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
#endif
	
	int nResult = 0;
//	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_pRecvBuff;
	if (pstHead->usCmd == USER_RESPONSE_LOGIN)
	{
		switch (pstHead->usResult)
		{
		case RESULT_SUCCESS:
			{
				char szData[1024] = { 0 };
				m_bLogin = true;
				nResult = 1;
//				memcpy(szData, m_szRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				memcpy(szData, m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				pST_LOGIN_RESULT pstResult = (pST_LOGIN_RESULT)szData;

				m_strEzs = pstResult->szEzs;
				m_nTeacherId = pstResult->nTeacherId;
				m_nUserId = pstResult->nUserId;

				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result;
				try
				{
					result = parser.parse(pstResult->szUserInfo);
					Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
				#ifdef TO_WHTY
					std::string strPersonId = object->get("personid").convert<std::string>();
					std::string strNickName = CMyCodeConvert::Utf8ToGb2312(object->get("name").convert<std::string>());
					std::string strUserName = object->get("account").convert<std::string>();
					m_strPersonId = strPersonId.c_str();
				#else
					std::string strUserName = object->get("name").convert<std::string>();
					Poco::JSON::Object::Ptr objDetail = object->getObject("detail");
					std::string strNickName = CMyCodeConvert::Utf8ToGb2312(objDetail->get("nickName").convert<std::string>());
				#endif
					m_strUser		= strUserName.c_str();
					m_strNickName	= strNickName.c_str();
				}
				catch (Poco::JSON::JSONException& jsone)
				{
					std::string strErrInfo;
					strErrInfo.append("Error when parse json(获取用户信息): ");
					strErrInfo.append(jsone.message() + "\tData:" + pstResult->szUserInfo);
					g_pLogger->information(strErrInfo);
				}
				catch (Poco::Exception& exc)
				{
					std::string strErrInfo;
					strErrInfo.append("Error(获取用户信息): ");
					strErrInfo.append(exc.message() + "\tData:" + pstResult->szUserInfo);
					g_pLogger->information(strErrInfo);
				}
			}
			break;
		case RESULT_LOGIN_FAIL:
			{
				char szData[1024] = { 0 };
				m_bLogin = false;
				nResult = 0;
//				memcpy(szData, m_szRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				memcpy(szData, m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				strResultInfo = szData;
			}
			break;
		case RESULT_LOGIN_PLATFORM_TY:
			{
			#ifdef TO_WHTY
				char szData[4096] = { 0 };
				nResult = 2;	//天喻平台选择
// 				char* pData = new char[pstHead->uPackSize + 1];
// 				pData[pstHead->uPackSize] = '\0';
				memcpy(szData, m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);

				std::string strLog;
				strLog.append("收到平台数据如下: " + CMyCodeConvert::Utf8ToGb2312(szData));
				g_pLogger->information(strLog);

				VEC_PLATFORM_TY::iterator it = _vecPlatformList.begin();
				for (; it != _vecPlatformList.end();)
				{
					pST_PLATFORMINFO p = *it;
					it = _vecPlatformList.erase(it);
					SAFE_RELEASE(p);
				}
				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result;
				try
				{
					result = parser.parse(szData);
					Poco::JSON::Array::Ptr objectArry = result.extract<Poco::JSON::Array::Ptr>();
					for (int i = 0; i < objectArry->size(); i++)
					{
						Poco::JSON::Object::Ptr obj = objectArry->getObject(i);
						std::string strPlatformUrl = obj->get("platformUrl").convert<std::string>();
						std::string strPlatformCode = obj->get("platformCode").convert<std::string>();
						std::string strPlatformName = CMyCodeConvert::Utf8ToGb2312(obj->get("platformName").convert<std::string>());
						std::string strEncryption = obj->get("encryption").convert<std::string>();
						pST_PLATFORMINFO pstPlatform = new ST_PLATFORMINFO;
						pstPlatform->strPlatformUrl = strPlatformUrl;
						pstPlatform->strPlatformCode = strPlatformCode;
						pstPlatform->strPlatformName = strPlatformName;
						pstPlatform->strEncryption = strEncryption;
						_vecPlatformList.push_back(pstPlatform);
					}
				}
				catch (Poco::JSON::JSONException& jsone)
				{
					std::string strErrInfo;
					strErrInfo.append("Error when parse json(获取用户信息): ");
					strErrInfo.append(jsone.message() + "\tData:" + szData);
					g_pLogger->information(strErrInfo);
				}
			#endif
			}
			break;
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_EXAMINFO)
	{
	}
	return nResult;
}

int CLoginDlg::GetExamInfo()
{
	USES_CONVERSION;
	if (!m_bLogin)
		return 0;

	int nResult = 0;
//	ZeroMemory(m_szRecvBuff, sizeof(m_szRecvBuff));
	ZeroMemory(m_pRecvBuff, nRecvBuffSize);
	m_nRecvLen = 0;
	m_nWantLen = 0;

// 	ST_CMD_HEADER stHead;
// 	stHead.usCmd = USER_GETEXAMINFO;
// 	stHead.uPackSize = sizeof(ST_EXAM_INFO);
	ST_EXAM_INFO stExamInfo;
	ZeroMemory(&stExamInfo, sizeof(ST_EXAM_INFO));
#ifdef TO_WHTY
	strcpy(stExamInfo.szEzs, T2A(m_strPersonId));
#else
	strcpy(stExamInfo.szEzs, T2A(m_strEzs));
#endif

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_GETEXAMINFO;
	pTcpTask->nPkgLen = sizeof(ST_EXAM_INFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stExamInfo, sizeof(ST_EXAM_INFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();

	g_eGetExamList.reset();

	return nResult;
}

int CLoginDlg::GetBmkInfo()
{
	USES_CONVERSION;
	if (!m_bLogin)
		return 0;

	int nResult = 0;
//	ZeroMemory(m_szRecvBuff, sizeof(m_szRecvBuff));
	ZeroMemory(m_pRecvBuff, nRecvBuffSize);
	m_nRecvLen = 0;
	m_nWantLen = 0;

	ST_GET_BMK_INFO stGetBmkInfo;
	ZeroMemory(&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
#ifdef TO_WHTY
	strcpy(stGetBmkInfo.szEzs, T2A(m_strPersonId));
#else
	strcpy(stGetBmkInfo.szEzs, T2A(m_strEzs));
#endif

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_GET_BMK;
	pTcpTask->nPkgLen = sizeof(ST_GET_BMK_INFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();

	return nResult;
}

#ifdef TO_WHTY
void CLoginDlg::LoginPlatform4TY(pST_PLATFORMINFO p)
{
	ZeroMemory(m_pRecvBuff, nRecvBuffSize);
	m_nRecvLen = 0;
	m_nWantLen = 0;

	USES_CONVERSION;
	Poco::Timespan ts(6, 0);
	m_ss.setReceiveTimeout(ts);

	ST_CMD_HEADER stHead;
	stHead.usCmd = USER_LOGIN_4TY;
	stHead.uPackSize = sizeof(ST_LOGIN_INFO4TY);
	ST_LOGIN_INFO4TY stLogin;
	ZeroMemory(&stLogin, sizeof(ST_LOGIN_INFO4TY));
	strcpy_s(stLogin.szUserNo, T2A(m_strUserName));
	strcpy_s(stLogin.szPWD, T2A(m_strPwd));
	strcpy_s(stLogin.szPlatformCode, p->strPlatformCode.c_str());
	strcpy_s(stLogin.szEncryption, p->strEncryption.c_str());
	strcpy_s(stLogin.szPlatformUrl, p->strPlatformUrl.c_str());

	char szSendBuf[2048] = { 0 };
	memcpy(szSendBuf, (char*)&stHead, HEAD_SIZE);
	memcpy(szSendBuf + HEAD_SIZE, (char*)&stLogin, sizeof(ST_LOGIN_INFO4TY));
	m_ss.sendBytes(szSendBuf, HEAD_SIZE + stHead.uPackSize);
}
#endif
void CLoginDlg::OnOK()
{
	OnBnClickedBtnLogin();
}
