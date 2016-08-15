#pragma once
#include "global.h"


class MulticastServer : public Poco::Runnable
{
public:
	MulticastServer(std::string& strIp, int nPort);
	~MulticastServer();

	Poco::UInt16 port() const;
	/// Returns the port the echo server is
	/// listening on.

	void run();
	/// Does the work.

	bool recvMyData(char* szBuf);
	bool HandleCmd(char* szBuf);

	const Poco::Net::SocketAddress& group() const;
	/// Returns the group address where the server listens.

	const Poco::Net::NetworkInterface& interfc() const;
	/// Returns the network interface for multicasting.

protected:
	static Poco::Net::NetworkInterface findInterface();
	/// Finds an appropriate network interface for
	/// multicasting.

private:
	Poco::Net::MulticastSocket  _socket;
	Poco::Net::SocketAddress    _group;
	Poco::Net::NetworkInterface _if;
	Poco::Thread _thread;
	Poco::Event  _ready;
	std::string _strIP;
	int			_nPort;

	bool         _stop;
};

