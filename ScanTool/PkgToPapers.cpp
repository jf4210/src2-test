#include "stdafx.h"
#include "PkgToPapers.h"

bool SortbyNumASC2(const std::string& x, const std::string& y)
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

bool SortByPaper(const pST_PaperInfo& x, const pST_PaperInfo& y)
{
	return SortbyNumASC2(x->strStudentInfo, y->strStudentInfo);
}

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
	strUnzipBasePath = g_strCurrentPath + _T("Paper\\Tmp2\\");

	try
	{
		Poco::File p(CMyCodeConvert::Gb2312ToUtf8(T2A(strUnzipBasePath)));
		p.createDirectories();
	}
	catch (...)
	{
	}

	CString strUnzipPath = _T("");
	strUnzipPath = strUnzipBasePath + A2T(strPkgName.c_str());

	CZipObj zipObj;
	zipObj.setLogger(g_pLogger);
	zipObj.UnZipFile(A2T(strPkgPath.c_str()), strUnzipPath);		//*****************************************************

	pPAPERSINFO pPapers = new PAPERSINFO;
	pPapers->nPapersType = 1;
	pPapers->strPapersName = strPkgName;
	SearchExtractFile(pPapers, T2A(strUnzipPath));
	pPapers->lPaper.sort(SortByPaper);
	std::string strPapersFilePath = strUnzipPath + "\\papersInfo.dat";
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
				if (jsnPaperObj->has("modify"))
					pPaper->bModifyZKZH = jsnPaperObj->get("modify").convert<bool>();
				if (jsnPaperObj->has("reScan"))
					pPaper->bReScan = jsnPaperObj->get("reScan").convert<bool>();
				if (jsnPaperObj->has("IssueList"))
				{
					int nIssueList = jsnPaperObj->get("IssueList").convert<int>();
					if (nIssueFlag)
					{
						pPapers->lPaper.erase(itPaper);
						pPapers->lIssue.push_back(pPaper);
					}
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
