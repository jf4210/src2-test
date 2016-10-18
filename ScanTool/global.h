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

#include "zip.h"
#include "unzip.h"
#include "MyCodeConvert.h"
#include "./pdf2jpg/MuPDFConvert.h"
#include "modelInfo.h"


#include "zbar.h"   

//#define PIC_RECTIFY_TEST	//ͼ����ת��������
#define WarpAffine_TEST		//����任����
#ifdef _DEBUG
	#define PaintOmrSnRect		//�Ƿ��ӡʶ�������OMR����
//	#define Test_ShowOriPosition	//���Դ�ӡģ�������Ӧ��ԭͼ����λ��
#endif
#ifndef WarpAffine_TEST
//	#define TriangleSide_TEST		//���߶�λ�㷨
	#ifndef TriangleSide_TEST
		#define TriangleCentroid_TEST	//���������㷨
	#endif
#endif

//+++++++++	ѡ��汾 ++++++++++++++++++
//#define TO_WHTY							//���人������Ϣʹ�ã���ʶ��ֻɨ���ϴ�
#ifndef TO_WHTY
	#define SHOW_GUIDEDLG				//��ʾ��������,�׿���ѧ�Լ��ã���ʾ��������.******** �˴������壬��ֱ����ʾ������	***********
#endif
//+++++++++++++++++++++++++++

#define USES_GETTHRESHOLD_ZTFB	//ʹ����̬�ֲ���ʽ��ȡУ���ķ�ֵ��δ����ʱʹ�ù̶���ֵ���ж�ֵ�����Ҿ���
#define USES_FILE_ENC			//�Ƿ���ļ�ʹ�ü���

#ifndef TO_WHTY
	#define USES_PWD_ZIP_UNZIP		//�Ƿ�ʹ�������ѹ��
	#define PAPERS_EXT_NAME	_T(".pkg")			//�Ծ��ѹ�������չ��(��֤��4λ)
#else
	#define PAPERS_EXT_NAME	_T(".typkg")			//�Ծ��ѹ�������չ��(��֤��4λ),ͨ�������������ļ���������Ϊ.zip
#endif

#ifdef SHOW_GUIDEDLG
	//#define SHOW_LOGIN_MAINDLG			//�Ƿ�������������ʾ��¼��ť
	//#define SHOW_MODELMGR_MAINDLG			//�Ƿ�������������ʾģ�����ť
	//#define SHOW_MODELMAKE_MAINDLG		//�Ƿ�������������ʾģ��������ť
	//#define SHOW_COMBOLIST_MAINDLG		//�Ƿ�������������ʾ�����б�ؼ�
	//#define SHOW_SCANALL_MAINDLG			//�Ƿ�������������ʾ����ɨ�谴ť
	#define SHOW_PAPERINPUT_MAINDLG			//�Ƿ�������������ʾ�Ծ��밴ť
#elif defined (TO_WHTY)
	//#define SHOW_LOGIN_MAINDLG			//�Ƿ�������������ʾ��¼��ť
	//#define SHOW_MODELMGR_MAINDLG			//�Ƿ�������������ʾģ�����ť
	//#define SHOW_MODELMAKE_MAINDLG		//�Ƿ�������������ʾģ��������ť
	//#define SHOW_COMBOLIST_MAINDLG		//�Ƿ�������������ʾ�����б�ؼ�
	//#define SHOW_SCANALL_MAINDLG			//�Ƿ�������������ʾ����ɨ�谴ť
	//#define SHOW_PAPERINPUT_MAINDLG		//�Ƿ�������������ʾ�Ծ��밴ť
#else
	#define SHOW_LOGIN_MAINDLG				//�Ƿ�������������ʾ��¼��ť
	#define SHOW_MODELMGR_MAINDLG			//�Ƿ�������������ʾģ�����ť
	//#define SHOW_MODELMAKE_MAINDLG		//�Ƿ�������������ʾģ��������ť
	//#define SHOW_COMBOLIST_MAINDLG		//�Ƿ�������������ʾ�����б�ؼ�
	//#define SHOW_SCANALL_MAINDLG			//�Ƿ�������������ʾ����ɨ�谴ť
	#define SHOW_PAPERINPUT_MAINDLG			//�Ƿ�������������ʾ�Ծ��밴ť
#endif

#define  MSG_ERR_RECOG	(WM_USER + 110)

#define SOFT_VERSION	_T("1.1011")
#define SYS_BASE_NAME	_T("YKLX-ScanTool")


#define MAX_DLG_WIDTH	1024
#define MAX_DLG_HEIGHT	768

// #define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}
// #define SAFE_RELEASE_ARRY(pObj) if(pObj) {delete[] pObj; pObj = NULL;}

extern CString				g_strCurrentPath;
extern std::string			g_strPaperSavePath;
extern std::string			g_strModelSavePath;
extern Poco::Logger*		g_pLogger;
extern int					g_nExitFlag;

extern std::string			g_strEncPwd;				//�ļ����ܽ�������
extern std::string			g_strCmdIP;
extern std::string			g_strFileIP;
extern int					g_nCmdPort;
extern int					g_nFilePort;

extern int		_nGauseKernel_;			//��˹�任������
extern int		_nSharpKernel_;			//�񻯺�����
extern int		_nCannyKernel_;			//������������
extern int		_nDilateKernel_;		//���ͺ�����
extern int		_nErodeKernel_;			//��ʴ������

extern int		g_nRecogGrayMin;		//�Ҷȵ�(���հ׵�,OMR��)����Ҷȵ���С���Է�Χ
extern int		g_nRecogGrayMax_White;	//�հ׵�У������Ҷȵ�����Է�Χ
extern int		g_nRecogGrayMin_OMR;	//OMR����Ҷȵ���С���Է�Χ
extern int		g_RecogGrayMax_OMR;		//OMR����Ҷȵ�����Է�Χ

extern bool				g_bCmdConnect;		//����ͨ������
extern bool				g_bFileConnect;		//�ļ�ͨ������

extern bool				g_bCmdNeedConnect;	//����ͨ���Ƿ���Ҫ����������ͨ����ַ��Ϣ�޸ĵ����
extern bool				g_bFileNeedConnect;	//�ļ�ͨ���Ƿ���Ҫ����������ͨ����ַ��Ϣ�޸ĵ����

#if 0
typedef enum CPType
{
	UNKNOWN = 0,
	Fix_CP,			//���㣬���������
	H_HEAD,			//ˮƽͬ��ͷ
	V_HEAD,			//��ֱͬ��ͷ
	ABMODEL,		//AB����
	COURSE,			//��Ŀ
	QK_CP,			//ȱ��
	GRAY_CP,		//�Ҷ�У��
	WHITE_CP,		//��У��
	SN,				//������
	OMR				//ѡ��������
};

typedef struct _RectInfo_
{
	CPType		eCPType;						//У�������
	int			nThresholdValue;				//�˾���ʶ��ʱҪ��ı�׼ֵ�����ﵽ�ĻҶ�ֵ�ķ�ֵ
	float		fStandardValue;					//�˾���ʶ��ʱ��׼����ֵ�������ջҶȷ�ֵ����ĻҶ�ֵ
	float		fRealValue;						//
	float		fStandardValuePercent;			//�˾���Ҫ��ı�׼ֵ���������ﵽ�����ĻҶ�ֵ�ı���
	float		fRealValuePercent;				//�˾���ʵ�ʵı���(�Ҷ�ֵ)
	int			nHItem;							//�ڼ���ˮƽͬ��ͷ
	int			nVItem;							//�ڼ�����ֱͬ��ͷ
	int			nSnVal;							//��ʶ׼��֤������ 0-9
	int			nTH;							//���
	int			nAnswer;						//��ѭ�����ڵڼ����𰸣���1-A,2-B,3-C,4-D,5-E,6-F...
	int			nSingle;						//0-��ѡ��1-��ѡ
	int			nRecogFlag;						//ʶ���ʶ��ʶ��SNʱ--ʶ�𿼺�˳����ѡ���Ŀ��Ŵ��ڱ�ʶֵ��ʶ��OMRʱ--ʶ�����˳����ѡ����OMR���ô��ڵı�ʶֵ
//	cv::Point	ptFix;
	cv::Rect	rt;				//cv::Rect
//	cv::Rect	rtFix;
	_RectInfo_()
	{
		eCPType = UNKNOWN;
		nSnVal = -1;
		nHItem = -1;
		nVItem = -1;
		nTH = -1;
		nAnswer = -1;
		nRecogFlag = 0;
		nSingle = 0;
		nThresholdValue = 0;
		fRealValuePercent = 0.0;
		fStandardValuePercent = 0.0;
		fStandardValue = 0.0;
		fRealValue = 0.0;
//		ptFix = cv::Point(0, 0);
	}
}RECTINFO,*pRECTINFO;

typedef std::list<RECTINFO> RECTLIST;			//����λ���б�

typedef struct _OmrQuestion_			//��Ŀ
{
	int nTH;
	int nSingle;						//0-��ѡ��1-��ѡ
	std::string strRecogVal;			//ʶ������A��B��C...
	RECTLIST	lSelAnswer;				//ѡ���б�
	_OmrQuestion_()
	{
		nTH = -1;
		nSingle = 0;
	}
}OMR_QUESTION, *pOMR_QUESTION;
typedef std::list<OMR_QUESTION> OMRLIST;

typedef struct _OmrResult_
{
	int		nTH;				//���
	int		nSingle;			//0-��ѡ��1-��ѡ
	int		nDoubt;				//0-�޻���, 1-�л���, 2-��ֵ
	std::string strRecogVal;	//ʶ������A��B��C...
	std::string strRecogVal2;
	RECTLIST	lSelAnswer;				//ѡ���б�
	_OmrResult_()
	{
		nDoubt = 0;
		nTH = -1;
		nSingle = 0;
	}
}OMR_RESULT, *pOMR_RESULT;
typedef std::list<OMR_RESULT> OMRRESULTLIST;

typedef struct _SN_
{
	int nItem;			//�ڼ�λ��
	int nRecogVal;		//ʶ��Ľ��
	RECTLIST	lSN;
	_SN_()
	{
		nItem = -1;
	}
}SN_ITEM, *pSN_ITEM;
typedef std::list<pSN_ITEM> SNLIST;

typedef struct _PaperModel_
{
	int			nPaper;					//��ʶ��ģ�����ڵڼ����Ծ�
	int			nPicW;					//ͼƬ��
	int			nPicH;					//ͼƬ��
	std::string	strModelPicName;		//ģ��ͼƬ����
	cv::Rect	rtHTracker;
	cv::Rect	rtVTracker;
	cv::Rect	rtSNTracker;
	SNLIST		lSNInfo;				//SN��Ϣ
	RECTLIST	lSelHTracker;			//ѡ���ˮƽͬ��ͷ����
	RECTLIST	lSelVTracker;			//ѡ��Ĵ�ֱͬ��ͷ����
	RECTLIST	lSelFixRoi;				//ѡ�񶨵��ROI�ľ����б���ѡ����Ĵ���ο�
	OMRLIST		lOMR2;
	RECTLIST	lFix;					//�����б�
	RECTLIST	lH_Head;				//ˮƽУ����б�
	RECTLIST	lV_Head;				//��ֱͬ��ͷ�б�
	RECTLIST	lABModel;				//����У���
	RECTLIST	lCourse;				//��ĿУ���
	RECTLIST	lQK_CP;					//ȱ��У���
	RECTLIST	lGray;					//�Ҷ�У���
	RECTLIST	lWhite;					//�հ�У���
	_PaperModel_()
	{
		nPaper = -1;
		nPicW = -1;
		nPicH = -1;
	}
	~_PaperModel_()
	{
		SNLIST::iterator itSn = lSNInfo.begin();
		for (; itSn != lSNInfo.end();)
		{
			pSN_ITEM pSNItem = *itSn;
			itSn = lSNInfo.erase(itSn);
			SAFE_RELEASE(pSNItem);
		}
	}
}PAPERMODEL,*pPAPERMODEL;

typedef struct _Model_
{
	int			nType;					//���ͣ�0-ͨ��ɨ���ͼƬ�ֶ������ģ�1-ͨ���ƾ����Զ����ɵ�
	int			nEnableModify;			//ģ���Ƿ�����޸�
	int			nPicNum;				//ͼƬ����
	int			nABModel;				//�Ƿ���AB��ģʽ
	int			nHasHead;				//�Ƿ���ˮƽ�ʹ�ֱͬ��ͷ
	int			nExamID;
	int			nSubjectID;
	int			nSaveMode;				//����ģʽ: 1-����ģʽ��2-Զ������ģʽ
	std::string	strModelName;			//ģ������
	std::string	strModelDesc;			//ģ������

	std::vector<pPAPERMODEL> vecPaperModel;	//�洢ÿһҳ�Ծ��ģ����Ϣ
	_Model_()
	{
		nType = 0;
		nEnableModify = 1;
		nPicNum = 0;
		nABModel = 0;
		nHasHead = 1;
		nExamID = 0;
		nSubjectID = 0;
		nSaveMode = 1;
	}
	~_Model_()
	{
		std::vector<pPAPERMODEL>::iterator itModel = vecPaperModel.begin();
		for (; itModel != vecPaperModel.end();)
		{
			pPAPERMODEL pModel = *itModel;
			itModel = vecPaperModel.erase(itModel);
			SAFE_RELEASE(pModel);
		}
	}
}MODEL, *pMODEL;
typedef std::list<pMODEL> MODELLIST;	//ģ���б�
#endif

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
	int			nQKFlag;			//ȱ����ʶ
	pMODEL		pModel;				//ʶ���ѧ���Ծ����õ�ģ��
	void*		pPapers;			//�������Ծ����Ϣ
	void*		pSrcDlg;			//��Դ�������ĸ����ڣ�ɨ��or�����Ծ���
	std::string strStudentInfo;		//ѧ����Ϣ	
	std::string strSN;
	
	SNLIST				lSnResult;
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//ʶ���ѡ����OMR���
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
}ST_PaperInfo, *pST_PaperInfo;		//�Ծ���Ϣ��һ��ѧ����Ӧһ���Ծ�һ���Ծ�����ж��ͼƬ
typedef std::list<pST_PaperInfo> PAPER_LIST;	//�Ծ��б�

typedef struct _PapersInfo_				//�Ծ����Ϣ�ṹ��
{
	int		nPaperCount;				//�Ծ�����Ծ�������(ѧ����)
	int		nRecogErrCount;				//ʶ������Ծ�����
	Poco::FastMutex fmlPaper;			//���Ծ��б��д��
	Poco::FastMutex fmlIssue;			//�������Ծ��б��д��
	std::string  strPapersName;			//�Ծ������
	std::string	 strPapersDesc;			//�Ծ����ϸ����

	PAPER_LIST	lPaper;					//���Ծ�����Ծ��б�
	PAPER_LIST	lIssue;					//���Ծ����ʶ����������Ծ��б�
	_PapersInfo_()
	{
		nPaperCount = 0;
		nRecogErrCount = 0;
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

extern Poco::FastMutex		g_fmRecog;		//ʶ���̻߳�ȡ������
extern RECOGTASKLIST		g_lRecogTask;	//ʶ�������б�

extern Poco::FastMutex		g_fmPapers;		//�����Ծ���б��������
extern PAPERS_LIST			g_lPapers;		//���е��Ծ����Ϣ


//TCP��������
typedef struct _TcpTask_
{
	unsigned short usCmd;
	int		nPkgLen;
	char	szSendBuf[2048];
	_TcpTask_()
	{
		ZeroMemory(szSendBuf, 2048);
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
	int			nSubjCode;		//���Կ�Ŀ����
	std::string strSubjName;	//���Կ�Ŀ����
	std::string strModelName;	//ɨ������ģ������
}EXAM_SUBJECT, *pEXAM_SUBJECT;
typedef std::list<EXAM_SUBJECT> SUBJECT_LIST;

typedef struct _examInfo_
{
	int			nExamID;			//����ID
	int			nExamGrade;			//�꼶
	int			nExamState;			//����״̬
	std::string strExamName;		//��������
	std::string strExamTypeName;	//������������
	std::string strGradeName;		//�꼶����
	SUBJECT_LIST lSubjects;			//��Ŀ�б�
}EXAMINFO, *pEXAMINFO;
typedef std::list<EXAMINFO> EXAM_LIST;

extern EXAM_LIST	g_lExamList;


extern Poco::Event			g_eTcpThreadExit;
extern Poco::Event			g_eSendFileThreadExit;
extern Poco::Event			g_eFileUpLoadThreadExit;

int		GetRectInfoByPoint(cv::Point pt, CPType eType, pPAPERMODEL pPaperModel, RECTINFO*& pRc);
bool	ZipFile(CString strSrcPath, CString strDstPath, CString strExtName = _T(".zip"));
bool	UnZipFile(CString strZipPath);
pMODEL	LoadModelFile(CString strModelPath);		//����ģ���ļ�
bool	SortByArea(cv::Rect& rt1, cv::Rect& rt2);		//���������
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
bool	FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//������з���任
bool	FixwarpPerspective(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//����͸�ӱ任
bool	PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
int		WriteRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char * regValue);
int		ReadRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char* & regValue);
bool	encString(std::string& strSrc, std::string& strDst);
bool	decString(std::string& strSrc, std::string& strDst);

bool	GetInverseMat(RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	GetPosition2(cv::Mat& inverseMat, cv::Rect& rtSrc, cv::Rect& rtDst);

void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);

//--------------	�����ƾ��ߵ�����ģ������	-------------------
typedef struct _RectPos_
{
	int nIndex;
	cv::Rect rt;
}RECTPOS;
pMODEL	LoadMakePaperData(std::string strData);	//�����ƾ��ߵ�����ģ������
bool Pdf2Jpg(std::string strPdfPath, std::string strBaseName);
bool InitModelRecog(pMODEL pModel);		//��ʼ���ƾ���ģ���ʶ�����
//-----------------------------------------------------------------

//----------------	OMRʶ��ҶȲ�ֵ�Ƚ�	------------------
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

//----------------	��ά�롢����ʶ��	------------------
//zbar�ӿ�
std::string ZbarDecoder(cv::Mat img, std::string& strTypeName);

//�Զ�ֵͼ�����ʶ�����ʧ����������ж���ʶ��
std::string GetQRInBinImg(cv::Mat binImg, std::string& strTypeName);

//main function
std::string GetQR(cv::Mat img, std::string& strTypeName);
//--------------------------------------------------------
