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


// //#define TO_WHTY							//给武汉天喻信息使用，无识别，只扫描上传
// #ifdef TO_WHTY
// 	#define USE_POCO_UNZIP			//给武汉天喻信息使用,不使用加密压缩
// #endif

#define DecompressTest		//解压测试，多线程解压

#define SOFT_VERSION "DataControlServer V1.60111"


#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}

extern CLog g_Log;
extern int	g_nExitFlag;

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
	int		nType;			//解压类型，1-试卷袋pkg包，2-模板mod
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
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
	int			nHasElectOmr;	//是否有多选题
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
		nHasElectOmr = 0;
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
	int		nResultSendState;		//试卷袋的图片MD5地址提交、OMR、准考证号、选做题结果发送给后端EZS的完成状态，
									//0-未完成，1-图片地址提交完成，2-OMR结果提交完成，3-ZKZH提交完成，4-选做题信息提交完成

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
		nUserId = -1;
		nTeacherId = -1;
		nTotalPics = 0;
		nUpLoadFail = 0;
		nUpLoadSuccess = 0;
		nTotalPaper = 0;
		nQk = 0;
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
	}
}SCAN_REQ_TASK, *pSCAN_REQ_TASK;
typedef std::list<pSCAN_REQ_TASK> LIST_SCAN_REQ;

extern Poco::FastMutex		g_fmScanReq;
extern LIST_SCAN_REQ		g_lScanReq;		//扫描端请求任务列表

bool encString(std::string& strSrc, std::string& strDst);
bool decString(std::string& strSrc, std::string& strDst);

std::string calcFileMd5(std::string strPath);