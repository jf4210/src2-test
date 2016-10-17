#pragma once
#include "global.h"
#include "Log.h"
#include "MyCodeConvert.h"




extern CLog g_Log;
extern int	g_nExitFlag;

extern std::string _strEncryptPwd_;


typedef struct _DecompressTask_
{
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
	std::string strDecompressDir;
}DECOMPRESSTASK, *pDECOMPRESSTASK;
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//ʶ�������б�

extern Poco::FastMutex			g_fmDecompressLock;		//��ѹ�ļ��б���
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//��ѹ�ļ��б�


bool encString(std::string& strSrc, std::string& strDst);
bool decString(std::string& strSrc, std::string& strDst);
