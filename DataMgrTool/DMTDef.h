#pragma once
#include "global.h"
#include "Log.h"
#include "MyCodeConvert.h"

#include "zbar.h"   

//****************************************************************
//****************************************************************
//****************************************************************
//#define	DATE_LIMIT		//日期限制，过了一定时间不让用
//****************************************************************
//****************************************************************
//****************************************************************


//#define PIC_RECTIFY_TEST	//图像旋转纠正测试
#define WarpAffine_TEST		//仿射变换测试
#ifdef _DEBUG
	#define PaintOmrSnRect		//是否打印识别出来的OMR矩形
//	#define Test_ShowOriPosition	//测试打印模板坐标对应的原图坐标位置
	#define PrintAllOmrVal		//打印所有OMR选项值
#endif
#ifndef WarpAffine_TEST
//	#define TriangleSide_TEST		//三边定位算法
	#ifndef TriangleSide_TEST
		#define TriangleCentroid_TEST	//三边质心算法
	#endif
#endif

//++ test 2017.4.27
#define Test_RecogFirst_NoThreshord		//针对第一种算法，不用阀值直接判断要求最少选项数，如达到多少密度比例就认为选项一定填涂，不用这种方法
//--

#define Test_RecogOmr3			//第3种OMR识别方法测试
#define Test_SendRecogResult	//直接在试卷袋识别完成时发送识别结果给后端服务器

#define USES_GETTHRESHOLD_ZTFB	//使用正态分布方式获取校验点的阀值，未定义时使用固定阀值进行二值化查找矩形

#define  MSG_ERR_RECOG		(WM_USER + 110)
#define  MSG_RECOG_COMPLETE	(WM_USER + 111)
#define  MSG_SENDRESULT_STATE (WM_USER + 112)

#define DecompressTest		//解压测试，多线程解压

#define SOFT_VERSION	_T("1.70511-1")
#define SYS_BASE_NAME	_T("YKLX-DMT")
//#define WH_CCBKS		//武汉楚才杯专用，解析二维码需要json解析

#define CHK_NUM		//將試卷袋中不在參數文件列表中的文件从试卷袋信息中剔除
//#define WIN_THREAD_TEST		//使用window线程，不用poco创建线程

extern CLog g_Log;
extern int	g_nExitFlag;

extern CString				g_strCurrentPath;
extern int					g_nRecogMode;		//识别模式，0-严格模式，1-简单模式

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


extern double	_dOmrThresholdPercent_Fix_;		//定点模式OMR识别可认为是选中的标准百分比
extern double	_dSnThresholdPercent_Fix_;		//定点模式SN识别可认为是选中的标准百分比
extern double	_dQKThresholdPercent_Fix_;		//定点模式QK识别可认为是选中的标准百分比
extern double	_dOmrThresholdPercent_Head_;	//同步头模式OMR识别可认为是选中的标准百分比
extern double	_dSnThresholdPercent_Head_;		//同步头模式SN识别可认为是选中的标准百分比
extern double	_dQKThresholdPercent_Head_;		//同步头模式QK识别可认为是选中的标准百分比

extern double	_dCompThread_Fix_;
extern double	_dDiffThread_Fix_;
extern double	_dDiffExit_Fix_;
extern double	_dCompThread_Head_;
extern double	_dDiffThread_Head_;
extern double	_dDiffExit_Head_;
extern int		_nThreshold_Recog2_;	//第2中识别方法的二值化阀值
extern double	_dCompThread_3_;		//第三种识别方法
extern double	_dDiffThread_3_;
extern double	_dDiffExit_3_;
extern double	_dAnswerSure_;		//可以确认是答案的最大灰度

extern int		_nOMR_;		//重新识别模板时，用来识别OMR的密度值的阀值
extern int		_nSN_;		//重新识别模板时，用来识别ZKZH的密度值的阀值

extern int		_nDecompress_;	//解压试卷袋数量
extern int		_nRecog_;		//识别试卷数量
extern int		_nRecogPapers_;	//识别试卷袋
extern int		_nCompress_;	//压缩试卷袋数量
extern Poco::FastMutex _fmDecompress_;
extern Poco::FastMutex _fmRecog_;
extern Poco::FastMutex _fmRecogPapers_;
extern Poco::FastMutex _fmCompress_;

extern double	_dDoubtPer_;

extern std::string		g_strUploadUri;		//识别结果提交的uri地址

typedef struct _DecompressTask_
{
	//---------	参数传递，标识这次的解压任务需要识别的项目 --------
	bool bRecogOmr;			//识别OMR
	bool bRecogZkzh;		//识别准考证号
	bool bRecogElectOmr;	//识别选做题
	int	 nSendEzs;			//是否在识别完成时直接发送给EZS，不进行压缩
	int	 nNoNeedRecogVal;	////试卷袋不识别阀值：omr怀疑 + omr空 + SN空总数大于此阀值才进行重识别
	//---------

	int nTaskType;				//1-普通解压，2-区分试卷包到不同目录, 3-重新识别OMR和SN, 4-加载模板, 5-识别试卷包并统计识别正确率比例, 6-解压试卷袋中的特定文件
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
	std::string strDecompressDir;
	std::string strDecompressPaperFile;		//解压试卷袋中特定的文件，如S1，S2，nTaskType = 6 时有用
	_DecompressTask_()
	{
		nTaskType = 1;
		bRecogElectOmr = true;
		bRecogOmr = true;
		bRecogZkzh = true;
		nSendEzs = 2;
		nNoNeedRecogVal = 0;
	}
}DECOMPRESSTASK, *pDECOMPRESSTASK;
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//识别任务列表

extern Poco::FastMutex			g_fmDecompressLock;		//解压文件列表锁
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//解压文件列表


//----------------------------------------------

typedef struct  _StSearchTash_
{
	//---------	参数传递，标识这次的解压任务需要识别的项目 --------
	bool bRecogOmr;			//识别OMR
	bool bRecogZkzh;		//识别准考证号
	bool bRecogElectOmr;	//识别选做题
	int  nSendEzs;			//是否在识别完成时直接发送给EZS，不进行压缩		//0-使用压缩包方式，1-直接发送结果给ezs，2-不发送结果也不压缩试卷包，测试模式，看识别率
	int	 nNoNeedRecogVal;	////试卷袋不识别阀值：omr怀疑 + omr空 + SN空总数大于此阀值才进行重识别
	//---------
	int	 nSearchType;		//1-搜索目录下的所有pkg并进行全解压，2-搜索目录下指定的pkg
	std::string strSearchPath;
	//-------------参数传递, 针对nSearchType = 2 时有效
	std::string strSearchName;	//搜索指定的试卷袋文件，针对nSearchType = 2 时有效
	std::string strPaperName;	//需要解压试卷袋中的特定试卷文件，针对nSearchType = 2 时有效
	std::string strDecompressPath;	//文件解压的目的路径，针对nSearchType = 2 时有效
	//-------------
	_StSearchTash_()
	{
		nSearchType = 1;
		bRecogElectOmr = true;
		bRecogOmr = true;
		bRecogZkzh = true;
		nSendEzs = 2;
		nNoNeedRecogVal = 0;
	}
}ST_SEARCH, *pST_SEARCH;

typedef std::list<pST_SEARCH>	L_SearchTask;

typedef struct  _StPkgInfo_
{
	std::string strFileName;
	std::string strFilePath;
}ST_PKG_INFO, *pST_PKG_INFO;

typedef std::list<pST_PKG_INFO>	L_PKGINFO;

extern int g_nExitFlag;
extern	Poco::FastMutex	_fmAddPkg_;
extern	L_PKGINFO		_PkgInfoList_;
extern	Poco::Event		_E_StartSearch_;
extern	std::string		_strPkgSearchPath_;

extern	Poco::FastMutex	_fmSearchPathList_;
extern	L_SearchTask	_SearchPathList_;
//----------------------------------------------


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
	int			nChkFlag;			//此图片是否合法校验；在试卷袋里面的试卷图片，如果图片序号名称在Param.dat中不存在，则认为此试卷图片是错误图片，不進行圖片识别
	int			nQKFlag;			//缺考标识
	pMODEL		pModel;				//识别此学生试卷所用的模板
	void*		pPapers;			//所属的试卷袋信息
	void*		pSrcDlg;			//来源，来自哪个窗口，扫描or导入试卷窗口
	std::string strStudentInfo;		//学生信息	
	std::string strSN;
	std::string strMd5Key;


	//++
	int		nOmrDoubt;				//OMR怀疑的数量
	int		nOmrNull;				//OMR识别为空的数量
	int		nSnNull;				//准考证号识别为空的数量
	//--

	SNLIST				lSnResult;
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//识别的选做题OMR结果
	PIC_LIST	lPic;
	_PaperInfo_()
	{
		nChkFlag = 0;
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
	bool	bRecogOmr;			//直接发送识别结果时，是否发送OMR
	bool	bRecogZkzh;			//直接发送识别结果时，是否发送准考证号
	bool	bRecogElectOmr;		//直接发送识别结果时，是否发送选做题
	int		nSendEzs;			//是否在识别完成时直接发送给EZS，不进行压缩	//0-使用压缩包方式，1-直接发送结果给ezs，2-不发送结果也不压缩试卷包，测试模式，看识别率

	int		nPaperCount;				//试卷袋中试卷总数量(学生数)
	int		nRecogErrCount;				//识别错误试卷数量
	int		nTotalPics;

	int			nTotalPaper;		//总学生数量，从试卷袋文件夹读取
	int			nExamID;			//考试ID
	int			nSubjectID;			//科目ID
	int			nTeacherId;			//教师ID
	int			nUserId;			//用户ID

	int			nRecogMode;			//识别模式，1-简单模式(遇到问题校验点不停止识别)，2-严格模式
	//++统计信息
	int		nPkgOmrDoubt;				//OMR怀疑的数量					客户端传递的
	int		nPkgOmrNull;				//OMR识别为空的数量				客户端传递的
	int		nPkgSnNull;				//准考证号识别为空的数量		客户端传递的

	int		nOmrDoubt;				//OMR怀疑的数量					
	int		nOmrNull;				//OMR识别为空的数量				
	int		nSnNull;				//准考证号识别为空的数量		

	int		nOmrError_1;			//根据学生答案，第1种识别方法识别错误的值		统计时用
	int		nOmrError_2;			//根据学生答案，第1种识别方法识别错误的值		统计时用

	Poco::FastMutex	fmOmrStatistics;//omr统计锁
	Poco::FastMutex fmSnStatistics; //zkzh统计锁
	//--


	Poco::FastMutex	fmTask;			//对提交后端数据的任务(即:nTaskCounts)计数操作的锁
	int			nTaskCounts;		//该试卷袋的任务数，在提交完图片后，需要给后端提交图片数据、准考证号、OMR、选做题信息
									//该数据可以用来在试卷袋的所有任务处理完毕后进行试卷袋删除操作


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
		bRecogOmr = true;
		bRecogZkzh = true;
		bRecogElectOmr = true;
		nSendEzs = 2;
		nTaskCounts = 0;
		nTotalPics = 0;
		nPaperCount = 0;
		nRecogErrCount = 0;
		nOmrDoubt = 0;
		nOmrNull = 0;
		nSnNull = 0;
		nRecogPics = 0;
		nRecogMode = 2;

		nTotalPaper = 0;
		nExamID = -1;
		nSubjectID = -1;
		nTeacherId = -1;
		nUserId = -1;
		nOmrError_1 = 0;
		nOmrError_2 = 0;
		nPkgOmrDoubt = -1;
		nPkgOmrNull = -1;
		nPkgSnNull = -1;
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
	pPAPERSINFO	pPapers;
	std::string strSrcFilePath;
	std::string strCompressFileName;
	std::string strSavePath;
	std::string strExtName;
	_CompressTask_()
	{
		pPapers = NULL;
	}
}COMPRESSTASK, *pCOMPRESSTASK;
typedef std::list<pCOMPRESSTASK> COMPRESSTASKLIST;	//识别任务列表

extern Poco::FastMutex			g_fmCompressLock;		//压缩文件列表锁
extern COMPRESSTASKLIST			g_lCompressTask;		//解压文件列表

extern Poco::Event			g_eCompressThreadExit;


typedef struct _RecogResult_
{
	int			nTaskType;			//任务类型: 1-给img服务器提交图片，2-给后端提交图片数据, 3-提交OMR，4-提交ZKZH，5-提交选做题信息, 6-模板图片提交zimg服务器, 7-试卷袋只提交OMR、SN、选做信息
	int			nSendFlag;			//发送标示，1：发送失败1次，2：发送失败2次...
	Poco::Timestamp sTime;			//创建任务时间，用于发送失败时延时发送
	pPAPERSINFO pPapers;
	std::string strUri;
	std::string strResult;			//发送考场信息给后端服务器，当nTaskType = 2时有用
	std::string strEzs;				//当nTaskType = 2时有用
	_RecogResult_()
	{
		nTaskType = 0;
		nSendFlag = 0;
		pPapers = NULL;
	}
}ST_RECOG_RESULT_TASK, *p_ST_RECOG_RESULT_TASK;
typedef std::list<p_ST_RECOG_RESULT_TASK> RECOG_RESULT_LIST;

extern	Poco::FastMutex			g_fmReSult;		//发送线程获取任务锁
extern	RECOG_RESULT_LIST		g_lResultTask;	//发送任务列表



typedef struct _SendHttpTask_
{
	bool	bSendOmr;			//直接发送识别结果时，是否发送OMR
	bool	bSendZkzh;		//直接发送识别结果时，是否发送准考证号
	bool	bSendElectOmr;	//直接发送识别结果时，是否发送选做题

	int			nTaskType;			//任务类型: 1-给img服务器提交图片，2-给后端提交图片数据, 3-提交OMR，4-提交ZKZH，5-提交选做题信息, 6-模板图片提交zimg服务器, 7-试卷袋只提交OMR、SN、选做信息
	int			nSendFlag;			//发送标示，1：发送失败1次，2：发送失败2次...
	Poco::Timestamp sTime;			//创建任务时间，用于发送失败时延时发送
	pST_PicInfo pPic;
	pPAPERSINFO pPapers;
	std::string strUri;
	std::string strResult;			//发送考场信息给后端服务器，当nTaskType = 2时有用
	std::string strEzs;				//当nTaskType = 2时有用
	_SendHttpTask_()
	{
		bSendOmr = true;
		bSendZkzh = true;
		bSendElectOmr = true;
		nTaskType = 0;
		nSendFlag = 0;
		pPic = NULL;
		pPapers = NULL;
	}
}SEND_HTTP_TASK, *pSEND_HTTP_TASK;
typedef std::list<pSEND_HTTP_TASK> LIST_SEND_HTTP;

extern Poco::FastMutex		g_fmHttpSend;
extern LIST_SEND_HTTP		g_lHttpSend;		//发送HTTP任务列表



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
	char szVal[10];
	float fFirst;
	float fSecond;
	float fDiff;
}ST_ITEM_DIFF, *pST_ITEM_DIFF;
bool	SortByItemDiff(ST_ITEM_DIFF& item1, ST_ITEM_DIFF& item2);
bool	SortByItemDensity(pRECTINFO item1, pRECTINFO item2);
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
extern int		_nOmrDoubtStatistics_;		//识别怀疑总数
extern int		_nOmrNullStatistics_;		//识别为空总数
extern int		_nSnNullStatistics_;	//SN识别为空总数
extern int		_nAllOmrStatistics_;		//统计总数
extern int		_nAllSnStatistics_;			//SN统计总数
extern int		_nPkgDoubtStatistics_;	//原始试卷包识别怀疑总数
extern int		_nPkgOmrNullStatistics_;	//原始试卷包识别为空总数
extern int		_nPkgSnNullStatistics_;		//原始试卷包中SN识别为空总数
std::string calcStatistics(pPAPERSINFO pPapers);
//--
