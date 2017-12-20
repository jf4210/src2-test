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
	m_pTcpContext = NULL;
	m_PaperUserList.RemoveUser(this);
	std::string strLog = Poco::format("file sender close, Current connecter: %z", m_PaperUserList.m_UserList.size());
	strLog.append("\tfileName: ");
	strLog.append(m_szFileName);

	if (m_pf)
	{
		fclose(m_pf);
		m_pf = NULL;
	}
	int nUnRecvLen = m_dwTotalFileSize - m_dwRecvFileSize;
	if (nUnRecvLen > 0)
	{
		std::string strRecvFileInfo = Poco::format("\t未接收: %d(%d/%d)", (int)(m_dwTotalFileSize - m_dwRecvFileSize), (int)m_dwRecvFileSize, (int)m_dwTotalFileSize);
		strLog.append(strRecvFileInfo);
	}
	
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
			{
				char szLog[200] = { 0 };
				strcpy_s(szLog, "非法数据包，连接关闭(包头非法)");
				g_Log.LogOut(szLog);
				std::cout << szLog << std::endl;
				m_pTcpContext->ReleaseConnections();
				return;
			}

			if (header.usCmd != REQUEST_UPLOADANS)
			{
				char szLog[200] = { 0 };
				strcpy_s(szLog, "非法数据包，连接关闭(包命令非法)");
				g_Log.LogOut(szLog);
				std::cout << szLog << std::endl;
				m_pTcpContext->ReleaseConnections();
				return;
			}

			ST_FILE_INFO AnswerInfo = *(ST_FILE_INFO*)(m_PacketBuf + HEAD_SIZE);
			strcpy_s(m_szFileName, AnswerInfo.szFileName);
			char szLog[300] = { 0 };
			sprintf(szLog, "start recv file: %s(%.3fM)", AnswerInfo.szFileName, AnswerInfo.dwFileLen / (1024.0 * 1024.0));
			g_Log.LogOut(szLog);
			std::cout << szLog << std::endl;

			SetAnswerInfo(AnswerInfo);
			if (!SendResult(RESPONSE_UPLOADANS, RESULT_SUCCESS))
				return;
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
#if 1
					m_end = clock();
					char szLog[300] = { 0 };
					sprintf(szLog, "写文件失败，连接关闭: %s, Timed = %d", m_szFileName, m_end - m_start);
					g_Log.LogOut(szLog);
					std::cout << szLog << std::endl;

					m_bIOError = true;
					m_pTcpContext->ReleaseConnections();	//写文件错误直接断开，让客户端重连
					return;

					if (!m_bIOError)
					{
						m_end = clock();
						char szLog[300] = { 0 };
						sprintf(szLog, "WriteAnswerFile: %s failed. Timed = %d", m_szFileName, (m_end - m_start) / 1000.0);
						g_Log.LogOut(szLog);

						m_bIOError = true;											
					}
					if (m_dwRecvFileSize == m_dwTotalFileSize)
					{
						m_nAnswerPacketError = 1;
						ClearAnswerInfo();
						if (!SendResult(RESPONSE_UPLOADANS, RESULT_ERROR_FILEIO))
							return;
						m_pTcpContext->RecvData(m_PacketBuf, HEAD_SIZE + sizeof(ST_FILE_INFO));		//再次投递，准备重新接收文件
						return;
					}
	
					int nWantDataLen = m_dwTotalFileSize - m_dwRecvFileSize;
					if (nWantDataLen > ANSWERPACK_LEN)
					{
						nWantDataLen = ANSWERPACK_LEN;
					}
					m_pTcpContext->RecvData(m_PacketBuf, nWantDataLen);

 					return;
#else
					m_end = clock();
					char szLog[300] = { 0 };
					sprintf(szLog, "WriteAnswerFile: %s failed. Timed = %d", m_szFileName, m_end - m_start);
					g_Log.LogOut(szLog);

					m_nAnswerPacketError = 1;
					ClearAnswerInfo();
					SendResult(RESPONSE_UPLOADANS, RESULT_ERROR_FILEIO);

					m_pTcpContext->RecvData(m_PacketBuf, HEAD_SIZE + sizeof(ST_FILE_INFO));		//再次投递，准备重新接收文件	*****有问题？
					return;
#endif
				}

				if (m_dwRecvFileSize == m_dwTotalFileSize)
				{
					//文件接收完成
					if (CheckAnswerFile())
					{
						if (!SendResult(NOTIFY_RECVANSWERFIN, RESULT_SUCCESS))
							return;
						m_end = clock();

						float fSpeed = (m_dwTotalFileSize / 1024.0) / ((m_end - m_start) / 1000.0);
						char szSpeed[50] = { 0 };
						if (fSpeed > 1024)
							sprintf_s(szSpeed, "%.3fMB/s", fSpeed / 1024);
						else
							sprintf_s(szSpeed, "%.3fKB/s", fSpeed);
						char szLog[300] = { 0 };
						sprintf(szLog, "recv file: %s completed. time = %.2f, transmissionSpeed = %s", m_szFilePath, (m_end - m_start)/1000.0, szSpeed);
						g_Log.LogOut(szLog);
						std::cout << szLog << std::endl;

						//接收到模板，移动到指定目录
						std::string strName = m_szFileName;
						if (strName.find(".mod") != std::string::npos)
						{
							Poco::File filePath(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strModelSavePath));
							if (!filePath.exists())
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
								std::string strExamID;
								std::string strSubjectID;
								int nPos = 0;
								int nOldPos = 0;
								nPos = strModelName.find("_N_");
								if (nPos != std::string::npos)	//新模板名称
								{
									int nPos2 = strModelName.find("_", nPos + 3);

									strExamID = strModelName.substr(nPos + 3, nPos2 - nPos - 3);
									nOldPos = nPos2;
									nPos2 = strModelName.find(".", nPos2 + 1);
									strSubjectID = strModelName.substr(nOldPos + 1, nPos2 - nOldPos - 1);
								}
								else
								{
									nPos = strModelName.find("_");
									strExamID = strModelName.substr(0, nPos);
									nOldPos = nPos;
									nPos = strModelName.find(".", nPos + 1);
									strSubjectID = strModelName.substr(nOldPos + 1, nPos - nOldPos - 1);
								}

								std::string strLog;

								pMODELINFO pModelInfo = NULL;
								char szIndex[50] = { 0 };
								sprintf(szIndex, "%s_%s", strExamID.c_str(), strSubjectID.c_str());
								MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
								if (itFind == _mapModel_.end())
								{
									pModelInfo = new MODELINFO;
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
									pModelInfo = itFind->second;
									pModelInfo->strName = m_szFileName;
									pModelInfo->strPath = strModelNewPath;
									pModelInfo->strMd5 = m_szAnswerMd5;

									strLog = "modify modelinfo. modelName = ";
									strLog.append(m_szFileName);
								}
								g_Log.LogOut(strLog);
								std::cout << strLog << std::endl;

								//++模板上传完成后，需要解压，向zimg提交图片给后端
								pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
								pDecompressTask->nType = 2;
								pDecompressTask->strFilePath = strModelNewPath;
								pDecompressTask->strFileBaseName = m_szFileName;
								pDecompressTask->strFileBaseName = pDecompressTask->strFileBaseName.substr(0, nPos);	//pDecompressTask->strFileBaseName.length() - 4
								pDecompressTask->strSrcFileName = m_szFileName;
								g_fmDecompressLock.lock();
								g_lDecompressTask.push_back(pDecompressTask);
								g_fmDecompressLock.unlock();
								//--
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
							std::string strExtFileName = m_szFileName;
							int nPos = strExtFileName.rfind('.');
							if (nPos != std::string::npos)
							{
								strExtFileName = strExtFileName.substr(nPos, strExtFileName.length());
								if (strExtFileName == ".typkg")		//武汉天喻版本，收到文件后重命名	8.18	*******	注意	********
								{
									try
									{
										Poco::Path filePath(CMyCodeConvert::Gb2312ToUtf8(m_szFilePath));
										std::string strNewFilePath = SysSet.m_strPapersBackupPath + "\\" + filePath.getBaseName() + ".zip";

										Poco::File newFile(CMyCodeConvert::Gb2312ToUtf8(strNewFilePath));
										if (newFile.exists())
											newFile.remove(true);

										Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(m_szFilePath));
										fileList.renameTo(CMyCodeConvert::Gb2312ToUtf8(strNewFilePath));
									}
									catch (Poco::Exception& exc)
									{
										std::string strFileName = m_szFileName;
										std::string strErrInfo = Poco::format("重命名并移动试卷袋(%s)失败, %s", strFileName, exc.message());
										g_Log.LogOutError(strErrInfo);
										std::cout << strErrInfo << std::endl;
									}
								}
								else if (strExtFileName == ".pkg")
								{
									std::string strUploadPath = SysSet.m_strUpLoadPath + "\\";
									strUploadPath.append(m_szFileName);
									try
									{
										Poco::File filePapers(CMyCodeConvert::Gb2312ToUtf8(m_szFilePath));
										filePapers.moveTo(CMyCodeConvert::Gb2312ToUtf8(strUploadPath));
									}
									catch (Poco::Exception& exc)
									{
										std::string strFileName = m_szFileName;
										std::string strErrInfo = Poco::format("移动试卷袋(%s)失败,%s", strFileName, exc.message());
										g_Log.LogOutError(strErrInfo);
										std::cout << strErrInfo << std::endl;
									}

									pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
									pDecompressTask->strFilePath = strUploadPath;
									pDecompressTask->strFileBaseName = m_szFileName;
									pDecompressTask->strFileBaseName = pDecompressTask->strFileBaseName.substr(0, nPos);	//pDecompressTask->strFileBaseName.length() - 4
									pDecompressTask->strSrcFileName = m_szFileName;
									g_fmDecompressLock.lock();
									g_lDecompressTask.push_back(pDecompressTask);
									g_fmDecompressLock.unlock();
								}
								else if (strExtFileName == ".dmp")
								{
									Poco::LocalDateTime now;
									std::string strDay = Poco::format("%04d-%02d-%02d", now.year(), now.month(), now.day());
									std::string strDumpDir = SysSet.m_strCurrentDir + "DumpDir\\" + strDay;	//utf8
									std::string strDumpFilePath = strDumpDir + "\\" + CMyCodeConvert::Gb2312ToUtf8(m_szFileName);
									std::string strTmp = CMyCodeConvert::Utf8ToGb2312(strDumpFilePath);
									try
									{
										Poco::File fDumpDir(CMyCodeConvert::Gb2312ToUtf8(strDumpDir));
										fDumpDir.createDirectories();

										Poco::File fileModel(strDumpFilePath);
										if (fileModel.exists())
										{
											//strLog.append("\n移除原文件.");
											fileModel.remove(true);
										}

										Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(m_szFilePath));
										fileList.renameTo(strDumpFilePath);
									}
									catch (Poco::Exception &e)
									{
										std::cout << "接收客户端.dmp检查路径时异常: " << e.displayText() << std::endl;
										//strLog.append("\n接收客户端.dmp检查路径时异常: " + e.displayText());
									}
								}
								else	//上传的模板图像
								{
									//								MAP_MODEL_PIC::iterator itFind = _mapModelPic__.find(szIndex);
									std::string strModelPicName = m_szFileName;
									std::string strExamID;
									std::string strSubjectID;
									std::string strPicIndex;
									std::string strPicOriName;
									std::string strExtName;
									int nPos = 0;
									int nOldPos = 0;
									nPos = strModelPicName.find("_");
									if (nPos != std::string::npos)
									{
										strExamID = strModelPicName.substr(0, nPos);
										nOldPos = nPos;
										nPos = strModelPicName.find("_", nPos + 1);
										strSubjectID = strModelPicName.substr(nOldPos + 1, nPos - nOldPos - 1);
										nOldPos = nPos;
										nPos = strModelPicName.find("_", nPos + 1);
										strPicIndex = strModelPicName.substr(nOldPos + 1, nPos - nOldPos - 1);

										nPos = strModelPicName.find("_#_");
										nOldPos = nPos;
										nPos = strModelPicName.find(".", nPos + 3);
										strPicOriName = strModelPicName.substr(nOldPos + 3, nPos - nOldPos - 3);
										strExtName = strModelPicName.substr(nPos);


										std::string strLog;
										strLog = "获取到模板图片(" + strExamID + "_" + strSubjectID + "_" + strPicIndex + strExtName + ")";
										std::cout << strLog << std::endl;

										std::string strPicSaveDir = SysSet.m_strModelSavePath + "\\" + strExamID;
										std::string strPicSavePath = strPicSaveDir + "\\" + strExamID + "_" + strSubjectID + "_" + strPicIndex + strExtName;

										try
										{
											Poco::File fileModelPicPath(CMyCodeConvert::Gb2312ToUtf8(strPicSaveDir));
											fileModelPicPath.createDirectories();

											Poco::File fileModel(CMyCodeConvert::Gb2312ToUtf8(strPicSavePath));
											if (fileModel.exists())
											{
												strLog.append("\n移除原文件.");
												fileModel.remove(true);
											}

											Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(m_szFilePath));
											fileList.renameTo(CMyCodeConvert::Gb2312ToUtf8(strPicSavePath));
										}
										catch (Poco::Exception &e)
										{
											std::cout << "接收模板图像检查路径时异常: " << e.displayText() << std::endl;
											strLog.append("\n接收模板图像检查路径时异常: " + e.displayText());
										}
										g_Log.LogOut(strLog);
									}
								}
							}
							else
							{
								std::string strFileName = m_szFileName;
								std::string strErrInfo = Poco::format("接收到非法试卷袋(%s)，丢弃不处理", strFileName);
								g_Log.LogOutError(strErrInfo);
								std::cout << strErrInfo << std::endl;
							}
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
						if (!SendResult(NOTIFY_RECVANSWERFIN, RESULT_ERROR_CHECKMD5))
							return;
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
	int nResult = m_pTcpContext->SendData(m_ResponseBuf, HEAD_SIZE);
	if (nResult < 0)
	{
		std::string strLog = "发送数据失败，套接字可能关闭，context可能被释放, 文件名: ";
		strLog.append(m_szFileName);
		g_Log.LogOutError(strLog);
		return false;
	}
	return true;
}

void CPaperUser::SetAnswerInfo(ST_FILE_INFO info)
{
	string strFilePath = SysSet.m_strRecvFilePath;
	strFilePath.append(info.szFileName);

	try
	{
		Poco::File upLoadPath(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strRecvFilePath));
		if (!upLoadPath.exists())
			upLoadPath.createDirectories();

		Poco::File filePath(CMyCodeConvert::Gb2312ToUtf8(strFilePath));
		if (filePath.exists())
			filePath.remove(true);				
	}
	catch (Poco::Exception &exc)
	{
		std::string strLog;
		strLog.append("upLoadPath createDirectories or filePath remove error: " + exc.displayText());
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
	}

	strcpy(m_szAnswerMd5, info.szMD5);
	sprintf(m_szFilePath, "%s", strFilePath.c_str());
	m_dwTotalFileSize = info.dwFileLen;
	m_dwRecvFileSize = 0;

	m_start = clock();

	m_bIOError = false;
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
		std::string strLog = "文件首次打开异常，文件名: ";
		strLog.append(m_szFileName);
		g_Log.LogOutError(strLog);
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
	std::cout << "****ClearAnswerInfo...\n" << std::endl;
}

int CPaperUser::WriteAnswerFile(char* pData, int nDataLen)
{
	if (m_bIOError)
	{
		m_dwRecvFileSize += nDataLen;
		return 0;
	}

	try
	{
#if 1
		if (!m_pf)
		{
			m_pf = fopen(m_szFilePath, "wb");
			if (!m_pf)
			{
				std::string strLog = "文件句柄打开失败，文件名: ";
				strLog.append(m_szFileName);
				g_Log.LogOutError(strLog);
				m_dwRecvFileSize += nDataLen;	//++8.18
				return 0;
			}
		}

		int ret = fwrite(pData, 1, nDataLen, m_pf);
		if (ret <= 0)
		{
			std::string strLog = "写文件失败，文件名: ";
			strLog.append(m_szFileName);
			g_Log.LogOutError(strLog);
			OutputDebugStringA(strLog.c_str());


			m_dwRecvFileSize += nDataLen;	//++8.18

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
		m_dwRecvFileSize += nDataLen;	//++8.18
		return 0;
	}

	return nDataLen;
}
