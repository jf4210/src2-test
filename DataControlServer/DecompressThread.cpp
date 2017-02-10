#include "DecompressThread.h"
#include <algorithm>
#ifndef USE_POCO_UNZIP
#include "miniunz/miniunz.c"
#endif

bool SortbyNumASC(const std::string& x, const std::string& y)
{
	char szX[MAX_PATH] = { 0 };
	char szY[MAX_PATH] = { 0 };
	sprintf_s(szX, "%s", x.c_str()/*T2A(x)*/);
	sprintf_s(szY, "%s", y.c_str()/*T2A(y)*/);
	int nLenX = x.length();	// x.GetLength();
	int nLenY = y.length();	// y.GetLength();

	//	TRACE(_T("compare: %s, %s\n"), x, y);

	int nFlag = 0;
	while (nLenX && nLenY)
	{
		char szXPart[MAX_PATH] = { 0 };
		char szYPart[MAX_PATH] = { 0 };
		sscanf(szX, "%[A-Za-z]", szXPart);
		sscanf(szY, "%[A-Za-z]", szYPart);
		if (strlen(szXPart) && strlen(szYPart))
		{
			int nResult = stricmp(szXPart, szYPart);
			if (nResult == 0)
			{
				int nX = strlen(szXPart);
				int nY = strlen(szYPart);
				int nXAll = strlen(szX);
				int nYAll = strlen(szY);
				memmove(szX, szX + strlen(szXPart), nXAll - nX);
				memmove(szY, szY + strlen(szYPart), nYAll - nY);
				szX[nXAll - nX] = '\0';
				szY[nYAll - nY] = '\0';
				nLenX = strlen(szX);
				nLenY = strlen(szY);
			}
			else
			{
				return nResult < 0 ? true : false;
			}
		}
		else if (strlen(szXPart))
			return false;
		else if (strlen(szYPart))
			return true;
		else
		{
			sscanf(szX, "%[0-9]", szXPart);
			sscanf(szY, "%[0-9]", szYPart);
			if (strlen(szXPart) && strlen(szYPart))
			{
				int x = atoi(szXPart);
				int y = atoi(szYPart);
				if (x == y)
				{
					if (strlen(szXPart) == strlen(szYPart))
					{
						int nX = strlen(szXPart);
						int nY = strlen(szYPart);
						int nXAll = strlen(szX);
						int nYAll = strlen(szY);
						memmove(szX, szX + strlen(szXPart), nXAll - nX);
						memmove(szY, szY + strlen(szYPart), nYAll - nY);
						szX[nXAll - nX] = '\0';
						szY[nYAll - nY] = '\0';
						nLenX = strlen(szX);
						nLenY = strlen(szY);
					}
					else
					{
						return strlen(szXPart) > strlen(szYPart);		//大小相同，长度越大越靠前
					}
				}
				else
					return x < y;
			}
			else if (strlen(szXPart))
				return false;
			else if (strlen(szYPart))
				return true;
			else
			{
				sscanf(szX, "%[^0-9A-Za-z]", szXPart);
				sscanf(szY, "%[^0-9A-Za-z]", szYPart);
				int nResult = stricmp(szXPart, szYPart);
				if (nResult == 0)
				{
					int nX = strlen(szXPart);
					int nY = strlen(szYPart);
					int nXAll = strlen(szX);
					int nYAll = strlen(szY);
					memmove(szX, szX + strlen(szXPart), nXAll - nX);
					memmove(szY, szY + strlen(szYPart), nYAll - nY);
					szX[nXAll - nX] = '\0';
					szY[nYAll - nY] = '\0';
					nLenX = strlen(szX);
					nLenY = strlen(szY);
				}
				else
				{
					char* p1 = szXPart;
					char* p2 = szYPart;
					while (*p1 != '\0' && *p2 != '\0')
					{
						if (*p1 == '-'&& *p2 != '-')
							return false;
						else if (*p1 != '-' && *p2 == '-')
							return true;
						else if (*p1 == '=' && *p2 != '=')
							return false;
						else if (*p1 != '=' && *p2 == '=')
							return true;
						else if (*p1 == '+' && *p2 != '+')
							return false;
						else if (*p1 != '+' && *p2 == '+')
							return true;
						else if (*p1 > *p2)
							return false;
						else if (*p1 < *p2)
							return true;
						else
						{
							p1++;
							p2++;
						}
					}
					if (*p1 == '\0' && *p2 != '\0')
					{
						if (*p2 == ' ')
							return false;
						else
							return true;
					}
					else if (*p1 != '\0' && *p2 == '\0')
					{
						if (*p1 == ' ')
							return true;
						else
							return false;
					}
					//return nResult < 0?true:false;
				}
			}
		}
	}

	return x.length() < y.length();
}

bool SortByPaper(const pPAPER_INFO& x, const pPAPER_INFO& y)
{
	return SortbyNumASC(x->strName, y->strName);
}

class DecompressHandler
{
public:
	DecompressHandler(pPAPERS_DETAIL pPapers) : _pPapers(pPapers)
	{
	}

	~DecompressHandler()
	{
	}

	void onError(const void*, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info)
	{
		std::string strLog = "Decompress ERR: " + info.second;
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
	}

	inline void onOk(const void*, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& info)
	{
		std::string strLog = "Decompress OK: " + info.second.toString(Poco::Path::PATH_UNIX);
		g_Log.LogOut(strLog);
//		std::cout << strLog << std::endl;

		if (info.second.toString().find("papersInfo.dat") != std::string::npos)
			return;

		pPIC_DETAIL pPic = new PIC_DETAIL;
		pPic->strFileName = info.second.toString();
		pPic->strFilePath = _pPapers->strPapersPath + "\\" + pPic->strFileName;

		int nPos = pPic->strFileName.find("_");
		std::string strPaperName = pPic->strFileName.substr(0, nPos);
		bool bFind = false;
		LIST_PAPER_INFO::iterator it = _pPapers->lPaper.begin();
		for (; it != _pPapers->lPaper.end(); it++)
		{
			if ((*it)->strName == strPaperName)
			{
				(*it)->lPic.push_back(pPic);
				bFind = true;
				break;
			}
		}
		if (!bFind)
		{
			pPAPER_INFO pPaper = new PAPER_INFO;
			pPaper->lPic.push_back(pPic);
			pPaper->strName = strPaperName;
			_pPapers->lPaper.push_back(pPaper);
		}
		_pPapers->nTotalPics++;						//图片数增加一张

// 		pSEND_HTTP_TASK pHttpTask = new SEND_HTTP_TASK;
// 		pHttpTask->nTaskType	= 1;
// 		pHttpTask->pPic			= pPic;
// 		pHttpTask->pPapers		= _pPapers;
// 		pHttpTask->strUri		= SysSet.m_strUpLoadHttpUri;
// 		g_fmHttpSend.lock();
// 		g_lHttpSend.push_back(pHttpTask);
// 		g_fmHttpSend.unlock();
	}
	pPAPERS_DETAIL _pPapers;
};


CDecompressThread::CDecompressThread()
{
	std::cout << "decompressThread start.\n";
}

CDecompressThread::~CDecompressThread()
{
	std::cout << "decompressThread exit.\n";
}

void CDecompressThread::run()
{
	while (!g_nExitFlag)
	{
		pDECOMPRESSTASK pTask = NULL;
		g_fmDecompressLock.lock();
		DECOMPRESSTASKLIST::iterator it = g_lDecompressTask.begin();
		for (; it != g_lDecompressTask.end();)
		{
			pTask = *it;
			it = g_lDecompressTask.erase(it);
			break;
		}
		g_fmDecompressLock.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(200);
			continue;
		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}
}

void CDecompressThread::HandleTask(pDECOMPRESSTASK pTask)
{
	std::string strOutDir = SysSet.m_strDecompressPath + "\\" + CMyCodeConvert::Gb2312ToUtf8(pTask->strFileBaseName);

	bool bNeedDecompress = true;
	pPAPERS_DETAIL pPapers = NULL;
	if (pTask->nType >= 3)
	{
		_mapResendPkgLock_.lock();
		MAP_RESEND_PKG::iterator itPkg = _mapResendPkg_.find(pTask->strFileBaseName);
		if (itPkg == _mapResendPkg_.end())
		{
			pPapers = new PAPERS_DETAIL;
			pPapers->strPapersName = pTask->strFileBaseName;
			pPapers->strPapersPath = CMyCodeConvert::Utf8ToGb2312(strOutDir);
			pPapers->strSrcPapersPath = pTask->strFilePath;
			pPapers->strSrcPapersFileName = pTask->strSrcFileName;

			_mapResendPkg_.insert(std::pair<std::string, pPAPERS_DETAIL>(pTask->strFileBaseName, pPapers));
		}
		else
		{
			pPapers = itPkg->second;
			bNeedDecompress = false;
		}
		_mapResendPkgLock_.unlock();
	}
	else
	{
		pPapers = new PAPERS_DETAIL;
		pPapers->strPapersName = pTask->strFileBaseName;
		pPapers->strPapersPath = CMyCodeConvert::Utf8ToGb2312(strOutDir);
		pPapers->strSrcPapersPath = pTask->strFilePath;
		pPapers->strSrcPapersFileName = pTask->strSrcFileName;
	}
	
	if (bNeedDecompress)
	{
		std::string strLog = "获得试卷包解压任务: " + pTask->strFilePath;
		g_Log.LogOut(strLog);
		std::ifstream inp(pTask->strFilePath.c_str(), std::ios::binary);
		if (!inp)
		{
			strLog = "打开试卷包文件失败";
			g_Log.LogOutError(strLog);
			return;
		}

		Poco::File decompressDir(strOutDir);
		if (decompressDir.exists())
			decompressDir.remove(true);
		decompressDir.createDirectories();

	#ifdef USE_POCO_UNZIP
		Poco::Zip::Decompress dec(inp, strOutDir);
		DecompressHandler handler(pPapers);
		dec.EError += Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(&handler, &DecompressHandler::onError);
		dec.EOk += Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(&handler, &DecompressHandler::onOk);
		dec.decompressAllFiles();
		dec.EError -= Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(&handler, &DecompressHandler::onError);
		dec.EOk -= Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(&handler, &DecompressHandler::onOk);
	#else
		unzFile uf = NULL;

		#ifdef USEWIN32IOAPI
		zlib_filefunc64_def ffunc;
		fill_win32_filefunc64A(&ffunc);
		uf = unzOpen2_64(pTask->strFilePath.c_str(), &ffunc);
		#else
		uf = unzOpen64(pTask->strFilePath.c_str());
		#endif

		if (uf == NULL)
		{
			std::string strLog = "打开压缩文件失败:" + pPapers->strPapersPath;
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			return;
		}
		int ret = 0;
		int opt_do_extract_withoutpath = 0;
		int opt_overwrite = 1;
		const char *password = NULL;
		password = "static";

		#ifndef DecompressTest
		if (CHDIR(pPapers->strPapersPath.c_str()))
		{
			std::string strLog = "切换目录失败:" + pPapers->strPapersPath;
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			return;
		}
		#else
		std::string strBaseDir = CMyCodeConvert::Utf8ToGb2312(strOutDir);
		char szBaseDir[256] = { 0 };
		strncpy_s(szBaseDir, strBaseDir.c_str(), strBaseDir.length());
		#endif
		if (pTask->nType <= 3)
			ret = do_extract_all(uf, opt_do_extract_withoutpath, opt_overwrite, password, szBaseDir);
		else
			ret = do_extract_onefile(uf, "papersInfo.dat", opt_do_extract_withoutpath, opt_overwrite, password, szBaseDir);
		unzClose(uf);

		if (ret != 0)
		{
			//************	注意：解压失败需要再次尝试	*************************

			std::string strLog = "解压试卷包(";
			strLog.append(pPapers->strPapersName);
			strLog.append(")失败, 路径: " + pPapers->strPapersPath);
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			return;
		}

		#ifndef DecompressTest
		CHDIR(CMyCodeConvert::Utf8ToGb2312(SysSet.m_strDecompressPath).c_str());		//切换回解压根目录，否则删除压缩文件夹失败
		#endif

		if (pTask->nType == 2)
		{
			UploadModelPic(pPapers, pPapers->strPapersPath);

			g_fmPapers.lock();
			g_lPapers.push_back(pPapers);
			g_fmPapers.unlock();
			return;
		}

		if (pTask->nType <= 3)
			SearchExtractFile(pPapers, pPapers->strPapersPath);
	#endif
		//解压完成
		pPapers->lPaper.sort(SortByPaper);

		//读取试卷袋文件夹里面的文件获取试卷袋信息

		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
		bool bResult_Data = GetFileData(strPapersFilePath, pPapers, pTask);
		if (!bResult_Data)
		{
			std::string strLog = Poco::format("试卷袋(%s),解压后数据不一致，需要重新解压,已解压次数%d", pPapers->strPapersName, pTask->nTimes + 1);
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			SAFE_RELEASE(pPapers);

			Poco::Thread::sleep(500);
			pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
			pDecompressTask->nTimes = pTask->nTimes + 1;
			pDecompressTask->strFilePath = pTask->strFilePath;
			pDecompressTask->strFileBaseName = pTask->strFileBaseName;
			pDecompressTask->strSrcFileName = pTask->strSrcFileName;
			pDecompressTask->nType = pTask->nType;
			pDecompressTask->strTransferData = pTask->strTransferData;
			g_fmDecompressLock.lock();
			g_lDecompressTask.push_back(pDecompressTask);
			g_fmDecompressLock.unlock();
			return;
		}
	}

#ifdef TEST_MODE
// 	if (SysSet.m_nBackupPapers)
// 	{
// 		Poco::LocalDateTime now;
// 		//			std::string strBackupDir = Poco::format("%s\\%04d-%02d-%02d", CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strPapersBackupPath), now.year(), now.month(), now.day());
// 		std::string strBackupDir = Poco::format("%s\\%04d-%02d-%02d\\%d_%d\\%s", CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strPapersBackupPath), now.year(), now.month(), now.day(), pPapers->nExamID, pPapers->nSubjectID, pPapers->strUploader);
// 		std::string strBackupPath = strBackupDir + "\\" + CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersFileName);
// 		try
// 		{
// 			Poco::File backupDir(strBackupDir);
// 			if (!backupDir.exists())
// 				backupDir.createDirectories();
// 
// 			Poco::File filePapers(CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersPath));
// 			filePapers.moveTo(strBackupPath);
// 			std::string strLog = "备份试卷袋文件(" + pPapers->strSrcPapersFileName + ")完成";
// 			g_Log.LogOut(strLog);
// 			std::cout << strLog << std::endl;
// 		}
// 		catch (Poco::Exception& exc)
// 		{
// 			std::string strErrInfo = Poco::format("备份试卷袋(%s)失败,%s", pPapers->strSrcPapersPath, exc.message());
// 			g_Log.LogOutError(strErrInfo);
// 			std::cout << strErrInfo << std::endl;
// 		}
// 	}
#else
	if (pTask->nType <= 2)	//普通试卷袋||扫描模板
	{
		if (SysSet.m_nUpPicData)
		{
			LIST_PAPER_INFO::iterator itPaper = pPapers->lPaper.begin();
			for (; itPaper != pPapers->lPaper.end(); itPaper++)
			{
				pPAPER_INFO pPaper = *itPaper;
				LIST_PIC_DETAIL::iterator itPic = pPaper->lPic.begin();
				for (; itPic != pPaper->lPic.end(); itPic++)
				{
					pPIC_DETAIL pPic = *itPic;

					pSEND_HTTP_TASK pHttpTask = new SEND_HTTP_TASK;
					pHttpTask->nTaskType = 1;
					pHttpTask->pPic = pPic;
					pHttpTask->pPapers = pPapers;
					pHttpTask->strUri = SysSet.m_strUpLoadHttpUri;
					g_fmHttpSend.lock();
					g_lHttpSend.push_back(pHttpTask);
					g_fmHttpSend.unlock();
				}
			}
		}
		else
		{
			pPapers->fmResultState.lock();
			pPapers->nResultSendState = pPapers->nResultSendState | 1;	//将图片地址上传位置设置为上传成功，因为进入到此步骤时说明地址已经提交成功
			pPapers->fmResultState.unlock();

			pSEND_HTTP_TASK pHttpTask = new SEND_HTTP_TASK;
			pHttpTask->nTaskType = 7;
			pHttpTask->pPapers = pPapers;
			g_fmHttpSend.lock();
			g_lHttpSend.push_back(pHttpTask);
			g_fmHttpSend.unlock();
		}
	}
	else if (pTask->nType == 3)
	{
		pPapers->strSendPicsAddrResult = pTask->strTransferData;	//如果再次提交失败时，此结果可以继续写入文件中

		pSEND_HTTP_TASK pNewTask = new SEND_HTTP_TASK;
		pNewTask->nTaskType = 2;
		pNewTask->strResult = pTask->strTransferData;
		pNewTask->pPapers = pPapers;		//检查图片地址、OMR、ZKZH、选做题信息是否都提交成功，没有提交成功的将此信息记录到文本文件，下次重启时自动重新提交
		pNewTask->strEzs = pPapers->strEzs;
		pNewTask->strUri = SysSet.m_strBackUri + "/studentAnswerSheet";
		g_fmHttpSend.lock();
		g_lHttpSend.push_back(pNewTask);
		g_fmHttpSend.unlock();

		Poco::File fSrc(pTask->strTransferFilePath);
		if (fSrc.exists())
			fSrc.remove();
	}
	else if (pTask->nType == 4)
	{
		pPapers->fmResultState.lock();
		pPapers->nResultSendState = pPapers->nResultSendState | 1;	//将图片地址上传位置设置为上传成功，因为进入到此步骤时说明地址已经提交成功
		pPapers->fmResultState.unlock();

		pPapers->fmTask.lock();
		pPapers->nTaskCounts++;			//omr
		pPapers->fmTask.unlock();
		pPapers->strSendOmrResult = pTask->strTransferData;	//将结果信息保存到试卷袋结构体中，以防异常错误时需要重新计算 2017.1.19

		pSEND_HTTP_TASK pOmrTask = new SEND_HTTP_TASK;
		pOmrTask->nTaskType = 3;
		pOmrTask->strResult = pTask->strTransferData;
		pOmrTask->pPapers = pPapers;
		pOmrTask->strEzs = pPapers->strEzs;
		pOmrTask->strUri = SysSet.m_strBackUri + "/omr";
		g_fmHttpSend.lock();
		g_lHttpSend.push_back(pOmrTask);
		g_fmHttpSend.unlock();

		Poco::File fSrc(pTask->strTransferFilePath);
		if (fSrc.exists())
			fSrc.remove();
	}
	else if (pTask->nType == 5)
	{
		pPapers->fmResultState.lock();
		pPapers->nResultSendState = pPapers->nResultSendState | 1;	//将图片地址上传位置设置为上传成功，因为进入到此步骤时说明地址已经提交成功
		pPapers->fmResultState.unlock();

		pPapers->fmTask.lock();
		pPapers->nTaskCounts++;			//zkzh
		pPapers->fmTask.unlock();
		pPapers->strSendZkzhResult = pTask->strTransferData;	//将结果信息保存到试卷袋结构体中，以防异常错误时需要重新计算 2017.1.19

		pSEND_HTTP_TASK pSnTask = new SEND_HTTP_TASK;
		pSnTask->nTaskType = 4;
		pSnTask->strResult = pTask->strTransferData;
		pSnTask->pPapers = pPapers;
		pSnTask->strEzs = pPapers->strEzs;
		pSnTask->strUri = SysSet.m_strBackUri + "/zkzh";
		g_fmHttpSend.lock();
		g_lHttpSend.push_back(pSnTask);
		g_fmHttpSend.unlock();

		Poco::File fSrc(pTask->strTransferFilePath);
		if (fSrc.exists())
			fSrc.remove();
	}
	else if (pTask->nType == 6)
	{
		pPapers->fmResultState.lock();
		pPapers->nResultSendState = pPapers->nResultSendState | 1;	//将图片地址上传位置设置为上传成功，因为进入到此步骤时说明地址已经提交成功
		pPapers->fmResultState.unlock();

		pPapers->fmTask.lock();
		pPapers->nTaskCounts++;			//electOmr
		pPapers->fmTask.unlock();
		pPapers->strSendElectOmrResult = pTask->strTransferData;	//将结果信息保存到试卷袋结构体中，以防异常错误时需要重新计算 2017.1.19

		pSEND_HTTP_TASK pElectOmrTask = new SEND_HTTP_TASK;
		pElectOmrTask->nTaskType = 5;
		pElectOmrTask->strResult = pTask->strTransferData;
		pElectOmrTask->pPapers = pPapers;
		pElectOmrTask->strEzs = pPapers->strEzs;
		pElectOmrTask->strUri = SysSet.m_strBackUri + "/choosetitleinfo";
		g_fmHttpSend.lock();
		g_lHttpSend.push_back(pElectOmrTask);
		g_fmHttpSend.unlock();

		Poco::File fSrc(pTask->strTransferFilePath);
		if (fSrc.exists())
			fSrc.remove();
	}
#endif
	
	if (bNeedDecompress)
	{
		g_fmPapers.lock();
		g_lPapers.push_back(pPapers);
		g_fmPapers.unlock();
	}
}

bool CDecompressThread::GetFileData(std::string strFilePath, pPAPERS_DETAIL pPapers, pDECOMPRESSTASK pTask)
{
	std::string strUtf8Path = CMyCodeConvert::Utf8ToGb2312(strFilePath);
	std::string strJsnData;
	std::ifstream in(strUtf8Path);

	if (!in)
		return false;

	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);
		strJsnData.append(strJsnLine);
	}
	in.close();

	std::string strFileData;
#ifdef USES_FILE_CRYPTOGRAM
	if (!decString(strJsnData, strFileData))
		strFileData = strJsnData;
#else
	strFileData = strJsnData;
#endif

	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strFileData);		//strJsnData
		Poco::JSON::Object::Ptr objData = result.extract<Poco::JSON::Object::Ptr>();

		int nOmrDoubt = -1;
		int nOmrNull = -1;
		int nSnNull = -1;
		if (objData->has("nOmrDoubt"))
			nOmrDoubt = objData->get("nOmrDoubt").convert<int>();
		if (objData->has("nOmrNull"))
			nOmrNull = objData->get("nOmrNull").convert<int>();
		if (objData->has("nSnNull"))
			nSnNull = objData->get("nSnNull").convert<int>();

		int nExamId		= objData->get("examId").convert<int>();
		int nSubjectId	= objData->get("subjectId").convert<int>();
		int nTeacherId	= objData->get("nTeacherId").convert<int>();
		int nUserId		= objData->get("nUserId").convert<int>();
		int nStudentNum = objData->get("scanNum").convert<int>();
		std::string strUploader = objData->get("uploader").convert<std::string>();
		std::string strEzs	= objData->get("ezs").convert<std::string>();

		if (pTask->nType <= 3)
		{
			bool bFindDeff = false;
			std::string strStudentList_Src;
			std::string strStudentList_Decompress;
			Poco::JSON::Array::Ptr jsnDetailArry = objData->getArray("detail");
			for (int i = 0; i < jsnDetailArry->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnPaperObj = jsnDetailArry->getObject(i);
				std::string strStudentInfo = jsnPaperObj->get("name").convert<std::string>();

				pPAPER_INFO pPaper = NULL;
				LIST_PAPER_INFO::iterator itPaper = pPapers->lPaper.begin();
				for (int j = 0; itPaper != pPapers->lPaper.end(); itPaper++)
				{
					strStudentList_Decompress.append((*itPaper)->strName + " ");
					if ((*itPaper)->strName == strStudentInfo)
					{
						pPaper = *itPaper;
						break;
					}
				}
				if (pPaper)
				{
					pPaper->nChkFlag = 1;	//此图片合法，在参数文件中可以找到
					pPaper->strZkzh = jsnPaperObj->get("zkzh").convert<std::string>();
					pPaper->nQkFlag = jsnPaperObj->get("qk").convert<int>();

					Poco::JSON::Array::Ptr jsnSnArry = jsnPaperObj->getArray("snDetail");
					Poco::JSON::Object jsnSn;
					jsnSn.set("examId", nExamId);
					jsnSn.set("subjectId", nSubjectId);
					jsnSn.set("userId", nUserId);
					jsnSn.set("teacherId", nTeacherId);
					jsnSn.set("zkzh", pPaper->strZkzh);
					jsnSn.set("papers", pPapers->strPapersName);
					if (pPaper->strZkzh != "")
						jsnSn.set("doubt", 0);
					else
						jsnSn.set("doubt", 1);
					jsnSn.set("detail", jsnSnArry);
					std::stringstream jsnSnString;
					jsnSn.stringify(jsnSnString, 0);
					pPaper->strSnDetail = jsnSnString.str();
					// 				for (int i = 0; i < jsnSnArry->size(); i++)
					// 				{
					// 					Poco::JSON::Object::Ptr jsnSnItem = jsnSnArry->getObject(i);
					// 					Poco::JSON::Object::Ptr jsnSnPosition = jsnSnItem->getObject("position");
					// 
					// 					SN_ITEM snItem;
					// 					snItem.nItem		= jsnSnItem->get("sn").convert<int>();
					// 					snItem.nRecogVal	= jsnSnItem->get("val").convert<int>();
					// 					snItem.rt.x			= jsnSnPosition->get("x").convert<int>();
					// 					snItem.rt.y			= jsnSnPosition->get("y").convert<int>();
					// 					snItem.rt.width		= jsnSnPosition->get("w").convert<int>();
					// 					snItem.rt.height	= jsnSnPosition->get("h").convert<int>();
					// 					pPaper->lSnResult.push_back(snItem);
					// 				}
					Poco::JSON::Array::Ptr jsnOmrArry = jsnPaperObj->getArray("omr");
					Poco::JSON::Object jsnOmr;
					jsnOmr.set("examId", nExamId);
					jsnOmr.set("subjectId", nSubjectId);
					jsnOmr.set("userId", nUserId);
					jsnOmr.set("teacherId", nTeacherId);
					jsnOmr.set("zkzh", pPaper->strZkzh);
					jsnOmr.set("papers", pPapers->strPapersName);
					jsnOmr.set("omr", jsnOmrArry);
					std::stringstream jsnOmrString;
					jsnOmr.stringify(jsnOmrString, 0);
					pPaper->strOmrDetail = jsnOmrString.str();

					if (jsnPaperObj->has("electOmr"))
					{
						//*************	注意：这里需要和后端确认，现在还不行	********************

						Poco::JSON::Array::Ptr jsnElectOmrArry = jsnPaperObj->getArray("electOmr");
						Poco::JSON::Object jsnElectOmr;
						jsnElectOmr.set("examId", nExamId);
						jsnElectOmr.set("subjectId", nSubjectId);
						jsnElectOmr.set("userId", nUserId);
						jsnElectOmr.set("teacherId", nTeacherId);
						jsnElectOmr.set("zkzh", pPaper->strZkzh);
						jsnElectOmr.set("papers", pPapers->strPapersName);
						jsnElectOmr.set("electOmr", jsnElectOmrArry);
						std::stringstream jsnElectOmrString;
						jsnElectOmr.stringify(jsnElectOmrString, 0);
						pPaper->strElectOmrDetail = jsnElectOmrString.str();
						pPaper->nHasElectOmr = 1;
					}
				}
				else
				{
					strStudentList_Src.append(strStudentInfo + " ");
					bFindDeff = true;
				}
			}
			if (bFindDeff)
			{
				std::string strLog;
				strLog = "出现OMR和SN号信息与学生试卷不一致的情况，详情: 源(" + strStudentList_Src + ")未发现, 解压后(" + strStudentList_Decompress + ")";
				g_Log.LogOutError(strLog);
				return false;
			}
		}

		//将不在试卷袋参数文件中存在的图片都删除，不往图片服务器提交
		std::string strErrStudent;
		LIST_PAPER_INFO::iterator itPaper = pPapers->lPaper.begin();
		for (int j = 0; itPaper != pPapers->lPaper.end();)
		{
			pPAPER_INFO pObj = *itPaper;
			if (pObj->nChkFlag == 0)
			{
				strErrStudent.append(pObj->strName + " ");
				pPapers->nTotalPics -= pObj->lPic.size();
				SAFE_RELEASE(pObj);
				itPaper = pPapers->lPaper.erase(itPaper);
			}
			else
			{
				//**********************************************************************************************

				//**********************************************************************************************

				//**********************************************************************************************

				//**********************************************************************************************

				//***********************************	注意	************************************************

				//针对肇庆考试，高一高二物理4-4，3-5科目为单页，将试卷袋中多的试卷删除

				//**********************************************************************************************

				//**********************************************************************************************

				//**********************************************************************************************

				//**********************************************************************************************

				//**********************************************************************************************

				//**********************************************************************************************
			#if 0
				if ((nExamId == 4 && nSubjectId == 4))
				{
					LIST_PIC_DETAIL::iterator itPic = pObj->lPic.begin();
					for (int j = 0; itPic != pObj->lPic.end(); j++)
					{
						pPIC_DETAIL pPicObj = *itPic;
						if (j != 0)
						{
							SAFE_RELEASE(pPicObj);
							itPic = pObj->lPic.erase(itPic);
						}
						else
							itPic++;
					}
				}
			#endif
				itPaper++;
			}
		}
		if (strErrStudent.length() > 0)
		{
			std::string strErrStudentInfo = "试卷袋(" + pPapers->strPapersName + ")中以下学生多了，不在试卷袋记录信息中，这些学生图像将不会提交图像服务器：" + strErrStudent;
			g_Log.LogOutError(strErrStudentInfo);
		}
		//--

		pPapers->nOmrDoubt	= nOmrDoubt;
		pPapers->nOmrNull	= nOmrNull;
		pPapers->nSnNull	= nSnNull;
		pPapers->nExamID	= nExamId;
		pPapers->nSubjectID = nSubjectId;
		pPapers->nTeacherId = nTeacherId;
		pPapers->nUserId	= nUserId;
		pPapers->nTotalPaper = nStudentNum;
		pPapers->strUploader = strUploader;
		pPapers->strEzs		= SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("解析试卷袋文件夹中文件失败: ");
		strErrInfo.append(jsone.message());
		g_Log.LogOutError(strErrInfo);
		return false;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("解析试卷袋文件夹中文件失败2: ");
		strErrInfo.append(exc.message());
		g_Log.LogOutError(strErrInfo);
		return false;
	}
	return true;
}

void CDecompressThread::SearchExtractFile(pPAPERS_DETAIL pPapers, std::string strPath)
{
	std::string strPapersPath = CMyCodeConvert::Gb2312ToUtf8(strPath);
	Poco::DirectoryIterator it(strPapersPath);
	Poco::DirectoryIterator end;
	while (it != end)
	{
		Poco::Path p(it->path());
		if (it->isFile())
		{
			std::string strName = p.getFileName();
			if (strName.find("papersInfo.dat") != std::string::npos)
			{
				it++;
				continue;
			}

			pPIC_DETAIL pPic = new PIC_DETAIL;
			pPic->strFileName = strName;
			pPic->strFilePath = pPapers->strPapersPath + "\\" + pPic->strFileName;

			int nPos = pPic->strFileName.find("_");
			std::string strPaperName = pPic->strFileName.substr(0, nPos);
			bool bFind = false;
			LIST_PAPER_INFO::iterator it = pPapers->lPaper.begin();
			for (; it != pPapers->lPaper.end(); it++)
			{
				if ((*it)->strName == strPaperName)
				{
					(*it)->lPic.push_back(pPic);
					bFind = true;
					break;
				}
			}
			if (!bFind)
			{
				pPAPER_INFO pPaper = new PAPER_INFO;
				pPaper->lPic.push_back(pPic);
				pPaper->strName = strPaperName;
				pPapers->lPaper.push_back(pPaper);
			}
			pPapers->nTotalPics++;						//图片数增加一张
		}
		it++;
	}
}

void CDecompressThread::UploadModelPic(pPAPERS_DETAIL pPapers, std::string strPath)
{
	std::string strJsnModelPath = strPath + "\\model.dat";

	std::string strJsnData;
	std::ifstream in(strJsnModelPath);
	if (!in)
		return ;

	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);					//不过滤空格
		strJsnData.append(strJsnLine);
	}

	in.close();

	std::string strFileData;
	if (!decString(strJsnData, strFileData))
		strFileData = strJsnData;

	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strFileData);		//strJsnData
		Poco::JSON::Object::Ptr objData = result.extract<Poco::JSON::Object::Ptr>();

		pPapers->nExamID = objData->get("nExamId").convert<int>();
		pPapers->nSubjectID = objData->get("nSubjectId").convert<int>();

		Poco::JSON::Array::Ptr arrayPapers = objData->getArray("paperInfo");
		for (int i = 0; i < arrayPapers->size(); i++)
		{
			Poco::JSON::Object::Ptr jsnPaperObj = arrayPapers->getObject(i);

			pPAPER_INFO pPaper = new PAPER_INFO;		//模板图片以一张图片当成一个学生来放
			pPapers->lPaper.push_back(pPaper);

			int nID = jsnPaperObj->get("paperNum").convert<int>() + 1;		//id 从0开始
			pPaper->strName = Poco::format("S%d", nID);

			pPIC_DETAIL pPic = new PIC_DETAIL;
			pPic->nPicW = jsnPaperObj->get("picW").convert<int>();
			pPic->nPicH = jsnPaperObj->get("picH").convert<int>();
			pPic->strFileName = CMyCodeConvert::Utf8ToGb2312(jsnPaperObj->get("modelPicName").convert<std::string>());;
			pPic->strFilePath = pPapers->strPapersPath + "\\" + pPic->strFileName;
			pPaper->lPic.push_back(pPic);
			pPapers->nTotalPics++;
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "在解压完模板后，解析模板信息并上传模板图片时，模板文件信息解析失败，" + exc.displayText();
		g_Log.LogOutError(strLog);
	}

	pPapers->lPaper.sort(SortByPaper);

	LIST_PAPER_INFO::iterator itPaper = pPapers->lPaper.begin();
	for (; itPaper != pPapers->lPaper.end(); itPaper++)
	{
		pPAPER_INFO pPaper = *itPaper;
		LIST_PIC_DETAIL::iterator itPic = pPaper->lPic.begin();
		for (; itPic != pPaper->lPic.end(); itPic++)
		{
			pPIC_DETAIL pPic = *itPic;

			pSEND_HTTP_TASK pHttpTask = new SEND_HTTP_TASK;
			pHttpTask->nTaskType = 6;
			pHttpTask->pPic = pPic;
			pHttpTask->pPapers = pPapers;
			pHttpTask->strUri = SysSet.m_strUpLoadHttpUri;
			g_fmHttpSend.lock();
			g_lHttpSend.push_back(pHttpTask);
			g_fmHttpSend.unlock();
		}
	}
}


