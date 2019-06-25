#include "ModifyPic.h"

bool CModifyPic::ModifyPic(std::string strPicPath, std::string strPapersName)
{
	try
	{
		cv::Mat matSrc = cv::imread(strPicPath);
		int nW = matSrc.cols;
		int nH = matSrc.rows;
		int x1 = nW * 0.3;
		int x2 = nW * 0.6;
		int x3 = nW * 0.9;
		int y = 5;
		if (matSrc.channels() == 3)
		{
			matSrc.at<cv::Vec3b>(y, x1)[0] = 255;
			matSrc.at<cv::Vec3b>(y, x1)[1] = 255;
			matSrc.at<cv::Vec3b>(y, x1)[2] = 255;

			matSrc.at<cv::Vec3b>(y, x2)[0] = 255;
			matSrc.at<cv::Vec3b>(y, x2)[1] = 255;
			matSrc.at<cv::Vec3b>(y, x2)[2] = 255;

			matSrc.at<cv::Vec3b>(y, x3)[0] = 255;
			matSrc.at<cv::Vec3b>(y, x3)[1] = 255;
			matSrc.at<cv::Vec3b>(y, x3)[2] = 255;
		}
		else if (matSrc.channels() == 1)
		{
			matSrc.at<uchar>(y, x1) = 255;
			matSrc.at<uchar>(y, x2) = 255;
			matSrc.at<uchar>(y, x3) = 255;
		}
		imwrite(strPicPath, matSrc);
		std::string strTmpLog = "[" + strPapersName + "]修改图片" + strPicPath + "的像素点成功\n";
		g_Log.LogOutError(strTmpLog);
	}
	catch (cv::Exception& exc)
	{
		std::string strTmpLog = "[" + strPapersName + "]修改图片" + strPicPath + "的像素点失败: " + exc.msg;
		g_Log.LogOutError(strTmpLog);
		return false;
	}
	return true;
}
