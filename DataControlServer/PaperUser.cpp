#include "PaperUser.h"
#include "DCSDef.h"

CPaperUser::CPaperUser(ITcpContext* pTcpContext, CListPaperUser& PaperUserList)
: m_dwTotalFileSize(0)
, m_dwRecvFileSize(0)
, m_nAnswerPacketError(0)
, m_pTcpContext(pTcpContext)
, m_PaperUserList(PaperUserList)
, m_pf(NULL)
{
	ZeroMemory(m_szFileName, sizeof(m_szFileName));
	ZeroMemory(m_szFilePath, sizeof(m_szFilePath));
	if (m_pTcpContext)
	{
		m_pTcpContext->SetTcpContextNotify(this);
	}
}

CPaperUser::~CPaperUser()
{
	if (m_pf)
	{
		fclose(m_pf);
		m_pf = NULL;
	}
}

void CPaperUser::OnClose(void)
{
	m_PaperUserList.RemoveUser(this);
	std::string strLog = Poco::format("file sender close, Current connecter: %z", m_PaperUserList.m_UserList.size());
	
	std::cout << strLog << std::endl;
	g_Log.LogOut(strLog);
	delete this;
}

void CPaperUser::OnWrite(int nDataLen)
{
}

void CPaperUser::OnRead(char* pData, int nDataLen)
{
	if (nDataLen == 0)
	{
		m_pTcpContext->RecvData(m_PacketBuf, HEAD_SIZE + sizeof(ST_FILE_INFO));
	}
	else
	{
		if (m_nAnswerPacketError == 1 || m_dwTotalFileSize == 0)
		{
			//头信息
			ST_CMD_HEADER header = *(ST_CMD_HEADER*)m_PacketBuf;
			if (header.usVerifyCode != VERIFYCODE)
				return;

			if (header.usCmd != REQUEST_UPLOADANS)
				return;

			ST_FILE_INFO AnswerInfo = *(ST_FILE_INFO*)(m_PacketBuf + HEAD_SIZE);
			strcpy_s(m_szFileName, AnswerInfo.szFileName);
			char szLog[300] = { 0 };
			sprintf(szLog, "start recv file: %s", AnswerInfo.szFileName);
			g_Log.LogOut(szLog);
			std::cout << szLog << std::endl;

			SetAnswerInfo(AnswerInfo);
			SendResult(RESPONSE_UPLOADANS, RESULT_SUCCESS);
			m_nAnswerPacketError = 0;
			int nWantDataLen = m_dwTotalFileSize - m_dwRecvFileSize;
			if (nWantDataLen > ANSWERPACK_LEN)
			{
				nWantDataLen = ANSWERPACK_LEN;
			}
			m_pTcpContext->RecvData(m_PacketBuf, nWantDataLen);
		}
		else
		{
			int nWantDataSize = m_dwTotalFileSize - m_dwRecvFileSize;
			if (nDataLen > nWantDataSize)
			{
			}
			else
			{
				if (WriteAnswerFile(m_PacketBuf, nDataLen) <= 0)
				{
					m_nAnswerPacketError = 1;
					ClearAnswerInfo();
					SendResult(RESPONSE_UPLOADANS, RESULT_ERROR_FILEIO);
					m_end = clock();
					char szLog[300] = { 0 };
					sprintf(szLog, "WriteAnswerFile: %s failed. Timed = %d", m_szFilePath, m_end - m_start);
					g_Log.LogOut(szLog);
					return;
				}

				if (m_dwRecvFileSize == m_dwTotalFileSize)
				{
					//文件接收完成
					if (CheckAnswerFile())
					{
						SendResult(NOTIFY_RECVANSWERFIN, RESULT_SUCCESS);
						m_end = clock();
						char szLog[300] = { 0 };
						sprintf(szLog, "recv file: %s completed. time = %.2f", m_szFilePath, (m_end - m_start)/1000.0);
						g_Log.LogOut(szLog);
						std::cout << szLog << std::endl;

						//接收到模板，移动到指定目录
						std::string strName = m_szFileName;
						if (strName.find(".mod") != std::string::npos)
						{
							Poco::File filePath(SysSet.m_strModelSavePath);
							filePath.createDirectories();

							std::string strModelNewPath = SysSet.m_strModelSavePath + "\\";
							strModelNewPath.append(m_szFileName);

							try
							{
								std::string strUtf8OldPath = CMyCodeConvert::Gb2312ToUtf8(m_szFilePath);
								std::string strUtf8ModelPath = CMyCodeConvert::Gb2312ToUtf8(strModelNewPath);

								Poco::File modelPicPath(strUtf8OldPath);
								modelPicPath.moveTo(strUtf8ModelPath);

								std::string strModelName = m_szFileName;
								int nPos = 0;
								int nOldPos = 0;
								nPos = strModelName.find("_");
								std::string strExamID = strModelName.substr(0, nPos);
								nOldPos = nPos;
								nPos = strModelName.find(".", nPos + 1);
								std::string strSubjectID = strModelName.substr(nOldPos + 1, nPos - nOldPos - 1);

								std::string strLog;

								char szIndex[50] = { 0 };
								sprintf(szIndex, "%s_%s", strExamID.c_str(), strSubjectID.c_str());
								MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
								if (itFind == _mapModel_.end())
								{
									pMODELINFO pModelInfo = new MODELINFO;
									pModelInfo->nExamID = atoi(strExamID.c_str());
									pModelInfo->nSubjectID = atoi(strSubjectID.c_str());
									pModelInfo->strName = m_szFileName;
									pModelInfo->strPath = strModelNewPath;
									pModelInfo->strMd5 = m_szAnswerMd5;

									_mapModelLock_.lock();
									_mapModel_.insert(MAP_MODEL::value_type(szIndex, pModelInfo));
									_mapModelLock_.unlock();

									strLog = "get a new modelinfo. modelName = ";
									strLog.append(m_szFileName);
								}
								else
								{
									pMODELINFO pModelInfo = itFind->second;
									pModelInfo->strName = m_szFileName;
									pModelInfo->strPath = strModelNewPath;
									pModelInfo->strMd5 = m_szAnswerMd5;

									strLog = "modify modelinfo. modelName = ";
									strLog.append(m_szFileName);
								}
								g_Log.LogOut(strLog);
								std::cout << strLog << std::endl;
							}
							catch (Poco::Exception &exc)
							{
								std::string strLog;
								strLog.append("model move error: " + exc.displayText() + "\tmodelPath: ");
								strLog.append(m_szFilePath);
								g_Log.LogOutError(strLog);
							}
						}
						else
						{
							#ifndef TEST_FILE_PRESSURE
							pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
							pDecompressTask->strFilePath = m_szFilePath;
							pDecompressTask->strFileName = m_szFileName;
							pDecompressTask->strFileName = pDecompressTask->strFileName.substr(0, pDecompressTask->strFileName.length() - 4);
							g_fmDecompressLock.lock();
							g_lDecompressTask.push_back(pDecompressTask);
							g_fmDecompressLock.unlock();
							#else
							Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(m_szFilePath));
							if (fileList.exists())
							{
								fileList.remove();
								std::string strLog = "文件接收完成，删除文件，文件名: ";
								strLog.append(m_szFileName);
								g_Log.LogOut(strLog);
							}
							#endif
						}						
					}
					else
					{
						m_nAnswerPacketError = 1;
						SendResult(NOTIFY_RECVANSWERFIN, RESULT_ERROR_CHECKMD5);
					}
					ClearAnswerInfo();
					m_pTcpContext->RecvData(m_PacketBuf, HEAD_SIZE + sizeof(ST_FILE_INFO));
				}
				else
				{
					int nWantDataLen = m_dwTotalFileSize - m_dwRecvFileSize;
					if (nWantDataLen > ANSWERPACK_LEN)
					{
						nWantDataLen = ANSWERPACK_LEN;
					}
					m_pTcpContext->RecvData(m_PacketBuf, nWantDataLen);
				}
			}
		}
	}
}

bool CPaperUser::SendResult(unsigned short usCmd, int nResultCode)
{
	ST_CMD_HEADER* resultCmd = (ST_CMD_HEADER*)m_ResponseBuf;
	resultCmd->usCmd	= VERIFYCODE;
	resultCmd->usCmd	= usCmd;
	resultCmd->uPackSize	= 0;
	resultCmd->usResult		= nResultCode;
	m_pTcpContext->SendData(m_ResponseBuf, HEAD_SIZE);
	return true;
}

void CPaperUser::SetAnswerInfo(ST_FILE_INFO info)
{
	string strFilePath = SysSet.m_strUpLoadPath + "\\";
	strFilePath.append(info.szFileName);

	try
	{
		Poco::File upLoadPath(SysSet.m_strUpLoadPath);
		if (!upLoadPath.exists())
			upLoadPath.createDirectories();

		Poco::File filePath(strFilePath);
		if (filePath.exists())
			filePath.remove(true);

		
	}
	catch (Poco::Exception &exc)
	{
		std::string strLog;
		strLog.append("upLoadPath createDirectories or filePath remove error: " + exc.displayText());
		g_Log.LogOut(strLog);
		std::cout << strLog << std::endl;
	}

	strcpy(m_szAnswerMd5, info.szMD5);
	sprintf(m_szFilePath, "%s", strFilePath.c_str());
	m_dwTotalFileSize = info.dwFileLen;
	m_dwRecvFileSize = 0;

	m_start = clock();

	if (m_pf)
	{
		fclose(m_pf);
		m_pf = NULL;
	}
	try
	{
		m_pf = fopen(m_szFilePath, "wb");
	}
	catch (...)
	{
		if (m_pf)
		{
			fclose(m_pf);
			m_pf = NULL;
		}
	}
}

bool CPaperUser::CheckAnswerFile(void)
{
	try
	{
		Poco::MD5Engine md5;
		Poco::DigestOutputStream dos(md5);

		std::ifstream istr(m_szFilePath, std::ios::binary);
		if (!istr)
		{
			string strLog = "calc MD5 failed 1: ";
			strLog.append(m_szFilePath);
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			return false;
		}
		Poco::StreamCopier::copyStream(istr, dos);
		dos.close();

		string strMd5 = Poco::DigestEngine::digestToHex(md5.digest());
		if (strcmp(strMd5.c_str(), m_szAnswerMd5))
		{
			string strLog = "calc MD5 failed 2: ";
			strLog.append(m_szFilePath);
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			return false;
		}
	}
	catch (...)
	{
		string strLog = "calc MD5 failed 3: ";
		strLog.append(m_szFilePath);
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return false;
	}

	return true;
}

void CPaperUser::ClearAnswerInfo(void)
{
	memset(m_szAnswerMd5, 0, LEN_MD5);
	memset(m_szFilePath, 0, 255);
	m_dwTotalFileSize = 0;
	m_dwRecvFileSize = 0;

	if (m_pf)
	{
		fclose(m_pf);
		m_pf = NULL;
	}
}

int CPaperUser::WriteAnswerFile(char* pData, int nDataLen)
{
	try
	{
#if 1
// 		if (m_dwRecvFileSize == 0)
// 		{
// 			m_pf = fopen(m_szFilePath, "wb");
// 		}
		if (!m_pf)
		{
			m_pf = fopen(m_szFilePath, "wb");
			if (!m_pf)	return 0;
//			return 0;
		}

		int ret = fwrite(pData, 1, nDataLen, m_pf);
		if (ret <= 0)
		{
			std::string strLog = "写文件失败，文件名: ";
			strLog.append(m_szFileName);
			g_Log.LogOutError(strLog);
			OutputDebugStringA(strLog.c_str());

			fclose(m_pf);
			m_pf = NULL;
			return ret;
		}
		m_dwRecvFileSize += nDataLen;

		if (m_dwRecvFileSize == m_dwTotalFileSize)
		{
			fclose(m_pf);
			m_pf = NULL;
		}

#else
		FILE *pf = NULL;
		if (m_dwRecvFileSize == 0)
		{
			pf = fopen(m_szFilePath, "wb");
		}
		else
			pf = fopen(m_szFilePath, "ab");
		if (!pf)
		{
			return 0;
		}

		int ret = fwrite(pData, 1, nDataLen, pf);
		if (ret <= 0)
		{
			fclose(pf);
			return ret;
		}
		fclose(pf);
		m_dwRecvFileSize += nDataLen;
#endif
	}
	catch (...)
	{
		std::string strLog = "写文件失败，文件名: ";
		strLog.append(m_szFileName);
		g_Log.LogOutError(strLog);
		OutputDebugStringA(strLog.c_str());
		
		if (m_pf)
		{
			fclose(m_pf);
			m_pf = NULL;
		}
		return 0;
	}

	return nDataLen;
}
