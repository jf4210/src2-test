#include "DecompressThread.h"
#include <algorithm>


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
						return strlen(szXPart) > strlen(szYPart);		//��С��ͬ������Խ��Խ��ǰ
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
		_pPapers->nTotalPics++;						//ͼƬ������һ��

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
	std::string strLog = "����Ծ����ѹ����: " + pTask->strFilePath;
	g_Log.LogOut(strLog);

	std::ifstream inp(pTask->strFilePath.c_str(), std::ios::binary);
	if (!inp)
	{
		strLog = "���Ծ���ļ�ʧ��";
		g_Log.LogOutError(strLog);
		return;
	}
	std::string strOutDir = SysSet.m_strDecompressPath + "\\" + CMyCodeConvert::Gb2312ToUtf8(pTask->strFileName);

	pPAPERS_DETAIL pPapers = new PAPERS_DETAIL;
	pPapers->strPapersName = pTask->strFileName;
	pPapers->strPapersPath = CMyCodeConvert::Utf8ToGb2312(strOutDir);
	
	Poco::Zip::Decompress dec(inp, strOutDir);
	DecompressHandler handler(pPapers);
	dec.EError += Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(&handler, &DecompressHandler::onError);
	dec.EOk += Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(&handler, &DecompressHandler::onOk);
	dec.decompressAllFiles();
	dec.EError -= Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(&handler, &DecompressHandler::onError);
	dec.EOk -= Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(&handler, &DecompressHandler::onOk);
	//��ѹ���
	pPapers->lPaper.sort(SortByPaper);

	//��ȡ�Ծ���ļ���������ļ���ȡ�Ծ����Ϣ
	std::string strPapersFilePath = strOutDir + "\\papersInfo.dat";
	GetFileData(strPapersFilePath, pPapers);

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
	
	g_fmPapers.lock();
	g_lPapers.push_back(pPapers);
	g_fmPapers.unlock();
}

void CDecompressThread::GetFileData(std::string strFilePath, pPAPERS_DETAIL pPapers)
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

	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strJsnData);		//strJsnData
		Poco::JSON::Object::Ptr objData = result.extract<Poco::JSON::Object::Ptr>();

		int nExamId		= objData->get("examId").convert<int>();
		int nSubjectId	= objData->get("subjectId").convert<int>();
		int nTeacherId	= objData->get("nTeacherId").convert<int>();
		int nUserId		= objData->get("nUserId").convert<int>();
		int nStudentNum = objData->get("scanNum").convert<int>();
		std::string strUploader = objData->get("uploader").convert<std::string>();
		std::string strEzs	= objData->get("ezs").convert<std::string>();

		Poco::JSON::Array::Ptr jsnDetailArry = objData->getArray("detail");
		for (int i = 0; i < jsnDetailArry->size(); i++)
		{
			Poco::JSON::Object::Ptr jsnPaperObj = jsnDetailArry->getObject(i);
			std::string strStudentInfo = jsnPaperObj->get("name").convert<std::string>();

			pPAPER_INFO pPaper = NULL;
			LIST_PAPER_INFO::iterator itPaper = pPapers->lPaper.begin();
			for (int j = 0; itPaper != pPapers->lPaper.end(); itPaper++)
			{
				if ((*itPaper)->strName == strStudentInfo)
				{
					pPaper = *itPaper;
					break;
				}
			}
			if (pPaper)
			{
				pPaper->strZkzh = jsnPaperObj->get("zkzh").convert<std::string>();

 				Poco::JSON::Array::Ptr jsnSnArry = jsnPaperObj->getArray("snDetail");

				std::stringstream jsnSnString;
				jsnSnArry->stringify(jsnSnString, 0);
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
				std::stringstream jsnOmrString;
				jsnOmrArry->stringify(jsnOmrString, 0);
				pPaper->strOmrDetail = jsnOmrString.str();
			}
			else
			{
				std::string strLog;
				strLog = "����OMR��SN����Ϣ��ѧ���Ծ�һ�µ����������: studentName = " + strStudentInfo;
				g_Log.LogOutError(strLog);
			}
		}

		pPapers->nExamID	= nExamId;
		pPapers->nSubjectID = nSubjectId;
		pPapers->nTeacherId = nTeacherId;
		pPapers->nUserId	= nUserId;
		pPapers->nTotalPaper = nStudentNum;
		pPapers->strUploader = strUploader;
		pPapers->strEzs		= "ezs=" + strEzs;
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("�����Ծ���ļ������ļ�ʧ��: ");
		strErrInfo.append(jsone.message());
		g_Log.LogOutError(strErrInfo);
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("�����Ծ���ļ������ļ�ʧ��2: ");
		strErrInfo.append(exc.message());
		g_Log.LogOutError(strErrInfo);
	}

}
