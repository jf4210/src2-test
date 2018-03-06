#include "stdafx.h"
#include "PaperRecogMgr.h"
#include "SNPoint.h"
#include "OmrPoint.h"


CPaperRecogMgr::CPaperRecogMgr(int nRecogModel): _nRecogModel(nRecogModel), m_pTess(NULL)
{
}


CPaperRecogMgr::~CPaperRecogMgr()
{
	SAFE_RELEASE(pPrintPointRecogObj);
	SAFE_RELEASE(pWritePointRecogObj);
}

bool CPaperRecogMgr::RecogCharacter(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CCharacterPoint(m_pTess);
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CFixPoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel,_strLog);
}

bool CPaperRecogMgr::RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CABPoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CCoursePoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new CQKPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new CWJPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CGrayPoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CWhitePoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new CSNPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new COmrPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new CElectOmrPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

std::string CPaperRecogMgr::GetLog()
{
	return _strLog;
}

void CPaperRecogMgr::SetTesseractObj(tesseract::TessBaseAPI* pTess)
{
	m_pTess = pTess;
}
