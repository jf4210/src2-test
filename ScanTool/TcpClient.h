#pragma once
#include "global.h"


class CTcpClient : public Poco::Runnable
{
public:
	CTcpClient(std::string strIp, int nPort);
	~CTcpClient();

	virtual void run();
	bool	connectServer();
	bool	receiveData();			//接收数据

	void	HandleCmd();
	void HandleTask(pTCP_TASK pTask);

public:
	Poco::Event	eExit;
private:
	std::string _strIP;
	int		_nPort;
	bool	_bConnect;
	int		_nRecvLen;
	int		_nWantLen;
	Poco::Net::StreamSocket m_ss;
	char	m_szRecvBuff[1024 * 10];
};

