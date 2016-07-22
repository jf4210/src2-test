
#include "Poco/Foundation.h"
#include "Poco/SharedLibrary.h"
#include "Poco/Exception.h"

#include "Poco/Manifest.h"
#include "Poco/ClassLoader.h"
#include "TestPlugin.h"


using Poco::ClassLoader;
using Poco::Manifest;
using Poco::SharedLibrary;
using Poco::AbstractMetaObject;
using Poco::NotFoundException;
using Poco::InvalidAccessException;

void main()
{
	std::string path = "RecogDLL";
	path.append(SharedLibrary::suffix());

	ClassLoader<CTestPlugin> cl;
	cl.loadLibrary(path);

	CTestPlugin* p = NULL;
	
	int n = cl.manifestFor(path).size();

	CTestPlugin* pPluginA = cl.classFor("CPluginA").create();
	pPluginA->test();
//	delete pPluginA;

	try
	{
		const AbstractMetaObject<CTestPlugin>& meta2 = cl.classFor("PluginA");
		CTestPlugin* pPlugin = meta2.create();
		meta2.autoDelete(pPlugin);

		pPlugin->test();
	}
	catch (NotFoundException& exc)
	{
		std::cout << "err" << std::endl;
	}
	

	cl.unloadLibrary(path);
	system("pause");
}


