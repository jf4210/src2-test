#include "stdafx.h"
#include "PkgToPapers.h"


CPkgToPapers::CPkgToPapers()
{
}


CPkgToPapers::~CPkgToPapers()
{
}

pPAPERSINFO CPkgToPapers::Pkg2Papers(std::string strPkgPath)	//gb2312
{
	USES_CONVERSION;
	std::string strPkgName;
	int nPos1 = strPkgPath.rfind('.');
	int nPos2 = strPkgPath.rfind('\\');
	strPkgName = strPkgPath.substr(nPos2 + 1, nPos1 - nPos2 - 1);
	CString strUnzipBasePath = _T("");
	strUnzipBasePath = g_strCurrentPath + _T("Paper\\Tmp2\\") + A2T(strPkgName.c_str());

	CZipObj zipObj;
	zipObj.setLogger(g_pLogger);
	zipObj.UnZipFile(A2T(strPkgPath.c_str()), strUnzipBasePath);

	pPAPERSINFO pPapers = new PAPERSINFO;
	pPapers->nPapersType = 1;
	pPapers->strPapersName = strPkgName;
	SearchExtractFile(pPapers, T2A(strUnzipBasePath));
	std::string strPapersFilePath = strUnzipBasePath + "\\papersInfo.dat";
	if (!GetFileData(strPapersFilePath, pPapers))
		SAFE_RELEASE(pPapers);
	return pPapers;
}

void CPkgToPapers::SearchExtractFile(pPAPERSINFO pPapers, std::string strPath)
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
			pPic->strPicPath = strPath + "\\" + pPic->strPicName;

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
				pPaper->pPapers = pPapers;
				pPapers->lPaper.push_back(pPaper);
			}
		}
		it++;
	}
}

bool CPkgToPapers::GetFileData(std::string strFilePath, pPAPERSINFO pPapers)
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

		int nExamID = objData->get("examId").convert<int>();
		int nSubjectID = objData->get("subjectId").convert<int>();
		int nTeacherId = objData->get("nTeacherId").convert<int>();
		int nUserId = objData->get("nUserId").convert<int>();
		int nStudentNum = objData->get("scanNum").convert<int>();
		std::string strUploader = objData->get("uploader").convert<std::string>();
		std::string strEzs = objData->get("ezs").convert<std::string>();
		if (objData->has("desc"))
			pPapers->strPapersDesc = CMyCodeConvert::Utf8ToGb2312(objData->get("desc").convert<std::string>());
		
		pPapers->nOmrDoubt = nOmrDoubt;
		pPapers->nOmrNull = nOmrNull;
		pPapers->nSnNull = nSnNull;

		pPapers->nExamID = nExamID;
		pPapers->nSubjectID = nSubjectID;
		pPapers->nTeacherId = nTeacherId;
		pPapers->nUserId = nUserId;
		pPapers->nPaperCount = nStudentNum;
// 		pPapers->strUploader = strUploader;
// 		pPapers->strEzs = strEzs;		//"ezs=" + strEzs;

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
				pPaper->strSN = jsnPaperObj->get("zkzh").convert<std::string>();
				pPaper->nQKFlag = jsnPaperObj->get("qk").convert<int>();
				int nIssueFlag = 0;
				if (jsnPaperObj->has("issueFlag"))
					nIssueFlag = jsnPaperObj->get("issueFlag").convert<int>();

				switch (nIssueFlag)
				{
					case 0:
						break;
					case 1:
						pPaper->bModifyZKZH = true;
						break;
					case 2:
						break;
					case 3:
						pPaper->bModifyZKZH = true;
						pPaper->bReScan = true;
						break;
				}
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
				SAFE_RELEASE(pObj);
				itPaper = pPapers->lPaper.erase(itPaper);
			}
			else
				itPaper++;
		}
		//--

		return true;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("CPkgToPapers-->解析试卷袋文件夹中文件失败: ");
		strErrInfo.append(exc.message());
		g_pLogger->information(strErrInfo);
	}
	return false;
}
