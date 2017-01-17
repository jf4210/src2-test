#include "stdafx.h"
#include "ThreadMgr.h"


CThreadMgr::CThreadMgr()
: m_nThreads(0)
{
}


CThreadMgr::~CThreadMgr()
{
	StopThread();
}

void CThreadMgr::InitThread(int nTheads)
{
	m_nThreads = nTheads;
	for (int i = 0; i < m_nThreads; i++)
	{
//		CreateThread(NULL, 0, FindFileThread, &param, 0, NULL);
	}
}

void CThreadMgr::StopThread()
{
	if (m_vecThread.size() > 0)
	{
		for (int i = 0; i < m_vecThread.size(); i++)
		{
			if (m_vecThread[i])
			{
				WaitForSingleObject(m_vecThread[i], 1000);
				DWORD dwExitCode;
				GetExitCodeThread(m_vecThread[i], &dwExitCode);
				if (dwExitCode == STILL_ACTIVE)
				{
					TerminateThread(m_vecThread[i], dwExitCode);
				}

				if (m_vecThread[i])
				{
					CloseHandle(m_vecThread[i]);
					m_vecThread[i] = NULL;
				}
			}
		}
	}
	m_vecThread.clear();
}
