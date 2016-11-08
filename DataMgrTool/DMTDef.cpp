#include "stdafx.h"
#include "DMTDef.h"

#include "miniunz/minizip.c"



bool encString(std::string& strSrc, std::string& strDst)
{
	bool bResult = true;
	if (_strEncryptPwd_.empty())
	{
		strDst = strSrc;
		return bResult;
	}
	try
	{
		Poco::Crypto::Cipher::Ptr pCipher = Poco::Crypto::CipherFactory::defaultFactory().createCipher(Poco::Crypto::CipherKey("aes256", _strEncryptPwd_));

		strDst = pCipher->encryptString(strSrc, Poco::Crypto::Cipher::ENC_BINHEX);
	}
	catch (...)
	{
		bResult = false;
		std::string strLog = "数据加密失败，按原数据操作";
		g_Log.LogOut(strLog);
	}
	return bResult;
}

bool decString(std::string& strSrc, std::string& strDst)
{
	bool bResult = true;
	if (_strEncryptPwd_.empty())
	{
		strDst = strSrc;
		return bResult;
	}
	try
	{
		Poco::Crypto::Cipher::Ptr pCipher = Poco::Crypto::CipherFactory::defaultFactory().createCipher(Poco::Crypto::CipherKey("aes256", _strEncryptPwd_));

		strDst = pCipher->decryptString(strSrc, Poco::Crypto::Cipher::ENC_BINHEX);
	}
	catch (...)
	{
		bResult = false;
		std::string strLog = "数据解密失败，按原数据操作";
		g_Log.LogOut(strLog);
	}
	return bResult;
}


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
	return SortbyNumASC(x->strStudentInfo, y->strStudentInfo);
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

bool SortByItemGray(pRECTINFO item1, pRECTINFO item2)
{
	return item1->fRealValuePercent > item2->fRealValuePercent ? true : false;
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
	if (ptNewB.y == ptNewA.y)
	{
		long double x = (ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) - pow(ptNewA.y, 2) + pow(ptNewB.y, 2)) / (2 * (ptNewB.x - ptNewA.x));
		long double y = (rb2 - rc2 - pow(ptNewB.x, 2) + pow(ptNewC.x, 2) - pow(ptNewB.y, 2) + pow(ptNewC.y, 2) - 2 * (ptNewC.x - ptNewB.x)*x) / (2 * (ptNewC.y - ptNewB.y));
		ptNewChk.x = x;
		ptNewChk.y = y;
		return ptNewChk;
	}

#if 0
	long double v1 = rb2 - rc2 - pow(ptNewB.x, 2) + pow(ptNewC.x, 2) - pow(ptNewB.y, 2) + pow(ptNewC.y, 2) - ((ptNewC.y - ptNewB.y)*(ra2 - rb2 - pow(ptNewA.x, 2) + pow(ptNewB.x, 2) - pow(ptNewA.y, 2) + pow(ptNewB.y, 2)) / (ptNewB.y - ptNewA.y));
	long double v2 = 2 * (ptNewC.x - ptNewB.x) - 2 * (ptNewC.y - ptNewB.y)*(ptNewB.x - ptNewA.x) / (ptNewB.y - ptNewA.y);
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
	cv::Point2d ptNewC;
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
	end = clock();
	//	TRACE("原C点的垂直点D(%f,%f), 新的C点坐标(%f, %f)或者(%f, %f),确定后为(%d,%d)耗时: %d\n", dDx, dDy, dXc1, dYc1, dXc2, dYc2, ptNewC.x, ptNewC.y, end - start);
	TRACE("新的C点坐标(%f, %f)或者(%f, %f),确定后为(%f,%f)耗时: %d\n", dXc1, dYc1, dXc2, dYc2, ptNewC.x, ptNewC.y, end - start);
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
		long double x = (ptResult1.x * (v1 / da + v2 / db) + ptResult2.x * (v1 / da + v3 / dc) + ptResult3.x * (v2 / db + v3 / dc)) / (2 * (v1 / da + v2 / db + v3 / dc));
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
		TRACE("三边质心算法: chk(%f,%f),ptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f),最终质心(%f,%f)备用点(%f,%f)\n", ptChk.x, ptChk.y, \
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
		TRACE("三边质心算法: chk(%f,%f), da=%f,db=%f,dc=%f,dd=%f,\nptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f),ptResult4(%f,%f),ptResult5(%f,%f),ptResult6(%f,%f),最终质心(%f,%f)\n", ptChk.x, ptChk.y, \
			  da, db, dc, dd, ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult4.x, ptResult4.y, ptResult5.x, ptResult5.y, ptResult6.x, ptResult6.y, ptResult.x, ptResult.y);
#endif
		rt.x = ptResult.x;
		rt.y = ptResult.y;
#endif
	}
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
	g_Log.LogOut(szTmpLog);
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
	sprintf_s(szTmpLog, "图像变换时间: %d, ptMod1(%.2f,%.2f), ptMod2(%.2f,%.2f), ptMod3(%.2f,%.2f), ptMod4(%.2f,%.2f), pt1(%.2f,%.2f), pt2(%.2f,%.2f), pt3(%.2f,%.2f), pt4(%.2f,%.2f)\n", end - start, \
			  vecFixPt[0].x, vecFixPt[0].y, vecFixPt[1].x, vecFixPt[1].y, vecFixPt[2].x, vecFixPt[2].y, vecFixPt[3].x, vecFixPt[3].y, \
			  vecFixNewPt[0].x, vecFixNewPt[0].y, vecFixNewPt[1].x, vecFixNewPt[1].y, vecFixNewPt[2].x, vecFixNewPt[2].y, vecFixNewPt[3].x, vecFixNewPt[3].y);
	g_Log.LogOut(szTmpLog);
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
	cv::Mat binImg;
	//在otsu二值结果的基础上，不断增加阈值，用于识别模糊图像
	int thre = threshold(img, binImg, 0, 255, cv::THRESH_OTSU);
	std::string result;
	while (result.empty() && thre < 255)
	{
		threshold(img, binImg, thre, 255, cv::THRESH_BINARY);
		result = GetQRInBinImg(binImg, strTypeName);
		thre += 20;//阈值步长设为20，步长越大，识别率越低，速度越快
	}
	return result;
}
//===================================================



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

	if (!decString(strJsnData, strFileData))
		strFileData = strJsnData;


	pMODEL pModel = NULL;
	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strFileData);		//strJsnData
		Poco::JSON::Object::Ptr objData = result.extract<Poco::JSON::Object::Ptr>();

		pModel = new MODEL;
		pModel->strModelName = CMyCodeConvert::Utf8ToGb2312(objData->get("modelName").convert<std::string>());
		pModel->strModelDesc = CMyCodeConvert::Utf8ToGb2312(objData->get("modelDesc").convert<std::string>());
		if (objData->has("modelType"))
			pModel->nType = objData->get("modelType").convert<int>();
		else
			pModel->nType = 0;
		pModel->nSaveMode = objData->get("modeSaveMode").convert<int>();
		pModel->nPicNum = objData->get("paperModelCount").convert<int>();
		pModel->nEnableModify = objData->get("enableModify").convert<int>();
		pModel->nABModel = objData->get("abPaper").convert<int>();
		pModel->nHasHead = objData->get("hasHead").convert<int>();
		if (objData->has("hasElectOmr"))
			pModel->nHasElectOmr = objData->get("hasElectOmr").convert<int>();
		if (objData->has("nZkzhType"))
			pModel->nZkzhType = objData->get("nZkzhType").convert<int>();

		// 		if (objData->has("gaussKernel"))
		// 			pModel->nGaussKernel = objData->get("gaussKernel").convert<int>();
		// 		if (objData->has("sharpKernel"))
		// 			pModel->nSharpKernel = objData->get("sharpKernel").convert<int>();
		// 		if (objData->has("cannyKernel"))
		// 			pModel->nCannyKernel = objData->get("cannyKernel").convert<int>();
		// 		if (objData->has("dilateKernel"))
		// 			pModel->nDilateKernel = objData->get("dilateKernel").convert<int>();

		if (objData->has("nExamId"))
			pModel->nExamID = objData->get("nExamId").convert<int>();
		if (objData->has("nSubjectId"))
			pModel->nSubjectID = objData->get("nSubjectId").convert<int>();

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
			paperModelInfo->rtSNTracker.x = jsnPaperObj->get("rtSNTracker.x").convert<int>();
			paperModelInfo->rtSNTracker.y = jsnPaperObj->get("rtSNTracker.y").convert<int>();
			paperModelInfo->rtSNTracker.width = jsnPaperObj->get("rtSNTracker.width").convert<int>();
			paperModelInfo->rtSNTracker.height = jsnPaperObj->get("rtSNTracker.height").convert<int>();

			Poco::JSON::Array::Ptr arraySelHTracker = jsnPaperObj->getArray("hTrackerRect");
			Poco::JSON::Array::Ptr arraySelVTracker = jsnPaperObj->getArray("vTrackerRect");
			Poco::JSON::Array::Ptr arraySelFixRoi = jsnPaperObj->getArray("selRoiRect");
			Poco::JSON::Array::Ptr arrayFixCP = jsnPaperObj->getArray("FixCP");
			Poco::JSON::Array::Ptr arrayHHead = jsnPaperObj->getArray("H_Head");
			Poco::JSON::Array::Ptr arrayVHead = jsnPaperObj->getArray("V_Head");
			Poco::JSON::Array::Ptr arrayABModel = jsnPaperObj->getArray("ABModel");
			Poco::JSON::Array::Ptr arrayCourse = jsnPaperObj->getArray("Course");
			Poco::JSON::Array::Ptr arrayQKCP = jsnPaperObj->getArray("QKCP");
			Poco::JSON::Array::Ptr arrayGrayCP = jsnPaperObj->getArray("GrayCP");
			Poco::JSON::Array::Ptr arrayWhiteCP = jsnPaperObj->getArray("WhiteCP");
			Poco::JSON::Array::Ptr arraySn = jsnPaperObj->getArray("snList");
			Poco::JSON::Array::Ptr arrayOmr = jsnPaperObj->getArray("selOmrRect");
			Poco::JSON::Array::Ptr arrayElectOmr;
			if (jsnPaperObj->has("electOmrList"))
				arrayElectOmr = jsnPaperObj->getArray("electOmrList");

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
		g_Log.LogOut(strErrInfo);
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
		g_Log.LogOut(strErrInfo);
	}

	return pModel;
}

bool ZipFile(std::string& strSavePath, std::string& strSrcDir, std::string strExtName /*= ".pkg"*/)
{
	bool bResult = true;
	std::string strZipName = strSavePath + strExtName;

	try
	{
		Poco::File zipModel(CMyCodeConvert::Gb2312ToUtf8(strZipName));
		if (zipModel.exists())
			zipModel.remove(true);
	}
	catch (Poco::Exception& exc)
	{
		std::string strErr = "压缩文件(" + strZipName + ")判断异常: " + exc.message();
		g_Log.LogOutError(strErr);
	}

	clock_t startTime, endTime;
	startTime = clock();
	std::string strLog = "开始文件压缩: " + strZipName;
	g_Log.LogOut(strLog);
//	std::cout << strLog << std::endl;

	zipFile zf = NULL;
#ifdef USEWIN32IOAPI
	zlib_filefunc64_def ffunc = { 0 };
#endif
	char *zipfilename = const_cast<char*>(strZipName.c_str());
	const char* password = NULL;
	password = "static";

	void* buf = NULL;
	int size_buf = WRITEBUFFERSIZE;
	int err = 0;
	int errclose = 0;

	int opt_overwrite = APPEND_STATUS_CREATE;
	int opt_compress_level = Z_DEFAULT_COMPRESSION;

	buf = (void*)malloc(size_buf);
	if (buf == NULL)
	{
		printf("Error allocating memory\n");
		return ZIP_INTERNALERROR;
	}

#ifdef USEWIN32IOAPI
	fill_win32_filefunc64A(&ffunc);
	zf = zipOpen2_64(zipfilename, opt_overwrite, NULL, &ffunc);
#else
	zf = zipOpen64(zipfilename, opt_overwrite);
#endif

	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strSrcDir);
	Poco::DirectoryIterator it(strUtf8Path);
	Poco::DirectoryIterator end;
	while (it != end)
	{
		Poco::Path p(it->path());
		std::string strFilePath = CMyCodeConvert::Utf8ToGb2312(it->path());
		if (it->isFile())
		{
			Poco::File modelFile(it->path());

			FILE *fin = NULL;
			int size_read = 0;
			const char* filenameinzip = strFilePath.c_str();
			const char *savefilenameinzip;
			zip_fileinfo zi = { 0 };
			unsigned long crcFile = 0;
			int zip64 = 0;

			/* Get information about the file on disk so we can store it in zip */
			filetime(filenameinzip, &zi.tmz_date, &zi.dosDate);

			std::string strName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());
			savefilenameinzip = strName.c_str();

			/* Add to zip file */
			err = zipOpenNewFileInZip3_64(zf, savefilenameinzip, &zi,
										  NULL, 0, NULL, 0, NULL /* comment*/,
										  (opt_compress_level != 0) ? Z_DEFLATED : 0,
										  opt_compress_level, 0,
										  -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
										  password, crcFile, zip64);

			if (err != ZIP_OK)
				printf("error in opening %s in zipfile (%d)\n", filenameinzip, err);
			else
			{
				fin = FOPEN_FUNC(filenameinzip, "rb");
				if (fin == NULL)
				{
					err = ZIP_ERRNO;
					printf("error in opening %s for reading\n", filenameinzip);
				}
			}

			if (err == ZIP_OK)
			{
				/* Read contents of file and write it to zip */
				do
				{
					size_read = (int)fread(buf, 1, size_buf, fin);
					if ((size_read < size_buf) && (feof(fin) == 0))
					{
						printf("error in reading %s\n", filenameinzip);
						err = ZIP_ERRNO;
					}

					if (size_read > 0)
					{
						err = zipWriteInFileInZip(zf, buf, size_read);
						if (err < 0)
							printf("error in writing %s in the zipfile (%d)\n", filenameinzip, err);
					}
				} while ((err == ZIP_OK) && (size_read > 0));
			}

			if (fin)
				fclose(fin);

			if (err < 0)
			{
				err = ZIP_ERRNO;
				strLog = "压缩文件失败(" + strName + ")";
				g_Log.LogOutError(strLog);
			}
			else
			{
				err = zipCloseFileInZip(zf);
				if (err != ZIP_OK)
					printf("error in closing %s in the zipfile (%d)\n", filenameinzip, err);
			}
		}
		it++;
	}

	errclose = zipClose(zf, NULL);
	if (errclose != ZIP_OK)
		printf("error in closing %s (%d)\n", zipfilename, errclose);

	free(buf);

	endTime = clock();
	strLog = Poco::format("文件压缩完成: %s, 时间: %dms", strZipName, (int)(endTime - startTime));
	g_Log.LogOut(strLog);
//	std::cout << strLog << std::endl;
	return bResult;
}

bool SavePapersInfo(pPAPERSINFO pPapers)
{
	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itNomarlPaper = pPapers->lPaper.begin();
	for (; itNomarlPaper != pPapers->lPaper.end(); itNomarlPaper++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itNomarlPaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itNomarlPaper)->strSN);
		jsnPaper.set("qk", (*itNomarlPaper)->nQKFlag);

		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itNomarlPaper)->lSnResult.begin();
		for (; itSn != (*itNomarlPaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				jsnSnPosition.set("x", itRect->rt.x);
				jsnSnPosition.set("y", itRect->rt.y);
				jsnSnPosition.set("w", itRect->rt.width);
				jsnSnPosition.set("h", itRect->rt.height);
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaper.set("snDetail", jsnSnDetailArry);

		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itNomarlPaper)->lOmrResult.begin();
		for (; itOmr != (*itNomarlPaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value2", itOmr->strRecogVal2);
			jsnOmr.set("doubt", itOmr->nDoubt);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);

		Poco::JSON::Array jsnElectOmrArry;
		ELECTOMR_LIST::iterator itElectOmr = (*itNomarlPaper)->lElectOmrResult.begin();
		for (; itElectOmr != (*itNomarlPaper)->lElectOmrResult.end(); itElectOmr++)
		{
			Poco::JSON::Object jsnElectOmr;
			jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnElectOmr.set("value", itElectOmr->strRecogResult);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
			for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnElectOmr.set("position", jsnPositionArry);
			jsnElectOmrArry.add(jsnElectOmr);
		}
		jsnPaper.set("electOmr", jsnElectOmrArry);		//选做题结果
		jsnPaperArry.add(jsnPaper);
	}
	PAPER_LIST::iterator itIssuePaper = pPapers->lIssue.begin();
	for (; itIssuePaper != pPapers->lIssue.end(); itIssuePaper++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itIssuePaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itIssuePaper)->strSN);
		jsnPaper.set("qk", (*itIssuePaper)->nQKFlag);

		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itIssuePaper)->lSnResult.begin();
		for (; itSn != (*itIssuePaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				jsnSnPosition.set("x", itRect->rt.x);
				jsnSnPosition.set("y", itRect->rt.y);
				jsnSnPosition.set("w", itRect->rt.width);
				jsnSnPosition.set("h", itRect->rt.height);
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaper.set("snDetail", jsnSnDetailArry);

		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itIssuePaper)->lOmrResult.begin();
		for (; itOmr != (*itIssuePaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value2", itOmr->strRecogVal2);
			jsnOmr.set("doubt", itOmr->nDoubt);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);

		Poco::JSON::Array jsnElectOmrArry;
		ELECTOMR_LIST::iterator itElectOmr = (*itIssuePaper)->lElectOmrResult.begin();
		for (; itElectOmr != (*itIssuePaper)->lElectOmrResult.end(); itElectOmr++)
		{
			Poco::JSON::Object jsnElectOmr;
			jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnElectOmr.set("value", itElectOmr->strRecogResult);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
			for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnElectOmr.set("position", jsnPositionArry);
			jsnElectOmrArry.add(jsnElectOmr);
		}
		jsnPaper.set("electOmr", jsnElectOmrArry);		//选做题结果
		jsnPaperArry.add(jsnPaper);
	}
	//写试卷袋信息到文件
	std::string strUploader = pPapers->strUploader;
	std::string sEzs = pPapers->strEzs;
	Poco::JSON::Object jsnFileData;
	jsnFileData.set("examId", pPapers->nExamID);
	jsnFileData.set("subjectId", pPapers->nSubjectID);
	jsnFileData.set("uploader", pPapers->strUploader);
	jsnFileData.set("ezs", sEzs);
	jsnFileData.set("nTeacherId", pPapers->nTeacherId);
	jsnFileData.set("nUserId", pPapers->nUserId);
	jsnFileData.set("scanNum", pPapers->nTotalPaper);		//扫描的学生数量
	jsnFileData.set("detail", jsnPaperArry);

	jsnFileData.set("nOmrDoubt", pPapers->nOmrDoubt);
	jsnFileData.set("nOmrNull", pPapers->nOmrNull);
	jsnFileData.set("nSnNull", pPapers->nSnNull);
	std::stringstream jsnString;
	jsnFileData.stringify(jsnString, 0);


	std::string strFileData;
	if (!encString(jsnString.str(), strFileData))
		strFileData = jsnString.str();

	char szExamInfoPath[MAX_PATH] = { 0 };
	sprintf_s(szExamInfoPath, "%s\\papersInfo.dat", pPapers->strPapersPath.c_str());

	try
	{
		std::string strUtf8PapersInfoFilePath = CMyCodeConvert::Gb2312ToUtf8(szExamInfoPath);
		Poco::File papersInfoFile(strUtf8PapersInfoFilePath);
		if (papersInfoFile.exists())
			papersInfoFile.remove(true);
	}
	catch (Poco::Exception& exc)
	{
		g_Log.LogOutError("删除文件失败(" + pPapers->strPapersName + ")");
	}

	ofstream out(szExamInfoPath);
	out << strFileData.c_str();
	out.close();

	return true;
}

std::string calcStatistics(pPAPERSINFO pPapers)
{
	std::stringstream ss;
	int nOmrCount = 0;
// 	if (answerMap.size() == 0)
// 		return "";

	std::string strErrorInfo1;
	std::string strErrorInfo2;
	//omr统计
	PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
	for (; itPaper != pPapers->lPaper.end(); itPaper++)
	{
		pST_PaperInfo pPaper = *itPaper;
		nOmrCount += pPaper->lOmrResult.size();

		OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
		for (int i = 1; itOmr != pPaper->lOmrResult.end(); i++, itOmr++)
		{
			ss.str("");
			ss << pPapers->strPapersName << ":" << pPaper->strStudentInfo << ":" << i;
			std::string strTmp = ss.str();
			std::map<std::string, std::string>::iterator itAnswer = answerMap.find(ss.str());
			if (itAnswer != answerMap.end())
			{
				if (itOmr->strRecogVal != itAnswer->second)
				{
					pPapers->nOmrError_1++;
					strErrorInfo1.append(ss.str() + "-" + itOmr->strRecogVal + "   R=" + itAnswer->second + "\n");
				}
				if (itOmr->strRecogVal2 != itAnswer->second)
				{
					pPapers->nOmrError_2++;
					strErrorInfo2.append(ss.str() + "-" + itOmr->strRecogVal2 + "   R=" + itAnswer->second + "\n");
				}
			}
		}
	}
	PAPER_LIST::iterator itPaper2 = pPapers->lIssue.begin();
	for (; itPaper2 != pPapers->lIssue.end(); itPaper2++)
	{
		pST_PaperInfo pPaper = *itPaper2;
		nOmrCount += pPaper->lOmrResult.size();

		OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
		for (int i = 1; itOmr != pPaper->lOmrResult.end(); i++, itOmr++)
		{
			ss.str("");
			ss << pPapers->strPapersName << ":" << pPaper->strStudentInfo << ":" << i;
			std::map<std::string, std::string>::iterator itAnswer = answerMap.find(ss.str());
			if (itAnswer != answerMap.end())
			{
				if (itOmr->strRecogVal != itAnswer->second)
					pPapers->nOmrError_1++;
				if (itOmr->strRecogVal2 != itAnswer->second)
					pPapers->nOmrError_2++;
			}
		}
	}

	ss.str("");
	char szStatisticsInfo[300] = { 0 };
	sprintf_s(szStatisticsInfo, "\n识别错误信息统计: omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\n", (float)pPapers->nOmrError_1 / nOmrCount * 100, pPapers->nOmrError_1, nOmrCount, \
			  (float)pPapers->nOmrError_2 / nOmrCount * 100, pPapers->nOmrError_2, nOmrCount);

	ss << "\r\n-------------------\r\n"<< pPapers->strPapersName << "结果正确率统计完成:\r\n" << szStatisticsInfo << "\r\n-------------------\r\n\r\n";

	_fmErrorStatistics_.lock();
	_nErrorStatistics1_ += pPapers->nOmrError_1;
	_nErrorStatistics2_ += pPapers->nOmrError_2;
	_nDoubtStatistics += pPapers->nOmrDoubt;
	_nNullStatistics += pPapers->nOmrNull;
	_nAllStatistics_ += nOmrCount;
	_fmErrorStatistics_.unlock();

	std::string strLog;
	strLog = "识别错误详情1: \n" + strErrorInfo1 + "\n错误详情2: " + strErrorInfo2 + "\n";
	g_Log.LogOut(strLog);
	return ss.str();
}


inline bool RecogGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc)
{
	cv::cvtColor(matSrcRoi, matSrcRoi, CV_BGR2GRAY);
	cv::GaussianBlur(matSrcRoi, matSrcRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);
	SharpenImage(matSrcRoi, matSrcRoi, rc.nSharpKernel);

	const int channels[1] = { 0 };
	const float* ranges[1];
	const int histSize[1] = { 1 };
	float hranges[2];
	if (rc.eCPType != WHITE_CP)
	{
		hranges[0] = 0;
		hranges[1] = static_cast<float>(rc.nThresholdValue);
		ranges[0] = hranges;
	}
	else
	{
		hranges[0] = static_cast<float>(rc.nThresholdValue);
		hranges[1] = 255;	//255			//256时可统计完全空白的点，即RGB值为255的完全空白点;255时只能统计到RGB为254的值，255的值统计不到
		ranges[0] = hranges;
	}
	cv::MatND src_hist;
	cv::calcHist(&matSrcRoi, 1, channels, cv::Mat(), src_hist, 1, histSize, ranges, false);

	rc.fStandardValue = src_hist.at<float>(0);
	rc.fStandardArea = rc.rt.area();
	rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea;



	cv::MatND src_hist2;
	const int histSize2[1] = { 256 };	//rc.nThresholdValue - g_nRecogGrayMin
	const float* ranges2[1];
	float hranges2[2];
	hranges2[0] = 0;
	hranges2[1] = 255;
	ranges2[0] = hranges2;
	cv::calcHist(&matSrcRoi, 1, channels, cv::Mat(), src_hist2, 1, histSize2, ranges2, true, false);
	int nCount = 0;
	for (int i = 0; i < 256; i++)
	{
		nCount += i * src_hist2.at<float>(i);
	}
	rc.fStandardMeanGray = nCount / rc.fStandardArea;
	return true;
}

bool InitModelRecog(pMODEL pModel, std::string strModelPath)
{
	bool bResult = true;
	std::string strLog;

	for (int i = 0; i < pModel->vecPaperModel.size(); i++)
	{
		std::string strModelPicPath = strModelPath + "\\" + pModel->vecPaperModel[i]->strModelPicName;

		cv::Mat matSrc = cv::imread(strModelPicPath);

		pPAPERMODEL pPicModel = pModel->vecPaperModel[i];

		if (pModel->nZkzhType == 1)
		{
			SNLIST::iterator itSN = pPicModel->lSNInfo.begin();
			for (; itSN != pPicModel->lSNInfo.end(); itSN++)
			{
				pSN_ITEM pSNItem = *itSN;
				RECTLIST::iterator itSNItem = pSNItem->lSN.begin();
				for (; itSNItem != pSNItem->lSN.end(); itSNItem++)
				{
					itSNItem->nThresholdValue = _nSN_;
					
					cv::Mat matComp = matSrc(itSNItem->rt);
					RecogGrayValue(matComp, *itSNItem);
				}
			}
		}
		OMRLIST::iterator itOmr = pPicModel->lOMR2.begin();
		for (; itOmr != pPicModel->lOMR2.end(); itOmr++)
		{
			RECTLIST::iterator itOmrItem = itOmr->lSelAnswer.begin();
			for (; itOmrItem != itOmr->lSelAnswer.end(); itOmrItem++)
			{
				itOmrItem->nThresholdValue = _nOMR_;

				cv::Mat matComp = matSrc(itOmrItem->rt);
				RecogGrayValue(matComp, *itOmrItem);
			}
		}

		ELECTOMR_LIST::iterator itElectOmr = pPicModel->lElectOmr.begin();
		for (; itElectOmr != pPicModel->lElectOmr.end(); itElectOmr++)
		{
			RECTLIST::iterator itOmrItem = itElectOmr->lItemInfo.begin();
			for (; itOmrItem != itElectOmr->lItemInfo.end(); itOmrItem++)
			{
				itOmrItem->nThresholdValue = _nOMR_;
				
				cv::Mat matComp = matSrc(itOmrItem->rt);
				RecogGrayValue(matComp, *itOmrItem);
			}
		}
	}

	strLog = "模板图片校验信息识别完成";
	g_Log.LogOut(strLog);
	std::cout << strLog << std::endl;
	return bResult;
}

