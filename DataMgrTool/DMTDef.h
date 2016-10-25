#pragma once
#include "global.h"
#include "Log.h"
#include "MyCodeConvert.h"




extern CLog g_Log;
extern int	g_nExitFlag;

extern std::string _strEncryptPwd_;


typedef struct _DecompressTask_
{
	int nTaskType;				//1-普通解压，2-区分试卷包到不同目录
	std::string strFileBaseName;
	std::string strSrcFileName;
	std::string strFilePath;
	std::string strDecompressDir;
	_DecompressTask_()
	{
		nTaskType = 1;
	}
}DECOMPRESSTASK, *pDECOMPRESSTASK;
typedef std::list<pDECOMPRESSTASK> DECOMPRESSTASKLIST;	//识别任务列表

extern Poco::FastMutex			g_fmDecompressLock;		//解压文件列表锁
extern DECOMPRESSTASKLIST		g_lDecompressTask;		//解压文件列表


bool encString(std::string& strSrc, std::string& strDst);
bool decString(std::string& strSrc, std::string& strDst);
