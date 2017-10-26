#pragma once
#include "global.h"

typedef struct _WordMatchTempl_
{
	cv::Rect rtModel;	//��Ӧ���ֵ�ģ��λ����Ϣ
	RECTINFO rcReal;	//ʵ�ʽ���ģ��ƥ������Ϣ
}ST_WORDMATCHTEMPL, *pST_WORDMATCHTEMPL;

class COmrRecog
{
public:
	COmrRecog();
	~COmrRecog();

	//����һ�����Σ�������������ת��ľ���
	//1:���򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180
	cv::Rect GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation);

	bool RecogFixCP(int nPic, cv::Mat& matCompPic, RECTLIST& rlFix, pMODEL	pModel, int nOrientation = 1);		//ʶ�𶨵�, Ĭ�����򣬲�������ת
	bool RecogRtVal(RECTINFO& rc, cv::Mat& matCompPic);			//ʶ�������һ�����εĻҶȡ��ܶȵ���Ϣ
	bool RecogZkzh(int nPic, cv::Mat& matCompPic, pMODEL pModel, int nOrientation);	//ʶ���ά��������룬����ʶ��ɹ�������ж�ͼ����

	//��ȡͼƬ����ȷ���򣬺�ģ��һ��
	int GetRightPicOrientation(cv::Mat& matSrc, int n, bool bDoubleScan);
protected:	
	//ʶ����λҶȡ��ܶȵ���Ϣ
	inline bool Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL	pModel);

	//-------------------------------------------------------------
	void		sharpenImage1(const cv::Mat &image, cv::Mat &result, int nKernel);
	int			GetRects(cv::Mat& matSrc, cv::Rect rt, pMODEL pModel, int nPic, int nOrientation, int nHead);
//	cv::Rect	GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation);
	float		GetRtDensity(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod);
	bool		bGetMaxRect(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod, cv::Rect& rtMax);
	bool		RecogCodeOrientation(cv::Mat& matSrc, int n, int& nResult);	//���뷽���ж�
	void		PicRotate(cv::Mat& matSrc, int n);	//ͼ����ת��ʵ��ͼ����Ҫ���е���ת��1�����򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180
	bool		RecogWordOrientationByMatchTempl(cv::Mat& matSrc, int n, int nRotation, int& nResult, std::string& strLog); //ͨ��ģ�������ƥ����ʶ�����ֵķ��򣬾������
	bool		RecogWordOrientationByRectCount(cv::Mat& matSrc, int n, int nRotation, int& nResult, std::string& strLog); //ͨ������������ʶ�����ֵķ��򣬾������

	bool	MatchingMethod(int method, cv::Mat& src, cv::Mat& templ, cv::Point& ptResult);	//ģ��ƥ�����

	int		CheckOrientation4Fix(cv::Mat& matSrc, int n);	//����ģʽ�µķ���
	int		CheckOrientation4Head(cv::Mat& matSrc, int n);	//ͬ��ͷģʽ�µķ���
	int		CheckOrientation4Word(cv::Mat& matSrc, int n);	//���ֶ�λģʽ�µķ����ж�
	//���ͼ����ת�����ڻ�ȡͼ���ģ��ͼ�������ת
	//1:���ģ��ͼ����Ҫ���е���ת�����򣬲���Ҫ��ת��2����ת90(ģ��ͼ����ת), 3����ת90(ģ��ͼ����ת), 4����ת180(ģ��ͼ����ת)
	int		CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan);
	//-------------------------------------------------------------


	int			GetRectsInArea(cv::Mat& matSrc, RECTINFO rc, int nMinW, int nMaxW, int nMinH, int nMaxH, int nFindContoursModel = CV_RETR_EXTERNAL);	//��ȡ���������ڵľ�������
};

