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
	// ����ͨѶ�ı���IP
	string m_sLocalIP;
	// ����ͨ�������˿�
	int m_nCmdPort;

	string m_strCurrentDir;
	string m_strFileDir;		//��Ÿ����ļ���Ŀ¼
};

#define SysSet CSysSet::GetRef()
