#pragma once
#include "global.h"
#include "Log.h"
#include "SysSet.h"
#include "MyCodeConvert.h"
#include "NetIoBuffer.h"
#include "MissionFactory.h"
#include "Net_Cmd_Protocol.h"
#include "NetUser.h"

//#include "TypeDef.h"

#ifdef _DEBUG
	#define TEST_MODE	//测试模式，不向后端发送数据，本地模拟操作
#endif

#define USES_FILE_DNC			//是否对文件使用解密
//#define USE_POCO_UNZIP	//使用poco解压缩zip文件

#define SOFT_VERSION "DataControlServer V1.0"


#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}

extern CLog g_Log;
extern int	g_nExitFlag;


typedef std::map<std::string, CNetUser*>	MAP_USER;
extern Poco::FastMutex	_mapUserLock_;
extern MAP_USER		_mapUser_;					//用户映射

typedef struct _ModelInfo_
{
	int		nExamID;
	int		nSubjectID;
	std::string strMd5;
	std::string strName;
	std::string strPath;
	_ModelInfo_()
	{
		nExamID = 0;
		nSubjectID = 0;
	}
}MODELINFO,*pMODELINFO;

typedef std::map<std::string, pMODELINFO> MAP_MODEL;
extern	Poco::FastMutex	_mapModelLock_;
extern	MAP_MODEL	_mapModel_;

typedef struct _DecompressTask_
{
	std::string strFileName;
	std::string strFilePath;
}DECOMPRESSTASK, *pDECOMPRESSTASK;
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//识别任务列表

extern Poco::FastMutex			g_fmDecompressLock;		//解压文件列表锁
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//解压文件列表

#if 0
typedef struct _RectInfo_
{
	cv::Rect	rt;
}RECTINFO,*pRECTINFO;
typedef std::list<RECTINFO> RECTLIST;

typedef struct _SN_
{
	int nItem;			//第几位数
	int nRecogVal;		//识别的结果
	cv::Rect rt;
	_SN_()
	{
		nItem = -1;
		nRecogVal = -1;
	}
}SN_ITEM, *pSN_ITEM;
typedef std::list<SN_ITEM> SNLIST;

typedef struct _OmrResult_
{
	int		nTH;				//题号
	int		nSingle;			//0-单选，1-多选
	int		nDoubt;				//0-无怀疑, 1-有怀疑
	std::string strRecogVal;	//识别结果：A、B、C...
	std::string strRecogVal2;
	RECTLIST	lSelAnswer;				//选项列表
	_OmrResult_()
	{
		nDoubt = 0;
		nTH = -1;
		nSingle = 0;
	}
}OMR_RESULT, *pOMR_RESULT;
typedef std::list<OMR_RESULT> OMRRESULTLIST;
#endif

// 图片文件
typedef struct _Pic_
{
	bool		bUpLoadFlag;		//上传http服务器成功标志
	std::string strFileName;
	std::string strFilePath;
	std::string strHashVal;			//上传http后返回的hash
	_Pic_()
	{
		bUpLoadFlag = false;
	}
}PIC_DETAIL, *pPIC_DETAIL;
typedef std::list<pPIC_DETAIL> LIST_PIC_DETAIL;

//试卷,针对考生
typedef struct _Paper_
{
	int			nQkFlag;	//缺考标识,0-未缺考, 1-缺考
	std::string strName;	//识别出来的考生名称S1、S2、。。。
	std::string strMd5Key;	//给后端的MD5--密号
	std::string strZkzh;	//识别出来的考生序列号、准考证号
	std::string strSnDetail;	//zkzh的详细识别情况，从文件读取
	std::string strOmrDetail;	//同上

	LIST_PIC_DETAIL lPic;

// 	SNLIST				lSnResult;
// 	OMRRESULTLIST		lOmrResult;
	_Paper_()
	{
		nQkFlag = 0;
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
	int			nTotalPics;			//总的图片数
	int			nUpLoadSuccess;
	int			nUpLoadFail;
	int			nTotalPaper;		//总学生数量，从试卷袋文件夹读取
	int			nQk;				//缺考学生数量，从试卷袋文件夹读取
	int			nExamID;			//考试ID
	int			nSubjectID;			//科目ID
	int			nTeacherId;			//教师ID
	int			nUserId;			//用户ID
	std::string	strUploader;		//上传者
	std::string strEzs;				//上传给后端服务器用，--cookie
	std::string strDesc;			//从试卷袋文件夹读取
	std::string strPapersName;
	std::string strPapersPath;
	Poco::FastMutex	fmNum;			//对上传结果的计数操作的锁
	LIST_PAPER_INFO lPaper;
	_Papers_()
	{
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
	int			nTaskType;			//任务类型: 1-给img服务器提交图片，2-给后端提交图片数据, 3-提交OMR，4-提交ZKZH
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
	CNetUser*		pUser;
	std::string strRequest;
	std::string strUri;
	std::string strMsg;
	std::string strEzs;
	std::string strUser;
	std::string strPwd;
	_ScanReqTask_()
	{
		pUser = NULL;
	}
}SCAN_REQ_TASK, *pSCAN_REQ_TASK;
typedef std::list<pSCAN_REQ_TASK> LIST_SCAN_REQ;

extern Poco::FastMutex		g_fmScanReq;
extern LIST_SCAN_REQ		g_lScanReq;		//扫描端请求任务列表

