#include "stdafx.h"
#include "DecompressThread.h"
#include <algorithm>
#ifndef USE_POCO_UNZIP
#include "miniunz/miniunz.c"
#endif
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"
#include "ModelMgr.h"



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
	eExit.reset();
	while (!g_nExitFlag)
	{
		bool bSleepMoreTime = false;
		pDECOMPRESSTASK pTask = NULL;
		g_fmDecompressLock.lock();
		DECOMPRESSTASKLIST::iterator it = g_lDecompressTask.begin();
		for (; it != g_lDecompressTask.end();)
		{
			if (g_lRecogTask.size() > 500)
			{
				bSleepMoreTime = true;
				break;
			}
			pTask = *it;
			it = g_lDecompressTask.erase(it);
			break;
		}
		g_fmDecompressLock.unlock();
		if (NULL == pTask)
		{
			if (bSleepMoreTime)
				Poco::Thread::sleep(2000);
			else
				Poco::Thread::sleep(1000);
			continue;
		}

		HandleTask(pTask);

		_fmDecompress_.lock();
		_nDecompress_--;
		_fmDecompress_.unlock();

		delete pTask;
		pTask = NULL;
	}
	eExit.set();
}

void CDecompressThread::HandleTask(pDECOMPRESSTASK pTask)
{
	std::string strLog = "获得试卷包解压任务: " + pTask->strFilePath;
	g_Log.LogOut(strLog);

	std::ifstream inp(pTask->strFilePath.c_str(), std::ios::binary);
	if (!inp)
	{
		strLog = "打开文件文件(" + pTask->strSrcFileName + ")失败";
		g_Log.LogOutError(strLog);

		strLog.append("\r\n");
		USES_CONVERSION;
		CString strMsg = A2T(strLog.c_str());
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
		return;
	}
	std::string strOutDir = CMyCodeConvert::Gb2312ToUtf8(pTask->strDecompressDir + "\\" + pTask->strFileBaseName);

	pPAPERSINFO pPapers = NULL;
	if (pTask->nTaskType == 3 || pTask->nTaskType == 5 || pTask->nTaskType == 6)
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

		strLog.append("\r\n");
		USES_CONVERSION;
		CString strMsg = A2T(strLog.c_str());
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
		return;
	}
	int ret = 0;
	int opt_do_extract_withoutpath = 0;
	int opt_overwrite = 1;
	const char *password = NULL;
	password = "static";
	
#ifndef DecompressTest
	if (CHDIR(CMyCodeConvert::Utf8ToGb2312(strOutDir).c_str()))
	{
		std::string strLog = "切换目录失败:" + strOutDir;
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
#else
	std::string strBaseDir = CMyCodeConvert::Utf8ToGb2312(strOutDir);
	char szBaseDir[256] = { 0 };
	strncpy_s(szBaseDir, strBaseDir.c_str(), strBaseDir.length());
#endif
	if (pTask->nTaskType != 7)
		ret = do_extract_all(uf, opt_do_extract_withoutpath, opt_overwrite, password, szBaseDir);
	else
	{
		for (int i = 0; i < 5; i++)		//解压考生的所有试卷，如S1，则解压S1_1.jpg,S1_2.jpg....
		{
			std::string strFileName = Poco::format("%s_%d.jpg", pTask->strDecompressPaperFile, i + 1);
			ret = do_extract_onefile(uf, strFileName.c_str(), opt_do_extract_withoutpath, opt_overwrite, password, szBaseDir);
			if (ret == 2)	//file strFileName not found in the zipfile，考生的所有文件已经都解压出来了，不需要再尝试，默认一个考试试卷不会超过5张
			{
				ret = 0;
				break;
			}
		}
	}
	unzClose(uf);

	if (ret != 0)
	{
		//************	注意：解压失败需要再次尝试	*************************

		std::string strLog = "解压试卷包(";
		strLog.append(pTask->strFileBaseName);
		strLog.append(")失败, 路径: " + pTask->strFilePath);
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;

		strLog.append("\r\n");
		USES_CONVERSION;
		CString strMsg = A2T(strLog.c_str());
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
		return;
	}
	strLog = "试卷包解压完成: " + pTask->strFilePath;
	g_Log.LogOut(strLog);
#ifndef DecompressTest
	CHDIR(pTask->strDecompressDir.c_str());		//切换回解压根目录，否则删除压缩文件夹失败
#endif
	if (pTask->nTaskType == 3 || pTask->nTaskType == 5 || pTask->nTaskType == 6)
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
		pPapers = new PAPERSINFO;
		pPapers->strPapersName = pTask->strFileBaseName;
		pPapers->strPapersPath = CMyCodeConvert::Utf8ToGb2312(strOutDir);
		pPapers->strSrcPapersPath = pTask->strFilePath;
		pPapers->strSrcPapersFileName = pTask->strSrcFileName;

		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
		GetFileData(strPapersFilePath, pPapers);

		std::string strNewPkgDir = Poco::format("%s\\%d_%d", CMyCodeConvert::Gb2312ToUtf8(pTask->strDecompressDir), pPapers->nExamID, pPapers->nSubjectID);
		std::string strNewPkgPath = Poco::format("%s\\%d_%d\\%s", CMyCodeConvert::Gb2312ToUtf8(pTask->strDecompressDir), pPapers->nExamID, pPapers->nSubjectID, pTask->strSrcFileName);
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
		SAFE_RELEASE(pPapers);
	}
	else if (pTask->nTaskType == 3)
	{
		//读取试卷袋文件夹里面的文件获取试卷袋信息
		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
		bool bResult_Data = GetFileData(CMyCodeConvert::Utf8ToGb2312(strPapersFilePath), pPapers);

		//***************************************************
		//***************************************************
		//***************************************************
		//***************************************************
		if (pPapers->nExamID != _pModel_->nExamID || pPapers->nSubjectID != _pModel_->nSubjectID)
		{
			std::string strErr = "试卷包" + pPapers->strPapersName + "考试ID和科目ID与识别模板不一致，不需要再次识别\r\n";
			g_Log.LogOut(strErr);

			CString strMsg = A2T(strErr.c_str());
			((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);

			_fmRecogPapers_.lock();
			_nRecogPapers_++;
			_fmRecogPapers_.unlock();

			_fmCompress_.lock();
			_nCompress_++;
			_fmCompress_.unlock();

			//删除源文件夹
			try
			{
				Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
				if (srcFileDir.exists())
					srcFileDir.remove(true);
			}
			catch (Poco::Exception& exc)
			{
				std::string strErr = "删除文件夹(" + pPapers->strPapersPath + ")失败: " + exc.message();
				g_Log.LogOutError(strErr);
			}
			return;
		}

		//		如果怀疑比例低于1%，不进行识别
		if (pTask->nNoNeedRecogVal > 0 && pPapers->nPkgOmrDoubt + pPapers->nPkgOmrNull + pPapers->nPkgSnNull < pTask->nNoNeedRecogVal)
		{
			int nReal = pPapers->nPkgOmrDoubt + pPapers->nPkgOmrNull + pPapers->nPkgSnNull;
			std::stringstream ss;
			ss << "试卷包" << pPapers->strPapersName << "(" << pPapers->nTotalPaper << ")中怀疑(" << pPapers->nPkgOmrDoubt << ")、omr空(" << pPapers->nPkgOmrNull
				<< ")、准考证空(" << pPapers->nPkgSnNull << ")总数 = " << nReal << "，低于重识别阀值(" << pTask->nNoNeedRecogVal << ")不需要再次识别\r\n";
			std::string strErr = ss.str();
			g_Log.LogOut(strErr);
			ss.str("");

			CString strMsg = A2T(strErr.c_str());
			((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);

			_fmRecogPapers_.lock();
			_nRecogPapers_++;
			_fmRecogPapers_.unlock();

			_fmCompress_.lock();
			_nCompress_++;
			_fmCompress_.unlock();

			//删除源文件夹
			try
			{
				Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
				if (srcFileDir.exists())
					srcFileDir.remove(true);

			}
			catch (Poco::Exception& exc)
			{
				std::string strErr = "删除文件夹(" + pPapers->strPapersPath + ")失败: " + exc.message();
				g_Log.LogOutError(strErr);
			}
			return;
		}
		//***************************************************
		//***************************************************
		//***************************************************
		//***************************************************

		pPapers->bRecogOmr = pTask->bRecogOmr;
		pPapers->bRecogZkzh = pTask->bRecogZkzh;
		pPapers->bRecogElectOmr = pTask->bRecogElectOmr;
		pPapers->nSendEzs = pTask->nSendEzs;

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
		std::string modelPath = T2A(g_strCurrentPath + _T("Model\\"));
		CModelMgr modelObj;
		modelObj.SetBaseInfo(strModelPath, _strEncryptPwd_);
		_pModel_ = modelObj.LoadModelFile(strModelPath);

		//_pModel_ = LoadModelFile(A2T(strModelPath.c_str()));
		if (_nUseNewParam_)
			InitModelRecog(_pModel_, strModelPath);

		strMsg.Format(_T("模板加载(%s)完成\r\n"), A2T(pTask->strSrcFileName.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);

		if (pTask->nExcuteTask == 1)
		{
			pST_SEARCH pDirTask = new ST_SEARCH;
			pDirTask->strSearchPath = _strPkgSearchPath_;
			pDirTask->bRecogOmr = pTask->bRecogOmr;
			pDirTask->bRecogZkzh = pTask->bRecogZkzh;
			pDirTask->bRecogElectOmr = pTask->bRecogElectOmr;
			pDirTask->nSendEzs = pTask->nSendEzs;
			pDirTask->nNoNeedRecogVal = pTask->nNoNeedRecogVal;

			_fmSearchPathList_.lock();
			_SearchPathList_.push_back(pDirTask);
			_fmSearchPathList_.unlock();
		}
		else if (pTask->nExcuteTask == 2)	//搜索试卷袋文件夹，进行普通试卷jpg图像搜索
		{
			std::string strPapersName = _strJpgSearchPath_.substr(_strJpgSearchPath_.rfind('\\')+ 1, _strJpgSearchPath_.length());

			pPapers = new PAPERSINFO;
			pPapers->nSendEzs = 3;

			g_fmPapers.lock();
			g_lPapers.push_back(pPapers);
			g_fmPapers.unlock();
			
			SearchDirPaper(pPapers, CMyCodeConvert::Utf8ToGb2312(_strJpgSearchPath_));

			//添加到识别任务列表
			PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
			for (; itPaper != pPapers->lPaper.end(); itPaper++)
			{
				pRECOGTASK pTask = new RECOGTASK;
				pTask->pPaper = *itPaper;
				g_lRecogTask.push_back(pTask);
			}
		}
		else if (pTask->nExcuteTask == 3)	//(搜索文件夹A，A中包含文件夹B，B中是图片jpg)
		{
			SearchDirPapers(_strJpgSearchPath_);
		}
	}
	else if (pTask->nTaskType == 5)
	{
		//读取试卷袋文件夹里面的文件获取试卷袋信息
		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
		bool bResult_Data = GetFileData2(CMyCodeConvert::Utf8ToGb2312(strPapersFilePath), pPapers);
		
		std::string strStatisticsInfo = calcStatistics(pPapers);

		CString strMsg;
		strMsg.Format(_T("%s"), A2T(strStatisticsInfo.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);

		//删除源文件夹
		try
		{
			Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
			if (srcFileDir.exists())
				srcFileDir.remove(true);
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "删除文件夹(" + pPapers->strPapersPath + ")失败: " + exc.message();
			g_Log.LogOutError(strErr);
		}

		_fmRecogPapers_.lock();
		_nRecogPapers_++;
		_fmRecogPapers_.unlock();

		g_fmPapers.lock();			//释放试卷袋列表
		PAPERS_LIST::iterator itPapers = g_lPapers.begin();
		for (; itPapers != g_lPapers.end(); itPapers++)
		{
			pPAPERSINFO pPapersTask = *itPapers;
			if (pPapersTask == pPapers)
			{
				itPapers = g_lPapers.erase(itPapers);
				SAFE_RELEASE(pPapersTask);
				break;
			}
		}
		g_fmPapers.unlock();
	}
	else if (pTask->nTaskType == 6)
	{
		//读取试卷袋文件夹里面的文件获取试卷袋信息
		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";

		CString strMsg;
		strMsg.Format(_T("解压%s:%s完成\r\n"), A2T(pTask->strFileBaseName.c_str()), A2T(pTask->strDecompressPaperFile.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);

		if (g_nRecogWithShowPkg)
		{
			bool bResult_Data = GetFileData(CMyCodeConvert::Utf8ToGb2312(strPapersFilePath), pPapers);

			pPapers->bRecogOmr = pTask->bRecogOmr;
			pPapers->bRecogZkzh = pTask->bRecogZkzh;
			pPapers->bRecogElectOmr = pTask->bRecogElectOmr;
			pPapers->nSendEzs = 3;

			//添加到识别任务列表
			PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
			for (; itPaper != pPapers->lPaper.end(); itPaper++)
			{
				pRECOGTASK pTask = new RECOGTASK;
				pTask->pPaper = *itPaper;
				g_lRecogTask.push_back(pTask);
			}
		}
		else
		{
			bool bResult_Data = GetFileData2(CMyCodeConvert::Utf8ToGb2312(strPapersFilePath), pPapers);
			((CDataMgrToolDlg*)m_pDlg)->showPapers(pPapers);
		}
	}
	else if (pTask->nTaskType == 7)
	{
		CString strMsg;
		strMsg.Format(_T("解压%s:%s完成\r\n"), A2T(pTask->strFileBaseName.c_str()), A2T(pTask->strDecompressPaperFile.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
	}
#endif
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
		result = parser.parse(CMyCodeConvert::Gb2312ToUtf8(strFileData));		//strJsnData
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

		if (objData->has("RecogMode"))
			pPapers->nRecogMode = objData->get("RecogMode").convert<int>();

		int nExamID = objData->get("examId").convert<int>();
		int nSubjectID = objData->get("subjectId").convert<int>();
		int nTeacherId = objData->get("nTeacherId").convert<int>();
		int nUserId = objData->get("nUserId").convert<int>();
		int nStudentNum = objData->get("scanNum").convert<int>();
		std::string strUploader = objData->get("uploader").convert<std::string>();
		std::string strEzs = objData->get("ezs").convert<std::string>();
		if (objData->has("desc"))
		{
			pPapers->strDesc = objData->get("desc").convert<std::string>();
		}

		pPapers->nPkgOmrDoubt = nOmrDoubt;
		pPapers->nPkgOmrNull = nOmrNull;
		pPapers->nPkgSnNull = nSnNull;

		pPapers->nExamID = nExamID;
		pPapers->nSubjectID = nSubjectID;
		pPapers->nTeacherId = nTeacherId;
		pPapers->nUserId = nUserId;
		pPapers->nTotalPaper = nStudentNum;
		pPapers->nPaperCount = nStudentNum;
		pPapers->strUploader = strUploader;
		pPapers->strEzs = strEzs;		//"ezs=" + strEzs;


	#ifdef CHK_NUM		//將試卷袋中不在參數文件列表中的文件刪除
		Poco::JSON::Array::Ptr jsnDetailArry = objData->getArray("detail");
		for (int i = 0; i < jsnDetailArry->size(); i++)
		{
			Poco::JSON::Object::Ptr jsnPaperObj = jsnDetailArry->getObject(i);
			std::string strStudentInfo = jsnPaperObj->get("name").convert<std::string>();

			pST_PaperInfo pPaper = NULL;
			PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
			for (int j = 0; itPaper != pPapers->lPaper.end(); itPaper++)
			{
				if ((*itPaper)->strStudentInfo == strStudentInfo)
				{
					pPaper = *itPaper;
					break;
				}
			}
			if (pPaper)
			{
				pPaper->nChkFlag = 1;	//此图片合法，在参数文件中可以找到

				if (jsnPaperObj->has("wj"))
					pPaper->nWJFlag = jsnPaperObj->get("wj").convert<int>();
				if (jsnPaperObj->has("issueFlag"))
					pPaper->nIssueFlag = jsnPaperObj->get("issueFlag").convert<int>();
				if (jsnPaperObj->has("standardAnswer"))
					pPaper->nStandardAnswer = jsnPaperObj->get("standardAnswer").convert<int>();
			}
		}
		//将不在试卷袋参数文件中存在的图片都删除，不往图片服务器提交
		std::string strErrStudent;
		PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
		for (int j = 0; itPaper != pPapers->lPaper.end();)
		{
			pST_PaperInfo pObj = *itPaper;
			if (pObj->nChkFlag == 0)
			{
				pPapers->nTotalPics -= pObj->lPic.size();
				SAFE_RELEASE(pObj);
				itPaper = pPapers->lPaper.erase(itPaper);
			}
			else
				itPaper++;
		}
	#endif
		//--


		return true;
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
	return false;
}

bool CDecompressThread::GetFileData2(std::string strFilePath, pPAPERSINFO pPapers)
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

		int nOmrDoubt = -1;
		int nOmrNull = -1;
		int nSnNull = -1;
		if (objData->has("nOmrDoubt"))
			nOmrDoubt = objData->get("nOmrDoubt").convert<int>();
		if (objData->has("nOmrNull"))
			nOmrNull = objData->get("nOmrNull").convert<int>();
		if (objData->has("nSnNull"))
			nSnNull = objData->get("nSnNull").convert<int>();

		if (objData->has("RecogMode"))
			pPapers->nRecogMode = objData->get("RecogMode").convert<int>();

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

#if 1
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
			for (int j = 0; itPaper != pPapers->lPaper.end(); j++, itPaper++)
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
				pPaper->strStudentInfo = jsnPaperObj->get("name").convert<std::string>();
				pPaper->strSN = jsnPaperObj->get("zkzh").convert<std::string>();
				pPaper->nQKFlag = jsnPaperObj->get("qk").convert<int>();

				Poco::JSON::Array::Ptr jsnSnArry = jsnPaperObj->getArray("snDetail");
				for (int k = 0; k < jsnSnArry->size(); k++)
				{
					Poco::JSON::Object::Ptr jsnSnObj = jsnSnArry->getObject(k);
					pSN_ITEM pSnItem = new SN_ITEM;
					pSnItem->nItem = jsnSnObj->get("sn").convert<int>();
					pSnItem->nRecogVal = jsnSnObj->get("val").convert<int>();

					Poco::JSON::Object::Ptr jsnSnPosition = jsnSnObj->getObject("position");
					pSnItem->rt.x = jsnSnPosition->get("x").convert<int>();
					pSnItem->rt.y = jsnSnPosition->get("y").convert<int>();
					pSnItem->rt.width = jsnSnPosition->get("w").convert<int>();
					pSnItem->rt.height = jsnSnPosition->get("h").convert<int>();

					if (jsnSnObj->has("Pos"))
					{
						Poco::JSON::Array::Ptr jsnPositionArry = jsnSnObj->getArray("Pos");
						for (int j = 0; j < jsnPositionArry->size(); j++)
						{
							Poco::JSON::Object::Ptr jsnRectInfoObj = jsnPositionArry->getObject(j);
							RECTINFO rc;
							rc.rt.x = jsnRectInfoObj->get("x").convert<int>();
							rc.rt.y = jsnRectInfoObj->get("y").convert<int>();
							rc.rt.width = jsnRectInfoObj->get("w").convert<int>();
							rc.rt.height = jsnRectInfoObj->get("h").convert<int>();
							pSnItem->lSN.push_back(rc);
						}
					}
					pPaper->lSnResult.push_back(pSnItem);
				}
				
				int* pOmrCountArry = NULL;
				if (_pModel_)
				{
					pOmrCountArry = new int[_pModel_->vecPaperModel.size()];
					memset(pOmrCountArry, 0, _pModel_->vecPaperModel.size() * sizeof(int));
					for (int i = 0; i < _pModel_->vecPaperModel.size(); i++)
						pOmrCountArry[i] = _pModel_->vecPaperModel[i]->lOMR2.size();	//每页图片上有多少OMR题
				}
				int nOmrRectInfoCount = 0;

				Poco::JSON::Array::Ptr jsnOmrArry = jsnPaperObj->getArray("omr");
				for (int k = 0; k < jsnOmrArry->size(); k++)
				{
					Poco::JSON::Object::Ptr jsnOmrObj = jsnOmrArry->getObject(k);
					OMR_RESULT omrResult;
					omrResult.nTH = jsnOmrObj->get("th").convert<int>();
					omrResult.nSingle = jsnOmrObj->get("type").convert<int>() - 1;
					omrResult.strRecogVal = jsnOmrObj->get("value").convert<std::string>();
					if (jsnOmrObj->has("value1"))
						omrResult.strRecogVal1 = jsnOmrObj->get("value1").convert<std::string>();
					if (jsnOmrObj->has("value2"))
						omrResult.strRecogVal2 = jsnOmrObj->get("value2").convert<std::string>();
					if (jsnOmrObj->has("value3"))
						omrResult.strRecogVal3 = jsnOmrObj->get("value3").convert<std::string>();
					omrResult.nDoubt = jsnOmrObj->get("doubt").convert<int>();

					//将OMR放入对应的页面信息中
					nOmrRectInfoCount++;
					int nPicOmrCount = 0;
					PIC_LIST::iterator  itPic = pPaper->lPic.begin();
					for (int m = 0; m < _pModel_->vecPaperModel.size(); m++, itPic++)
					{
						nPicOmrCount += pOmrCountArry[m];
						if (nOmrRectInfoCount <= nPicOmrCount)
							break;
					}

					Poco::JSON::Array::Ptr jsnPositionArry = jsnOmrObj->getArray("position");
					for (int j = 0; j < jsnPositionArry->size(); j++)
					{
						Poco::JSON::Object::Ptr jsnRectInfoObj = jsnPositionArry->getObject(j);
						RECTINFO rc;
						std::string strChar = jsnRectInfoObj->get("val").convert<std::string>();
						rc.nAnswer = (int)(strChar.c_str()) - 65;						
						rc.rt.x = jsnRectInfoObj->get("x").convert<int>();
						rc.rt.y = jsnRectInfoObj->get("y").convert<int>();
						rc.rt.width = jsnRectInfoObj->get("w").convert<int>();
						rc.rt.height = jsnRectInfoObj->get("h").convert<int>();
						omrResult.lSelAnswer.push_back(rc);

						if (_pModel_) (*itPic)->lNormalRect.push_back(rc);
					}

					pPaper->lOmrResult.push_back(omrResult);
				}
				SAFE_RELEASE_ARRY(pOmrCountArry);

				if (jsnPaperObj->has("electOmr"))
				{
					Poco::JSON::Array::Ptr jsnElectOmrArry = jsnPaperObj->getArray("electOmr");
					for (int k = 0; k < jsnElectOmrArry->size(); k++)
					{
						Poco::JSON::Object::Ptr jsnElectOmrObj = jsnElectOmrArry->getObject(k);
						ELECTOMR_QUESTION electOmrResult;
						electOmrResult.nDoubt = jsnElectOmrObj->get("doubt").convert<int>();
						electOmrResult.sElectOmrGroupInfo.nGroupID = jsnElectOmrObj->get("th").convert<int>();
						electOmrResult.sElectOmrGroupInfo.nAllCount = jsnElectOmrObj->get("allItems").convert<int>();
						electOmrResult.sElectOmrGroupInfo.nRealCount = jsnElectOmrObj->get("realItem").convert<int>();
						electOmrResult.strRecogResult = jsnElectOmrObj->get("value").convert<std::string>();

						Poco::JSON::Array::Ptr jsnPositionArry = jsnElectOmrObj->getArray("position");
						for (int j = 0; j < jsnPositionArry->size(); j++)
						{
							Poco::JSON::Object::Ptr jsnRectInfoObj = jsnPositionArry->getObject(j);
							RECTINFO rc;
							std::string strChar = jsnRectInfoObj->get("val").convert<std::string>();
							rc.nAnswer = (int)(strChar.c_str()) - 65;
							rc.rt.x = jsnRectInfoObj->get("x").convert<int>();
							rc.rt.y = jsnRectInfoObj->get("y").convert<int>();
							rc.rt.width = jsnRectInfoObj->get("w").convert<int>();
							rc.rt.height = jsnRectInfoObj->get("h").convert<int>();
							electOmrResult.lItemInfo.push_back(rc);
						}

						pPaper->lElectOmrResult.push_back(electOmrResult);
					}
				}
				//--------------------------------------------------
				if (jsnPaperObj->has("ChkPoint"))
				{
					PIC_LIST::iterator itPic = pPaper->lPic.begin();

					Poco::JSON::Array::Ptr jsnPaperFixArry = jsnPaperObj->getArray("ChkPoint");
					for (int j = 0; j < jsnPaperFixArry->size(); j++, itPic++)
					{
						Poco::JSON::Object::Ptr jsnPicObj = jsnPaperFixArry->getObject(j);
						Poco::JSON::Array::Ptr jsnPaperFixArry = jsnPicObj->getArray("FL");
						for (int k = 0; k < jsnPaperFixArry->size(); k++)
						{
							Poco::JSON::Object::Ptr jsnRectInfoObj = jsnPaperFixArry->getObject(k);
							RECTINFO rc;
							rc.rt.x = jsnRectInfoObj->get("x").convert<int>();
							rc.rt.y = jsnRectInfoObj->get("y").convert<int>();
							rc.rt.width = jsnRectInfoObj->get("w").convert<int>();
							rc.rt.height = jsnRectInfoObj->get("h").convert<int>();

							if (jsnRectInfoObj->has("Th"))
								rc.nTH = jsnRectInfoObj->get("Th").convert<int>();
							(*itPic)->lFix.push_back(rc);
						}
						Poco::JSON::Array::Ptr jsnPaperModelCharAncharPointArry = jsnPicObj->getArray("MFL");
						for (int k = 0; k < jsnPaperModelCharAncharPointArry->size(); k++)
						{
							Poco::JSON::Object::Ptr jsnRectInfoObj = jsnPaperModelCharAncharPointArry->getObject(k);
							RECTINFO rc;
							rc.rt.x = jsnRectInfoObj->get("x").convert<int>();
							rc.rt.y = jsnRectInfoObj->get("y").convert<int>();
							rc.rt.width = jsnRectInfoObj->get("w").convert<int>();
							rc.rt.height = jsnRectInfoObj->get("h").convert<int>();
							(*itPic)->lModelWordFix.push_back(rc);
						}
						Poco::JSON::Array::Ptr jsnPaperCharAncharAreaArry = jsnPicObj->getArray("CL");
						for (int k = 0; k < jsnPaperCharAncharAreaArry->size(); k++)
						{
							Poco::JSON::Object::Ptr jsnRectInfoObj = jsnPaperCharAncharAreaArry->getObject(k);
							pST_CHARACTER_ANCHOR_AREA pCharacterAnchorArea = new ST_CHARACTER_ANCHOR_AREA();
							pCharacterAnchorArea->rt.x = jsnRectInfoObj->get("x").convert<int>();
							pCharacterAnchorArea->rt.y = jsnRectInfoObj->get("y").convert<int>();
							pCharacterAnchorArea->rt.width = jsnRectInfoObj->get("w").convert<int>();
							pCharacterAnchorArea->rt.height = jsnRectInfoObj->get("h").convert<int>();
							(*itPic)->lCharacterAnchorArea.push_back(pCharacterAnchorArea);

							Poco::JSON::Array::Ptr jsnPaperCharAncharPointArry = jsnRectInfoObj->getArray("AP");
							for (int m = 0; m < jsnPaperCharAncharPointArry->size(); m++)
							{
								Poco::JSON::Object::Ptr jsnRectInfoObj2 = jsnPaperCharAncharPointArry->getObject(m);
								pST_CHARACTER_ANCHOR_POINT pCharactAnchorPoint = new ST_CHARACTER_ANCHOR_POINT();
								pCharactAnchorPoint->rc.rt.x = jsnRectInfoObj2->get("x").convert<int>();
								pCharactAnchorPoint->rc.rt.y = jsnRectInfoObj2->get("y").convert<int>();
								pCharactAnchorPoint->rc.rt.width = jsnRectInfoObj2->get("w").convert<int>();
								pCharactAnchorPoint->rc.rt.height = jsnRectInfoObj2->get("h").convert<int>();
								pCharacterAnchorArea->vecCharacterRt.push_back(pCharactAnchorPoint);
							}
						}
					}
				}
				//--------------------------------------------------
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
			USES_CONVERSION;
			CString strMsg;
			strMsg.Format(_T("解压(%s)完成\r\n"), A2T(strLog.c_str()));
			((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
			return false;
		}
#endif
		pPapers->nPkgOmrDoubt = nOmrDoubt;
		pPapers->nPkgOmrNull = nOmrNull;
		pPapers->nPkgSnNull = nSnNull;

		pPapers->nExamID = nExamId;
		pPapers->nSubjectID = nSubjectId;
		pPapers->nTeacherId = nTeacherId;
		pPapers->nUserId = nUserId;
		pPapers->nTotalPaper = nStudentNum;
		pPapers->nPaperCount = nStudentNum;
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
	USES_CONVERSION;
	bool bErr = false;
	std::string strErr;
	std::string strPapersPath = CMyCodeConvert::Gb2312ToUtf8(strPath);
	try
	{
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
						pPic->pPaper = *it;
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
					pPaper->nIndex = atoi(strPaperName.substr(1).c_str());
					pPapers->lPaper.push_back(pPaper);
				}
				pPapers->nTotalPics++;						//图片数增加一张
			}
			it++;
		}
	}
	catch (Poco::Exception& exc)
	{
		strErr = "搜索文件夹图片失败，" + CMyCodeConvert::Utf8ToGb2312(exc.displayText());
		bErr = true;
	}

	if (bErr)
	{
		CString strMsg;
		strMsg.Format(_T("%s"), A2T(strErr.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
	}
}

bool CDecompressThread::SearchDirPaper(pPAPERSINFO pPapers, std::string strPath)
{
	USES_CONVERSION;
	bool bErr = false;
	std::string strErr;
	int nModelPicNums = _pModel_->nPicNum;
	try
	{
		std::string strPapersName = strPath.substr(strPath.rfind('\\') + 1, strPath.length());

		pPapers->strPapersName = strPapersName;

		std::string strDirName = strPapersName;		//gb2312

		std::vector<std::string> lFileName;
		Poco::DirectoryIterator itSub(CMyCodeConvert::Gb2312ToUtf8(strPath));
		Poco::DirectoryIterator endSub;
		while (itSub != endSub)
		{
			Poco::Path pSubFile(itSub->path());
			if (itSub->isFile())
			{
				std::string strOldFileName = pSubFile.getFileName();

				if (pSubFile.getExtension() == "jpg")
				{
					lFileName.push_back(strOldFileName);
				}
			}
			itSub++;
		}

		if (lFileName.size() % nModelPicNums != 0)
		{
			char szErrorInfo[MAX_PATH] = { 0 };
			sprintf_s(szErrorInfo, "****************\r\n扫描到文件夹[%s]试卷数量 = %d,模板要求每考生 %d 张试卷, 请检查是否有考生试卷缺失\r\n****************\r\n", \
					  strDirName.c_str(), lFileName.size(), nModelPicNums);

			CString strMsg;
			strMsg.Format(_T("%s"), A2T(szErrorInfo));
			((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
			return false;
		}

		//文件复制本地临时目录
		std::string strSaveBasePath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath)) + "tmpDir\\";
		std::string strSubPaperPath = strSaveBasePath + CMyCodeConvert::Gb2312ToUtf8(strDirName);
		Poco::File tmpPath(strSubPaperPath);
		if (tmpPath.exists())
			tmpPath.remove(true);
		tmpPath.createDirectories();

		pPapers->strPapersPath = CMyCodeConvert::Utf8ToGb2312(strSubPaperPath);
		pPapers->nPaperCount = lFileName.size() / nModelPicNums;
		pPapers->strPapersName = strDirName;

		int i = 0;
		pST_PaperInfo pPaper = NULL;
		std::sort(lFileName.begin(), lFileName.end(), SortbyNumASC);
		std::vector<std::string>::iterator itName = lFileName.begin();
		for (; itName != lFileName.end(); itName++)
		{
			TRACE("%s\n", (*itName).c_str());	//(*itName).c_str()

			char szNewName[100] = { 0 };
			sprintf_s(szNewName, "S%d_%s", i / nModelPicNums + 1, (*itName).c_str());

			std::string strNewName = szNewName;
			std::string strNewFilePath = strSubPaperPath + "\\" + strNewName;

			std::string strFileOldPath = CMyCodeConvert::Gb2312ToUtf8(strPath) + "\\" + *itName;
			Poco::File oldFile(strFileOldPath);
			oldFile.copyTo(strNewFilePath);

			if (i % nModelPicNums == 0)
			{
				pPaper = new ST_PaperInfo;
				pPapers->lPaper.push_back(pPaper);
				char szStudentInfo[20] = { 0 };
				sprintf_s(szStudentInfo, "S%d", i / nModelPicNums + 1);
				pPaper->strStudentInfo = szStudentInfo;
				pPaper->pModel = _pModel_;
				pPaper->pPapers = pPapers;
				pPaper->pSrcDlg = m_pDlg;
				pPaper->nIndex = i / nModelPicNums + 1;
			}
			pST_PicInfo pPic = new ST_PicInfo;
			pPaper->lPic.push_back(pPic);

			pPic->strPicName = strNewName;
			pPic->strPicPath = CMyCodeConvert::Utf8ToGb2312(strNewFilePath);	// strNewFilePath;
			pPic->pPaper = pPaper;
			i++;
		}
	}
	catch (Poco::Exception& exc)
	{
		strErr = "----------\r\n搜索文件夹图片失败，" + CMyCodeConvert::Utf8ToGb2312(exc.displayText()) + "----------\r\n";
		bErr = true;
	}
	if (bErr)
	{
		CString strMsg;
		strMsg.Format(_T("%s"), A2T(strErr.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
		return false;
	}
	return true;
}

void CDecompressThread::SearchDirPapers(std::string strPath)
{
	USES_CONVERSION;
	bool bErr = false;
	std::string strErr;
	int nModelPicNums = _pModel_->nPicNum;
	try
	{
		Poco::DirectoryIterator it(_strJpgSearchPath_);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			if (it->isDirectory())
			{
				std::string strDirName = p.getFileName();

				pPAPERSINFO pPapers = NULL;
				pPapers = new PAPERSINFO;

				pPapers->nSendEzs = 2;

				g_fmPapers.lock();
				g_lPapers.push_back(pPapers);
				g_fmPapers.unlock();
				
				SearchDirPaper(pPapers, CMyCodeConvert::Utf8ToGb2312(p.toString()));

				//添加到识别任务列表
				PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
				for (; itPaper != pPapers->lPaper.end(); itPaper++)
				{
					pRECOGTASK pTask = new RECOGTASK;
					pTask->pPaper = *itPaper;
					g_lRecogTask.push_back(pTask);
				}
			}
			it++;
		}
	}
	catch (Poco::Exception& exc)
	{
		strErr = "----------\r\n搜索试卷袋文件夹失败，" + CMyCodeConvert::Utf8ToGb2312(exc.displayText()) + "----------\r\n";
		bErr = true;
	}
	if (bErr)
	{
		CString strMsg;
		strMsg.Format(_T("%s"), A2T(strErr.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
	}
}

// bool CDecompressThread::calcStatistics(pPAPERSINFO pPapers)
// {
// 	std::stringstream ss;
// 	int nOmrCount = 0;
// 
// 	//omr统计
// 	PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
// 	for (; itPaper != pPapers->lPaper.end(); itPaper++)
// 	{
// 		pST_PaperInfo pPaper = *itPaper;
// 		nOmrCount += pPaper->lOmrResult.size();
// 
// 		OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
// 		for (int i = 1; itOmr != pPaper->lOmrResult.end(); i++, itOmr++)
// 		{
// 			ss.str("");
// 			ss << pPapers->strPapersName << ":" << pPaper->strStudentInfo << ":" << i;
// 			std::string strTmp = ss.str();
// 			std::map<std::string, std::string>::iterator itAnswer = answerMap.find(ss.str());
// 			if (itAnswer != answerMap.end())
// 			{
// 				if (itOmr->strRecogVal != itAnswer->second)
// 					pPapers->nOmrError_1++;
// 				if (itOmr->strRecogVal2 != itAnswer->second)
// 					pPapers->nOmrError_2++;
// 			}
// 		}
// 	}
// 	PAPER_LIST::iterator itPaper2 = pPapers->lIssue.begin();
// 	for (; itPaper2 != pPapers->lIssue.end(); itPaper2++)
// 	{
// 		pST_PaperInfo pPaper = *itPaper2;
// 		nOmrCount += pPaper->lOmrResult.size();
// 
// 		OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
// 		for (int i = 1; itOmr != pPaper->lOmrResult.end(); i++, itOmr++)
// 		{
// 			ss.str("");
// 			ss << pPapers->strPapersName << ":" << pPaper->strStudentInfo << ":" << i;
// 			std::map<std::string, std::string>::iterator itAnswer = answerMap.find(ss.str());
// 			if (itAnswer != answerMap.end())
// 			{
// 				if (itOmr->strRecogVal != itAnswer->second)
// 					pPapers->nOmrError_1++;
// 				if (itOmr->strRecogVal2 != itAnswer->second)
// 					pPapers->nOmrError_2++;
// 			}
// 		}
// 	}
// 
// 
// 	USES_CONVERSION;
// 	CString strMsg;
// 
// 	char szStatisticsInfo[300] = { 0 };
// 	sprintf_s(szStatisticsInfo, "\n识别错误信息统计: omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\n", (float)pPapers->nOmrError_1 / nOmrCount * 100, pPapers->nOmrError_1, nOmrCount, \
// 			  (float)pPapers->nOmrError_2 / nOmrCount * 100, pPapers->nOmrError_2, nOmrCount);
// 
// 	strMsg.Format(_T("%s识别完成\r\n%s\r\n"), A2T(pPapers->strPapersName.c_str()), A2T(szStatisticsInfo));
// 	((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
// 	return true;
// }

