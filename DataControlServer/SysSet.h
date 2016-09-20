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
	int m_nPaperUpLoadPort;
	// ָ��ͨ�������˿�
	int m_nCmdPort;

	int m_nDecompressThreads;
	int m_nSendHttpThreads;
	int m_nHttpTimeOut;		//�����ļ����ʱ��
	int m_nSendTimes;		//����ʧ�ܺ��ط�����
	int m_nIntervalTime;	//����ʧ�ܵļ��ʱ��

	//�������������λ��
	int m_nHeartPacketTime;
	string m_strCurrentDir;			//utf8
	string m_strUpLoadPath;
	string m_strDecompressPath;		//utf8
	string m_strModelSavePath;
	string m_strUpLoadHttpUri;
	string m_strBackUri;
	string m_strEncryptPwd;	//ɨ��˽����ļ����ܽ��ܵ�����
	string m_strSessionName;	//�ύ���ݸ����ʱ��cookie�ֶε����ƣ�Ĭ��ezs
};

#define SysSet CSysSet::GetRef()
