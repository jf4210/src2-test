#pragma once
#include "TestPlugin.h"
#include <iostream>

class CPluginA : public CTestPlugin
{
public:
	CPluginA();
	~CPluginA();

	void test();
};

