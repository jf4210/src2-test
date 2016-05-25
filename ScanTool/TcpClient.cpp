#include "stdafx.h"
#include "TcpClient.h"
#include "Net_Cmd_Protocol.h"


CTcpClient::CTcpClient(std::string strIp, int nPort)
: _strIP(strIp), _nPort(nPort), _bConnect(false)
{
}


CTcpClient::~CTcpClient()
{
}

void CTcpClient::run()
{
	Poco::Net::SocketAddress sa(_strIP, _nPort);
	while (!g_nExitFlag)
	{
		if (!_bConnect)
		{
			try
			{
				m_ss.connect(sa);					//重新连接服务器
				m_ss.setBlocking(false);
				m_ss.setNoDelay(true);
				_bConnect = true;
			}
			catch (Poco::Exception& exc)
			{
				std::cout << "RTS --> " << exc.displayText() << std::endl;
			}
		}

		if (receiveData())
		{

		}

		pTCP_TASK pTask = NULL;
		if (g_lTcpTask.size() > 0)
		{
			g_fmTcpTaskLock.lock();
			pTask = g_lTcpTask.front();
			g_lTcpTask.pop_front();
			g_fmTcpTaskLock.unlock();
		}
		if (pTask == NULL)
		{
			Sleep(1000);
			continue;
		}




		SAFE_RELEASE(pTask);
	}
}

bool CTcpClient::receiveData()
{
	if (!_bConnect)
		return false;

	memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
	int nCount = 0;
	int nBaseLen = HEAD_SIZE;
	int nRecvLen = 0;
	int nWantLen = 0;
	while (!g_nExitFlag)
	{
		int nLen;
		if (nRecvLen < nBaseLen)
		{
			nWantLen = nBaseLen - nRecvLen;
			nLen = m_ss.receiveBytes(m_szRecvBuff + nRecvLen, nWantLen);
			if (nLen > 0)
			{
				nRecvLen += nLen;
				if (nRecvLen == nBaseLen)
				{
					if (nCount == 0)
					{
						ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
						nBaseLen += pstHead->uPackSize;
					}
					nCount++;
				}
			}
			else if (nLen == 0)
			{
				TRACE("the peer has closed.\n");
				return 0;
			}
			else
				Poco::Thread::sleep(1);
		}
		else
			break;
	}

	return true;
}
