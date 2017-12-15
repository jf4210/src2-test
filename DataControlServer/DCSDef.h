#pragma once
#include "global.h"
#include "Log.h"
#include "SysSet.h"
#include "MyCodeConvert.h"
#include "NetIoBuffer.h"
#include "MissionFactory.h"
#include "Net_Cmd_Protocol.h"
#include "NetUser.h"
#include "modelInfo.h"
#include "./pdf2jpg/MuPDFConvert.h"

//#include "TypeDef.h"

#ifdef _DEBUG
//	#define TEST_MODE	//测试模式，不向后端发送数据，本地模拟操作
//	#define TEST_FILE_PRESSURE	//文件上传压力测试
#endif

#define USES_FILE_CRYPTOGRAM			//是否对文件使用密码
//#define USE_POCO_UNZIP			//使用poco解压缩zip文件,解压无加密文件


//#define TO_WHTY							//给武汉天喻信息使用，无识别，只扫描上传
#ifdef TO_WHTY
	#define USE_POCO_UNZIP			//给武汉天喻信息使用,不使用加密压缩
#endif

#define DecompressTest		//解压测试，多线程解压

#ifndef TO_WHTY
	#define SOFT_VERSION "DataControlServer V2.61215-1"
	#define SOFT_VERSION4TY "DataControlServer for TY V2.1-1215"
#else
	#define SOFT_VERSION "DataControlServer for TY V2.1-0309"
#endif

#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}

extern CLog g_Log;
extern int	g_nExitFlag;

//===========================
extern int		g_nRecogGrayMin;		//灰度点(除空白点,OMR外)计算灰度的最小考试范围
extern int		g_nRecogGrayMax_White;	//空白点校验点计算灰度的最大考试范围
// extern int		g_nRecogGrayMin_OMR;	//OMR计算灰度的最小考试范围
// extern int		g_RecogGrayMax_OMR;		//OMR计算灰度的最大考试范围

extern float	_fFixThresholdPercent_;
extern float	_fHeadThresholdPercent_;	//同步头达到阀值的比例
extern float	_fABModelThresholdPercent_;
extern float	_fCourseThresholdPercent_;
extern float	_fGrayThresholdPercent_;
extern float	_fWhiteThresholdPercent_;

extern double	_dOmrThresholdPercent_Fix_;		//定点模式OMR识别可认为是选中的标准百分比
extern double	_dSnThresholdPercent_Fix_;		//定点模式SN识别可认为是选中的标准百分比
extern double	_dQKThresholdPercent_Fix_;		//定点模式QK识别可认为是选中的标准百分比
extern double	_dOmrThresholdPercent_Head_;	//同步头模式OMR识别可认为是选中的标准百分比
extern double	_dSnThresholdPercent_Head_;		//同步头模式SN识别可认为是选中的标准百分比
extern double	_dQKThresholdPercent_Head_;		//同步头模式QK识别可认为是选中的标准百分比

// extern double	_dCompThread_Fix_;
// extern double	_dDiffThread_Fix_;
// extern double	_dDiffExit_Fix_;
// extern double	_dCompThread_Head_;
// extern double	_dDiffThread_Head_;
// extern double	_dDiffExit_Head_;
// extern int		_nThreshold_Recog2_;	//第2中识别方法的二值化阀值
// extern double	_dCompThread_3_;		//第三种识别方法
// extern double	_dDiffThread_3_;
// extern double	_dDiffExit_3_;
// extern double	_dAnswerSure_;		//可以确认是答案的最大灰度

extern int		_nGaussKernel_;
extern int		_nSharpKernel_;
extern int		_nCannyKernel_;
extern int		_nDilateKernel_;
extern int		_nErodeKernel_;
extern int		_nFixVal_;
extern int		_nHeadVal_;				//水平垂直同步头的阀值
extern int		_nABModelVal_;
extern int		_nCourseVal_;
extern int		_nQK_CPVal_;
extern int		_nGrayVal_;
extern int		_nWhiteVal_;
extern int		_nOMR_;		//重新识别模板时，用来识别OMR的密度值的阀值
extern int		_nSN_;		//重新识别模板时，用来识别ZKZH的密度值的阀值
//===========================

typedef std::map<std::string, std::string> MAP_PIC_ADDR;
extern Poco::FastMutex _mapPicAddrLock_;
extern MAP_PIC_ADDR _mapPicAddr_;

typedef std::map<std::string, CNetUser*>	MAP_USER;
extern Poco::FastMutex	_mapUserLock_;
extern MAP_USER		_mapUser_;					//用户映射

typedef struct _ModelInfo_
{
	int		nExamID;
	int		nSubjectID;
	pMODEL		pModel;
	pST_MODELINFO pUploadModelInfo;			//请求上传模板时上传的模板信息
	CNetUser* pUser;
	std::string strMd5;
	std::string strName;
	std::string strPath;
	_ModelInfo_()
	{
		nExamID = 0;
		nSubjectID = 0;
		pModel = NULL;
		pUploadModelInfo = NULL;
		pUser = NULL;
	}
	~_ModelInfo_()
	{
		SAFE_RELEASE(pModel);
		SAFE_RELEASE(pUploadModelInfo);
	}
}MODELINFO,*pMODELINFO;

typedef std::map<std::string, pMODELINFO> MAP_MODEL;
extern	Poco::FastMutex	_mapModelLock_;
extern	MAP_MODEL	_mapModel_;

typedef struct _DecompressTask_
{
	int		nTimes;			//解压的次数
	int		nType;			//解压类型，1-试卷袋pkg包，2-模板mod，3-单独解压papersInfo.dat，然后发送图像地址信息，此时需要完全解压，因为后面在发送成功后需要提交OMR等信息
							//4-单独解压papersInfo.dat，然后发送OMR，5-单独解压papersInfo.dat，然后发送ZKZH，6-单独解压papersInfo.dat，然后发送选做信息
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
	std::string strTransferData;		//透传数据
	std::string strTransferFilePath;	//透传数据，TXT文件路径, utf8
	_DecompressTask_()
	{
		nType = 1;
		nTimes = 0;
	}
}DECOMPRESSTASK, *pDECOMPRESSTASK;
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//识别任务列表

extern Poco::FastMutex			g_fmDecompressLock;		//解压文件列表锁
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//解压文件列表


// 图片文件
typedef struct _Pic_
{
	bool		bUpLoadFlag;		//上传http服务器成功标志
	int			nPicW;				//图片宽，设置模板信息的时候有用
	int			nPicH;				//图片高，设置模板信息的时候有用
	std::string strFileName;
	std::string strFilePath;
	std::string strHashVal;			//上传http后返回的hash
	_Pic_()
	{
		bUpLoadFlag = false;
		nPicW = 0;
		nPicH = 0;
	}
}PIC_DETAIL, *pPIC_DETAIL;
typedef std::list<pPIC_DETAIL> LIST_PIC_DETAIL;

//试卷,针对考生
typedef struct _Paper_
{
	int			nChkFlag;	//此图片是否合法校验；在试卷袋里面的试卷图片，如果图片序号名称在Param.dat中不存在，则认为此试卷图片是错误图片，不往zimg图像服务器提交
	int			nQkFlag;	//缺考标识,0-未缺考, 1-缺考
	int			nWjFlag;	//违纪标识,0-未违纪，1-违纪
	int			nHasElectOmr;	//是否有多选题
	int			nStandardAnswer;//当前试卷表示：0-正常试卷，1-Omr标答，2-主观题标答
	int			nIssueFlag;		//问题标识，0 - 正常试卷，完全机器识别正常的，无人工干预，1 - 正常试卷，扫描员手动修改过，2-准考证号为空，扫描员没有修改，3-扫描员标识了需要重扫的试卷。
	std::string strName;	//识别出来的考生名称S1、S2、。。。
	std::string strMd5Key;	//给后端的MD5--密号
	std::string strZkzh;	//识别出来的考生序列号、准考证号
	std::string strSnDetail;	//zkzh的详细识别情况，从文件读取
	std::string strOmrDetail;	//同上
	std::string strElectOmrDetail;	//选做题详细识别情况

	LIST_PIC_DETAIL lPic;

// 	SNLIST				lSnResult;
// 	OMRRESULTLIST		lOmrResult;
	_Paper_()
	{
		nChkFlag = 0;
		nQkFlag = 0;
		nWjFlag = 0;
		nHasElectOmr = 0;
		nIssueFlag = 0;
		nStandardAnswer = 0;
	}
	~_Paper_()
	{
		LIST_PIC_DETAIL::iterator it = lPic.begin();
		for (; it != lPic.end();)
		{
			pPIC_DETAIL pObj = *it;
			SAFE_RELEASE(pObj);
			it = lPic.erase(it);
		}
	}
}PAPER_INFO, *pPAPER_INFO;
typedef std::list<pPAPER_INFO> LIST_PAPER_INFO;
//试卷袋
typedef struct _Papers_
{
	int			nTaskCounts;		//该试卷袋的任务数，在提交完图片后，需要给后端提交图片数据、准考证号、OMR、选做题信息
									//该数据可以用来在试卷袋的所有任务处理完毕后进行试卷袋删除操作
	int			nTotalPics;			//总的图片数
	int			nUpLoadSuccess;
	int			nUpLoadFail;
	int			nTotalPaper;		//总学生数量，从试卷袋文件夹读取
	int			nQk;				//缺考学生数量，从试卷袋文件夹读取
	int			nExamID;			//考试ID
	int			nSubjectID;			//科目ID
	int			nTeacherId;			//教师ID
	int			nUserId;			//用户ID

	//++
	int		nOmrDoubt;				//OMR怀疑的数量
	int		nOmrNull;				//OMR识别为空的数量
	int		nSnNull;				//准考证号识别为空的数量
	//--
	int			nStandardAnswer;	//当前试卷袋表示：0-正常试卷，1-Omr标答，2-主观题标答

	Poco::FastMutex fmResultState;	//对nResultSendState结果状态修改的锁
	int		nResultSendState;		//试卷袋的图片MD5地址提交、OMR、准考证号、选做题结果发送给后端EZS的完成状态，
									//采用位标识，从右往左数，第1位--图片地址提交完成，第2位--OMR结果提交完成，第3位--ZKZH提交完成，第4位--选做题信息提交完成

	//++	将需要发送的试卷袋图片地址信息、OMR、ZKZH、选做题信息都保存下来，在发生错误需要记录文件时，省去重新取值的过程	2017.1.19
	std::string strSendPicsAddrResult;
	std::string strSendOmrResult;
	std::string strSendZkzhResult;
	std::string strSendElectOmrResult;
	//--

	std::string	strUploader;		//上传者
	std::string strEzs;				//上传给后端服务器用，--cookie
	std::string strDesc;			//从试卷袋文件夹读取
	std::string strPapersName;		//试卷袋名称
	std::string strPapersPath;		//试卷袋路径，解压后的
	std::string strSrcPapersFileName;	//试卷袋文件名称，原始压缩包名称	gb2312
	std::string strSrcPapersPath;	//试卷袋文件路径，原始压缩包路径
	Poco::FastMutex	fmNum;			//对上传结果的计数操作的锁
	Poco::FastMutex	fmTask;			//对提交后端数据的任务(即:nTaskCounts)计数操作的锁
	LIST_PAPER_INFO lPaper;
	_Papers_()
	{
		nTaskCounts = 0;
		nStandardAnswer = 0;
		nUserId = -1;
		nTeacherId = -1;
		nTotalPics = 0;
		nUpLoadFail = 0;
		nUpLoadSuccess = 0;
		nTotalPaper = 0;
		nQk = 0;
		nOmrDoubt = 0;
		nOmrNull = 0;
		nSnNull = 0;
		nResultSendState = 0;
	}
	~_Papers_()
	{
		LIST_PAPER_INFO::iterator it = lPaper.begin();
		for (; it != lPaper.end();)
		{
			pPAPER_INFO pObj = *it;
			SAFE_RELEASE(pObj);
			it = lPaper.erase(it);
		}
	}
}PAPERS_DETAIL, *pPAPERS_DETAIL;
typedef std::list<pPAPERS_DETAIL> LIST_PAPERS_DETAIL;

extern Poco::FastMutex		g_fmPapers;		
extern LIST_PAPERS_DETAIL	g_lPapers;		//试卷袋列表


typedef struct _SendHttpTask_
{
	int			nTaskType;			//任务类型: 1-给img服务器提交图片，2-给后端提交图片数据, 3-提交OMR，4-提交ZKZH，5-提交选做题信息, 6-模板图片提交zimg服务器, 7-试卷袋只提交OMR、SN、选做信息
	int			nSendFlag;			//发送标示，1：发送失败1次，2：发送失败2次...
	Poco::Timestamp sTime;			//创建任务时间，用于发送失败时延时发送
	pPIC_DETAIL pPic;
	pPAPERS_DETAIL pPapers;
	std::string strUri;
	std::string strResult;			//发送考场信息给后端服务器，当nTaskType = 2时有用
	std::string strEzs;				//当nTaskType = 2时有用
	_SendHttpTask_()
	{
		nTaskType = 0;
		nSendFlag = 0;
		pPic	= NULL;
		pPapers = NULL;
	}
}SEND_HTTP_TASK, *pSEND_HTTP_TASK;
typedef std::list<pSEND_HTTP_TASK> LIST_SEND_HTTP;

extern Poco::FastMutex		g_fmHttpSend;
extern LIST_SEND_HTTP		g_lHttpSend;		//发送HTTP任务列表

typedef struct _ScanReqTask_
{
	int			nExamID;
	int			nSubjectID;
	int			nTeacherID;
	int			nLoginFlag;		//登录第3方平台标识，要求先登录第3方，成功后再登录易考乐学后台平台, 0-第一次登录，1-第一次登录第3方平台成功
	CNetUser*	pUser;
	std::string strRequest;
	std::string strUri;
	std::string strMsg;
	std::string strEzs;
	std::string strUser;
	std::string strPwd;
	std::string strExamName;
	std::string strSubjectName;
	_ScanReqTask_()
	{
		pUser = NULL;
		nExamID = 0;
		nSubjectID = 0;
		nTeacherID = 0;
		nLoginFlag = 0;
	}
}SCAN_REQ_TASK, *pSCAN_REQ_TASK;
typedef std::list<pSCAN_REQ_TASK> LIST_SCAN_REQ;

extern Poco::FastMutex		g_fmScanReq;
extern LIST_SCAN_REQ		g_lScanReq;		//扫描端请求任务列表

typedef std::map<std::string, pPAPERS_DETAIL> MAP_RESEND_PKG;
extern Poco::FastMutex	_mapResendPkgLock_;
extern MAP_RESEND_PKG _mapResendPkg_;

typedef struct _UserSession_
{
	int nChkHeartPkgFailTimes;	//检测心跳包失败次数
	int nTeacherID;
	int nUserID;
	CNetUser* pUser;
	Poco::Timestamp tmStamp;
	std::string strUser;
	_UserSession_()
	{
		nChkHeartPkgFailTimes = 0;
		nTeacherID = 0;
		nUserID = 0;
	}
}ST_USER_SESSION, *pST_USER_SESSION;
typedef std::map<std::string, ST_USER_SESSION> MAP_SESSION;		//用户与session映射表，与后端进行心跳，维持session存活
extern Poco::FastMutex _mapSessionLock_;
extern MAP_SESSION _mapSession_;

bool encString(std::string& strSrc, std::string& strDst);
bool decString(std::string& strSrc, std::string& strDst);

std::string calcFileMd5(std::string strPath);