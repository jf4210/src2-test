#pragma once
#include <vector>

class CThreadMgr
{
public:
	CThreadMgr();
	~CThreadMgr();

public:
	void	InitThread(int nTheads);
	void	StopThread();

private:
	int		m_nThreads;
	std::vector<HANDLE>		m_vecThread;
};

