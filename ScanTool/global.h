#pragma once
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <opencv2\opencv.hpp>

#include "Poco/Runnable.h"
#include "Poco/Exception.h"

#include "Poco/AutoPtr.h"  
#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"

#include "Poco/DirectoryIterator.h"
#include "Poco/File.h"
#include "Poco/Path.h"

#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"

#include "Poco/AutoPtr.h"  
#include "Poco/Util/IniFileConfiguration.h" 

#include "Poco/Random.h"

#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include "Poco/StreamCopier.h"

#include "Poco/Stopwatch.h"
#include "Poco/LocalDateTime.h"

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/URI.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/TCPServer.h"

#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/Crypto/X509Certificate.h"
#include "Poco/Crypto/CryptoStream.h"


//-------------------------------------
// #include "Poco/Data/Statement.h"
// #include "Poco/Data/RecordSet.h"
// #include "Poco/Data/SQLChannel.h"
// #include "Poco/Data/SessionFactory.h"
// #include "Poco/Data/SQLite/Connector.h"
// #include "Poco/Data/SQLite/Utility.h"
// #include "Poco/Data/SQLite/Notifier.h"
// #include "Poco/Data/SQLite/Connector.h"
// 
// #include "Poco/Nullable.h"
// #include "Poco/Data/Transaction.h"
// #include "Poco/Data/DataException.h"
// #include "Poco/Data/SQLite/SQLiteException.h"
// #include "Poco/Data/TypeHandler.h"
// #include "Poco/Data/SQLite/Connector.h"
// #include "Poco/Data/SQLite/Utility.h"
// #include "Poco/Data/SQLite/Notifier.h"
// #include "Poco/Data/SQLite/Connector.h"
//-------------------------------------

#include "zip.h"
#include "unzip.h"
#include "MyCodeConvert.h"
#include "./pdf2jpg/MuPDFConvert.h"
#include "modelInfo.h"
#include "StudentDef.h"
#include "zbar.h"   

//#define PIC_RECTIFY_TEST	//图像旋转纠正测试
#define WarpAffine_TEST		//仿射变换测试
#ifdef _DEBUG
	#define PaintOmrSnRect		//是否打印识别出来的OMR矩形
	#define PrintRecogLog		//打印识别日志
//	#define Test_ShowOriPosition	//测试打印模板坐标对应的原图坐标位置
	#define	 TEST_MODEL_NAME	//模板名称测试
//	#define	 TEST_SCAN_THREAD	//扫描线程测试
//	#define Test_Data			//测试数据，测试模式
	#define Test_TraceLog		//测试日志
#else	//release版本
	#define	 TEST_MODEL_NAME	//模板名称测试
//	#define PUBLISH_VERSION			//发布版本,发布版本不开放“试卷导入功能”
#endif
#ifndef WarpAffine_TEST
//	#define TriangleSide_TEST		//三边定位算法
	#ifndef TriangleSide_TEST
		#define TriangleCentroid_TEST	//三边质心算法
	#endif
#endif

#define Test_RecogOmr3			//第3种OMR识别方法测试

//+++++++++	选择版本 ++++++++++++++++++
//#define TO_WHTY							//给武汉天喻信息使用，无识别，只扫描上传
#ifndef TO_WHTY
	#define SHOW_GUIDEDLG				//显示引导界面,易考乐学自己用，显示引导界面.******** 此处不定义，则直接显示主界面	***********
#endif

//#define WH_CCBKS	//武汉楚才杯考试专用
//+++++++++++++++++++++++++++

#define USES_GETTHRESHOLD_ZTFB	//使用正态分布方式获取校验点的阀值，未定义时使用固定阀值进行二值化查找矩形
#define USES_FILE_ENC			//是否对文件使用加密
#define MSG_NOTIFY_UPDATE (WM_APP + 101)

#ifndef TO_WHTY
	#define USES_PWD_ZIP_UNZIP		//是否使用密码解压缩
	#define PAPERS_EXT_NAME	_T(".pkg")			//试卷袋压缩后的扩展名(保证有4位)
#else
	#define PAPERS_EXT_NAME	_T(".typkg")			//试卷袋压缩后的扩展名(保证有4位),通过服务器收完文件后重命名为.zip
#endif

#ifdef SHOW_GUIDEDLG
	//#define SHOW_LOGIN_MAINDLG			//是否在主界面上显示登录按钮
	//#define SHOW_MODELMGR_MAINDLG			//是否在主界面上显示模板管理按钮
	//#define SHOW_MODELMAKE_MAINDLG		//是否在主界面上显示模板制作按钮
	//#define SHOW_COMBOLIST_MAINDLG		//是否在主界面上显示下拉列表控件
	//#define SHOW_SCANALL_MAINDLG			//是否在主界面上显示整袋扫描按钮
	#ifndef PUBLISH_VERSION
		#define SHOW_PAPERINPUT_MAINDLG			//是否在主界面上显示试卷导入按钮
	#endif
#elif defined (TO_WHTY)
	#define SHOW_LOGIN_MAINDLG			//是否在主界面上显示登录按钮
	//#define SHOW_MODELMGR_MAINDLG			//是否在主界面上显示模板管理按钮
	//#define SHOW_MODELMAKE_MAINDLG		//是否在主界面上显示模板制作按钮
	//#define SHOW_COMBOLIST_MAINDLG		//是否在主界面上显示下拉列表控件
	//#define SHOW_SCANALL_MAINDLG			//是否在主界面上显示整袋扫描按钮
	//#define SHOW_PAPERINPUT_MAINDLG		//是否在主界面上显示试卷导入按钮
#else
	#define SHOW_LOGIN_MAINDLG				//是否在主界面上显示登录按钮
	#define SHOW_MODELMGR_MAINDLG			//是否在主界面上显示模板管理按钮
	//#define SHOW_MODELMAKE_MAINDLG		//是否在主界面上显示模板制作按钮
	//#define SHOW_COMBOLIST_MAINDLG		//是否在主界面上显示下拉列表控件
	//#define SHOW_SCANALL_MAINDLG			//是否在主界面上显示整袋扫描按钮
	#ifndef PUBLISH_VERSION
		#define SHOW_PAPERINPUT_MAINDLG			//是否在主界面上显示试卷导入按钮
	#endif
#endif

#define  MSG_ERR_RECOG	(WM_USER + 110)
#define  MSG_ZKZH_RECOG (WM_USER + 111)		//在准考证号识别完成时通知UI线程修改试卷列表，显示已经识别的ZKZH		2017.2.14
#define	 MSG_Pkg2Papers_OK (WM_USER + 112)	//从pkg恢复到Papers完成

#ifndef TO_WHTY
	#ifdef PUBLISH_VERSION
		#define SOFT_VERSION	_T("2.10612-1")
	#else
		#define SOFT_VERSION	_T("2.10612-1-Pir")		//-Pri
	#endif
#else
	#define SOFT_VERSION	_T("2.1-0323")
#endif
#define SYS_BASE_NAME	_T("YKLX-ScanTool")
#define SYS_GUIDE_NAME	_T("GuideDlg")


#define MAX_DLG_WIDTH	1024
#define MAX_DLG_HEIGHT	768

// #define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}
// #define SAFE_RELEASE_ARRY(pObj) if(pObj) {delete[] pObj; pObj = NULL;}

extern CString				g_strCurrentPath;
extern std::string			g_strPaperSavePath;
extern std::string			g_strModelSavePath;
extern std::string			g_strPaperBackupPath;	//试卷发送完成后的备份路径
extern Poco::Logger*		g_pLogger;
extern int					g_nExitFlag;

extern std::string			g_strEncPwd;				//文件加密解密密码
extern std::string			g_strCmdIP;
extern std::string			g_strFileIP;
extern int					g_nCmdPort;
extern int					g_nFilePort;

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

extern int		_nAnticlutterKernel_;	//识别同步头时防干扰膨胀腐蚀的核因子

extern int		_nGauseKernel_;			//高斯变换核因子
extern int		_nSharpKernel_;			//锐化核因子
extern int		_nCannyKernel_;			//轮廓化核因子
extern int		_nDilateKernel_;		//膨胀核因子
extern int		_nErodeKernel_;			//腐蚀核因子

extern int		g_nRecogGrayMin;		//灰度点(除空白点,OMR外)计算灰度的最小考试范围
extern int		g_nRecogGrayMax_White;	//空白点校验点计算灰度的最大考试范围
extern int		g_nRecogGrayMin_OMR;	//OMR计算灰度的最小考试范围
extern int		g_RecogGrayMax_OMR;		//OMR计算灰度的最大考试范围


extern int				g_nManulUploadFile;		//手动上传文件，通过qq这类的

extern bool				g_bCmdConnect;		//命令通道连接
extern bool				g_bFileConnect;		//文件通道连接

extern bool				g_bCmdNeedConnect;	//命令通道是否需要重连，用于通道地址信息修改的情况
extern bool				g_bFileNeedConnect;	//文件通道是否需要重连，用于通道地址信息修改的情况

extern bool				g_bShowScanSrcUI;	//是否显示原始扫描界面
extern int				g_nOperatingMode;	//操作模式，1--简易模式(遇到问题点不停止扫描)，2-严格模式(遇到问题点时立刻停止扫描)
extern int				g_nZkzhNull2Issue;	//识别到准考证号为空时，是否认为是问题试卷

#if 1
typedef struct _PicInfo_				//图片信息
{
	bool			bFindIssue;		//是否找到问题点
	int 			nRecoged;		//是否已经识别过, 0-未识别，1-正在识别，2-识别完成
	void*			pPaper;			//所属试卷的信息
	cv::Rect		rtFix;			//定点矩形
	std::string		strPicName;		//图片名称
	std::string		strPicPath;		//图片路径
	RECTLIST		lFix;			//定点列表
	RECTLIST		lNormalRect;	//识别出来的正常点位置
	RECTLIST		lIssueRect;		//识别出来的问题试卷的问题点位置，只要出现问题点就不进行下一页的识别(严格模式)，或者存储已经发行的问题点，但是继续后面的识别(简单模式)
// 	cv::Mat			matSrc;
// 	cv::Mat			matDest;
	_PicInfo_()
	{
		nRecoged = 0;
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
	bool		bModifyZKZH;		//准考证号人工修改标识
	bool		bRecogComplete;		//该学生已经识别完成
	bool		bReScan;			//重新扫描标识，在准考证号修改窗口中设置
	int			nQKFlag;			//缺考标识
	//++从Pkg恢复Papers时的参数
	int			nChkFlag;			//此图片是否合法校验；在试卷袋里面的试卷图片，如果图片序号名称在Param.dat中不存在，则认为此试卷图片是错误图片，不進行圖片识别
	//--
	pMODEL		pModel;				//识别此学生试卷所用的模板
	void*		pPapers;			//所属的试卷袋信息
	void*		pSrcDlg;			//来源，来自哪个窗口，扫描or导入试卷窗口
	std::string strStudentInfo;		//学生信息, S1、S2、S3...
	std::string strSN;				//识别出的考号、准考证号
	
	SNLIST				lSnResult;
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//识别的选做题OMR结果
	PIC_LIST	lPic;
	_PaperInfo_()
	{
		bIssuePaper = false;
		bModifyZKZH = false;
		bRecogComplete = false;
		bReScan = false;
		nQKFlag = 0;
		nChkFlag = 0;
		pModel = NULL;
		pPapers = NULL;
		pSrcDlg = NULL;
	}
	~_PaperInfo_()
	{
#if 1
		for (auto itSn : lSnResult)
		{
			pSN_ITEM pSNItem = itSn;
			SAFE_RELEASE(pSNItem);
		}
		lSnResult.clear();
#else
		SNLIST::iterator itSn = lSnResult.begin();
		for (; itSn != lSnResult.end();)
		{
			pSN_ITEM pSNItem = *itSn;
			itSn = lSnResult.erase(itSn);
			SAFE_RELEASE(pSNItem);
		}
#endif

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
	int		nPapersType;				//试卷类型，0-正常创建的试卷袋，在扫描时创建，1-从Pkg恢复到Papers时创建的
	int		nPaperCount;				//试卷袋中试卷总数量(学生数)
	int		nRecogErrCount;				//识别错误试卷数量

	//++统计信息
	int		nOmrDoubt;				//OMR怀疑的数量
	int		nOmrNull;				//OMR识别为空的数量
	int		nSnNull;				//准考证号识别为空的数量
	Poco::FastMutex	fmOmrStatistics;//omr统计锁
	Poco::FastMutex fmSnStatistics; //zkzh统计锁
	//--

	//++从Pkg恢复Papers时的参数
	int			nExamID;			//考试ID
	int			nSubjectID;			//科目ID
	int			nTeacherId;			//教师ID
	int			nUserId;			//用户ID
	//--

	Poco::FastMutex fmlPaper;			//对试卷列表读写锁
	Poco::FastMutex fmlIssue;			//对问题试卷列表读写锁
	std::string  strPapersName;			//试卷袋名称
	std::string	 strPapersDesc;			//试卷袋详细描述

	PAPER_LIST	lPaper;					//此试卷袋中试卷列表
	PAPER_LIST	lIssue;					//此试卷袋中识别有问题的试卷列表
	_PapersInfo_()
	{
		nPapersType = 0;
		nPaperCount = 0;
		nRecogErrCount = 0;
		nOmrDoubt = 0;
		nOmrNull = 0;
		nSnNull = 0;
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

typedef struct _RecogTask_
{
	int		nPic;						//此试卷属于模板的第几张
	pMODEL pModel;						//识别用的模板
	std::string strPath;	
	pST_PaperInfo	pPaper;				//需要识别的试卷
}RECOGTASK, *pRECOGTASK;
typedef std::list<pRECOGTASK> RECOGTASKLIST;	//识别任务列表
#endif

extern Poco::FastMutex		g_fmRecog;		//识别线程获取任务锁
extern RECOGTASKLIST		g_lRecogTask;	//识别任务列表

extern Poco::FastMutex		g_fmPapers;		//操作试卷袋列表的任务锁
extern PAPERS_LIST			g_lPapers;		//所有的试卷袋信息


//TCP命令任务
typedef struct _TcpTask_
{
	unsigned short usCmd;
	int		nPkgLen;
	char	szSendBuf[2500];
	_TcpTask_()
	{
		ZeroMemory(szSendBuf, 2500);
	}
}TCP_TASK, *pTCP_TASK;
typedef std::list<pTCP_TASK> TCP_TASKLIST;

extern Poco::FastMutex		g_fmTcpTaskLock;
extern TCP_TASKLIST			g_lTcpTask;

//文件上传任务
typedef struct _SendTask_
{
	int		nSendState;			//0-未发送，1-正在发送，2-发送完成，3-发送失败
	float	fSendPercent;
	std::string strFileName;
	std::string strPath;
	_SendTask_()
	{
		nSendState = 0;
		fSendPercent = 0.0;
	}
}SENDTASK, *pSENDTASK;
typedef std::list<pSENDTASK> SENDTASKLIST;	//识别任务列表

extern Poco::FastMutex		g_fmSendLock;
extern SENDTASKLIST			g_lSendTask;

typedef struct _ExamSubjects_
{
	int			nSubjID;		//考试科目ID
//	int			nSubjCode;		//考试科目代码
	std::string strSubjName;	//考试科目名称
	std::string strModelName;	//扫描所用模板名称
}EXAM_SUBJECT, *pEXAM_SUBJECT;
typedef std::list<EXAM_SUBJECT> SUBJECT_LIST;

typedef struct _examInfo_
{
	int			nExamID;			//考试ID
	int			nExamGrade;			//年级
	int			nExamState;			//考试状态
	std::string	strExamID;			//天喻版本, 考试ID
	std::string strExamName;		//考试名称
	std::string strExamTypeName;	//考试类型名称
	std::string strGradeName;		//年级名称
	SUBJECT_LIST lSubjects;			//科目列表
}EXAMINFO, *pEXAMINFO;
typedef std::list<EXAMINFO> EXAM_LIST;

extern Poco::FastMutex	g_lfmExamList;
extern EXAM_LIST	g_lExamList;

//报名库学生信息
#if 0
typedef struct _studentInfo_
{
	std::string strZkzh;
	std::string strName;		//gb2312
	std::string strClassroom;	//gb2312
	std::string strSchool;		//gb2312
}ST_STUDENT, *pST_STUDENT;
class CBmkStudent
{
	CBmkStudent(){}
	CBmkStudent(std::string& strZkzh, std::string& strName, std::string& strClassroom, std::string& strSchool) :_strZkzh(strZkzh), _strName(strName), _strClassroom(strClassroom), _strSchool(strSchool){}
	bool operator==(const CBmkStudent& other) const
	{
		return _strZkzh == other._strZkzh && _strName == other._strName && _strClassroom == other._strClassroom && _strSchool == other._strSchool;
	}

	bool operator < (const CBmkStudent& p) const
	{
		if (_strZkzh < p._strZkzh)
			return true;
		if (_strName < p._strName)
			return true;
		if (_strClassroom < p._strClassroom)
			return true;
		if (_strSchool < p._strSchool)
			return true;
	}

	const std::string& operator () () const
		/// This method is required so we can extract data to a map!
	{
		return _strZkzh;
	}
private:
	std::string _strZkzh;
	std::string _strName;
	std::string _strClassroom;
	std::string _strSchool;
};


namespace Poco {
	namespace Data {

		template <>
		class TypeHandler<ST_STUDENT>
		{
		public:
			static void bind(std::size_t pos, const ST_STUDENT& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
			{
				// the table is defined as Person (LastName VARCHAR(30), FirstName VARCHAR, Address VARCHAR, Age INTEGER(3))
				poco_assert_dbg(!pBinder.isNull());
				pBinder->bind(pos++, obj.strZkzh, dir);
				pBinder->bind(pos++, CMyCodeConvert::Gb2312ToUtf8(obj.strName), dir);
				pBinder->bind(pos++, CMyCodeConvert::Gb2312ToUtf8(obj.strClassroom), dir);
				pBinder->bind(pos++, CMyCodeConvert::Gb2312ToUtf8(obj.strSchool), dir);
			}

			static void prepare(std::size_t pos, const ST_STUDENT& obj, AbstractPreparator::Ptr pPrepare)
			{
				// no-op (SQLite is prepare-less connector)
			}

			static std::size_t size()
			{
				return 4;
			}

			static void extract(std::size_t pos, ST_STUDENT& obj, const ST_STUDENT& defVal, AbstractExtractor::Ptr pExt)
			{
				poco_assert_dbg(!pExt.isNull());
				std::string strZkzh;
				std::string strName;
				std::string strClassroom;
				std::string strSchool;

				if (pExt->extract(pos++, strZkzh))
					obj.strZkzh = strZkzh;
				else
					obj.strZkzh = defVal.strZkzh;

				if (pExt->extract(pos++, strName))
					obj.strName = CMyCodeConvert::Utf8ToGb2312(strName);
				else
					obj.strName = defVal.strName;

				if (pExt->extract(pos++, strClassroom))
					obj.strClassroom = CMyCodeConvert::Utf8ToGb2312(strClassroom);
				else
					obj.strClassroom = defVal.strClassroom;

				if (pExt->extract(pos++, strSchool))
					obj.strSchool = CMyCodeConvert::Utf8ToGb2312(strSchool);
				else
					obj.strSchool = defVal.strSchool;
			}

		private:
			TypeHandler();
			~TypeHandler();
			TypeHandler(const TypeHandler&);
			TypeHandler& operator=(const TypeHandler&);
		};
	}
}
#endif
typedef std::list<ST_STUDENT> STUDENT_LIST;	//报名库列表
extern STUDENT_LIST		g_lBmkStudent;	//报名库学生列表


typedef struct _CompressTask_
{
	bool	bDelSrcDir;				//自动删除原文件夹
	bool	bReleasePapers;			//是否解压完后自动是否试卷袋信息，即释放pPapersInfo内存数据
	int		nCompressType;			//压缩类型，1-压缩试卷袋，2-解压试卷袋
	pPAPERSINFO pPapersInfo;		//压缩的试卷袋文件
	std::string strSrcFilePath;
	std::string strCompressFileName;
	std::string strSavePath;
	std::string strExtName;
	_CompressTask_()
	{
		bDelSrcDir = true;
		bReleasePapers = true;
		nCompressType = 1;
		pPapersInfo = NULL;
	}
	~_CompressTask_()
	{
		if(bReleasePapers) SAFE_RELEASE(pPapersInfo);
	}
}COMPRESSTASK, *pCOMPRESSTASK;
typedef std::list<pCOMPRESSTASK> COMPRESSTASKLIST;	//识别任务列表

extern Poco::FastMutex			g_fmCompressLock;		//压缩文件列表锁
extern COMPRESSTASKLIST			g_lCompressTask;		//解压文件列表

extern Poco::Event			g_eTcpThreadExit;
extern Poco::Event			g_eSendFileThreadExit;
extern Poco::Event			g_eFileUpLoadThreadExit;
extern Poco::Event			g_eCompressThreadExit;

//模板文件信息
typedef struct _ModelFile
{
	std::string strModelName;		//gb2312
	std::string strModifyTime;
}ST_MODELFILE;

typedef struct stPlatformInfo
{
	std::string strPlatformUrl;
	std::string strPlatformCode;
	std::string strPlatformName;
	std::string strEncryption;
}ST_PLATFORMINFO, *pST_PLATFORMINFO;
typedef std::vector<pST_PLATFORMINFO> VEC_PLATFORM_TY;

int		GetRectInfoByPoint(cv::Point pt, CPType eType, pPAPERMODEL pPaperModel, RECTINFO*& pRc);
//bool	ZipFile(CString strSrcPath, CString strDstPath, CString strExtName = _T(".zip"));
//bool	UnZipFile(CString strZipPath);
pMODEL	LoadModelFile(CString strModelPath);		//加载模板文件
bool	SortByArea(cv::Rect& rt1, cv::Rect& rt2);		//按面积排序
bool	SortByPositionX(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionY(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionX2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionY2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionXYInterval(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByTH(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByOmrTH(OMR_QUESTION& rc1, OMR_QUESTION& rc2);
bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);
std::string calcFileMd5(std::string strPath);
void	CopyData(char *dest, const char *src, int dataByteSize, bool isConvert, int height);
bool	PicRectify(cv::Mat& src, cv::Mat& dst, cv::Mat& rotMat);
bool	FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//定点进行仿射变换
bool	FixwarpPerspective(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//定点透视变换
bool	FixWarpAffine2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//3个定点仿射变换，对90度旋转图像有效，目标矩形大小为原矩形最大值的正方形
bool	FixwarpPerspective2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//4个定点透视变换，对90度旋转图像有效，目标矩形大小为原矩形最大值的正方形
bool	PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	PicTransfer2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//针对有3个或者4个定点的变换，而且是对90度图像旋转，目标矩形大小为原矩形最大值的正方形
int		WriteRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char * regValue);
int		ReadRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char* & regValue);
bool	encString(std::string& strSrc, std::string& strDst);
bool	decString(std::string& strSrc, std::string& strDst);

bool	GetInverseMat(RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	GetPosition2(cv::Mat& inverseMat, cv::Rect& rtSrc, cv::Rect& rtDst);

void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);

//--------------	加载制卷工具导出的模板数据	-------------------
typedef struct _RectPos_
{
	int nIndex;
	cv::Rect rt;
}RECTPOS;
pMODEL	LoadMakePaperData(std::string strData);	//加载制卷工具导出的模板数据
bool Pdf2Jpg(std::string strPdfPath, std::string strBaseName);
bool InitModelRecog(pMODEL pModel);		//初始化制卷工具模板的识别参数
//-----------------------------------------------------------------

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


BOOL CheckProcessExist(CString &str, int& nProcessID);