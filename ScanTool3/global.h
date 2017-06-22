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
//#include "./pdf2jpg/MuPDFConvert.h"
#include "modelInfo.h"

#include "zbar.h"
#include "StudentDef.h"
#include "FileUpLoad.h"

//#define PIC_RECTIFY_TEST	//ͼ����ת��������
#define WarpAffine_TEST		//����任����
#ifdef _DEBUG
	#define PaintOmrSnRect		//�Ƿ��ӡʶ�������OMR����
//	#define PrintRecogLog		//��ӡʶ����־
//	#define Test_ShowOriPosition	//���Դ�ӡģ�������Ӧ��ԭͼ����λ��
	#define	 TEST_MODEL_NAME	//ģ�����Ʋ���
//	#define Test_Data			//�������ݣ�����ģʽ
//	#define TEST_TIP_SHOW		//��ʾ��Ϣ��ʾ����
	#define TEST_MULTI_SENDER	//�ļ����Ͳ��ԣ��ö����ַ���Ͳ���
#else	//release�汾
	#define TEST_MULTI_SENDER	//�ļ����Ͳ��ԣ��ö����ַ���Ͳ���
	#define	 TEST_MODEL_NAME	//ģ�����Ʋ���
	#define PUBLISH_VERSION			//�����汾,�����汾�����š��Ծ��빦�ܡ�
#endif
#ifndef WarpAffine_TEST
//	#define TriangleSide_TEST		//���߶�λ�㷨
	#ifndef TriangleSide_TEST
		#define TriangleCentroid_TEST	//���������㷨
	#endif
#endif

#define TEST_NEW_MAKEMODEL		//��ģ�����
#ifdef TEST_NEW_MAKEMODEL
	#define MAKEMODEL_IN_NEW
#endif

#define Test_RecogOmr3			//��3��OMRʶ�𷽷�����
#define TEST_MODIFY_ZKZH_CHIld	//���޸�׼��֤�Ŵ���Ƕ�뵱���Ӵ��ڣ�������������

//+++++++++	ѡ��汾 ++++++++++++++++++
//#define TO_WHTY							//���人������Ϣʹ�ã���ʶ��ֻɨ���ϴ�
#ifndef TO_WHTY
	#define SHOW_GUIDEDLG				//��ʾ��������,�׿���ѧ�Լ��ã���ʾ��������.******** �˴������壬��ֱ����ʾ������	***********
#endif

//#define WH_CCBKS	//�人���ű�����ר��
//+++++++++++++++++++++++++++

#define USES_GETTHRESHOLD_ZTFB	//ʹ����̬�ֲ���ʽ��ȡУ���ķ�ֵ��δ����ʱʹ�ù̶���ֵ���ж�ֵ�����Ҿ���
#define USES_FILE_ENC			//�Ƿ���ļ�ʹ�ü���
#define MSG_NOTIFY_UPDATE (WM_APP + 101)

#ifndef TO_WHTY
	#define USES_PWD_ZIP_UNZIP		//�Ƿ�ʹ�������ѹ��
	#define PAPERS_EXT_NAME	_T(".pkg")			//�Ծ��ѹ�������չ��(��֤��4λ)
	#define PAPERS_EXT_NAME_4TY _T(".typkg")		//�����汾��ͨ�������������ļ���������Ϊ.zip
#else
	#define PAPERS_EXT_NAME	_T(".typkg")			//�Ծ��ѹ�������չ��(��֤��4λ),ͨ�������������ļ���������Ϊ.zip
#endif

#define  MSG_ERR_RECOG	(WM_USER + 110)
#define  MSG_ZKZH_RECOG (WM_USER + 111)		//��׼��֤��ʶ�����ʱ֪ͨUI�߳��޸��Ծ��б���ʾ�Ѿ�ʶ���ZKZH		2017.2.14
#define	 MSG_Pkg2Papers_OK (WM_USER + 112)	//��pkg�ָ���Papers���

#ifndef TO_WHTY
	#ifdef PUBLISH_VERSION
		#define SOFT_VERSION	_T("v2.1")
	#else
		#define SOFT_VERSION	_T("v2.1-Pir")		//-Pri
	#endif
#else
	#define SOFT_VERSION	_T("v2.1")
#endif
#define SYS_BASE_NAME	_T("��������ɨ��ͻ���")
#define SYS_GUIDE_NAME	_T("GuideDlg")


#define MAX_DLG_WIDTH	1024
#define MAX_DLG_HEIGHT	768

// #define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}
// #define SAFE_RELEASE_ARRY(pObj) if(pObj) {delete[] pObj; pObj = NULL;}

//++��¼��Ϣ
extern bool	_bHandModel_;			//�Ƿ�������ģʽ������ģʽ��������
extern bool	_bLogin_;				//�Ƿ��Ѿ���¼
extern std::string _strUserName_;	//��¼�û���
extern std::string _strNickName_;	//�û��ǳ�
extern std::string _strPwd_;		//����
extern std::string _strEzs_;		//�����Ҫ��EZS
extern int _nTeacherId_;			//��ʦID
extern int _nUserId_;				//�û�ID
extern std::string _strPersonID_;	//����ģʽ������ר��
extern std::string _strSchoolID_;	//����ģʽ������ר��
extern int _nPicNum4Ty_;			//����ģʽ��ģ��ͼƬ����
//--
extern bool		_bGetBmk_;			//�Ƿ��õ�ǰ��Ŀ������
extern int					_nReocgThreads_;		//ʶ���߳�����

//++�¼�����
extern Poco::Event			g_eGetExamList;		//��ȡ�����б��¼�
extern Poco::Event			g_eGetBmk;			//��ȡ�������¼�
extern Poco::Event			g_eDownLoadModel;	//����ģ�����״̬
//--
extern int					g_nDownLoadModelStatus;		//����ģ���״̬	0-δ���أ���ʼ����1-ģ�������У�2-���سɹ���3-���ش��ڴ��ļ�������Ҫ����, -1-�������˿�Ŀģ�岻����, -2-��������ȡ�ļ�ʧ��
extern int					_nScanStatus_;				//ɨ����� 0-δɨ�裬1-����ɨ�裬2-ɨ�����, 3-ɨ����ֹ, -1--����ɨ����ʧ��, -2--����ɨ����ʧ��, -3--ɨ��ʧ��
extern int					_nScanCount_;				//ɨ�����������ǰ��ɨ����ٷ�
extern int					_nScanPaperCount_;			//��ǰ�Ѿ�ɨ�������������������ʼ����

extern CString				g_strCurrentPath;
extern std::string			g_strPaperSavePath;
extern std::string			g_strModelSavePath;
extern std::string			g_strPaperBackupPath;	//�Ծ�����ɺ�ı���·��
extern Poco::Logger*		g_pLogger;
extern int					g_nExitFlag;

extern std::string			g_strEncPwd;				//�ļ����ܽ�������
extern std::string			g_strCmdIP;
extern std::string			g_strFileIP;
extern int					g_nCmdPort;
extern int					g_nFilePort;
extern std::string			g_strFileIp4HandModel;		//����ģʽʱ�ļ���������ַ
extern int					g_nFilePort4HandModel;		//����ģʽʱ�ļ��������˿�

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

extern int		_nAnticlutterKernel_;	//ʶ��ͬ��ͷʱ���������͸�ʴ�ĺ�����

extern int		_nGauseKernel_;			//��˹�任������
extern int		_nSharpKernel_;			//�񻯺�����
extern int		_nCannyKernel_;			//������������
extern int		_nDilateKernel_;		//���ͺ�����
extern int		_nErodeKernel_;			//��ʴ������

extern int		g_nRecogGrayMin;		//�Ҷȵ�(���հ׵�,OMR��)����Ҷȵ���С���Է�Χ
extern int		g_nRecogGrayMax_White;	//�հ׵�У������Ҷȵ�����Է�Χ
extern int		g_nRecogGrayMin_OMR;	//OMR����Ҷȵ���С���Է�Χ
extern int		g_RecogGrayMax_OMR;		//OMR����Ҷȵ�����Է�Χ


extern int				g_nManulUploadFile;		//�ֶ��ϴ��ļ���ͨ��qq�����

extern bool				g_bCmdConnect;		//����ͨ������
extern bool				g_bFileConnect;		//�ļ�ͨ������

extern bool				g_bCmdNeedConnect;	//����ͨ���Ƿ���Ҫ����������ͨ����ַ��Ϣ�޸ĵ����
extern bool				g_bFileNeedConnect;	//�ļ�ͨ���Ƿ���Ҫ����������ͨ����ַ��Ϣ�޸ĵ����

extern bool				g_bShowScanSrcUI;	//�Ƿ���ʾԭʼɨ�����
extern int				g_nOperatingMode;	//����ģʽ��1--����ģʽ(��������㲻ֹͣɨ��)��2-�ϸ�ģʽ(���������ʱ����ֹͣɨ��)
extern bool				g_bModifySN;		//�Ƿ������޸�׼��֤��
extern int				g_nZkzhNull2Issue;	//ʶ��׼��֤��Ϊ��ʱ���Ƿ���Ϊ�������Ծ�

//�������ͣ���ǰ��ʾ�Ĵ���
typedef enum _eDlgType_
{
	DLG_Login = 0,			//��¼����
	DLG_ExamMgr,			//�����б������
	DLG_DownloadModle,		//���ر�������ģ�崰��
	DLG_ScanStart,			//ɨ�迪ʼ����
	Dlg_ScanProcess,		//ɨ����ȴ���
	Dlg_ScanRecordMgr		//ɨ���¼�����ڣ���������Ϣչʾ����
}E_DLG_TYPE;
extern E_DLG_TYPE		_eCurrDlgType_;	//��ǰ��ʾ�Ĵ��ڣ��������ڲ���

#if 1
typedef struct _PicInfo_				//ͼƬ��Ϣ
{
	bool			bFindIssue;		//�Ƿ��ҵ������
	int 			nRecoged;		//�Ƿ��Ѿ�ʶ���, 0-δʶ��1-����ʶ��2-ʶ�����
	void*			pPaper;			//�����Ծ����Ϣ
	cv::Rect		rtFix;			//�������
	std::string		strPicName;		//ͼƬ����
	std::string		strPicPath;		//ͼƬ·��
	RECTLIST		lFix;			//�����б�
	RECTLIST		lNormalRect;	//ʶ�������������λ��
	RECTLIST		lIssueRect;		//ʶ������������Ծ�������λ�ã�ֻҪ���������Ͳ�������һҳ��ʶ��(�ϸ�ģʽ)�����ߴ洢�Ѿ����е�����㣬���Ǽ��������ʶ��(��ģʽ)
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
typedef std::list<pST_PicInfo> PIC_LIST;	//ͼƬ�б���

typedef struct _PaperInfo_
{
	bool		bIssuePaper;		//�Ƿ��������Ծ�
	bool		bModifyZKZH;		//׼��֤���˹��޸ı�ʶ
	bool		bRecogComplete;		//��ѧ���Ѿ�ʶ�����
	bool		bReScan;			//����ɨ���ʶ����׼��֤���޸Ĵ���������
	int			nQKFlag;			//ȱ����ʶ
	int			nZkzhInBmkStatus;	//׼��֤���Ƿ��ڱ������д��ڣ��ڱ������б�����ʱ��������Ч, 0--�������в����ڣ�1--�������д��ڣ�-1--ɨ��ʱ�غ���
	//++��Pkg�ָ�Papersʱ�Ĳ���
	int			nChkFlag;			//��ͼƬ�Ƿ�Ϸ�У�飻���Ծ��������Ծ�ͼƬ�����ͼƬ���������Param.dat�в����ڣ�����Ϊ���Ծ�ͼƬ�Ǵ���ͼƬ�����M�ЈDƬʶ��
	//--
	int			nIndex;				//���Ծ���е���������S1Ϊ1��S2Ϊ2��S3Ϊ3...
	pMODEL		pModel;				//ʶ���ѧ���Ծ����õ�ģ��
	void*		pPapers;			//�������Ծ����Ϣ
	void*		pSrcDlg;			//��Դ�������ĸ����ڣ�ɨ��or�����Ծ���
	std::string strStudentInfo;		//ѧ����Ϣ, S1��S2��S3...
	std::string strSN;				//ʶ����Ŀ��š�׼��֤��
	std::string strRecogSN4Search;	//����ģ�������Ŀ��ţ���δʶ������Ĳ�����#���棬���ڽ���ģ������
	
	SNLIST				lSnResult;
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//ʶ���ѡ����OMR���
	PIC_LIST	lPic;
	_PaperInfo_()
	{
		bIssuePaper = false;
		bModifyZKZH = false;
		nZkzhInBmkStatus = 0;
		bRecogComplete = false;
		bReScan = false;
		nIndex = 0;
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
}ST_PaperInfo, *pST_PaperInfo;		//�Ծ���Ϣ��һ��ѧ����Ӧһ���Ծ�һ���Ծ�����ж��ͼƬ
typedef std::list<pST_PaperInfo> PAPER_LIST;	//�Ծ��б�

typedef struct _PapersInfo_				//�Ծ����Ϣ�ṹ��
{
	int		nPapersType;				//�Ծ����ͣ�0-�����������Ծ������ɨ��ʱ������1-��Pkg�ָ���Papersʱ������
	int		nPaperCount;				//�Ծ�����Ծ�������(ѧ����)
	int		nRecogErrCount;				//ʶ������Ծ�����

	//++ͳ����Ϣ
	int		nOmrDoubt;				//OMR���ɵ�����
	int		nOmrNull;				//OMRʶ��Ϊ�յ�����
	int		nSnNull;				//׼��֤��ʶ��Ϊ�յ�����
	Poco::FastMutex	fmOmrStatistics;//omrͳ����
	Poco::FastMutex fmSnStatistics; //zkzhͳ����
	//--

	//++��Pkg�ָ�Papersʱ�Ĳ���
	int			nExamID;			//����ID
	int			nSubjectID;			//��ĿID
	int			nTeacherId;			//��ʦID
	int			nUserId;			//�û�ID
	//--

	Poco::FastMutex fmlPaper;			//���Ծ��б��д��
	Poco::FastMutex fmlIssue;			//�������Ծ��б��д��
	std::string  strPapersName;			//�Ծ������
	std::string	 strPapersDesc;			//�Ծ����ϸ����

	PAPER_LIST	lPaper;					//���Ծ�����Ծ��б�
	PAPER_LIST	lIssue;					//���Ծ����ʶ����������Ծ��б�
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
typedef std::list<pPAPERSINFO> PAPERS_LIST;		//�Ծ���б�


typedef struct _RecogTask_
{
	int		nPic;						//���Ծ�����ģ��ĵڼ���
	pMODEL pModel;						//ʶ���õ�ģ��
	std::string strPath;	
	pST_PaperInfo	pPaper;				//��Ҫʶ����Ծ�
}RECOGTASK, *pRECOGTASK;
typedef std::list<pRECOGTASK> RECOGTASKLIST;	//ʶ�������б�
#endif

extern pPAPERSINFO			_pCurrPapersInfo_;

extern Poco::FastMutex		g_fmRecog;		//ʶ���̻߳�ȡ������
extern RECOGTASKLIST		g_lRecogTask;	//ʶ�������б�

extern Poco::FastMutex		g_fmPapers;		//�����Ծ���б��������
extern PAPERS_LIST			g_lPapers;		//���е��Ծ����Ϣ


//TCP��������
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

//�ļ��ϴ�����
typedef struct _SendTask_
{
	int		nSendState;			//0-δ���ͣ�1-���ڷ��ͣ�2-������ɣ�3-����ʧ��
	float	fSendPercent;
	std::string strFileName;
	std::string strPath;
	_SendTask_()
	{
		nSendState = 0;
		fSendPercent = 0.0;
	}
}SENDTASK, *pSENDTASK;
typedef std::list<pSENDTASK> SENDTASKLIST;	//ʶ�������б�

extern Poco::FastMutex		g_fmSendLock;
extern SENDTASKLIST			g_lSendTask;

typedef struct _ExamSubjects_
{
	int			nSubjID;		//���Կ�ĿID
//	int			nSubjCode;		//���Կ�Ŀ����
	std::string strSubjName;	//���Կ�Ŀ����
	std::string strModelName;	//ɨ������ģ������
}EXAM_SUBJECT, *pEXAM_SUBJECT;
typedef std::list<pEXAM_SUBJECT> SUBJECT_LIST;

typedef struct _examInfo_
{
	int			nModel;				//0--���ģ�1--���ģ������Ŀ��ԣ�
	int			nExamID;			//����ID
//	int			nExamGrade;			//�꼶
	int			nExamState;			//����״̬
	std::string	strExamID;			//�����汾, ����ID
	std::string strExamName;		//��������
	std::string strExamTime;		//����ʱ��
	std::string strExamTypeName;	//������������
	std::string strGradeName;		//�꼶����
	std::string strPersonID;		//����ʱ����
	SUBJECT_LIST lSubjects;			//��Ŀ�б�
	_examInfo_()
	{
		nModel = 0;
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

//++ɨ�����
extern pEXAMINFO			_pCurrExam_;	//��ǰ����
extern pEXAM_SUBJECT		_pCurrSub_;		//��ǰ���Կ�Ŀ
extern pMODEL				_pModel_;		//��ǰɨ��ʹ�õ�ģ��
//--

//������ѧ����Ϣ
typedef std::list<ST_STUDENT> STUDENT_LIST;	//�������б�
extern STUDENT_LIST		g_lBmkStudent;	//������ѧ���б�
#ifdef NewBmkTest
extern ALLSTUDENT_LIST		g_lBmkAllStudent;	//�������������п�Ŀ�ı�����ѧ���б�
extern EXAMBMK_MAP			g_mapBmkMgr;			//���Ա���������ϣ��
#endif

#ifdef TEST_MULTI_SENDER
typedef struct _SendInfo_
{
	CFileUpLoad* pUpLoad;
	int			nPort;
	std::string strIP;
}ST_SENDER, *pST_SENDER;
typedef std::map<std::string, pST_SENDER> MAP_FILESENDER;

extern Poco::FastMutex	_fmMapSender_;
extern MAP_FILESENDER	_mapSender_;
#endif

typedef struct _CompressTask_
{
	bool	bDelSrcDir;				//�Զ�ɾ��ԭ�ļ���
	bool	bReleasePapers;			//�Ƿ��ѹ����Զ��Ƿ��Ծ����Ϣ�����ͷ�pPapersInfo�ڴ�����
	int		nCompressType;			//ѹ�����ͣ�1-ѹ���Ծ����2-��ѹ�Ծ��
	pPAPERSINFO pPapersInfo;		//ѹ�����Ծ���ļ�
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
typedef std::list<pCOMPRESSTASK> COMPRESSTASKLIST;	//ʶ�������б�

extern Poco::FastMutex			g_fmCompressLock;		//ѹ���ļ��б���
extern COMPRESSTASKLIST			g_lCompressTask;		//��ѹ�ļ��б�

extern Poco::Event			g_eTcpThreadExit;
extern Poco::Event			g_eSendFileThreadExit;
extern Poco::Event			g_eFileUpLoadThreadExit;
extern Poco::Event			g_eCompressThreadExit;

//ģ���ļ���Ϣ
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
pMODEL	LoadModelFile(CString strModelPath);		//����ģ���ļ�
bool	SortByArea(cv::Rect& rt1, cv::Rect& rt2);		//���������
bool	SortByPositionX(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionY(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionX2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionY2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionXYInterval(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByTH(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByOmrTH(OMR_QUESTION& rc1, OMR_QUESTION& rc2);
bool	SortStringByDown(std::string& str1, std::string& str2);

bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);
std::string calcFileMd5(std::string strPath);
void	CopyData(char *dest, const char *src, int dataByteSize, bool isConvert, int height);
bool	PicRectify(cv::Mat& src, cv::Mat& dst, cv::Mat& rotMat);
bool	FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//������з���任
bool	FixwarpPerspective(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//����͸�ӱ任
bool	FixWarpAffine2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//3���������任����90����תͼ����Ч��Ŀ����δ�СΪԭ�������ֵ��������
bool	FixwarpPerspective2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//4������͸�ӱ任����90����תͼ����Ч��Ŀ����δ�СΪԭ�������ֵ��������
bool	PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	PicTransfer2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//�����3������4������ı任�������Ƕ�90��ͼ����ת��Ŀ����δ�СΪԭ�������ֵ��������
int		WriteRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char * regValue);
int		ReadRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char* & regValue);
bool	encString(std::string& strSrc, std::string& strDst);
bool	decString(std::string& strSrc, std::string& strDst);

bool	GetInverseMat(RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	GetPosition2(cv::Mat& inverseMat, cv::Rect& rtSrc, cv::Rect& rtDst);

void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);

//--------------	�����ƾ��ߵ�����ģ������	-------------------
// typedef struct _RectPos_
// {
// 	int nIndex;
// 	cv::Rect rt;
// }RECTPOS;
// pMODEL	LoadMakePaperData(std::string strData);	//�����ƾ��ߵ�����ģ������
// bool Pdf2Jpg(std::string strPdfPath, std::string strBaseName);
// bool InitModelRecog(pMODEL pModel);		//��ʼ���ƾ���ģ���ʶ�����
//-----------------------------------------------------------------

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


BOOL CheckProcessExist(CString &str, int& nProcessID);