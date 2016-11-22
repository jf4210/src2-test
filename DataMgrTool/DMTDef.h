#pragma once
#include "global.h"
#include "Log.h"
#include "MyCodeConvert.h"

#include "zbar.h"   


//#define PIC_RECTIFY_TEST	//图像旋转纠正测试
#define WarpAffine_TEST		//仿射变换测试
#ifdef _DEBUG
	#define PaintOmrSnRect		//是否打印识别出来的OMR矩形
//	#define Test_ShowOriPosition	//测试打印模板坐标对应的原图坐标位置
#endif
#ifndef WarpAffine_TEST
//	#define TriangleSide_TEST		//三边定位算法
	#ifndef TriangleSide_TEST
		#define TriangleCentroid_TEST	//三边质心算法
	#endif
#endif

#define USES_GETTHRESHOLD_ZTFB	//使用正态分布方式获取校验点的阀值，未定义时使用固定阀值进行二值化查找矩形

#define  MSG_ERR_RECOG		(WM_USER + 110)
#define  MSG_RECOG_COMPLETE	(WM_USER + 111)

#define DecompressTest		//解压测试，多线程解压

extern CLog g_Log;
extern int	g_nExitFlag;

extern CString				g_strCurrentPath;

extern std::string _strEncryptPwd_;
extern pMODEL _pModel_;

extern std::string		_strSessionName_;

extern std::map<std::string, std::string> answerMap;


extern bool		_nUseNewParam_;			//是否使用新的参数重新识别模板

extern int		_nAnticlutterKernel_;	//识别同步头时防干扰膨胀腐蚀的核因子
extern int		_nCannyKernel_;			//轮廓化核因子

extern int		g_nRecogGrayMin;		//灰度点(除空白点,OMR外)计算灰度的最小考试范围
extern int		g_nRecogGrayMax_White;	//空白点校验点计算灰度的最大考试范围
extern int		g_nRecogGrayMin_OMR;	//OMR计算灰度的最小考试范围
extern int		g_RecogGrayMax_OMR;		//OMR计算灰度的最大考试范围

extern double	_dCompThread_Fix_;
extern double	_dDiffThread_Fix_;
extern double	_dDiffExit_Fix_;
extern double	_dCompThread_Head_;
extern double	_dDiffThread_Head_;
extern double	_dDiffExit_Head_;
extern int		_nThreshold_Recog2_;	//第2中识别方法的二值化阀值

extern int		_nOMR_;		//重新识别模板时，用来识别OMR的密度值的阀值
extern int		_nSN_;		//重新识别模板时，用来识别ZKZH的密度值的阀值


typedef struct _DecompressTask_
{
	int nTaskType;				//1-普通解压，2-区分试卷包到不同目录, 3-重新识别OMR和SN, 4-加载模板, 5-识别试卷包并统计识别正确率比例
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
	std::string strDecompressDir;
	_DecompressTask_()
	{
		nTaskType = 1;
	}
}DECOMPRESSTASK, *pDECOMPRESSTASK;
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//识别任务列表

extern Poco::FastMutex			g_fmDecompressLock;		//解压文件列表锁
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//解压文件列表



typedef struct _PicInfo_				//图片信息
{
	bool			bRecoged;		//是否已经识别过
	bool			bFindIssue;		//是否找到问题点
	void*			pPaper;			//所属试卷的信息
	cv::Rect		rtFix;			//定点矩形
	std::string		strPicName;		//图片名称
	std::string		strPicPath;		//图片路径
	RECTLIST		lFix;			//定点列表
	RECTLIST		lNormalRect;	//识别出来的正常点位置
	RECTLIST		lIssueRect;		//识别出来的问题试卷的问题点位置，只要出现问题点就不进行下一页的识别
	// 	cv::Mat			matSrc;
	// 	cv::Mat			matDest;
	_PicInfo_()
	{
		bRecoged = false;
		bFindIssue = false;
		pPaper = NULL;
		//		ptFix = cv::Point(0, 0);
		//		ptModelFix = cv::Point(0, 0);
		//		bImgOpen = false;
	}
}ST_PicInfo, *pST_PicInfo;
typedef std::list<pST_PicInfo> PIC_LIST;	//图片列表定义

typedef struct _PaperInfo_
{
	bool		bIssuePaper;		//是否是问题试卷
	int			nQKFlag;			//缺考标识
	pMODEL		pModel;				//识别此学生试卷所用的模板
	void*		pPapers;			//所属的试卷袋信息
	void*		pSrcDlg;			//来源，来自哪个窗口，扫描or导入试卷窗口
	std::string strStudentInfo;		//学生信息	
	std::string strSN;

	SNLIST				lSnResult;
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//识别的选做题OMR结果
	PIC_LIST	lPic;
	_PaperInfo_()
	{
		bIssuePaper = false;
		nQKFlag = 0;
		pModel = NULL;
		pPapers = NULL;
		pSrcDlg = NULL;
	}
	~_PaperInfo_()
	{
		SNLIST::iterator itSn = lSnResult.begin();
		for (; itSn != lSnResult.end();)
		{
			pSN_ITEM pSNItem = *itSn;
			itSn = lSnResult.erase(itSn);
			SAFE_RELEASE(pSNItem);
		}

		PIC_LIST::iterator itPic = lPic.begin();
		for (; itPic != lPic.end();)
		{
			pST_PicInfo pPic = *itPic;
			SAFE_RELEASE(pPic);
			itPic = lPic.erase(itPic);
		}
	}
}ST_PaperInfo, *pST_PaperInfo;		//试卷信息，一个学生对应一份试卷，一份试卷可以有多份图片
typedef std::list<pST_PaperInfo> PAPER_LIST;	//试卷列表

typedef struct _PapersInfo_				//试卷袋信息结构体
{
	int		nPaperCount;				//试卷袋中试卷总数量(学生数)
	int		nRecogErrCount;				//识别错误试卷数量
	int		nTotalPics;

	int			nTotalPaper;		//总学生数量，从试卷袋文件夹读取
	int			nExamID;			//考试ID
	int			nSubjectID;			//科目ID
	int			nTeacherId;			//教师ID
	int			nUserId;			//用户ID

	//++统计信息
	int		nOmrDoubt;				//OMR怀疑的数量					客户端传递的
	int		nOmrNull;				//OMR识别为空的数量				客户端传递的
	int		nSnNull;				//准考证号识别为空的数量		客户端传递的

	int		nOmrError_1;			//根据学生答案，第1种识别方法识别错误的值		统计时用
	int		nOmrError_2;			//根据学生答案，第1种识别方法识别错误的值		统计时用

	Poco::FastMutex	fmOmrStatistics;//omr统计锁
	Poco::FastMutex fmSnStatistics; //zkzh统计锁
	//--

	int		nRecogPics;				//已经识别完成的数量
	Poco::FastMutex fmRecogCompleteOK; //上面的锁

	Poco::FastMutex fmlPaper;			//对试卷列表读写锁
	Poco::FastMutex fmlIssue;			//对问题试卷列表读写锁
	std::string  strPapersName;			//试卷袋名称
	std::string	 strPapersDesc;			//试卷袋详细描述

	std::string		strPapersPath;
	std::string		strSrcPapersPath;
	std::string		strSrcPapersFileName;

	std::string	strUploader;		//上传者
	std::string strEzs;				//上传给后端服务器用，--cookie
	std::string strDesc;			//从试卷袋文件夹读取

	PAPER_LIST	lPaper;					//此试卷袋中试卷列表
	PAPER_LIST	lIssue;					//此试卷袋中识别有问题的试卷列表
	_PapersInfo_()
	{
		nTotalPics = 0;
		nPaperCount = 0;
		nRecogErrCount = 0;
		nOmrDoubt = 0;
		nOmrNull = 0;
		nSnNull = 0;
		nRecogPics = 0;

		nTotalPaper = 0;
		nExamID = -1;
		nSubjectID = -1;
		nTeacherId = -1;
		nUserId = -1;
		nOmrError_1 = 0;
		nOmrError_2 = 0;
	}
	~_PapersInfo_()
	{
		fmlPaper.lock();
		PAPER_LIST::iterator itPaper = lPaper.begin();
		for (; itPaper != lPaper.end();)
		{
			pST_PaperInfo pPaper = *itPaper;
			SAFE_RELEASE(pPaper);
			itPaper = lPaper.erase(itPaper);
		}
		fmlPaper.unlock();
		fmlIssue.lock();
		PAPER_LIST::iterator itIssuePaper = lIssue.begin();
		for (; itIssuePaper != lIssue.end();)
		{
			pST_PaperInfo pPaper = *itIssuePaper;
			SAFE_RELEASE(pPaper);
			itIssuePaper = lIssue.erase(itIssuePaper);
		}
		fmlIssue.unlock();
	}
}PAPERSINFO, *pPAPERSINFO;
typedef std::list<pPAPERSINFO> PAPERS_LIST;		//试卷袋列表



typedef struct _CompressTask_
{
	std::string strSrcFilePath;
	std::string strCompressFileName;
	std::string strSavePath;
	std::string strExtName;
}COMPRESSTASK, *pCOMPRESSTASK;
typedef std::list<pCOMPRESSTASK> COMPRESSTASKLIST;	//识别任务列表

extern Poco::FastMutex			g_fmCompressLock;		//压缩文件列表锁
extern COMPRESSTASKLIST			g_lCompressTask;		//解压文件列表

extern Poco::Event			g_eCompressThreadExit;

typedef struct _RecogTask_
{
	int		nPic;						//此试卷属于模板的第几张
	pMODEL pModel;						//识别用的模板
	std::string strPath;
	pST_PaperInfo	pPaper;				//需要识别的试卷
}RECOGTASK, *pRECOGTASK;
typedef std::list<pRECOGTASK> RECOGTASKLIST;	//识别任务列表

extern Poco::FastMutex		g_fmRecog;		//识别线程获取任务锁
extern RECOGTASKLIST		g_lRecogTask;	//识别任务列表

extern Poco::FastMutex		g_fmPapers;		//操作试卷袋列表的任务锁
extern PAPERS_LIST			g_lPapers;		//所有的试卷袋信息

bool encString(std::string& strSrc, std::string& strDst);
bool decString(std::string& strSrc, std::string& strDst);


void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);

pMODEL	LoadModelFile(CString strModelPath);		//加载模板文件
bool	InitModelRecog(pMODEL pModel, std::string strModelPath);				//重新识别模板各校验点和OMR的灰度
bool	SortByArea(cv::Rect& rt1, cv::Rect& rt2);		//按面积排序
bool	SortByPositionX(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionY(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionX2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionY2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortbyNumASC(const std::string& x, const std::string& y);
bool	SortByPaper(const pST_PaperInfo& x, const pST_PaperInfo& y);

bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);
bool	FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//定点进行仿射变换
bool	FixwarpPerspective(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//定点透视变换
bool	PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);

//----------------	OMR识别灰度差值比较	------------------
typedef struct
{
	char szVal[4];
	float fFirst;
	float fSecond;
	float fDiff;
}ST_ITEM_DIFF, *pST_ITEM_DIFF;
bool	SortByItemDiff(ST_ITEM_DIFF& item1, ST_ITEM_DIFF& item2);
bool	SortByItemGray(pRECTINFO item1, pRECTINFO item2);
//--------------------------------------------------------


//----------------	二维码、条码识别	------------------
//zbar接口
std::string ZbarDecoder(cv::Mat img, std::string& strTypeName);

//对二值图像进行识别，如果失败则开运算进行二次识别
std::string GetQRInBinImg(cv::Mat binImg, std::string& strTypeName);

//main function
std::string GetQR(cv::Mat img, std::string& strTypeName);
//--------------------------------------------------------

bool ZipFile(std::string& strSavePath, std::string& strSrcDir, std::string strExtName = ".pkg");
bool SavePapersInfo(pPAPERSINFO pPapers);


//统计信息
//统计信息
extern Poco::FastMutex _fmErrorStatistics_;
extern int		_nErrorStatistics1_;	//第一种方法识别错误数
extern int		_nErrorStatistics2_;	//第二种方法识别错误数
extern int		_nDoubtStatistics;		//识别怀疑总数
extern int		_nNullStatistics;		//识别为空总数
extern int		_nAllStatistics_;		//统计总数
std::string calcStatistics(pPAPERSINFO pPapers);
//--
