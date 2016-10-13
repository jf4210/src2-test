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
	int m_nPaperUpLoadPort;
	// 指令通道监听端口
	int m_nCmdPort;

	int m_nDecompressThreads;
	int m_nSendHttpThreads;
	int m_nHttpTimeOut;		//发送文件间隔时间
	int m_nSendTimes;		//发送失败后重发次数
	int m_nIntervalTime;	//发送失败的间隔时间
	int m_nBackupPapers;	//是否备份试卷袋原始文件

	//心跳监测间隔，单位秒
	int m_nHeartPacketTime;
	string m_strCurrentDir;			//utf8
	string m_strUpLoadPath;			//gb2312	试卷袋处理路径
	string m_strDecompressPath;		//utf8		试卷袋解压路径
	string m_strPapersBackupPath;	//gb2312	试卷袋备份路径
	string m_strModelSavePath;		//gb2312	模板保存路径
	string m_strRecvFilePath;		//gb2312	接收文件的临时文件夹路径
	string m_strUpLoadHttpUri;
	string m_strBackUri;
	string m_strEncryptPwd;	//扫描端进行文件加密解密的密码
	string m_strSessionName;	//提交数据给后端时的cookie字段的名称，默认ezs
};

#define SysSet CSysSet::GetRef()
