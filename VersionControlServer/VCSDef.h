#pragma once
#include "global.h"
#include "Net_Cmd_Protocol_Ver.h"
#include "NetIoBuffer.h"
#include "MissionFactory.h"
#include "MyCodeConvert.h"
#include "Log.h"
#include "SysSet.h"



extern CLog g_Log;
extern int	g_nExitFlag;

#define SOFT_VERSION "VerControlServer V1.0"

#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}

typedef struct _FileInfo_
{
	std::string strFileName;
	std::string strMd5;
	std::string strFilePath;
}ST_FILEINFO, *pST_FILEINFO;
typedef std::list<pST_FILEINFO> LIST_FILEINFO;
extern LIST_FILEINFO g_lFileInfo;

std::string calcFileMd5(std::string strPath);


