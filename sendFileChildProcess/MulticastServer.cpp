//#include "stdafx.h"
#include "MulticastServer.h"
#include "Net_Cmd_Protocol.h"


MulticastServer::MulticastServer(std::string& strIp, int nPort) :
_strIP(strIp),
_nPort(nPort),
_group(strIp, nPort),
_if(findInterface()),
_thread("MulticastEchoServer"),
_stop(false)
{
	_socket.bind(Poco::Net::SocketAddress(Poco::Net::IPAddress(), _group.port()), true);
	_socket.joinGroup(_group.host(), _if);
	std::string strInfo = Poco::format("组播地址:%s, 网卡名:%s, IP:%s\n", _group.host().toString(), _if.displayName(), _if.address(1).toString());
	std::cout << strInfo << std::endl;
	_thread.start(*this);
	_ready.wait();
}


MulticastServer::~MulticastServer()
{
	_stop = true;
	_thread.join();
	_socket.leaveGroup(_group.host(), _if);
}

Poco::UInt16 MulticastServer::port() const
{
	return _socket.address().port();
}

void MulticastServer::run()
{
	_ready.set();
	Poco::Timespan span(250000);
	while (!_stop)
	{
		if (_socket.poll(span, Poco::Net::Socket::SELECT_READ))
		{
			try
			{
				char buffer[1024] = { 0 };
				recvMyData(buffer);
				HandleCmd(buffer);
// 				Poco::Net::SocketAddress sender;
// 				int n = _socket.receiveFrom(buffer, sizeof(buffer), sender);
// 				_socket.sendTo(buffer, n, sender);
			}
			catch (Poco::Exception& exc)
			{
				std::string strLog = "MulticastServer异常 ==> " + exc.displayText();
				_pLogger_->error(strLog);
			}
		}
	}
}


const Poco::Net::SocketAddress& MulticastServer::group() const
{
	return _group;
}


const Poco::Net::NetworkInterface& MulticastServer::interfc() const
{
	return _if;
}


Poco::Net::NetworkInterface MulticastServer::findInterface()
{
	Poco::Net::NetworkInterface::Map m = Poco::Net::NetworkInterface::map();
	for (Poco::Net::NetworkInterface::Map::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		if (it->second.supportsIPv4() &&
			it->second.firstAddress(Poco::Net::IPAddress::IPv4).isUnicast() &&
			!it->second.isLoopback() &&
			!it->second.isPointToPoint())
		{
			return it->second;
		}
	}
	return Poco::Net::NetworkInterface();
}

bool MulticastServer::recvMyData(char* szBuf)
{
	int nCount = 0;
	int nRecvLen = 0;
	int nWantLen = 0;
	int nBaseLen = HEAD_SIZE;
	bool bResult = false;

	Poco::Net::SocketAddress sender;
	try
	{
		while (true)
		{
			if (nRecvLen < nBaseLen)
			{
				nWantLen = nBaseLen - nRecvLen;
				int nLen = _socket.receiveBytes(szBuf + nRecvLen, nWantLen);
//				int nLen = _socket.receiveFrom(szBuf + nRecvLen, nWantLen, sender);
				if (nLen > 0)
				{
					nRecvLen += nLen;
					if (nRecvLen == nBaseLen)
					{
						if (nCount == 0)
						{
							ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)szBuf;
							nBaseLen += pstHead->uPackSize;
							std::string strLog = "接收到组播数据";
							_pLogger_->information(strLog);
						}
						nCount++;
					}
				}
				else if (nLen == 0)
				{
//					TRACE("the peer has closed.\n");
					std::string strLog = "the peer has closed";
					_pLogger_->information(strLog);
					return bResult;
				}
				else
					Poco::Thread::sleep(1);
			}
			else
				break;
		}
		bResult = true;
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "接收数据异常 ==> " + exc.displayText();
		_pLogger_->information(strLog);
//		TRACE(strLog.c_str());
	}

	return bResult;
}

bool MulticastServer::HandleCmd(char* szBuf)
{
	bool bResult = false;
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)szBuf;	//m_pRecvBuff
	switch (pstHead->usCmd)
	{
		case MULTICAST_START:
			{
				_peStartMulticast_->set();
				bResult = true;
				std::string strLog = "组播命令: 传输开始";
				_pLogger_->information(strLog);
				std::cout << strLog << std::endl;
			}
			break;
		case MULTICAST_INIT_THREAD:
			{
				_eInitMulticast_.set();
				_nThreads_ = pstHead->usResult;
				bResult = true;
				std::string strLog = "组播命令: 初始化子连接线程";
				_pLogger_->information(strLog);
				std::cout << strLog << std::endl;
			}
			break;
	}

	return bResult;
}
