#pragma once
#include "global.h"

class CTcpClient : public Poco::Runnable
{
public:
	CTcpClient(std::string strIp, int nPort);
	~CTcpClient();

	virtual void run();

private:
	std::string _strIP;
	int		_nPort;
};

