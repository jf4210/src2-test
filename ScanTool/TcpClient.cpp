#include "stdafx.h"
#include "TcpClient.h"


CTcpClient::CTcpClient(std::string strIp, int nPort)
: _strIP(strIp), _nPort(nPort)
{
}


CTcpClient::~CTcpClient()
{
}

void CTcpClient::run()
{
	while (!g_nExitFlag)
	{

	}
}
