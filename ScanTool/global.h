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
	#define	 TEST_MODEL_NAME	//ģ�����Ʋ���
//	#define	 TEST_SCAN_THREAD	//ɨ���̲߳���
#else	//release�汾
	#define	 TEST_MODEL_NAME	//ģ�����Ʋ���
//	#define PUBLISH_VERSION			//�����汾,�����汾�����š��Ծ��빦�ܡ�
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

//#define WH_CCBKS	//�人���ű�����ר��
//+++++++++++++++++++++++++++

#define USES_GETTHRESHOLD_ZTFB	//ʹ����̬�ֲ���ʽ��ȡУ���ķ�ֵ��δ����ʱʹ�ù̶���ֵ���ж�ֵ�����Ҿ���
#define USES_FILE_ENC			//�Ƿ���ļ�ʹ�ü���
#define MSG_NOTIFY_UPDATE (WM_APP + 101)

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
	#ifndef PUBLISH_VERSION
		#define SHOW_PAPERINPUT_MAINDLG			//�Ƿ�������������ʾ�Ծ��밴ť
	#endif
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
	#ifndef PUBLISH_VERSION
		#define SHOW_PAPERINPUT_MAINDLG			//�Ƿ�������������ʾ�Ծ��밴ť
	#endif
#endif

#define  MSG_ERR_RECOG	(WM_USER + 110)

#define SOFT_VERSION	_T("1.1208")
#define SYS_BASE_NAME	_T("YKLX-ScanTool")
#define SYS_GUIDE_NAME	_T("GuideDlg")


#define MAX_DLG_WIDTH	1024
#define MAX_DLG_HEIGHT	768

// #define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}
// #define SAFE_RELEASE_ARRY(pObj) if(pObj) {delete[] pObj; pObj = NULL;}

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

extern double	_dCompThread_Fix_;
extern double	_dDiffThread_Fix_;
extern double	_dDiffExit_Fix_;
extern double	_dCompThread_Head_;
extern double	_dDiffThread_Head_;
extern double	_dDiffExit_Head_;
extern int		_nThreshold_Recog2_;	//��2��ʶ�𷽷��Ķ�ֵ����ֵ

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

#if 1
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

	//++ͳ����Ϣ
	int		nOmrDoubt;				//OMR���ɵ�����
	int		nOmrNull;				//OMRʶ��Ϊ�յ�����
	int		nSnNull;				//׼��֤��ʶ��Ϊ�յ�����

	Poco::FastMutex	fmOmrStatistics;//omrͳ����
	Poco::FastMutex fmSnStatistics; //zkzhͳ����
	//--

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


typedef struct _CompressTask_
{
	bool	bDelSrcDir;				//�Զ�ɾ��ԭ�ļ���
	std::string strSrcFilePath;
	std::string strCompressFileName;
	std::string strSavePath;
	std::string strExtName;
	_CompressTask_()
	{
		bDelSrcDir = true;
	}
}COMPRESSTASK, *pCOMPRESSTASK;
typedef std::list<pCOMPRESSTASK> COMPRESSTASKLIST;	//ʶ�������б�

extern Poco::FastMutex			g_fmCompressLock;		//ѹ���ļ��б���
extern COMPRESSTASKLIST			g_lCompressTask;		//��ѹ�ļ��б�

extern Poco::Event			g_eTcpThreadExit;
extern Poco::Event			g_eSendFileThreadExit;
extern Poco::Event			g_eFileUpLoadThreadExit;
extern Poco::Event			g_eCompressThreadExit;

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


BOOL CheckProcessExist(CString &str);