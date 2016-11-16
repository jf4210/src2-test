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
	std::string strLog = "����Ծ�����ѹ����: " + pTask->strFilePath;
	g_Log.LogOut(strLog);

	std::ifstream inp(pTask->strFilePath.c_str(), std::ios::binary);
	if (!inp)
	{
		strLog = "���Ծ����ļ�ʧ��";
		g_Log.LogOutError(strLog);
		return;
	}
	std::string strOutDir = CMyCodeConvert::Gb2312ToUtf8(pTask->strDecompressDir + "\\" + pTask->strFileBaseName);

	pPAPERSINFO pPapers = NULL;
	if (pTask->nTaskType == 3 || pTask->nTaskType == 5)
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
		std::string strLog = "��ѹ���ļ�ʧ��:" + pTask->strFilePath;
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
	if (CHDIR(CMyCodeConvert::Utf8ToGb2312(strOutDir).c_str()))
	{
		std::string strLog = "�л�Ŀ¼ʧ��:" + strOutDir;
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
#else
	std::string strBaseDir = CMyCodeConvert::Utf8ToGb2312(strOutDir);
	char szBaseDir[256] = { 0 };
	strncpy_s(szBaseDir, strBaseDir.c_str(), strBaseDir.length());
#endif
	ret = do_extract_all(uf, opt_do_extract_withoutpath, opt_overwrite, password, szBaseDir);
	unzClose(uf);

	if (ret != 0)
	{
		//************	ע�⣺��ѹʧ����Ҫ�ٴγ���	*************************

		std::string strLog = "��ѹ�Ծ���(";
		strLog.append(pTask->strFileBaseName);
		strLog.append(")ʧ��, ·��: " + pTask->strFilePath);
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
#ifndef DecompressTest
	CHDIR(pTask->strDecompressDir.c_str());		//�л��ؽ�ѹ��Ŀ¼������ɾ��ѹ���ļ���ʧ��
#endif
	if (pTask->nTaskType == 3 || pTask->nTaskType == 5)
	{
		SearchExtractFile(pPapers, pPapers->strPapersPath);

		//��ѹ���
		pPapers->lPaper.sort(SortByPaper);
	}
	


	USES_CONVERSION;
	if (pTask->nTaskType == 1)
	{
		CString strMsg;
		strMsg.Format(_T("��ѹ(%s)���\r\n"), A2T(pTask->strSrcFileName.c_str()));
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
			strMsg.Format(_T("��ѹ(%s)���\r\n"), A2T(pTask->strSrcFileName.c_str()));
			((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
		}
		catch (Poco::Exception& e)
		{
		}
		SAFE_RELEASE(pPapers);
	}
	else if (pTask->nTaskType == 3)
	{
		//��ȡ�Ծ����ļ���������ļ���ȡ�Ծ�����Ϣ
		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
		bool bResult_Data = GetFileData(CMyCodeConvert::Utf8ToGb2312(strPapersFilePath), pPapers);

		//���ӵ�ʶ�������б�
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
		strMsg.Format(_T("ʶ��ģ���ѹ(%s)���\r\n"), A2T(pTask->strSrcFileName.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);

		if (_pModel_)
		{
			delete _pModel_;
			_pModel_ = NULL;
		}

		std::string strModelPath = CMyCodeConvert::Utf8ToGb2312(strOutDir);
		_pModel_ = LoadModelFile(A2T(strModelPath.c_str()));
		if (_nUseNewParam_)
			InitModelRecog(_pModel_, strOutDir);

		strMsg.Format(_T("ģ�����(%s)���\r\n"), A2T(pTask->strSrcFileName.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
	}
	else if (pTask->nTaskType == 5)
	{
		//��ȡ�Ծ����ļ���������ļ���ȡ�Ծ�����Ϣ
		std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
		bool bResult_Data = GetFileData2(CMyCodeConvert::Utf8ToGb2312(strPapersFilePath), pPapers);
		
		std::string strStatisticsInfo = calcStatistics(pPapers);

		CString strMsg;
		strMsg.Format(_T("%s"), A2T(strStatisticsInfo.c_str()));
		((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);

		//ɾ��Դ�ļ���
		try
		{
			Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
			if (srcFileDir.exists())
				srcFileDir.remove(true);
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "ɾ���ļ���(" + pPapers->strPapersPath + ")ʧ��: " + exc.message();
			g_Log.LogOutError(strErr);
		}
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
		result = parser.parse(strFileData);		//strJsnData
		Poco::JSON::Object::Ptr objData = result.extract<Poco::JSON::Object::Ptr>();

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

		pPapers->nExamID = nExamID;
		pPapers->nSubjectID = nSubjectID;
		pPapers->nTeacherId = nTeacherId;
		pPapers->nUserId = nUserId;
		pPapers->nTotalPaper = nStudentNum;
		pPapers->nPaperCount = nStudentNum;
		pPapers->strUploader = strUploader;
		pPapers->strEzs = strEzs;		//"ezs=" + strEzs;

		return true;
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("�����Ծ����ļ������ļ�ʧ��: ");
		strErrInfo.append(jsone.message());
		g_Log.LogOutError(strErrInfo);
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("�����Ծ����ļ������ļ�ʧ��2: ");
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
					pPaper->lSnResult.push_back(pSnItem);
				}

				Poco::JSON::Array::Ptr jsnOmrArry = jsnPaperObj->getArray("omr");
				for (int k = 0; k < jsnOmrArry->size(); k++)
				{
					Poco::JSON::Object::Ptr jsnOmrObj = jsnOmrArry->getObject(k);
					OMR_RESULT omrResult;
					omrResult.nTH = jsnOmrObj->get("th").convert<int>();
					omrResult.nSingle = jsnOmrObj->get("type").convert<int>() - 1;
					omrResult.strRecogVal = jsnOmrObj->get("value").convert<std::string>();
					if (jsnOmrObj->has("value2"))
						omrResult.strRecogVal2 = jsnOmrObj->get("value2").convert<std::string>();
					omrResult.nDoubt = jsnOmrObj->get("doubt").convert<int>();
					pPaper->lOmrResult.push_back(omrResult);
				}



				if (jsnPaperObj->has("electOmr"))
				{
					Poco::JSON::Array::Ptr jsnElectOmrArry = jsnPaperObj->getArray("electOmr");
					for (int k = 0; k < jsnElectOmrArry->size(); k++)
					{
						Poco::JSON::Object::Ptr jsnElectOmrObj = jsnElectOmrArry->getObject(k);
						ELECTOMR_QUESTION electOmrResult;
						electOmrResult.sElectOmrGroupInfo.nGroupID = jsnElectOmrObj->get("th").convert<int>();
						electOmrResult.sElectOmrGroupInfo.nAllCount = jsnElectOmrObj->get("allItems").convert<int>();
						electOmrResult.sElectOmrGroupInfo.nRealCount = jsnElectOmrObj->get("realItem").convert<int>();
						electOmrResult.strRecogResult = jsnElectOmrObj->get("value").convert<std::string>();
						pPaper->lElectOmrResult.push_back(electOmrResult);
					}
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
			strLog = "����OMR��SN����Ϣ��ѧ���Ծ���һ�µ����������: Դ(" + strStudentList_Src + ")δ����, ��ѹ��(" + strStudentList_Decompress + ")";
			g_Log.LogOutError(strLog);
			USES_CONVERSION;
			CString strMsg;
			strMsg.Format(_T("��ѹ(%s)���\r\n"), A2T(strLog.c_str()));
			((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
			return false;
		}
#endif
		pPapers->nOmrDoubt = nOmrDoubt;
		pPapers->nOmrNull = nOmrNull;
		pPapers->nSnNull = nSnNull;

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
		strErrInfo.append("�����Ծ����ļ������ļ�ʧ��: ");
		strErrInfo.append(jsone.message());
		g_Log.LogOutError(strErrInfo);
		return false;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("�����Ծ����ļ������ļ�ʧ��2: ");
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
			pPapers->nTotalPics++;						//ͼƬ������һ��
		}
		it++;
	}
}

// bool CDecompressThread::calcStatistics(pPAPERSINFO pPapers)
// {
// 	std::stringstream ss;
// 	int nOmrCount = 0;
// 
// 	//omrͳ��
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
// 	sprintf_s(szStatisticsInfo, "\nʶ�������Ϣͳ��: omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\n", (float)pPapers->nOmrError_1 / nOmrCount * 100, pPapers->nOmrError_1, nOmrCount, \
// 			  (float)pPapers->nOmrError_2 / nOmrCount * 100, pPapers->nOmrError_2, nOmrCount);
// 
// 	strMsg.Format(_T("%sʶ�����\r\n%s\r\n"), A2T(pPapers->strPapersName.c_str()), A2T(szStatisticsInfo));
// 	((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
// 	return true;
// }
