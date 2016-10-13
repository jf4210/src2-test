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
	int m_nBackupPapers;	//�Ƿ񱸷��Ծ��ԭʼ�ļ�

	//�������������λ��
	int m_nHeartPacketTime;
	string m_strCurrentDir;			//utf8
	string m_strUpLoadPath;			//gb2312	�Ծ������·��
	string m_strDecompressPath;		//utf8		�Ծ����ѹ·��
	string m_strPapersBackupPath;	//gb2312	�Ծ������·��
	string m_strModelSavePath;		//gb2312	ģ�屣��·��
	string m_strRecvFilePath;		//gb2312	�����ļ�����ʱ�ļ���·��
	string m_strUpLoadHttpUri;
	string m_strBackUri;
	string m_strEncryptPwd;	//ɨ��˽����ļ����ܽ��ܵ�����
	string m_strSessionName;	//�ύ���ݸ����ʱ��cookie�ֶε����ƣ�Ĭ��ezs
};

#define SysSet CSysSet::GetRef()
