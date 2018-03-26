#include "stdafx.h"
#include "global.h"
#include <Tlhelp32.h>

const char* pPwd = NULL;
static char s_szZipPwd[20] = "static";

//扫描用
void CopyData(char *dest, const char *src, int dataByteSize, bool isConvert, int height)
{
#if 1
	char * p = dest;
	if (!isConvert)
	{
		memcpy_s(dest, dataByteSize, src, dataByteSize);
		return;
	}
	if (height <= 0) return;
	//int height = dataByteSize/rowByteSize;
	int rowByteSize = dataByteSize / height;
	src = src + dataByteSize - rowByteSize;
	for (int i = 0; i < height; i++)
	{
		memcpy_s(dest, rowByteSize, src, rowByteSize);
		dest += rowByteSize;
		src -= rowByteSize;
	}
#else
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
#endif
}

bool SortByCharAnchorArea(pST_CHARACTER_ANCHOR_AREA& st1, pST_CHARACTER_ANCHOR_AREA& st2)
{
	return st1->nIndex < st2->nIndex;
}

bool SortByCharacterConfidence(pST_CHARACTER_ANCHOR_POINT& st1, pST_CHARACTER_ANCHOR_POINT& st2)
{
	return st1->fConfidence > st2->fConfidence;
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

bool SortByItemDiff(ST_ITEM_DIFF& item1, ST_ITEM_DIFF& item2)
{
	return abs(item1.fDiff) > abs(item2.fDiff) ? true : false;
}

bool SortByItemDensity(pRECTINFO item1, pRECTINFO item2)
{
	return item1->fRealValuePercent > item2->fRealValuePercent ? true : false;
}

bool SortByItemGray(pRECTINFO item1, pRECTINFO item2)
{
	return item1->fRealMeanGray < item2->fRealMeanGray ? true : false;
}

bool SortByPositionXYInterval(cv::Rect& rt1, cv::Rect& rt2)
{
	bool bResult = true;

	if (abs(rt1.y - rt2.y) > 9)
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

bool SortByTH(RECTINFO& rc1, RECTINFO& rc2)
{
	bool bResult = rc1.nTH < rc2.nTH ? true : false;
	if (!bResult)
	{
		if (rc1.nTH == rc2.nTH)
		{
			if (rc1.eCPType == SN)
				bResult = rc1.nTH == rc2.nTH ? rc1.nSnVal < rc2.nSnVal : false;
			else if (rc1.eCPType == OMR)
				bResult = rc1.nTH == rc2.nTH ? rc1.nAnswer < rc2.nAnswer : false;
			else if (rc1.eCPType == ELECT_OMR)
				bResult = rc1.nTH == rc2.nTH ? rc1.nAnswer < rc2.nAnswer : false;
		}
	}
		
	return bResult;
}

bool SortByOmrTH(OMR_QUESTION& omr1, OMR_QUESTION& omr2)
{
	bool bResult = omr1.nTH < omr2.nTH ? true : false;

	return bResult;
}

bool SortStringByDown(std::string& str1, std::string& str2)
{
	return str1 > str2 ? true : false;
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

// bool ZipFile(CString strSrcPath, CString strDstPath, CString strExtName /*= _T(".zip")*/)
// {
// 	USES_CONVERSION;
// //	CString modelName = strSrcPath.Right(strSrcPath.GetLength() - strSrcPath.ReverseFind('\\') - 1);
// 	CString zipName = strDstPath + strExtName;
// 	std::string strUtf8ZipName = CMyCodeConvert::Gb2312ToUtf8(T2A(zipName));
// 
// 	try
// 	{
// 		Poco::File p(strUtf8ZipName);	//T2A(zipName)
// 		if (p.exists())
// 			p.remove(true);
// 	}
// 	catch (Poco::Exception)
// 	{
// 	}
// 
// #ifdef USES_PWD_ZIP_UNZIP
// 	pPwd = s_szZipPwd;
// #endif
// 
// //	std::string strModelPath = T2A(strSrcPath);
// 	std::string strUtf8ModelPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strSrcPath));
// 	try
// 	{
// 		Poco::File p2(strUtf8ModelPath);	//T2A(zipName)
// 		if (!p2.exists())
// 		{
// 			std::string strErr = Poco::format("需要压缩的原文件夹(%s)不存在。", T2A(strSrcPath));
// 			g_pLogger->information(strErr);
// 			return false;
// 		}
// 	}
// 	catch (Poco::Exception)
// 	{
// 	}
// 
// 	HZIP hz = CreateZip(zipName, pPwd);
// 
// 	Poco::DirectoryIterator it(strUtf8ModelPath);	//strModelPath
// 	Poco::DirectoryIterator end;
// 	while (it != end)
// 	{
// 		Poco::Path p(it->path());
// //		std::string strZipFileName = p.getFileName();
// 		std::string strPath = CMyCodeConvert::Utf8ToGb2312(p.toString());
// 		std::string strZipFileName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());
// 		CString strZipPath = A2T(strPath.c_str());
// 		CString strName = A2T(strZipFileName.c_str());
// //		ZipAdd(hz, A2T(strZipFileName.c_str()), A2T(p.toString().c_str()));
// 		ZipAdd(hz, strName, strZipPath);
// 		it++;
// 	}
// 	CloseZip(hz);
// 
// 	return true;
// }
// 
// bool UnZipFile(CString strZipPath)
// {
// 	USES_CONVERSION;
// 	int nPos = strZipPath.ReverseFind('.');
// 	CString strPath = strZipPath.Left(nPos);		//.zip		strZipPath.GetLength() - 4
// 	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strPath));
// 
// 	try
// 	{
// 		Poco::File p(strUtf8Path);	//T2A(strPath)
// 		if (p.exists())
// 			p.remove(true);
// 	}
// 	catch (cv::Exception)
// 	{
// 	}
// 
// #ifdef USES_PWD_ZIP_UNZIP
// 	pPwd = s_szZipPwd;
// #endif
// 
// 	HZIP hz = OpenZip(strZipPath, pPwd);
// 	ZIPENTRY ze;
// 	GetZipItem(hz, -1, &ze);
// 	int numitems = ze.index;
// 	SetUnzipBaseDir(hz, strPath);
// 	for (int i = 0; i < numitems; i++)
// 	{
// 		GetZipItem(hz, i, &ze);
// 		UnzipItem(hz, i, ze.name);
// 	}
// 	CloseZip(hz);
// 
// 	return true;
// }

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
		pModel->strModelName	= CMyCodeConvert::Utf8ToGb2312(objData->get("modelName").convert<std::string>());
		pModel->strModelDesc	= CMyCodeConvert::Utf8ToGb2312(objData->get("modelDesc").convert<std::string>());
		if (objData->has("modelType"))
			pModel->nType		= objData->get("modelType").convert<int>();
		else
			pModel->nType		= 0;
		pModel->nSaveMode		= objData->get("modeSaveMode").convert<int>();
		pModel->nPicNum			= objData->get("paperModelCount").convert<int>();
		pModel->nEnableModify	= objData->get("enableModify").convert<int>();
		pModel->nABModel		= objData->get("abPaper").convert<int>();
		pModel->nHasHead		= objData->get("hasHead").convert<int>();
		if (objData->has("hasElectOmr"))
			pModel->nHasElectOmr = objData->get("hasElectOmr").convert<int>();
		if (objData->has("nZkzhType"))
			pModel->nZkzhType = objData->get("nZkzhType").convert<int>();
		if (objData->has("nScanDpi"))
			pModel->nScanDpi = objData->get("nScanDpi").convert<int>();
		if (objData->has("nScanAutoCut"))
			pModel->nAutoCut = objData->get("nScanAutoCut").convert<int>();
		if (objData->has("nScanSize"))
			pModel->nScanSize = objData->get("nScanSize").convert<int>();
		if (objData->has("nScanType"))
			pModel->nScanType = objData->get("nScanType").convert<int>();
		if (objData->has("nUseWordAnchorPoint"))
			pModel->nUseWordAnchorPoint = objData->get("nUseWordAnchorPoint").convert<int>();
		if (objData->has("nCharacterAnchorPoint"))
			pModel->nCharacterAnchorPoint = objData->get("nCharacterAnchorPoint").convert<int>();
		if (objData->has("nUsePagination"))
			pModel->nUsePagination = objData->get("nUsePagination").convert<int>();
		if (objData->has("nChkLostCorner"))
			pModel->nChkLostCorner = objData->get("nChkLostCorner").convert<int>();

// 		if (objData->has("gaussKernel"))
// 			pModel->nGaussKernel = objData->get("gaussKernel").convert<int>();
// 		if (objData->has("sharpKernel"))
// 			pModel->nSharpKernel = objData->get("sharpKernel").convert<int>();
// 		if (objData->has("cannyKernel"))
// 			pModel->nCannyKernel = objData->get("cannyKernel").convert<int>();
// 		if (objData->has("dilateKernel"))
// 			pModel->nDilateKernel = objData->get("dilateKernel").convert<int>();

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
			paperModelInfo->strModelPicName = CMyCodeConvert::Utf8ToGb2312(jsnPaperObj->get("modelPicName").convert<std::string>());

			if (jsnPaperObj->has("picW"))			//add on 16.8.29
				paperModelInfo->nPicW = jsnPaperObj->get("picW").convert<int>();
			if (jsnPaperObj->has("picH"))			//add on 16.8.29
				paperModelInfo->nPicH = jsnPaperObj->get("picH").convert<int>();

			paperModelInfo->rtHTracker.x = jsnPaperObj->get("rtHTracker.x").convert<int>();
			paperModelInfo->rtHTracker.y = jsnPaperObj->get("rtHTracker.y").convert<int>();
			paperModelInfo->rtHTracker.width = jsnPaperObj->get("rtHTracker.width").convert<int>();
			paperModelInfo->rtHTracker.height = jsnPaperObj->get("rtHTracker.height").convert<int>();
			paperModelInfo->rtVTracker.x = jsnPaperObj->get("rtVTracker.x").convert<int>();
			paperModelInfo->rtVTracker.y = jsnPaperObj->get("rtVTracker.y").convert<int>();
			paperModelInfo->rtVTracker.width = jsnPaperObj->get("rtVTracker.width").convert<int>();
			paperModelInfo->rtVTracker.height = jsnPaperObj->get("rtVTracker.height").convert<int>();
			paperModelInfo->rcSNTracker.rt.x = jsnPaperObj->get("rtSNTracker.x").convert<int>();
			paperModelInfo->rcSNTracker.rt.y = jsnPaperObj->get("rtSNTracker.y").convert<int>();
			paperModelInfo->rcSNTracker.rt.width = jsnPaperObj->get("rtSNTracker.width").convert<int>();
			paperModelInfo->rcSNTracker.rt.height = jsnPaperObj->get("rtSNTracker.height").convert<int>();
			if (jsnPaperObj->has("rtSNTracker.nRecogFlag"))
				paperModelInfo->rcSNTracker.nRecogFlag = jsnPaperObj->get("rtSNTracker.nRecogFlag").convert<int>();

			Poco::JSON::Array::Ptr arraySelHTracker = jsnPaperObj->getArray("hTrackerRect");
			Poco::JSON::Array::Ptr arraySelVTracker = jsnPaperObj->getArray("vTrackerRect");
			Poco::JSON::Array::Ptr arraySelFixRoi = jsnPaperObj->getArray("selRoiRect");
			Poco::JSON::Array::Ptr arrayFixCP = jsnPaperObj->getArray("FixCP");
			Poco::JSON::Array::Ptr arrayHHead = jsnPaperObj->getArray("H_Head");
			Poco::JSON::Array::Ptr arrayVHead = jsnPaperObj->getArray("V_Head");
			Poco::JSON::Array::Ptr arrayPage;
			if(jsnPaperObj->has("Pagination"))
				arrayPage = jsnPaperObj->getArray("Pagination");
			Poco::JSON::Array::Ptr arrayABModel = jsnPaperObj->getArray("ABModel");
			Poco::JSON::Array::Ptr arrayCourse = jsnPaperObj->getArray("Course");
			Poco::JSON::Array::Ptr arrayQKCP = jsnPaperObj->getArray("QKCP");
			Poco::JSON::Array::Ptr arrayWJCP;
			if(jsnPaperObj ->has("WJCP"))
				arrayWJCP = jsnPaperObj->getArray("WJCP");
			Poco::JSON::Array::Ptr arrayGrayCP = jsnPaperObj->getArray("GrayCP");
			Poco::JSON::Array::Ptr arrayWhiteCP = jsnPaperObj->getArray("WhiteCP");
			Poco::JSON::Array::Ptr arraySn = jsnPaperObj->getArray("snList");
			Poco::JSON::Array::Ptr arrayOmr = jsnPaperObj->getArray("selOmrRect");
			Poco::JSON::Array::Ptr arrayElectOmr;
			if (jsnPaperObj->has("electOmrList"))
				arrayElectOmr = jsnPaperObj->getArray("electOmrList");
			Poco::JSON::Array::Ptr arrayCharacterAnchorArea;
			if (jsnPaperObj->has("characterAnchorArea"))
			{
				arrayCharacterAnchorArea = jsnPaperObj->getArray("characterAnchorArea");

				//加载模板图像，只有存在文字定时时才需要，用到了模板匹配
				CString strPicPath = g_strCurrentPath + _T("Model\\") + A2T(pModel->strModelName.c_str()) + _T("\\") + A2T(paperModelInfo->strModelPicName.c_str());
				paperModelInfo->matModel = cv::imread((std::string)(CT2CA)strPicPath);
			}

			for (int i = 0; i < arrayFixCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayFixCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

				if (jsnRectInfoObj->has("standardArea"))
					rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
				if (jsnRectInfoObj->has("standardDensity"))
					rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
				if (jsnRectInfoObj->has("standardMeanGray"))
					rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
				if (jsnRectInfoObj->has("standardStddev"))
					rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

				paperModelInfo->lFix.push_back(rc);
			}
			for (int i = 0; i < arrayHHead->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayHHead->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

				if (jsnRectInfoObj->has("standardArea"))
					rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
				if (jsnRectInfoObj->has("standardDensity"))
					rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
				if (jsnRectInfoObj->has("standardMeanGray"))
					rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
				if (jsnRectInfoObj->has("standardStddev"))
					rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

				paperModelInfo->lH_Head.push_back(rc);
			}
			for (int i = 0; i < arrayVHead->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayVHead->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

				if (jsnRectInfoObj->has("standardArea"))
					rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
				if (jsnRectInfoObj->has("standardDensity"))
					rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
				if (jsnRectInfoObj->has("standardMeanGray"))
					rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
				if (jsnRectInfoObj->has("standardStddev"))
					rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

				paperModelInfo->lV_Head.push_back(rc);
			}
			if (jsnPaperObj->has("Pagination"))
			{
				for (int i = 0; i < arrayPage->size(); i++)
				{
					Poco::JSON::Object::Ptr jsnRectInfoObj = arrayPage->getObject(i);
					RECTINFO rc;
					rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
					rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
					rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

					if (jsnRectInfoObj->has("standardArea"))
						rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
					if (jsnRectInfoObj->has("standardDensity"))
						rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
					if (jsnRectInfoObj->has("standardMeanGray"))
						rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
					if (jsnRectInfoObj->has("standardStddev"))
						rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

					rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
					rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
					rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
					rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
					rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
					rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
					rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

					if (jsnRectInfoObj->has("gaussKernel"))
						rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
					if (jsnRectInfoObj->has("sharpKernel"))
						rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
					if (jsnRectInfoObj->has("cannyKernel"))
						rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
					if (jsnRectInfoObj->has("dilateKernel"))
						rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

					paperModelInfo->lPagination.push_back(rc);
				}
			}
			for (int i = 0; i < arrayABModel->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayABModel->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

				if (jsnRectInfoObj->has("standardArea"))
					rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
				if (jsnRectInfoObj->has("standardDensity"))
					rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
				if (jsnRectInfoObj->has("standardMeanGray"))
					rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
				if (jsnRectInfoObj->has("standardStddev"))
					rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

				paperModelInfo->lABModel.push_back(rc);
			}
			for (int i = 0; i < arrayCourse->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayCourse->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

				if (jsnRectInfoObj->has("standardArea"))
					rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
				if (jsnRectInfoObj->has("standardDensity"))
					rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
				if (jsnRectInfoObj->has("standardMeanGray"))
					rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
				if (jsnRectInfoObj->has("standardStddev"))
					rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

				paperModelInfo->lCourse.push_back(rc);
			}
			for (int i = 0; i < arrayQKCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayQKCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

				if (jsnRectInfoObj->has("standardArea"))
					rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
				if (jsnRectInfoObj->has("standardDensity"))
					rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
				if (jsnRectInfoObj->has("standardMeanGray"))
					rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
				if (jsnRectInfoObj->has("standardStddev"))
					rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

				paperModelInfo->lQK_CP.push_back(rc);
			}
			if (jsnPaperObj->has("WJCP"))
			{
				for (int i = 0; i < arrayWJCP->size(); i++)
				{
					Poco::JSON::Object::Ptr jsnRectInfoObj = arrayWJCP->getObject(i);
					RECTINFO rc;
					rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
					rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
					rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

					if (jsnRectInfoObj->has("standardArea"))
						rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
					if (jsnRectInfoObj->has("standardDensity"))
						rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
					if (jsnRectInfoObj->has("standardMeanGray"))
						rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
					if (jsnRectInfoObj->has("standardStddev"))
						rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

					rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
					rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
					rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
					rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
					rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
					rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
					rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

					if (jsnRectInfoObj->has("gaussKernel"))
						rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
					if (jsnRectInfoObj->has("sharpKernel"))
						rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
					if (jsnRectInfoObj->has("cannyKernel"))
						rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
					if (jsnRectInfoObj->has("dilateKernel"))
						rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

					paperModelInfo->lWJ_CP.push_back(rc);
				}
			}			
			for (int i = 0; i < arrayGrayCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayGrayCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

				if (jsnRectInfoObj->has("standardArea"))
					rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
				if (jsnRectInfoObj->has("standardDensity"))
					rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
				if (jsnRectInfoObj->has("standardMeanGray"))
					rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
				if (jsnRectInfoObj->has("standardStddev"))
					rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

				paperModelInfo->lGray.push_back(rc);
			}
			for (int i = 0; i < arrayWhiteCP->size(); i++)
			{
				Poco::JSON::Object::Ptr jsnRectInfoObj = arrayWhiteCP->getObject(i);
				RECTINFO rc;
				rc.eCPType = (CPType)jsnRectInfoObj->get("eType").convert<int>();
				rc.fStandardValuePercent = jsnRectInfoObj->get("standardValPercent").convert<float>();
				rc.fStandardValue = jsnRectInfoObj->get("standardVal").convert<float>();

				if (jsnRectInfoObj->has("standardArea"))
					rc.fStandardArea = jsnRectInfoObj->get("standardArea").convert<float>();
				if (jsnRectInfoObj->has("standardDensity"))
					rc.fStandardDensity = jsnRectInfoObj->get("standardDensity").convert<float>();
				if (jsnRectInfoObj->has("standardMeanGray"))
					rc.fStandardMeanGray = jsnRectInfoObj->get("standardMeanGray").convert<float>();
				if (jsnRectInfoObj->has("standardStddev"))
					rc.fStandardStddev = jsnRectInfoObj->get("standardStddev").convert<float>();

				rc.nThresholdValue = jsnRectInfoObj->get("thresholdValue").convert<int>();
				rc.nHItem = jsnRectInfoObj->get("hHeadItem").convert<int>();
				rc.nVItem = jsnRectInfoObj->get("vHeadItem").convert<int>();
				rc.rt.x = jsnRectInfoObj->get("left").convert<int>();
				rc.rt.y = jsnRectInfoObj->get("top").convert<int>();
				rc.rt.width = jsnRectInfoObj->get("width").convert<int>();
				rc.rt.height = jsnRectInfoObj->get("height").convert<int>();

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

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

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

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

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

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

				if (jsnRectInfoObj->has("gaussKernel"))
					rc.nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
				if (jsnRectInfoObj->has("sharpKernel"))
					rc.nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
				if (jsnRectInfoObj->has("cannyKernel"))
					rc.nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
				if (jsnRectInfoObj->has("dilateKernel"))
					rc.nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

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

					if (jsnOmrObj->has("standardArea"))
						rc.fStandardArea = jsnOmrObj->get("standardArea").convert<float>();
					if (jsnOmrObj->has("standardDensity"))
						rc.fStandardDensity = jsnOmrObj->get("standardDensity").convert<float>();
					if (jsnOmrObj->has("standardMeanGray"))
						rc.fStandardMeanGray = jsnOmrObj->get("standardMeanGray").convert<float>();
					if (jsnOmrObj->has("standardStddev"))
						rc.fStandardStddev = jsnOmrObj->get("standardStddev").convert<float>();

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

					if (jsnOmrObj->has("gaussKernel"))
						rc.nGaussKernel = jsnOmrObj->get("gaussKernel").convert<int>();
					if (jsnOmrObj->has("sharpKernel"))
						rc.nSharpKernel = jsnOmrObj->get("sharpKernel").convert<int>();
					if (jsnOmrObj->has("cannyKernel"))
						rc.nCannyKernel = jsnOmrObj->get("cannyKernel").convert<int>();
					if (jsnOmrObj->has("dilateKernel"))
						rc.nDilateKernel = jsnOmrObj->get("dilateKernel").convert<int>();

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

					if (jsnSnObj->has("standardArea"))
						rc.fStandardArea = jsnSnObj->get("standardArea").convert<float>();
					if (jsnSnObj->has("standardDensity"))
						rc.fStandardDensity = jsnSnObj->get("standardDensity").convert<float>();
					if (jsnSnObj->has("standardMeanGray"))
						rc.fStandardMeanGray = jsnSnObj->get("standardMeanGray").convert<float>();
					if (jsnSnObj->has("standardStddev"))
						rc.fStandardStddev = jsnSnObj->get("standardStddev").convert<float>();

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

					if (jsnSnObj->has("gaussKernel"))
						rc.nGaussKernel = jsnSnObj->get("gaussKernel").convert<int>();
					if (jsnSnObj->has("sharpKernel"))
						rc.nSharpKernel = jsnSnObj->get("sharpKernel").convert<int>();
					if (jsnSnObj->has("cannyKernel"))
						rc.nCannyKernel = jsnSnObj->get("cannyKernel").convert<int>();
					if (jsnSnObj->has("dilateKernel"))
						rc.nDilateKernel = jsnSnObj->get("dilateKernel").convert<int>();

					pSnItem->lSN.push_back(rc);
				}
				paperModelInfo->lSNInfo.push_back(pSnItem);
			}
			if (jsnPaperObj->has("electOmrList"))
			{
				for (int i = 0; i < arrayElectOmr->size(); i++)
				{
					Poco::JSON::Object::Ptr jsnRectInfoObj = arrayElectOmr->getObject(i);
					ELECTOMR_QUESTION objElectOmr;
					objElectOmr.sElectOmrGroupInfo.nGroupID = jsnRectInfoObj->get("nGroupID").convert<int>();
					objElectOmr.sElectOmrGroupInfo.nAllCount = jsnRectInfoObj->get("nAllCount").convert<int>();
					objElectOmr.sElectOmrGroupInfo.nRealCount = jsnRectInfoObj->get("nRealCount").convert<int>();
					Poco::JSON::Array::Ptr omrList = jsnRectInfoObj->getArray("omrlist");
					for (int j = 0; j < omrList->size(); j++)
					{
						Poco::JSON::Object::Ptr jsnOmrObj = omrList->getObject(j);
						RECTINFO rc;
						rc.eCPType = (CPType)jsnOmrObj->get("eType").convert<int>();
						rc.nThresholdValue = jsnOmrObj->get("thresholdValue").convert<int>();
						rc.fStandardValuePercent = jsnOmrObj->get("standardValPercent").convert<float>();
						rc.fStandardValue = jsnOmrObj->get("standardVal").convert<float>();

						if (jsnOmrObj->has("standardArea"))
							rc.fStandardArea = jsnOmrObj->get("standardArea").convert<float>();
						if (jsnOmrObj->has("standardDensity"))
							rc.fStandardDensity = jsnOmrObj->get("standardDensity").convert<float>();
						if (jsnOmrObj->has("standardMeanGray"))
							rc.fStandardMeanGray = jsnOmrObj->get("standardMeanGray").convert<float>();
						if (jsnOmrObj->has("standardStddev"))
							rc.fStandardStddev = jsnOmrObj->get("standardStddev").convert<float>();

						rc.nTH = jsnOmrObj->get("nTH").convert<int>();
						rc.nAnswer = jsnOmrObj->get("nAnswer").convert<int>();
						rc.rt.x = jsnOmrObj->get("left").convert<int>();
						rc.rt.y = jsnOmrObj->get("top").convert<int>();
						rc.rt.width = jsnOmrObj->get("width").convert<int>();
						rc.rt.height = jsnOmrObj->get("height").convert<int>();
						rc.nHItem = jsnOmrObj->get("hHeadItem").convert<int>();
						rc.nVItem = jsnOmrObj->get("vHeadItem").convert<int>();

						if (jsnOmrObj->has("gaussKernel"))
							rc.nGaussKernel = jsnOmrObj->get("gaussKernel").convert<int>();
						if (jsnOmrObj->has("sharpKernel"))
							rc.nSharpKernel = jsnOmrObj->get("sharpKernel").convert<int>();
						if (jsnOmrObj->has("cannyKernel"))
							rc.nCannyKernel = jsnOmrObj->get("cannyKernel").convert<int>();
						if (jsnOmrObj->has("dilateKernel"))
							rc.nDilateKernel = jsnOmrObj->get("dilateKernel").convert<int>();

						objElectOmr.lItemInfo.push_back(rc);
					}
					paperModelInfo->lElectOmr.push_back(objElectOmr);
				}
			}
			if (jsnPaperObj->has("characterAnchorArea"))
			{
				for (int i = 0; i < arrayCharacterAnchorArea->size(); i++)
				{
					Poco::JSON::Object::Ptr jsnRectInfoObj = arrayCharacterAnchorArea->getObject(i);
					pST_CHARACTER_ANCHOR_AREA pobjCharacterAnchorArea = new ST_CHARACTER_ANCHOR_AREA();
					pobjCharacterAnchorArea->nIndex = jsnRectInfoObj->get("nIndex").convert<int>();
					pobjCharacterAnchorArea->nThresholdValue = jsnRectInfoObj->get("nThreshold").convert<int>();
					if(jsnRectInfoObj->has("nConfidence"))
						pobjCharacterAnchorArea->nCharacterConfidence = jsnRectInfoObj->get("nConfidence").convert<int>();
					if (jsnRectInfoObj->has("nRectsInArea"))
						pobjCharacterAnchorArea->nRects = jsnRectInfoObj->get("nRectsInArea").convert<int>();
					pobjCharacterAnchorArea->nGaussKernel = jsnRectInfoObj->get("gaussKernel").convert<int>();
					pobjCharacterAnchorArea->nSharpKernel = jsnRectInfoObj->get("sharpKernel").convert<int>();
					pobjCharacterAnchorArea->nCannyKernel = jsnRectInfoObj->get("cannyKernel").convert<int>();
					pobjCharacterAnchorArea->nDilateKernel = jsnRectInfoObj->get("dilateKernel").convert<int>();

					pobjCharacterAnchorArea->rt.x = jsnRectInfoObj->get("left").convert<int>();
					pobjCharacterAnchorArea->rt.y = jsnRectInfoObj->get("top").convert<int>();
					pobjCharacterAnchorArea->rt.width = jsnRectInfoObj->get("width").convert<int>();
					pobjCharacterAnchorArea->rt.height = jsnRectInfoObj->get("height").convert<int>();

					Poco::JSON::Array::Ptr omrList = jsnRectInfoObj->getArray("characterAnchorPointList");
					for (int j = 0; j < omrList->size(); j++)
					{
						Poco::JSON::Object::Ptr jsnOmrObj = omrList->getObject(j);
						pST_CHARACTER_ANCHOR_POINT pstCharacterRt = new ST_CHARACTER_ANCHOR_POINT();;
						RECTINFO rc;
						rc.eCPType = (CPType)jsnOmrObj->get("eType").convert<int>();
						rc.nThresholdValue = jsnOmrObj->get("thresholdValue").convert<int>();
						rc.fStandardValuePercent = jsnOmrObj->get("standardValPercent").convert<float>();
						rc.fStandardValue = jsnOmrObj->get("standardVal").convert<float>();

						if (jsnOmrObj->has("standardArea"))
							rc.fStandardArea = jsnOmrObj->get("standardArea").convert<float>();
						if (jsnOmrObj->has("standardDensity"))
							rc.fStandardDensity = jsnOmrObj->get("standardDensity").convert<float>();
						if (jsnOmrObj->has("standardMeanGray"))
							rc.fStandardMeanGray = jsnOmrObj->get("standardMeanGray").convert<float>();
						if (jsnOmrObj->has("standardStddev"))
							rc.fStandardStddev = jsnOmrObj->get("standardStddev").convert<float>();

						rc.nTH = jsnOmrObj->get("nTH").convert<int>();
						rc.nAnswer = jsnOmrObj->get("nAnswer").convert<int>();
						rc.rt.x = jsnOmrObj->get("left").convert<int>();
						rc.rt.y = jsnOmrObj->get("top").convert<int>();
						rc.rt.width = jsnOmrObj->get("width").convert<int>();
						rc.rt.height = jsnOmrObj->get("height").convert<int>();
						rc.nHItem = jsnOmrObj->get("hHeadItem").convert<int>();
						rc.nVItem = jsnOmrObj->get("vHeadItem").convert<int>();

						if (jsnOmrObj->has("gaussKernel"))
							rc.nGaussKernel = jsnOmrObj->get("gaussKernel").convert<int>();
						if (jsnOmrObj->has("sharpKernel"))
							rc.nSharpKernel = jsnOmrObj->get("sharpKernel").convert<int>();
						if (jsnOmrObj->has("cannyKernel"))
							rc.nCannyKernel = jsnOmrObj->get("cannyKernel").convert<int>();
						if (jsnOmrObj->has("dilateKernel"))
							rc.nDilateKernel = jsnOmrObj->get("dilateKernel").convert<int>();

						//---------------
						pstCharacterRt->nIndex = jsnOmrObj->get("nIndex").convert<int>();
						pstCharacterRt->fConfidence = jsnOmrObj->get("fConfidence").convert<double>();
						pstCharacterRt->strVal = CMyCodeConvert::Utf8ToGb2312(jsnOmrObj->get("strRecogChar").convert<std::string>());
						pstCharacterRt->rc = rc;
						pobjCharacterAnchorArea->vecCharacterRt.push_back(pstCharacterRt);
					}
					paperModelInfo->lCharacterAnchorArea.push_back(pobjCharacterAnchorArea);
				}
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
	case WJ_CP:
		if (eType == WJ_CP || eType == UNKNOWN)
		{
			if (!nFind)
			{
				RECTLIST::iterator it = pPaperModel->lWJ_CP.begin();
				for (int i = 0; it != pPaperModel->lWJ_CP.end(); i++, it++)
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

//快速平方根算法
float SquareRootFloat(float number)
{
	long i;
	float x, y;
	const float f = 1.5F;
	x = number * 0.5F;
	y = number;
	i = *(long *)&y;
	i = 0x5f3759df - (i >> 1); //魔术数        
	y = *(float *)&i;
	y = y * (f - (x * y * y));        //迭代1    1/sqrt(number)    
	y = y * (f - (x * y * y));        //迭代2    1/sqrt(number)   
	//y    = y * ( f - ( x * y * y ) );        //迭代3    1/sqrt(number)，如需要更高的精度请迭代多次   
	return number * y;
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

//三边质心算法
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
//三边定位算法
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
	TRACE("点1(%f,%f),点2(%f,%f)\n", ptNewChk.x, ptNewChk.y, x1, y1);
	//--

	return ptNewChk;
}

inline cv::Point2d TriangleCoordinate(cv::Point ptA, cv::Point ptB, cv::Point ptC, cv::Point ptNewA, cv::Point ptNewB)
{
// 	clock_t start, end;
// 	start = clock();
	long double c02 = pow((ptB.x - ptA.x), 2) + pow((ptB.y - ptA.y), 2);
	long double b02 = pow((ptC.x - ptA.x), 2) + pow((ptC.y - ptA.y), 2);
	long double a02 = pow((ptC.x - ptB.x), 2) + pow((ptC.y - ptB.y), 2);
	long double c2 = pow((ptNewB.x - ptNewA.x), 2) + pow((ptNewB.y - ptNewA.y), 2);

	long double m = sqrt(c2 / c02);	//新三角形边长与原三角形的比例

	long double a2 = pow(m, 2) * a02;
	long double b2 = pow(m, 2) * b02;
	long double dT1 = 2 * b2 * (a2 + c2) - pow(a2 - c2, 2) - pow(b2, 2);
	//++C到AB直线的距离 
	long double A = ptB.y - ptA.y;
	long double B = ptA.x - ptB.x;
	long double C = ptB.x * ptA.y - ptA.x * ptB.y;
	long double dC2AB = abs((A * ptC.x + B * ptC.y + C) / sqrt(pow(A, 2) + pow(B, 2)));		//C到AB直线的距离
	//--


	long double k_ab;		//原AB直线斜率
	long double dDx;			//原C点垂直于AB的D点
	long double dDy;
	long double dFlag;		//标识原C点位于AB的上方还是下方
#if 1	//通过二维向量叉乘判断方向
	cv::Point2f Xab;	//向量AB
	cv::Point2f Xac;	//向量AC
	Xab.x = ptB.x - ptA.x;
	Xab.y = ptB.y - ptA.y;
	Xac.x = ptC.x - ptA.x;
	Xac.y = ptC.y - ptA.y;
	dFlag = Xab.x * Xac.y - Xab.y * Xac.x;	//向量AB * 向量AC的叉乘

// 	cv::Point2f Xac;	//向量AC
// 	cv::Point2f Xbc;	//向量BC
// 	Xac.x = ptC.x - ptA.x;
// 	Xac.y = ptC.y - ptA.y;
// 	Xbc.x = ptC.x - ptA.x;
// 	Xbc.y = ptB.y - ptB.y;
// 	dFlag = Xab.x * Xac.y - Xab.y * Xac.x;	//向量AB * 向量AC的叉乘
// 	cv::Point2f Xab;	//向量AB
// 	cv::Point2f Xac;	//向量AC
// 	Xab.x = ptB.x - ptA.x;
// 	Xab.y = ptB.y - ptB.y;
// 	Xac.x = ptC.x - ptA.x;
// 	Xac.y = ptC.y - ptA.y;
// 	dFlag = Xab.x * Xac.y - Xab.y * Xac.x;	//向量AB * 向量AC的叉乘


// 	//通过D = Ax + By + C判断在直线哪一侧，D<0在直线左侧，D>0在直线右侧，D=0在直线上
// 	long double DptC = A * ptC.x + B * ptC.y + C;
#else
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
#endif

	long double dTmp1 = (ptNewA.x - ptNewB.x) * sqrt(2 * b2 * (a2 + c2) - pow(a2 - c2, 2) - pow(b2, 2)) / (2 * c2);
	long double dTmp2 = (pow(m, 2) * (a02 - b02) * (ptNewB.y - ptNewA.y) - (ptNewA.y + ptNewB.y) * c2) / (2 * c2);

	long double dK1 = (pow(m, 2) * (a02 - b02) + pow(ptNewA.x, 2) - pow(ptNewB.x, 2) + pow(ptNewA.y, 2) - pow(ptNewB.y, 2)) / (2 * (ptNewA.x - ptNewB.x));
	long double dK2 = (long double)(ptNewB.y - ptNewA.y) / (ptNewA.x - ptNewB.x);

	long double dTmp3 = sqrt((pow(dK2, 2) + 1) * pow(m, 2) * a02 - pow(dK2 * ptNewB.y + dK1 - ptNewB.x, 2)) / (pow(dK2, 2) + 1);
	long double dTmp4 = (dK1 * dK2 - ptNewB.x * dK2 - ptNewB.y) / (pow(dK2, 2) + 1);

	long double dYc1;	//新的C点的坐标1
	long double dXc1;
	long double dYc2;	//新的C点的坐标2
	long double dXc2;
	if (ptNewA.x == ptNewB.x)	//此时dK1, dK2为无限大
	{
		dYc1 = dTmp1 - dTmp2;
		dXc1 = ptNewA.x + dC2AB * m;

		dYc2 = -dTmp1 - dTmp2;
		dXc2 = ptNewA.x - dC2AB * m;
	}
	else
	{
		dYc1 = dTmp1 - dTmp2;
		dXc1 = dK1 + dK2 * dYc1;

		dYc2 = -dTmp1 - dTmp2;
		dXc2 = dK1 + dK2 * dYc2;
	}
	cv::Point2d ptNewC;
#if 1
	cv::Point2f Xa1b1;		//向量A1B1
	cv::Point2f Xa1c1;		//向量A1C1
	cv::Point2f Xa1c2;		//向量A1C2
	Xa1b1.x = ptNewB.x - ptNewA.x;
	Xa1b1.y = ptNewB.y - ptNewA.y;
	Xa1c1.x = dXc1 - ptNewA.x;
	Xa1c1.y = dYc1 - ptNewA.y;
	Xa1c2.x = dXc2 - ptNewA.x;
	Xa1c2.y = dYc2 - ptNewA.y;
	long double dNewFlag = Xa1b1.x * Xa1c1.y - Xa1b1.y * Xa1c1.x;	//向量A1B1 * 向量A1C1的叉乘
	long double dNewFlag2 = Xa1b1.x * Xa1c2.y - Xa1b1.y * Xa1c2.x;	//向量A1B1 * 向量A1C2的叉乘

// 	//通过D = Ax + By + C判断在直线哪一侧，D<0在直线左侧，D>0在直线右侧，D=0在直线上
// 	long double A1 = ptNewB.y - ptNewA.y;
// 	long double B1 = ptNewA.x - ptNewB.x;
// 	long double C1 = ptNewB.x * ptNewA.y - ptNewA.x * ptNewB.y;
// 	long double DptC1 = A1 * dXc1 + B1 * dYc1 + C1;
// 	long double DptC2 = A1 * dXc2 + B1 * dYc2 + C1;
#else
	long double k_newAB = (double)(ptNewB.y - ptNewA.y) / (ptNewB.x - ptNewA.x);
	long double dNewFlag = k_newAB*(dXc1 - ptNewA.x) + ptNewA.y - dYc1;
	long double dNewFlag2 = k_newAB*(dXc2 - ptNewA.x) + ptNewA.y - dYc2;
#endif
	if (dFlag >= 0)
	{
		if (dNewFlag >= 0)		//xy坐标要调换，不明白
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
// 	end = clock();
// 	TRACE("新的C点坐标(%f, %f)或者(%f, %f),确定后为(%f,%f)耗时: %d\n", dXc1, dYc1, dXc2, dYc2, ptNewC.x, ptNewC.y, end - start);
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

		if (nPicW != 0 && nPicH != 0)
		{
			int nCenterX = nPicW / 2 + 0.5;
			int nCenterY = nPicH / 2 + 0.5;
			if (rcModelA.rt.x < nCenterX)
			{
				if (rcModelA.rt.y < nCenterY)
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.8;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.8;
					ptA.x = rcA.rt.x + rcA.rt.width * 0.8;
					ptA.y = rcA.rt.y + rcA.rt.height * 0.8;
				}
				else
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.8;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.2;
					ptA.x = rcA.rt.x + rcA.rt.width * 0.8;
					ptA.y = rcA.rt.y + rcA.rt.height * 0.2;
				}
			}
			else
			{
				if (rcModelA.rt.y < nCenterY)
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.2;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.8;
					ptA.x = rcA.rt.x + rcA.rt.width * 0.2;
					ptA.y = rcA.rt.y + rcA.rt.height * 0.8;
				}
				else
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.2;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.2;
					ptA.x = rcA.rt.x + rcA.rt.width * 0.2;
					ptA.y = rcA.rt.y + rcA.rt.height * 0.2;
				}
			}

			if (rcModelB.rt.x < nCenterX)
			{
				if (rcModelB.rt.y < nCenterY)
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.8;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.8;
					ptB.x = rcB.rt.x + rcB.rt.width * 0.8;
					ptB.y = rcB.rt.y + rcB.rt.height * 0.8;
				}
				else
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.8;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.2;
					ptB.x = rcB.rt.x + rcB.rt.width * 0.8;
					ptB.y = rcB.rt.y + rcB.rt.height * 0.2;
				}
			}
			else
			{
				if (rcModelB.rt.y < nCenterY)
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.2;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.8;
					ptB.x = rcB.rt.x + rcB.rt.width * 0.2;
					ptB.y = rcB.rt.y + rcB.rt.height * 0.8;
				}
				else
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.2;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.2;
					ptB.x = rcB.rt.x + rcB.rt.width * 0.2;
					ptB.y = rcB.rt.y + rcB.rt.height * 0.2;
				}
			}
		}
		else
		{
			ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.8;
			ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.8;
			ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.8;
			ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.8;

			ptA.x = rcA.rt.x + rcA.rt.width * 0.8;
			ptA.y = rcA.rt.y + rcA.rt.height * 0.8;
			ptB.x = rcB.rt.x + rcB.rt.width * 0.8;
			ptB.y = rcB.rt.y + rcB.rt.height * 0.8;
		}
		ptC0.x = rt.x;
		ptC0.y = rt.y;
		
		ptC = TriangleCoordinate(ptA0, ptB0, ptC0, ptA, ptB);
		rt.x = ptC.x;
		rt.y = ptC.y;

		//右下的点也计算
// 		cv::Point ptC1;
// 		ptC1.x = ptC0.x + rt.width;
// 		ptC1.y = ptC0.y + rt.height;
// 		ptC = TriangleCoordinate(ptA0, ptB0, ptC1, ptA, ptB);
// 		rt.width = ptC1.x - rt.x;
// 		rt.height = ptC1.y - rt.y;

//		TRACE("定点1(%d, %d), 定点2(%d, %d),新的C点(%d, %d), C点(%d, %d), 原定点1(%d, %d), 定点2(%d, %d)\n", ptA.x, ptA.y, ptB.x, ptB.y, ptC.x, ptC.y, ptC0.x, ptC0.y, ptA0.x, ptA0.y, ptB0.x, ptB0.y);
	}
	else if (lModelFix.size() == 3)
	{
		if (lFix.size() < 3)
		{
			if (lFix.size() > 0)
			{
				RECTLIST lModelTmp;
				RECTLIST::iterator itFix = lFix.begin();
				for (int i = 0; itFix != lFix.end(); i++, itFix++)
				{
					RECTLIST::iterator itModel = lModelFix.begin();
					for (int j = 0; itModel != lModelFix.end(); j++, itModel++)
					{
						if (j == itFix->nTH)
						{
							lModelTmp.push_back(*itModel);
							break;
						}
					}
				}
				return GetPosition(lFix, lModelTmp, rt, nPicW, nPicH);
			}
			else
				return false;
		}
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
			std::string strLog = "质心计算失败，没有交点";
		}
		TRACE("三边质心算法: chk(%f,%f),ptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f),最终质心(%f,%f)备用点(%f,%f)\n", ptChk.x, ptChk.y,\
			  ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult.x, ptResult.y, x1, y1);
	#endif
		rt.x = ptResult.x;
		rt.y = ptResult.y;
#endif
	}
	else if (lModelFix.size() == 4)
	{
		if (lFix.size() < 4)
		{
			if (lFix.size() > 0)
			{
				RECTLIST lModelTmp;
				RECTLIST::iterator itFix = lFix.begin();
				for (int i = 0; itFix != lFix.end(); i++, itFix++)
				{
					RECTLIST::iterator itModel = lModelFix.begin();
					for (int j = 0; itModel != lModelFix.end(); j++, itModel++)
					{
						if (j == itFix->nTH)
						{
							lModelTmp.push_back(*itModel);
							break;
						}
					}
				}
				return GetPosition(lFix, lModelTmp, rt, nPicW, nPicH);
			}
			else
				return false;
		}
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
			std::string strLog = "质心计算失败，没有交点";
		}
#endif
		TRACE("三边定位算法: ptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f), ptResult4(%f,%f)最终质心(%f,%f)\n", \
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
#if 0	//根据最近的3个点来求，这3个点中，取经过最近点的2条直线，如ABC是最近的3个点，其中A点最近，则区AB、AC计算的点
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
		TRACE("参考(%f,%f), ptResultT1((%f,%f), ptResultT1((%f,%f)\n", dSumX / nCount, dSumY / nCount, ptResultT1.x, ptResultT1.y, ptResultT2.x, ptResultT2.y);
#endif


		if (nCount > 0)
		{
			ptResult.x = dSumX / nCount;
			ptResult.y = dSumY / nCount;
		}
		else
		{
			std::string strLog = "质心计算失败，没有交点";
		}
		TRACE("三边质心算法: chk(%f,%f), da=%f,db=%f,dc=%f,dd=%f,\nptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f),ptResult4(%f,%f),ptResult5(%f,%f),ptResult6(%f,%f),最终质心(%f,%f)\n", ptChk.x, ptChk.y,\
			  da,db,dc,dd,ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult4.x, ptResult4.y, ptResult5.x, ptResult5.y, ptResult6.x, ptResult6.y, ptResult.x, ptResult.y);
#endif
		rt.x = ptResult.x;
		rt.y = ptResult.y;
#endif
	}
	return true;
}

bool GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, cv::Rect& rt)
{
#ifdef USE_TESSERACT
	//生成模板比较的定点
	if (pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() > 0)
	{
		if (pPic->lFix.size() < 3)
		{
			clock_t start, end;
			start = clock();
			cv::Rect rtLT, rtRB;	//左上，右下两个矩形
			rtLT = rt;
			rtRB = rt;
			rtRB.x += rtRB.width;
			rtRB.y += rtRB.height;
			GetPosition(pPic->lFix, pPic->lModelWordFix, rtLT);
			GetPosition(pPic->lFix, pPic->lModelWordFix, rtRB);

			int nWidth = abs(rtRB.x - rtLT.x);
			int nHeight = abs(rtRB.y - rtLT.y);
			rt.x = rtLT.x + nWidth / 2 - rt.width / 2;
			rt.y = rtLT.y + nHeight / 2 - rt.height / 2;

			end = clock();
			TRACE("计算矩形位置时间: %dms\n", (int)(end - start));
			return true;
		}
		else
		{
			clock_t start, end;
			start = clock();
			VEC_NEWRTBY2FIX vecNewRt;
#if 1		//根据距离顶点最远的点计算矩形位置，顶点默认防止队列第一个
			RECTLIST::iterator itFix = pPic->lFix.begin();
			RECTLIST::iterator itModelFix = pPic->lModelWordFix.begin();
			if (itModelFix != pPic->lModelWordFix.end())
			{
				itFix++;
				itModelFix++;
				for (int i = 1; itFix != pPic->lFix.end(); itFix++, itModelFix++, i++)
				{
					RECTLIST lTmpFix, lTmpModelFix;
					lTmpFix.push_back(pPic->lFix.front());
					lTmpModelFix.push_back(pPic->lModelWordFix.front());

					lTmpFix.push_back(*itFix);
					lTmpModelFix.push_back(*itModelFix);

					ST_NEWRTBY2FIX stNewRt;
					stNewRt.nFirstFix = 0;
					stNewRt.nSecondFix = i;
					stNewRt.rt = rt;
					GetPosition(lTmpFix, lTmpModelFix, stNewRt.rt);
					vecNewRt.push_back(stNewRt);
				}
			}
#else
			VEC_FIXRECTINFO lFixRtInfo;
			RECTLIST::iterator itFix = pPic->lFix.begin();
			RECTLIST::iterator itModelFix = pPic->lModelWordFix.begin();
			for(; itFix != pPic->lFix.end(); itFix++, itModelFix++)
			{
				GetNewRt((*itFix), (*itModelFix), lFixRtInfo, vecNewRt, rt);
			}
#endif
			int nRidus = rt.width < rt.height ? rt.width * 0.5 : rt.height * 0.5;
			nRidus = nRidus > 3 ? 3 : nRidus;
			VEC_POINTDISTWEIGHT vecPointDistWeight;
			for (auto newRt : vecNewRt)
			{
				GetPointDistWeight(nRidus, newRt.rt.tl(), vecPointDistWeight);
			}
			VEC_POINTDISTWEIGHT::iterator itPoint = vecPointDistWeight.begin();
			for (; itPoint != vecPointDistWeight.end(); )
			{
				if (itPoint->nWeight < 1)
					itPoint = vecPointDistWeight.erase(itPoint);
				else
					itPoint++;
			}

			int nXCount = 0, nYCount = 0;
			int nCount = 0;
			if (vecPointDistWeight.size() > 0)
			{
				for (auto newPt : vecPointDistWeight)
				{
					nXCount += newPt.pt.x;
					nYCount += newPt.pt.y;
				}
				nCount = vecPointDistWeight.size();
			}
			else
			{
				for (auto newRt : vecNewRt)
				{
					nXCount += newRt.rt.x;
					nYCount += newRt.rt.y;
				}
				nCount = vecNewRt.size();
			}
			if (nCount > 0)
			{
				rt.x = nXCount / nCount;
				rt.y = nYCount / nCount;
			}
			end = clock();
//			TRACE("计算矩形位置时间: %dms\n", (int)(end - start));
			return true;
		}
	}
	else
		return GetPosition(pPic->lFix, pModel->vecPaperModel[nPic]->lFix, rt);
#endif
	return GetPosition(pPic->lFix, pModel->vecPaperModel[nPic]->lFix, rt);
}

bool GetPointDistWeight(int nRidus, cv::Point pt, VEC_POINTDISTWEIGHT& vecPointDistWeight)
{
	ST_POINTDISTWEIGHT stPtDistWeight;
	stPtDistWeight.pt = pt;

	VEC_POINTDISTWEIGHT::iterator itPoint = vecPointDistWeight.begin();
	for (int i = 0; itPoint != vecPointDistWeight.end(); itPoint++, i++)
	{
		double distance;
		distance = powf((pt.x - itPoint->pt.x), 2) + powf((pt.y - itPoint->pt.y), 2);
		distance = sqrtf(distance);
		if (distance <= nRidus)
		{
			itPoint->nWeight += 2;
			stPtDistWeight.nWeight += 2;
		}
		else if (distance < 2 * nRidus)
		{
			itPoint->nWeight += 1;
			stPtDistWeight.nWeight += 1;
		}
	}
	vecPointDistWeight.push_back(stPtDistWeight);

	return true;
}

bool GetFixDist(int nPic, pST_PicInfo pPic, pMODEL pModel)
{
	std::vector<pST_CHARACTER_ANCHOR_POINT> vecTmpAnchorPoint;
	for (auto itAnchorArea : pPic->lCharacterAnchorArea)
		for(auto itAnchorPoint : itAnchorArea->vecCharacterRt)
		vecTmpAnchorPoint.push_back(itAnchorPoint);

// 	std::sort(vecTmpAnchorPoint.begin(), vecTmpAnchorPoint.end(), [](ST_CHARACTER_ANCHOR_POINT& st1, ST_CHARACTER_ANCHOR_POINT&st2)
// 	{
// 		
// 	});
	return true;
}

bool GetNewRt(RECTINFO rc, RECTINFO rcModel, VEC_FIXRECTINFO& lFixRtInfo, VEC_NEWRTBY2FIX& vecNewRt, cv::Rect rt)
{
//	if (lFixRtInfo.size() < 1) return false;

	VEC_FIXRECTINFO::iterator itFixRt = lFixRtInfo.begin();
	for (int i = 0; itFixRt != lFixRtInfo.end(); itFixRt++, i++)
	{
		RECTLIST lTmpFix, lTmpModelFix;
		lTmpFix.push_back(itFixRt->rcFix);
		lTmpModelFix.push_back(itFixRt->rcModelFix);

		lTmpFix.push_back(rc);
		lTmpModelFix.push_back(rcModel);

		ST_NEWRTBY2FIX stNewRt;
		stNewRt.nFirstFix = i;
		stNewRt.nSecondFix = lFixRtInfo.size();
		stNewRt.rt = rt;
	#if 1
		GetPosition(lTmpFix, lTmpModelFix, stNewRt.rt);
	#else
		cv::Rect rtLT, rtRB;	//左上，右下两个矩形
		rtLT = rt;
		rtRB = rt;
		rtRB.x += rtRB.width;
		rtRB.y += rtRB.height;
		GetPosition(lTmpFix, lTmpModelFix, rtLT);
		GetPosition(lTmpFix, lTmpModelFix, rtRB);

		int nWidth = abs(rtRB.x - rtLT.x);
		int nHeight = abs(rtRB.y - rtLT.y);
		stNewRt.rt.x = rtLT.x + nWidth / 2 - rt.width / 2;
		stNewRt.rt.y = rtLT.y + nHeight / 2 - rt.height / 2;
	#endif
		vecNewRt.push_back(stNewRt);
	}

	ST_FIXRECTTINFO stFixRtInfo;
	stFixRtInfo.rcFix = rc;
	stFixRtInfo.rcModelFix = rcModel;
	lFixRtInfo.push_back(stFixRtInfo);
	return true;
}

//-----------------------------------------
#if 0
#define  CIRCLE_RADIANS  6.283185307179586476925286766559

//  Determines the radian angle of the specified point (as it relates to the origin).
//
//  Warning:  Do not pass zero in both parameters, as this will cause division-by-zero.
double angleOf(double x, double y) 
{

	double  dist = sqrt(x*x + y*y);

	if (y >= 0.) return acos(x / dist);
	else       return acos(-x / dist) + .5*CIRCLE_RADIANS;
}

//  Pass in a set of 2D points in x,y,points.  Returns a polygon in polyX,polyY,polyCorners.
//
//  To be safe, polyX and polyY should have enough space to store all the points passed in x,y,points.
void findSmallestPolygon(double *x, double *y, long points, double *polyX, double *polyY, long *polyCorners) 
{
	double  newX = x[0], newY = y[0], xDif, yDif, oldAngle = .5*CIRCLE_RADIANS, newAngle, angleDif, minAngleDif;
	long    i;

	//  Find a starting point.
	for (i = 0; i < points; i++)
		if (y[i] > newY || y[i] == newY && x[i] < newX)
		{
			newX = x[i]; newY = y[i];
		}
	*polyCorners = 0;

	//  Polygon-construction loop.
	while (!(*polyCorners) || newX != polyX[0] || newY != polyY[0]) 
	{
		polyX[*polyCorners] = newX;
		polyY[*polyCorners] = newY;
		minAngleDif = CIRCLE_RADIANS;
		for (i = 0; i < points; i++) 
		{
			xDif = x[i] - polyX[*polyCorners];
			yDif = y[i] - polyY[*polyCorners];
			if (xDif || yDif) 
			{
				newAngle = angleOf(xDif, yDif); 
				angleDif = oldAngle - newAngle;
				while (angleDif < 0.) angleDif += CIRCLE_RADIANS;
				while (angleDif >= CIRCLE_RADIANS) angleDif -= CIRCLE_RADIANS;
				if (angleDif < minAngleDif) 
				{
					minAngleDif = angleDif; newX = x[i]; newY = y[i];
				}
			}
		}
		(*polyCorners)++; 
		oldAngle += .5*CIRCLE_RADIANS - minAngleDif;
	}
}
#endif
//-----------------------------------------

bool GetPicFix(int nPic, pST_PicInfo pPic, pMODEL pModel)
{
	int nModelCharArea = pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size();	//模板上的文字定位区
	int nRealRecogCharArea = pPic->lCharacterAnchorArea.size();	//实际识别出的文字定位区

	if (nModelCharArea <= 0 || nRealRecogCharArea <= 0) return false;

	pPic->lFix.clear();
	int nNeedCount = pModel->nCharacterAnchorPoint;	//需要取的文字定点个数

	//2个定点时，选距离最远的两个，如何根据给成矩形左上点和右下点分别计算2个矩形并计算重合度，根据重合度最高的矩形的中心点作为结果矩形的中心点

//	nNeedCount = nNeedCount > 4 ? 4 : nNeedCount;	//定点不超过4个
#if 1
	//查找一个顶点，它的Y最大，X最小
	pST_CHARACTER_ANCHOR_POINT ptPeak = pPic->lCharacterAnchorArea.front()->vecCharacterRt[0];
	for (auto itArea : pPic->lCharacterAnchorArea)
		for (auto itPoint : itArea->vecCharacterRt)
			if ((itPoint->rc.rt.y > ptPeak->rc.rt.y) || (itPoint->rc.rt.y == ptPeak->rc.rt.y && itPoint->rc.rt.x < ptPeak->rc.rt.x))
				ptPeak = itPoint;

	pPic->lFix.push_back(ptPeak->rc);
	for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
		if (itModelCharAnchorArea->nIndex == ptPeak->rc.nTH)
		{
			for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
			{
				if (itModelCharAnchorPoint->strVal == ptPeak->strVal)
				{
					pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
					break;
				}
			}
			break;
		}

	//遍历所有点，计算距离顶点的距离，将距离最大的点依次作为定点求矩形位置
	VEC_POINTDIST2PEAK vecPeakDist;
	for (auto itArea : pPic->lCharacterAnchorArea)
		for (auto itPoint : itArea->vecCharacterRt)
		{
			if(itPoint == ptPeak) continue;
			ST_POINTDIST2PEAK stPtDist;
			stPtDist.pAnchorPoint = itPoint;
			stPtDist.nDist = sqrt((itPoint->rc.rt.x - ptPeak->rc.rt.x) * (itPoint->rc.rt.x - ptPeak->rc.rt.x) + (itPoint->rc.rt.y - ptPeak->rc.rt.y) * (itPoint->rc.rt.y - ptPeak->rc.rt.y));
			vecPeakDist.push_back(stPtDist);
		}
	std::sort(vecPeakDist.begin(), vecPeakDist.end(), [](ST_POINTDIST2PEAK& s1, ST_POINTDIST2PEAK& s2)
	{
		return s1.nDist > s2.nDist;
	});
	for (int i = 0; i < nNeedCount; i++)
	{
		if(i >= vecPeakDist.size())
			break;
		pPic->lFix.push_back(vecPeakDist[i].pAnchorPoint->rc);
		for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
			if (itModelCharAnchorArea->nIndex == vecPeakDist[i].pAnchorPoint->rc.nTH)
			{
				for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
				{
					if (itModelCharAnchorPoint->strVal == vecPeakDist[i].pAnchorPoint->strVal)
					{
						pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
						break;
					}
				}
				break;
			}
	}
#else
	if (nRealRecogCharArea == 1)	//只识别出一个文字定位区，在这个识别区取 nNeedCount 个准确度最高的字作为定点
	{
		CHARACTER_ANCHOR_AREA_LIST::iterator it = pPic->lCharacterAnchorArea.begin();
		
		nNeedCount = (*it)->vecCharacterRt.size() > nNeedCount ? nNeedCount : (*it)->vecCharacterRt.size();

	#if 1
		if ((*it)->vecCharacterRt.size() <= 2)	//只识别到1个或2个文字时，就选这两个做为文字定位点
		{
			for (int i = 0; i < nNeedCount; i++)
			{
				pPic->lFix.push_back((*it)->vecCharacterRt[i]->rc);

				for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
					if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
					{
						for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
						{
							if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[i]->strVal)
							{
								pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
								break;
							}
						}
						break;
					}
			}
		}
		else	//识别到2个以上文字时，取其中的某几个
		{
			int* pTmpArry = new int[nNeedCount];

			for (int i = 0; i < nNeedCount; i++)
			{
				if (i < 2)
				{
					pPic->lFix.push_back((*it)->vecCharacterRt[(*it)->arryMaxDist[i]]->rc);
					pTmpArry[i] = (*it)->arryMaxDist[i];
				}
				else
				{
					//随机取点
					Poco::Random rnd;
					rnd.seed();
					int n = rnd.next(pPic->lFix.size());

					for (int j = 0; j < i; j++)
					{
						if (pTmpArry[j] == n)
						{
							n++;
							j = 0;
						}
					}
					pTmpArry[i] = n;		//存储定点在列表中的的索引
					pPic->lFix.push_back((*it)->vecCharacterRt[n]->rc);
				}

				for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
				if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
				{
					for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
					{
						if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[pTmpArry[i]]->strVal)	//这里要改
						{
							pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
							break;
						}
					}
					break;
				}
			}
			SAFE_RELEASE_ARRY(pTmpArry);
		}

		//-------------------------------------------------

// 		//只对2个的情况，去首尾两个做定点
// 		pPic->lFix.push_back((*it)->vecCharacterRt[0]->rc);
// 		pPic->lFix.push_back((*it)->vecCharacterRt[(*it)->vecCharacterRt.size() - 1]->rc);
// 
// 		//获取模板上的对应字的定点位置
// 		for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
// 			if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
// 			{
// 				for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
// 				{
// 					if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[0]->strVal)
// 					{
// 						pPic->lModelFix.push_back(itModelCharAnchorPoint->rc);
// 						break;
// 					}
// 				}
// 				for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
// 				{
// 					if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[(*it)->vecCharacterRt.size() - 1]->strVal)
// 					{
// 						pPic->lModelFix.push_back(itModelCharAnchorPoint->rc);
// 						break;
// 					}
// 				}
// 				break;
// 			}
	#else
		//取前两个准确度最高的字做定点, 并放入定点列表
		std::sort((*it)->vecCharacterRt.begin(), (*it)->vecCharacterRt.end(), [](pST_CHARACTER_ANCHOR_POINT& st1, pST_CHARACTER_ANCHOR_POINT& st2)
		{
			return st1->fConfidence > st2->fConfidence;
		});
		for (int i = 0; i < nNeedCount; i++)
			pPic->lFix.push_back((*it)->vecCharacterRt[i]->rc);

		//获取模板上的对应字的定点位置
		for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
			if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
			{
				for (int i = 0; i < nNeedCount; i++)
				{
					for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
					{
						if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[i]->strVal)
						{
							pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
							break;
						}
					}
				}
				break;
			}
	#endif
	}
	else if (nRealRecogCharArea < nNeedCount)	//在所有识别区中所有文字识别准确度排序，依次在每个识别区取一个字并循环取，直到达到要求的文字数
	{
		//每个识别区的文字识别准确度排序
		CHARACTER_ANCHOR_AREA_LIST::iterator itAnchorArea = pPic->lCharacterAnchorArea.begin();
		for(; itAnchorArea != pPic->lCharacterAnchorArea.end(); itAnchorArea++)
			std::sort((*itAnchorArea)->vecCharacterRt.begin(), (*itAnchorArea)->vecCharacterRt.end(), SortByCharacterConfidence);
		
		for (int i = 0; i < nNeedCount; i++)
		{
			int nItem = i / nRealRecogCharArea;

			CHARACTER_ANCHOR_AREA_LIST::iterator it = pPic->lCharacterAnchorArea.begin();
			for (int k = 0; k < i - nItem * nRealRecogCharArea; k++)
			{
				it++;
				while((*it)->vecCharacterRt.size() < nItem + 1 )			//*********************		算法有问题	****************************************
					it++;
			}

			pPic->lFix.push_back((*it)->vecCharacterRt[nItem]->rc);

			//获取模板上的对应字的定点位置
			for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
				if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
				{
					for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
					{
						if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[nItem]->strVal)
						{
							pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
							break;
						}
					}
					break;
				}
		}

	}
	else		//识别出多个文字定位区，只在前 nNeedCount 个识别区取1个准确度最高的字作为定点
	{
		CHARACTER_ANCHOR_AREA_LIST::iterator it = pPic->lCharacterAnchorArea.begin();
		for (int i = 0; it != pPic->lCharacterAnchorArea.end(); it++, i++)
		{
			if (i > nNeedCount - 1) break;

			if ((*it)->vecCharacterRt.size() >= 2)
				std::sort((*it)->vecCharacterRt.begin(), (*it)->vecCharacterRt.end(), SortByCharacterConfidence);
			//取准确度最高的点做定点, 并放入定点列表
			pPic->lFix.push_back((*it)->vecCharacterRt[0]->rc);

			//获取模板上的对应字的定点位置
			for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
				if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
				{
					for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
					{
						if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[0]->strVal)
						{
							pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
							break;
						}
					}
					break;
				}
		}
	}
#endif
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
// 	sprintf_s(szTmpLog, "时间1: %d\n", end - start);
// 	TRACE(szTmpLog);

	cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat comImg;
	//Merge into a double-channel image
	cv::merge(planes, 2, comImg);


// 	end = clock();
// 	sprintf_s(szTmpLog, "时间2-0: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Use the same image as input and output,
	//so that the results can fit in Mat well
	cv::dft(comImg, comImg);


// 	end = clock();
// 	sprintf_s(szTmpLog, "时间2-1: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Compute the magnitude
	//planes[0]=Re(DFT(I)), planes[1]=Im(DFT(I))
	//magnitude=sqrt(Re^2+Im^2)
	split(comImg, planes);
	cv::magnitude(planes[0], planes[1], planes[0]);

// 	end = clock();
// 	sprintf_s(szTmpLog, "时间2-2: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Switch to logarithmic scale, for better visual results
	//M2=log(1+M1)
	cv::Mat magMat = planes[0];
	magMat += cv::Scalar::all(1);
	cv::log(magMat, magMat);

// 	end = clock();
// 	sprintf_s(szTmpLog, "时间3: %d\n", end - start);
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
// 	sprintf_s(szTmpLog, "时间4: %d\n", end - start);
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
// 	sprintf_s(szTmpLog, "时间5: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Find lines with Hough Transformation
	std::vector<cv::Vec2f> lines;
	float pi180 = (float)CV_PI / 180;
	cv::Mat linImg(magImg.size(), CV_8UC3);
	cv::HoughLines(magImg, lines, 1, pi180, 100, 0, 0);


// 	end = clock();
// 	sprintf_s(szTmpLog, "时间6-0: %d\n", end - start);
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
// 	sprintf_s(szTmpLog, "时间7: %d\n", end - start);
// 	TRACE(szTmpLog);

	//Rotate the image to recover
	rotMat = cv::getRotationMatrix2D(center, angelD, 1.0);
	dst = cv::Mat::ones(src.size(), CV_8UC3);
	warpAffine(src, dst, rotMat, src.size(), cv::INTER_LINEAR, 0, cv::Scalar(255, 255, 255));

	end = clock();
	sprintf_s(szTmpLog, "图像纠偏总时间: %d, angelD = %f\n", end - start, angelD);
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
#if 1
	std::vector<cv::Point2f> vecFixPt;
	std::vector<cv::Point2f> vecFixNewPt;
	RECTLIST::iterator itCP2 = lFix.begin();
	for (; itCP2 != lFix.end(); itCP2++)
	{
		cv::Point2f pt;

		pt.x = itCP2->rt.x + itCP2->rt.width / 2;
		pt.y = itCP2->rt.y + itCP2->rt.height / 2;

		vecFixNewPt.push_back(pt);
		//获取该定点属于第几个模板定点
		RECTLIST::iterator itCP = lModelFix.begin();
		for (int i = 0; itCP != lModelFix.end(); i++, itCP++)
		{
			if (i == itCP2->nTH)
			{
				cv::Point2f pt2;

				pt2.x = itCP->rt.x + itCP->rt.width / 2;
				pt2.y = itCP->rt.y + itCP->rt.height / 2;

				vecFixPt.push_back(pt2);
				break;
			}
		}
	}
#else
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
#endif
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

#if 1	//计算逆矩阵，计算相对模板的原坐标
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
	sprintf_s(szTmpLog, "图像变换时间: %d, ptMod1(%.2f,%.2f), ptMod2(%.2f,%.2f), ptMod3(%.2f,%.2f), pt1(%.2f,%.2f), pt2(%.2f,%.2f), pt3(%.2f,%.2f)\n", end - start,\
		vecFixPt[0].x, vecFixPt[0].y, vecFixPt[1].x, vecFixPt[1].y, vecFixPt[2].x, vecFixPt[2].y, vecFixNewPt[0].x, vecFixNewPt[0].y, vecFixNewPt[1].x, vecFixNewPt[1].y, vecFixNewPt[2].x, vecFixNewPt[2].y);
	g_pLogger->information(szTmpLog);
	TRACE(szTmpLog);

	return true;
}

bool FixWarpAffine2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat)
{
	if (lFix.size() < 3)
		return false;

	clock_t start, end;
	start = clock();
	char szTmpLog[400] = { 0 };

	std::vector<cv::Point2f> vecFixPt;
	std::vector<cv::Point2f> vecFixNewPt;
	RECTLIST::iterator itCP2 = lFix.begin();
	for (; itCP2 != lFix.end(); itCP2++)
	{
		cv::Point2f pt;

		pt.x = itCP2->rt.x + itCP2->rt.width / 2;
		pt.y = itCP2->rt.y + itCP2->rt.height / 2;

		vecFixNewPt.push_back(pt);
		//获取该定点属于第几个模板定点
		RECTLIST::iterator itCP = lModelFix.begin();
		for (int i = 0; itCP != lModelFix.end(); i++, itCP++)
		{
			if (i == itCP2->nTH)
			{
				cv::Point2f pt2;

				pt2.x = itCP->rt.x + itCP->rt.width / 2;
				pt2.y = itCP->rt.y + itCP->rt.height / 2;

				vecFixPt.push_back(pt2);
				break;
			}
		}
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
	int nMax = matCompPic.cols > matCompPic.rows ? matCompPic.cols : matCompPic.rows;
	cv::Mat st(nMax, nMax, CV_32FC1);
	cv::warpAffine(matCompPic, matDstPic, warp_mat, st.size(), 1, 0, cv::Scalar(255, 255, 255));

#if 1	//计算逆矩阵，计算相对模板的原坐标
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
	sprintf_s(szTmpLog, "图像变换时间: %d, ptMod1(%.2f,%.2f), ptMod2(%.2f,%.2f), ptMod3(%.2f,%.2f), pt1(%.2f,%.2f), pt2(%.2f,%.2f), pt3(%.2f,%.2f)\n", end - start, \
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
	cv::Mat warp_mat(3, 3, CV_32FC1);		//warp_mat(2, 3, CV_32FC1);
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
	sprintf_s(szTmpLog, "图像变换时间: %d, ptMod1(%.2f,%.2f), ptMod2(%.2f,%.2f), ptMod3(%.2f,%.2f), ptMod4(%.2f,%.2f), pt1(%.2f,%.2f), pt2(%.2f,%.2f), pt3(%.2f,%.2f), pt4(%.2f,%.2f)\n", end - start, \
		vecFixPt[0].x, vecFixPt[0].y, vecFixPt[1].x, vecFixPt[1].y, vecFixPt[2].x, vecFixPt[2].y, vecFixPt[3].x, vecFixPt[3].y,\
		vecFixNewPt[0].x, vecFixNewPt[0].y, vecFixNewPt[1].x, vecFixNewPt[1].y, vecFixNewPt[2].x, vecFixNewPt[2].y, vecFixNewPt[3].x, vecFixNewPt[3].y);
	//g_pLogger->information(szTmpLog);
	TRACE(szTmpLog);

	return true;
}

bool FixwarpPerspective2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat)
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
	int nMax = matCompPic.cols > matCompPic.rows ? matCompPic.cols : matCompPic.rows;
//	cv::Mat st(matCompPic.cols, matCompPic.rows, CV_32FC1);
	cv::Mat st(nMax, nMax, CV_32FC1);
	cv::warpPerspective(matCompPic, matDstPic, warp_mat, st.size(), 1, 0, cv::Scalar(255, 255, 255));

	end = clock();
	sprintf_s(szTmpLog, "图像变换时间: %d, ptMod1(%.2f,%.2f), ptMod2(%.2f,%.2f), ptMod3(%.2f,%.2f), ptMod4(%.2f,%.2f), pt1(%.2f,%.2f), pt2(%.2f,%.2f), pt3(%.2f,%.2f), pt4(%.2f,%.2f)\n", end - start, \
			  vecFixPt[0].x, vecFixPt[0].y, vecFixPt[1].x, vecFixPt[1].y, vecFixPt[2].x, vecFixPt[2].y, vecFixPt[3].x, vecFixPt[3].y, \
			  vecFixNewPt[0].x, vecFixNewPt[0].y, vecFixNewPt[1].x, vecFixNewPt[1].y, vecFixNewPt[2].x, vecFixNewPt[2].y, vecFixNewPt[3].x, vecFixNewPt[3].y);
	g_pLogger->information(szTmpLog);
	TRACE(szTmpLog);

	return true;
}

bool GetFixPicTransfer(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, cv::Mat& inverseMat)
{
#ifdef USE_TESSERACT
	if (pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() > 0)
		return true;	// PicTransfer(nPic, matCompPic, pPic->lFix, pPic->lModelFix, inverseMat);
	else
		return PicTransfer(nPic, matCompPic, pPic->lFix, pModel->vecPaperModel[nPic]->lFix, inverseMat);
#endif
	return PicTransfer(nPic, matCompPic, pPic->lFix, pModel->vecPaperModel[nPic]->lFix, inverseMat);
}

bool PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat)
{
#if 1
	if (lModelFix.size() >= lFix.size())
	{
		if (lFix.size() == 3)
			FixWarpAffine(nPic, matCompPic, lFix, lModelFix, inverseMat);
		else if (lFix.size() == 4)
			FixwarpPerspective(nPic, matCompPic, lFix, lModelFix, inverseMat);
	}
#else
	if (lFix.size() != lModelFix.size())
		return false;

	if (lFix.size() == 3)
		FixWarpAffine(nPic, matCompPic, lFix, lModelFix, inverseMat);
	else if (lFix.size() == 4)
		FixwarpPerspective(nPic, matCompPic, lFix, lModelFix, inverseMat);
#endif
	return true;
}

bool PicTransfer2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat)
{
	if (lModelFix.size() >= lFix.size())
	{
		if (lFix.size() == 3)
			FixWarpAffine2(nPic, matCompPic, matDstPic, lFix, lModelFix, inverseMat);
		else if (lFix.size() == 4)
			FixwarpPerspective2(nPic, matCompPic, matDstPic, lFix, lModelFix, inverseMat);
	}
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
		std::string strLog = "数据加密失败，按原数据操作";
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
		std::string strLog = "数据解密失败，按原数据操作";
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


void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel)
{
	//创建并初始化滤波模板
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = nSharpKernel;		//_nSharpKernel_
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//对图像进行滤波
	cv::filter2D(image, result, image.depth(), kernel);
}

//--------------	加载制卷模板数据	-----------------------------
// bool SortByIndex(RECTPOS& rc1, RECTPOS& rc2)
// {
// 	return rc1.nIndex < rc2.nIndex;
// }
// 
// bool GetHeader(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
// {
// 	Poco::JSON::Object::Ptr objHeader = objTK->getObject("syncHeader");
// 	Poco::JSON::Object::Ptr objAnchorPoint = objTK->getObject("anchorPoint");
// 
// 	int nPaperType = 1;		//纸张类型：A3-1，A4-2
// 	nPaperType = objHeader->get("sheetType").convert<int>();
// 
// 	//同步头
// 	std::vector<RECTPOS> vecHeader_H;
// 	std::vector<RECTPOS> vecHeader_V;
// 	//获取定点中的同步头信息
// 	Poco::JSON::Array::Ptr arryItems = objAnchorPoint->getArray("items");
// 	for (int j = 0; j < arryItems->size(); j++)
// 	{
// 		Poco::JSON::Object::Ptr objItem = arryItems->getObject(j);
// 		int nHor = objItem->get("horIndex").convert<int>();
// 		int nVer = objItem->get("verIndex").convert<int>();
// 		RECTPOS rc;
// 		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
// 		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
// 		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
// 		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
// 		if (nPaperType == 2)	//A4垂直同步头在右边，A3垂直同步头在左边
// 		{
// 			if (nHor == 0 && nVer == 0)
// 			{
// 				rc.nIndex = 0;
// 				vecHeader_H.push_back(rc);
// 			}
// 			else if (nHor != 0 && nVer == 0)
// 			{
// 				rc.nIndex = nHor;
// 				vecHeader_H.push_back(rc);
// 				rc.nIndex = 0;		//交叉定位点，对垂直同步头来说索引是0
// 				vecHeader_V.push_back(rc);
// 			}
// 			else if (nHor != 0 && nVer != 0)
// 			{
// 				rc.nIndex = nVer;
// 				vecHeader_V.push_back(rc);
// 			}
// 		}
// 		else
// 		{
// 			if (nHor == 0 && nVer == 0)
// 			{
// 				rc.nIndex = 0;
// 				vecHeader_H.push_back(rc);
// 				vecHeader_V.push_back(rc);
// 			}
// 			else if (nHor != 0 && nVer == 0)
// 			{
// 				rc.nIndex = nHor;
// 				vecHeader_H.push_back(rc);
// 			}
// 			else if (nHor == 0 && nVer != 0)
// 			{
// 				rc.nIndex = nVer;
// 				vecHeader_V.push_back(rc);
// 			}
// 		}
// 	}
// 	Poco::JSON::Array::Ptr arryHorHeaders = objHeader->getArray("horHeaders");
// 	for (int k = 0; k < arryHorHeaders->size(); k++)
// 	{
// 		Poco::JSON::Object::Ptr objItem = arryHorHeaders->getObject(k);
// 		RECTPOS rc;
// 		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
// 		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
// 		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
// 		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
// 		rc.nIndex = objItem->get("index").convert<int>();
// 		vecHeader_H.push_back(rc);
// 	}
// 	Poco::JSON::Array::Ptr arryVerHeaders = objHeader->getArray("verHeaders");
// 	for (int k = 0; k < arryVerHeaders->size(); k++)
// 	{
// 		Poco::JSON::Object::Ptr objItem = arryVerHeaders->getObject(k);
// 		RECTPOS rc;
// 		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
// 		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
// 		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
// 		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
// 		rc.nIndex = objItem->get("index").convert<int>();
// 		vecHeader_V.push_back(rc);
// 	}
// 	std::sort(vecHeader_H.begin(), vecHeader_H.end(), SortByIndex);
// 	std::sort(vecHeader_V.begin(), vecHeader_V.end(), SortByIndex);
// 
// 	//设置同步头橡皮筋大小
// 	cv::Rect rcTracker_H, rcTracker_V;
// 	cv::Point pt1 = vecHeader_H[0].rt.tl() - cv::Point(50, 50);
// 	cv::Point pt2 = vecHeader_H[vecHeader_H.size() - 1].rt.br() + cv::Point(50, 50);
// 	cv::Point pt3 = vecHeader_V[0].rt.tl() - cv::Point(50, 50);
// 	cv::Point pt4 = vecHeader_V[vecHeader_V.size() - 1].rt.br() + cv::Point(50, 50);
// 	pPaperModel->rtHTracker = cv::Rect(pt1, pt2);
// 	pPaperModel->rtVTracker = cv::Rect(pt3, pt4);
// 
// 	RECTINFO rtSelHTracker, rtSelVTracker;
// 	rtSelHTracker.eCPType = H_HEAD;
// 	rtSelHTracker.rt = pPaperModel->rtHTracker;
// 	rtSelVTracker.eCPType = V_HEAD;
// 	rtSelVTracker.rt = pPaperModel->rtVTracker;
// 	pPaperModel->lSelHTracker.push_back(rtSelHTracker);		//识别时查找同步头
// 	pPaperModel->lSelVTracker.push_back(rtSelVTracker);		//识别时查找同步头
// 
// 	//设置同步头
// 	for (int m = 0; m < vecHeader_H.size(); m++)
// 	{
// 		RECTINFO rc;
// 		rc.eCPType = H_HEAD;
// 		rc.nHItem = vecHeader_H[m].nIndex;
// 		rc.nVItem = 0;
// 		rc.rt = vecHeader_H[m].rt;
// 		pPaperModel->lH_Head.push_back(rc);
// 	}
// 	for (int m = 0; m < vecHeader_V.size(); m++)
// 	{
// 		RECTINFO rc;
// 		rc.eCPType = V_HEAD;
// 		rc.nHItem = 0;
// 		rc.nVItem = vecHeader_V[m].nIndex;
// 		rc.rt = vecHeader_V[m].rt;
// 		pPaperModel->lV_Head.push_back(rc);
// 	}
// 	return true;
// }
// 
// bool GetZkzh(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
// {
// 	cv::Point ptZkzh1, ptZkzh2;
// 	if (!objTK->isNull("baseInfo"))
// 	{
// 		Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
// 		Poco::JSON::Object::Ptr objZKZH = objBaseInfo->getObject("zkzh");
// 		Poco::JSON::Array::Ptr arryZkzhItems = objZKZH->getArray("items");
// 		for (int n = 0; n < arryZkzhItems->size(); n++)
// 		{
// 			Poco::JSON::Object::Ptr objItem = arryZkzhItems->getObject(n);
// 			Poco::JSON::Object::Ptr objPanel = objItem->getObject("panel");
// 			int nItem = objPanel->get("index").convert<int>();
// 			if (nItem == 0)
// 			{
// 				ptZkzh1.x = objPanel->get("x").convert<int>();
// 				ptZkzh1.y = objPanel->get("y").convert<int>();
// 			}
// 			if (nItem == arryZkzhItems->size() - 1)
// 			{
// 				ptZkzh2.x = objPanel->get("x").convert<int>() + objPanel->get("width").convert<int>();
// 				ptZkzh2.y = objPanel->get("y").convert<int>() + objPanel->get("height").convert<int>();
// 			}
// 
// 			//获取每列的准考证信息
// 			pSN_ITEM pSnItem = new SN_ITEM;
// 			pSnItem->nItem = nItem;
// 
// 			Poco::JSON::Array::Ptr arryZkzhGrids = objItem->getArray("grids");
// 			for (int k = 0; k < arryZkzhGrids->size(); k++)
// 			{
// 				Poco::JSON::Object::Ptr objGrids = arryZkzhGrids->getObject(k);
// 				RECTINFO rc;
// 				rc.eCPType = SN;
// 				rc.rt.x = objGrids->get("x").convert<int>();
// 				rc.rt.y = objGrids->get("y").convert<int>();
// 				rc.rt.width = objGrids->get("width").convert<int>();
// 				rc.rt.height = objGrids->get("height").convert<int>();
// 				rc.nTH = nItem;
// 				rc.nSnVal = objGrids->get("index").convert<int>();
// 				rc.nHItem = objGrids->get("horIndex").convert<int>();
// 				rc.nVItem = objGrids->get("verIndex").convert<int>();
// 				rc.nRecogFlag = 10;
// 				pSnItem->lSN.push_back(rc);
// 			}
// 			pPaperModel->lSNInfo.push_back(pSnItem);
// 		}
// 		ptZkzh1 += cv::Point(2, 2);			//准考证橡皮筋缩放，防止选框太大
// 		ptZkzh2 -= cv::Point(2, 2);			//准考证橡皮筋缩放，防止选框太大
// //		pPaperModel->rtSNTracker = cv::Rect(ptZkzh1, ptZkzh2);
// 		pPaperModel->rcSNTracker.rt = cv::Rect(ptZkzh1, ptZkzh2);
// 		pPaperModel->rcSNTracker.nRecogFlag = 10;
// 	}
// 	return true;
// }
// 
// bool GetOMR(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
// {
// 	Poco::JSON::Array::Ptr arryElement = objTK->getArray("elements");
// 	for (int m = 0; m < arryElement->size(); m++)
// 	{
// 		Poco::JSON::Object::Ptr objElement = arryElement->getObject(m);
// 		Poco::JSON::Object::Ptr objItem = objElement->getObject("item");
// 		int nQuestionType = objElement->get("type").convert<int>();
// 		if (nQuestionType == 11)	//11表示选择题
// 		{
// 			Poco::JSON::Array::Ptr arryQuestions = objItem->getArray("questions");
// 			for (int n = 0; n < arryQuestions->size(); n++)
// 			{
// 				Poco::JSON::Object::Ptr objQuestion = arryQuestions->getObject(n);
// 				OMR_QUESTION omrItem;
// 				omrItem.nTH = objQuestion->get("num").convert<int>();
// 				omrItem.nSingle = objQuestion->get("choiceType").convert<int>() - 1;
// 
// 				Poco::JSON::Array::Ptr arryOptions = objQuestion->getArray("options");
// 				for (int k = 0; k < arryOptions->size(); k++)
// 				{
// 					Poco::JSON::Object::Ptr objOptions = arryOptions->getObject(k);
// 					Poco::JSON::Object::Ptr objPanel = objOptions->getObject("panel");
// 					RECTINFO rc;
// 					rc.eCPType = OMR;
// 					rc.rt.x = objPanel->get("x").convert<int>();
// 					rc.rt.y = objPanel->get("y").convert<int>();
// 					rc.rt.width = objPanel->get("width").convert<int>();
// 					rc.rt.height = objPanel->get("height").convert<int>();
// 					rc.nHItem = objPanel->get("horIndex").convert<int>();
// 					rc.nVItem = objPanel->get("verIndex").convert<int>();
// 					rc.nAnswer = (int)objOptions->get("label").convert<char>() - 65;
// 					rc.nTH = omrItem.nTH;
// 					rc.nSingle = omrItem.nSingle;
// 					omrItem.lSelAnswer.push_back(rc);
// 				}
// 				pPaperModel->lOMR2.push_back(omrItem);
// 			}
// 		}		
// 	}
// 	return true;
// }
// 
// bool GetCourse(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
// {
// 	Poco::JSON::Object::Ptr objSubject = objTK->getObject("subject");
// 	Poco::JSON::Array::Ptr arryCourse = objSubject->getArray("items");
// 	for (int k = 0; k < arryCourse->size(); k++)
// 	{
// 		Poco::JSON::Object::Ptr objItem = arryCourse->getObject(k);
// 		RECTINFO rcCourse;
// 		rcCourse.eCPType = COURSE;
// 		rcCourse.rt.x = objItem->get("x").convert<int>();
// 		rcCourse.rt.y = objItem->get("y").convert<int>();
// 		rcCourse.rt.width = objItem->get("width").convert<int>();
// 		rcCourse.rt.height = objItem->get("height").convert<int>();
// 		rcCourse.nHItem = objItem->get("horIndex").convert<int>();
// 		rcCourse.nVItem = objItem->get("verIndex").convert<int>();
// 		pPaperModel->lCourse.push_back(rcCourse);
// 	}
// 	return true;
// }
// 
// bool GetQK(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
// {
// 	if (!objTK->isNull("baseInfo"))
// 	{
// 		Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
// 		Poco::JSON::Object::Ptr objAbsentBreach = objBaseInfo->getObject("absentAndBreach");
// 		Poco::JSON::Object::Ptr objAbsent = objAbsentBreach->getObject("absent");
// 		RECTINFO rcAbsent;
// 		rcAbsent.eCPType = QK_CP;
// 		rcAbsent.rt.x = objAbsent->get("x").convert<int>();
// 		rcAbsent.rt.y = objAbsent->get("y").convert<int>();
// 		rcAbsent.rt.width = objAbsent->get("width").convert<int>();
// 		rcAbsent.rt.height = objAbsent->get("height").convert<int>();
// 		rcAbsent.nHItem = objAbsent->get("horIndex").convert<int>();
// 		rcAbsent.nVItem = objAbsent->get("verIndex").convert<int>();
// 		pPaperModel->lQK_CP.push_back(rcAbsent);
// 	}
// 	return true;
// }

// pMODEL LoadMakePaperData(std::string strData)
// {
// 	pMODEL pModel = new MODEL;
// 
// 	USES_CONVERSION;
// 	Poco::JSON::Parser parser;
// 	Poco::Dynamic::Var result;
// 	try
// 	{
// 		result = parser.parse(strData);		//strJsnData
// 		Poco::JSON::Array::Ptr arryData = result.extract<Poco::JSON::Array::Ptr>();
// 		pModel->nPicNum = arryData->size();
// 		for (int i = 0; i < arryData->size(); i++)
// 		{
// 			Poco::JSON::Object::Ptr objTK = arryData->getObject(i);
// 			Poco::JSON::Object::Ptr objSubject = objTK->getObject("subject");
// 			Poco::JSON::Object::Ptr objPageNum = objTK->getObject("pageNum");
// 
// 			if (i == 0)
// 			{
// 				Poco::JSON::Object::Ptr objCurSubject = objSubject->getObject("curSubject");
// 				pModel->strModelName = CMyCodeConvert::Utf8ToGb2312(objCurSubject->get("name").convert<std::string>());
// 			}			
// 
// 			std::string strName = Poco::format("model%d.jpg", i + 1);
// 
// 			pPAPERMODEL pPaperModel = new PAPERMODEL;
// 			pPaperModel->nPaper = objPageNum->get("curPageNum").convert<int>() - 1;			//add from 0
// 			pPaperModel->strModelPicName = strName;	//图片名称，目前不知道，考虑从PDF直接转图片然后命名			//**********	test	*****************
// 			
//  			//同步头
// 			GetHeader(objTK, pPaperModel);
// 
// 			//准考证号
// 			GetZkzh(objTK, pPaperModel);
// 
// 			//OMR设置
// 			GetOMR(objTK, pPaperModel);
// 
// 			//添加科目点
// 			GetCourse(objTK, pPaperModel);
// 
// 			//添加缺考点
// 			GetQK(objTK, pPaperModel);
// 
// 			//添加灰度点
// 
// 			//添加白校验点
// 
// 			//添加试卷模板到总模板
// 			pModel->vecPaperModel.push_back(pPaperModel);
// 		}
// 		pModel->nType = 1;
// 	}
// 	catch (Poco::JSON::JSONException& jsone)
// 	{
// 		SAFE_RELEASE(pModel);
// 		std::string strErrInfo;
// 		strErrInfo.append("加载制卷模板文件解析json失败: ");
// 		strErrInfo.append(jsone.message());
// 		g_pLogger->information(strErrInfo);
// 	}
// 	catch (Poco::Exception& exc)
// 	{
// 		SAFE_RELEASE(pModel);
// 		std::string strErrInfo;
// 		strErrInfo.append("加载制卷模板文件解析json失败2: ");
// 		strErrInfo.append(exc.message());
// 		g_pLogger->information(strErrInfo);
// 	}
// 
// 	return pModel;
// }
// 
// bool Pdf2Jpg(std::string strPdfPath, std::string strBaseName)
// {
// 	USES_CONVERSION;
// 	Poco::File pdfFile(CMyCodeConvert::Gb2312ToUtf8(strPdfPath));
// 	if (!pdfFile.exists())
// 	{
// 		AfxMessageBox(_T("pdf文件不存在"));
// 		return false;
// 	}
// 
// 	std::string strModelSavePath = g_strModelSavePath + "\\" + CMyCodeConvert::Gb2312ToUtf8(strBaseName);
// 	Poco::File modelFile(strModelSavePath);
// 	if (modelFile.exists())
// 		modelFile.remove(true);
// 
// 	modelFile.createDirectories();
// 
// 	CString strSrcPdfPath = A2T(strPdfPath.c_str());
// 	std::string outPicPath = CMyCodeConvert::Utf8ToGb2312(strModelSavePath + "\\" + "model");
// 	
// 	CMuPDFConvert pdfConvert;
// 	int nNum = 0;
// 	bool bResult = pdfConvert.Pdf2Png(strSrcPdfPath, outPicPath.c_str(), nNum);
// 	return bResult;
// }


//++ test
bool RecogHHead(int nPic, cv::Mat& matCompPic, pPAPERMODEL pPicModel, RECTINFO rc)
{
	bool bResult = true;
	std::vector<RECTINFO> vecH_Head;

	std::string strErrDesc;
	
	std::vector<cv::Rect>RectCompList;
	try
	{
		if (rc.rt.x < 0) rc.rt.x = 0;
		if (rc.rt.y < 0) rc.rt.y = 0;
		if (rc.rt.br().x > matCompPic.cols)
		{
			rc.rt.width = matCompPic.cols - rc.rt.x;
		}
		if (rc.rt.br().y > matCompPic.rows)
		{
			rc.rt.height = matCompPic.rows - rc.rt.y;
		}

		cv::Mat matCompRoi;
		matCompRoi = matCompPic(rc.rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
		SharpenImage(matCompRoi, matCompRoi, _nSharpKernel_);

		int nThreshold = 100;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);

		cv::Canny(matCompRoi, matCompRoi, 0, _nCannyKernel_, 5);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));	//Size(6, 6)	普通空白框可识别
		cv::dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;
			rm.x = rm.x + rc.rt.x;
			rm.y = rm.y + rc.rt.y;

			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "识别水平同步头异常: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());

		bResult = false;
	}
	if (RectCompList.size() == 0)
	{
		bResult = false;
		strErrDesc = "水平同步头数量为0.";
	}
	else
	{
		for (int i = 0; i < RectCompList.size(); i++)
		{
			RECTINFO rcHead;
			rcHead.rt = RectCompList[i];
			rcHead.eCPType = H_HEAD;
			vecH_Head.push_back(rcHead);
		}
		std::sort(vecH_Head.begin(), vecH_Head.end(), SortByPositionX);

		for (int i = 0; i < vecH_Head.size(); i++)
			pPicModel->lH_Head.push_back(vecH_Head[i]);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别水平同步头失败, 原因: %s\n", strErrDesc.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool RecogVHead(int nPic, cv::Mat& matCompPic, pPAPERMODEL pPicModel, RECTINFO rc)
{
	bool bResult = true;
	std::vector<RECTINFO> vecV_Head;

	std::string strErrDesc;
	std::vector<cv::Rect>RectCompList;
	try
	{
		if (rc.rt.x < 0) rc.rt.x = 0;
		if (rc.rt.y < 0) rc.rt.y = 0;
		if (rc.rt.br().x > matCompPic.cols)
		{
			rc.rt.width = matCompPic.cols - rc.rt.x;
		}
		if (rc.rt.br().y > matCompPic.rows)
		{
			rc.rt.height = matCompPic.rows - rc.rt.y;
		}

		cv::Mat matCompRoi;
		matCompRoi = matCompPic(rc.rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
		SharpenImage(matCompRoi, matCompRoi, _nSharpKernel_);

		int nThreshold = 100;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);

		cv::Canny(matCompRoi, matCompRoi, 0, _nCannyKernel_, 5);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));	//Size(6, 6)	普通空白框可识别
		cv::dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		
		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;
			rm.x = rm.x + rc.rt.x;
			rm.y = rm.y + rc.rt.y;

			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "识别垂直同步头异常: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());

		bResult = false;
	}
	if (RectCompList.size() == 0)
	{
		bResult = false;
		strErrDesc = "垂直同步头数量为0.";
	}
	else
	{
		for (int i = 0; i < RectCompList.size(); i++)
		{
			RECTINFO rcHead;
			rcHead.rt = RectCompList[i];
			rcHead.eCPType = V_HEAD;
			vecV_Head.push_back(rcHead);
		}
		std::sort(vecV_Head.begin(), vecV_Head.end(), SortByPositionY);

		for (int i = 0; i < vecV_Head.size(); i++)
			pPicModel->lV_Head.push_back(vecV_Head[i]);
	}

	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别垂直同步头失败, 原因: %s\n", strErrDesc.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool GetPositionByHead(pPAPERMODEL pPicModel, int nH, int nV, cv::Rect& rt)
{
	RECTLIST::iterator itItemH = pPicModel->lH_Head.begin();
	for (int i = 0; itItemH != pPicModel->lH_Head.end(); itItemH++, i++)
	{
		if (i == nH)
			break;
	}
	RECTLIST::iterator itItemV = pPicModel->lV_Head.begin();
	for (int i = 0; itItemV != pPicModel->lV_Head.end(); itItemV++, i++)
	{
		if (i == nV)
			break;
	}
	rt.x = itItemH->rt.x;
	rt.y = itItemV->rt.y;
	rt.width = itItemH->rt.width;
	rt.height = itItemV->rt.height;
	return true;
}
//--

inline bool RecogGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc)
{
	cv::cvtColor(matSrcRoi, matSrcRoi, CV_BGR2GRAY);
	cv::GaussianBlur(matSrcRoi, matSrcRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
	SharpenImage(matSrcRoi, matSrcRoi, _nSharpKernel_);

	const int channels[1] = { 0 };
	const float* ranges[1];
	const int histSize[1] = { 1 };
	float hranges[2];
	if (rc.eCPType != WHITE_CP)
	{
		hranges[0] = g_nRecogGrayMin;
		hranges[1] = static_cast<float>(rc.nThresholdValue);
		ranges[0] = hranges;
	}
	else
	{
		hranges[0] = static_cast<float>(rc.nThresholdValue);
		hranges[1] = g_nRecogGrayMax_White;	//255			//256时可统计完全空白的点，即RGB值为255的完全空白点;255时只能统计到RGB为254的值，255的值统计不到
		ranges[0] = hranges;
	}
	cv::MatND src_hist;
	cv::calcHist(&matSrcRoi, 1, channels, cv::Mat(), src_hist, 1, histSize, ranges, false);

	rc.fStandardValue = src_hist.at<float>(0);
	return true;
}

bool InitModelRecog(pMODEL pModel)
{
	bool bResult = false;
	USES_CONVERSION;
	for(int i = 0; i < pModel->vecPaperModel.size(); i++)
	{
		std::string strModelPicPath = g_strModelSavePath + "\\" + pModel->strModelName + "\\" + pModel->vecPaperModel[i]->strModelPicName;

		cv::Mat matSrc = cv::imread(strModelPicPath);

		pPAPERMODEL pPicModel = pModel->vecPaperModel[i];

		//++ test	识别模板的同步头大小
		RECTLIST::iterator itBegin = pPicModel->lH_Head.begin();
		RECTINFO rcFist = *itBegin;
		RECTLIST::reverse_iterator itLast = pPicModel->lH_Head.rbegin();
		RECTINFO rcLast = *(itLast);
		RECTINFO rcH_Tracker;
		cv::Point pt1, pt2;
		pt1 = rcFist.rt.tl() - cv::Point(15, 15);
		pt2 = rcLast.rt.br() + cv::Point(15, 15);
		rcH_Tracker.rt = cv::Rect(pt1, pt2);
		pPicModel->lH_Head.clear();
		RecogHHead(i, matSrc, pPicModel, rcH_Tracker);

		itBegin = pPicModel->lV_Head.begin();
		itLast = pPicModel->lV_Head.rbegin();
		rcFist = *itBegin;
		rcLast = *itLast;
		RECTINFO rcV_Tracker;
		pt1 = rcFist.rt.tl() - cv::Point(15, 15);
		pt2 = rcLast.rt.br() + cv::Point(15, 15);
		rcV_Tracker.rt = cv::Rect(pt1, pt2);
		pPicModel->lV_Head.clear();
		RecogVHead(i, matSrc, pPicModel, rcV_Tracker);
		//--

		RECTLIST::iterator itHead_H = pPicModel->lH_Head.begin();
		for (; itHead_H != pPicModel->lH_Head.end(); itHead_H++)
		{
			itHead_H->nThresholdValue = 150;
			itHead_H->fStandardValuePercent = 0.75;

			cv::Mat matComp = matSrc(itHead_H->rt);
			RecogGrayValue(matComp, *itHead_H);
		}
		RECTLIST::iterator itHead_V = pPicModel->lV_Head.begin();
		for (; itHead_V != pPicModel->lV_Head.end(); itHead_V++)
		{
			itHead_V->nThresholdValue = 150;
			itHead_V->fStandardValuePercent = 0.75;

			cv::Mat matComp = matSrc(itHead_V->rt);
			RecogGrayValue(matComp, *itHead_V);
		}
		RECTLIST::iterator itABModel = pPicModel->lABModel.begin();
		for (; itABModel != pPicModel->lABModel.end(); itABModel++)
		{
			itABModel->nThresholdValue = 150;
			itABModel->fStandardValuePercent = 0.75;

			GetPositionByHead(pPicModel, itABModel->nHItem, itABModel->nVItem, itABModel->rt);

			cv::Mat matComp = matSrc(itABModel->rt);
			RecogGrayValue(matComp, *itABModel);
		}
		RECTLIST::iterator itCourse = pPicModel->lCourse.begin();
		for (; itCourse != pPicModel->lCourse.end(); itCourse++)
		{
			itCourse->nThresholdValue = 150;
			itCourse->fStandardValuePercent = 0.75;
			
			GetPositionByHead(pPicModel, itCourse->nHItem, itCourse->nVItem, itCourse->rt);

			cv::Mat matComp = matSrc(itCourse->rt);
			RecogGrayValue(matComp, *itCourse);
		}
		RECTLIST::iterator itQK = pPicModel->lQK_CP.begin();
		for (; itQK != pPicModel->lQK_CP.end(); itQK++)
		{
			itQK->nThresholdValue = 150;
			itQK->fStandardValuePercent = 0.75;

			GetPositionByHead(pPicModel, itQK->nHItem, itQK->nVItem, itQK->rt);

			cv::Mat matComp = matSrc(itQK->rt);
			RecogGrayValue(matComp, *itQK);
		}
		RECTLIST::iterator itWJ = pPicModel->lWJ_CP.begin();
		for (; itWJ != pPicModel->lWJ_CP.end(); itWJ++)
		{
			itWJ->nThresholdValue = 150;
			itWJ->fStandardValuePercent = 0.75;

			GetPositionByHead(pPicModel, itWJ->nHItem, itWJ->nVItem, itWJ->rt);

			cv::Mat matComp = matSrc(itWJ->rt);
			RecogGrayValue(matComp, *itWJ);
		}
		RECTLIST::iterator itGray = pPicModel->lGray.begin();
		for (; itGray != pPicModel->lGray.end(); itGray++)
		{
			itGray->nThresholdValue = 150;
			itGray->fStandardValuePercent = 0.75;

			GetPositionByHead(pPicModel, itGray->nHItem, itGray->nVItem, itGray->rt);

			cv::Mat matComp = matSrc(itGray->rt);
			RecogGrayValue(matComp, *itGray);
		}
		RECTLIST::iterator itWhite = pPicModel->lWhite.begin();
		for (; itWhite != pPicModel->lWhite.end(); itWhite++)
		{
			itWhite->nThresholdValue = 225;
			itWhite->fStandardValuePercent = 0.75;

			GetPositionByHead(pPicModel, itWhite->nHItem, itWhite->nVItem, itWhite->rt);

			cv::Mat matComp = matSrc(itWhite->rt);
			RecogGrayValue(matComp, *itWhite);
		}
		SNLIST::iterator itSN = pPicModel->lSNInfo.begin();
		for (; itSN != pPicModel->lSNInfo.end(); itSN++)
		{
			pSN_ITEM pSNItem = *itSN;
			RECTLIST::iterator itSNItem = pSNItem->lSN.begin();
			for (; itSNItem != pSNItem->lSN.end(); itSNItem++)
			{
				itSNItem->nThresholdValue = 200;
				itSNItem->fStandardValuePercent = 1.1;

				GetPositionByHead(pPicModel, itSNItem->nHItem, itSNItem->nVItem, itSNItem->rt);

				cv::Mat matComp = matSrc(itSNItem->rt);
				RecogGrayValue(matComp, *itSNItem);
			}
		}
		OMRLIST::iterator itOmr = pPicModel->lOMR2.begin();
		for (; itOmr != pPicModel->lOMR2.end(); itOmr++)
		{
			RECTLIST::iterator itOmrItem = itOmr->lSelAnswer.begin();
			for (; itOmrItem != itOmr->lSelAnswer.end(); itOmrItem++)
			{
				itOmrItem->nThresholdValue = g_RecogGrayMax_OMR;
				itOmrItem->fStandardValuePercent = 1.1;

				GetPositionByHead(pPicModel, itOmrItem->nHItem, itOmrItem->nVItem, itOmrItem->rt);

				cv::Mat matComp = matSrc(itOmrItem->rt);
				RecogGrayValue(matComp, *itOmrItem);
			}
		}
	}
	return bResult;
}
//---------------------------------------------------------------




//================ 条码 =============================
//zbar接口
std::string ZbarDecoder(cv::Mat img, std::string& strTypeName)
{
	std::string result;
	zbar::ImageScanner scanner;
	const void *raw = (&img)->data;
	// configure the reader
	scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);
	// wrap image data
	zbar::Image image(img.cols, img.rows, "Y800", raw, img.cols * img.rows);
	// scan the image for barcodes
	int n = scanner.scan(image);
	// extract results
	result = image.symbol_begin()->get_data();
	strTypeName = image.symbol_begin()->get_type_name();
	image.set_data(NULL, 0);
	return result;
}

//对二值图像进行识别，如果失败则开运算进行二次识别
std::string GetQRInBinImg(cv::Mat binImg, std::string& strTypeName)
{
	std::string result = ZbarDecoder(binImg, strTypeName);
	if (result.empty())
	{
		cv::Mat openImg;
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
		cv::morphologyEx(binImg, openImg, cv::MORPH_OPEN, element);
		result = ZbarDecoder(openImg, strTypeName);
	}
	return result;
}

//main function
std::string GetQR(cv::Mat img, std::string& strTypeName)
{
	std::string result;
	result = GetQRInBinImg(img, strTypeName);
 	if (!result.empty())
 		return result;

	cv::Mat binImg;
	//在otsu二值结果的基础上，不断增加阈值，用于识别模糊图像
	int thre = threshold(img, binImg, 0, 255, cv::THRESH_OTSU);
	while (result.empty() && thre < 255)
	{
		threshold(img, binImg, thre, 255, cv::THRESH_BINARY);
		result = GetQRInBinImg(binImg, strTypeName);
		thre += 20;//阈值步长设为20，步长越大，识别率越低，速度越快
	}
	return result;
}
//===================================================


BOOL CheckProcessExist(CString &str, int& nProcessID)
{
	BOOL bResult;
	CString strTemp, strProcessName;
	HANDLE hSnapshot;               //内存进程的“快照”句柄      
	PROCESSENTRY32 ProcessEntry;    //描述进程的结构   
	//输入要结束的进程名称    
	strProcessName = str/*m_strProcessName*/;
	strProcessName.MakeLower();
	//返回内存所有进程的快照。参数为TH32CS_SNAPPROCESS取有的进程,忽略参数2；    
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	//获取要的进程名称对应的所有进程ID    
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	bResult = Process32First(hSnapshot, &ProcessEntry);//获取第一个进程 
	int processcount = 0;
	while (bResult)
	{
		//判断是否为要结束的进程    
		strTemp.Format(_T("%s"), ProcessEntry.szExeFile);
		strTemp.MakeLower();
		if (strTemp == strProcessName)
		{
			nProcessID = ProcessEntry.th32ProcessID;
			processcount++;
			return TRUE;
		}
		//获取下一个进程    
		bResult = Process32Next(hSnapshot, &ProcessEntry);
	}
	return FALSE;
}

//kill进程from名字
BOOL KillProcessFromName(CString strProcessName)
{
	//创建进程快照(TH32CS_SNAPPROCESS表示创建所有进程的快照)  
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	//PROCESSENTRY32进程快照的结构体  
	PROCESSENTRY32 pe;

	//实例化后使用Process32First获取第一个快照的进程前必做的初始化操作  
	pe.dwSize = sizeof(PROCESSENTRY32);

	//下面的IF效果同:  
	//if(hProcessSnap == INVALID_HANDLE_VALUE)   无效的句柄  
	if (!Process32First(hSnapShot, &pe))
	{
		return FALSE;
	}

	//将字符串转换为小写  
	strProcessName.MakeLower();

	//如果句柄有效  则一直获取下一个句柄循环下去  
	while (Process32Next(hSnapShot, &pe))
	{
		//pe.szExeFile获取当前进程的可执行文件名称  
		CString scTmp = pe.szExeFile;

		//将可执行文件名称所有英文字母修改为小写  
		scTmp.MakeLower();

		//比较当前进程的可执行文件名称和传递进来的文件名称是否相同  
		//相同的话Compare返回0  
		if (!scTmp.Compare(strProcessName))
		{
			//从快照进程中获取该进程的PID(即任务管理器中的PID)  
			DWORD dwProcessID = pe.th32ProcessID;
			HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
			::TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
			return TRUE;
		}
		scTmp.ReleaseBuffer();
	}
	strProcessName.ReleaseBuffer();
	return FALSE;
}