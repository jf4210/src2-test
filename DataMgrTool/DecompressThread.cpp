#include "stdafx.h"
#include "DecompressThread.h"
#include <algorithm>
#ifndef USE_POCO_UNZIP
#include "miniunz/miniunz.c"
#endif
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"




class DecompressHandler
{
public:
	DecompressHandler()
	{
	}

	~DecompressHandler()
	{
	}

	void onError(const void*, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info)
	{
		std::string strLog = "Decompress ERR: " + info.second;
		g_Log.LogOutError(strLog);
	}

	inline void onOk(const void*, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& info)
	{
		std::string strLog = "Decompress OK: " + info.second.toString(Poco::Path::PATH_UNIX);
		g_Log.LogOut(strLog);
	}
};


CDecompressThread::CDecompressThread(void* pDlg) :m_pDlg(pDlg)
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
	std::string strLog = "获得试卷包解压任务: " + pTask->strFilePath;
	g_Log.LogOut(strLog);

	std::ifstream inp(pTask->strFilePath.c_str(), std::ios::binary);
	if (!inp)
	{
		strLog = "打开试卷包文件失败";
		g_Log.LogOutError(strLog);
		return;
	}
	std::string strOutDir = CMyCodeConvert::Gb2312ToUtf8(pTask->strDecompressDir + "\\" + pTask->strFileBaseName);

	pPAPERSINFO pPapers = NULL;
	if (pTask->nTaskType == 3)
	{
		pPapers = new PAPERSINFO;
		pPapers->strPapersName = pTask->strFileBaseName;
		pPapers->strPapersPath = CMyCodeConvert::Utf8ToGb2312(strOutDir);
		pPapers->strSrcPapersPath = pTask->strFilePath;
		pPapers->strSrcPapersFileName = pTask->strSrcFileName;
		
		g_fmPapers.lock();
		g_lPapers.push_back(pPapers);
		g_fmPapers.unlock();
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
		std::string strLog = "打开压缩文件失败:" + pTask->strFilePath;
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
	int ret = 0;
	int opt_do_extract_withoutpath = 0;
	int opt_overwrite = 1;
	const char *password = NULL;
	password = "static";
	
	if (CHDIR(CMyCodeConvert::Utf8ToGb2312(strOutDir).c_str()))
	{
		std::string strLog = "切换目录失败:" + strOutDir;
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
	ret = do_extract_all(uf, opt_do_extract_withoutpath, opt_overwrite, password);
	unzClose(uf);

	if (ret != 0)
	{
		//************	注意：解压失败需要再次尝试	*************************

		std::string strLog = "解压试卷包(";
		strLog.append(pTask->strFileBaseName);
		strLog.append(")失败, 路径: " + pTask->strFilePath);
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
	CHDIR(pTask->strDecompressDir.c_str());		//切换回解压根目录，否则删除压缩文件夹失败

	if (pTask->nTaskType == 3)
	{
		SearchExtractFile(pPapers, pPapers->strPapersPath);

		//解压完成
		pPapers->lPaper.sort(SortByPaper);
	}
	


	USES_CONVERSION;
	if (pTask->nTaskType == 1)
	{
		CString strMsg;
		strMsg.Format(_T("解压(%s)完成\r\n"), A2T(pTask->strSrcFileName.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
	}
	else if (pTask->nTaskType == 2)
	{
		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
		int nExamID = 0;
		int nSubjectID = 0;
		GetFileData(strPapersFilePath, nExamID, nSubjectID);

		std::string strNewPkgDir = Poco::format("%s\\%d_%d", CMyCodeConvert::Gb2312ToUtf8(pTask->strDecompressDir), nExamID, nSubjectID);
		std::string strNewPkgPath = Poco::format("%s\\%d_%d\\%s", CMyCodeConvert::Gb2312ToUtf8(pTask->strDecompressDir), nExamID, nSubjectID, pTask->strSrcFileName);
		try
		{
			Poco::File copyDir(strNewPkgDir);
			if (!copyDir.exists())
				copyDir.createDirectories();

			Poco::File copyFilePath(pTask->strFilePath);
			copyFilePath.copyTo(strNewPkgPath);

			Poco::File decompressDir(strOutDir);
			if (decompressDir.exists())
				decompressDir.remove(true);

			CString strMsg;
			strMsg.Format(_T("解压(%s)完成\r\n"), A2T(pTask->strSrcFileName.c_str()));
			((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
		}
		catch (Poco::Exception& e)
		{
		}
		
	}
	else if (pTask->nTaskType == 3)
	{
		//读取试卷袋文件夹里面的文件获取试卷袋信息
		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
		bool bResult_Data = GetFileData(CMyCodeConvert::Utf8ToGb2312(strPapersFilePath), pPapers);

		//添加到识别任务列表
		PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
		for (; itPaper != pPapers->lPaper.end(); itPaper++)
		{
			pRECOGTASK pTask = new RECOGTASK;
			pTask->pPaper = *itPaper;
			g_lRecogTask.push_back(pTask);
		}

	}
	else if (pTask->nTaskType == 4)
	{
		CString strMsg;
		strMsg.Format(_T("识别模板解压(%s)完成\r\n"), A2T(pTask->strSrcFileName.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);

		if (_pModel_)
		{
			delete _pModel_;
			_pModel_ = NULL;
		}

		std::string strModelPath = CMyCodeConvert::Utf8ToGb2312(strOutDir);
		_pModel_ = LoadModelFile(A2T(strModelPath.c_str()));

		strMsg.Format(_T("模板加载(%s)完成\r\n"), A2T(pTask->strSrcFileName.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
	}

#endif
}


void CDecompressThread::GetFileData(std::string strFilePath, int& nExamID, int& nSubjectID)
{
	std::string strJsnData;
	std::ifstream in(strFilePath);
	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);
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

		nExamID = objData->get("examId").convert<int>();
		nSubjectID = objData->get("subjectId").convert<int>();
		int nTeacherId = objData->get("nTeacherId").convert<int>();
		int nUserId = objData->get("nUserId").convert<int>();
		int nStudentNum = objData->get("scanNum").convert<int>();
		std::string strUploader = objData->get("uploader").convert<std::string>();
		std::string strEzs = objData->get("ezs").convert<std::string>();
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("解析试卷袋文件夹中文件失败: ");
		strErrInfo.append(jsone.message());
		g_Log.LogOutError(strErrInfo);
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("解析试卷袋文件夹中文件失败2: ");
		strErrInfo.append(exc.message());
		g_Log.LogOutError(strErrInfo);
	}
}

bool CDecompressThread::GetFileData(std::string strFilePath, pPAPERSINFO pPapers)
{
	std::string strJsnData;
	std::ifstream in(strFilePath);
	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);
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

// 		int nOmrDoubt = -1;
// 		int nOmrNull = -1;
// 		int nSnNull = -1;
// 		if (objData->has("nOmrDoubt"))
// 			nOmrDoubt = objData->get("nOmrDoubt").convert<int>();
// 		if (objData->has("nOmrNull"))
// 			nOmrNull = objData->get("nOmrNull").convert<int>();
// 		if (objData->has("nSnNull"))
// 			nSnNull = objData->get("nSnNull").convert<int>();

		int nExamId = objData->get("examId").convert<int>();
		int nSubjectId = objData->get("subjectId").convert<int>();
		int nTeacherId = objData->get("nTeacherId").convert<int>();
		int nUserId = objData->get("nUserId").convert<int>();
		int nStudentNum = objData->get("scanNum").convert<int>();
		std::string strUploader = objData->get("uploader").convert<std::string>();
		std::string strEzs = objData->get("ezs").convert<std::string>();
		if (objData->has("desc"))
		{
			pPapers->strDesc = objData->get("desc").convert<std::string>();
		}

#if 0
		bool bFindDeff = false;
		std::string strStudentList_Src;
		std::string strStudentList_Decompress;
		Poco::JSON::Array::Ptr jsnDetailArry = objData->getArray("detail");
		for (int i = 0; i < jsnDetailArry->size(); i++)
		{
			Poco::JSON::Object::Ptr jsnPaperObj = jsnDetailArry->getObject(i);
			std::string strStudentInfo = jsnPaperObj->get("name").convert<std::string>();

			pST_PaperInfo pPaper = NULL;
			PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
			for (int j = 0; itPaper != pPapers->lPaper.end(); itPaper++)
			{
				strStudentList_Decompress.append((*itPaper)->strStudentInfo + " ");
				if ((*itPaper)->strStudentInfo == strStudentInfo)
				{
					pPaper = *itPaper;
					break;
				}
			}
			if (pPaper)
			{
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
#endif
// 		pPapers->nOmrDoubt = nOmrDoubt;
// 		pPapers->nOmrNull = nOmrNull;
// 		pPapers->nSnNull = nSnNull;
		pPapers->nExamID = nExamId;
		pPapers->nSubjectID = nSubjectId;
		pPapers->nTeacherId = nTeacherId;
		pPapers->nUserId = nUserId;
		pPapers->nTotalPaper = nStudentNum;
		pPapers->strUploader = strUploader;
		pPapers->strEzs = strEzs;		//"ezs=" + strEzs;

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

void CDecompressThread::SearchExtractFile(pPAPERSINFO pPapers, std::string strPath)
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

			pST_PicInfo pPic = new ST_PicInfo;
			pPic->strPicName = strName;
			pPic->strPicPath = pPapers->strPapersPath + "\\" + pPic->strPicName;

			int nPos = pPic->strPicName.find("_");
			std::string strPaperName = pPic->strPicName.substr(0, nPos);
			bool bFind = false;
			PAPER_LIST::iterator it = pPapers->lPaper.begin();
			for (; it != pPapers->lPaper.end(); it++)
			{
				if ((*it)->strStudentInfo == strPaperName)
				{
					(*it)->lPic.push_back(pPic);
					bFind = true;
					break;
				}
			}
			if (!bFind)
			{
				pST_PaperInfo pPaper = new ST_PaperInfo;
				pPaper->lPic.push_back(pPic);
				pPic->pPaper = pPaper;
				pPaper->strStudentInfo = strPaperName;
				pPaper->pModel = _pModel_;
				pPaper->pPapers = pPapers;
				pPaper->pSrcDlg = m_pDlg;
				pPapers->lPaper.push_back(pPaper);
			}
			pPapers->nTotalPics++;						//图片数增加一张
		}
		it++;
	}
}

