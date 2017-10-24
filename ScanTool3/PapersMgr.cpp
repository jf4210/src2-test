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
		dlg.setShowInfo(2, 1, "没有试卷袋信息");
		dlg.DoModal();
		return bResult;
	}
	int nCount = pPapers->lPaper.size() + pPapers->lIssue.size();
	if (nCount == 0)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "无试卷存在！");
		dlg.DoModal();
		return bResult;
	}

	if (pPapers->nPapersType == 1)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "这是已经打包过的试卷包，不能再次打包上传");
		dlg.DoModal();
		return bResult;
	}
	if (!_pCurrExam)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "没有考试信息");
		dlg.DoModal();
		return bResult;
	}
	if (_pCurrExam->nModel == 0 && !_pModel)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "未设置模板信息");
		dlg.DoModal();
		return bResult;
	}

	if (_nScanAnswerModel_ == 1)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(1, 1, "注意: 当前处于保存 客观题(Omr) 答案模式！");
		dlg.DoModal();
	}
	else if (_nScanAnswerModel_ == 2)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(1, 1, "注意: 当前处于保存 主观题 答案模式！");
		dlg.DoModal();
	}

	if (_pCurrExam->nModel == 0 && _nScanAnswerModel_ != 2)	//网阅模式
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
			dlg.setShowInfo(2, 1, "请稍后，图像正在识别！");
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
			dlg.setShowInfo(2, 1, "存在识别异常试卷，不能上传，请先处理异常试卷");
			dlg.DoModal();
			return bResult;
		}
		else
		{
			std::string strMsg = Poco::format("存在%d份问题卷，这些试卷需要单独找出扫描，是否确定上传?", (int)pPapers->lIssue.size());
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 2, strMsg);
			dlg.DoModal();
			if (dlg.m_nResult != IDYES)
				return bResult;
			pPapers->nPaperCount = pPapers->lPaper.size();		//修改扫描数量，将问题试卷删除，不算到扫描试卷中。
		}
	}

	int nSubjectID = 0;
	if (_pCurrExam->nModel == 0)
		nSubjectID = _pModel->nSubjectID;
	
	bResult = WriteJsonFile(pPapers);
	if (!bResult)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "保存试卷袋信息到文件失败，请重试！");
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

	//临时目录改名，以便压缩时继续扫描
	std::string strSrcPicDirPath;
	try
	{
		Poco::File tmpPath(CMyCodeConvert::Gb2312ToUtf8(_strCurrSavePath));

		char szCompressDirPath[500] = { 0 };
		if (_pCurrExam->nModel == 1)	//手阅不用密码
			sprintf_s(szCompressDirPath, "%sPaper\\%s_ToCompress_UnPwd", T2A(g_strCurrentPath), szZipBaseName);
		else
			sprintf_s(szCompressDirPath, "%sPaper\\%s_ToCompress", T2A(g_strCurrentPath), szZipBaseName);

		std::string strUtf8NewPath = CMyCodeConvert::Gb2312ToUtf8(szCompressDirPath);

		tmpPath.renameTo(strUtf8NewPath);
		strSrcPicDirPath = szCompressDirPath;
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "临时文件夹重命名失败(" + exc.message() + "): ";
		strLog.append(_strCurrSavePath);
		g_pLogger->information(strLog);
		strSrcPicDirPath = _strCurrSavePath;

		//******************	注意	*******************
		//*************************************************
		//*************************************************
		//这里保存有问题，会发生错乱
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
		jsnPaper.set("standardAnswer", _nScanAnswerModel_);		//0-正常试卷，1-Omr标答，2-主观题标答

		int nIssueFlag = 0;			//0 - 正常试卷，完全机器识别正常的，无人工干预，1 - 正常试卷，扫描员手动修改过，2-准考证号为空，扫描员没有修改，3-扫描员标识了需要重扫的试卷。
		if ((*itNomarlPaper)->strSN.empty() && !(*itNomarlPaper)->bModifyZKZH)
			nIssueFlag = 2;
		if ((*itNomarlPaper)->bModifyZKZH)
			nIssueFlag = 1;
		jsnPaper.set("issueFlag", nIssueFlag);
		//++在上传服务器时无用，只在从Pkg恢复Papers时有用
		jsnPaper.set("modify", (*itNomarlPaper)->bModifyZKZH);	//准考证号修改标识
		jsnPaper.set("reScan", (*itNomarlPaper)->bReScan);		//重扫标识
		jsnPaper.set("IssueList", 0);		//标识此考生属于问题列表，在上传服务器时无用，只在从Pkg恢复Papers时有用
		//--
		jsnPaper.set("picExchangeTimes", (*itNomarlPaper)->nPicsExchange);	//这个学生的试卷被调换图像的次数，即第1页与第2页调换顺序的次数，偶数次相当于没有动，奇数次就被调换了

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
			jsnPaper.set("electOmr", jsnElectOmrArry);		//选做题结果

		//-------------------------------------	记录定点缺考校验点等信息，在恢复查看试卷包时有用，不做其他用途
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

				jsnItem.set("Th", j);	//定点的题号是通框选的顺序确定的，在重识别时可查看3个定点的情况
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
				for (int j = 0; j < (*itCharAnchorArea)->vecCharacterRt.size(); j++)	//添加文字识别区已识别到的定点
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
			jsnPicPoint.set("FL", jsnPaperFixArry);	//定点列表
			jsnPicPoint.set("MFL", jsnPaperModelCharAncharPointArry);	//模板定点列表，与上面文字定点对应的文字点
			jsnPicPoint.set("CL", jsnPaperCharAncharAreaArry);	//文字识别区列表

			jsnPaperChkPointArry.add(jsnPicPoint);
		}
		jsnPaper.set("ChkPoint", jsnPaperChkPointArry);
		//-------------------------------------

		jsnPaperArry.add(jsnPaper);
	}

	if (g_nOperatingMode == 1)		//简单模式时，异常试卷也一起上传，做特殊标识
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
			jsnPaper.set("standardAnswer", _nScanAnswerModel_);		//0-正常试卷，1-Omr标答，2-主观题标答

			int nIssueFlag = 0;			//0 - 正常试卷，完全机器识别正常的，无人工干预，1 - 正常试卷，扫描员手动修改过，2-准考证号为空，扫描员没有修改，3-扫描员标识了需要重扫的试卷。
			if ((*itIssuePaper)->strSN.empty())
				nIssueFlag = 2;
			if ((*itIssuePaper)->bReScan)		//设置重扫权限更大，放后面设置
				nIssueFlag = 3;
			jsnPaper.set("issueFlag", nIssueFlag);
			//++在上传服务器时无用，只在从Pkg恢复Papers时有用
			jsnPaper.set("modify", (*itIssuePaper)->bModifyZKZH);	//准考证号修改标识
			jsnPaper.set("reScan", (*itIssuePaper)->bReScan);		//重扫标识
			jsnPaper.set("IssueList", 1);		//标识此考生属于问题列表，在上传服务器时无用，只在从Pkg恢复Papers时有用
												//--
			jsnPaper.set("picExchangeTimes", (*itIssuePaper)->nPicsExchange);	//这个学生的试卷被调换图像的次数，即第1页与第2页调换顺序的次数，偶数次相当于没有动，奇数次就被调换了

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
			jsnPaper.set("electOmr", jsnElectOmrArry);		//选做题结果

			//-------------------------------------	记录定点缺考校验点等信息，在恢复查看试卷包时有用，不做其他用途
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

					jsnItem.set("Th", j);	//定点的题号是通框选的顺序确定的，在重识别时可查看3个定点的情况
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
					for (int j = 0; j < (*itCharAnchorArea)->vecCharacterRt.size(); j++)	//添加文字识别区已识别到的定点
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
				jsnPicPoint.set("FL", jsnPaperFixArry);	//定点列表
				jsnPicPoint.set("MFL", jsnPaperModelCharAncharPointArry);	//模板定点列表，与上面文字定点对应的文字点
				jsnPicPoint.set("CL", jsnPaperCharAncharAreaArry);	//文字识别区列表

				jsnPaperChkPointArry.add(jsnPicPoint);
			}
			jsnPaper.set("ChkPoint", jsnPaperChkPointArry);
			//-------------------------------------

			jsnPaperArry.add(jsnPaper);						//问题试卷也放入列表中
		}
	}

	//写试卷袋信息到文件
	std::string strUploader;
	if (_pCurrExam->nModel == 0)
		strUploader = CMyCodeConvert::Gb2312ToUtf8(_strUserName_);
	else
		strUploader = CMyCodeConvert::Gb2312ToUtf8(_pCurrExam->strPersonID);
	std::string sEzs = _strEzs_;
	Poco::JSON::Object jsnFileData;

	if (_pCurrExam->nModel == 0)		//手阅模式(即天喻版本)，服务器收到直接重命名zip文件，不需要识别其中的信息文件
	{
		jsnFileData.set("examId", _pModel->nExamID);
		jsnFileData.set("subjectId", _pModel->nSubjectID);
	}
	jsnFileData.set("uploader", strUploader);
	jsnFileData.set("ezs", sEzs);
	jsnFileData.set("nTeacherId", _nTeacherId_);
	jsnFileData.set("nUserId", _nUserId_);
	jsnFileData.set("scanNum", pPapers->nPaperCount);		//扫描的学生数量
	jsnFileData.set("detail", jsnPaperArry);
	jsnFileData.set("desc", CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersDesc));

	jsnFileData.set("nOmrDoubt", pPapers->nOmrDoubt);
	jsnFileData.set("nOmrNull", pPapers->nOmrNull);
	jsnFileData.set("nSnNull", pPapers->nSnNull);
	jsnFileData.set("RecogMode", g_nOperatingMode);			//识别模式，1-简单模式(遇到问题校验点不停止识别)，2-严格模式
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
	//网阅考试检查paper写入情况
	try
	{
		Poco::File jsonFile(CMyCodeConvert::Gb2312ToUtf8(szExamInfoPath));
		if (jsonFile.exists())
			bResult = true;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErr = "保存试卷袋信息到文件[" + std::string(szExamInfoPath) + "]失败" + exc.message();
		g_pLogger->information(strErr);
	}
	return bResult;
}
