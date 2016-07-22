#pragma once
#include "Poco/Foundation.h"
#include <opencv2\opencv.hpp>

class CTestPlugin
{
public:
	CTestPlugin();
	virtual ~CTestPlugin();

	virtual void test() = 0;
};

