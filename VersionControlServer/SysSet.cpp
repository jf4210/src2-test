#include "SysSet.h"
#include "global.h"

CSysSet CSysSet::m_SysSet;
CSysSet::CSysSet(void)
{
}

CSysSet::~CSysSet(void)
{
}

CSysSet& CSysSet::GetRef()
{
	return m_SysSet;
}

bool CSysSet::Load(std::string& strConfPath)
{
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strConfPath));
	m_sLocalIP			= pConf->getString("Net.LocalIP", "127.0.0.1");
	m_nCmdPort			= pConf->getInt("Net.CmdPort", 19981);

	m_strFileDir = pConf->getString("Sys.VersionFileDir");
	return true;
}
