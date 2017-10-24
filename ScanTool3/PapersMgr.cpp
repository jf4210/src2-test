#include "stdafx.h"
#include "PapersMgr.h"
#include "NewMessageBox.h"


CPapersMgr::CPapersMgr()
{
	_pCurrExam = NULL;
	_pModel = NULL;
}


CPapersMgr::~CPapersMgr()
{
}

bool CPapersMgr::SavePapers(pPAPERSINFO pPapers)
{
	bool bResult = false;
	if (!pPapers)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "û���Ծ����Ϣ");
		dlg.DoModal();
		return bResult;
	}
	int nCount = pPapers->lPaper.size() + pPapers->lIssue.size();
	if (nCount == 0)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "���Ծ���ڣ�");
		dlg.DoModal();
		return bResult;
	}

	if (pPapers->nPapersType == 1)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "�����Ѿ���������Ծ���������ٴδ���ϴ�");
		dlg.DoModal();
		return bResult;
	}
	if (!_pCurrExam)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "û�п�����Ϣ");
		dlg.DoModal();
		return bResult;
	}
	if (_pCurrExam->nModel == 0 && !_pModel)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "δ����ģ����Ϣ");
		dlg.DoModal();
		return bResult;
	}

	if (_nScanAnswerModel_ == 1)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(1, 1, "ע��: ��ǰ���ڱ��� �͹���(Omr) ��ģʽ��");
		dlg.DoModal();
	}
	else if (_nScanAnswerModel_ == 2)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(1, 1, "ע��: ��ǰ���ڱ��� ������ ��ģʽ��");
		dlg.DoModal();
	}

	if (_pCurrExam->nModel == 0 && _nScanAnswerModel_ != 2)	//����ģʽ
	{
		bool bRecogComplete = true;
		for (auto p : pPapers->lPaper)
		{
			if (!p->bRecogComplete)
			{
				bRecogComplete = false;
				break;
			}
		}
		if (!bRecogComplete)
		{
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "���Ժ�ͼ������ʶ��");
			dlg.DoModal();
			return bResult;
		}
	}

	std::string strUser;
	std::string strEzs;
	int nTeacherId = -1;
	int nUserId = -1;

	strUser = _strUserName_;
	if (_pCurrExam->nModel == 1)
		strUser = _pCurrExam->strPersonID;
	strEzs = _strEzs_;
	nTeacherId = _nTeacherId_;
	nUserId = _nUserId_;

	USES_CONVERSION;
	if (pPapers->lIssue.size() > 0)
	{
		if (g_nOperatingMode == 2)
		{
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "����ʶ���쳣�Ծ������ϴ������ȴ����쳣�Ծ�");
			dlg.DoModal();
			return bResult;
		}
		else
		{
			std::string strMsg = Poco::format("����%d���������Щ�Ծ���Ҫ�����ҳ�ɨ�裬�Ƿ�ȷ���ϴ�?", (int)pPapers->lIssue.size());
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 2, strMsg);
			dlg.DoModal();
			if (dlg.m_nResult != IDYES)
				return bResult;
			pPapers->nPaperCount = pPapers->lPaper.size();		//�޸�ɨ���������������Ծ�ɾ�������㵽ɨ���Ծ��С�
		}
	}

	int nSubjectID = 0;
	if (_pCurrExam->nModel == 0)
		nSubjectID = _pModel->nSubjectID;
	
	bResult = WriteJsonFile(pPapers);
	if (!bResult)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "�����Ծ����Ϣ���ļ�ʧ�ܣ������ԣ�");
		dlg.DoModal();
		return bResult;
	}

	bResult = true;
	return bResult;
}

std::string CPapersMgr::AddPapersCompress(pPAPERSINFO pPapers)
{
	std::string strUser;
	std::string strEzs;
	int nTeacherId = -1;
	int nUserId = -1;

	strUser = _strUserName_;
	if (_pCurrExam->nModel == 1)
		strUser = _pCurrExam->strPersonID;
	strEzs = _strEzs_;
	nTeacherId = _nTeacherId_;
	nUserId = _nUserId_;

	int nSubjectID = 0;
	if (_pCurrExam->nModel == 0)
		nSubjectID = _pModel->nSubjectID;

	USES_CONVERSION;
	char szPapersSavePath[500] = { 0 };
	char szZipName[210] = { 0 };
	char szZipBaseName[200] = { 0 };

	Poco::LocalDateTime now;
	char szTime[50] = { 0 };
	sprintf_s(szTime, "%d%02d%02d%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

	CString strExtName = _T("");
	if (_pCurrExam->nModel == 1)
		strExtName = PAPERS_EXT_NAME_4TY;
	else
		strExtName = PAPERS_EXT_NAME;
	if (_pCurrExam->nModel == 0)
	{
		sprintf_s(szPapersSavePath, "%sPaper\\%s_%d-%d_%s_%d", T2A(g_strCurrentPath), strUser.c_str(), _pModel->nExamID, nSubjectID, szTime, pPapers->nPaperCount);
		sprintf_s(szZipBaseName, "%s_%d-%d_%s_%d", strUser.c_str(), _pModel->nExamID, nSubjectID, szTime, pPapers->nPaperCount);
		sprintf_s(szZipName, "%s_%d-%d_%s_%d%s", strUser.c_str(), _pModel->nExamID, nSubjectID, szTime, pPapers->nPaperCount, T2A(strExtName));
	}
	else
	{
		std::string strExamID = _pCurrExam->strExamID;
		sprintf_s(szPapersSavePath, "%sPaper\\%s_%s_%d_%s_%d", T2A(g_strCurrentPath), strUser.c_str(), strExamID.c_str(), nSubjectID, szTime, pPapers->nPaperCount);
		sprintf_s(szZipBaseName, "%s_%s_%d_%s_%d", strUser.c_str(), strExamID.c_str(), nSubjectID, szTime, pPapers->nPaperCount);
		sprintf_s(szZipName, "%s_%s_%d_%s_%d%s", strUser.c_str(), strExamID.c_str(), nSubjectID, szTime, pPapers->nPaperCount, T2A(strExtName));
	}

	//��ʱĿ¼�������Ա�ѹ��ʱ����ɨ��
	std::string strSrcPicDirPath;
	try
	{
		Poco::File tmpPath(CMyCodeConvert::Gb2312ToUtf8(_strCurrSavePath));

		char szCompressDirPath[500] = { 0 };
		if (_pCurrExam->nModel == 1)	//���Ĳ�������
			sprintf_s(szCompressDirPath, "%sPaper\\%s_ToCompress_UnPwd", T2A(g_strCurrentPath), szZipBaseName);
		else
			sprintf_s(szCompressDirPath, "%sPaper\\%s_ToCompress", T2A(g_strCurrentPath), szZipBaseName);

		std::string strUtf8NewPath = CMyCodeConvert::Gb2312ToUtf8(szCompressDirPath);

		tmpPath.renameTo(strUtf8NewPath);
		strSrcPicDirPath = szCompressDirPath;
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "��ʱ�ļ���������ʧ��(" + exc.message() + "): ";
		strLog.append(_strCurrSavePath);
		g_pLogger->information(strLog);
		strSrcPicDirPath = _strCurrSavePath;

		//******************	ע��	*******************
		//*************************************************
		//*************************************************
		//���ﱣ�������⣬�ᷢ������
		//*************************************************
	}

	TRACE("------------------- 3\n");

	pCOMPRESSTASK pTask = new COMPRESSTASK;
	pTask->strCompressFileName = szZipName;
	pTask->strExtName = T2A(strExtName);	//T2A(PAPERS_EXT_NAME);
	pTask->strSavePath = szPapersSavePath;
	pTask->strSrcFilePath = strSrcPicDirPath;
	pTask->pPapersInfo = pPapers;
	g_fmCompressLock.lock();
	g_lCompressTask.push_back(pTask);
	g_fmCompressLock.unlock();

	return szZipName;
}

void CPapersMgr::setCurrSavePath(std::string strPath)
{
	_strCurrSavePath = strPath;
}

void CPapersMgr::setExamInfo(pEXAMINFO pCurrExam, pMODEL pModel)
{
	_pCurrExam = pCurrExam;
	_pModel = pModel;
}

bool CPapersMgr::WriteJsonFile(pPAPERSINFO pPapers)
{
	clock_t start_pic, end_pic;
	start_pic = clock();
	TRACE("-------------- 11\n");
	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itNomarlPaper = pPapers->lPaper.begin();
	for (int i = 0; itNomarlPaper != pPapers->lPaper.end(); itNomarlPaper++, i++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itNomarlPaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itNomarlPaper)->strSN);
		jsnPaper.set("qk", (*itNomarlPaper)->nQKFlag);
		jsnPaper.set("wj", (*itNomarlPaper)->nWJFlag);
		jsnPaper.set("standardAnswer", _nScanAnswerModel_);		//0-�����Ծ�1-Omr���2-��������

		int nIssueFlag = 0;			//0 - �����Ծ���ȫ����ʶ�������ģ����˹���Ԥ��1 - �����Ծ�ɨ��Ա�ֶ��޸Ĺ���2-׼��֤��Ϊ�գ�ɨ��Աû���޸ģ�3-ɨ��Ա��ʶ����Ҫ��ɨ���Ծ�
		if ((*itNomarlPaper)->strSN.empty() && !(*itNomarlPaper)->bModifyZKZH)
			nIssueFlag = 2;
		if ((*itNomarlPaper)->bModifyZKZH)
			nIssueFlag = 1;
		jsnPaper.set("issueFlag", nIssueFlag);
		//++���ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
		jsnPaper.set("modify", (*itNomarlPaper)->bModifyZKZH);	//׼��֤���޸ı�ʶ
		jsnPaper.set("reScan", (*itNomarlPaper)->bReScan);		//��ɨ��ʶ
		jsnPaper.set("IssueList", 0);		//��ʶ�˿������������б����ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
		//--
		jsnPaper.set("picExchangeTimes", (*itNomarlPaper)->nPicsExchange);	//���ѧ�����Ծ�����ͼ��Ĵ���������1ҳ���2ҳ����˳��Ĵ�����ż�����൱��û�ж��������ξͱ�������

		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itNomarlPaper)->lSnResult.begin();
		for (; itSn != (*itNomarlPaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			jsnSnPosition.set("x", (*itSn)->rt.x);
			jsnSnPosition.set("y", (*itSn)->rt.y);
			jsnSnPosition.set("w", (*itSn)->rt.width);
			jsnSnPosition.set("h", (*itSn)->rt.height);
			jsnSnItem.set("position", jsnSnPosition);

			Poco::JSON::Array jsnSnValPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				Poco::JSON::Object jsnSnPosition2;
				jsnSnPosition2.set("x", itRect->rt.x);
				jsnSnPosition2.set("y", itRect->rt.y);
				jsnSnPosition2.set("w", itRect->rt.width);
				jsnSnPosition2.set("h", itRect->rt.height);
				jsnSnValPosition.add(jsnSnPosition2);
			}
			jsnSnItem.set("Pos", jsnSnValPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaper.set("snDetail", jsnSnDetailArry);

		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itNomarlPaper)->lOmrResult.begin();
		for (; itOmr != (*itNomarlPaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value1", itOmr->strRecogVal1);
			jsnOmr.set("value2", itOmr->strRecogVal2);
			jsnOmr.set("value3", itOmr->strRecogVal3);
			jsnOmr.set("doubt", itOmr->nDoubt);
			jsnOmr.set("pageId", itOmr->nPageId);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);

		Poco::JSON::Array jsnElectOmrArry;
		ELECTOMR_LIST::iterator itElectOmr = (*itNomarlPaper)->lElectOmrResult.begin();
		for (; itElectOmr != (*itNomarlPaper)->lElectOmrResult.end(); itElectOmr++)
		{
			Poco::JSON::Object jsnElectOmr;
			jsnElectOmr.set("paperId", i + 1);
			jsnElectOmr.set("doubt", itElectOmr->nDoubt);
			jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnElectOmr.set("value", itElectOmr->strRecogResult);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
			for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnElectOmr.set("position", jsnPositionArry);
			jsnElectOmrArry.add(jsnElectOmr);
		}
		if (jsnElectOmrArry.size() > 0)
			jsnPaper.set("electOmr", jsnElectOmrArry);		//ѡ������

		//-------------------------------------	��¼����ȱ��У������Ϣ���ڻָ��鿴�Ծ��ʱ���ã�����������;
		Poco::JSON::Array jsnPaperChkPointArry;
		for (auto itPic : (*itNomarlPaper)->lPic)
		{
			Poco::JSON::Object jsnPicPoint;

			Poco::JSON::Array jsnPaperFixArry;
			RECTLIST::iterator itFix = itPic->lFix.begin();
			for (int j = 0; itFix != itPic->lFix.end(); itFix++, j++)
			{
				Poco::JSON::Object jsnItem;
				jsnItem.set("x", itFix->rt.x);
				jsnItem.set("y", itFix->rt.y);
				jsnItem.set("w", itFix->rt.width);
				jsnItem.set("h", itFix->rt.height);

				jsnItem.set("Th", j);	//����������ͨ��ѡ��˳��ȷ���ģ�����ʶ��ʱ�ɲ鿴3����������
				jsnPaperFixArry.add(jsnItem);
			}
			Poco::JSON::Array jsnPaperModelCharAncharPointArry;
			RECTLIST::iterator itModelFix = itPic->lModelWordFix.begin();
			for (int j = 0; itModelFix != itPic->lModelWordFix.end(); itModelFix++, j++)
			{
				Poco::JSON::Object jsnItem;
				jsnItem.set("x", itModelFix->rt.x);
				jsnItem.set("y", itModelFix->rt.y);
				jsnItem.set("w", itModelFix->rt.width);
				jsnItem.set("h", itModelFix->rt.height);
				jsnPaperModelCharAncharPointArry.add(jsnItem);
			}
			Poco::JSON::Array jsnPaperCharAncharAreaArry;
			CHARACTER_ANCHOR_AREA_LIST::iterator itCharAnchorArea = itPic->lCharacterAnchorArea.begin();
			for (; itCharAnchorArea != itPic->lCharacterAnchorArea.end(); itCharAnchorArea++)
			{
				Poco::JSON::Object jsnItem;
				jsnItem.set("x", (*itCharAnchorArea)->rt.x);
				jsnItem.set("y", (*itCharAnchorArea)->rt.y);
				jsnItem.set("w", (*itCharAnchorArea)->rt.width);
				jsnItem.set("h", (*itCharAnchorArea)->rt.height);

				Poco::JSON::Array jsnPaperCharAncharPointArry;
				for (int j = 0; j < (*itCharAnchorArea)->vecCharacterRt.size(); j++)	//�������ʶ������ʶ�𵽵Ķ���
				{
					Poco::JSON::Object jsnItem2;
					jsnItem2.set("x", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.x);
					jsnItem2.set("y", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.y);
					jsnItem2.set("w", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.width);
					jsnItem2.set("h", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.height);
					jsnPaperCharAncharPointArry.add(jsnItem2);
				}
				jsnItem.set("AP", jsnPaperCharAncharPointArry);

				jsnPaperCharAncharAreaArry.add(jsnItem);
			}
			jsnPicPoint.set("FL", jsnPaperFixArry);	//�����б�
			jsnPicPoint.set("MFL", jsnPaperModelCharAncharPointArry);	//ģ�嶨���б����������ֶ����Ӧ�����ֵ�
			jsnPicPoint.set("CL", jsnPaperCharAncharAreaArry);	//����ʶ�����б�

			jsnPaperChkPointArry.add(jsnPicPoint);
		}
		jsnPaper.set("ChkPoint", jsnPaperChkPointArry);
		//-------------------------------------

		jsnPaperArry.add(jsnPaper);
	}

	if (g_nOperatingMode == 1)		//��ģʽʱ���쳣�Ծ�Ҳһ���ϴ����������ʶ
	{
		PAPER_LIST::iterator itIssuePaper = pPapers->lIssue.begin();
		int nNomarlCount = pPapers->lPaper.size();
		for (int j = nNomarlCount; itIssuePaper != pPapers->lIssue.end(); itIssuePaper++, j++)
		{
			Poco::JSON::Object jsnPaper;
			jsnPaper.set("name", (*itIssuePaper)->strStudentInfo);
			jsnPaper.set("zkzh", (*itIssuePaper)->strSN);
			jsnPaper.set("qk", (*itIssuePaper)->nQKFlag);
			jsnPaper.set("wj", (*itIssuePaper)->nWJFlag);
			jsnPaper.set("standardAnswer", _nScanAnswerModel_);		//0-�����Ծ�1-Omr���2-��������

			int nIssueFlag = 0;			//0 - �����Ծ���ȫ����ʶ�������ģ����˹���Ԥ��1 - �����Ծ�ɨ��Ա�ֶ��޸Ĺ���2-׼��֤��Ϊ�գ�ɨ��Աû���޸ģ�3-ɨ��Ա��ʶ����Ҫ��ɨ���Ծ�
			if ((*itIssuePaper)->strSN.empty())
				nIssueFlag = 2;
			if ((*itIssuePaper)->bReScan)		//������ɨȨ�޸��󣬷ź�������
				nIssueFlag = 3;
			jsnPaper.set("issueFlag", nIssueFlag);
			//++���ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
			jsnPaper.set("modify", (*itIssuePaper)->bModifyZKZH);	//׼��֤���޸ı�ʶ
			jsnPaper.set("reScan", (*itIssuePaper)->bReScan);		//��ɨ��ʶ
			jsnPaper.set("IssueList", 1);		//��ʶ�˿������������б����ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
												//--
			jsnPaper.set("picExchangeTimes", (*itIssuePaper)->nPicsExchange);	//���ѧ�����Ծ�����ͼ��Ĵ���������1ҳ���2ҳ����˳��Ĵ�����ż�����൱��û�ж��������ξͱ�������

			Poco::JSON::Array jsnSnDetailArry;
			SNLIST::iterator itSn = (*itIssuePaper)->lSnResult.begin();
			for (; itSn != (*itIssuePaper)->lSnResult.end(); itSn++)
			{
				Poco::JSON::Object jsnSnItem;
				jsnSnItem.set("sn", (*itSn)->nItem);
				jsnSnItem.set("val", (*itSn)->nRecogVal);

				Poco::JSON::Object jsnSnPosition;
				jsnSnPosition.set("x", (*itSn)->rt.x);
				jsnSnPosition.set("y", (*itSn)->rt.y);
				jsnSnPosition.set("w", (*itSn)->rt.width);
				jsnSnPosition.set("h", (*itSn)->rt.height);
				jsnSnItem.set("position", jsnSnPosition);

				Poco::JSON::Array jsnSnValPosition;
				RECTLIST::iterator itRect = (*itSn)->lSN.begin();
				for (; itRect != (*itSn)->lSN.end(); itRect++)
				{
					Poco::JSON::Object jsnSnPosition2;
					jsnSnPosition2.set("x", itRect->rt.x);
					jsnSnPosition2.set("y", itRect->rt.y);
					jsnSnPosition2.set("w", itRect->rt.width);
					jsnSnPosition2.set("h", itRect->rt.height);
					jsnSnValPosition.add(jsnSnPosition2);
				}
				jsnSnItem.set("Pos", jsnSnValPosition);
				jsnSnDetailArry.add(jsnSnItem);
			}
			jsnPaper.set("snDetail", jsnSnDetailArry);

			Poco::JSON::Array jsnOmrArry;
			OMRRESULTLIST::iterator itOmr = (*itIssuePaper)->lOmrResult.begin();
			for (; itOmr != (*itIssuePaper)->lOmrResult.end(); itOmr++)
			{
				Poco::JSON::Object jsnOmr;
				jsnOmr.set("th", itOmr->nTH);
				jsnOmr.set("type", itOmr->nSingle + 1);
				jsnOmr.set("value", itOmr->strRecogVal);
				jsnOmr.set("value1", itOmr->strRecogVal1);
				jsnOmr.set("value2", itOmr->strRecogVal2);
				jsnOmr.set("value3", itOmr->strRecogVal3);
				jsnOmr.set("doubt", itOmr->nDoubt);
				Poco::JSON::Array jsnPositionArry;
				RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
				for (; itRect != itOmr->lSelAnswer.end(); itRect++)
				{
					Poco::JSON::Object jsnItem;
					char szVal[5] = { 0 };
					sprintf_s(szVal, "%c", itRect->nAnswer + 65);
					jsnItem.set("val", szVal);
					jsnItem.set("x", itRect->rt.x);
					jsnItem.set("y", itRect->rt.y);
					jsnItem.set("w", itRect->rt.width);
					jsnItem.set("h", itRect->rt.height);
					jsnPositionArry.add(jsnItem);
				}
				jsnOmr.set("position", jsnPositionArry);
				jsnOmrArry.add(jsnOmr);
			}
			jsnPaper.set("omr", jsnOmrArry);

			Poco::JSON::Array jsnElectOmrArry;
			ELECTOMR_LIST::iterator itElectOmr = (*itIssuePaper)->lElectOmrResult.begin();
			for (; itElectOmr != (*itIssuePaper)->lElectOmrResult.end(); itElectOmr++)
			{
				Poco::JSON::Object jsnElectOmr;
				jsnElectOmr.set("paperId", j + 1);
				jsnElectOmr.set("doubt", itElectOmr->nDoubt);
				jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
				jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
				jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
				jsnElectOmr.set("value", itElectOmr->strRecogResult);
				Poco::JSON::Array jsnPositionArry;
				RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
				for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
				{
					Poco::JSON::Object jsnItem;
					char szVal[5] = { 0 };
					sprintf_s(szVal, "%c", itRect->nAnswer + 65);
					jsnItem.set("val", szVal);
					jsnItem.set("x", itRect->rt.x);
					jsnItem.set("y", itRect->rt.y);
					jsnItem.set("w", itRect->rt.width);
					jsnItem.set("h", itRect->rt.height);
					jsnPositionArry.add(jsnItem);
				}
				jsnElectOmr.set("position", jsnPositionArry);
				jsnElectOmrArry.add(jsnElectOmr);
			}
			jsnPaper.set("electOmr", jsnElectOmrArry);		//ѡ������

			//-------------------------------------	��¼����ȱ��У������Ϣ���ڻָ��鿴�Ծ��ʱ���ã�����������;
			Poco::JSON::Array jsnPaperChkPointArry;
			for (auto itPic : (*itIssuePaper)->lPic)
			{
				Poco::JSON::Object jsnPicPoint;

				Poco::JSON::Array jsnPaperFixArry;
				RECTLIST::iterator itFix = itPic->lFix.begin();
				for (int j = 0; itFix != itPic->lFix.end(); itFix++, j++)
				{
					Poco::JSON::Object jsnItem;
					jsnItem.set("x", itFix->rt.x);
					jsnItem.set("y", itFix->rt.y);
					jsnItem.set("w", itFix->rt.width);
					jsnItem.set("h", itFix->rt.height);

					jsnItem.set("Th", j);	//����������ͨ��ѡ��˳��ȷ���ģ�����ʶ��ʱ�ɲ鿴3����������
					jsnPaperFixArry.add(jsnItem);
				}
				Poco::JSON::Array jsnPaperModelCharAncharPointArry;
				RECTLIST::iterator itModelFix = itPic->lModelWordFix.begin();
				for (int j = 0; itModelFix != itPic->lModelWordFix.end(); itModelFix++, j++)
				{
					Poco::JSON::Object jsnItem;
					jsnItem.set("x", itModelFix->rt.x);
					jsnItem.set("y", itModelFix->rt.y);
					jsnItem.set("w", itModelFix->rt.width);
					jsnItem.set("h", itModelFix->rt.height);
					jsnPaperModelCharAncharPointArry.add(jsnItem);
				}
				Poco::JSON::Array jsnPaperCharAncharAreaArry;
				CHARACTER_ANCHOR_AREA_LIST::iterator itCharAnchorArea = itPic->lCharacterAnchorArea.begin();
				for (; itCharAnchorArea != itPic->lCharacterAnchorArea.end(); itCharAnchorArea++)
				{
					Poco::JSON::Object jsnItem;
					jsnItem.set("x", (*itCharAnchorArea)->rt.x);
					jsnItem.set("y", (*itCharAnchorArea)->rt.y);
					jsnItem.set("w", (*itCharAnchorArea)->rt.width);
					jsnItem.set("h", (*itCharAnchorArea)->rt.height);

					Poco::JSON::Array jsnPaperCharAncharPointArry;
					for (int j = 0; j < (*itCharAnchorArea)->vecCharacterRt.size(); j++)	//�������ʶ������ʶ�𵽵Ķ���
					{
						Poco::JSON::Object jsnItem2;
						jsnItem2.set("x", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.x);
						jsnItem2.set("y", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.y);
						jsnItem2.set("w", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.width);
						jsnItem2.set("h", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.height);
						jsnPaperCharAncharPointArry.add(jsnItem2);
					}
					jsnItem.set("AP", jsnPaperCharAncharPointArry);

					jsnPaperCharAncharAreaArry.add(jsnItem);
				}
				jsnPicPoint.set("FL", jsnPaperFixArry);	//�����б�
				jsnPicPoint.set("MFL", jsnPaperModelCharAncharPointArry);	//ģ�嶨���б����������ֶ����Ӧ�����ֵ�
				jsnPicPoint.set("CL", jsnPaperCharAncharAreaArry);	//����ʶ�����б�

				jsnPaperChkPointArry.add(jsnPicPoint);
			}
			jsnPaper.set("ChkPoint", jsnPaperChkPointArry);
			//-------------------------------------

			jsnPaperArry.add(jsnPaper);						//�����Ծ�Ҳ�����б���
		}
	}

	//д�Ծ����Ϣ���ļ�
	std::string strUploader;
	if (_pCurrExam->nModel == 0)
		strUploader = CMyCodeConvert::Gb2312ToUtf8(_strUserName_);
	else
		strUploader = CMyCodeConvert::Gb2312ToUtf8(_pCurrExam->strPersonID);
	std::string sEzs = _strEzs_;
	Poco::JSON::Object jsnFileData;

	if (_pCurrExam->nModel == 0)		//����ģʽ(�������汾)���������յ�ֱ��������zip�ļ�������Ҫʶ�����е���Ϣ�ļ�
	{
		jsnFileData.set("examId", _pModel->nExamID);
		jsnFileData.set("subjectId", _pModel->nSubjectID);
	}
	jsnFileData.set("uploader", strUploader);
	jsnFileData.set("ezs", sEzs);
	jsnFileData.set("nTeacherId", _nTeacherId_);
	jsnFileData.set("nUserId", _nUserId_);
	jsnFileData.set("scanNum", pPapers->nPaperCount);		//ɨ���ѧ������
	jsnFileData.set("detail", jsnPaperArry);
	jsnFileData.set("desc", CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersDesc));

	jsnFileData.set("nOmrDoubt", pPapers->nOmrDoubt);
	jsnFileData.set("nOmrNull", pPapers->nOmrNull);
	jsnFileData.set("nSnNull", pPapers->nSnNull);
	jsnFileData.set("RecogMode", g_nOperatingMode);			//ʶ��ģʽ��1-��ģʽ(��������У��㲻ֹͣʶ��)��2-�ϸ�ģʽ
	std::stringstream jsnString;
	jsnFileData.stringify(jsnString, 0);

	end_pic = clock();
	TRACE("-------------- 12 --> time: %d\n", end_pic - start_pic);
	std::string strFileData;
	if (!encString(jsnString.str(), strFileData))
		strFileData = jsnString.str();

	TRACE("-------------- 13\n");
	char szExamInfoPath[MAX_PATH] = { 0 };
	sprintf_s(szExamInfoPath, "%s\\papersInfo.dat", _strCurrSavePath.c_str());
	ofstream out(szExamInfoPath);
	out << strFileData.c_str();
	out.close();

	bool bResult = false;
	//���Ŀ��Լ��paperд�����
	try
	{
		Poco::File jsonFile(CMyCodeConvert::Gb2312ToUtf8(szExamInfoPath));
		if (jsonFile.exists())
			bResult = true;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErr = "�����Ծ����Ϣ���ļ�[" + std::string(szExamInfoPath) + "]ʧ��" + exc.message();
		g_pLogger->information(strErr);
	}
	return bResult;
}
