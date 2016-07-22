#include "PluginA.h"


CPluginA::CPluginA()
{
}


CPluginA::~CPluginA()
{
}

void CPluginA::test()
{
	MessageBox(NULL, L"test CPluginA", L"test CPluginA", MB_YESNO);
}
