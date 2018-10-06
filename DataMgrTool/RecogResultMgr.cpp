#include "stdafx.h"
#include "RecogResultMgr.h"


CRecogResultMgr::CRecogResultMgr(pPAPERSINFO pPapers)
	:_pPapers(pPapers)
{
	nExamId = _pPapers->nExamID;
	nSubjuctId = _pPapers->nSubjectID;
	strPkgName = _pPapers->strPapersName;
	GetRecogResult();
}


CRecogResultMgr::~CRecogResultMgr()
{
}

std::string CRecogResultMgr::GetSnResult()
{
	return _strSnResult;
}

std::string CRecogResultMgr::GetOmrResult()
{
	return _strOmrResult;
}

std::string CRecogResultMgr::GetElectOmrResult()
{
	return _strElectOmrResult;
}

void CRecogResultMgr::GetRecogResult()
{
	bool bHasElectOmr = false;
	Poco::JSON::Array snArry;
	Poco::JSON::Array omrArry;
	Poco::JSON::Array electOmrArry;
	std::stringstream jsnSnString;
	std::stringstream jsnOmrString;
	std::stringstream jsnElectOmrString;
	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itPaper = _pPapers->lPaper.begin();
	for (int i = 0; itPaper != _pPapers->lPaper.end(); itPaper++, i++)
	{
		if ((*itPaper)->strMd5Key.empty())
		{
			std::string strStudentInfo = _pPapers->strPapersName + "_" + (*itPaper)->strStudentInfo;
			std::string strStudentKey = calcMd5(strStudentInfo);
			(*itPaper)->strMd5Key = strStudentKey;
		}

		Poco::JSON::Object jsnPaperSN;
		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itPaper)->lSnResult.begin();
		for (; itSn != (*itPaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				jsnSnPosition.set("x", itRect->rt.x);
				jsnSnPosition.set("y", itRect->rt.y);
				jsnSnPosition.set("w", itRect->rt.width);
				jsnSnPosition.set("h", itRect->rt.height);
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaperSN.set("examId", _pPapers->nExamID);
		jsnPaperSN.set("subjectId", _pPapers->nSubjectID);
		jsnPaperSN.set("userId", _pPapers->nUserId);
		jsnPaperSN.set("teacherId", _pPapers->nTeacherId);
		jsnPaperSN.set("zkzh", (*itPaper)->strSN);
		jsnPaperSN.set("papers", _pPapers->strPapersName);
		if ((*itPaper)->strSN != "")
			jsnPaperSN.set("doubt", 0);
		else
			jsnPaperSN.set("doubt", 1);
		jsnPaperSN.set("studentKey", (*itPaper)->strMd5Key);
		jsnPaperSN.set("detail", jsnSnDetailArry);

		snArry.add(jsnPaperSN);

		Poco::JSON::Object jsnPaperOMR;
		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itPaper)->lOmrResult.begin();
		for (; itOmr != (*itPaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value2", itOmr->strRecogVal2);
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
		jsnPaperOMR.set("omr", jsnOmrArry);
		jsnPaperOMR.set("examId", _pPapers->nExamID);
		jsnPaperOMR.set("subjectId", _pPapers->nSubjectID);
		jsnPaperOMR.set("userId", _pPapers->nUserId);
		jsnPaperOMR.set("teacherId", _pPapers->nTeacherId);
		jsnPaperOMR.set("zkzh", (*itPaper)->strSN);
		jsnPaperOMR.set("nOmrAnswerFlag", (*itPaper)->nStandardAnswer);
		jsnPaperOMR.set("papers", _pPapers->strPapersName);
		jsnPaperOMR.set("studentKey", (*itPaper)->strMd5Key);
		omrArry.add(jsnPaperOMR);

		if ((*itPaper)->lElectOmrResult.size() > 0)
		{
			bHasElectOmr = true;
			Poco::JSON::Object jsnPaperElectOmr;
			Poco::JSON::Array jsnElectOmrArry;
			ELECTOMR_LIST::iterator itElectOmr = (*itPaper)->lElectOmrResult.begin();
			for (; itElectOmr != (*itPaper)->lElectOmrResult.end(); itElectOmr++)
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
			jsnPaperElectOmr.set("electOmr", jsnElectOmrArry);		//选做题结果
			jsnPaperElectOmr.set("examId", _pPapers->nExamID);
			jsnPaperElectOmr.set("subjectId", _pPapers->nSubjectID);
			jsnPaperElectOmr.set("userId", _pPapers->nUserId);
			jsnPaperElectOmr.set("teacherId", _pPapers->nTeacherId);
			jsnPaperElectOmr.set("zkzh", (*itPaper)->strSN);
			jsnPaperElectOmr.set("papers", _pPapers->strPapersName);
			jsnPaperElectOmr.set("studentKey", (*itPaper)->strMd5Key);
			electOmrArry.add(jsnPaperElectOmr);
		}
	}
	snArry.stringify(jsnSnString, 0);
	_strSnResult = jsnSnString.str();
	omrArry.stringify(jsnOmrString, 0);
	_strOmrResult = jsnOmrString.str();
	_bHasElectOmr = bHasElectOmr;
	if (bHasElectOmr)
	{
		electOmrArry.stringify(jsnElectOmrString, 0);
		_strElectOmrResult = jsnElectOmrString.str();
	}
}

std::string CRecogResultMgr::calcMd5(std::string& strInfo)
{
	Poco::MD5Engine md5;
	Poco::DigestOutputStream outstr(md5);
	outstr << strInfo;
	outstr.flush();
	const Poco::DigestEngine::Digest& digest = md5.digest();
	return Poco::DigestEngine::digestToHex(digest);
}
