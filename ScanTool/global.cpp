#include "stdafx.h"
#include "global.h"


//扫描用
void CopyData(char *dest, const char *src, int dataByteSize, bool isConvert, int height)
{
	char * p = dest;
	if (!isConvert)
	{
		memcpy(dest, src, dataByteSize);
		return;
	}
	if (height <= 0) return;
	//int height = dataByteSize/rowByteSize;
	int rowByteSize = dataByteSize / height;
	src = src + dataByteSize - rowByteSize;
	for (int i = 0; i < height; i++)
	{
		memcpy(dest, src, rowByteSize);
		dest += rowByteSize;
		src -= rowByteSize;
	}
}

bool SortByArea(cv::Rect& rt1, cv::Rect& rt2)
{
	return rt1.area() > rt2.area() ? true : (rt1.area() < rt2.area() ? false : (rt1.x > rt2.x ? true : false));
}
bool SortByPositionX(RECTINFO& rc1, RECTINFO& rc2)
{
	bool bResult = true;
	bResult = rc1.rt.x < rc2.rt.x ? true : false;
	if (!bResult)
	{
		if (rc1.rt.x == rc2.rt.x)
			bResult = rc1.rt.y < rc2.rt.y ? true : false;
	}
	return bResult;
}
bool SortByPositionY(RECTINFO& rc1, RECTINFO& rc2)
{
	bool bResult = true;
	bResult = rc1.rt.y < rc2.rt.y ? true : false;
	if (!bResult)
	{
		if (rc1.rt.y == rc2.rt.y)
			bResult = rc1.rt.x < rc2.rt.x ? true : false;
	}
	return bResult;
}

bool SortByPositionX2(cv::Rect& rt1, cv::Rect& rt2)
{
	bool bResult = true;
	bResult = rt1.x < rt2.x ? true : false;
	if (!bResult)
	{
		if (rt1.x == rt2.x)
			bResult = rt1.y < rt2.y ? true : false;
	}
	return bResult;
}

bool SortByPositionY2(cv::Rect& rt1, cv::Rect& rt2)
{
	bool bResult = true;
	bResult = rt1.y < rt2.y ? true : false;
	if (!bResult)
	{
		if (rt1.y == rt2.y)
			bResult = rt1.x < rt2.x ? true : false;
	}
	return bResult;
}

bool SortByPositionXYInterval(cv::Rect& rt1, cv::Rect& rt2)
{
	bool bResult = true;

	if (abs(rt1.y - rt2.y) > 5)
	{
		return rt1.y < rt2.y ? true : false;
	}
	else
	{
		bResult = rt1.x < rt2.x ? true : false;
		if (!bResult)
			bResult = rt1.x == rt2.x?  rt1.y < rt2.y : false;
	}
	return bResult;
}

bool ZipFile(CString strSrcPath, CString strDstPath, CString strExtName /*= _T(".zip")*/)
{
	USES_CONVERSION;
//	CString modelName = strSrcPath.Right(strSrcPath.GetLength() - strSrcPath.ReverseFind('\\') - 1);
	CString zipName = strDstPath + strExtName;
	std::string strUtf8ZipName = CMyCodeConvert::Gb2312ToUtf8(T2A(zipName));

	Poco::File p(strUtf8ZipName);	//T2A(zipName)
	if (p.exists())
		p.remove(true);

	HZIP hz = CreateZip(zipName, 0);
//	std::string strModelPath = T2A(strSrcPath);
	std::string strUtf8ModelPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strSrcPath));

	Poco::DirectoryIterator it(strUtf8ModelPath);	//strModelPath
	Poco::DirectoryIterator end;
	while (it != end)
	{
		Poco::Path p(it->path());
//		std::string strZipFileName = p.getFileName();
		std::string strPath = CMyCodeConvert::Utf8ToGb2312(p.toString());
		std::string strZipFileName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());
		CString strZipPath = A2T(strPath.c_str());
		CString strName = A2T(strZipFileName.c_str());
//		ZipAdd(hz, A2T(strZipFileName.c_str()), A2T(p.toString().c_str()));
		ZipAdd(hz, strName, strZipPath);
		it++;
	}
	CloseZip(hz);

	return true;
}

bool UnZipFile(CString strZipPath)
{
	USES_CONVERSION;
	CString strPath = strZipPath.Left(strZipPath.GetLength() - 4);		//.zip
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strPath));

	Poco::File p(strUtf8Path);	//T2A(strPath)
	if (p.exists())
		p.remove(true);

	HZIP hz = OpenZip(strZipPath, 0);
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	SetUnzipBaseDir(hz, strPath);
	for (int i = 0; i < numitems; i++)
	{
		GetZipItem(hz, i, &ze);
		UnzipItem(hz, i, ze.name);
	}
	CloseZip(hz);

	return true;
}

pMODEL LoadModelFile(CString strModelPath)
{
	USES_CONVERSION;
	std::string strJsnModel = T2A(strModelPath + _T("\\model.dat"));

	std::string strJsnData;
	std::ifstream in(strJsnModel);
	if (!in)
		return NULL;

	std::string strJsnLine;
	while (!in.eof())
	{	
		getline(in, strJsnLine);					//不过滤空格
		strJsnData.append(strJsnLine);
	}
// 	while (in >> strJsnLine)					//读入的文件如果有空格，将会去除
// 		strJsnData.append(strJsnLine);

	in.close();

	pMODEL pModel = NULL;
	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strJsnData);		//strJsnData
		Poco::JSON::Object::Ptr objData = result.extract<Poco::JSON::Object::Ptr>();

		pModel = new MODEL;
		pModel->strModelName	= A2T(CMyCodeConvert::Utf8ToGb2312(objData->get("modelName").convert<std::string>()).c_str());
		pModel->nPicNum			= objData->get("paperModelCount").convert<int>();
		pModel->nEnableModify	= objData->get("enableModify").convert<int>();
		pModel->nABModel		= objData->get("abPaper").convert<int>();
		pModel->nHasHead		= objData->get("hasHead").convert<int>();

		Poco::JSON::Array::Ptr arrayPapers = objData->getArray("paperInfo");
		for (int i = 0; i < arrayPapers->size(); i++)
		{
			Poco::JSON::Object::Ptr jsnPaperObj = arrayPapers->getObject(i);
			PAPERMODEL paperModelInfo;
			paperModelInfo.nPaper = jsnPaperObj->get("paperNum").convert<int>();
			paperModelInfo.strModelPicName = A2T(CMyCodeConvert::Utf8ToGb2312(jsnPaperObj->get("modelPicName").convert<std::string>()).c_str());

			Poco::JSON::Array::Ptr arraySelHTracker = jsnPaperObj->getArray("hTrackerRect");
			Poco::JSON::Array::Ptr arraySelVTracker = jsnPaperObj->getArray("vTrackerRect");
			Poco::JSON::Array::Ptr arraySelFixRoi	= jsnPaperObj->getArray("selRoiRect");
			Poco::JSON::Array::Ptr arrayOmr		= jsnPaperObj->getArray("selOmrRect");
			Poco::JSON::Array::Ptr arrayFixCP	= jsnPaperObj->getArray("FixCP");
			Poco::JSON::Array::Ptr arrayHHead	= jsnPaperObj->getArray("H_Head");
			Poco::JSON::Array::Ptr arrayVHead	= jsnPaperObj->getArray("V_Head");
			Poco::JSON::Array::Ptr arrayABModel = jsnPaperObj->getArray("ABModel");
			Poco::JSON::Array::Ptr arrayCourse	= jsnPaperObj->getArray("Course");
			Poco::JSON::Array::Ptr arrayQKCP	= jsnPaperObj->getArray("QKCP");
			Poco::JSON::Array::Ptr arrayGrayCP	= jsnPaperObj->getArray("GrayCP");
			Poco::JSON::Array::Ptr arrayWhiteCP = jsnPaperObj->getArray("WhiteCP");
			for (int i = 0; i < arrayFixCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayFixCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lFix.push_back(rc);
			}
			for (int i = 0; i < arrayHHead->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayHHead->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lH_Head.push_back(rc);
			}
			for (int i = 0; i < arrayVHead->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayVHead->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lV_Head.push_back(rc);
			}
			for (int i = 0; i < arrayABModel->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayABModel->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem		= jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem		= jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lABModel.push_back(rc);
			}
			for (int i = 0; i < arrayCourse->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayCourse->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem		= jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem		= jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lCourse.push_back(rc);
			}
			for (int i = 0; i < arrayQKCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayQKCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem		= jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem		= jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lQK_CP.push_back(rc);
			}
			for (int i = 0; i < arrayGrayCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayGrayCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem		= jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem		= jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lGray.push_back(rc);
			}
			for (int i = 0; i < arrayWhiteCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayWhiteCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem		= jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem		= jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lWhite.push_back(rc);
			}
			for (int i = 0; i < arraySelFixRoi->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arraySelFixRoi->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
//				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lSelFixRoi.push_back(rc);
			}
			for (int i = 0; i < arraySelHTracker->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arraySelHTracker->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				//				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lSelHTracker.push_back(rc);
			}
			for (int i = 0; i < arraySelVTracker->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arraySelVTracker->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				//				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x			= jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y			= jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width		= jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height	= jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo.lSelVTracker.push_back(rc);
			}
			for (int i = 0; i < arrayOmr->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayOmr->getObject(i);
				OMR_QUESTION objOmr;
				objOmr.nTH = jsnRectInfoObj->get("nTH").convert<int>();
				Poco::JSON::Array::Ptr omrList = jsnRectInfoObj->getArray("omrlist");
				int n = omrList->size();
				for (int j = 0; j < omrList->size(); j++)
				{
					Poco::JSON::Object::Ptr jsnOmrObj = omrList->getObject(j);
					RECTINFO rc;
					rc.eCPType		= (CPType)jsnOmrObj->get("eType").convert<int>();
					rc.fStandardValuePercent = jsnOmrObj->get("standardValPercent").convert<float>();
					rc.fStandardValue		= jsnOmrObj->get("standardVal").convert<float>();
					rc.nThresholdValue		= jsnOmrObj->get("thresholdValue").convert<int>();
					rc.nHItem		= jsnOmrObj->get("hHeadItem").convert<int>();
					rc.nVItem		= jsnOmrObj->get("vHeadItem").convert<int>();
					rc.nTH			= jsnOmrObj->get("nTH").convert<int>();
					rc.nAnswer		= jsnOmrObj->get("nAnswer").convert<int>();
					rc.nSingle		= jsnOmrObj->get("nSingle").convert<int>();
					rc.rt.x			= jsnOmrObj->get("left").convert<int>();
					rc.rt.y			= jsnOmrObj->get("top").convert<int>();
					rc.rt.width		= jsnOmrObj->get("width").convert<int>();
					rc.rt.height	= jsnOmrObj->get("height").convert<int>();
					objOmr.lSelAnswer.push_back(rc);
				}
				paperModelInfo.lOMR2.push_back(objOmr);
			}

			std::vector<PAPERMODEL>::iterator itBegin = pModel->vecPaperModel.begin();
			for (; itBegin != pModel->vecPaperModel.end();)
			{
				if (paperModelInfo.nPaper < (*itBegin).nPaper)
				{
					pModel->vecPaperModel.insert(itBegin, paperModelInfo);
					break;
				}
				else
					itBegin++;
			}
			if (itBegin == pModel->vecPaperModel.end())
				pModel->vecPaperModel.push_back(paperModelInfo);
		}
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		if (pModel)
		{
			delete pModel;
			pModel = NULL;
		}
		std::string strErrInfo;
		strErrInfo.append("加载模板文件解析json失败: ");
		strErrInfo.append(jsone.message());
		g_pLogger->information(strErrInfo);
	}
	catch (Poco::Exception& exc)
	{
		if (pModel)
		{
			delete pModel;
			pModel = NULL;
		}
		std::string strErrInfo;
		strErrInfo.append("加载模板文件解析json失败2: ");
		strErrInfo.append(exc.message());
		g_pLogger->information(strErrInfo);
	}

	return pModel;
}


int GetRectInfoByPoint(cv::Point pt, CPType eType, pPAPERMODEL pPaperModel, RECTINFO*& pRc)
{
	int  nFind = -1;

	switch (eType)
	{
	case UNKNOWN:
	case H_HEAD:
		if (eType == H_HEAD || eType == UNKNOWN)
		{
			RECTLIST::iterator it = pPaperModel->lH_Head.begin();
			for (int i = 0; it != pPaperModel->lH_Head.end(); i++, it++)
			{
				if (it->rt.contains(pt))
				{
					nFind = i;
					pRc = &(*it);
					break;
				}
			}
		}
	case V_HEAD:
		if (eType == V_HEAD || eType == UNKNOWN)
		{
			if (!nFind)
			{
				RECTLIST::iterator it = pPaperModel->lV_Head.begin();
				for (int i = 0; it != pPaperModel->lV_Head.end(); i++, it++)
				{
					if (it->rt.contains(pt))
					{
						nFind = i;
						pRc = &(*it);
						break;
					}
				}
			}
		}
	case ABMODEL:
		if (eType == ABMODEL || eType == UNKNOWN)
		{
			if (!nFind)
			{
				RECTLIST::iterator it = pPaperModel->lABModel.begin();
				for (int i = 0; it != pPaperModel->lABModel.end(); i++, it++)
				{
					if (it->rt.contains(pt))
					{
						nFind = i;
						pRc = &(*it);
						break;
					}
				}
			}
		}
	case COURSE:
		if (eType == COURSE || eType == UNKNOWN)
		{
			if (!nFind)
			{
				RECTLIST::iterator it = pPaperModel->lCourse.begin();
				for (int i = 0; it != pPaperModel->lCourse.end(); i++, it++)
				{
					if (it->rt.contains(pt))
					{
						nFind = i;
						pRc = &(*it);
						break;
					}
				}
			}
		}
	case QK_CP:
		if (eType == QK_CP || eType == UNKNOWN)
		{
			if (!nFind)
			{
				RECTLIST::iterator it = pPaperModel->lQK_CP.begin();
				for (int i = 0; it != pPaperModel->lQK_CP.end(); i++, it++)
				{
					if (it->rt.contains(pt))
					{
						nFind = i;
						pRc = &(*it);
						break;
					}
				}
			}
		}
	case GRAY_CP:
		if (eType == GRAY_CP || eType == UNKNOWN)
		{
			if (!nFind)
			{
				RECTLIST::iterator it = pPaperModel->lGray.begin();
				for (int i = 0; it != pPaperModel->lGray.end(); i++, it++)
				{
					if (it->rt.contains(pt))
					{
						nFind = i;
						pRc = &(*it);
						break;
					}
				}
			}
		}
	case WHITE_CP:
		if (eType == WHITE_CP || eType == UNKNOWN)
		{
			if (!nFind)
			{
				RECTLIST::iterator it = pPaperModel->lWhite.begin();
				for (int i = 0; it != pPaperModel->lWhite.end(); i++, it++)
				{
					if (it->rt.contains(pt))
					{
						nFind = i;
						pRc = &(*it);
						break;
					}
				}
			}
		}
	}

	return nFind;
}

inline cv::Point TriangleCoordinate(cv::Point ptA, cv::Point ptB, cv::Point ptC, cv::Point ptNewA, cv::Point ptNewB)
{
	clock_t start, end;
	start = clock();
	long double c02 = pow((ptB.x - ptA.x), 2) + pow((ptB.y - ptA.y), 2);
	long double b02 = pow((ptC.x - ptA.x), 2) + pow((ptC.y - ptA.y), 2);
	long double a02 = pow((ptC.x - ptB.x), 2) + pow((ptC.y - ptB.y), 2);
	long double c2 = pow((ptNewB.x - ptNewA.x), 2) + pow((ptNewB.y - ptNewA.y), 2);

	long double m = sqrt(c2 / c02);	//新三角形边长与原三角形的比例

	long double a2 = pow(m, 2) * a02;
	long double b2 = pow(m, 2) * b02;
	long double dT1 = 2 * b2 * (a2 + c2) - pow(a2 - c2, 2) - pow(b2, 2);

	long double k_ab;		//原AB直线斜率
	long double dDx;			//原C点垂直于AB的D点
	long double dDy;
	long double dFlag;		//标识原C点位于AB的上方还是下方
	if (ptA.x != ptB.x)
	{
		k_ab = (long double)(ptB.y - ptA.y) / (ptB.x - ptA.x);
		dDx = (ptC.x + ptC.y * k_ab - k_ab * ptA.y + pow(k_ab, 2) * ptA.x) / (pow(k_ab, 2) + 1);
		dDy = k_ab * ((ptC.x + ptC.y * k_ab - k_ab * ptA.y - ptA.x) / (pow(k_ab, 2) + 1)) + ptA.y;
		dFlag = k_ab*(ptC.x - ptA.x) + ptA.y - ptC.y;
	}
	else
	{
		dDx = ptA.x;
		dDy = ptC.y;
		dFlag = ptC.x - ptA.x;
	}


	long double dTmp1 = (ptNewA.x - ptNewB.x) * sqrt(2 * b2 * (a2 + c2) - pow(a2 - c2, 2) - pow(b2, 2)) / (2 * c2);
	long double dTmp2 = (pow(m, 2) * (a02 - b02) * (ptNewB.y - ptNewA.y) - (ptNewA.y + ptNewB.y) * c2) / (2 * c2);

	long double dK1 = (pow(m, 2) * (a02 - b02) + pow(ptNewA.x, 2) - pow(ptNewB.x, 2) + pow(ptNewA.y, 2) - pow(ptNewB.y, 2)) / (2 * (ptNewA.x - ptNewB.x));
	long double dK2 = (long double)(ptNewB.y - ptNewA.y) / (ptNewA.x - ptNewB.x);

	long double dTmp3 = sqrt((pow(dK2, 2) + 1) * pow(m, 2) * a02 - pow(dK2 * ptNewB.y + dK1 - ptNewB.x, 2)) / (pow(dK2, 2) + 1);
	//	double dTmp3 = sqrt((dK2 * dK2 + 1) * m * m * a02 - (dK2 * ptNewB.y + dK1 - ptNewB.x) * (dK2 * ptNewB.y + dK1 - ptNewB.x)) / (dK2 * dK2 + 1);
	long double dTmp4 = (dK1 * dK2 - ptNewB.x * dK2 - ptNewB.y) / (pow(dK2, 2) + 1);
	// 	double dYc1 = dTmp3 - dTmp4;
	// 	double dXc1 = dK1 + dK2 * dYc1;
	// 
	// 	double dYc2 = -dTmp3 - dTmp4;
	// 	double dXc2 = dK1 + dK2 * dYc2;

	long double dYc1 = dTmp1 - dTmp2;
	long double dXc1 = dK1 + dK2 * dYc1;

	long double dYc2 = -dTmp1 - dTmp2;
	long double dXc2 = dK1 + dK2 * dYc2;

	long double k_newAB = (double)(ptNewB.y - ptNewA.y) / (ptNewB.x - ptNewA.x);
	long double dNewFlag = k_newAB*(dXc1 - ptNewA.x) + ptNewA.y - dYc1;
	long double dNewFlag2 = k_newAB*(dXc2 - ptNewA.x) + ptNewA.y - dYc2;
	cv::Point ptNewC;
	if (dFlag >= 0)
	{
		if (dNewFlag >= 0)		//xy坐标要调换，不明白
		{
			ptNewC.x = dXc1 + 0.5;
			ptNewC.y = dYc1 + 0.5;
		}
		else if (dNewFlag < 0)
		{
			ptNewC.x = dXc2 + 0.5;
			ptNewC.y = dYc2 + 0.5;
		}
	}
	else if (dFlag < 0)
	{
		if (dNewFlag >= 0)
		{
			ptNewC.x = dXc2 + 0.5;
			ptNewC.y = dYc2 + 0.5;
		}
		else if (dNewFlag < 0)
		{
			ptNewC.x = dXc1 + 0.5;
			ptNewC.y = dYc1 + 0.5;
		}
	}
	end = clock();
//	TRACE("原C点的垂直点D(%f,%f), 新的C点坐标(%f, %f)或者(%f, %f),确定后为(%d,%d)耗时: %d\n", dDx, dDy, dXc1, dYc1, dXc2, dYc2, ptNewC.x, ptNewC.y, end - start);
	TRACE("新的C点坐标(%f, %f)或者(%f, %f),确定后为(%d,%d)耗时: %d\n", dXc1, dYc1, dXc2, dYc2, ptNewC.x, ptNewC.y, end - start);
	return ptNewC;
}
bool GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW, int nPicH)
{
	if (lModelFix.size() == 1)
	{
		if (lFix.size() < 1)
			return false;
		RECTLIST::iterator it = lFix.begin();
		RECTLIST::iterator itModel = lModelFix.begin();
		RECTINFO rc = *it;
		RECTINFO rcModel = *itModel;
		cv::Point pt, ptModel;
		pt.x = rc.rt.x + rc.rt.width / 2 + 0.5;
		pt.y = rc.rt.y + rc.rt.height / 2 + 0.5;
		ptModel.x = rcModel.rt.x + rcModel.rt.width / 2 + 0.5;
		ptModel.y = rcModel.rt.y + rcModel.rt.height / 2 + 0.5;
		int x = pt.x - ptModel.x;
		int y = pt.y - ptModel.y;
		rt.x = rt.x + x;
		rt.y = rt.y + y;
	}
	if (lModelFix.size() == 2)
	{
		if (lFix.size() < 2)
			return false;
		RECTLIST::iterator it = lFix.begin();
		RECTINFO rcA = *it++;
		RECTINFO rcB = *it;
		RECTLIST::iterator itModel = lModelFix.begin();
		RECTINFO rcModelA = *itModel++;
		RECTINFO rcModelB = *itModel;

		cv::Point ptA, ptB, ptC, ptA0, ptB0, ptC0;
#if 1
		if (nPicW != 0 && nPicH != 0)
		{
			int nCenterX = nPicW / 2 + 0.5;
			int nCenterY = nPicH / 2 + 0.5;
			if (rcModelA.rt.x < nCenterX)
			{
				if (rcModelA.rt.y < nCenterY)
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height;
					ptA.x = rcA.rt.x + rcA.rt.width;
					ptA.y = rcA.rt.y + rcA.rt.height;
				}
				else
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width;
					ptA0.y = rcModelA.rt.y;
					ptA.x = rcA.rt.x + rcA.rt.width;
					ptA.y = rcA.rt.y;
				}
			}
			else
			{
				if (rcModelA.rt.y < nCenterY)
				{
					ptA0.x = rcModelA.rt.x;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height;
					ptA.x = rcA.rt.x;
					ptA.y = rcA.rt.y + rcA.rt.height;
				}
				else
				{
					ptA0.x = rcModelA.rt.x;
					ptA0.y = rcModelA.rt.y;
					ptA.x = rcA.rt.x;
					ptA.y = rcA.rt.y;
				}
			}

			if (rcModelB.rt.x < nCenterX)
			{
				if (rcModelB.rt.y < nCenterY)
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height;
					ptB.x = rcB.rt.x + rcB.rt.width;
					ptB.y = rcB.rt.y + rcB.rt.height;
				}
				else
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width;
					ptB0.y = rcModelB.rt.y;
					ptB.x = rcB.rt.x + rcB.rt.width;
					ptB.y = rcB.rt.y;
				}
			}
			else
			{
				if (rcModelB.rt.y < nCenterY)
				{
					ptB0.x = rcModelB.rt.x;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height;
					ptB.x = rcB.rt.x;
					ptB.y = rcB.rt.y + rcB.rt.height;
				}
				else
				{
					ptB0.x = rcModelB.rt.x;
					ptB0.y = rcModelB.rt.y;
					ptB.x = rcB.rt.x;
					ptB.y = rcB.rt.y;
				}
			}
		}
		else
		{
			ptA0.x = rcModelA.rt.x + rcModelA.rt.width;
			ptA0.y = rcModelA.rt.y + rcModelA.rt.height;
			ptB0.x = rcModelB.rt.x + rcModelB.rt.width;
			ptB0.y = rcModelB.rt.y + rcModelB.rt.height;

			ptA.x = rcA.rt.x + rcA.rt.width;
			ptA.y = rcA.rt.y + rcA.rt.height;
			ptB.x = rcB.rt.x + rcB.rt.width;
			ptB.y = rcB.rt.y + rcB.rt.height;
		}
		ptC0.x = rt.x;
		ptC0.y = rt.y;

#else
		ptA0.x = rcModelA.rt.x + rcModelA.rt.width / 2 + 0.5;
		ptA0.y = rcModelA.rt.y + rcModelA.rt.height / 2 + 0.5;
		ptB0.x = rcModelB.rt.x + rcModelB.rt.width / 2 + 0.5;
		ptB0.y = rcModelB.rt.y + rcModelB.rt.height / 2 + 0.5;
		ptC0.x = rt.x;
		ptC0.y = rt.y;

		ptA.x = rcA.rt.x + rcA.rt.width / 2 + 0.5;
		ptA.y = rcA.rt.y + rcA.rt.height / 2 + 0.5;
		ptB.x = rcB.rt.x + rcB.rt.width / 2 + 0.5;
		ptB.y = rcB.rt.y + rcB.rt.height / 2 + 0.5;
#endif
		ptC = TriangleCoordinate(ptA0, ptB0, ptC0, ptA, ptB);
		rt.x = ptC.x;
		rt.y = ptC.y;
		TRACE("定点1(%d, %d), 定点2(%d, %d),新的C点(%d, %d), C点(%d, %d), 原定点1(%d, %d), 定点2(%d, %d)\n", ptA.x, ptA.y, ptB.x, ptB.y, ptC.x, ptC.y, ptC0.x, ptC0.y, ptA0.x, ptA0.y, ptB0.x, ptB0.y);
	}
	return true;
}

std::string calcFileMd5(std::string strPath)
{
	std::string strResult;
	try
	{
		Poco::MD5Engine md5;
		Poco::DigestOutputStream dos(md5);

		std::ifstream istr(strPath, std::ios::binary);
		if (!istr)
		{
			std::string strLog = "calc MD5 failed 1: ";
			strLog.append(strPath);
			g_pLogger->information(strLog);
			std::cout << strLog << std::endl;
			return strResult;
		}
		Poco::StreamCopier::copyStream(istr, dos);
		dos.close();

		strResult = Poco::DigestEngine::digestToHex(md5.digest());

	}
	catch (...)
	{
		std::string strLog = "calc MD5 failed 2: ";
		strLog.append(strPath);
		g_pLogger->information(strLog);
		std::cout << strLog << std::endl;
	}
	return strResult;
}
