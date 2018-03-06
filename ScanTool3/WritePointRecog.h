#pragma once
#include "BaseRecog.h"

//ÌîÍ¿µãÊ¶±ð
class CWritePointRecog :
	public CBaseRecog
{
public:
	CWritePointRecog() {};
	virtual ~CWritePointRecog() {};
	virtual bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog) = 0;
};

class CQKPoint :
	public CWritePointRecog
{
public:
	CQKPoint() {};
	~CQKPoint() {};
	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);
};

class CWJPoint :
	public CWritePointRecog
{
public:
	CWJPoint() {};
	~CWJPoint() {};
	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);
};

