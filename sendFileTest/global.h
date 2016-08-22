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

#include "Poco/Net/NetException.h"
#include "Poco/Net/TCPServer.h"

#include "Poco/Net/MulticastSocket.h"

#define MULTICAST_START		0x9001		//�鲥�����ʼ�����ļ�
#define MULTICAST_INIT_PROCESS		0x9002		//�鲥�����ʼ��ʼ������
#define MULTICAST_INIT_THREAD		0x9003		//�鲥�����ʼ��ʼ���߳�

extern int					g_nExitFlag;					//�˳���ʾ
extern Poco::Logger* g_pLogger;
extern Poco::Event*		g_peStartMulticast;

extern std::string			g_strCurPath;

typedef struct _SendTask_
{
	int		nSendTimes;
	std::string strName;
	std::string strPath;
	_SendTask_()
	{
		nSendTimes = 0;
	}
}SENDTASK, *pSENDTASK;
typedef std::list<pSENDTASK> SENDTASKLIST;	//ʶ�������б�

extern Poco::FastMutex		g_fmSendLock;
extern SENDTASKLIST			g_lSendTask;


//---------------	test	-----------------------
typedef struct _RectPos_
{
	int nIndex;
	cv::Rect rt;
}RECTPOS;

#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}
#define SAFE_RELEASE_ARRY(pObj) if(pObj) {delete[] pObj; pObj = NULL;}
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
	float		fStandardValue;					//�˾���ʶ��ʱ����ֵ
	float		fRealValue;						//
	float		fStandardValuePercent;			//�˾���Ҫ��ı�׼ֵ���������ﵽ�����ĻҶ�ֵ�ı���
	float		fRealValuePercent;				//�˾���ʵ�ʵ�ֵ(�Ҷ�ֵ)
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
}RECTINFO, *pRECTINFO;

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
	CString		strModelPicName;		//ģ��ͼƬ����
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
}PAPERMODEL, *pPAPERMODEL;

typedef struct _Model_
{
	int			nEnableModify;			//ģ���Ƿ�����޸�
	int			nPicNum;				//ͼƬ����
	int			nABModel;				//�Ƿ���AB��ģʽ
	int			nHasHead;				//�Ƿ���ˮƽ�ʹ�ֱͬ��ͷ
	int			nExamID;
	int			nSubjectID;
	int			nSaveMode;				//����ģʽ: 1-����ģʽ��2-Զ������ģʽ
	CString		strModelName;			//ģ������
	CString		strModelDesc;			//ģ������

	std::vector<pPAPERMODEL> vecPaperModel;	//�洢ÿһҳ�Ծ��ģ����Ϣ
	_Model_()
	{
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

//-------------------------------------------------
