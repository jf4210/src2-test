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
	string m_sPublicIP;
	// 命令通道监听端口
	int m_nPaperUpLoadPort;
	// 指令通道监听端口
	int m_nCmdPort;
	int m_nVerServerPort;	//版本控制服务器的地址信息

	int	m_nServerMode;		//服务类型，0--易考乐学，1--天喻(登录和获取考试通过天喻接口)
	int m_nUseThirdPlatform;	//是否使用第3方登录平台

	int m_nHandleCmdThreads;	//处理接收到的网络命令线程
	int m_nDecompressThreads;
	int m_nSendHttpThreads;
	int m_nHttpTimeOut;		//发送文件间隔时间
	int m_nSendTimes;		//发送失败后重发次数
	int m_nIntervalTime;	//发送失败的间隔时间
	int m_nBackupPapers;	//是否备份试卷袋原始文件
	int	m_nUpPicData;		//是否上传图片信息给Zimg服务器
	int m_nUpLoadOmrData;	//在上传完ZIMG信息后，是否上传OMR
	int	m_nUpLoadZKZH;		//上传ZKZH
	int	m_nUpLoadElectOmr;	//上传选做题

	int	m_nModifyPicOnFail;	//在上传图片失败时，修改图像内容再提交(在图片上修改3个点的像素值)
	int m_nQuYuVersion;		//区域版本，获取考试列表时不设置用户ID

	//心跳监测间隔，单位秒
	int m_nHeartPacketTime;			//维持session存活的时间
	string m_strCurrentDir;			//utf8
	string m_strUpLoadPath;			//gb2312	试卷袋处理路径
	string m_strDecompressPath;		//utf8		试卷袋解压路径
	string m_strPapersBackupPath;	//gb2312	试卷袋备份路径
	string m_strModelSavePath;		//gb2312	模板保存路径
	string m_strRecvFilePath;		//gb2312	接收文件的临时文件夹路径
	string m_strErrorPkg;			//gb2312	错误试卷包，指无法继续处理的包，如图片有重复等
	string m_strReSendPkg;			//gb2312	发送OMR、准考证号、选做题等信息失败时，将需要发送的数据记录到此文件夹下并写文本
	string m_strNewGuardProcessPath;	//gb2312	新的守护进程的存储目录
	string m_strUpLoadHttpUri;
	string m_strBackUri;
	string m_strEncryptPwd;		//扫描端进行文件加密解密的密码
	string m_strSessionName;	//提交数据给后端时的cookie字段的名称，默认ezs
	string m_strVerServerIP;	//版本控制服务器的地址信息
	string m_strPicWwwNetAddr;	//图片的外网访问地址
	string m_strFileAddrs;		//文件上传地址信息
	string m_str3PlatformUrl;	//第3方登录平台url
	string m_strLoginYklxPwd;	//登录第3方平台成功后，再登录易考乐学后端平台的默认密码
};

#define SysSet CSysSet::GetRef()
