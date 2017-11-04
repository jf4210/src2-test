// ScanRecordDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ExamBmkRecordDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"

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
	DDX_Control(pDX, IDC_BTN_ExamBmk_ExportScan, m_bmpBtnExport);
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
	ON_BN_CLICKED(IDC_BTN_ExamBmk_ExportScan, &CExamBmkRecordDlg::OnBnClickedBtnExambmkExportscan)
END_MESSAGE_MAP()


// CScanRecordDlg ��Ϣ�������

void CExamBmkRecordDlg::InitUI()
{
//	SetFontSize(m_nStatusSize);
//	UpDateInfo();

	m_comboScanStatus.AdjustDroppedWidth();
	m_comboSubject.AdjustDroppedWidth();
//	m_comboSubject.SetMode(CComboBoxExt::MODE_STANDARD);
//	m_comboSubject.SetAlertColorBkg(RGB(255, 255, 0));
// 	m_comboSubject.AlertText();
// 	m_comboSubject.AlertBkg();

	m_bmpBtnExport.SetStateBitmap(IDB_RecordDlg_Btn, 0, IDB_RecordDlg_Btn_Hover);
	m_bmpBtnExport.SetWindowText(_T("������ǰ����"));

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
		m_comboSubject.MoveWindow(nCurrLeft, nCurrTop + 5, nW, nStaticH);
		nCurrLeft += (nW + nGap* 3);
	}
	if (GetDlgItem(IDC_STATIC_2)->GetSafeHwnd())
	{
		int nW = 60;
		GetDlgItem(IDC_STATIC_2)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (m_comboScanStatus.GetSafeHwnd())
	{
		int nW = 60;
		m_comboScanStatus.MoveWindow(nCurrLeft, nCurrTop + 5, nW, nStaticH);
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
			int nColunmW = (nW - 330) / nColumns;
			if (nColunmW < 90)	nColunmW = 90;
			for (int i = 0; i < nColumns; i++)
			{
				m_lcBmk.SetColumnWidth(3 + i, nColunmW);
			}
		}
	}

	if (GetDlgItem(IDC_BTN_ExamBmk_ExportScan)->GetSafeHwnd())
	{
		int nW = 100;
		int nH = 25;
		nCurrLeft = cx / 2 - nW / 2;
		nCurrTop = cy - nBottomGap + nGap;
		GetDlgItem(IDC_BTN_ExamBmk_ExportScan)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}
	Invalidate();
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
	if (CurID == IDC_STATIC_1 || CurID == IDC_STATIC_2)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

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
		m_comboSubject.SetItemData(m_comboSubject.GetCount() - 1, objSubject->nSubjID);
		++i;
	}
	m_comboSubject.SetCurSel(0);
	m_comboScanStatus.SetCurSel(0);
	InitCtrlPosition();

	EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
	if (itFindExam == g_mapBmkMgr.end())
		return;

	//insert data
	GetBmkSearchResult();
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
		bool bInsertItem = false;	//�Ƿ��Ѿ������б���
		for (auto examSubject : objExamStudent.lSubjectScanStatus)
		{
			if (strCurrSubject == "ȫ��")
			{
				int j = 0;
				for (auto tmpSubject : _pCurrExam_->lSubjects)
				{
					if (examSubject.nSubjectID == tmpSubject->nSubjID)
					{
						if (strCurrScanStatus == "ȫ��")
						{
							std::string strScanStatus;
							if (examSubject.nScaned)
							{
								strScanStatus = "��ɨ";
							}
							else
							{
								strScanStatus = "δɨ";
							}
							int nCount = m_lcBmk.GetItemCount();
							if (!bInsertItem)
							{
								if (nCount >= 2000) break;		//ֻ��ʾǰ2000������

								char szCount[10] = { 0 };
								sprintf_s(szCount, "%d", nCount + 1);
								m_lcBmk.InsertItem(nCount, NULL);

								m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
								m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(objExamStudent.strZkzh.c_str()));
								m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(objExamStudent.strName.c_str()));
								m_lcBmk.SetItemText(nCount, 3 + j, (LPCTSTR)A2T(strScanStatus.c_str()));
								bInsertItem = true;
							}
							else
								m_lcBmk.SetItemText(nCount - 1, 3 + j, (LPCTSTR)A2T(strScanStatus.c_str()));

							break;
						}
						else
						{
							if (nCurrScanStatus == 1)	//��ɨ
							{
								std::string strScanStatus;
								if (examSubject.nScaned)
								{
									strScanStatus = "��ɨ";
									int nCount = m_lcBmk.GetItemCount();
									if (!bInsertItem)
									{
										if (nCount >= 2000) break;		//ֻ��ʾǰ2000������

										char szCount[10] = { 0 };
										sprintf_s(szCount, "%d", nCount + 1);
										m_lcBmk.InsertItem(nCount, NULL);

										m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
										m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(objExamStudent.strZkzh.c_str()));
										m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(objExamStudent.strName.c_str()));
										m_lcBmk.SetItemText(nCount, 3 + j, (LPCTSTR)A2T(strScanStatus.c_str()));
										bInsertItem = true;
									}
									else
										m_lcBmk.SetItemText(nCount - 1, 3 + j, (LPCTSTR)A2T(strScanStatus.c_str()));

									break;
								}
							}
							else
							{
								std::string strScanStatus;
								if (examSubject.nScaned == 0)
								{
									strScanStatus = "δɨ";
									int nCount = m_lcBmk.GetItemCount();
									if (!bInsertItem)
									{
										if (nCount >= 2000) break;		//ֻ��ʾǰ2000������

										char szCount[10] = { 0 };
										sprintf_s(szCount, "%d", nCount + 1);
										m_lcBmk.InsertItem(nCount, NULL);

										m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
										m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(objExamStudent.strZkzh.c_str()));
										m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(objExamStudent.strName.c_str()));
										m_lcBmk.SetItemText(nCount, 3 + j, (LPCTSTR)A2T(strScanStatus.c_str()));
										bInsertItem = true;
									}
									else
										m_lcBmk.SetItemText(nCount - 1, 3 + j, (LPCTSTR)A2T(strScanStatus.c_str()));

									break;
								}
							}
						}
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
							strScanStatus = "��ɨ";
						}
						else
						{
							strScanStatus = "δɨ";
						}
						int nCount = m_lcBmk.GetItemCount();
						if (nCount >= 2000) break;		//ֻ��ʾǰ2000������

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
								strScanStatus = "��ɨ";
								int nCount = m_lcBmk.GetItemCount();
								if (nCount >= 2000) break;		//ֻ��ʾǰ2000������

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
								strScanStatus = "δɨ";
								int nCount = m_lcBmk.GetItemCount();
								if (nCount >= 2000) break;		//ֻ��ʾǰ2000������

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


void CExamBmkRecordDlg::OnBnClickedBtnExambmkExportscan()
{
	EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
	if (itFindExam == g_mapBmkMgr.end() || itFindExam->second.size() <= 0)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "�޿������ݣ�");
		dlg.DoModal();
		return;
	}

// 	if (!_bGetBmk_)
// 	{
// 		CNewMessageBox	dlg;
// 		dlg.setShowInfo(2, 1, "�޿������ݣ�");
// 		dlg.DoModal();
// 		return;
// 	}

	std::string strData;
	USES_CONVERSION;
	int nColumns = m_lcBmk.GetColumns();	//��ȥ���ɶ��ı�ͷ��ʣ�µı�ͷ(��ÿ����Ŀ��ͷ)ƽ��ʣ�µĿ��
	if (nColumns > 0)
	{
		CHeaderCtrl *pHead = m_lcBmk.GetHeaderCtrl();
		for (int i = 1; i < nColumns; i++)
		{
			HDITEM hdItem = { 0 };
			TCHAR szBuf[129] = { 0 };
			hdItem.mask = HDI_TEXT; //ȡ�ַ�����
			hdItem.cchTextMax = 128; //��������С
			hdItem.pszText = szBuf; //�ַ�������    
			pHead->GetItem(i, &hdItem);
			TRACE("head: %d\t", hdItem.pszText);
			strData.append(T2A(szBuf));
			strData.append("\t\t");
		}
	}
	strData.append("\r\n");

#if 1
	CString strCurSub = _T("");
	CString strCurScanStatus = _T("");
	m_comboSubject.GetLBText(m_comboSubject.GetCurSel(), strCurSub);
	m_comboScanStatus.GetLBText(m_comboScanStatus.GetCurSel(), strCurScanStatus);
	std::string strCurrSubject = T2A(strCurSub);
	std::string strCurrScanStatus = T2A(strCurScanStatus);

	int nCurrSubID = m_comboSubject.GetItemData(m_comboSubject.GetCurSel());
	int nCurrScanStatus = m_comboScanStatus.GetItemData(m_comboScanStatus.GetCurSel());

	EXAMBMK_MAP::iterator itFindExam2 = g_mapBmkMgr.find(_pCurrExam_->nExamID);
	if (itFindExam2 != g_mapBmkMgr.end())
	{
		int nStudentCounts = 0; //��������
		for (auto objExamStudent : itFindExam2->second)
		{
			if (strCurrSubject == "ȫ��")	//��ʾ���п�Ŀ
			{
				if (strCurrScanStatus == "ȫ��")
				{
					bool bInsertStudent = false;	//��ǰ�����Ƿ��Ѿ����뿼��������׼��֤����Ϣ
					for (auto tmpSubject : _pCurrExam_->lSubjects)	//���ԵĿ�Ŀ�б����ԵĿ�Ŀ�� >= �����Ŀ�Ŀ��
					{
						std::string strScanStatus;
						bool bFindSub = false;	//�Ƿ��ڿ�����Ŀ�б����ҵ���ǰ��Ŀ��û�о���ӿ��ֶ�
						for (auto examSubject : objExamStudent.lSubjectScanStatus)	//�����Ŀ�Ŀ�б�
						{
							if (examSubject.nSubjectID == tmpSubject->nSubjID)
							{
								bFindSub = true;
								if (examSubject.nScaned)
									strScanStatus = "��ɨ";
								else
									strScanStatus = "δɨ";
								break;
							}
						}

						if (!bInsertStudent)
						{
							bInsertStudent = true;
// 							char szCount[10] = { 0 };
// 							sprintf_s(szCount, "%d", nStudentCounts + 1);
// 
// 							strData.append(szCount);
// 							strData.append("\t\t");
							strData.append(objExamStudent.strZkzh);
							strData.append("\t\t");
							strData.append(objExamStudent.strName);
							strData.append("\t\t");
						}
						strData.append(strScanStatus);
						strData.append("\t\t");
					}
					strData.append("\r\n");
				}
				else //��ʾ��ɨ����δɨ����
				{
					bool bInsertStudent = false;	//��ǰ�����Ƿ��Ѿ����뿼��������׼��֤����Ϣ
					for (auto tmpSubject : _pCurrExam_->lSubjects)	//���ԵĿ�Ŀ�б����ԵĿ�Ŀ�� >= �����Ŀ�Ŀ��
					{
						std::string strScanStatus;
						bool bFindSub = false;	//�Ƿ��ڿ�����Ŀ�б����ҵ���ǰ��Ŀ��û�о���ӿ��ֶ�
						for (auto examSubject : objExamStudent.lSubjectScanStatus)	//�����Ŀ�Ŀ�б�
						{
							if (examSubject.nSubjectID == tmpSubject->nSubjID)
							{
								bFindSub = true;
								if (nCurrScanStatus == 1)	//��ɨ
								{
									if (examSubject.nScaned)
									{
										strScanStatus = "��ɨ";

										if (!bInsertStudent)
										{
											bInsertStudent = true;
// 											char szCount[10] = { 0 };
// 											sprintf_s(szCount, "%d", nStudentCounts + 1);
// 
// 											strData.append(szCount);
// 											strData.append("\t\t");
											strData.append(objExamStudent.strZkzh);
											strData.append("\t\t");
											strData.append(objExamStudent.strName);
											strData.append("\t\t");
										}
										strData.append(strScanStatus);
										strData.append("\t\t");
									}
								}
								else //δɨ
								{
									if (!examSubject.nScaned)
									{
										strScanStatus = "δɨ";

										if (!bInsertStudent)
										{
											bInsertStudent = true;
// 											char szCount[10] = { 0 };
// 											sprintf_s(szCount, "%d", nStudentCounts + 1);
// 
// 											strData.append(szCount);
// 											strData.append("\t\t");
											strData.append(objExamStudent.strZkzh);
											strData.append("\t\t");
											strData.append(objExamStudent.strName);
											strData.append("\t\t");
										}
										strData.append(strScanStatus);
										strData.append("\t\t");
									}
								}
								break;
							}
						}
					}
					strData.append("\r\n");
				}
			}
			else  //��ʾ������Ŀ
			{
				if (strCurrScanStatus == "ȫ��")
				{
					std::string strScanStatus;
					bool bFindSub = false;	//�Ƿ��ڿ�����Ŀ�б����ҵ���ǰ��Ŀ��û�о���ӿ��ֶ�
					for (auto examSubject : objExamStudent.lSubjectScanStatus)	//�����Ŀ�Ŀ�б�
					{
						std::string strScanStatus;
						if (examSubject.nSubjectID == nCurrSubID)
						{
							if (examSubject.nScaned)
								strScanStatus = "��ɨ";
							else
								strScanStatus = "δɨ";

// 							char szCount[10] = { 0 };
// 							sprintf_s(szCount, "%d", nStudentCounts + 1);
// 
// 							strData.append(szCount);
// 							strData.append("\t\t");
							strData.append(objExamStudent.strZkzh);
							strData.append("\t\t");
							strData.append(objExamStudent.strName);
							strData.append("\t\t");
							strData.append(strScanStatus);
							strData.append("\t\t");

							break;
						}
					}
					strData.append("\r\n");
				}
				else //��ʾ��ɨ����δɨ����
				{
					bool bInsertStudent = false;	//��ǰ�����Ƿ��Ѿ����뿼��������׼��֤����Ϣ

					std::string strScanStatus;
					bool bFindSub = false;	//�Ƿ��ڿ�����Ŀ�б����ҵ���ǰ��Ŀ��û�о���ӿ��ֶ�
					for (auto examSubject : objExamStudent.lSubjectScanStatus)	//�����Ŀ�Ŀ�б�
					{
						if (examSubject.nSubjectID == nCurrSubID)
						{
							bFindSub = true;
							if (nCurrScanStatus == 1)	//��ɨ
							{
								if (examSubject.nScaned)
								{
									strScanStatus = "��ɨ";

// 									char szCount[10] = { 0 };
// 									sprintf_s(szCount, "%d", nStudentCounts + 1);
// 
// 									strData.append(szCount);
// 									strData.append("\t\t");
									strData.append(objExamStudent.strZkzh);
									strData.append("\t\t");
									strData.append(objExamStudent.strName);
									strData.append("\t\t");

									strData.append(strScanStatus);
									strData.append("\t\t");
								}
							}
							else //δɨ
							{
								if (!examSubject.nScaned)
								{
									strScanStatus = "δɨ";

									bInsertStudent = true;
// 									char szCount[10] = { 0 };
// 									sprintf_s(szCount, "%d", nStudentCounts + 1);
// 
// 									strData.append(szCount);
// 									strData.append("\t\t");
									strData.append(objExamStudent.strZkzh);
									strData.append("\t\t");
									strData.append(objExamStudent.strName);
									strData.append("\t\t");

									strData.append(strScanStatus);
									strData.append("\t\t");
								}
							}
							break;
						}
					}

					strData.append("\r\n");
				}
			}

			nStudentCounts++;
		}
	}
#else
	int nCount = m_lcBmk.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		std::string strRowData;
		for (int j = 1; j < nColumns; j++)
		{
			CString strItem = m_lcBmk.GetItemText(i, j);
			strData.append(T2A(strItem));
			strData.append("\t\t");
		}
		strData.append("\r\n");
	}
#endif

	TCHAR szFilter[] = { _T("TXT Files (*.txt)|*.txt|Excel Files (*.xls)|*.xls||") };
	CString fileName;
	fileName = "*.*";

	CFileDialog dlg(FALSE, _T("*.txt"), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);
	dlg.m_ofn.lpstrDefExt = _T(".txt");  

	if (dlg.DoModal() == IDOK)
	{
		fileName = dlg.GetFolderPath() + _T("\\") + dlg.GetFileName();  
		CFile file;
		file.Open(fileName, CFile::modeCreate | CFile::modeReadWrite);
		//file.Write(fileName.GetBuffer(fileName.GetLength()),fileName.GetLength());  

		//���༭���е�����д���ļ���  
		file.Write(strData.c_str(), strData.length());
		file.Close();
	}
}
