// ScanRecordDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ExamBmkRecordDlg.h"
#include "afxdialogex.h"


// CScanRecordDlg �Ի���

IMPLEMENT_DYNAMIC(CExamBmkRecordDlg, CDialog)

CExamBmkRecordDlg::CExamBmkRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExamBmkRecordDlg::IDD, pParent)
{

}

CExamBmkRecordDlg::~CExamBmkRecordDlg()
{
}

void CExamBmkRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Bmk, m_lcBmk);
	DDX_Control(pDX, IDC_BTN_ExportScan, m_bmpBtnExport);
	DDX_Control(pDX, IDC_COMBO_Bmk_Subject, m_comboSubject);
	DDX_Control(pDX, IDC_COMBO_Bmk_ScanStatus, m_comboScanStatus);
}


BOOL CExamBmkRecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	ResetBmkList();
	return TRUE;
}

BOOL CExamBmkRecordDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CExamBmkRecordDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO_Bmk_Subject, &CExamBmkRecordDlg::OnCbnSelchangeComboBmkSubject)
	ON_CBN_SELCHANGE(IDC_COMBO_Bmk_ScanStatus, &CExamBmkRecordDlg::OnCbnSelchangeComboBmkScanstatus)
END_MESSAGE_MAP()


// CScanRecordDlg ��Ϣ�������

void CExamBmkRecordDlg::InitUI()
{
//	SetFontSize(m_nStatusSize);
//	UpDateInfo();

	m_comboScanStatus.AdjustDroppedWidth();
	m_comboSubject.AdjustDroppedWidth();

	m_bmpBtnExport.SetStateBitmap(IDB_RecordDlg_Btn_Hover, 0, IDB_RecordDlg_Btn);
	m_bmpBtnExport.SetWindowText(_T("����δɨ����"));

	CBitmap bmp;
	bmp.LoadBitmap(IDB_Scrollbar);
	BITMAP bm;
	bmp.GetBitmap(&bm);
	m_bitmap_scrollbar = (HBITMAP)bmp.Detach();
//	SkinWndScroll(&m_lcBmk, m_bitmap_scrollbar);
	m_lcBmk.SetExtendedStyle(m_lcBmk.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcBmk.InsertColumn(0, _T("˳��"), LVCFMT_CENTER, 40);
	m_lcBmk.InsertColumn(1, _T("����"), LVCFMT_CENTER, 110);
	m_lcBmk.InsertColumn(2, _T("����"), LVCFMT_CENTER, 110);
	m_lcBmk.InsertColumn(3, _T("ɨ��״̬"), LVCFMT_CENTER, 100);
	InitCtrlPosition();
}

void CExamBmkRecordDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 5;	//�ϱߵļ��
	const int nBottomGap = 35;	//�±ߵļ��
	const int nLeftGap = 20;		//��ߵĿհ׼��
	const int nRightGap = 20;	//�ұߵĿհ׼��
	int nGap = 5;

	int nCurrLeft = nLeftGap;
	int nCurrTop = nTopGap;
	int nStaticH = 30;

	nCurrLeft = nLeftGap;
	
	if (GetDlgItem(IDC_STATIC_1)->GetSafeHwnd())
	{
		int nW = 50;
		GetDlgItem(IDC_STATIC_1)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (m_comboSubject.GetSafeHwnd())
	{
		int nW = 100;
		m_comboSubject.MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap* 3);
	}
	if (GetDlgItem(IDC_STATIC_2)->GetSafeHwnd())
	{
		int nW = 50;
		GetDlgItem(IDC_STATIC_2)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (m_comboSubject.GetSafeHwnd())
	{
		int nW = 60;
		m_comboSubject.MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrTop += (nStaticH + nGap);
	}

	nCurrLeft = nLeftGap;
	if (GetDlgItem(IDC_LIST_Bmk)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 1;
		int nH = cy - nTopGap - nStaticH - nGap - nBottomGap;
		int nXTmp = (cx - nLeftGap - nRightGap) * 0;
		GetDlgItem(IDC_LIST_Bmk)->MoveWindow(nCurrLeft + nXTmp, nCurrTop, nW, nH);
		int nColumns = m_lcBmk.GetColumns() - 3;	//��ȥ���ɶ��ı�ͷ��ʣ�µı�ͷ(��ÿ����Ŀ��ͷ)ƽ��ʣ�µĿ��
		if (nColumns > 0)
		{
			int nColunmW = (nW - 310) / nColumns;
			if (nColunmW < 100)	nColunmW = 100;
			for (int i = 0; i < nColumns; i++)
			{
				m_lcBmk.SetColumnWidth(3 + i, nColunmW);
			}
		}
	}

	if (GetDlgItem(IDC_BTN_ExportScan)->GetSafeHwnd())
	{
		int nW = 100;
		int nH = 25;
		nCurrLeft = cx / 2 - nW / 2;
		nCurrTop = cy - nBottomGap + nGap;
		GetDlgItem(IDC_BTN_ExportScan)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}
}

void CExamBmkRecordDlg::SetFontSize(int nSize)
{
// 	m_fontStatus.DeleteObject();
// 	m_fontStatus.CreateFont(nSize, 0, 0, 0,
// 							FW_BOLD, FALSE, FALSE, 0,
// 							DEFAULT_CHARSET,
// 							OUT_DEFAULT_PRECIS,
// 							CLIP_DEFAULT_PRECIS,
// 							DEFAULT_QUALITY,
// 							DEFAULT_PITCH | FF_SWISS,
// 							_T("Arial"));
// 	GetDlgItem(IDC_STATIC_1)->SetFont(&m_fontStatus);
// 	GetDlgItem(IDC_STATIC_2)->SetFont(&m_fontStatus);
// 	GetDlgItem(IDC_STATIC_3)->SetFont(&m_fontStatus);
// 	GetDlgItem(IDC_STATIC_GMK_Count)->SetFont(&m_fontStatus);
// 	GetDlgItem(IDC_STATIC_Scan_Count)->SetFont(&m_fontStatus);
// 	GetDlgItem(IDC_STATIC_UnScan_Count)->SetFont(&m_fontStatus);
}

HBRUSH CExamBmkRecordDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	return hbr;
}

BOOL CExamBmkRecordDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CExamBmkRecordDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CExamBmkRecordDlg::ResetBmkList()
{
	if (!_pCurrExam_) return;

	m_lcBmk.DeleteAllItems();
	//ɾ����ͷ��Ŀ���֣����²����Ŀ
	while (m_lcBmk.DeleteColumn(3));

	m_comboSubject.ResetContent();
	m_comboScanStatus.ResetContent();
	m_comboSubject.AddString(_T("ȫ��"));
	m_comboSubject.SetItemData(0, 0);
	m_comboScanStatus.AddString(_T("ȫ��"));
	m_comboScanStatus.SetItemData(0, 0);
	m_comboScanStatus.AddString(_T("��ɨ"));
	m_comboScanStatus.SetItemData(1, 1);
	m_comboScanStatus.AddString(_T("δɨ"));
	m_comboScanStatus.SetItemData(2, 2);

	USES_CONVERSION;
	int nSubjects = _pCurrExam_->lSubjects.size();	//��Ŀ����
	int i = 0;
	for (auto objSubject : _pCurrExam_->lSubjects)
	{
		//��Ŀ�б�ӱ�����ĵ�3�в���
		m_lcBmk.InsertColumn(i + 3, A2T(objSubject->strSubjName.c_str()), LVCFMT_CENTER, 100);
		m_comboSubject.AddString(A2T(objSubject->strSubjName.c_str()));
		m_comboSubject.SetItemData(m_comboSubject.GetCount(), objSubject->nSubjID);
		++i;
	}

	InitCtrlPosition();

	EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
	if (itFindExam == g_mapBmkMgr.end())
		return;

	//insert data
	for (auto objExamStudent : itFindExam->second)
	{
		int nCount = m_lcBmk.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lcBmk.InsertItem(nCount, NULL);

		m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(objExamStudent.strZkzh.c_str()));
		m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(objExamStudent.strName.c_str()));

		int nScaned = 0;
		int unScaned = 0;
		for (auto examSubject : objExamStudent.lSubjectScanStatus)
		{
			int j = 0;
			for (auto tmpSubject : _pCurrExam_->lSubjects)
			{
				if (examSubject.nSubjectID == tmpSubject->nSubjID)
				{
					std::string strScanStatus;
					if (examSubject.nScaned)
					{
						strScanStatus = "OK";
						nScaned++;
					}
					else
					{
						strScanStatus = "δɨ";
						unScaned++;
					}
					m_lcBmk.SetItemText(nCount, 3 + j, (LPCTSTR)A2T(strScanStatus.c_str()));
					break;
				}
				++j;
			}
		}
	}
}

void CExamBmkRecordDlg::GetBmkSearchResult()
{
	if (!_pCurrExam_) return;

	m_lcBmk.DeleteAllItems();
	//ɾ����ͷ��Ŀ���֣����²����Ŀ
	while (m_lcBmk.DeleteColumn(3));

	USES_CONVERSION;
	CString strCurSub = _T("");
	CString strCurScanStatus = _T("");
	m_comboSubject.GetLBText(m_comboSubject.GetCurSel(), strCurSub);
	m_comboScanStatus.GetLBText(m_comboScanStatus.GetCurSel(), strCurScanStatus);
	std::string strCurrSubject = T2A(strCurSub);
	std::string strCurrScanStatus = T2A(strCurScanStatus);

	int nCurrSubID = m_comboSubject.GetItemData(m_comboSubject.GetCurSel());
	int nCurrScanStatus = m_comboScanStatus.GetItemData(m_comboScanStatus.GetCurSel());
	
	int nSubjects = _pCurrExam_->lSubjects.size();	//��Ŀ����
	int i = 0;
	for (auto objSubject : _pCurrExam_->lSubjects)
	{
		//��Ŀ�б�ӱ�����ĵ�3�в���
		if (strCurrSubject == "ȫ��")
		{
			m_lcBmk.InsertColumn(i + 3, A2T(objSubject->strSubjName.c_str()), LVCFMT_CENTER, 100);
			++i;
		}
		else
		{
			if (objSubject->strSubjName == strCurrSubject)
			{
				m_lcBmk.InsertColumn(3, A2T(objSubject->strSubjName.c_str()), LVCFMT_CENTER, 100);
				break;
			}
		}
	}

	InitCtrlPosition();

	EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
	if (itFindExam == g_mapBmkMgr.end())
		return;

	//insert data
	for (auto objExamStudent : itFindExam->second)
	{
		for (auto examSubject : objExamStudent.lSubjectScanStatus)
		{
			if (strCurrSubject == "ȫ��")
			{
				int j = 0;
				for (auto tmpSubject : _pCurrExam_->lSubjects)
				{
					if (examSubject.nSubjectID == tmpSubject->nSubjID)
					{
						//*****************************************
						//�����ĿΪȫ���Ļ���ɨ��״̬�Ĺ������ã���Ϊ����ÿ������������ĳ����Ŀûɨ��ĳ����Ŀ��ɨ�ˣ�������������£����������Ҫ�������б���
						//����ĳ���������п�Ŀ��û��ɨ��
						//*****************************************
						std::string strScanStatus;
						if (examSubject.nScaned)
						{
							strScanStatus = "OK";
						}
						else
						{
							strScanStatus = "δɨ";
						}
						int nCount = m_lcBmk.GetItemCount();
						char szCount[10] = { 0 };
						sprintf_s(szCount, "%d", nCount + 1);
						m_lcBmk.InsertItem(nCount, NULL);

						m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
						m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(objExamStudent.strZkzh.c_str()));
						m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(objExamStudent.strName.c_str()));

						m_lcBmk.SetItemText(nCount, 3 + j, (LPCTSTR)A2T(strScanStatus.c_str()));
						break;
					}
					++j;
				}
			}
			else
			{
				if (examSubject.nSubjectID == nCurrSubID)
				{
					if (strCurrScanStatus == "ȫ��")
					{
						std::string strScanStatus;
						if (examSubject.nScaned)
						{
							strScanStatus = "OK";
						}
						else
						{
							strScanStatus = "δɨ";
						}
						int nCount = m_lcBmk.GetItemCount();
						char szCount[10] = { 0 };
						sprintf_s(szCount, "%d", nCount + 1);
						m_lcBmk.InsertItem(nCount, NULL);

						m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
						m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(objExamStudent.strZkzh.c_str()));
						m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(objExamStudent.strName.c_str()));

						m_lcBmk.SetItemText(nCount, 3, (LPCTSTR)A2T(strScanStatus.c_str()));
						break;
					}
					else
					{
						if (nCurrScanStatus == 1)	//��ɨ
						{
							std::string strScanStatus;
							if (examSubject.nScaned)
							{
								strScanStatus = "OK";
								int nCount = m_lcBmk.GetItemCount();
								char szCount[10] = { 0 };
								sprintf_s(szCount, "%d", nCount + 1);
								m_lcBmk.InsertItem(nCount, NULL);

								m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
								m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(objExamStudent.strZkzh.c_str()));
								m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(objExamStudent.strName.c_str()));

								m_lcBmk.SetItemText(nCount, 3, (LPCTSTR)A2T(strScanStatus.c_str()));
								break;
							}
						}
						else  //δɨ
						{
							std::string strScanStatus;
							if (examSubject.nScaned == 0)
							{
								strScanStatus = "OK";
								int nCount = m_lcBmk.GetItemCount();
								char szCount[10] = { 0 };
								sprintf_s(szCount, "%d", nCount + 1);
								m_lcBmk.InsertItem(nCount, NULL);

								m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
								m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(objExamStudent.strZkzh.c_str()));
								m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(objExamStudent.strName.c_str()));

								m_lcBmk.SetItemText(nCount, 3, (LPCTSTR)A2T(strScanStatus.c_str()));
								break;
							}
						}
					}
				}
			}
		}
	}
}

void CExamBmkRecordDlg::OnCbnSelchangeComboBmkSubject()
{
	if (m_comboSubject.GetCurSel() < 0)
		return;
	GetBmkSearchResult();
}


void CExamBmkRecordDlg::OnCbnSelchangeComboBmkScanstatus()
{
	if (m_comboScanStatus.GetCurSel() < 0)
		return;
	GetBmkSearchResult();
}
