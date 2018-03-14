#pragma once
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <opencv2\opencv.hpp>
#include "BasePocoHead.h"

#include "zip.h"
#include "unzip.h"
#include "MyCodeConvert.h"
//#include "./pdf2jpg/MuPDFConvert.h"
#include "modelInfo.h"

#include "zbar.h"
#include "StudentDef.h"
#include "FileUpLoad.h"

//#define PIC_RECTIFY_TEST	//图像旋转纠正测试
#define WarpAffine_TEST		//仿射变换测试
#ifdef _DEBUG
	#define PaintOmrSnRect		//是否打印识别出来的OMR矩形
	#define PrintRecogLog		//打印识别日志
	#define PrintAllOmrVal		//打印所有OMR选项值
	#define PrintWordRecogPoint		//打印通过文字定位时计算出来的矩形坐标点位置
//	#define Test_ShowOriPosition	//测试打印模板坐标对应的原图坐标位置
//	#define Test_Data			//测试数据，测试模式
//	#define TEST_TIP_SHOW		//提示信息显示测试
	#define TEST_MULTI_SENDER	//文件发送测试，用多个地址发送测试
	#define TEST_PAPERS_INPUT	//试卷袋导入功能测试
	#define TEST_MODEL_ROTATION	//模板旋转测试
	
	#define TEST_GRAY_WHITE		//空白处灰度测试
//	#define TEST_FAST_SCAN		//快速扫描模式，在扫描线程中不写文件
	#define TEST_PAGINATION		//页码测试，多页试卷的情况，多页A3、A4
	
	#ifdef TEST_PAGINATION
		#define	TEST_EXCEPTION_DLG	//新的异常处理窗口测试
	#endif
	#define TEST_ModelMgr		//使用模板保存测试类保存模板

	#define TEST_New_RecogClass	//使用新识别类测试
#else	//release版本
	#define TEST_MULTI_SENDER	//文件发送测试，用多个地址发送测试
	#define PUBLISH_VERSION			//发布版本,发布版本不开放“试卷导入功能”

	#define TEST_MODEL_ROTATION	//模板旋转测试
//	#define USE_TESSERACT		//使用Tesseract进行数字汉字识别

 	#define TEST_PAPERS_INPUT	//试卷袋导入功能测试
// 	#define TEST_PAGINATION		//页码测试，多页试卷的情况，多页A3、A4
#endif

#ifdef USE_TESSERACT
	#include "tesseract/baseapi.h"
	#include "leptonica/allheaders.h"
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
	#define PAPERS_EXT_NAME_4TY _T(".typkg")		//天喻版本，通过服务器收完文件后重命名为.zip
#else
	#define PAPERS_EXT_NAME	_T(".typkg")			//试卷袋压缩后的扩展名(保证有4位),通过服务器收完文件后重命名为.zip
#endif

#define  MSG_ERR_RECOG	(WM_USER + 110)
#define  MSG_ZKZH_RECOG (WM_USER + 111)		//在准考证号识别完成时通知UI线程修改试卷列表，显示已经识别的ZKZH		2017.2.14
#define	 MSG_Pkg2Papers_OK (WM_USER + 112)	//从pkg恢复到Papers完成

#ifndef TO_WHTY
	#ifdef PUBLISH_VERSION
		#define SOFT_VERSION	_T("v2.8")
	#else
		#define SOFT_VERSION	_T("v2.8")		//-Pri
	#endif
#else
	#define SOFT_VERSION	_T("v2.1")
#endif
#define SYS_BASE_NAME	_T("慧学扫描客户端")		//智能阅卷扫描客户端	//智能评阅扫描客户端
#define SYS_GUIDE_NAME	_T("GuideDlg")


#define MAX_DLG_WIDTH	1024
#define MAX_DLG_HEIGHT	768

#ifdef USE_TESSERACT
	#ifdef _DEBUG
		#pragma comment(lib, "libtesseract304d.lib")
	#else
		#pragma comment(lib, "libtesseract304.lib")
	#endif
#endif
// #define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}
// #define SAFE_RELEASE_ARRY(pObj) if(pObj) {delete[] pObj; pObj = NULL;}


//++标题信息
extern std::string _strCopyright_;
extern std::string _strMainTitle_;
extern std::string _strLitteTitle_;
//--


//================================================================================
/*
高厉害考试模式：
1、需要检查实际扫描试卷数与要求扫描数是否一致
2、扫描完成后按准考证号排序
3、异常处理界面，不能手动输入准考证号，只能从数据库查询
*/
extern int				g_nHighSevereMode;
extern int				g_nDefStudentsInKC;	//默认考场中考生数量
//================================================================================

extern int				_nUseOcrRecogSn_;		//使用OCR识别准考证号

													//++登录信息
extern bool	_bHandModel_;			//是否是手阅模式，手阅模式是天喻用
extern bool	_bLogin_;				//是否已经登录
extern std::string _strUserName_;	//登录用户名
extern std::string _strNickName_;	//用户昵称
extern std::string _strPwd_;		//密码
extern std::string _strEzs_;		//后端需要的EZS
extern int _nTeacherId_;			//教师ID
extern int _nUserId_;				//用户ID
extern std::string _strPersonID_;	//手阅模式，天喻专用
extern std::string _strSchoolID_;	//手阅模式，天喻专用
extern int _nPicNum4Ty_;			//手阅模式，模板图片数量
//--
extern bool		_bGetBmk_;			//是否获得当前科目报名库
extern int					_nReocgThreads_;		//识别线程数量

//++事件定义
extern Poco::Event			g_eGetExamList;		//获取考试列表事件
extern Poco::Event			g_eGetBmk;			//获取报名库事件
extern Poco::Event			g_eDownLoadModel;	//下载模板完成状态
extern Poco::Event			g_eGetModelPic;		//下载模板图片状态
//--
extern int					g_nDownLoadModelStatus;		//下载模板的状态	0-未下载，初始化，1-模板下载中，2-下载成功，3-本地存在此文件，不需要下载, -1-服务器此科目模板不存在, -2-服务器读取文件失败
extern int					_nScanStatus_;				//扫描进度 0-未扫描，1-正在扫描，2-扫描完成, 3-扫描中止, -1--连接扫描仪失败, -2--加载扫描仪失败, -3--扫描失败, -4-扫描取消
extern int					_nScanCount_;				//扫描计数器，当前已扫描多少份
extern int					_nScanPaperCount_;			//当前已经扫描人数，从软件启动开始计数
extern int					_nGetModelPic_;				//获取模板图片状态，0-未获取，1-正在获取(下载)， 2-获取模板图片成功，3-服务器无模板图片，4-服务器读取失败
extern int					_nDoubleScan_;				//是否双面扫描，0-单面， 1-双面，用于双面图像调换时处理
extern int					_nScanAnswerModel_;			//0-扫描试卷， 1-扫描Omr答案， 2-扫描主观题答案

extern CString				g_strCurrentPath;
extern std::string			g_strPaperSavePath;
extern std::string			g_strModelSavePath;
extern std::string			g_strPaperBackupPath;	//试卷发送完成后的备份路径
extern std::string			g_strFileVersion;		//文件版本，2.7.12.8
extern Poco::Logger*		g_pLogger;
extern int					g_nExitFlag;

extern std::string			g_strEncPwd;				//文件加密解密密码
extern std::string			g_strCmdIP;
extern std::string			g_strFileIP;
extern int					g_nCmdPort;
extern int					g_nFilePort;
extern std::string			g_strFileIp4HandModel;		//手阅模式时文件服务器地址
extern int					g_nFilePort4HandModel;		//手阅模式时文件服务器端口

extern double	_dAnswerSure_DensityFix_;	//密度算法确定为答案的比例
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
extern int				g_nReUploadWaitTime;	//上传文件完成后等待服务器响应时间，超过此时间则重上传

extern bool				g_bCmdConnect;		//命令通道连接
extern bool				g_bFileConnect;		//文件通道连接

extern bool				g_bCmdNeedConnect;	//命令通道是否需要重连，用于通道地址信息修改的情况
extern bool				g_bFileNeedConnect;	//文件通道是否需要重连，用于通道地址信息修改的情况

extern bool				g_bShowScanSrcUI;	//是否显示原始扫描界面
extern int				g_nOperatingMode;	//操作模式，1--简易模式(遇到问题点不停止扫描)，2-严格模式(遇到问题点时立刻停止扫描)
extern bool				g_bModifySN;		//是否允许修改准考证号
extern int				g_nZkzhNull2Issue;	//识别到准考证号为空时，是否认为是问题试卷

//窗口类型，当前显示的窗口
typedef enum _eDlgType_
{
	DLG_Login = 0,			//登录窗口
	DLG_ExamMgr,			//考试列表管理窗口
	DLG_DownloadModle,		//下载报名库与模板窗口
	DLG_ScanStart,			//扫描开始窗口
	Dlg_ScanProcess,		//扫描进度窗口
	Dlg_ScanRecordMgr,		//扫描记录管理窗口，报名库信息展示窗口
	Dlg_NewMakeModel,		//模板制作窗口
	Dlg_PapersInput			//试卷袋导入窗口
}E_DLG_TYPE;
extern E_DLG_TYPE		_eCurrDlgType_;	//当前显示的窗口，弹出窗口不算

//从扫描仪获取的图像信息
typedef struct _ScanPic
{
	int nOrder;					//考生的试卷顺序，1.jpg、2.jpg ...(S1_1,S1_2,S1_3中的1、2、3)
	int nStudentID;				//考生的索引，S1、S2、S3 ...
	int nModelPicID;			//这张试卷在模板上属于第几页试卷, 从0开始
	void*	pNotifyDlg;			//处理完成的通知窗口
	void* pParentScanPaper;		//所属的扫描试卷
	std::string strPicName;
	std::string strPicPath;		//gb2312
	cv::Mat mtPic;
}ST_SCAN_PIC, *pST_SCAN_PIC;
typedef struct _ScanPaper
{
	bool bDoubleScan;			//是否双面扫描，单面扫描时，pScanPic2不可用
	bool bCanRecog;				//是否可以进行识别，使用多页模式时生效，在根据页码查找这张试卷属于模板的第几张试卷失败，需要人工确认，之后再识别
	int  nSrcDlgType;			//0-来自扫描线程的数据，1-来自试卷导入窗口的数据
	int  nPaperID;				//扫描的第几张试卷
	int  nModelPaperID;			//这张试卷在模板上属于第几张试卷
	void* pPapersInfo;			//所属试卷袋
	std::vector<pST_SCAN_PIC> vecScanPic;
	_ScanPaper()
	{
		bDoubleScan = true;
		bCanRecog = true;
		nSrcDlgType = 0;
		nPaperID = 0;
		nModelPaperID = 0;
		pPapersInfo = NULL;
	}
	~_ScanPaper()
	{
		for (int i = 0; i < vecScanPic.size(); i++)
		{
			pST_SCAN_PIC pScanPic = vecScanPic[i];
			SAFE_RELEASE(pScanPic);
		}
	}
}ST_SCAN_PAPER, *pST_SCAN_PAPER;
typedef std::list<pST_SCAN_PAPER> SCAN_PAPER_LIST;			//从扫描仪获取的图像信息的列表
extern Poco::FastMutex			g_fmScanPaperListLock;		//从扫描仪获取的图像信息的列表锁
extern SCAN_PAPER_LIST			g_lScanPaperTask;			//从扫描仪获取的图像信息的列表

#if 1
typedef struct _PicInfo_				//图片信息
{
	bool			bFindIssue;		//是否找到问题点
	bool			bRecogCourse;		//科目识别是否正确
	int 			nRecoged;		//是否已经识别过, 0-未识别，1-正在识别，2-识别完成
	int				nRecogRotation;	//识别过程中判断需要调整的方向，1:针对模板图像需要进行的旋转，正向，不需要旋转，2：右转90(模板图像旋转), 3：左转90(模板图像旋转), 4：右转180(模板图像旋转)
	int				nPicSaveRotation;/*图像的原始方向，相对视觉的方向(即考试作答方向)，在文字识别时要调整到视觉正常方向，模板保存时设置
									0:未知方向，1: 正常视觉方向(考试作答方向)，2-正常方向左旋90后的方向，3-正常方向右旋90后的方向，4-正常方向旋转180度后的方向*/
	int				nPicModelIndex;	//图片索引, 设置图片是属于模板的第几页，从0计数
	int				nPicOldModelIndex;	//针对多页模式，修改图片的模板索引时，记录移动前属于模板的第几页，在移动图片所属试卷的omr等信息时有用
	int				nQKFlag;			//缺考标识
	int				nWJFlag;			//违纪标识
	void*			pPaper;			//所属试卷的信息
	pST_SCAN_PIC	pSrcScanPic;	//原始扫描的图像信息
	cv::Rect		rtFix;			//定点矩形
	std::string		strPicName;		//图片名称
	std::string		strPicPath;		//图片路径	gb2312
	std::string		strPicZKZH;		//多页模式生效，图片所属的准考证号
	RECTLIST		lFix;			//定点列表
	RECTLIST		lNormalRect;	//识别出来的正常点位置
	RECTLIST		lIssueRect;		//识别出来的问题试卷的问题点位置，只要出现问题点就不进行下一页的识别(严格模式)，或者存储已经发现的问题点，但是继续后面的识别(简单模式)
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//识别的选做题OMR结果

	RECTLIST		lCalcRect;		//通过定点计算出的点位置
	RECTLIST		lModelWordFix;		//模板文字定点列表，在使用文字定位时有用
	CHARACTER_ANCHOR_AREA_LIST lCharacterAnchorArea;	//文字定位区域
	_PicInfo_()
	{
		nRecogRotation = 0;
		nPicSaveRotation = 0;
		nRecoged = 0;
		nQKFlag = 0;
		nWJFlag = 0;
		bFindIssue = false;
		pPaper = NULL;
		nPicModelIndex = 0;
		nPicOldModelIndex = 0;
		pSrcScanPic = NULL;
		bRecogCourse = true;
	}
	~_PicInfo_()
	{
		CHARACTER_ANCHOR_AREA_LIST::iterator itCharAnchorArea = lCharacterAnchorArea.begin();
		for (; itCharAnchorArea != lCharacterAnchorArea.end();)
		{
			pST_CHARACTER_ANCHOR_AREA pCharAnchorArea = *itCharAnchorArea;
			itCharAnchorArea = lCharacterAnchorArea.erase(itCharAnchorArea);
			SAFE_RELEASE(pCharAnchorArea);
		}
	}
}ST_PicInfo, *pST_PicInfo;
typedef std::list<pST_PicInfo> PIC_LIST;	//图片列表定义

typedef struct _PaperInfo_
{
	bool		bIssuePaper;		//是否是问题试卷
	bool		bModifyZKZH;		//准考证号人工修改标识
	bool		bModifyPagination;	//多页模式时，在页码异常界面中有过修改
	bool		bRecogComplete;		//该学生已经识别完成
	bool		bReScan;			//重新扫描标识，在准考证号修改窗口中设置
	bool		bRecogCourse;		//科目识别是否正确
	int			nPicsExchange;		//图像调换标识，即第一页与第二页调换位置，调换次数，0-未调换，1-调换1次。。。
	int			nPaginationStatus;	/*多页模式时的试卷状态，
									0-没有识别到页码，不能参与识别，设置问题卷，人工确认后再识别；1-识别完页码，可以识别，不能确定具体属于哪个学生(默认)；2-整袋识别完，可以确定属于哪个考生的哪张试卷
									3-图片数量与模板不一致，4-存在重复的页码
									*/
	//int			nPagination_Del_Paper;	//多页模式下，1-此试卷被主动删除,0-未被删除；删除的试卷放入问题试卷列表中不显示
	int			nQKFlag;			//缺考标识
	int			nWJFlag;			//违纪标识
	int			nZkzhInBmkStatus;	//准考证号是否在报名库中存在，在报名库列表不存在时，此项无效, 0--报名库中不存在，1--报名库中存在，-1--扫描时重号了
	//++从Pkg恢复Papers时的参数
	int			nChkFlag;			//此图片是否合法校验；在试卷袋里面的试卷图片，如果图片序号名称在Param.dat中不存在，则认为此试卷图片是错误图片，不進行圖片识别
	//--
	//int			nScanTmpIndex;		//从扫描获取到一张试卷的信息后直接构建的临时试卷，在整袋试卷识别完后再合并到具体的考生
	int			nIndex;				//在试卷袋中的索引，即S1为1，S2为2，S3为3...
	pMODEL		pModel;				//识别此学生试卷所用的模板
	void*		pPapers;			//所属的试卷袋信息
	void*		pSrcDlg;			//来源，来自哪个窗口，扫描or导入试卷窗口
	std::string strStudentInfo;		//学生信息, S1、S2、S3...
	std::string strSN;				//识别出的考号、准考证号
	std::string strRecogSN4Search;	//用于模糊搜索的考号，将未识别出来的部分用#代替，后期进行模糊搜索
	SCAN_PAPER_LIST lSrcScanPaper;	//原始扫描的试卷信息

	SNLIST				lSnResult;
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//识别的选做题OMR结果
	PIC_LIST	lPic;
	_PaperInfo_()
	{
		bIssuePaper = false;
		bModifyZKZH = false;
		bModifyPagination = false;
		nZkzhInBmkStatus = 0;
		bRecogComplete = false;
		bRecogCourse = true;
		bReScan = false;
		nPicsExchange = 0;
		//nScanTmpIndex = 0;
		nPaginationStatus = 1;
		//nPagination_Del_Paper = 0;
		nIndex = 0;
		nQKFlag = 0;
		nWJFlag = 0;
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
		for (auto itScanPaper : lSrcScanPaper)
		{
			pST_SCAN_PAPER pScanPaper = itScanPaper;
			SAFE_RELEASE(pScanPaper);
		}
		lSrcScanPaper.clear();

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

	int		nMustScanNum;				//必须扫描的试卷数量，在高厉害模式时生效，保存试卷时，检查扫描的数量是否与此数量一致，不一致不能提交，只能重扫

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
		nMustScanNum = 0;
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

extern pPAPERSINFO			_pCurrPapersInfo_;

extern Poco::FastMutex		g_fmRecog;		//识别线程获取任务锁
extern RECOGTASKLIST		g_lRecogTask;	//识别任务列表

extern Poco::FastMutex		g_fmPapers;		//操作试卷袋列表的任务锁
extern PAPERS_LIST			g_lPapers;		//所有的试卷袋信息

//TCP命令任务
typedef struct _TcpTask_
{
	unsigned short usCmd;
	int		nPkgLen;
	char*	pszSendBuf;
	char	szSendBuf[2500];
	_TcpTask_()
	{
		ZeroMemory(szSendBuf, 2500);
		pszSendBuf = NULL;
	}
	~_TcpTask_()
	{
		SAFE_RELEASE(pszSendBuf);
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
typedef std::list<pEXAM_SUBJECT> SUBJECT_LIST;

typedef struct _examInfo_
{
	int			nModel;				//0--网阅，1--手阅（天喻的考试）
	int			nExamID;			//考试ID
//	int			nExamGrade;			//年级
	int			nExamState;			//考试状态
	std::string	strExamID;			//天喻版本, 考试ID
	std::string strExamName;		//考试名称
	std::string strExamTime;		//考试时间
	std::string strExamTypeName;	//考试类型名称
	std::string strGradeName;		//年级名称
	std::string strPersonID;		//手阅时有用
	SUBJECT_LIST lSubjects;			//科目列表
	_examInfo_()
	{
		nModel = 0;
		nExamID = 0;
	}
	~_examInfo_()
	{
		SUBJECT_LIST::iterator itSub = lSubjects.begin();
		for (; itSub != lSubjects.end();)
		{
			pEXAM_SUBJECT pSub = *itSub;
			itSub = lSubjects.erase(itSub);
			SAFE_RELEASE(pSub);
		}
	}
}EXAMINFO, *pEXAMINFO;
typedef std::list<pEXAMINFO> EXAM_LIST;

extern Poco::FastMutex	g_lfmExamList;
extern EXAM_LIST	g_lExamList;

//++扫描相关
extern pEXAMINFO			_pCurrExam_;	//当前考试
extern pEXAM_SUBJECT		_pCurrSub_;		//当前考试科目
extern pMODEL				_pModel_;		//当前扫描使用的模板
//--

//报名库学生信息
typedef std::list<ST_STUDENT> STUDENT_LIST;	//报名库列表
extern STUDENT_LIST		g_lBmkStudent;	//报名库学生列表
#ifdef NewBmkTest
extern ALLSTUDENT_LIST		g_lBmkAllStudent;	//单个考试中所有科目的报名库学生列表
extern EXAMBMK_MAP			g_mapBmkMgr;			//考试报名库管理哈希表
#endif

#ifdef TEST_MULTI_SENDER
typedef struct _SendInfo_
{
	CFileUpLoad* pUpLoad;
	int			nPort;
	std::string strIP;
	_SendInfo_()
	{
		pUpLoad = NULL;
	}
}ST_SENDER, *pST_SENDER;
typedef std::map<std::string, pST_SENDER> MAP_FILESENDER;

extern Poco::FastMutex	_fmMapSender_;
extern MAP_FILESENDER	_mapSender_;
#endif

//模板图像路径，用于创建模板用
typedef struct tagModelPicPath
{
	CString strName;
	CString strPath;
}MODELPICPATH;
extern std::vector<MODELPICPATH> _vecModelPicPath_;		//用于从服务器上下载图像后直接创建模板

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

//----------------------------------------------
//任意两个定点计算出的矩形位置
typedef struct _tagNewRtBy2Fix_
{
	int nFirstFix;
	int nSecondFix;
	cv::Rect rt;
}ST_NEWRTBY2FIX;
typedef std::vector<ST_NEWRTBY2FIX> VEC_NEWRTBY2FIX;

typedef struct _tagFixRectInfo_
{
	RECTINFO rcFix;
	RECTINFO rcModelFix;
}ST_FIXRECTTINFO;
typedef std::vector<ST_FIXRECTTINFO> VEC_FIXRECTINFO;

bool	GetNewRt(RECTINFO rc, RECTINFO rcModel, VEC_FIXRECTINFO& lFixRtInfo, VEC_NEWRTBY2FIX& vecNewRt, cv::Rect rt);

//点的距离权重，即在这个点的一个半径范围内，有多少其他点在其中，如果距离一个半径，则权重+2，如果距离2个半径，则距离+1
typedef struct _tagPointDistWeight_
{
	int nWeight;
	cv::Point pt;
	_tagPointDistWeight_()
	{
		nWeight = 0;
	}
}ST_POINTDISTWEIGHT, *pST_POINTDISTWEIGHT;
typedef std::vector<ST_POINTDISTWEIGHT> VEC_POINTDISTWEIGHT;

bool GetPointDistWeight(int nRidus, cv::Point pt, VEC_POINTDISTWEIGHT& vecPointDistWeight);

//点距离顶点的距离
typedef struct _tagPointDist2Peak_
{
	int nDist;
	pST_CHARACTER_ANCHOR_POINT pAnchorPoint;
	_tagPointDist2Peak_()
	{
		nDist = 0;
		pAnchorPoint = NULL;
	}
}ST_POINTDIST2PEAK, *pST_POINTDIST2PEAK;
typedef std::vector<ST_POINTDIST2PEAK> VEC_POINTDIST2PEAK;
//----------------------------------------------

int		GetRectInfoByPoint(cv::Point pt, CPType eType, pPAPERMODEL pPaperModel, RECTINFO*& pRc);
//bool	ZipFile(CString strSrcPath, CString strDstPath, CString strExtName = _T(".zip"));
//bool	UnZipFile(CString strZipPath);
pMODEL	LoadModelFile(CString strModelPath);		//加载模板文件
bool	SortByCharAnchorArea(pST_CHARACTER_ANCHOR_AREA& st1, pST_CHARACTER_ANCHOR_AREA& st2);
bool	SortByCharacterConfidence(pST_CHARACTER_ANCHOR_POINT& st1, pST_CHARACTER_ANCHOR_POINT& st2);
bool	SortByArea(cv::Rect& rt1, cv::Rect& rt2);		//按面积排序
bool	SortByPositionX(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionY(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionX2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionY2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionXYInterval(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByTH(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByOmrTH(OMR_QUESTION& rc1, OMR_QUESTION& rc2);
bool	SortStringByDown(std::string& str1, std::string& str2);

bool    GetPicFix(int nPic, pST_PicInfo pPic, pMODEL pModel);	//使用文字定位时，生成文字定位的定点列表，只需要2个
bool	GetFixDist(int nPic, pST_PicInfo pPic, pMODEL pModel);	//
bool	GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, cv::Rect& rt);
bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);
std::string calcFileMd5(std::string strPath);
void	CopyData(char *dest, const char *src, int dataByteSize, bool isConvert, int height);
bool	PicRectify(cv::Mat& src, cv::Mat& dst, cv::Mat& rotMat);
bool	FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//定点进行仿射变换
bool	FixwarpPerspective(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//定点透视变换
bool	FixWarpAffine2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//3个定点仿射变换，对90度旋转图像有效，目标矩形大小为原矩形最大值的正方形
bool	FixwarpPerspective2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//4个定点透视变换，对90度旋转图像有效，目标矩形大小为原矩形最大值的正方形

bool	GetFixPicTransfer(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, cv::Mat& inverseMat);	//根据定点进行图像变换
bool	PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	PicTransfer2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//针对有3个或者4个定点的变换，而且是对90度图像旋转，目标矩形大小为原矩形最大值的正方形
int		WriteRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char * regValue);
int		ReadRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char* & regValue);
bool	encString(std::string& strSrc, std::string& strDst);
bool	decString(std::string& strSrc, std::string& strDst);

bool	GetInverseMat(RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	GetPosition2(cv::Mat& inverseMat, cv::Rect& rtSrc, cv::Rect& rtDst);

void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);

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
BOOL KillProcessFromName(CString strProcessName);