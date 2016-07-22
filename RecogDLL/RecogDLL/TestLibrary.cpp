
#include "TestPlugin.h"
#include "PluginA.h"
#include "Poco/ClassLibrary.h"
#include <iostream>










POCO_BEGIN_MANIFEST(CTestPlugin)
POCO_EXPORT_CLASS(CPluginA)
POCO_END_MANIFEST


void pocoInitializeLibrary()
{
	std::cout << "TestLibrary initializing" << std::endl;
}


void pocoUninitializeLibrary()
{
	std::cout << "TestLibrary uninitialzing" << std::endl;
}




