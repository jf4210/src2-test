#include "stdafx.h"
#include "DecompressThread.h"
#include <algorithm>
#ifndef USE_POCO_UNZIP
#include "miniunz/miniunz.c"
#endif
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"

class DecompressHandler
{
public:
	DecompressHandler()
	{
	}

	~DecompressHandler()
	{
	}

	void onError(const void*, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info)
	{
		std::string strLog = "Decompress ERR: " + info.second;
		g_Log.LogOutError(strLog);
	}

	inline void onOk(const void*, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& info)
	{
		std::string strLog = "Decompress OK: " + info.second.toString(Poco::Path::PATH_UNIX);
		g_Log.LogOut(strLog);
	}
};


CDecompressThread::CDecompressThread(void* pDlg) :m_pDlg(pDlg)
{
	std::cout << "decompressThread start.\n";
}

CDecompressThread::~CDecompressThread()
{
	std::cout << "decompressThread exit.\n";
}

void CDecompressThread::run()
{
	while (!g_nExitFlag)
	{
		pDECOMPRESSTASK pTask = NULL;
		g_fmDecompressLock.lock();
		DECOMPRESSTASKLIST::iterator it = g_lDecompressTask.begin();
		for (; it != g_lDecompressTask.end();)
		{
			pTask = *it;
			it = g_lDecompressTask.erase(it);
			break;
		}
		g_fmDecompressLock.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(200);
			continue;
		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}
}

void CDecompressThread::HandleTask(pDECOMPRESSTASK pTask)
{
	std::string strLog = "获得试卷包解压任务: " + pTask->strFilePath;
	g_Log.LogOut(strLog);

	std::ifstream inp(pTask->strFilePath.c_str(), std::ios::binary);
	if (!inp)
	{
		strLog = "打开试卷包文件失败";
		g_Log.LogOutError(strLog);
		return;
	}
	std::string strOutDir = pTask->strDecompressDir + "\\" + CMyCodeConvert::Gb2312ToUtf8(pTask->strFileBaseName);

	Poco::File decompressDir(strOutDir);
	if (decompressDir.exists())
		decompressDir.remove(true);
	decompressDir.createDirectories();

#ifdef USE_POCO_UNZIP
	Poco::Zip::Decompress dec(inp, strOutDir);
	DecompressHandler handler(pPapers);
	dec.EError += Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(&handler, &DecompressHandler::onError);
	dec.EOk += Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(&handler, &DecompressHandler::onOk);
	dec.decompressAllFiles();
	dec.EError -= Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(&handler, &DecompressHandler::onError);
	dec.EOk -= Poco::Delegate<DecompressHandler, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(&handler, &DecompressHandler::onOk);
#else
	unzFile uf = NULL;

	#ifdef USEWIN32IOAPI
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64A(&ffunc);
	uf = unzOpen2_64(pTask->strFilePath.c_str(), &ffunc);
	#else
	uf = unzOpen64(pTask->strFilePath.c_str());
	#endif

	if (uf == NULL)
	{
		std::string strLog = "打开压缩文件失败:" + pTask->strFilePath;
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
	int ret = 0;
	int opt_do_extract_withoutpath = 0;
	int opt_overwrite = 1;
	const char *password = NULL;
	password = "static";
	
	if (CHDIR(strOutDir.c_str()))
	{
		std::string strLog = "切换目录失败:" + strOutDir;
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
	ret = do_extract_all(uf, opt_do_extract_withoutpath, opt_overwrite, password);
	unzClose(uf);

	if (ret != 0)
	{
		//************	注意：解压失败需要再次尝试	*************************

		std::string strLog = "解压试卷包(";
		strLog.append(pTask->strFileBaseName);
		strLog.append(")失败, 路径: " + pTask->strFilePath);
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return;
	}
	CHDIR(pTask->strDecompressDir.c_str());		//切换回解压根目录，否则删除压缩文件夹失败

	((CDataMgrToolDlg*)m_pDlg)->m_strMsg.Append(_T("解压完成"));
	((CDataMgrToolDlg*)m_pDlg)->UpdateData(FALSE);
//	((CDataMgrToolDlg*)AfxGetMainWnd())->m_strMsg.Append(_T("解压完成"));
#endif
}


