#pragma once
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <io.h>
#include "afxmt.h"



BOOL CheckProcessExist(CString &str, int& nProcessID);

bool EnableDebugPrivilege();
bool UpPrivilege();
BOOL KillProcess(CString& str);
