#pragma once
#include "BaseRecog.h"

//印刷点识别
class CPrintPointRecog :
	public CBaseRecog
{
public:
	CPrintPointRecog() {};
	virtual ~CPrintPointRecog() {};
	
	//识别印刷点
	virtual bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog) = 0;
};

class CCharacterPoint :
	public CPrintPointRecog
{
public:
	CCharacterPoint(tesseract::TessBaseAPI* pTess):m_pTess(pTess) {};
	~CCharacterPoint() {};

	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);

protected:
	bool InitCharacterRecog();

	bool GetPicFix(int nPic, pST_PicInfo pPic, pMODEL pModel);
private:
#ifdef USE_TESSERACT
	tesseract::TessBaseAPI* m_pTess;
#endif
};

class CFixPoint :
	public CPrintPointRecog
{
public:
	CFixPoint() {};
	~CFixPoint() {};

	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);
};

class CABPoint :
	public CPrintPointRecog
{
public:
	CABPoint() {};
	~CABPoint() {};

	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);
};

class CCoursePoint :
	public CPrintPointRecog
{
public:
	CCoursePoint() {};
	~CCoursePoint() {};

	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);
};

class CGrayPoint :
	public CPrintPointRecog
{
public:
	CGrayPoint() {};
	~CGrayPoint() {};
	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);
};

class CWhitePoint :
	public CPrintPointRecog
{
public:
	CWhitePoint() {};
	~CWhitePoint() {};
	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);
};