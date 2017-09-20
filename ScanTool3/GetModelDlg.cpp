// GetModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanTool3Dlg.h"
#include "GetModelDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"
//#include "Net_Cmd_Protocol.h"

// CGetModelDlg 对话框

IMPLEMENT_DYNAMIC(CGetModelDlg, CDialog)

CGetModelDlg::CGetModelDlg(CString strIP, int nPort, CWnd* pParent /*=NULL*/)
: CTipBaseDlg(CGetModelDlg::IDD, pParent)
, m_strExamTypeName(_T("")), m_strGradeName(_T("")), m_SubjectID(0), m_nExamID(0), m_strScanModelName(_T("")), m_strExamName(_T("")), m_strSubjectName(_T(""))
, m_strServerIP(strIP)
, m_nServerPort(nPort)
, m_nRecvLen(0)
, m_nWantLen(0)
, m_pFileRecv(NULL)
{
	EnableToolTips(TRUE);
	m_Tip.Create(this);
	m_Tip.SetMaxTipWidth(500);
	m_Tip.SetTipTextColor(RGB(255, 0, 0));//设置提示字体颜色
	m_Tip.SetTipBkColor(RGB(255, 255, 255));//设置提示背景颜色
	m_Tip.Activate(TRUE);
}

CGetModelDlg::~CGetModelDlg()
{
	m_ss.close();
	SAFE_RELEASE_ARRY(m_pFileRecv);
}

void CGetModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ExamID, m_nExamID);
	DDX_Text(pDX, IDC_EDIT_ExamTypeName, m_strExamTypeName);
	DDX_Text(pDX, IDC_EDIT_GradeName, m_strGradeName);
	DDX_Text(pDX, IDC_EDIT_SubjectID, m_SubjectID);
	DDX_Text(pDX, IDC_EDIT_ScanModelName, m_strScanModelName);
	DDX_Control(pDX, IDC_COMBO_ExamName, m_comboExamName);
	DDX_Control(pDX, IDC_COMBO_SubjectName, m_comboSubject);
	DDX_Control(pDX, IDC_PROGRESS_DOWN, m_progress);
}


BEGIN_MESSAGE_MAP(CGetModelDlg, CTipBaseDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_ExamName, &CGetModelDlg::OnCbnSelchangeComboExamname)
	ON_CBN_SELCHANGE(IDC_COMBO_SubjectName, &CGetModelDlg::OnCbnSelchangeComboSubjectname)
	ON_BN_CLICKED(IDC_BTN_DOWN, &CGetModelDlg::OnBnClickedBtnDown)
	ON_BN_CLICKED(IDC_BTN_Exit, &CGetModelDlg::OnBnClickedBtnExit)
END_MESSAGE_MAP()

BOOL CGetModelDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	USES_CONVERSION;

	InitUI();
//	m_Tip.AddTool(GetDlgItem(IDC_COMBO_ExamName), m_strExamName);
// 	m_comboExamName.SetEditTooltip(TRUE);
// 	m_comboExamName.SetListTooltip(TRUE, TRUE, TRUE);
// 	m_comboExamName.SetMode(CComboBoxExt::MODE_STANDARD);
	m_comboExamName.AdjustDroppedWidth();
//	m_comboExamName.SetMode(CComboBoxExt::MODE_AUTOCOMPLETE);
//	m_comboSubject.SetListTooltip(TRUE, TRUE, TRUE);
	m_comboSubject.AdjustDroppedWidth();

	return TRUE;
}

// CGetModelDlg 消息处理程序
void CGetModelDlg::OnCbnSelchangeComboExamname()
{
	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemDataPtr(n);
	if (!pExamInfo)
		return;

	USES_CONVERSION;
	m_comboSubject.ResetContent();
	SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
	for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
	{
		EXAM_SUBJECT* pSubject = *itSub;
		CString strSubjectName = A2T((*itSub)->strSubjName.c_str());
		int nCount = m_comboSubject.GetCount();
		m_comboSubject.InsertString(nCount, strSubjectName);
		m_comboSubject.SetItemDataPtr(nCount, pSubject);

		if (i == 0)
		{
			m_SubjectID = (*itSub)->nSubjID;
			m_strScanModelName = A2T((*itSub)->strModelName.c_str());
			m_strSubjectName = (*itSub)->strSubjName.c_str();
		}
	}
	m_comboSubject.SetCurSel(0);

	m_strExamName = pExamInfo->strExamName.c_str();
	m_nExamID = pExamInfo->nExamID;
	m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
	m_strGradeName = pExamInfo->strGradeName.c_str();

	m_Tip.AddTool(GetDlgItem(IDC_COMBO_ExamName), m_strExamName);

	UpdateData(FALSE);
}

void CGetModelDlg::OnCbnSelchangeComboSubjectname()
{
	int n2 = m_comboSubject.GetCurSel();

	USES_CONVERSION;
	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemDataPtr(n);
	if (!pExamInfo)
		return;

	SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
	for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
	{
		if (i == n2)
		{
			m_SubjectID = (*itSub)->nSubjID;
			m_strSubjectName = (*itSub)->strSubjName.c_str();
			m_strScanModelName = A2T((*itSub)->strModelName.c_str());
		}
	}

	UpdateData(FALSE);
}

void CGetModelDlg::OnBnClickedBtnDown()
{
	EnableBtn(FALSE);

	m_progress.SetPos(0);
	USES_CONVERSION;
	Poco::Net::SocketAddress sa(T2A(m_strServerIP), m_nServerPort);
	m_ss.close();

	try
	{
		Poco::Timespan ts(5, 0);
		m_ss.connect(sa);
		m_ss.setNoDelay(true);
		m_ss.setReceiveTimeout(ts);

		CString strUser = _T("");
		strUser = _strUserName_.c_str();

		//先查本地列表，如果没有则请求，如果有，计算crc，和服务器不同则下载
		USES_CONVERSION;
		CString modelPath = g_strCurrentPath + _T("Model");
		modelPath = modelPath + _T("\\") + m_strScanModelName;
		std::string strModelPath = T2A(modelPath);

		ST_DOWN_MODEL stModelInfo;
		ZeroMemory(&stModelInfo, sizeof(ST_DOWN_MODEL));
		stModelInfo.nExamID = m_nExamID;
		stModelInfo.nSubjectID = m_SubjectID;
		sprintf_s(stModelInfo.szUserNo, "%s", T2A(strUser));
		sprintf_s(stModelInfo.szModelName, "%s", T2A(m_strScanModelName));

		Poco::File fileModel(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
		if (fileModel.exists())
		{
			std::string strMd5 = calcFileMd5(strModelPath);
			strncpy(stModelInfo.szMD5, strMd5.c_str(), strMd5.length());
		}

		std::string strLog = "请求下载模板: ";
		strLog.append(stModelInfo.szModelName);
		g_pLogger->information(strLog);

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
			std::string strLog = "接收模板数据失败";
			g_pLogger->information(strLog);
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "接收模板数据失败！");
			dlg.DoModal();
			memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
			m_nRecvLen = 0;
			m_nWantLen = 0;
			EnableBtn(TRUE);
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
			if (nRecvResult > 0)
			{
				std::string strLog = "下载模板成功: ";
				strLog.append(stModelInfo.szModelName);
				g_pLogger->information(strLog);
				CNewMessageBox dlg;
				dlg.setShowInfo(1, 1, "下载成功！");
				dlg.DoModal();
			}
			else
			{
				std::string strLog = "下载模板失败: ";
				strLog.append(stModelInfo.szModelName);
				g_pLogger->information(strLog);
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, "下载模板失败！");
				dlg.DoModal();
			}
		}

	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "连接服务器失败，Detail: " + exc.displayText();
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "连接服务器失败！");
		dlg.DoModal();
	}

	EnableBtn(TRUE);
}

int CGetModelDlg::RecvData()
{
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
		std::string strLog = "接收数据异常(CGetModelDlg) ==> " + exc.displayText();
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

				//进度条控制
				m_progress.SetRange(0, pstModelInfo->nModelSize);
				m_progress.SetPos(0);
			}
			break;
		case RESULT_DOWNMODEL_FAIL:
			{
				std::string strLog = "服务器此科目模板不存在";
				g_pLogger->information(strLog);
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, "服务器此科目模板不存在！");
				dlg.DoModal();
				nResult = 2;
			}
			break;
		case RESULT_DOWNMODEL_NONEED:
			{
				std::string strLog = "本地存在此文件，不需要下载";
				g_pLogger->information(strLog);
				CNewMessageBox dlg;
				dlg.setShowInfo(1, 1, "本地存在此文件，不需要下载！");
				dlg.DoModal();
				nResult = 3;
			}
			break;
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_CREATE_MODEL)
	{
		switch (pstHead->usResult)
		{
			case RESULT_CREATE_MODEL_SUCCESS:
				{
					std::string strLog = "服务器端生成模板数据成功，请重新刷新数据并下载！";
					g_pLogger->information(strLog);
					CNewMessageBox dlg;
					dlg.setShowInfo(1, 1, "服务器端生成模板数据成功，请重新刷新数据并下载！");
					dlg.DoModal();
					nResult = 1;
				}
				break;
			case RESULT_CREATE_MODEL_FAIL:
				{
					std::string strLog = "服务器端生成模板数据失败，请重试！";
					g_pLogger->information(strLog);
					CNewMessageBox dlg;
					dlg.setShowInfo(2, 1, "服务器端生成模板数据失败，请重试！");
					dlg.DoModal();
					nResult = 1;
				}
				break;
			case RESULT_CREATE_MODEL_NOFIND:
				{
					std::string strLog = "服务器端无法生成模板数据，请确定是否进行过题卡设计操作！";
					g_pLogger->information(strLog);
					CNewMessageBox dlg;
					dlg.setShowInfo(2, 1, "服务器端无法生成模板数据，请确定是否进行过题卡设计操作！");
					dlg.DoModal();
					nResult = 1;
				}
				break;
			case RESULT_CREATE_MODEL_NONEED:
				{
					std::string strLog = "服务器端已经生成了模板，不需要进行再次生成！";
					g_pLogger->information(strLog);
					CNewMessageBox dlg;
					dlg.setShowInfo(1, 1, "服务器端已经生成了模板，不需要进行再次生成！");
					dlg.DoModal();
					nResult = 1;
				}
				break;
			case RESULT_CREATE_MODEL_DOING:
				{
					std::string strLog = "服务器端正在生成扫描模板，请稍后操作！";
					g_pLogger->information(strLog);
					CNewMessageBox dlg;
					dlg.setShowInfo(1, 1, "服务器端正在生成扫描模板，请稍后操作！");
					dlg.DoModal();
					nResult = 1;
				}
				break;
			case RESULT_ERROR_UNKNOWN:
				{
					std::string strLog = "创建扫描模板时发生未知错误，请重试！";
					g_pLogger->information(strLog);
					CNewMessageBox dlg;
					dlg.setShowInfo(2, 1, "创建扫描模板时发生未知错误，请重试！");
					dlg.DoModal();
					nResult = 1;
				}
				break;
			default:
				nResult = 1;
				break;
		}
	}
	return nResult;
}

int CGetModelDlg::RecvFile(pST_DOWN_MODEL pModelInfo)
{
	Poco::Timespan ts(60, 0);
	m_ss.setReceiveTimeout(ts);

	SAFE_RELEASE_ARRY(m_pFileRecv);
	m_pFileRecv = new char[DEFAULT_RECVBUFF + HEAD_SIZE];
	if (!m_pFileRecv)
	{
		TRACE("内存不足\n");
		std::string strLog = "内存不足";
		g_pLogger->information(strLog);
		return 0;
	}

#if 1
	int nCount = 0;
	int nBaseLen = HEAD_SIZE;
	int nWantlen = 0;
	int nRecvLen = 0;
	try
	{
		while (!g_nExitFlag)
		{
			int nLen;
			if (nRecvLen < nBaseLen)
			{
				nWantlen = nBaseLen - nRecvLen;
				nLen = m_ss.receiveBytes(m_pFileRecv + nRecvLen, nWantlen);
				if (nLen > 0)
				{
					nRecvLen += nLen;
					TRACE("get len: %d\n", nLen);
					m_progress.SetPos(nRecvLen - HEAD_SIZE);
					if (nRecvLen == nBaseLen)
					{
						if (nCount == 0)
						{
							ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_pFileRecv;
							nBaseLen += pstHead->uPackSize;
							if (pstHead->uPackSize > DEFAULT_RECVBUFF)
							{
								char* pOld = m_pFileRecv;
								m_pFileRecv = new char[pstHead->uPackSize + HEAD_SIZE];
								memcpy(m_pFileRecv, pOld, nRecvLen);
								SAFE_RELEASE_ARRY(pOld);
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
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "接收模板文件异常 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		return -1;
	}
#else
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
					int nRecvLen = m_nRecvLen - HEAD_SIZE;
					m_progress.SetPos(nRecvLen);
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
#endif
	TRACE("接收文件长度: %d\n", nRecvLen - HEAD_SIZE);

	USES_CONVERSION;
	int nResult = 0;
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_pFileRecv;
	if (pstHead->usCmd == USER_RESPONSE_DOWNMODEL)
	{
		switch (pstHead->usResult)
		{
			case RESULT_DOWNMODEL_RECV:
			{
				nResult = 1;

				//覆盖本地文件
				std::string strModelPath = T2A(g_strCurrentPath);
				strModelPath.append("Model\\");

				Poco::File fileModelPath(g_strModelSavePath);
				fileModelPath.createDirectories();

				strModelPath.append(pModelInfo->szModelName);
				Poco::File fileModel(strModelPath);
				if (fileModel.exists())
					fileModel.remove();

				ofstream out(strModelPath, std::ios::binary);
				if (!out)
				{
					nResult = 0;
					break;
				}
				std::stringstream buffer;
				buffer.write(m_pFileRecv + HEAD_SIZE, pstHead->uPackSize);
				int n = buffer.str().length();
				out << buffer.str();
				out.close();
			}
			break;
		}
	}
	return nResult;
}

void CGetModelDlg::OnBnClickedBtnExit()
{
	m_ss.close();
	OnOK();
}

void CGetModelDlg::InitUI()
{
	USES_CONVERSION;
	if (g_lExamList.size() == 0)
		return ;

	m_comboExamName.ResetContent();

	int nExamItem = 0;
	EXAM_LIST::iterator itExam = g_lExamList.begin();
	for (int i = 0; itExam != g_lExamList.end(); itExam++, i++)
	{
		CString strName = A2T((*itExam)->strExamName.c_str());
		if (i == 0)
			m_strExamName = strName;

		int nCount = m_comboExamName.GetCount();
		m_comboExamName.InsertString(nCount, strName);
		m_comboExamName.SetItemDataPtr(nCount, (void*)(*itExam));

		//一开始默认初始化到模板所在列表
		if ((*itExam)->nExamID == _pCurrExam_->nExamID)
		{
			nExamItem = nCount;
			m_strExamName = strName;
		}
	}
	m_comboExamName.SetCurSel(nExamItem);

	bool bFindSubId = false;
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemDataPtr(nExamItem);
	if (pExamInfo)
	{
		m_comboSubject.ResetContent();
		SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
		for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
		{
			EXAM_SUBJECT* pSubject = *itSub;
			CString strSubjectName = A2T((*itSub)->strSubjName.c_str());

			int nCount = m_comboSubject.GetCount();
			m_comboSubject.InsertString(nCount, strSubjectName);
			m_comboSubject.SetItemDataPtr(nCount, pSubject);

			//一开始默认初始化到模板所在列表
			if (_pCurrSub_->nSubjID == pSubject->nSubjID)
			{
				bFindSubId = true;
				m_comboSubject.SetCurSel(nCount);

				m_SubjectID = pSubject->nSubjID;
				m_strSubjectName = pSubject->strSubjName.c_str();
				m_strScanModelName = A2T(pSubject->strModelName.c_str());
			}
		}
		if (!bFindSubId)
		{
			m_comboSubject.SetCurSel(0);

			itSub = pExamInfo->lSubjects.begin();
			if (itSub != pExamInfo->lSubjects.end())
			{
				m_SubjectID = (*itSub)->nSubjID;
				m_strSubjectName = (*itSub)->strSubjName.c_str();
				m_strScanModelName = A2T((*itSub)->strModelName.c_str());
			}
		}

		m_nExamID = pExamInfo->nExamID;
		m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
		m_strGradeName = pExamInfo->strGradeName.c_str();
	}

	UpdateData(FALSE);
}

BOOL CGetModelDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		case WM_MOUSEMOVE:
			m_Tip.RelayEvent(pMsg);
	}
	return CTipBaseDlg::PreTranslateMessage(pMsg);
}

void CGetModelDlg::EnableBtn(BOOL bShow)
{
	GetDlgItem(IDC_BTN_DOWN)->EnableWindow(bShow);
//	GetDlgItem(IDC_BTN_Exit)->EnableWindow(bShow);
}
