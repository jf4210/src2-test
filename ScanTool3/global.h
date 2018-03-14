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

//#define PIC_RECTIFY_TEST	//ͼ����ת��������
#define WarpAffine_TEST		//����任����
#ifdef _DEBUG
	#define PaintOmrSnRect		//�Ƿ��ӡʶ�������OMR����
	#define PrintRecogLog		//��ӡʶ����־
	#define PrintAllOmrVal		//��ӡ����OMRѡ��ֵ
	#define PrintWordRecogPoint		//��ӡͨ�����ֶ�λʱ��������ľ��������λ��
//	#define Test_ShowOriPosition	//���Դ�ӡģ�������Ӧ��ԭͼ����λ��
//	#define Test_Data			//�������ݣ�����ģʽ
//	#define TEST_TIP_SHOW		//��ʾ��Ϣ��ʾ����
	#define TEST_MULTI_SENDER	//�ļ����Ͳ��ԣ��ö����ַ���Ͳ���
	#define TEST_PAPERS_INPUT	//�Ծ�����빦�ܲ���
	#define TEST_MODEL_ROTATION	//ģ����ת����
	
	#define TEST_GRAY_WHITE		//�հ״��ҶȲ���
//	#define TEST_FAST_SCAN		//����ɨ��ģʽ����ɨ���߳��в�д�ļ�
	#define TEST_PAGINATION		//ҳ����ԣ���ҳ�Ծ���������ҳA3��A4
	
	#ifdef TEST_PAGINATION
		#define	TEST_EXCEPTION_DLG	//�µ��쳣�����ڲ���
	#endif
	#define TEST_ModelMgr		//ʹ��ģ�屣������ౣ��ģ��

	#define TEST_New_RecogClass	//ʹ����ʶ�������
#else	//release�汾
	#define TEST_MULTI_SENDER	//�ļ����Ͳ��ԣ��ö����ַ���Ͳ���
	#define PUBLISH_VERSION			//�����汾,�����汾�����š��Ծ��빦�ܡ�

	#define TEST_MODEL_ROTATION	//ģ����ת����
//	#define USE_TESSERACT		//ʹ��Tesseract�������ֺ���ʶ��

 	#define TEST_PAPERS_INPUT	//�Ծ�����빦�ܲ���
// 	#define TEST_PAGINATION		//ҳ����ԣ���ҳ�Ծ���������ҳA3��A4
#endif

#ifdef USE_TESSERACT
	#include "tesseract/baseapi.h"
	#include "leptonica/allheaders.h"
#endif

#ifndef WarpAffine_TEST
//	#define TriangleSide_TEST		//���߶�λ�㷨
	#ifndef TriangleSide_TEST
		#define TriangleCentroid_TEST	//���������㷨
	#endif
#endif

#define Test_RecogOmr3			//��3��OMRʶ�𷽷�����

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
		#define SOFT_VERSION	_T("v2.8")
	#else
		#define SOFT_VERSION	_T("v2.8")		//-Pri
	#endif
#else
	#define SOFT_VERSION	_T("v2.1")
#endif
#define SYS_BASE_NAME	_T("��ѧɨ��ͻ���")		//�����ľ�ɨ��ͻ���	//��������ɨ��ͻ���
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


//++������Ϣ
extern std::string _strCopyright_;
extern std::string _strMainTitle_;
extern std::string _strLitteTitle_;
//--


//================================================================================
/*
����������ģʽ��
1����Ҫ���ʵ��ɨ���Ծ�����Ҫ��ɨ�����Ƿ�һ��
2��ɨ����ɺ�׼��֤������
3���쳣������棬�����ֶ�����׼��֤�ţ�ֻ�ܴ����ݿ��ѯ
*/
extern int				g_nHighSevereMode;
extern int				g_nDefStudentsInKC;	//Ĭ�Ͽ����п�������
//================================================================================

extern int				_nUseOcrRecogSn_;		//ʹ��OCRʶ��׼��֤��

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
extern Poco::Event			g_eGetModelPic;		//����ģ��ͼƬ״̬
//--
extern int					g_nDownLoadModelStatus;		//����ģ���״̬	0-δ���أ���ʼ����1-ģ�������У�2-���سɹ���3-���ش��ڴ��ļ�������Ҫ����, -1-�������˿�Ŀģ�岻����, -2-��������ȡ�ļ�ʧ��
extern int					_nScanStatus_;				//ɨ����� 0-δɨ�裬1-����ɨ�裬2-ɨ�����, 3-ɨ����ֹ, -1--����ɨ����ʧ��, -2--����ɨ����ʧ��, -3--ɨ��ʧ��, -4-ɨ��ȡ��
extern int					_nScanCount_;				//ɨ�����������ǰ��ɨ����ٷ�
extern int					_nScanPaperCount_;			//��ǰ�Ѿ�ɨ�������������������ʼ����
extern int					_nGetModelPic_;				//��ȡģ��ͼƬ״̬��0-δ��ȡ��1-���ڻ�ȡ(����)�� 2-��ȡģ��ͼƬ�ɹ���3-��������ģ��ͼƬ��4-��������ȡʧ��
extern int					_nDoubleScan_;				//�Ƿ�˫��ɨ�裬0-���棬 1-˫�棬����˫��ͼ�����ʱ����
extern int					_nScanAnswerModel_;			//0-ɨ���Ծ� 1-ɨ��Omr�𰸣� 2-ɨ���������

extern CString				g_strCurrentPath;
extern std::string			g_strPaperSavePath;
extern std::string			g_strModelSavePath;
extern std::string			g_strPaperBackupPath;	//�Ծ�����ɺ�ı���·��
extern std::string			g_strFileVersion;		//�ļ��汾��2.7.12.8
extern Poco::Logger*		g_pLogger;
extern int					g_nExitFlag;

extern std::string			g_strEncPwd;				//�ļ����ܽ�������
extern std::string			g_strCmdIP;
extern std::string			g_strFileIP;
extern int					g_nCmdPort;
extern int					g_nFilePort;
extern std::string			g_strFileIp4HandModel;		//����ģʽʱ�ļ���������ַ
extern int					g_nFilePort4HandModel;		//����ģʽʱ�ļ��������˿�

extern double	_dAnswerSure_DensityFix_;	//�ܶ��㷨ȷ��Ϊ�𰸵ı���
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
extern int				g_nReUploadWaitTime;	//�ϴ��ļ���ɺ�ȴ���������Ӧʱ�䣬������ʱ�������ϴ�

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
	Dlg_ScanRecordMgr,		//ɨ���¼�����ڣ���������Ϣչʾ����
	Dlg_NewMakeModel,		//ģ����������
	Dlg_PapersInput			//�Ծ�����봰��
}E_DLG_TYPE;
extern E_DLG_TYPE		_eCurrDlgType_;	//��ǰ��ʾ�Ĵ��ڣ��������ڲ���

//��ɨ���ǻ�ȡ��ͼ����Ϣ
typedef struct _ScanPic
{
	int nOrder;					//�������Ծ�˳��1.jpg��2.jpg ...(S1_1,S1_2,S1_3�е�1��2��3)
	int nStudentID;				//������������S1��S2��S3 ...
	int nModelPicID;			//�����Ծ���ģ�������ڵڼ�ҳ�Ծ�, ��0��ʼ
	void*	pNotifyDlg;			//������ɵ�֪ͨ����
	void* pParentScanPaper;		//������ɨ���Ծ�
	std::string strPicName;
	std::string strPicPath;		//gb2312
	cv::Mat mtPic;
}ST_SCAN_PIC, *pST_SCAN_PIC;
typedef struct _ScanPaper
{
	bool bDoubleScan;			//�Ƿ�˫��ɨ�裬����ɨ��ʱ��pScanPic2������
	bool bCanRecog;				//�Ƿ���Խ���ʶ��ʹ�ö�ҳģʽʱ��Ч���ڸ���ҳ����������Ծ�����ģ��ĵڼ����Ծ�ʧ�ܣ���Ҫ�˹�ȷ�ϣ�֮����ʶ��
	int  nSrcDlgType;			//0-����ɨ���̵߳����ݣ�1-�����Ծ��봰�ڵ�����
	int  nPaperID;				//ɨ��ĵڼ����Ծ�
	int  nModelPaperID;			//�����Ծ���ģ�������ڵڼ����Ծ�
	void* pPapersInfo;			//�����Ծ��
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
typedef std::list<pST_SCAN_PAPER> SCAN_PAPER_LIST;			//��ɨ���ǻ�ȡ��ͼ����Ϣ���б�
extern Poco::FastMutex			g_fmScanPaperListLock;		//��ɨ���ǻ�ȡ��ͼ����Ϣ���б���
extern SCAN_PAPER_LIST			g_lScanPaperTask;			//��ɨ���ǻ�ȡ��ͼ����Ϣ���б�

#if 1
typedef struct _PicInfo_				//ͼƬ��Ϣ
{
	bool			bFindIssue;		//�Ƿ��ҵ������
	bool			bRecogCourse;		//��Ŀʶ���Ƿ���ȷ
	int 			nRecoged;		//�Ƿ��Ѿ�ʶ���, 0-δʶ��1-����ʶ��2-ʶ�����
	int				nRecogRotation;	//ʶ��������ж���Ҫ�����ķ���1:���ģ��ͼ����Ҫ���е���ת�����򣬲���Ҫ��ת��2����ת90(ģ��ͼ����ת), 3����ת90(ģ��ͼ����ת), 4����ת180(ģ��ͼ����ת)
	int				nPicSaveRotation;/*ͼ���ԭʼ��������Ӿ��ķ���(������������)��������ʶ��ʱҪ�������Ӿ���������ģ�屣��ʱ����
									0:δ֪����1: �����Ӿ�����(����������)��2-������������90��ķ���3-������������90��ķ���4-����������ת180�Ⱥ�ķ���*/
	int				nPicModelIndex;	//ͼƬ����, ����ͼƬ������ģ��ĵڼ�ҳ����0����
	int				nPicOldModelIndex;	//��Զ�ҳģʽ���޸�ͼƬ��ģ������ʱ����¼�ƶ�ǰ����ģ��ĵڼ�ҳ�����ƶ�ͼƬ�����Ծ��omr����Ϣʱ����
	int				nQKFlag;			//ȱ����ʶ
	int				nWJFlag;			//Υ�ͱ�ʶ
	void*			pPaper;			//�����Ծ����Ϣ
	pST_SCAN_PIC	pSrcScanPic;	//ԭʼɨ���ͼ����Ϣ
	cv::Rect		rtFix;			//�������
	std::string		strPicName;		//ͼƬ����
	std::string		strPicPath;		//ͼƬ·��	gb2312
	std::string		strPicZKZH;		//��ҳģʽ��Ч��ͼƬ������׼��֤��
	RECTLIST		lFix;			//�����б�
	RECTLIST		lNormalRect;	//ʶ�������������λ��
	RECTLIST		lIssueRect;		//ʶ������������Ծ�������λ�ã�ֻҪ���������Ͳ�������һҳ��ʶ��(�ϸ�ģʽ)�����ߴ洢�Ѿ����ֵ�����㣬���Ǽ��������ʶ��(��ģʽ)
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//ʶ���ѡ����OMR���

	RECTLIST		lCalcRect;		//ͨ�����������ĵ�λ��
	RECTLIST		lModelWordFix;		//ģ�����ֶ����б���ʹ�����ֶ�λʱ����
	CHARACTER_ANCHOR_AREA_LIST lCharacterAnchorArea;	//���ֶ�λ����
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
typedef std::list<pST_PicInfo> PIC_LIST;	//ͼƬ�б���

typedef struct _PaperInfo_
{
	bool		bIssuePaper;		//�Ƿ��������Ծ�
	bool		bModifyZKZH;		//׼��֤���˹��޸ı�ʶ
	bool		bModifyPagination;	//��ҳģʽʱ����ҳ���쳣�������й��޸�
	bool		bRecogComplete;		//��ѧ���Ѿ�ʶ�����
	bool		bReScan;			//����ɨ���ʶ����׼��֤���޸Ĵ���������
	bool		bRecogCourse;		//��Ŀʶ���Ƿ���ȷ
	int			nPicsExchange;		//ͼ�������ʶ������һҳ��ڶ�ҳ����λ�ã�����������0-δ������1-����1�Ρ�����
	int			nPaginationStatus;	/*��ҳģʽʱ���Ծ�״̬��
									0-û��ʶ��ҳ�룬���ܲ���ʶ������������˹�ȷ�Ϻ���ʶ��1-ʶ����ҳ�룬����ʶ�𣬲���ȷ�����������ĸ�ѧ��(Ĭ��)��2-����ʶ���꣬����ȷ�������ĸ������������Ծ�
									3-ͼƬ������ģ�岻һ�£�4-�����ظ���ҳ��
									*/
	//int			nPagination_Del_Paper;	//��ҳģʽ�£�1-���Ծ�����ɾ��,0-δ��ɾ����ɾ�����Ծ���������Ծ��б��в���ʾ
	int			nQKFlag;			//ȱ����ʶ
	int			nWJFlag;			//Υ�ͱ�ʶ
	int			nZkzhInBmkStatus;	//׼��֤���Ƿ��ڱ������д��ڣ��ڱ������б�����ʱ��������Ч, 0--�������в����ڣ�1--�������д��ڣ�-1--ɨ��ʱ�غ���
	//++��Pkg�ָ�Papersʱ�Ĳ���
	int			nChkFlag;			//��ͼƬ�Ƿ�Ϸ�У�飻���Ծ��������Ծ�ͼƬ�����ͼƬ���������Param.dat�в����ڣ�����Ϊ���Ծ�ͼƬ�Ǵ���ͼƬ�����M�ЈDƬʶ��
	//--
	//int			nScanTmpIndex;		//��ɨ���ȡ��һ���Ծ����Ϣ��ֱ�ӹ�������ʱ�Ծ��������Ծ�ʶ������ٺϲ�������Ŀ���
	int			nIndex;				//���Ծ���е���������S1Ϊ1��S2Ϊ2��S3Ϊ3...
	pMODEL		pModel;				//ʶ���ѧ���Ծ����õ�ģ��
	void*		pPapers;			//�������Ծ����Ϣ
	void*		pSrcDlg;			//��Դ�������ĸ����ڣ�ɨ��or�����Ծ���
	std::string strStudentInfo;		//ѧ����Ϣ, S1��S2��S3...
	std::string strSN;				//ʶ����Ŀ��š�׼��֤��
	std::string strRecogSN4Search;	//����ģ�������Ŀ��ţ���δʶ������Ĳ�����#���棬���ڽ���ģ������
	SCAN_PAPER_LIST lSrcScanPaper;	//ԭʼɨ����Ծ���Ϣ

	SNLIST				lSnResult;
	OMRRESULTLIST		lOmrResult;			//OMRRESULTLIST
	ELECTOMR_LIST		lElectOmrResult;	//ʶ���ѡ����OMR���
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
}ST_PaperInfo, *pST_PaperInfo;		//�Ծ���Ϣ��һ��ѧ����Ӧһ���Ծ�һ���Ծ�����ж��ͼƬ
typedef std::list<pST_PaperInfo> PAPER_LIST;	//�Ծ��б�

typedef struct _PapersInfo_				//�Ծ����Ϣ�ṹ��
{
	int		nPapersType;				//�Ծ����ͣ�0-�����������Ծ������ɨ��ʱ������1-��Pkg�ָ���Papersʱ������
	int		nPaperCount;				//�Ծ�����Ծ�������(ѧ����)
	int		nRecogErrCount;				//ʶ������Ծ�����

	int		nMustScanNum;				//����ɨ����Ծ��������ڸ�����ģʽʱ��Ч�������Ծ�ʱ�����ɨ��������Ƿ��������һ�£���һ�²����ύ��ֻ����ɨ

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
	_SendInfo_()
	{
		pUpLoad = NULL;
	}
}ST_SENDER, *pST_SENDER;
typedef std::map<std::string, pST_SENDER> MAP_FILESENDER;

extern Poco::FastMutex	_fmMapSender_;
extern MAP_FILESENDER	_mapSender_;
#endif

//ģ��ͼ��·�������ڴ���ģ����
typedef struct tagModelPicPath
{
	CString strName;
	CString strPath;
}MODELPICPATH;
extern std::vector<MODELPICPATH> _vecModelPicPath_;		//���ڴӷ�����������ͼ���ֱ�Ӵ���ģ��

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

//----------------------------------------------
//�����������������ľ���λ��
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

//��ľ���Ȩ�أ�����������һ���뾶��Χ�ڣ��ж��������������У��������һ���뾶����Ȩ��+2���������2���뾶�������+1
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

//����붥��ľ���
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
pMODEL	LoadModelFile(CString strModelPath);		//����ģ���ļ�
bool	SortByCharAnchorArea(pST_CHARACTER_ANCHOR_AREA& st1, pST_CHARACTER_ANCHOR_AREA& st2);
bool	SortByCharacterConfidence(pST_CHARACTER_ANCHOR_POINT& st1, pST_CHARACTER_ANCHOR_POINT& st2);
bool	SortByArea(cv::Rect& rt1, cv::Rect& rt2);		//���������
bool	SortByPositionX(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionY(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByPositionX2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionY2(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByPositionXYInterval(cv::Rect& rt1, cv::Rect& rt2);
bool	SortByTH(RECTINFO& rc1, RECTINFO& rc2);
bool	SortByOmrTH(OMR_QUESTION& rc1, OMR_QUESTION& rc2);
bool	SortStringByDown(std::string& str1, std::string& str2);

bool    GetPicFix(int nPic, pST_PicInfo pPic, pMODEL pModel);	//ʹ�����ֶ�λʱ���������ֶ�λ�Ķ����б�ֻ��Ҫ2��
bool	GetFixDist(int nPic, pST_PicInfo pPic, pMODEL pModel);	//
bool	GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, cv::Rect& rt);
bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);
std::string calcFileMd5(std::string strPath);
void	CopyData(char *dest, const char *src, int dataByteSize, bool isConvert, int height);
bool	PicRectify(cv::Mat& src, cv::Mat& dst, cv::Mat& rotMat);
bool	FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//������з���任
bool	FixwarpPerspective(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//����͸�ӱ任
bool	FixWarpAffine2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);		//3���������任����90����תͼ����Ч��Ŀ����δ�СΪԭ�������ֵ��������
bool	FixwarpPerspective2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//4������͸�ӱ任����90����תͼ����Ч��Ŀ����δ�СΪԭ�������ֵ��������

bool	GetFixPicTransfer(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, cv::Mat& inverseMat);	//���ݶ������ͼ��任
bool	PicTransfer(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	PicTransfer2(int nPic, cv::Mat& matCompPic, cv::Mat& matDstPic, RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);	//�����3������4������ı任�������Ƕ�90��ͼ����ת��Ŀ����δ�СΪԭ�������ֵ��������
int		WriteRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char * regValue);
int		ReadRegKey(HKEY root, char * subDir, DWORD regType, char * regKey, char* & regValue);
bool	encString(std::string& strSrc, std::string& strDst);
bool	decString(std::string& strSrc, std::string& strDst);

bool	GetInverseMat(RECTLIST& lFix, RECTLIST& lModelFix, cv::Mat& inverseMat);
bool	GetPosition2(cv::Mat& inverseMat, cv::Rect& rtSrc, cv::Rect& rtDst);

void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);

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
BOOL KillProcessFromName(CString strProcessName);