#include "stdafx.h"
#include "global.h"

const char* pPwd = NULL;
static char s_szZipPwd[20] = "static";

//ɨ����
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

	if (abs(rt1.y - rt2.y) > 6)
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

int WriteRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char * regValue)
{
	USES_CONVERSION;
	char strTemp[_MAX_PATH];
	HKEY hKey;
	sprintf(strTemp, subDir);

	DWORD nbf = 0;

	long ret = RegCreateKeyEx(root, A2T(strTemp), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &nbf);
	if (ret != ERROR_SUCCESS)
	{
		return -1;
	}
	int n = _tcslen(A2T(regValue));
	ret = RegSetValueEx(hKey, A2T(regKey), 0, regType, LPBYTE(A2T(regValue)), (_tcslen(A2T(regValue)) + 1) * sizeof(TCHAR));	//REG_SZ
	if (ret != ERROR_SUCCESS) 
	{
		return -1;
	}
	RegCloseKey(hKey);
	return 0;
}

int ReadRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char* & regValue)
{
	char strTemp[_MAX_PATH];
	HKEY hKey;
	sprintf(strTemp, subDir);

	long ret = (::RegOpenKeyExA(root, strTemp, 0, KEY_READ, &hKey));
	if (ret != ERROR_SUCCESS)
	{
		return -1;
	}

	DWORD valueType = regType;	//REG_SZ;
	DWORD cbData = 255;

	ret = ::RegQueryValueExA(hKey, regKey, NULL, &valueType, (LPBYTE)regValue, &cbData);
	if (ret != ERROR_SUCCESS)
	{
		return -1;
	}
	RegCloseKey(hKey);

	return 0;
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

#ifdef USES_PWD_ZIP_UNZIP
	pPwd = s_szZipPwd;
#endif

	HZIP hz = CreateZip(zipName, pPwd);
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

#ifdef USES_PWD_ZIP_UNZIP
	pPwd = s_szZipPwd;
#endif

	HZIP hz = OpenZip(strZipPath, pPwd);
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
		getline(in, strJsnLine);					//�����˿ո�
		strJsnData.append(strJsnLine);
	}
// 	while (in >> strJsnLine)					//������ļ�����пո񣬽���ȥ��
// 		strJsnData.append(strJsnLine);

	in.close();

	std::string strFileData;
#ifdef USES_FILE_ENC
	if (!decString(strJsnData, strFileData))
		strFileData = strJsnData;
#else
	strFileData = strJsnData;
#endif

	pMODEL pModel = NULL;
	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strFileData);		//strJsnData
		Poco::JSON::Object::Ptr objData = result.extract<Poco::JSON::Object::Ptr>();

		pModel = new MODEL;
		pModel->strModelName	= A2T(CMyCodeConvert::Utf8ToGb2312(objData->get("modelName").convert<std::string>()).c_str());
		pModel->strModelDesc	= A2T(CMyCodeConvert::Utf8ToGb2312(objData->get("modelDesc").convert<std::string>()).c_str());
		if (objData->has("modelType"))
			pModel->nType		= objData->get("modelType").convert<int>();
		else
			pModel->nType		= 0;
		pModel->nSaveMode		= objData->get("modeSaveMode").convert<int>();
		pModel->nPicNum			= objData->get("paperModelCount").convert<int>();
		pModel->nEnableModify	= objData->get("enableModify").convert<int>();
		pModel->nABModel		= objData->get("abPaper").convert<int>();
		pModel->nHasHead		= objData->get("hasHead").convert<int>();
		if (objData->has("nExamId"))
			pModel->nExamID			= objData->get("nExamId").convert<int>();
		if (objData->has("nSubjectId"))
			pModel->nSubjectID		= objData->get("nSubjectId").convert<int>();

		Poco::JSON::Array::Ptr arrayPapers = objData->getArray("paperInfo");
		for (int i = 0; i < arrayPapers->size(); i++)
		{
			Poco::JSON::Object::Ptr jsnPaperObj = arrayPapers->getObject(i);

			pPAPERMODEL paperModelInfo = new PAPERMODEL;
			paperModelInfo->nPaper = jsnPaperObj->get("paperNum").convert<int>();
			paperModelInfo->strModelPicName = A2T(CMyCodeConvert::Utf8ToGb2312(jsnPaperObj->get("modelPicName").convert<std::string>()).c_str());

			paperModelInfo->rtHTracker.x = jsnPaperObj->get("rtHTracker.x").convert<int>();
			paperModelInfo->rtHTracker.y = jsnPaperObj->get("rtHTracker.y").convert<int>();
			paperModelInfo->rtHTracker.width = jsnPaperObj->get("rtHTracker.width").convert<int>();
			paperModelInfo->rtHTracker.height = jsnPaperObj->get("rtHTracker.height").convert<int>();
			paperModelInfo->rtVTracker.x = jsnPaperObj->get("rtVTracker.x").convert<int>();
			paperModelInfo->rtVTracker.y = jsnPaperObj->get("rtVTracker.y").convert<int>();
			paperModelInfo->rtVTracker.width = jsnPaperObj->get("rtVTracker.width").convert<int>();
			paperModelInfo->rtVTracker.height = jsnPaperObj->get("rtVTracker.height").convert<int>();
			paperModelInfo->rtSNTracker.x = jsnPaperObj->get("rtSNTracker.x").convert<int>();
			paperModelInfo->rtSNTracker.y = jsnPaperObj->get("rtSNTracker.y").convert<int>();
			paperModelInfo->rtSNTracker.width = jsnPaperObj->get("rtSNTracker.width").convert<int>();
			paperModelInfo->rtSNTracker.height = jsnPaperObj->get("rtSNTracker.height").convert<int>();

			Poco::JSON::Array::Ptr arraySn = jsnPaperObj->getArray("snList");
			Poco::JSON::Array::Ptr arraySelHTracker = jsnPaperObj->getArray("hTrackerRect");
			Poco::JSON::Array::Ptr arraySelVTracker = jsnPaperObj->getArray("vTrackerRect");
			Poco::JSON::Array::Ptr arraySelFixRoi = jsnPaperObj->getArray("selRoiRect");
			Poco::JSON::Array::Ptr arrayOmr = jsnPaperObj->getArray("selOmrRect");
			Poco::JSON::Array::Ptr arrayFixCP = jsnPaperObj->getArray("FixCP");
			Poco::JSON::Array::Ptr arrayHHead = jsnPaperObj->getArray("H_Head");
			Poco::JSON::Array::Ptr arrayVHead = jsnPaperObj->getArray("V_Head");
			Poco::JSON::Array::Ptr arrayABModel = jsnPaperObj->getArray("ABModel");
			Poco::JSON::Array::Ptr arrayCourse = jsnPaperObj->getArray("Course");
			Poco::JSON::Array::Ptr arrayQKCP = jsnPaperObj->getArray("QKCP");
			Poco::JSON::Array::Ptr arrayGrayCP = jsnPaperObj->getArray("GrayCP");
			Poco::JSON::Array::Ptr arrayWhiteCP = jsnPaperObj->getArray("WhiteCP");
			for (int i = 0; i < arrayFixCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayFixCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lFix.push_back(rc);
			}
			for (int i = 0; i < arrayHHead->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayHHead->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lH_Head.push_back(rc);
			}
			for (int i = 0; i < arrayVHead->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayVHead->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lV_Head.push_back(rc);
			}
			for (int i = 0; i < arrayABModel->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayABModel->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lABModel.push_back(rc);
			}
			for (int i = 0; i < arrayCourse->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayCourse->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lCourse.push_back(rc);
			}
			for (int i = 0; i < arrayQKCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayQKCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lQK_CP.push_back(rc);
			}
			for (int i = 0; i < arrayGrayCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayGrayCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lGray.push_back(rc);
			}
			for (int i = 0; i < arrayWhiteCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayWhiteCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lWhite.push_back(rc);
			}
			for (int i = 0; i < arraySelFixRoi->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arraySelFixRoi->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				//				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lSelFixRoi.push_back(rc);
			}
			for (int i = 0; i < arraySelHTracker->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arraySelHTracker->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				//				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lSelHTracker.push_back(rc);
			}
			for (int i = 0; i < arraySelVTracker->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arraySelVTracker->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				//				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();
				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();
				paperModelInfo->lSelVTracker.push_back(rc);
			}
			for (int i = 0; i < arrayOmr->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayOmr->getObject(i);
				OMR_QUESTION objOmr;
				objOmr.nTH = jsnRectInfoObj->get("nTH").convert<int>();
				objOmr.nSingle = jsnRectInfoObj->get("nSingle").convert<int>();
				Poco::JSON::Array::Ptr omrList = jsnRectInfoObj->getArray("omrlist");
				for (int j = 0; j < omrList->size(); j++)
				{
					Poco::JSON::Object::Ptr jsnOmrObj = omrList->getObject(j);
					RECTINFO rc;
					rc.eCPType = (CPType)jsnOmrObj->get("eType").convert<int>();
					rc.fStandardValuePercent = jsnOmrObj->get("standardValPercent").convert<float>();
					rc.fStandardValue = jsnOmrObj->get("standardVal").convert<float>();
					rc.nThresholdValue = jsnOmrObj->get("thresholdValue").convert<int>();
					rc.nHItem = jsnOmrObj->get("hHeadItem").convert<int>();
					rc.nVItem = jsnOmrObj->get("vHeadItem").convert<int>();
					rc.nTH = jsnOmrObj->get("nTH").convert<int>();
					rc.nAnswer = jsnOmrObj->get("nAnswer").convert<int>();
					rc.nSingle = jsnOmrObj->get("nSingle").convert<int>();
					rc.nRecogFlag = jsnOmrObj->get("nOmrRecogFlag").convert<int>();
					rc.rt.x = jsnOmrObj->get("left").convert<int>();
					rc.rt.y = jsnOmrObj->get("top").convert<int>();
					rc.rt.width = jsnOmrObj->get("width").convert<int>();
					rc.rt.height = jsnOmrObj->get("height").convert<int>();
					objOmr.lSelAnswer.push_back(rc);
				}
				paperModelInfo->lOMR2.push_back(objOmr);
			}
			for (int i = 0; i < arraySn->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arraySn->getObject(i);
				pSN_ITEM pSnItem = new SN_ITEM;
				pSnItem->nItem = jsnRectInfoObj->get("nItem").convert<int>();
				pSnItem->nRecogVal = jsnRectInfoObj->get("nRecogVal").convert<int>();
				Poco::JSON::Array::Ptr snList = jsnRectInfoObj->getArray("snList");
				for (int j = 0; j < snList->size(); j++)
				{
					Poco::JSON::Object::Ptr jsnSnObj = snList->getObject(j);
					RECTINFO rc;
					rc.eCPType = (CPType)jsnSnObj->get("eType").convert<int>();
					rc.fStandardValuePercent = jsnSnObj->get("standardValPercent").convert<float>();
					rc.fStandardValue = jsnSnObj->get("standardVal").convert<float>();
					rc.nThresholdValue = jsnSnObj->get("thresholdValue").convert<int>();
					rc.nHItem = jsnSnObj->get("hHeadItem").convert<int>();
					rc.nVItem = jsnSnObj->get("vHeadItem").convert<int>();
					rc.nTH = jsnSnObj->get("nTH").convert<int>();
					rc.nSnVal = jsnSnObj->get("nSnVal").convert<int>();
					rc.nAnswer = jsnSnObj->get("nAnswer").convert<int>();
					rc.nSingle = jsnSnObj->get("nSingle").convert<int>();
					rc.nRecogFlag = jsnSnObj->get("nSnRecogFlag").convert<int>();
					rc.rt.x = jsnSnObj->get("left").convert<int>();
					rc.rt.y = jsnSnObj->get("top").convert<int>();
					rc.rt.width = jsnSnObj->get("width").convert<int>();
					rc.rt.height = jsnSnObj->get("height").convert<int>();
					pSnItem->lSN.push_back(rc);
				}
				paperModelInfo->lSNInfo.push_back(pSnItem);
			}

			std::vector<pPAPERMODEL>::iterator itBegin = pModel->vecPaperModel.begin();
			for (; itBegin != pModel->vecPaperModel.end();)
			{
				if (paperModelInfo->nPaper < (*itBegin)->nPaper)
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
		strErrInfo.append("����ģ���ļ�����jsonʧ��: ");
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
		strErrInfo.append("����ģ���ļ�����jsonʧ��2: ");
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

void GetMaxMin(cv::Point2f ptChk, cv::Point2f ptA, cv::Point2f ptB, cv::Point2f ptC, cv::Point2f ptD, cv::Point2f ptA0, cv::Point2f ptB0, cv::Point2f ptC0, cv::Point2f ptD0, cv::Point2f pt[], cv::Point2f pt2[])
{
	long double da = sqrt(pow(ptChk.x - ptA.x, 2) + pow(ptChk.y - ptA.y, 2));
	long double db = sqrt(pow(ptChk.x - ptB.x, 2) + pow(ptChk.y - ptB.y, 2));
	long double dc = sqrt(pow(ptChk.x - ptC.x, 2) + pow(ptChk.y - ptC.y, 2));
	long double dd = sqrt(pow(ptChk.x - ptD.x, 2) + pow(ptChk.y - ptD.y, 2));

	long double dMin = da, dMax = da;
	cv::Point2f ptMin = ptA, ptMax = ptA;
	
	if (dMax < db){ dMax = db; ptMax = ptB; }
	if (dMax < dc){ dMax = dc; ptMax = ptC; }
	if (dMax < dd){ dMax = dd; ptMax = ptD; }

	if (dMin > db){ dMin = db; ptMin = ptB; }
	if (dMin > dc){ dMin = dc; ptMin = ptC; }
	if (dMin > dd){ dMin = dd; ptMin = ptD; }
	pt[0] = ptMin;
	int i = 1;
	if (ptA != ptMin && ptA != ptMax) { pt[i] = ptA; pt2[i++] = ptA0; };
	if (ptB != ptMin && ptB != ptMax) { pt[i] = ptB; pt2[i++] = ptB0; };
	if (ptC != ptMin && ptC != ptMax) { pt[i] = ptC; pt2[i++] = ptC0; };
	if (ptD != ptMin && ptD != ptMax) { pt[i] = ptD; pt2[i++] = ptD0; };
	if (ptA == ptMin) pt2[0] = ptA0;
	if (ptB == ptMin) pt2[0] = ptB0;
	if (ptC == ptMin) pt2[0] = ptC0;
	if (ptD == ptMin) pt2[0] = ptD0;
}

//���������㷨
inline cv::Point2d TriangleCentroid(cv::Point ptChk, cv::Point2f ptA, cv::Point2f ptB, cv::Point2f ptNewA, cv::Point2f ptNewB)
{
	long double rb2 = pow((ptChk.x - ptB.x), 2) + pow((ptChk.y - ptB.y), 2);
	long double ra2 = pow((ptChk.x - ptA.x), 2) + pow((ptChk.y - ptA.y), 2);

	cv::Point2d ptNewChk;
	long double d4;
	long double d5;
	long double x1;
	long double x2;
	long double y1;
	long double y2;
	if (ptNewB.y == ptNewA.y)
	{
		long double x = (ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) - pow(ptNewA.y, 2) + pow(ptNewB.y, 2)) / (2 * (ptNewB.x - ptNewA.x));
		y1 = ptNewA.y + sqrt(ra2 - pow(x - ptNewA.x, 2));
		y2 = ptNewA.y - sqrt(ra2 - pow(x - ptNewA.x, 2));
		x1 = x;
		x2 = x;

		d4 = pow(ptChk.x - x, 2) + pow(ptChk.y - y1, 2);
		d5 = pow(ptChk.x - x, 2) + pow(ptChk.y - y2, 2);
	}
	else
	{
		long double k = -(ptNewB.x - ptNewA.x) / (ptNewB.y - ptNewA.y);
		long double m = (ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) - pow(ptNewA.y, 2) + pow(ptNewB.y, 2)) / (2 * (ptNewB.y - ptNewA.y));
		long double d2 = (4 * ra2 * (pow(ptNewB.x - ptNewA.x, 2) + pow(ptNewB.y - ptNewA.y, 2)) - pow((ra2 - rb2 + pow(ptNewA.y - ptNewB.y, 2) + pow(ptNewA.x - ptNewB.x, 2)), 2)) / pow(ptNewB.y - ptNewA.y, 2);
		long double d1 = (ptNewB.x * (ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) + pow(ptNewA.y - ptNewB.y, 2)) - ptNewA.x * (ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2)) + ptNewA.x * pow(ptNewA.y - ptNewB.y, 2)) / pow(ptNewB.y - ptNewA.y, 2);
		long double d3 = 2 * (1 + pow(k, 2));
		if (d2 >= 0)
		{
			x1 = (d1 + sqrt(d2)) / d3;
			x2 = (d1 - sqrt(d2)) / d3;
			y1 = k*x1 + m;
			y2 = k*x2 + m;

			d4 = pow(ptChk.x - x1, 2) + pow(ptChk.y - y1, 2);
			d5 = pow(ptChk.x - x2, 2) + pow(ptChk.y - y2, 2);
		}
		else
		{
			return NULL;
			long double x = (ptNewA.x + ptNewB.x) / 2;
			long double y = (ptNewA.y + ptNewB.y) / 2;
			ptNewChk.x = x;
			ptNewChk.y = y;
			return ptNewChk;
		}
	}
	if (d4 <= d5)
	{
		ptNewChk.x = x1;
		ptNewChk.y = y1;
	}
	else
	{
		ptNewChk.x = x2;
		ptNewChk.y = y2;
	}
	return ptNewChk;
}
//���߶�λ�㷨
inline cv::Point2d TriangleSide(cv::Point ptChk, cv::Point2f ptA, cv::Point2f ptB, cv::Point2f ptC, cv::Point2f ptNewA, cv::Point2f ptNewB, cv::Point2f ptNewC)
{
	long double rc2 = pow((ptChk.x - ptC.x), 2) + pow((ptChk.y - ptC.y), 2);
	long double rb2 = pow((ptChk.x - ptB.x), 2) + pow((ptChk.y - ptB.y), 2);
	long double ra2 = pow((ptChk.x - ptA.x), 2) + pow((ptChk.y - ptA.y), 2);

	cv::Point2d ptNewChk;
	if(ptNewB.y == ptNewA.y)
	{
		long double x = (ra2 - rb2 - pow(ptNewA.x,2) + pow(ptNewB.x,2) - pow(ptNewA.y,2) + pow(ptNewB.y,2)) / (2*(ptNewB.x - ptNewA.x));
		long double y = (rb2 - rc2 - pow(ptNewB.x,2) + pow(ptNewC.x,2) - pow(ptNewB.y,2) + pow(ptNewC.y,2) - 2*(ptNewC.x - ptNewB.x)*x) / (2*(ptNewC.y - ptNewB.y));
		ptNewChk.x = x;
		ptNewChk.y = y;
		return ptNewChk;
	}

#if 0
	long double v1 = rb2 - rc2 - pow(ptNewB.x, 2) + pow(ptNewC.x, 2) - pow(ptNewB.y, 2) + pow(ptNewC.y, 2) - ((ptNewC.y - ptNewB.y)*(ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) - pow(ptNewA.y, 2) + pow(ptNewB.y, 2)) / (ptNewB.y - ptNewA.y));
	long double v2 = 2*(ptNewC.x - ptNewB.x) - 2*(ptNewC.y - ptNewB.y)*(ptNewB.x - ptNewA.x)/(ptNewB.y - ptNewA.y);
	long double x = v1 / v2;
	ptNewChk.x = x;
#else
	long double v3 = (ptNewA.y - ptNewB.y)*(rc2 - pow(ptNewC.x, 2) - pow(ptNewC.y, 2)) - (ptNewA.y - ptNewC.y)*(rb2 - pow(ptNewB.x, 2) - pow(ptNewB.y, 2)) + (ptNewB.y - ptNewC.y)*(ra2 - pow(ptNewA.x, 2) - pow(ptNewA.y, 2));
	long double v4 = 2 * ((ptNewC.x - ptNewB.x) * (ptNewB.y - ptNewA.y) - (ptNewC.y - ptNewB.y) * (ptNewB.x - ptNewA.x));
	long double x = v3 / v4;
	ptNewChk.x = x;
#endif
	ptNewChk.y = (ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) - pow(ptNewA.y, 2) + pow(ptNewB.y, 2)) / (2 * (ptNewB.y - ptNewA.y)) - ((ptNewB.x - ptNewA.x) / (ptNewB.y - ptNewA.y)) * x;

	//++check
	long double v1 = ra2 - rc2 - pow(ptNewA.x, 2) + pow(ptNewC.x, 2) - pow(ptNewA.y, 2) + pow(ptNewC.y, 2) - ((ptNewC.y - ptNewA.y)*(ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) - pow(ptNewA.y, 2) + pow(ptNewB.y, 2)) / (ptNewB.y - ptNewA.y));
	long double v2 = 2 * (ptNewC.x - ptNewA.x) - 2 * (ptNewC.y - ptNewA.y)*(ptNewB.x - ptNewA.x) / (ptNewB.y - ptNewA.y);
	long double x1 = v1 / v2;
	long double y1 = (ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) - pow(ptNewA.y, 2) + pow(ptNewB.y, 2)) / (2 * (ptNewB.y - ptNewA.y)) - ((ptNewB.x - ptNewA.x) / (ptNewB.y - ptNewA.y)) * x1;
	TRACE("��1(%f,%f),��2(%f,%f)\n", ptNewChk.x, ptNewChk.y, x1, y1);
	//--

	return ptNewChk;
}

inline cv::Point2d TriangleCoordinate(cv::Point ptA, cv::Point ptB, cv::Point ptC, cv::Point ptNewA, cv::Point ptNewB)
{
	clock_t start, end;
	start = clock();
	long double c02 = pow((ptB.x - ptA.x), 2) + pow((ptB.y - ptA.y), 2);
	long double b02 = pow((ptC.x - ptA.x), 2) + pow((ptC.y - ptA.y), 2);
	long double a02 = pow((ptC.x - ptB.x), 2) + pow((ptC.y - ptB.y), 2);
	long double c2 = pow((ptNewB.x - ptNewA.x), 2) + pow((ptNewB.y - ptNewA.y), 2);

	long double m = sqrt(c2 / c02);	//�������α߳���ԭ�����εı���

	long double a2 = pow(m, 2) * a02;
	long double b2 = pow(m, 2) * b02;
	long double dT1 = 2 * b2 * (a2 + c2) - pow(a2 - c2, 2) - pow(b2, 2);

	long double k_ab;		//ԭABֱ��б��
	long double dDx;			//ԭC�㴹ֱ��AB��D��
	long double dDy;
	long double dFlag;		//��ʶԭC��λ��AB���Ϸ������·�
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
	cv::Point2d ptNewC;
	if (dFlag >= 0)
	{
		if (dNewFlag >= 0)		//xy����Ҫ������������
		{
			ptNewC.x = dXc1;
			ptNewC.y = dYc1;
		}
		else if (dNewFlag < 0)
		{
			ptNewC.x = dXc2;
			ptNewC.y = dYc2;
		}
	}
	else if (dFlag < 0)
	{
		if (dNewFlag >= 0)
		{
			ptNewC.x = dXc2;
			ptNewC.y = dYc2;
		}
		else if (dNewFlag < 0)
		{
			ptNewC.x = dXc1;
			ptNewC.y = dYc1;
		}
	}
	end = clock();
//	TRACE("ԭC��Ĵ�ֱ��D(%f,%f), �µ�C������(%f, %f)����(%f, %f),ȷ����Ϊ(%d,%d)��ʱ: %d\n", dDx, dDy, dXc1, dYc1, dXc2, dYc2, ptNewC.x, ptNewC.y, end - start);
	TRACE("�µ�C������(%f, %f)����(%f, %f),ȷ����Ϊ(%f,%f)��ʱ: %d\n", dXc1, dYc1, dXc2, dYc2, ptNewC.x, ptNewC.y, end - start);
	return ptNewC;
}
bool GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW /*= 0*/, int nPicH /*= 0*/)
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

		cv::Point ptA, ptB, ptA0, ptB0, ptC0;
		cv::Point2d ptC;
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

		//���µĵ�Ҳ����
// 		cv::Point ptC1;
// 		ptC1.x = ptC0.x + rt.width;
// 		ptC1.y = ptC0.y + rt.height;
// 		ptC = TriangleCoordinate(ptA0, ptB0, ptC1, ptA, ptB);
// 		rt.width = ptC1.x - rt.x;
// 		rt.height = ptC1.y - rt.y;

//		TRACE("����1(%d, %d), ����2(%d, %d),�µ�C��(%d, %d), C��(%d, %d), ԭ����1(%d, %d), ����2(%d, %d)\n", ptA.x, ptA.y, ptB.x, ptB.y, ptC.x, ptC.y, ptC0.x, ptC0.y, ptA0.x, ptA0.y, ptB0.x, ptB0.y);
	}
	else if (lModelFix.size() == 3)
	{
		if (lFix.size() < 3)
			return false;
#ifdef WarpAffine_TEST
		return true;
#else
		RECTLIST::iterator it = lFix.begin();
		RECTINFO rcA = *it++;
		RECTINFO rcB = *it++;
		RECTINFO rcC = *it;
		RECTLIST::iterator itModel = lModelFix.begin();
		RECTINFO rcModelA = *itModel++;
		RECTINFO rcModelB = *itModel++;
		RECTINFO rcModelC = *itModel;

		cv::Point2f ptA, ptB, ptC, ptA0, ptB0, ptC0;
		cv::Point2f ptChk;

		ptA0.x = rcModelA.rt.x + (double)rcModelA.rt.width / 2;
		ptA0.y = rcModelA.rt.y + (double)rcModelA.rt.height / 2;
		ptB0.x = rcModelB.rt.x + (double)rcModelB.rt.width / 2;
		ptB0.y = rcModelB.rt.y + (double)rcModelB.rt.height / 2;
		ptC0.x = rcModelC.rt.x + (double)rcModelC.rt.width / 2;
		ptC0.y = rcModelC.rt.y + (double)rcModelC.rt.height / 2;

		ptA.x = rcA.rt.x + (double)rcA.rt.width / 2;
		ptA.y = rcA.rt.y + (double)rcA.rt.height / 2;
		ptB.x = rcB.rt.x + (double)rcB.rt.width / 2;
		ptB.y = rcB.rt.y + (double)rcB.rt.height / 2;
		ptC.x = rcC.rt.x + (double)rcC.rt.width / 2;
		ptC.y = rcC.rt.y + (double)rcC.rt.height / 2;

		ptChk.x = rt.x;
		ptChk.y = rt.y;
		cv::Point2d ptResult;
	#ifdef TriangleSide_TEST
		ptResult = TriangleSide(ptChk, ptA0, ptB0, ptC0, ptA, ptB, ptC);
	#else
		long double dMax_X = 0, dMax_Y = 0, dMin_X = 0, dMin_Y = 0;
		long double dSumX = 0;
		long double dSumY = 0;
		long double v1 = 0, v2 = 0, v3 = 0;
		int nCount = 0;
		cv::Point2d ptResult1 = TriangleCentroid(ptChk, ptA0, ptB0, ptA, ptB);
		if (ptResult1.x != 0 && ptResult1.y != 0)
		{
			dSumX += ptResult1.x;
			dSumY += ptResult1.y;
			nCount++;

			dMin_X = ptResult1.x;
			dMin_Y = ptResult1.y;
			if (dMax_X < ptResult1.x) dMax_X = ptResult1.x;
			if (dMax_Y < ptResult1.y) dMax_Y = ptResult1.y;
			if (dMin_X > ptResult1.x) dMin_X = ptResult1.x;
			if (dMin_Y > ptResult1.y) dMin_Y = ptResult1.y;

			v1 = 1;
		}
		cv::Point2d ptResult2 = TriangleCentroid(ptChk, ptA0, ptC0, ptA, ptC);
		if (ptResult2.x != 0 && ptResult2.y != 0)
		{
			dSumX += ptResult2.x;
			dSumY += ptResult2.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult2.x;
			if (dMin_Y == 0) dMin_Y = ptResult2.y;
			if (dMax_X < ptResult2.x) dMax_X = ptResult2.x;
			if (dMax_Y < ptResult2.y) dMax_Y = ptResult2.y;
			if (dMin_X > ptResult2.x) dMin_X = ptResult2.x;
			if (dMin_Y > ptResult2.y) dMin_Y = ptResult2.y;

			v2 = 1;
		}
		cv::Point2d ptResult3 = TriangleCentroid(ptChk, ptB0, ptC0, ptB, ptC);
		if (ptResult3.x != 0 && ptResult3.y != 0)
		{
			dSumX += ptResult3.x;
			dSumY += ptResult3.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult3.x;
			if (dMin_Y == 0) dMin_Y = ptResult3.y;
			if (dMax_X < ptResult3.x) dMax_X = ptResult3.x;
			if (dMax_Y < ptResult3.y) dMax_Y = ptResult3.y;
			if (dMin_X > ptResult3.x) dMin_X = ptResult3.x;
			if (dMin_Y > ptResult3.y) dMin_Y = ptResult3.y;

			v3 = 1;
		}

		long double da = sqrt(pow(ptChk.x - ptA.x, 2) + pow(ptChk.y - ptA.y, 2));
		long double db = sqrt(pow(ptChk.x - ptB.x, 2) + pow(ptChk.y - ptB.y, 2));
		long double dc = sqrt(pow(ptChk.x - ptC.x, 2) + pow(ptChk.y - ptC.y, 2));

		long double x1 = (ptResult1.x / (da + db) + ptResult2.x / (da + dc) + ptResult3.x / (db + dc)) / (v1 / (da + db) + v2 / (da + dc) + v3 / (db + dc));
		long double y1 = (ptResult1.y / (da + db) + ptResult2.y / (da + dc) + ptResult3.y / (db + dc)) / (v1 / (da + db) + v2 / (da + dc) + v3 / (db + dc));
		long double x = (ptResult1.x * (v1/da + v2/db) + ptResult2.x * (v1/da + v3/dc) + ptResult3.x * (v2/db + v3/dc)) / (2*(v1/da + v2/db + v3/dc));
		long double y = (ptResult1.y * (v1 / da + v2 / db) + ptResult2.y * (v1 / da + v3 / dc) + ptResult3.y * (v2 / db + v3 / dc)) / (2 * (v1 / da + v2 / db + v3 / dc));

		if (nCount > 0)
		{
// 			ptResult.x = dSumX / nCount;
// 			ptResult.y = dSumY / nCount;
			ptResult.x = x;
			ptResult.y = y;
		}
		else
		{
			std::string strLog = "���ļ���ʧ�ܣ�û�н���";
		}
		TRACE("���������㷨: chk(%f,%f),ptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f),��������(%f,%f)���õ�(%f,%f)\n", ptChk.x, ptChk.y,\
			  ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult.x, ptResult.y, x1, y1);
	#endif
		rt.x = ptResult.x;
		rt.y = ptResult.y;
#endif
	}
	else if (lModelFix.size() == 4)
	{
		if (lFix.size() < 4)
			return false;
#ifdef WarpAffine_TEST
		return true;
#else
		RECTLIST::iterator it = lFix.begin();
		RECTINFO rcA = *it++;
		RECTINFO rcB = *it++;
		RECTINFO rcC = *it++;
		RECTINFO rcD = *it;
		RECTLIST::iterator itModel = lModelFix.begin();
		RECTINFO rcModelA = *itModel++;
		RECTINFO rcModelB = *itModel++;
		RECTINFO rcModelC = *itModel++;
		RECTINFO rcModelD = *itModel;

		cv::Point2f ptA, ptB, ptC, ptD, ptA0, ptB0, ptC0, ptD0;
		cv::Point2f ptChk;

		ptA0.x = rcModelA.rt.x + (double)rcModelA.rt.width / 2;
		ptA0.y = rcModelA.rt.y + (double)rcModelA.rt.height / 2;
		ptB0.x = rcModelB.rt.x + (double)rcModelB.rt.width / 2;
		ptB0.y = rcModelB.rt.y + (double)rcModelB.rt.height / 2;
		ptC0.x = rcModelC.rt.x + (double)rcModelC.rt.width / 2;
		ptC0.y = rcModelC.rt.y + (double)rcModelC.rt.height / 2;
		ptD0.x = rcModelD.rt.x + (double)rcModelD.rt.width / 2;
		ptD0.y = rcModelD.rt.y + (double)rcModelD.rt.height / 2;

		ptA.x = rcA.rt.x + (double)rcA.rt.width / 2;
		ptA.y = rcA.rt.y + (double)rcA.rt.height / 2;
		ptB.x = rcB.rt.x + (double)rcB.rt.width / 2;
		ptB.y = rcB.rt.y + (double)rcB.rt.height / 2;
		ptC.x = rcC.rt.x + (double)rcC.rt.width / 2;
		ptC.y = rcC.rt.y + (double)rcC.rt.height / 2;
		ptD.x = rcD.rt.x + (double)rcD.rt.width / 2;
		ptD.y = rcD.rt.y + (double)rcD.rt.height / 2;

		ptChk.x = rt.x;
		ptChk.y = rt.y;
		cv::Point2d ptResult;
#ifdef TriangleSide_TEST
		long double dSumX = 0;
		long double dSumY = 0;
		int nCount = 0;
//		ptResult = TriangleSide(ptChk, ptA0, ptB0, ptC0, ptA, ptB, ptC);
		cv::Point2d ptResult1 = TriangleSide(ptChk, ptA0, ptB0, ptC0, ptA, ptB, ptC);
		if (ptResult1.x != 0 && ptResult1.y != 0)
		{
			dSumX += ptResult1.x;
			dSumY += ptResult1.y;
			nCount++;
		}
		cv::Point2d ptResult2 = TriangleSide(ptChk, ptA0, ptB0, ptD0, ptA, ptB, ptD);
		if (ptResult2.x != 0 && ptResult2.y != 0)
		{
			dSumX += ptResult2.x;
			dSumY += ptResult2.y;
			nCount++;
		}
		cv::Point2d ptResult3 = TriangleSide(ptChk, ptB0, ptC0, ptD0, ptB, ptC, ptD);
		if (ptResult3.x != 0 && ptResult3.y != 0)
		{
			dSumX += ptResult3.x;
			dSumY += ptResult3.y;
			nCount++;
		}
		cv::Point2d ptResult4 = TriangleSide(ptChk, ptA0, ptC0, ptD0, ptA, ptC, ptD);
		if (ptResult4.x != 0 && ptResult4.y != 0)
		{
			dSumX += ptResult4.x;
			dSumY += ptResult4.y;
			nCount++;
		}
#if 0
		double d1 = abs(ptResult1.x - ptResult2.x);
		double d2 = abs(ptResult1.x - ptResult3.x);
		double d3 = abs(ptResult2.x - ptResult3.x);
		if (d1 < d2)
		{
			if (d1 < d3){ ptResult.x = (ptResult1.x + ptResult2.x) / 2; }
			else if (d1 > d3) { ptResult.x = (ptResult3.x + ptResult2.x) / 2; }
			else { ptResult.x = (ptResult1.x + ptResult2.x + ptResult3.x) / 3; }
		}
		else if (d1 > d2)
		{
			if (d2 < d3){ ptResult.x = (ptResult1.x + ptResult3.x) / 2; }
			else if (d2 > d3) { ptResult.x = (ptResult3.x + ptResult2.x) / 2; }
			else { ptResult.x = (ptResult1.x + ptResult2.x + ptResult3.x) / 3; }
		}
		else
		{
			ptResult.x = (ptResult1.x + ptResult2.x + ptResult3.x) / 3;
		}
		double d4 = abs(ptResult1.y - ptResult2.y);
		double d5 = abs(ptResult1.y - ptResult3.y);
		double d6 = abs(ptResult2.y - ptResult3.y);
		if (d4 < d5)
		{
			if (d4 < d6){ ptResult.y = (ptResult1.y + ptResult2.y) / 2; }
			else if (d4 > d6) { ptResult.y = (ptResult3.y + ptResult2.y) / 2; }
			else { ptResult.y = (ptResult1.y + ptResult2.y + ptResult3.y) / 3; }
		}
		else if (d4 > d5)
		{
			if (d5 < d6){ ptResult.y = (ptResult1.y + ptResult3.y) / 2; }
			else if (d5 > d6) { ptResult.y = (ptResult3.y + ptResult2.y) / 2; }
			else { ptResult.y = (ptResult1.y + ptResult2.y + ptResult3.y) / 3; }
		}
		else
		{
			ptResult.y = (ptResult1.y + ptResult2.y + ptResult3.y) / 3;
		}
#else
		if (nCount > 0)
		{
			ptResult.x = dSumX / nCount;
			ptResult.y = dSumY / nCount;
		}
		else
		{
			std::string strLog = "���ļ���ʧ�ܣ�û�н���";
		}
#endif
		TRACE("���߶�λ�㷨: ptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f), ptResult4(%f,%f)��������(%f,%f)\n", \
			  ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult4.x, ptResult4.y, ptResult.x, ptResult.y);
#else
		long double dMax_X = 0, dMax_Y = 0, dMin_X = 0, dMin_Y = 0;
		long double dSumX = 0;
		long double dSumY = 0;
		int nCount = 0;
		cv::Point2d ptResult1 = TriangleCentroid(ptChk, ptA0, ptB0, ptA, ptB);
		if (ptResult1.x > 0 && ptResult1.y > 0)
		{
			dSumX += ptResult1.x;
			dSumY += ptResult1.y;
			nCount++;

			dMin_X = ptResult1.x;
			dMin_Y = ptResult1.y;
			if (dMax_X < ptResult1.x) dMax_X = ptResult1.x;
			if (dMax_Y < ptResult1.y) dMax_Y = ptResult1.y;
			if (dMin_X > ptResult1.x) dMin_X = ptResult1.x;
			if (dMin_Y > ptResult1.y) dMin_Y = ptResult1.y;
		}
		
		cv::Point2d ptResult2 = TriangleCentroid(ptChk, ptA0, ptC0, ptA, ptC);
		if (ptResult2.x > 0 && ptResult2.y > 0)
		{
			dSumX += ptResult2.x;
			dSumY += ptResult2.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult2.x;
			if (dMin_Y == 0) dMin_Y = ptResult2.y;
			if (dMax_X < ptResult2.x) dMax_X = ptResult2.x;
			if (dMax_Y < ptResult2.y) dMax_Y = ptResult2.y;
			if (dMin_X > ptResult2.x) dMin_X = ptResult2.x;
			if (dMin_Y > ptResult2.y) dMin_Y = ptResult2.y;
		}
		
		cv::Point2d ptResult3 = TriangleCentroid(ptChk, ptB0, ptC0, ptB, ptC);
		if (ptResult3.x > 0 && ptResult3.y > 0)
		{
			dSumX += ptResult3.x;
			dSumY += ptResult3.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult3.x;
			if (dMin_Y == 0) dMin_Y = ptResult3.y;
			if (dMax_X < ptResult3.x) dMax_X = ptResult3.x;
			if (dMax_Y < ptResult3.y) dMax_Y = ptResult3.y;
			if (dMin_X > ptResult3.x) dMin_X = ptResult3.x;
			if (dMin_Y > ptResult3.y) dMin_Y = ptResult3.y;
		}
		
		cv::Point2d ptResult4 = TriangleCentroid(ptChk, ptA0, ptD0, ptA, ptD);
		if (ptResult4.x > 0 && ptResult4.y > 0)
		{
			dSumX += ptResult4.x;
			dSumY += ptResult4.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult4.x;
			if (dMin_Y == 0) dMin_Y = ptResult4.y;
			if (dMax_X < ptResult4.x) dMax_X = ptResult4.x;
			if (dMax_Y < ptResult4.y) dMax_Y = ptResult4.y;
			if (dMin_X > ptResult4.x) dMin_X = ptResult4.x;
			if (dMin_Y > ptResult4.y) dMin_Y = ptResult4.y;
		}
		
		cv::Point2d ptResult5 = TriangleCentroid(ptChk, ptB0, ptD0, ptB, ptD);
		if (ptResult5.x > 0 && ptResult5.y > 0)
		{
			dSumX += ptResult5.x;
			dSumY += ptResult5.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult5.x;
			if (dMin_Y == 0) dMin_Y = ptResult5.y;
			if (dMax_X < ptResult5.x) dMax_X = ptResult5.x;
			if (dMax_Y < ptResult5.y) dMax_Y = ptResult5.y;
			if (dMin_X > ptResult5.x) dMin_X = ptResult5.x;
			if (dMin_Y > ptResult5.y) dMin_Y = ptResult5.y;
		}
		
		cv::Point2d ptResult6 = TriangleCentroid(ptChk, ptC0, ptD0, ptC, ptD);
		if (ptResult6.x != 0 && ptResult6.y != 0)
		{
			dSumX += ptResult6.x;
			dSumY += ptResult6.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult6.x;
			if (dMin_Y == 0) dMin_Y = ptResult6.y;
			if (dMax_X < ptResult6.x) dMax_X = ptResult6.x;
			if (dMax_Y < ptResult6.y) dMax_Y = ptResult6.y;
			if (dMin_X > ptResult6.x) dMin_X = ptResult6.x;
			if (dMin_Y > ptResult6.y) dMin_Y = ptResult6.y;
		}
		
		dSumX -= dMax_X;
		dSumX -= dMin_X;
		dSumY -= dMax_Y;
		dSumY -= dMin_Y;
		nCount -= 2;

		long double da = sqrt(pow(ptChk.x - ptA.x, 2) + pow(ptChk.y - ptA.y, 2));
		long double db = sqrt(pow(ptChk.x - ptB.x, 2) + pow(ptChk.y - ptB.y, 2));
		long double dc = sqrt(pow(ptChk.x - ptC.x, 2) + pow(ptChk.y - ptC.y, 2));
		long double dd = sqrt(pow(ptChk.x - ptD.x, 2) + pow(ptChk.y - ptD.y, 2));
#if 0	//���������3����������3�����У�ȡ����������2��ֱ�ߣ���ABC�������3���㣬����A�����������AB��AC����ĵ�
		cv::Point2f pt[3];
		cv::Point2f pt2[3];
		GetMaxMin(ptChk, ptA, ptB, ptC, ptD, ptA0, ptB0, ptC0, ptD0, pt, pt2);

		long double dSx = 0;
		long double dSy = 0;
		int nCount2 = 0;
		cv::Point2d ptResultT1 = TriangleCentroid(ptChk, pt2[0], pt2[1], pt[0], pt[1]);
		if (ptResultT1.x > 0 && ptResultT1.y > 0)
		{
			dSx += ptResultT1.x;
			dSy += ptResultT1.y;
			nCount2++;
		}
		cv::Point2d ptResultT2 = TriangleCentroid(ptChk, pt2[0], pt2[2], pt[0], pt[2]);
		if (ptResultT2.x > 0 && ptResultT2.y > 0)
		{
			dSx += ptResultT2.x;
			dSy += ptResultT2.y;
			nCount2++;
		}
		ptResult.x = dSx / nCount2;
		ptResult.y = dSy / nCount2;
		TRACE("�ο�(%f,%f), ptResultT1((%f,%f), ptResultT1((%f,%f)\n", dSumX / nCount, dSumY / nCount, ptResultT1.x, ptResultT1.y, ptResultT2.x, ptResultT2.y);
#endif


		if (nCount > 0)
		{
			ptResult.x = dSumX / nCount;
			ptResult.y = dSumY / nCount;
		}
		else
		{
			std::string strLog = "���ļ���ʧ�ܣ�û�н���";
		}
		TRACE("���������㷨: chk(%f,%f), da=%f,db=%f,dc=%f,dd=%f,\nptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f),ptResult4(%f,%f),ptResult5(%f,%f),ptResult6(%f,%f),��������(%f,%f)\n", ptChk.x, ptChk.y,\
			  da,db,dc,dd,ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult4.x, ptResult4.y, ptResult5.x, ptResult5.y, ptResult6.x, ptResult6.y, ptResult.x, ptResult.y);
#endif
		rt.x = ptResult.x;
		rt.y = ptResult.y;
#endif
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

bool PicRectify(cv::Mat& src, cv::Mat& dst, cv::Mat& rotMat)
{
	clock_t start, end;
	start = clock();
	char szTmpLog[200] = { 0 };

	cv::Rect rt;
#if 0
	rt.x = src.cols * 0.05;
	rt.y = src.rows * 0.05;
	rt.width = src.cols * 0.9;
	rt.height = src.rows / 4;
#else
	rt.width = src.cols;
	rt.height = src.rows * 0.4;		//4
#endif

	cv::Mat matSrc = src(rt);
	cv::Mat hsv;
	cv::cvtColor(matSrc, hsv, CV_BGR2GRAY);
	cv::Mat srcImg = hsv;

	cv::Point center(src.cols / 2, src.rows / 2);

	//Expand image to an optimal size, for faster processing speed
	//Set widths of borders in four directions
	//If borderType==BORDER_CONSTANT, fill the borders with (0,0,0)
	cv::Mat padded;
	int opWidth = cv::getOptimalDFTSize(srcImg.rows);
	int opHeight = cv::getOptimalDFTSize(srcImg.cols);
	cv::copyMakeBorder(srcImg, padded, 0, opWidth - srcImg.rows, 0, opHeight - srcImg.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��1: %d\n", end - start);
// 	TRACE(szTmpLog);

	cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat comImg;
	//Merge into a double-channel image
	cv::merge(planes, 2, comImg);


// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��2-0: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Use the same image as input and output,
	//so that the results can fit in Mat well
	cv::dft(comImg, comImg);


// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��2-1: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Compute the magnitude
	//planes[0]=Re(DFT(I)), planes[1]=Im(DFT(I))
	//magnitude=sqrt(Re^2+Im^2)
	split(comImg, planes);
	cv::magnitude(planes[0], planes[1], planes[0]);

// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��2-2: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Switch to logarithmic scale, for better visual results
	//M2=log(1+M1)
	cv::Mat magMat = planes[0];
	magMat += cv::Scalar::all(1);
	cv::log(magMat, magMat);

// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��3: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Crop the spectrum
	//Width and height of magMat should be even, so that they can be divided by 2
	//-2 is 11111110 in binary system, operator & make sure width and height are always even
	magMat = magMat(cv::Rect(0, 0, magMat.cols & -2, magMat.rows & -2));

	//Rearrange the quadrants of Fourier image,
	//so that the origin is at the center of image,
	//and move the high frequency to the corners
	int cx = magMat.cols / 2;
	int cy = magMat.rows / 2;

	cv::Mat q0(magMat, cv::Rect(0, 0, cx, cy));
	cv::Mat q1(magMat, cv::Rect(0, cy, cx, cy));
	cv::Mat q2(magMat, cv::Rect(cx, cy, cx, cy));
	cv::Mat q3(magMat, cv::Rect(cx, 0, cx, cy));

	cv::Mat tmp;
	q0.copyTo(tmp);
	q2.copyTo(q0);
	tmp.copyTo(q2);

	q1.copyTo(tmp);
	q3.copyTo(q1);
	tmp.copyTo(q3);

// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��4: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Normalize the magnitude to [0,1], then to[0,255]
	cv::normalize(magMat, magMat, 0, 1, CV_MINMAX);
	cv::Mat magImg(magMat.size(), CV_8UC1);
	magMat.convertTo(magImg, CV_8UC1, 255, 0);
	//	imshow("magnitude", magImg);

	//Turn into binary image
	cv::threshold(magImg, magImg, 150, 255, CV_THRESH_BINARY);
	//	imshow("mag_binary", magImg);

// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��5: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Find lines with Hough Transformation
	std::vector<cv::Vec2f> lines;
	float pi180 = (float)CV_PI / 180;
	cv::Mat linImg(magImg.size(), CV_8UC3);
	cv::HoughLines(magImg, lines, 1, pi180, 100, 0, 0);


// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��6-0: %d\n", end - start);
// 	TRACE(szTmpLog);

	int numLines = lines.size();

	//Find the proper angel from the three found angels
	float angel = 0;
	float piThresh = (float)CV_PI / 90;
	float pi2 = (float)CV_PI / 2;
	for (int l = 0; l < numLines; l++)
	{
		float theta = lines[l][1];
		if (abs(theta) < piThresh || abs(theta - pi2) < piThresh)
			continue;
		else{
			angel = theta;
			break;
		}
	}

	//Calculate the rotation angel
	//The image has to be square,
	//so that the rotation angel can be calculate right
	angel = angel < pi2 ? angel : angel - CV_PI;
	if (angel != pi2){
		float angelT = srcImg.rows*tan(angel) / srcImg.cols;
		angel = atan(angelT);
	}
	float angelD = angel * 180 / (float)CV_PI;

// 	sprintf_s(szTmpLog, "the rotation angel to be applied: %f\n", angelD);
// 	g_pLogger->information(szTmpLog);
// 	TRACE(szTmpLog);

// 	end = clock();
// 	sprintf_s(szTmpLog, "ʱ��7: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Rotate the image to recover
	rotMat = cv::getRotationMatrix2D(center, angelD, 1.0);
	dst = cv::Mat::ones(src.size(), CV_8UC3);
	warpAffine(src, dst, rotMat, src.size(), cv::INTER_LINEAR, 0, cv::Scalar(255, 255, 255));

	end = clock();
	sprintf_s(szTmpLog, "ͼ���ƫ��ʱ��: %d, angelD = %f\n", end - start, angelD);
	g_pLogger->information(szTmpLog);
	TRACE(szTmpLog);

	return true;
}

bool FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat)
{
	if (lFix.size() < 3)
		return false;

	clock_t start, end;
	start = clock();
	char szTmpLog[400] = { 0 };

	std::vector<cv::Point2f> vecFixPt;
	RECTLIST::iterator itCP = lModelFix.begin();
	for (; itCP != lModelFix.end(); itCP++)
	{
		cv::Point2f pt;
#if 0
		pt.x = itCP->rt.x;
		pt.y = itCP->rt.y;
#else
		pt.x = itCP->rt.x + itCP->rt.width / 2;
		pt.y = itCP->rt.y + itCP->rt.height / 2;
#endif
		vecFixPt.push_back(pt);
	}
	std::vector<cv::Point2f> vecFixNewPt;
	RECTLIST::iterator itCP2 = lFix.begin();
	for (; itCP2 != lFix.end(); itCP2++)
	{
		cv::Point2f pt;
#if 0
		pt.x = itCP2->rt.x;
		pt.y = itCP2->rt.y;
#else
		pt.x = itCP2->rt.x + itCP2->rt.width / 2;
		pt.y = itCP2->rt.y + itCP2->rt.height / 2;
#endif
		vecFixNewPt.push_back(pt);
	}

	cv::Point2f srcTri[3];
	cv::Point2f dstTri[3];
	cv::Mat warp_mat(2, 3, CV_32FC1);
	cv::Mat warp_dst, warp_rotate_dst;
	for (int i = 0; i < vecFixPt.size(); i++)
	{
		srcTri[i] = vecFixNewPt[i];
		dstTri[i] = vecFixPt[i];
	}

	//	warp_dst = Mat::zeros(matCompPic.rows, matCompPic.cols, matCompPic.type());
	warp_mat = cv::getAffineTransform(srcTri, dstTri);
	cv::warpAffine(matCompPic, matCompPic, warp_mat, matCompPic.size(), 1, 0, cv::Scalar(255, 255, 255));

#if 1	//��������󣬼������ģ���ԭ����
	cv::Mat warp_mat2(2, 3, CV_32FC1);
	inverseMat = cv::getAffineTransform(dstTri, srcTri);
#endif

// 	RECTLIST::iterator itCP3 = lFix.begin();
// 	for (; itCP3 != lFix.end(); itCP3++)
// 	{
// 		cv::Point2f pt;
// 
// 		pt.x = warp_mat.ptr<double>(0)[0] * itCP3->rt.x + warp_mat.ptr<double>(0)[1] * itCP3->rt.y + warp_mat.ptr<double>(0)[2];
// 		pt.y = warp_mat.ptr<double>(1)[0] * itCP3->rt.x + warp_mat.ptr<double>(1)[1] * itCP3->rt.y + warp_mat.ptr<double>(1)[2];
// 		itCP3->rt.x = pt.x;
// 		itCP3->rt.y = pt.y;
// 	}

	end = clock();
	sprintf_s(szTmpLog, "ͼ��任ʱ��: %d, ptMod1(%.2f,%.2f), ptMod2(%.2f,%.2f), ptMod3(%.2f,%.2f), pt1(%.2f,%.2f), pt2(%.2f,%.2f), pt3(%.2f,%.2f)\n", end - start,\
		vecFixPt[0].x, vecFixPt[0].y, vecFixPt[1].x, vecFixPt[1].y, vecFixPt[2].x, vecFixPt[2].y, vecFixNewPt[0].x, vecFixNewPt[0].y, vecFixNewPt[1].x, vecFixNewPt[1].y, vecFixNewPt[2].x, vecFixNewPt[2].y);
	g_pLogger->information(szTmpLog);
	TRACE(szTmpLog);

	return true;
}

bool FixwarpPerspective(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat)
{
	if (lFix.size() < 4)
		return false;

	clock_t start, end;
	start = clock();
	char szTmpLog[400] = { 0 };

	std::vector<cv::Point2f> vecFixPt;
	RECTLIST::iterator itCP = lModelFix.begin();
	for (; itCP != lModelFix.end(); itCP++)
	{
		cv::Point2f pt;
		pt.x = itCP->rt.x + itCP->rt.width / 2;
		pt.y = itCP->rt.y + itCP->rt.height / 2;
		vecFixPt.push_back(pt);
	}
	std::vector<cv::Point2f> vecFixNewPt;
	RECTLIST::iterator itCP2 = lFix.begin();
	for (; itCP2 != lFix.end(); itCP2++)
	{
		cv::Point2f pt;
		pt.x = itCP2->rt.x + itCP2->rt.width / 2;
		pt.y = itCP2->rt.y + itCP2->rt.height / 2;
		vecFixNewPt.push_back(pt);
	}

	cv::Point2f srcTri[4];
	cv::Point2f dstTri[4];
	cv::Mat warp_mat(2, 3, CV_32FC1);
	cv::Mat warp_dst, warp_rotate_dst;
	for (int i = 0; i < vecFixPt.size(); i++)
	{
		srcTri[i] = vecFixNewPt[i];
		dstTri[i] = vecFixPt[i];
	}

	//	warp_dst = Mat::zeros(matCompPic.rows, matCompPic.cols, matCompPic.type());
	warp_mat = cv::getPerspectiveTransform(srcTri, dstTri);
	cv::warpPerspective(matCompPic, matCompPic, warp_mat, matCompPic.size(), 1, 0, cv::Scalar(255, 255, 255));

	end = clock();
	sprintf_s(szTmpLog, "ͼ��任ʱ��: %d, ptMod1(%.2f,%.2f), ptMod2(%.2f,%.2f), ptMod3(%.2f,%.2f), ptMod4(%.2f,%.2f), pt1(%.2f,%.2f), pt2(%.2f,%.2f), pt3(%.2f,%.2f), pt4(%.2f,%.2f)\n", end - start, \
		vecFixPt[0].x, vecFixPt[0].y, vecFixPt[1].x, vecFixPt[1].y, vecFixPt[2].x, vecFixPt[2].y, vecFixPt[3].x, vecFixPt[3].y,\
		vecFixNewPt[0].x, vecFixNewPt[0].y, vecFixNewPt[1].x, vecFixNewPt[1].y, vecFixNewPt[2].x, vecFixNewPt[2].y, vecFixNewPt[3].x, vecFixNewPt[3].y);
	g_pLogger->information(szTmpLog);
	TRACE(szTmpLog);

	return true;
}

bool PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat)
{
	if (lFix.size() != lModelFix.size())
		return false;

	if (lFix.size() == 3)
		FixWarpAffine(nPic, matCompPic, lFix, lModelFix, inverseMat);
	else if (lFix.size() == 4)
		FixwarpPerspective(nPic, matCompPic, lFix, lModelFix, inverseMat);

	return true;
}

bool encString(std::string& strSrc, std::string& strDst)
{
	bool bResult = true;
	if (g_strEncPwd.empty())
	{
		strDst = strSrc;
		return bResult;
	}
	try
	{
		Poco::Crypto::Cipher::Ptr pCipher = Poco::Crypto::CipherFactory::defaultFactory().createCipher(Poco::Crypto::CipherKey("aes256", g_strEncPwd));

		strDst = pCipher->encryptString(strSrc, Poco::Crypto::Cipher::ENC_BINHEX);
	}
	catch (...)
	{
		bResult = false;
		std::string strLog = "���ݼ���ʧ�ܣ���ԭ���ݲ���";
		g_pLogger->information(strLog);
	}
	return bResult;
}

bool decString(std::string& strSrc, std::string& strDst)
{
	bool bResult = true;
	if (g_strEncPwd.empty())
	{
		strDst = strSrc;
		return bResult;
	}
	try
	{
		Poco::Crypto::Cipher::Ptr pCipher = Poco::Crypto::CipherFactory::defaultFactory().createCipher(Poco::Crypto::CipherKey("aes256", g_strEncPwd));

		strDst = pCipher->decryptString(strSrc, Poco::Crypto::Cipher::ENC_BINHEX);
	}
	catch (...)
	{
		bResult = false;
		std::string strLog = "���ݽ���ʧ�ܣ���ԭ���ݲ���";
		g_pLogger->information(strLog);
	}
	return bResult;
}








bool GetInverseMat(RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat)
{
	std::vector<cv::Point2f> vecFixPt;
	std::vector<cv::Point2f> vecFixNewPt;

	RECTLIST::iterator itCP = lModelFix.begin();
	for (; itCP != lModelFix.end(); itCP++)
	{
		cv::Point2f pt;
		pt.x = itCP->rt.x + itCP->rt.width / 2;
		pt.y = itCP->rt.y + itCP->rt.height / 2;
		vecFixPt.push_back(pt);
	}
	RECTLIST::iterator itCP2 = lFix.begin();
	for (; itCP2 != lFix.end(); itCP2++)
	{
		cv::Point2f pt;
		pt.x = itCP2->rt.x + itCP2->rt.width / 2;
		pt.y = itCP2->rt.y + itCP2->rt.height / 2;
		vecFixNewPt.push_back(pt);
	}
	if (lFix.size() == 3)
	{
		cv::Point2f srcTri[3];
		cv::Point2f dstTri[3];
		cv::Mat warp_mat(2, 3, CV_32FC1);
		cv::Mat warp_dst, warp_rotate_dst;
		for (int i = 0; i < vecFixPt.size(); i++)
		{
			srcTri[i] = vecFixNewPt[i];
			dstTri[i] = vecFixPt[i];
		}
		inverseMat = cv::getAffineTransform(dstTri, srcTri);
	}
	else if (lFix.size() == 4)
	{
		cv::Point2f srcTri[4];
		cv::Point2f dstTri[4];
		cv::Mat warp_mat(2, 3, CV_32FC1);
		cv::Mat warp_dst, warp_rotate_dst;
		for (int i = 0; i < vecFixPt.size(); i++)
		{
			srcTri[i] = vecFixNewPt[i];
			dstTri[i] = vecFixPt[i];
		}
		inverseMat = cv::getAffineTransform(dstTri, srcTri);
	}
	return true;
}

bool GetPosition2(cv::Mat& inverseMat, cv::Rect& rtSrc, cv::Rect& rtDst)
{
	cv::Point2f pt;
	rtDst = rtSrc;

	pt.x = inverseMat.ptr<double>(0)[0] * rtSrc.x + inverseMat.ptr<double>(0)[1] * rtSrc.y + inverseMat.ptr<double>(0)[2];
	pt.y = inverseMat.ptr<double>(1)[0] * rtSrc.x + inverseMat.ptr<double>(1)[1] * rtSrc.y + inverseMat.ptr<double>(1)[2];
	rtDst.x = pt.x;
	rtDst.y = pt.y;
	return true;
}

//--------------	�����ƾ�ģ������	-----------------------------
bool SortByIndex(RECTPOS& rc1, RECTPOS& rc2)
{
	return rc1.nIndex < rc2.nIndex;
}

bool GetHeader(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	Poco::JSON::Object::Ptr objHeader = objTK->getObject("syncHeader");
	Poco::JSON::Object::Ptr objAnchorPoint = objTK->getObject("anchorPoint");

	int nPaperType = 1;		//ֽ�����ͣ�A3-1��A4-2
	nPaperType = objHeader->get("sheetType").convert<int>();

	//ͬ��ͷ
	std::vector<RECTPOS> vecHeader_H;
	std::vector<RECTPOS> vecHeader_V;
	//��ȡ�����е�ͬ��ͷ��Ϣ
	Poco::JSON::Array::Ptr arryItems = objAnchorPoint->getArray("items");
	for (int j = 0; j < arryItems->size(); j++)
	{
		Poco::JSON::Object::Ptr objItem = arryItems->getObject(j);
		int nHor = objItem->get("horIndex").convert<int>();
		int nVer = objItem->get("verIndex").convert<int>();
		RECTPOS rc;
		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
		if (nPaperType == 2)	//A4��ֱͬ��ͷ���ұߣ�A3��ֱͬ��ͷ�����
		{
			if (nHor == 0 && nVer == 0)
			{
				rc.nIndex = 0;
				vecHeader_H.push_back(rc);
			}
			else if (nHor != 0 && nVer == 0)
			{
				rc.nIndex = nHor;
				vecHeader_H.push_back(rc);
				rc.nIndex = 0;		//���涨λ�㣬�Դ�ֱͬ��ͷ��˵������0
				vecHeader_V.push_back(rc);
			}
			else if (nHor != 0 && nVer != 0)
			{
				rc.nIndex = nVer;
				vecHeader_V.push_back(rc);
			}
		}
		else
		{
			if (nHor == 0 && nVer == 0)
			{
				rc.nIndex = 0;
				vecHeader_H.push_back(rc);
				vecHeader_V.push_back(rc);
			}
			else if (nHor != 0 && nVer == 0)
			{
				rc.nIndex = nHor;
				vecHeader_H.push_back(rc);
			}
			else if (nHor == 0 && nVer != 0)
			{
				rc.nIndex = nVer;
				vecHeader_V.push_back(rc);
			}
		}
	}
	Poco::JSON::Array::Ptr arryHorHeaders = objHeader->getArray("horHeaders");
	for (int k = 0; k < arryHorHeaders->size(); k++)
	{
		Poco::JSON::Object::Ptr objItem = arryHorHeaders->getObject(k);
		RECTPOS rc;
		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
		rc.nIndex = objItem->get("index").convert<int>();
		vecHeader_H.push_back(rc);
	}
	Poco::JSON::Array::Ptr arryVerHeaders = objHeader->getArray("verHeaders");
	for (int k = 0; k < arryVerHeaders->size(); k++)
	{
		Poco::JSON::Object::Ptr objItem = arryVerHeaders->getObject(k);
		RECTPOS rc;
		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
		rc.nIndex = objItem->get("index").convert<int>();
		vecHeader_V.push_back(rc);
	}
	std::sort(vecHeader_H.begin(), vecHeader_H.end(), SortByIndex);
	std::sort(vecHeader_V.begin(), vecHeader_V.end(), SortByIndex);

	//����ͬ��ͷ��Ƥ���С
	cv::Rect rcTracker_H, rcTracker_V;
	cv::Point pt1 = vecHeader_H[0].rt.tl() - cv::Point(50, 50);
	cv::Point pt2 = vecHeader_H[vecHeader_H.size() - 1].rt.br() + cv::Point(50, 50);
	cv::Point pt3 = vecHeader_V[0].rt.tl() - cv::Point(50, 50);
	cv::Point pt4 = vecHeader_V[vecHeader_V.size() - 1].rt.br() + cv::Point(50, 50);
	pPaperModel->rtHTracker = cv::Rect(pt1, pt2);
	pPaperModel->rtVTracker = cv::Rect(pt3, pt4);

	RECTINFO rtSelHTracker, rtSelVTracker;
	rtSelHTracker.eCPType = H_HEAD;
	rtSelHTracker.rt = pPaperModel->rtHTracker;
	rtSelVTracker.eCPType = V_HEAD;
	rtSelVTracker.rt = pPaperModel->rtVTracker;
	pPaperModel->lSelHTracker.push_back(rtSelHTracker);		//ʶ��ʱ����ͬ��ͷ
	pPaperModel->lSelVTracker.push_back(rtSelVTracker);		//ʶ��ʱ����ͬ��ͷ

	//����ͬ��ͷ
	for (int m = 0; m < vecHeader_H.size(); m++)
	{
		RECTINFO rc;
		rc.eCPType = H_HEAD;
		rc.nHItem = vecHeader_H[m].nIndex;
		rc.nVItem = 0;
		rc.rt = vecHeader_H[m].rt;
		pPaperModel->lH_Head.push_back(rc);
	}
	for (int m = 0; m < vecHeader_V.size(); m++)
	{
		RECTINFO rc;
		rc.eCPType = V_HEAD;
		rc.nHItem = 0;
		rc.nVItem = vecHeader_V[m].nIndex;
		rc.rt = vecHeader_V[m].rt;
		pPaperModel->lV_Head.push_back(rc);
	}
	return true;
}

bool GetZkzh(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	cv::Point ptZkzh1, ptZkzh2;
	if (!objTK->isNull("baseInfo"))
	{
		Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
		Poco::JSON::Object::Ptr objZKZH = objBaseInfo->getObject("zkzh");
		Poco::JSON::Array::Ptr arryZkzhItems = objZKZH->getArray("items");
		for (int n = 0; n < arryZkzhItems->size(); n++)
		{
			Poco::JSON::Object::Ptr objItem = arryZkzhItems->getObject(n);
			Poco::JSON::Object::Ptr objPanel = objItem->getObject("panel");
			int nItem = objPanel->get("index").convert<int>();
			if (nItem == 0)
			{
				ptZkzh1.x = objPanel->get("x").convert<int>();
				ptZkzh1.y = objPanel->get("y").convert<int>();
			}
			if (nItem == arryZkzhItems->size() - 1)
			{
				ptZkzh2.x = objPanel->get("x").convert<int>() + objPanel->get("width").convert<int>();
				ptZkzh2.y = objPanel->get("y").convert<int>() + objPanel->get("height").convert<int>();
			}

			//��ȡÿ�е�׼��֤��Ϣ
			pSN_ITEM pSnItem = new SN_ITEM;
			pSnItem->nItem = nItem;

			Poco::JSON::Array::Ptr arryZkzhGrids = objItem->getArray("grids");
			for (int k = 0; k < arryZkzhGrids->size(); k++)
			{
				Poco::JSON::Object::Ptr objGrids = arryZkzhGrids->getObject(k);
				RECTINFO rc;
				rc.eCPType = SN;
				rc.rt.x = objGrids->get("x").convert<int>();
				rc.rt.y = objGrids->get("y").convert<int>();
				rc.rt.width = objGrids->get("width").convert<int>();
				rc.rt.height = objGrids->get("height").convert<int>();
				rc.nTH = nItem;
				rc.nSnVal = objGrids->get("index").convert<int>();
				rc.nHItem = objGrids->get("horIndex").convert<int>();
				rc.nVItem = objGrids->get("verIndex").convert<int>();
				pSnItem->lSN.push_back(rc);
			}
			pPaperModel->lSNInfo.push_back(pSnItem);
		}
		ptZkzh1 += cv::Point(2, 2);			//׼��֤��Ƥ�����ţ���ֹѡ��̫��
		ptZkzh2 -= cv::Point(2, 2);			//׼��֤��Ƥ�����ţ���ֹѡ��̫��
		pPaperModel->rtSNTracker = cv::Rect(ptZkzh1, ptZkzh2);
	}
	return true;
}

bool GetOMR(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	Poco::JSON::Array::Ptr arryElement = objTK->getArray("elements");
	for (int m = 0; m < arryElement->size(); m++)
	{
		Poco::JSON::Object::Ptr objElement = arryElement->getObject(m);
		Poco::JSON::Object::Ptr objItem = objElement->getObject("item");
		Poco::JSON::Array::Ptr arryQuestions = objItem->getArray("questions");
		for (int n = 0; n < arryQuestions->size(); n++)
		{
			Poco::JSON::Object::Ptr objQuestion = arryQuestions->getObject(n);
			OMR_QUESTION omrItem;
			omrItem.nTH = objQuestion->get("num").convert<int>();
			omrItem.nSingle = objQuestion->get("choiceType").convert<int>() - 1;

			Poco::JSON::Array::Ptr arryOptions = objQuestion->getArray("options");
			for (int k = 0; k < arryOptions->size(); k++)
			{
				Poco::JSON::Object::Ptr objOptions = arryOptions->getObject(k);
				Poco::JSON::Object::Ptr objPanel = objOptions->getObject("panel");
				RECTINFO rc;
				rc.eCPType = OMR;
				rc.rt.x = objPanel->get("x").convert<int>();
				rc.rt.y = objPanel->get("y").convert<int>();
				rc.rt.width = objPanel->get("width").convert<int>();
				rc.rt.height = objPanel->get("height").convert<int>();
				rc.nHItem = objPanel->get("horIndex").convert<int>();
				rc.nVItem = objPanel->get("verIndex").convert<int>();
				rc.nAnswer = (int)objOptions->get("label").convert<char>() - 65;
				rc.nTH = omrItem.nTH;
				rc.nSingle = omrItem.nSingle;
				omrItem.lSelAnswer.push_back(rc);
			}
			pPaperModel->lOMR2.push_back(omrItem);
		}
	}
	return true;
}

bool GetCourse(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	Poco::JSON::Object::Ptr objSubject = objTK->getObject("subject");
	Poco::JSON::Array::Ptr arryCourse = objSubject->getArray("items");
	for (int k = 0; k < arryCourse->size(); k++)
	{
		Poco::JSON::Object::Ptr objItem = arryCourse->getObject(k);
		RECTINFO rcCourse;
		rcCourse.eCPType = COURSE;
		rcCourse.rt.x = objItem->get("x").convert<int>();
		rcCourse.rt.y = objItem->get("y").convert<int>();
		rcCourse.rt.width = objItem->get("width").convert<int>();
		rcCourse.rt.height = objItem->get("height").convert<int>();
		rcCourse.nHItem = objItem->get("horIndex").convert<int>();
		rcCourse.nVItem = objItem->get("verIndex").convert<int>();
		pPaperModel->lCourse.push_back(rcCourse);
	}
	return true;
}

bool GetQK(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	if (!objTK->isNull("baseInfo"))
	{
		Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
		Poco::JSON::Object::Ptr objAbsentBreach = objBaseInfo->getObject("absentAndBreach");
		Poco::JSON::Object::Ptr objAbsent = objAbsentBreach->getObject("absent");
		RECTINFO rcAbsent;
		rcAbsent.eCPType = QK_CP;
		rcAbsent.rt.x = objAbsent->get("x").convert<int>();
		rcAbsent.rt.y = objAbsent->get("y").convert<int>();
		rcAbsent.rt.width = objAbsent->get("width").convert<int>();
		rcAbsent.rt.height = objAbsent->get("height").convert<int>();
		rcAbsent.nHItem = objAbsent->get("horIndex").convert<int>();
		rcAbsent.nVItem = objAbsent->get("verIndex").convert<int>();
		pPaperModel->lQK_CP.push_back(rcAbsent);
	}
	return true;
}

pMODEL LoadMakePaperData(std::string strData)
{
	pMODEL pModel = new MODEL;

	USES_CONVERSION;
	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strData);		//strJsnData
		Poco::JSON::Array::Ptr arryData = result.extract<Poco::JSON::Array::Ptr>();
		pModel->nPicNum = arryData->size();
		for (int i = 0; i < arryData->size(); i++)
		{
			Poco::JSON::Object::Ptr objTK = arryData->getObject(i);
			Poco::JSON::Object::Ptr objSubject = objTK->getObject("subject");
			Poco::JSON::Object::Ptr objPageNum = objTK->getObject("pageNum");

			if (i == 0)
			{
				Poco::JSON::Object::Ptr objCurSubject = objSubject->getObject("curSubject");
				pModel->strModelName = A2T(CMyCodeConvert::Utf8ToGb2312(objCurSubject->get("name").convert<std::string>()).c_str());
			}			

			std::string strName = Poco::format("model%d.jpg", i + 1);

			pPAPERMODEL pPaperModel = new PAPERMODEL;
			pPaperModel->nPaper = objPageNum->get("curPageNum").convert<int>() - 1;			//add from 0
			pPaperModel->strModelPicName = A2T(strName.c_str());	//ͼƬ���ƣ�Ŀǰ��֪��			//**********	test	*****************

 			//ͬ��ͷ
			GetHeader(objTK, pPaperModel);

			//׼��֤��
			GetZkzh(objTK, pPaperModel);

			//OMR����
			GetOMR(objTK, pPaperModel);

			//��ӿ�Ŀ��
			GetCourse(objTK, pPaperModel);

			//���ȱ����
			GetQK(objTK, pPaperModel);

			//��ӻҶȵ�

			//��Ӱ�У���

			//����Ծ�ģ�嵽��ģ��
			pModel->vecPaperModel.push_back(pPaperModel);
		}
		pModel->nType = 1;
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		SAFE_RELEASE(pModel);
		std::string strErrInfo;
		strErrInfo.append("����ģ���ļ�����jsonʧ��: ");
		strErrInfo.append(jsone.message());
		g_pLogger->information(strErrInfo);
	}
	catch (Poco::Exception& exc)
	{
		SAFE_RELEASE(pModel);
		std::string strErrInfo;
		strErrInfo.append("����ģ���ļ�����jsonʧ��2: ");
		strErrInfo.append(exc.message());
		g_pLogger->information(strErrInfo);
	}

	return pModel;
}
//---------------------------------------------------------------