// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "global.h"
#include "Net_Cmd_Protocol.h"
#include "MultiPlatform4TYDlg.h"
#include "NewMessageBox.h"
#include "structured_exception.h"
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
	, m_bRecordPwd(true)
	, m_nStatusSize(15)
{
	ZeroMemory(m_szRecvBuff, 1024);
}

CLoginDlg::~CLoginDlg()
{
//#ifdef TO_WHTY
#if 1
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
	DDX_Control(pDX, IDC_EDIT_UserName, m_icnEditUser);
	DDX_Control(pDX, IDC_EDIT_Pwd, m_icnEditPwd);
	DDX_Control(pDX, IDC_BTN_Login, m_bmpBtnLogin);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_bmpBtnExit);
	DDX_Control(pDX, IDC_BTN_Min, m_bmpBtnMin);
	DDX_Text(pDX, IDC_STATIC_LoginTitle, m_strTitle);
	DDX_Text(pDX, IDC_STATIC_Version, m_strVersion);
	DDX_Text(pDX, IDC_STATIC_Bottom, m_strCopyright);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_Login, &CLoginDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(IDC_CHK_RecordPwd, &CLoginDlg::OnBnClickedChkRecordpwd)
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CLoginDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_Min, &CLoginDlg::OnBnClickedBtnMin)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_CheckBox, &CLoginDlg::OnStnClickedStaticCheckbox)
END_MESSAGE_MAP()


// CLoginDlg 消息处理程序

BOOL CLoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//++加密版权、标题数据信息
#if 0
	std::string strTmpMain = "扫描客户端";
	std::string strTmpLitteTitle = "big data scan tool";
	std::string strTmpCopyright = "山西省太原市万柏林教育局 2017~2018 版权所有";
	encString(strTmpMain, _strMainTitle_);
	encString(strTmpLitteTitle, _strLitteTitle_);
	encString(strTmpCopyright, _strCopyright_);
#endif
	//--
	decString(_strMainTitle_, _strMainTitle_);
	decString(_strLitteTitle_, _strLitteTitle_);
	decString(_strCopyright_, _strCopyright_);
	
	InitUI();
	USES_CONVERSION;

//	m_strTitle.Format(_T("%s"), SYS_BASE_NAME);

	InitCtrlPosition();
	m_strTitle.Format(_T("%s"), A2T(_strMainTitle_.c_str()));
	m_strCopyright.Format(_T("%s"), A2T(_strCopyright_.c_str()));
	m_strVersion = SOFT_VERSION;
	SetWindowText(m_strTitle);

	char* ret;
	ret = new char[255];
	ret[0] = '\0';
	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", ret) == 0)
	{
		m_strUserName = ret;
	}
	memset(ret, 0, 255);
	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "recordPwd", ret) == 0)
	{
		m_bRecordPwd = atoi(ret);
	}
	if (m_bRecordPwd)
	{
		memset(ret, 0, 255);
		if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "pwd", ret) == 0)
		{
			m_strPwd = ret;
		}
	}
	((CButton*)GetDlgItem(IDC_CHK_RecordPwd))->SetCheck(m_bRecordPwd);
	SAFE_RELEASE_ARRY(ret);
	
	UpdateData(FALSE);

	return TRUE;
}

void CLoginDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	CRect rt1, rt2;
	int nTmpX = 0;
	if (GetDlgItem(IDC_STATIC_LoginTitle)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_LoginTitle)->GetWindowRect(rt1);
		int nLen = _strMainTitle_.length() / 2;
		int nMinLen = nLen * (m_nStatusSize + 2);
		rt1.right = rt1.left + nMinLen;
		nTmpX = rt1.right + 5;
		
		ScreenToClient(rt1);
		GetDlgItem(IDC_STATIC_LoginTitle)->MoveWindow(rt1);
	}
	if (GetDlgItem(IDC_STATIC_Version)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Version)->GetWindowRect(rt2);
		int nW = rt2.Width();
		rt2.left = nTmpX;
		rt2.right = rt2.left + nW;

		ScreenToClient(rt2);
		GetDlgItem(IDC_STATIC_Version)->MoveWindow(rt2);
	}
}

BOOL CLoginDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			OnCancel();
		}
		else if (pMsg->wParam == VK_RETURN)
		{
			OnOK();
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CLoginDlg::OnBnClickedBtnLogin()
{
	UpdateData(TRUE);
	USES_CONVERSION;

	//检查账号的非法字符
	m_strUserName = m_strUserName.Trim();
 	std::string strChkUserName = T2A(m_strUserName);
	if (strChkUserName.find('\\') != std::string::npos || strChkUserName.find('\/') != std::string::npos || strChkUserName.find('\:') != std::string::npos || strChkUserName.find('\*') != std::string::npos
		|| strChkUserName.find('\?') != std::string::npos || strChkUserName.find('\"') != std::string::npos || strChkUserName.find('\<') != std::string::npos || strChkUserName.find('\>') != std::string::npos
		|| strChkUserName.find('\|') != std::string::npos)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "用户名中含有非法字符或者隐藏字符, 请仔细检查！");
		dlg.DoModal();
		return;
	}

	if (m_bRecordPwd)
	{
		char szRet[50] = { 0 };
		sprintf_s(szRet, "%s", T2A(m_strPwd));
		WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "pwd", szRet);

		sprintf_s(szRet, "%d", 1);
		WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "recordPwd", szRet);
	}
	else
	{
		char szRet[50] = { 0 };
		sprintf_s(szRet, "%d", 0);
		WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "recordPwd", szRet);
	}

	Poco::Net::SocketAddress sa(T2A(m_strServerIP), m_nServerPort);
	try
	{
		Poco::Timespan ts(6, 0);
		m_ss.close();
		m_ss.connect(sa);
		m_ss.setReceiveTimeout(ts);
		SAFE_RELEASE(m_pRecvBuff);
		m_nRecvLen = 0;

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
		int nSendLen = m_ss.sendBytes(szSendBuf, HEAD_SIZE + stHead.uPackSize);

		CString strResult = _T("");
		int nResult = RecvData(strResult);
		if (nResult == 1)
		{
			GetFileAddrs();
			GetExamInfo();
//			GetBmkInfo();
			WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));
//			AfxMessageBox(_T("登录成功"));	//登录成功，获取考试信息失败
			CDialog::OnOK();
		}
		else if (nResult == 2)
		{
			//天喻版本，弹出平台列表，选择一个平台
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
				GetFileAddrs();
				GetExamInfo();
				WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));
//				AfxMessageBox(_T("登录成功"));	//登录成功，获取考试信息失败
				CDialog::OnOK();
			}
			else
			{
				WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));
				std::string strErrShow;
				if (strResult2 != _T(""))
				{
//					AfxMessageBox(_T("登录失败: ") + strResult2);
					strErrShow = "登录失败: " + std::string(T2A(strResult2));
				}
				else
				{
//					AfxMessageBox(_T("登录失败"));
					strErrShow = "登录失败";
				}
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, strErrShow);
				dlg.DoModal();
//				CDialog::OnCancel();
			}
		}
		else
		{
			WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "login", T2A(m_strUserName));
			std::string strErrShow;
			if (strResult != _T(""))
			{
//				AfxMessageBox(_T("登录失败: ") + strResult);
				strErrShow = "登录失败: " + std::string(T2A(strResult));
			}
			else
			{
//				AfxMessageBox(_T("登录失败"));
				strErrShow = "登录失败";
			}
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, strErrShow);
			dlg.DoModal();
//			CDialog::OnCancel();
		}

	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "连接服务器失败，Detail: " + exc.displayText();
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
//		AfxMessageBox(_T("登录失败"));
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "登录失败");
		dlg.DoModal();
//		OnCancel();
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
						if (pstHead->usVerifyCode != VERIFYCODE)
							return 0;
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
					//判断是否是手阅模式登录，即是否直接登录天喻的服务器
					if (object->has("personid") && !object->isNull("personid"))
						_bHandModel_ = true;
					else
						_bHandModel_ = false;

					if (_bHandModel_)
					{
						std::string strPersonId = object->get("personid").convert<std::string>();
						std::string strNickName = CMyCodeConvert::Utf8ToGb2312(object->get("name").convert<std::string>());
						std::string strUserName = object->get("account").convert<std::string>();
						std::string strSchoolID;
						if (object->has("orgaid"))
							strSchoolID = object->get("orgaid").convert<std::string>();
						m_strPersonId = strPersonId.c_str();
						m_strSchoolID = strSchoolID.c_str();
						m_strUser = strUserName.c_str();
						m_strNickName = strNickName.c_str();
					}
					else
					{
						std::string strUserName = object->get("name").convert<std::string>();
						Poco::JSON::Object::Ptr objDetail = object->getObject("detail");
						std::string strNickName = CMyCodeConvert::Utf8ToGb2312(objDetail->get("nickName").convert<std::string>());
						m_strUser = strUserName.c_str();
						m_strNickName = strNickName.c_str();
					}
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
//			#ifdef TO_WHTY
			#if 1
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
	ZeroMemory(m_pRecvBuff, nRecvBuffSize);
	m_nRecvLen = 0;
	m_nWantLen = 0;

	ST_EXAM_INFO stExamInfo;
	ZeroMemory(&stExamInfo, sizeof(ST_EXAM_INFO));
	if (_bHandModel_)
	{
	#if 1
		std::string strTmp = T2A(m_strPersonId);
		strTmp.append("###");
		strTmp.append(T2A(m_strSchoolID));
		strcpy(stExamInfo.szEzs, strTmp.c_str());
	#else
		strcpy(stExamInfo.szEzs, T2A(m_strPersonId));
	#endif
	}
	else
	{
		stExamInfo.nUserId = m_nUserId;
		strcpy(stExamInfo.szEzs, T2A(m_strEzs));
	}

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
	if (_bHandModel_)
		strcpy(stGetBmkInfo.szEzs, T2A(m_strPersonId));
	else
		strcpy(stGetBmkInfo.szEzs, T2A(m_strEzs));

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_GET_BMK;
	pTcpTask->nPkgLen = sizeof(ST_GET_BMK_INFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();

	return nResult;
}

void CLoginDlg::GetFileAddrs()
{
	TRACE("请求其他格式的文件上传地址\n");

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_GET_FILE_UPLOAD_ADDR;
	pTcpTask->nPkgLen = 0;
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
}

void CLoginDlg::InitUI()
{
	m_bmpBtnExit.SetStateBitmap(IDB_LOGIN_CloseBtn, 0, IDB_LOGIN_CloseBtn_Hover);
	m_bmpBtnMin.SetStateBitmap(IDB_LOGIN_MinBtn, 0, IDB_LOGIN_MinBtn_Hover);
	m_bmpBtnLogin.SetStateBitmap(IDB_LOGIN_Btn, 0, IDB_LOGIN_Btn_Hover);
	m_bmpBtnLogin.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 0);

	m_icnEditUser.SetIcon(IDI_ICON_LOGIN_User);
	m_icnEditPwd.SetIcon(IDI_ICON_LOGIN_Pwd);
	m_icnEditPwd.SetPasswordChar('*');

	m_bmpTitle.LoadBitmap(IDB_LOGIN_TITLE);
	m_bmpBkg.LoadBitmap(IDB_LOGIN_BK);

	SetFontSize(m_nStatusSize);
}

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

void CLoginDlg::OnOK()
{
	OnBnClickedBtnLogin();
}


void CLoginDlg::OnBnClickedChkRecordpwd()
{
	if (((CButton*)GetDlgItem(IDC_CHK_RecordPwd))->GetCheck())
		m_bRecordPwd = true;
	else
		m_bRecordPwd = false;
}

void CLoginDlg::OnStnClickedStaticCheckbox()
{
	m_bRecordPwd = !m_bRecordPwd;
	((CButton*)GetDlgItem(IDC_CHK_RecordPwd))->SetCheck(m_bRecordPwd);
}

LRESULT CLoginDlg::OnNcHitTest(CPoint point)
{
	CRect rcWndRect;
	GetWindowRect(rcWndRect);
//	rcWndRect.bottom = rcWndRect.top + 40;	//40
	if (rcWndRect.PtInRect(point))
		return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}


void CLoginDlg::OnBnClickedBtnClose()
{
	OnCancel();
}

void CLoginDlg::OnBnClickedBtnMin()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

BOOL CLoginDlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	int iX, iY;
	CDC memDC;
	CRect rectClient;
	BITMAP bmp;

	iX = iY = 0;
	GetClientRect(&rectClient);

	if (memDC.CreateCompatibleDC(pDC))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_bmpTitle);
		m_bmpTitle.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, rectClient.Width(), bmp.bmHeight, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);

		int nTmpX = bmp.bmWidth;
		int nTitleH = bmp.bmHeight;
		
		iX = 0;
		iY = iY + nTitleH;
		pOldBmp = memDC.SelectObject(&m_bmpBkg);
		m_bmpBkg.GetBitmap(&bmp);

		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, rectClient.Width(), rectClient.Height() - nTitleH, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);
	}
	memDC.DeleteDC();

	return TRUE;
}

HBRUSH CLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC || CurID == IDC_STATIC_CheckBox)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_Bottom || CurID == IDC_STATIC_Version)
	{
		pDC->SetTextColor(RGB(173, 209, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_LoginTitle )
	{
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_CHK_RecordPwd )
	{
		HBRUSH hMYbr = ::CreateSolidBrush(RGB(62, 147, 254));

//		pDC->SetBkColor(RGB(0,0,255));
		pDC->SetBkMode(TRANSPARENT);

//		return (HBRUSH)GetStockObject(NULL_BRUSH);
		return hMYbr;
	}
// 	else if (CurID == IDC_EDIT_Pwd || CurID == IDC_EDIT_UserName)
// 	{
// 		HBRUSH hMYbr = ::CreateSolidBrush(RGB(62, 147, 254));
// 		pDC->SetBkMode(TRANSPARENT);
// 		return hMYbr;
// 	}
	return hbr;
}

void CLoginDlg::SetFontSize(int nSize)
{
	m_fontTitle.DeleteObject();
	m_fontTitle.CreateFont(nSize, 0, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("幼圆"));	//Arial
	m_fontVersion.DeleteObject();
	m_fontVersion.CreateFont(nSize - 3, 0, 0, 0,
						   FW_BOLD, FALSE, FALSE, 0,
						   DEFAULT_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH | FF_SWISS,
						   _T("幼圆"));	//Arial
	m_fontLogin.DeleteObject();
	m_fontLogin.CreateFont(15, 0, 0, 0,
							 FW_BOLD, FALSE, FALSE, 0,
							 DEFAULT_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,
							 DEFAULT_PITCH | FF_SWISS,
							 _T("幼圆"));	//Arial
	GetDlgItem(IDC_STATIC_LoginTitle)->SetFont(&m_fontTitle);
	GetDlgItem(IDC_STATIC_Version)->SetFont(&m_fontVersion);
	m_bmpBtnLogin.SetBtnFont(m_fontLogin);
}

