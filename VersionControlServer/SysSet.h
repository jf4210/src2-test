#pragma once
#include <string>

using namespace std;
class CSysSet
{
public:
	~CSysSet(void);
	static CSysSet&		GetRef();
private:
	CSysSet(void);
	static CSysSet m_SysSet;
public:
	bool Load(std::string& strConfPath);
	// 用于通讯的本地IP
	string m_sLocalIP;
	// 命令通道监听端口
	int m_nCmdPort;

	string m_strCurrentDir;
	string m_strFileDir;		//存放更新文件的目录
};

#define SysSet CSysSet::GetRef()
