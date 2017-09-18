#pragma once
#include "afxwin.h"


class AdvanceParam;
class CBaseTabDlg : public CDialog
{
public:
	CBaseTabDlg(UINT nIDTemplate, CWnd* pParent = NULL) {};
	virtual ~CBaseTabDlg() {};

	virtual void	InitData(AdvanceParam& stParam) = 0;
	virtual BOOL	SaveParamData(AdvanceParam& stParam) = 0;
};

