#pragma once
#include "global.h"

#define MSG_CMD_DL_MODEL_OK	(WM_USER + 200)		//模板下载完成消息
#define MSG_CMD_GET_BMK_OK	(WM_USER + 201)		//报名库下载完成
#define MSG_CMD_CONNECTION_FAIL	(WM_USER + 202)	//连接异常，接收数据异常

class CTcpClient : public Poco::Runnable
{
public:
	CTcpClient(std::string strIp, int nPort);
	~CTcpClient();

	virtual void run();
	bool	connectServer();
	bool	receiveData();			//接收数据

	void	HandleCmd();
	void	HandleTask(pTCP_TASK pTask);
	void	SetMainWnd(void* p);
private:
	std::string _strIP;
	int		_nPort;
	bool	_bConnect;
	int		_nRecvLen;
	int		_nWantLen;
	Poco::Net::StreamSocket m_ss;
	char	m_szRecvBuff[1024];
	char*	m_pRecvBuff;
	char*	m_pSendBuff;
	int		_nRecvBuffSize;
	void*	_pMainDlg;
};

