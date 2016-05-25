#pragma once
#include "global.h"


class CTcpClient : public Poco::Runnable
{
public:
	CTcpClient(std::string strIp, int nPort);
	~CTcpClient();

	virtual void run();
	bool	receiveData();			//接收数据
private:
	std::string _strIP;
	int		_nPort;
	bool	_bConnect;
	Poco::Net::StreamSocket m_ss;
	char	m_szRecvBuff[1024 * 10];
};

