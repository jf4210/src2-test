#pragma once
#include "global.h"
#include "Log.h"
#include "MyCodeConvert.h"

#include "zbar.h"   

//****************************************************************
//****************************************************************
//****************************************************************
//#define	DATE_LIMIT		//�������ƣ�����һ��ʱ�䲻����
//****************************************************************
//****************************************************************
//****************************************************************


//#define PIC_RECTIFY_TEST	//ͼ����ת��������
#define WarpAffine_TEST		//����任����
#ifdef _DEBUG
	#define PaintOmrSnRect		//�Ƿ��ӡʶ�������OMR����
//	#define Test_ShowOriPosition	//���Դ�ӡģ�������Ӧ��ԭͼ����λ��
	#define PrintAllOmrVal		//��ӡ����OMRѡ��ֵ
#endif
#ifndef WarpAffine_TEST
//	#define TriangleSide_TEST		//���߶�λ�㷨
	#ifndef TriangleSide_TEST
		#define TriangleCentroid_TEST	//���������㷨
	#endif
#endif

//++ test 2017.4.27
#define Test_RecogFirst_NoThreshord		//��Ե�һ���㷨�����÷�ֱֵ���ж�Ҫ������ѡ��������ﵽ�����ܶȱ�������Ϊѡ��һ����Ϳ���������ַ���
//--

#define Test_RecogOmr3			//��3��OMRʶ�𷽷�����
#define Test_SendRecogResult	//ֱ�����Ծ��ʶ�����ʱ����ʶ��������˷�����

#define USES_GETTHRESHOLD_ZTFB	//ʹ����̬�ֲ���ʽ��ȡУ���ķ�ֵ��δ����ʱʹ�ù̶���ֵ���ж�ֵ�����Ҿ���

#define  MSG_ERR_RECOG		(WM_USER + 110)
#define  MSG_RECOG_COMPLETE	(WM_USER + 111)
#define  MSG_SENDRESULT_STATE (WM_USER + 112)

#define DecompressTest		//��ѹ���ԣ����߳̽�ѹ

#define SOFT_VERSION	_T("1.70511-1")
#define SYS_BASE_NAME	_T("YKLX-DMT")
//#define WH_CCBKS		//�人���ű�ר�ã�������ά����Ҫjson����

#define CHK_NUM		//��ԇ����в��څ����ļ��б��е��ļ����Ծ����Ϣ���޳�
//#define WIN_THREAD_TEST		//ʹ��window�̣߳�����poco�����߳�

extern CLog g_Log;
extern int	g_nExitFlag;

extern CString				g_strCurrentPath;
extern int					g_nRecogMode;		//ʶ��ģʽ��0-�ϸ�ģʽ��1-��ģʽ

extern std::string _strEncryptPwd_;
extern pMODEL _pModel_;

extern std::string		_strSessionName_;

extern std::map<std::string, std::string> answerMap;


extern bool		_nUseNewParam_;			//�Ƿ�ʹ���µĲ�������ʶ��ģ��

extern int		_nAnticlutterKernel_;	//ʶ��ͬ��ͷʱ���������͸�ʴ�ĺ�����
extern int		_nCannyKernel_;			//������������

extern int		g_nRecogGrayMin;		//�Ҷȵ�(���հ׵�,OMR��)����Ҷȵ���С���Է�Χ
extern int		g_nRecogGrayMax_White;	//�հ׵�У������Ҷȵ�����Է�Χ
extern int		g_nRecogGrayMin_OMR;	//OMR����Ҷȵ���С���Է�Χ
extern int		g_RecogGrayMax_OMR;		//OMR����Ҷȵ�����Է�Χ


extern double	_dOmrThresholdPercent_Fix_;		//����ģʽOMRʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
extern double	_dSnThresholdPercent_Fix_;		//����ģʽSNʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
extern double	_dQKThresholdPercent_Fix_;		//����ģʽQKʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
extern double	_dOmrThresholdPercent_Head_;	//ͬ��ͷģʽOMRʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
extern double	_dSnThresholdPercent_Head_;		//ͬ��ͷģʽSNʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
extern double	_dQKThresholdPercent_Head_;		//ͬ��ͷģʽQKʶ�����Ϊ��ѡ�еı�׼�ٷֱ�

extern double	_dCompThread_Fix_;
extern double	_dDiffThread_Fix_;
extern double	_dDiffExit_Fix_;
extern double	_dCompThread_Head_;
extern double	_dDiffThread_Head_;
extern double	_dDiffExit_Head_;
extern int		_nThreshold_Recog2_;	//��2��ʶ�𷽷��Ķ�ֵ����ֵ
extern double	_dCompThread_3_;		//������ʶ�𷽷�
extern double	_dDiffThread_3_;
extern double	_dDiffExit_3_;
extern double	_dAnswerSure_;		//����ȷ���Ǵ𰸵����Ҷ�

extern int		_nOMR_;		//����ʶ��ģ��ʱ������ʶ��OMR���ܶ�ֵ�ķ�ֵ
extern int		_nSN_;		//����ʶ��ģ��ʱ������ʶ��ZKZH���ܶ�ֵ�ķ�ֵ

extern int		_nDecompress_;	//��ѹ�Ծ������
extern int		_nRecog_;		//ʶ���Ծ�����
extern int		_nRecogPapers_;	//ʶ���Ծ��
extern int		_nCompress_;	//ѹ���Ծ������
extern Poco::FastMutex _fmDecompress_;
extern Poco::FastMutex _fmRecog_;
extern Poco::FastMutex _fmRecogPapers_;
extern Poco::FastMutex _fmCompress_;

extern double	_dDoubtPer_;

extern std::string		g_strUploadUri;		//ʶ�����ύ��uri��ַ

typedef struct _DecompressTask_
{
	//---------	�������ݣ���ʶ��εĽ�ѹ������Ҫʶ�����Ŀ --------
	bool bRecogOmr;			//ʶ��OMR
	bool bRecogZkzh;		//ʶ��׼��֤��
	bool bRecogElectOmr;	//ʶ��ѡ����
	int	 nSendEzs;			//�Ƿ���ʶ�����ʱֱ�ӷ��͸�EZS��������ѹ��
	int	 nNoNeedRecogVal;	////�Ծ����ʶ��ֵ��omr���� + omr�� + SN���������ڴ˷�ֵ�Ž�����ʶ��
	//---------

	int nTaskType;				//1-��ͨ��ѹ��2-�����Ծ������ͬĿ¼, 3-����ʶ��OMR��SN, 4-����ģ��, 5-ʶ���Ծ����ͳ��ʶ����ȷ�ʱ���, 6-��ѹ�Ծ���е��ض��ļ�
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
	std::string strDecompressDir;
	std::string strDecompressPaperFile;		//��ѹ�Ծ�����ض����ļ�����S1��S2��nTaskType = 6 ʱ����
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
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//ʶ�������б�

extern Poco::FastMutex			g_fmDecompressLock;		//��ѹ�ļ��б���
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//��ѹ�ļ��б�


//----------------------------------------------

typedef struct  _StSearchTash_
{
	//---------	�������ݣ���ʶ��εĽ�ѹ������Ҫʶ�����Ŀ --------
	bool bRecogOmr;			//ʶ��OMR
	bool bRecogZkzh;		//ʶ��׼��֤��
	bool bRecogElectOmr;	//ʶ��ѡ����
	int  nSendEzs;			//�Ƿ���ʶ�����ʱֱ�ӷ��͸�EZS��������ѹ��		//0-ʹ��ѹ������ʽ��1-ֱ�ӷ��ͽ����ezs��2-�����ͽ��Ҳ��ѹ���Ծ��������ģʽ����ʶ����
	int	 nNoNeedRecogVal;	////�Ծ����ʶ��ֵ��omr���� + omr�� + SN���������ڴ˷�ֵ�Ž�����ʶ��
	//---------
	int	 nSearchType;		//1-����Ŀ¼�µ�����pkg������ȫ��ѹ��2-����Ŀ¼��ָ����pkg
	std::string strSearchPath;
	//-------------��������, ���nSearchType = 2 ʱ��Ч
	std::string strSearchName;	//����ָ�����Ծ���ļ������nSearchType = 2 ʱ��Ч
	std::string strPaperName;	//��Ҫ��ѹ�Ծ���е��ض��Ծ��ļ������nSearchType = 2 ʱ��Ч
	std::string strDecompressPath;	//�ļ���ѹ��Ŀ��·�������nSearchType = 2 ʱ��Ч
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


typedef struct _PicInfo_				//ͼƬ��Ϣ
{
	bool			bRecoged;		//�Ƿ��Ѿ�ʶ���
	bool			bFindIssue;		//�Ƿ��ҵ������
	void*			pPaper;			//�����Ծ����Ϣ
	cv::Rect		rtFix;			//�������
	std::string		strPicName;		//ͼƬ����
	std::string		strPicPath;		//ͼƬ·��
	RECTLIST		lFix;			//�����б�
	RECTLIST		lNormalRect;	//ʶ�������������λ��
	RECTLIST		lIssueRect;		//ʶ������������Ծ�������λ�ã�ֻҪ���������Ͳ�������һҳ��ʶ��
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
typedef std::list<pST_PicInfo> PIC_LIST;	//ͼƬ�б���

typedef struct _PaperInfo_
{
	bool		bIssuePaper;		//�Ƿ��������Ծ�
	int			nChkFlag;			//��ͼƬ�Ƿ�Ϸ�У�飻���Ծ��������Ծ�ͼƬ�����ͼƬ���������Param.dat�в����ڣ�����Ϊ���Ծ�ͼƬ�Ǵ���ͼƬ�����M�ЈDƬʶ��
	int			nQKFlag;			//ȱ����ʶ
	pMODEL		pModel;				//ʶ���ѧ���Ծ����õ�ģ��
	void*		pPapers;			//�������Ծ����Ϣ
	void*		pSrcDlg;			//��Դ�������ĸ����ڣ�ɨ��or�����Ծ���
	std::string strStudentInfo;		//ѧ����Ϣ	
	std::string strSN;
	std::string strMd5Key;


	//++
	int		nOmrDoubt;				//OMR���ɵ�����
	int		nOmrNull;				//OMRʶ��Ϊ�յ�����
	int		nSnNull;				//׼��֤��ʶ��Ϊ�յ�����
	//--

	SNLIST				lSnResult;
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//ʶ���ѡ����OMR���
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
}ST_PaperInfo, *pST_PaperInfo;		//�Ծ���Ϣ��һ��ѧ����Ӧһ���Ծ�һ���Ծ�����ж��ͼƬ
typedef std::list<pST_PaperInfo> PAPER_LIST;	//�Ծ��б�

typedef struct _PapersInfo_				//�Ծ����Ϣ�ṹ��
{
	bool	bRecogOmr;			//ֱ�ӷ���ʶ����ʱ���Ƿ���OMR
	bool	bRecogZkzh;			//ֱ�ӷ���ʶ����ʱ���Ƿ���׼��֤��
	bool	bRecogElectOmr;		//ֱ�ӷ���ʶ����ʱ���Ƿ���ѡ����
	int		nSendEzs;			//�Ƿ���ʶ�����ʱֱ�ӷ��͸�EZS��������ѹ��	//0-ʹ��ѹ������ʽ��1-ֱ�ӷ��ͽ����ezs��2-�����ͽ��Ҳ��ѹ���Ծ��������ģʽ����ʶ����

	int		nPaperCount;				//�Ծ�����Ծ�������(ѧ����)
	int		nRecogErrCount;				//ʶ������Ծ�����
	int		nTotalPics;

	int			nTotalPaper;		//��ѧ�����������Ծ���ļ��ж�ȡ
	int			nExamID;			//����ID
	int			nSubjectID;			//��ĿID
	int			nTeacherId;			//��ʦID
	int			nUserId;			//�û�ID

	int			nRecogMode;			//ʶ��ģʽ��1-��ģʽ(��������У��㲻ֹͣʶ��)��2-�ϸ�ģʽ
	//++ͳ����Ϣ
	int		nPkgOmrDoubt;				//OMR���ɵ�����					�ͻ��˴��ݵ�
	int		nPkgOmrNull;				//OMRʶ��Ϊ�յ�����				�ͻ��˴��ݵ�
	int		nPkgSnNull;				//׼��֤��ʶ��Ϊ�յ�����		�ͻ��˴��ݵ�

	int		nOmrDoubt;				//OMR���ɵ�����					
	int		nOmrNull;				//OMRʶ��Ϊ�յ�����				
	int		nSnNull;				//׼��֤��ʶ��Ϊ�յ�����		

	int		nOmrError_1;			//����ѧ���𰸣���1��ʶ�𷽷�ʶ������ֵ		ͳ��ʱ��
	int		nOmrError_2;			//����ѧ���𰸣���1��ʶ�𷽷�ʶ������ֵ		ͳ��ʱ��

	Poco::FastMutex	fmOmrStatistics;//omrͳ����
	Poco::FastMutex fmSnStatistics; //zkzhͳ����
	//--


	Poco::FastMutex	fmTask;			//���ύ������ݵ�����(��:nTaskCounts)������������
	int			nTaskCounts;		//���Ծ���������������ύ��ͼƬ����Ҫ������ύͼƬ���ݡ�׼��֤�š�OMR��ѡ������Ϣ
									//�����ݿ����������Ծ����������������Ϻ�����Ծ��ɾ������


	int		nRecogPics;				//�Ѿ�ʶ����ɵ�����
	Poco::FastMutex fmRecogCompleteOK; //�������

	Poco::FastMutex fmlPaper;			//���Ծ��б��д��
	Poco::FastMutex fmlIssue;			//�������Ծ��б��д��
	std::string  strPapersName;			//�Ծ������
	std::string	 strPapersDesc;			//�Ծ����ϸ����

	std::string		strPapersPath;
	std::string		strSrcPapersPath;
	std::string		strSrcPapersFileName;

	std::string	strUploader;		//�ϴ���
	std::string strEzs;				//�ϴ�����˷������ã�--cookie
	std::string strDesc;			//���Ծ���ļ��ж�ȡ
	
	PAPER_LIST	lPaper;					//���Ծ�����Ծ��б�
	PAPER_LIST	lIssue;					//���Ծ����ʶ����������Ծ��б�
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
typedef std::list<pPAPERSINFO> PAPERS_LIST;		//�Ծ���б�



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
typedef std::list<pCOMPRESSTASK> COMPRESSTASKLIST;	//ʶ�������б�

extern Poco::FastMutex			g_fmCompressLock;		//ѹ���ļ��б���
extern COMPRESSTASKLIST			g_lCompressTask;		//��ѹ�ļ��б�

extern Poco::Event			g_eCompressThreadExit;


typedef struct _RecogResult_
{
	int			nTaskType;			//��������: 1-��img�������ύͼƬ��2-������ύͼƬ����, 3-�ύOMR��4-�ύZKZH��5-�ύѡ������Ϣ, 6-ģ��ͼƬ�ύzimg������, 7-�Ծ��ֻ�ύOMR��SN��ѡ����Ϣ
	int			nSendFlag;			//���ͱ�ʾ��1������ʧ��1�Σ�2������ʧ��2��...
	Poco::Timestamp sTime;			//��������ʱ�䣬���ڷ���ʧ��ʱ��ʱ����
	pPAPERSINFO pPapers;
	std::string strUri;
	std::string strResult;			//���Ϳ�����Ϣ����˷���������nTaskType = 2ʱ����
	std::string strEzs;				//��nTaskType = 2ʱ����
	_RecogResult_()
	{
		nTaskType = 0;
		nSendFlag = 0;
		pPapers = NULL;
	}
}ST_RECOG_RESULT_TASK, *p_ST_RECOG_RESULT_TASK;
typedef std::list<p_ST_RECOG_RESULT_TASK> RECOG_RESULT_LIST;

extern	Poco::FastMutex			g_fmReSult;		//�����̻߳�ȡ������
extern	RECOG_RESULT_LIST		g_lResultTask;	//���������б�



typedef struct _SendHttpTask_
{
	bool	bSendOmr;			//ֱ�ӷ���ʶ����ʱ���Ƿ���OMR
	bool	bSendZkzh;		//ֱ�ӷ���ʶ����ʱ���Ƿ���׼��֤��
	bool	bSendElectOmr;	//ֱ�ӷ���ʶ����ʱ���Ƿ���ѡ����

	int			nTaskType;			//��������: 1-��img�������ύͼƬ��2-������ύͼƬ����, 3-�ύOMR��4-�ύZKZH��5-�ύѡ������Ϣ, 6-ģ��ͼƬ�ύzimg������, 7-�Ծ��ֻ�ύOMR��SN��ѡ����Ϣ
	int			nSendFlag;			//���ͱ�ʾ��1������ʧ��1�Σ�2������ʧ��2��...
	Poco::Timestamp sTime;			//��������ʱ�䣬���ڷ���ʧ��ʱ��ʱ����
	pST_PicInfo pPic;
	pPAPERSINFO pPapers;
	std::string strUri;
	std::string strResult;			//���Ϳ�����Ϣ����˷���������nTaskType = 2ʱ����
	std::string strEzs;				//��nTaskType = 2ʱ����
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
extern LIST_SEND_HTTP		g_lHttpSend;		//����HTTP�����б�



typedef struct _RecogTask_
{
	int		nPic;						//���Ծ�����ģ��ĵڼ���
	pMODEL pModel;						//ʶ���õ�ģ��
	std::string strPath;
	pST_PaperInfo	pPaper;				//��Ҫʶ����Ծ�
}RECOGTASK, *pRECOGTASK;
typedef std::list<pRECOGTASK> RECOGTASKLIST;	//ʶ�������б�

extern Poco::FastMutex		g_fmRecog;		//ʶ���̻߳�ȡ������
extern RECOGTASKLIST		g_lRecogTask;	//ʶ�������б�

extern Poco::FastMutex		g_fmPapers;		//�����Ծ���б��������
extern PAPERS_LIST			g_lPapers;		//���е��Ծ����Ϣ

bool encString(std::string& strSrc, std::string& strDst);
bool decString(std::string& strSrc, std::string& strDst);


void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);

pMODEL	LoadModelFile(CString strModelPath);		//����ģ���ļ�
bool	InitModelRecog(pMODEL pModel, std::string strModelPath);				//����ʶ��ģ���У����OMR�ĻҶ�
bool	SortByArea(cv::Rect& rt1, cv::Rect& rt2);		//���������
bool	SortByPositionX(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionY(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionX2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionY2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortbyNumASC(const std::string& x, const std::string& y);
bool	SortByPaper(const pST_PaperInfo& x, const pST_PaperInfo& y);

bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);
bool	FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//������з���任
bool	FixwarpPerspective(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//����͸�ӱ任
bool	PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);

//----------------	OMRʶ��ҶȲ�ֵ�Ƚ�	------------------
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


//----------------	��ά�롢����ʶ��	------------------
//zbar�ӿ�
std::string ZbarDecoder(cv::Mat img, std::string& strTypeName);

//�Զ�ֵͼ�����ʶ�����ʧ����������ж���ʶ��
std::string GetQRInBinImg(cv::Mat binImg, std::string& strTypeName);

//main function
std::string GetQR(cv::Mat img, std::string& strTypeName);
//--------------------------------------------------------

bool ZipFile(std::string& strSavePath, std::string& strSrcDir, std::string strExtName = ".pkg");
bool SavePapersInfo(pPAPERSINFO pPapers);


//ͳ����Ϣ
//ͳ����Ϣ
extern Poco::FastMutex _fmErrorStatistics_;
extern int		_nErrorStatistics1_;	//��һ�ַ���ʶ�������
extern int		_nErrorStatistics2_;	//�ڶ��ַ���ʶ�������
extern int		_nOmrDoubtStatistics_;		//ʶ��������
extern int		_nOmrNullStatistics_;		//ʶ��Ϊ������
extern int		_nSnNullStatistics_;	//SNʶ��Ϊ������
extern int		_nAllOmrStatistics_;		//ͳ������
extern int		_nAllSnStatistics_;			//SNͳ������
extern int		_nPkgDoubtStatistics_;	//ԭʼ�Ծ��ʶ��������
extern int		_nPkgOmrNullStatistics_;	//ԭʼ�Ծ��ʶ��Ϊ������
extern int		_nPkgSnNullStatistics_;		//ԭʼ�Ծ����SNʶ��Ϊ������
std::string calcStatistics(pPAPERSINFO pPapers);
//--
