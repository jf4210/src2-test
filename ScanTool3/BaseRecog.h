#pragma once
#include "global.h"

class CBaseRecog
{
public:
	CBaseRecog();
	virtual ~CBaseRecog();

protected:
	bool Recog(RECTINFO& rc, cv::Mat& matCompPic, std::string& strLog);
	bool Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//��ʶ���ѡ���м�����������ѡȡ����һ������������ʶ�𣬴˷������ڷ�ֹ���ֿ�ѡ������ѡ���������
	bool RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, RECTLIST& lSelInfo, std::string& strResult);	//ͨ��ʶ���������ѡ���������ж�ѡ����
public:
	CAssistRecog AssistObj;
};

class CAssistRecog
{
public:
	CAssistRecog();
	~CAssistRecog();

	//����ģʽ
	bool GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, cv::Rect& rt);
	//ͬ��ͷģʽ
	bool GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, RECTINFO& rc);
private:
	bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);

	inline cv::Point2d TriangleCoordinate(cv::Point ptA, cv::Point ptB, cv::Point ptC, cv::Point ptNewA, cv::Point ptNewB);
};
