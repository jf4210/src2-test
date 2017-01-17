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
//	#define TEST_MODE	//����ģʽ�������˷������ݣ�����ģ�����
//	#define TEST_FILE_PRESSURE	//�ļ��ϴ�ѹ������
#endif

#define USES_FILE_CRYPTOGRAM			//�Ƿ���ļ�ʹ������
//#define USE_POCO_UNZIP			//ʹ��poco��ѹ��zip�ļ�,��ѹ�޼����ļ�


// //#define TO_WHTY							//���人������Ϣʹ�ã���ʶ��ֻɨ���ϴ�
// #ifdef TO_WHTY
// 	#define USE_POCO_UNZIP			//���人������Ϣʹ��,��ʹ�ü���ѹ��
// #endif

#define DecompressTest		//��ѹ���ԣ����߳̽�ѹ

#define SOFT_VERSION "DataControlServer V1.60111"


#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}

extern CLog g_Log;
extern int	g_nExitFlag;

typedef std::map<std::string, std::string> MAP_PIC_ADDR;
extern Poco::FastMutex _mapPicAddrLock_;
extern MAP_PIC_ADDR _mapPicAddr_;

typedef std::map<std::string, CNetUser*>	MAP_USER;
extern Poco::FastMutex	_mapUserLock_;
extern MAP_USER		_mapUser_;					//�û�ӳ��

typedef struct _ModelInfo_
{
	int		nExamID;
	int		nSubjectID;
	pMODEL		pModel;
	pST_MODELINFO pUploadModelInfo;			//�����ϴ�ģ��ʱ�ϴ���ģ����Ϣ
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
	int		nTimes;			//��ѹ�Ĵ���
	int		nType;			//��ѹ���ͣ�1-�Ծ��pkg����2-ģ��mod
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
	_DecompressTask_()
	{
		nType = 1;
		nTimes = 0;
	}
}DECOMPRESSTASK, *pDECOMPRESSTASK;
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//ʶ�������б�

extern Poco::FastMutex			g_fmDecompressLock;		//��ѹ�ļ��б���
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//��ѹ�ļ��б�


// ͼƬ�ļ�
typedef struct _Pic_
{
	bool		bUpLoadFlag;		//�ϴ�http�������ɹ���־
	int			nPicW;				//ͼƬ������ģ����Ϣ��ʱ������
	int			nPicH;				//ͼƬ�ߣ�����ģ����Ϣ��ʱ������
	std::string strFileName;
	std::string strFilePath;
	std::string strHashVal;			//�ϴ�http�󷵻ص�hash
	_Pic_()
	{
		bUpLoadFlag = false;
		nPicW = 0;
		nPicH = 0;
	}
}PIC_DETAIL, *pPIC_DETAIL;
typedef std::list<pPIC_DETAIL> LIST_PIC_DETAIL;

//�Ծ�,��Կ���
typedef struct _Paper_
{
	int			nChkFlag;	//��ͼƬ�Ƿ�Ϸ�У�飻���Ծ��������Ծ�ͼƬ�����ͼƬ���������Param.dat�в����ڣ�����Ϊ���Ծ�ͼƬ�Ǵ���ͼƬ������zimgͼ��������ύ
	int			nQkFlag;	//ȱ����ʶ,0-δȱ��, 1-ȱ��
	int			nHasElectOmr;	//�Ƿ��ж�ѡ��
	std::string strName;	//ʶ������Ŀ�������S1��S2��������
	std::string strMd5Key;	//����˵�MD5--�ܺ�
	std::string strZkzh;	//ʶ������Ŀ������кš�׼��֤��
	std::string strSnDetail;	//zkzh����ϸʶ����������ļ���ȡ
	std::string strOmrDetail;	//ͬ��
	std::string strElectOmrDetail;	//ѡ������ϸʶ�����

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
//�Ծ��
typedef struct _Papers_
{
	int			nTaskCounts;		//���Ծ���������������ύ��ͼƬ����Ҫ������ύͼƬ���ݡ�׼��֤�š�OMR��ѡ������Ϣ
									//�����ݿ����������Ծ����������������Ϻ�����Ծ��ɾ������
	int			nTotalPics;			//�ܵ�ͼƬ��
	int			nUpLoadSuccess;
	int			nUpLoadFail;
	int			nTotalPaper;		//��ѧ�����������Ծ���ļ��ж�ȡ
	int			nQk;				//ȱ��ѧ�����������Ծ���ļ��ж�ȡ
	int			nExamID;			//����ID
	int			nSubjectID;			//��ĿID
	int			nTeacherId;			//��ʦID
	int			nUserId;			//�û�ID

	//++
	int		nOmrDoubt;				//OMR���ɵ�����
	int		nOmrNull;				//OMRʶ��Ϊ�յ�����
	int		nSnNull;				//׼��֤��ʶ��Ϊ�յ�����
	//--
	int		nResultSendState;		//�Ծ����ͼƬMD5��ַ�ύ��OMR��׼��֤�š�ѡ���������͸����EZS�����״̬��
									//0-δ��ɣ�1-ͼƬ��ַ�ύ��ɣ�2-OMR����ύ��ɣ�3-ZKZH�ύ��ɣ�4-ѡ������Ϣ�ύ���

	std::string	strUploader;		//�ϴ���
	std::string strEzs;				//�ϴ�����˷������ã�--cookie
	std::string strDesc;			//���Ծ���ļ��ж�ȡ
	std::string strPapersName;		//�Ծ������
	std::string strPapersPath;		//�Ծ��·������ѹ���
	std::string strSrcPapersFileName;	//�Ծ���ļ����ƣ�ԭʼѹ��������	gb2312
	std::string strSrcPapersPath;	//�Ծ���ļ�·����ԭʼѹ����·��
	Poco::FastMutex	fmNum;			//���ϴ�����ļ�����������
	Poco::FastMutex	fmTask;			//���ύ������ݵ�����(��:nTaskCounts)������������
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
extern LIST_PAPERS_DETAIL	g_lPapers;		//�Ծ���б�


typedef struct _SendHttpTask_
{
	int			nTaskType;			//��������: 1-��img�������ύͼƬ��2-������ύͼƬ����, 3-�ύOMR��4-�ύZKZH��5-�ύѡ������Ϣ, 6-ģ��ͼƬ�ύzimg������, 7-�Ծ��ֻ�ύOMR��SN��ѡ����Ϣ
	int			nSendFlag;			//���ͱ�ʾ��1������ʧ��1�Σ�2������ʧ��2��...
	Poco::Timestamp sTime;			//��������ʱ�䣬���ڷ���ʧ��ʱ��ʱ����
	pPIC_DETAIL pPic;
	pPAPERS_DETAIL pPapers;
	std::string strUri;
	std::string strResult;			//���Ϳ�����Ϣ����˷���������nTaskType = 2ʱ����
	std::string strEzs;				//��nTaskType = 2ʱ����
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
extern LIST_SEND_HTTP		g_lHttpSend;		//����HTTP�����б�

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
extern LIST_SCAN_REQ		g_lScanReq;		//ɨ������������б�

bool encString(std::string& strSrc, std::string& strDst);
bool decString(std::string& strSrc, std::string& strDst);

std::string calcFileMd5(std::string strPath);