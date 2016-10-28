#pragma once
#include "global.h"
#include "Log.h"
#include "MyCodeConvert.h"

#include "zbar.h"   


#define  MSG_ERR_RECOG		(WM_USER + 110)
#define  MSG_RECOG_COMPLETE	(WM_USER + 110)


extern CLog g_Log;
extern int	g_nExitFlag;

extern CString				g_strCurrentPath;

extern std::string _strEncryptPwd_;
extern pMODEL _pModel_;


extern int		_nCannyKernel_;			//������������

extern int		g_nRecogGrayMin;		//�Ҷȵ�(���հ׵�,OMR��)����Ҷȵ���С���Է�Χ
extern int		g_nRecogGrayMax_White;	//�հ׵�У������Ҷȵ�����Է�Χ
extern int		g_nRecogGrayMin_OMR;	//OMR����Ҷȵ���С���Է�Χ
extern int		g_RecogGrayMax_OMR;		//OMR����Ҷȵ�����Է�Χ

extern double	_dCompThread_Fix_;
extern double	_dDiffThread_Fix_;
extern double	_dDiffExit_Fix_;
extern double	_dCompThread_Head_;
extern double	_dDiffThread_Head_;
extern double	_dDiffExit_Head_;


typedef struct _DecompressTask_
{
	int nTaskType;				//1-��ͨ��ѹ��2-�����Ծ������ͬĿ¼, 3-����ʶ��OMR��SN, 4-��ѹģ��
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
	std::string strDecompressDir;
	_DecompressTask_()
	{
		nTaskType = 1;
	}
}DECOMPRESSTASK, *pDECOMPRESSTASK;
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//ʶ�������б�

extern Poco::FastMutex			g_fmDecompressLock;		//��ѹ�ļ��б���
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//��ѹ�ļ��б�



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
	int		nTotalPics;

	//++ͳ����Ϣ
	int		nOmrDoubt;				//OMR���ɵ�����
	int		nOmrNull;				//OMRʶ��Ϊ�յ�����
	int		nSnNull;				//׼��֤��ʶ��Ϊ�յ�����

	Poco::FastMutex	fmOmrStatistics;//omrͳ����
	Poco::FastMutex fmSnStatistics; //zkzhͳ����
	//--

	int		nRecogPics;				//�Ѿ�ʶ����ɵ�����
	Poco::FastMutex fmRecogCompleteOK; //�������

	Poco::FastMutex fmlPaper;			//���Ծ��б��д��
	Poco::FastMutex fmlIssue;			//�������Ծ��б��д��
	std::string  strPapersName;			//�Ծ������
	std::string	 strPapersDesc;			//�Ծ����ϸ����

	std::string		strPapersPath;
	std::string		strSrcPapersPath;
	std::string		strSrcPapersFileName;

	PAPER_LIST	lPaper;					//���Ծ�����Ծ��б�
	PAPER_LIST	lIssue;					//���Ծ����ʶ����������Ծ��б�
	_PapersInfo_()
	{
		nTotalPics = 0;
		nPaperCount = 0;
		nRecogErrCount = 0;
		nOmrDoubt = 0;
		nOmrNull = 0;
		nSnNull = 0;
		nRecogPics = 0;
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

bool encString(std::string& strSrc, std::string& strDst);
bool decString(std::string& strSrc, std::string& strDst);


void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);

pMODEL	LoadModelFile(CString strModelPath);		//����ģ���ļ�
bool	SortByArea(cv::Rect& rt1, cv::Rect& rt2);		//���������
bool	SortByPositionX(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionY(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionX2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionY2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortbyNumASC(const std::string& x, const std::string& y);
bool	SortByPaper(const pST_PaperInfo& x, const pST_PaperInfo& y);

bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);


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



