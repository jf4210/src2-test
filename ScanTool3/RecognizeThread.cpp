#include "stdafx.h"
#include "RecognizeThread.h"
#include "ScanTool3.h"
#include "OmrRecog.h"
#include "ScanMgrDlg.h"
#include "CoordinationConvert.h"
#include "PaperRecogMgr.h"

using namespace cv;
CRecognizeThread::CRecognizeThread()
{
	m_pStudentMgr = NULL;
#ifdef USE_TESSERACT
	m_pTess = NULL;
#endif
}

CRecognizeThread::~CRecognizeThread()
{
	if (m_vecContours.size())
		m_vecContours.clear();
	g_pLogger->information("RecognizeThread exit.");
	TRACE("RecognizeThread exit1.\n");
}

void CRecognizeThread::run()
{
	g_pLogger->information("RecognizeThread start...");
	TRACE("RecognizeThread start...\n");
	eExit.reset();
	
	if (_nUseOcrRecogSn_)
	{
		USES_CONVERSION;
		m_pStudentMgr = new CStudentMgr();
		std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
		bool bResult = m_pStudentMgr->InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
	}

	while (!g_nExitFlag)
	{
		//===========================
		pST_SCAN_PAPER pScanPaperTask = NULL;
		g_fmScanPaperListLock.lock();
		SCAN_PAPER_LIST::iterator itScanPaper = g_lScanPaperTask.begin();
		for (; itScanPaper != g_lScanPaperTask.end();)
		{
			pScanPaperTask = *itScanPaper;
			itScanPaper = g_lScanPaperTask.erase(itScanPaper);
			break;
		}
		g_fmScanPaperListLock.unlock();

		if(pScanPaperTask) HandleScanPicTask(pScanPaperTask);	//原始扫描的试卷信息放入对应的考试试卷列表中，由试卷列表去释放
		//SAFE_RELEASE(pScanPaperTask);
		//===========================

		pRECOGTASK pTask = NULL;
		g_fmRecog.lock();
		RECOGTASKLIST::iterator it = g_lRecogTask.begin();
		for (; it != g_lRecogTask.end();)
		{
			pTask = *it;
			it = g_lRecogTask.erase(it);
			break;
		}
		g_fmRecog.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(100);
			continue;
		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}

	std::map<pMODEL, pMODELINFO>::iterator it = _mapModel.begin();
	for (; it != _mapModel.end();)
	{
		pMODELINFO pModelInfo = NULL;
		pModelInfo = it->second;
		if (pModelInfo)
		{
			delete pModelInfo;
			pModelInfo = NULL;
		}
		it = _mapModel.erase(it);
	}
	eExit.set();

#ifdef USE_TESSERACT
	SAFE_RELEASE(m_pTess);
#endif
	SAFE_RELEASE(m_pStudentMgr);
	TRACE("RecognizeThread exit 0\n");
}

bool CRecognizeThread::RecogPaper(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	clock_t start_pic, end_pic;
	start_pic = clock();

	bool bResult = false;
	CPaperRecogMgr paperRecogMgrObj(g_nOperatingMode);
	if (g_nOperatingMode == 1)
	{
		if (pModel->nUseWordAnchorPoint)
			bResult = paperRecogMgrObj.RecogCharacter(nPic, matCompPic, pPic, pModel);
		else
			bResult = paperRecogMgrObj.RecogFixCP(nPic, matCompPic, pPic, pModel);

	#ifdef WarpAffine_TEST
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		bResult = GetFixPicTransfer(nPic, matCompPic, pPic, pModel, inverseMat);
	#endif
		bResult = paperRecogMgrObj.RecogABModel(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogCourse(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogQKCP(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogWJCP(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogGrayCP(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogWhiteCP(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogSN(nPic, matCompPic, pPic, pModel);
		{
			(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
			if ((static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.empty())
			{
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
			}
		}
		bResult = paperRecogMgrObj.RecogOMR(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogElectOmr(nPic, matCompPic, pPic, pModel);
	}
	else
	{
		if (pModel->nUseWordAnchorPoint)
			bResult = paperRecogMgrObj.RecogCharacter(nPic, matCompPic, pPic, pModel);
		else
			bResult = paperRecogMgrObj.RecogFixCP(nPic, matCompPic, pPic, pModel);

	#ifdef WarpAffine_TEST
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		if (bResult) bResult = GetFixPicTransfer(nPic, matCompPic, pPic, pModel, inverseMat);
	#endif
		if (bResult) bResult = paperRecogMgrObj.RecogABModel(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogCourse(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogQKCP(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogWJCP(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogGrayCP(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogWhiteCP(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogSN(nPic, matCompPic, pPic, pModel);
		if (bResult)
		{
			(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
			if ((static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.empty())
			{
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
			}
		}
		if (bResult) bResult = paperRecogMgrObj.RecogOMR(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogElectOmr(nPic, matCompPic, pPic, pModel);
	}
	pPic->nRecoged = 2;

	end_pic = clock();
	TRACE("试卷 %s, 识别总时间: %d\n", pPic->strPicName.c_str(), end_pic - start_pic);
	char szLog[MAX_PATH] = { 0 };
	sprintf_s(szLog, "试卷 %s, 识别总时间: %d\n", pPic->strPicName.c_str(), end_pic - start_pic);
	std::string strLog = paperRecogMgrObj.GetLog();
	strLog.append("------------------------------------------\n");
	strLog.append(szLog);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::HandleScanPicTask(pST_SCAN_PAPER pScanPaperTask)
{
#if 1
	clock_t sTime, eTime;
	sTime = clock();

	CAdjustPaperPic AdjustPaperObj;
	AdjustPaperObj.AdjustScanPaperToModel(pScanPaperTask);
	AdjustPaperObj.SaveScanPaperPic(pScanPaperTask);
	eTime = clock();
	std::stringstream ssLog;
	ssLog << "试卷(S" << pScanPaperTask->nPaperID << ")正反、方向调整及保存完成. " << (int)(eTime - sTime) << "\n";
	std::string strLog = AdjustPaperObj.GetLog();
	g_pLogger->information(strLog + ssLog.str());
#else
	clock_t sTime, mT0, mT1, mT2, mT3, eTime;
	sTime = clock();
	std::stringstream ssLog;
	ssLog << "检测试卷(S" << pScanPaperTask->nPaperID << ")的图像正反面:\n";

	if (pScanPaperTask->vecScanPic.size() <= 1)
	{
		ssLog << "这张试卷只有" << pScanPaperTask->vecScanPic.size() << "页图片, 不需要判断正反，直接判断旋转方向\n";
		if (pScanPaperTask->vecScanPic.size())
		{
			pST_SCAN_PIC pScanPic = pScanPaperTask->vecScanPic[0];

			clock_t sT, eT1, eT2;
			sT = clock();
			COmrRecog chkRotationObj;
			int nResult1 = chkRotationObj.GetRightPicOrientation(pScanPic->mtPic, 0, pScanPaperTask->bDoubleScan);
			eT1 = clock();
			ssLog << "新图像" << pScanPic->strPicName << "方向调整: " << nResult1 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

			imwrite(pScanPic->strPicPath, pScanPic->mtPic);
			eT2 = clock();
			ssLog << "新图像" << pScanPic->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
		}
		else
			ssLog << "错误：该试卷没有发现有扫描的图片信息\n";
	}
	else
	{
		COmrRecog chkRotationObj;
		bool bPreviousFirstPic = false;	//检测一张试卷时，上一页试卷是否为正面
		pST_SCAN_PIC pPreviousPic = NULL;
		for (int i = 0; i < pScanPaperTask->vecScanPic.size(); i++)
		{
			pST_SCAN_PIC pScanPic = pScanPaperTask->vecScanPic[i];

			//一张试卷的前一页是正面，或者已经是最后一页单独的试卷
			if (bPreviousFirstPic || (i == pScanPaperTask->vecScanPic.size() - 1 && floor(i / 2) * 2 == i))
			{
				if (bPreviousFirstPic)
				{
					bPreviousFirstPic = false;
					ssLog << "图像" << pScanPic->strPicName << "属于这张试卷的第2页(考生总试卷的第" << i + 1 << "面)，不需要检测正反面，上一页试卷是正面，开始旋转方向判断\n";
				}
				else if(i == pScanPaperTask->vecScanPic.size() - 1 && floor(i / 2) * 2 == i)
					ssLog << "这份试卷的最后一页" << pScanPic->strPicName << "(" << i + 1 << "/" << pScanPaperTask->vecScanPic.size() << "), 属于最后一张试卷只有单独一面的情况，不需要检测正反面. 开始旋转方向判断\n";

				clock_t sT, eT1, eT2;
				sT = clock();
				int nResult1 = chkRotationObj.GetRightPicOrientation(pScanPic->mtPic, i, pScanPaperTask->bDoubleScan);
				eT1 = clock();
				ssLog << "新图像" << pScanPic->strPicName << "方向调整: " << nResult1 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

				imwrite(pScanPic->strPicPath, pScanPic->mtPic);
				eT2 = clock();
				ssLog << "新图像" << pScanPic->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
				continue;
			}

			mT1 = clock();
			bool bResult = chkRotationObj.IsFirstPic(i, pScanPic->mtPic, _pModel_);
			ssLog << chkRotationObj.GetRecogLog();
			mT2 = clock();

			if (bResult)
			{
				int nFrontPage = floor(i / 2) * 2;
				if (i % 2 == 0)
				{
					//第1页为正面，第2页可以不需要判断
					pPreviousPic = pScanPic;
					bPreviousFirstPic = true;
					ssLog << "图像" << pScanPic->strPicName << "检测到属于这张试卷的正面, 下一页试卷不需要检测正反. 开始旋转方向判断. " << (int)(mT2 - mT1) << "ms\n";

					clock_t sT, eT1, eT2;
					sT = clock();
					int nResult1 = chkRotationObj.GetRightPicOrientation(pScanPic->mtPic, i, pScanPaperTask->bDoubleScan);
					eT1 = clock();
					ssLog << "新图像" << pScanPic->strPicName << "方向调整: " << nResult1 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

					imwrite(pScanPic->strPicPath, pScanPic->mtPic);
					eT2 = clock();
					ssLog << "新图像" << pScanPic->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
				}
				else
				{
					//第2页为正面
					ssLog << "图像" << pScanPic->strPicName << "检测到属于这张试卷的正面, 开始与上一页试卷" << pPreviousPic->strPicName << "调换. " << (int)(mT2 - mT1) << "ms\n";
					//图像重命名
					bool bPicsExchangeSucc = true;
					pST_SCAN_PIC pPic1 = pScanPic;
					pST_SCAN_PIC pPic2 = pPreviousPic;
				#if 1	//在这里，原图像数据还没有写到文件，直接交换图像存储的路径
					std::string strTmp = pPic1->strPicPath;
					pPic1->strPicPath = pPic2->strPicPath;
					pPic2->strPicPath = strTmp;
				#else
					try
					{
						Poco::File fPic1(pPic1->strPicPath);
						fPic1.renameTo(pPic1->strPicPath + "_tmp");

						Poco::File fPic2(pPic2->strPicPath);
						fPic2.renameTo(pPic1->strPicPath);

						fPic1.renameTo(pPic2->strPicPath);
					}
					catch (Poco::Exception &e)
					{
						bPicsExchangeSucc = false;
						std::string strErr = e.displayText();

						ssLog << "图像" << pScanPic->strPicName << "与" << pPreviousPic->strPicName << "调换重命名失败: " << strErr << "\n";
					}
				#endif
					mT3 = clock();

					if (bPicsExchangeSucc)
					{
						ssLog << "图像调换成功，开始重写原图. " << (int)(mT3 - mT2) << "ms\n";
						clock_t sT, eT1, eT2;
						sT = clock();

						bool bDoubleScan = pScanPaperTask->bDoubleScan;	//使用此功能必须是双面扫描。	 m_pModel->vecPaperModel.size() % 2 == 0 ? true : false;
						int nResult1 = chkRotationObj.GetRightPicOrientation(pScanPic->mtPic, nFrontPage, bDoubleScan);
						eT1 = clock();
						ssLog << "新图像" << pPic1->strPicName << "方向调整: " << nResult1 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

						std::string strPicPath1 = pPic2->strPicPath;
						//if (nResult1 >= 2 && nResult1 <= 4)
						//{
							imwrite(strPicPath1, pScanPic->mtPic);
							eT2 = clock();
							ssLog << "新图像" << pPic1->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
						//}

						sT = clock();
						int nResult2 = chkRotationObj.GetRightPicOrientation(pPreviousPic->mtPic, i, bDoubleScan);
						eT1 = clock();
						ssLog << "新图像" << pPic2->strPicName << "方向调整: " << nResult2 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

						std::string strPicPath2 = pPic1->strPicPath;
						//if (nResult2 >= 2 && nResult2 <= 4)
						//{
							imwrite(strPicPath2, pPreviousPic->mtPic);
							eT2 = clock();
							ssLog << "新图像" << pPic2->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
						//}

						bPreviousFirstPic = false;	//这张试卷的正反已经判断完了，重置

						//交换原始图像信息
						cv::Mat mtTmp = pPreviousPic->mtPic;
						pPreviousPic->mtPic = pScanPic->mtPic;
						pScanPic->mtPic = mtTmp;
					}
				}
			}
			else
			{
				pPreviousPic = pScanPic;
				bPreviousFirstPic = false;
				ssLog << "判断" << pScanPic->strPicName << "属于这张试卷的正面失败，不能确定为正面. " << (int)(mT2 - mT1) << "ms\n";
			}
		}
	}
	eTime = clock();
	ssLog << "判断考生(S" << pScanPaperTask->nPaperID << ")正反面结束(" << (int)(eTime - sTime) << "ms)\n";
	TRACE(ssLog.str().c_str());
	g_pLogger->information(ssLog.str());
#endif

	//识别页面
	if (_pModel_->nUsePagination)
	{
		RecogPagination(pScanPaperTask, _pModel_);
	}

	//++添加试卷
	pST_PaperInfo pCurrentPaper = NULL;
	for (int i = 0; i < pScanPaperTask->vecScanPic.size(); i++)
	{
		pST_SCAN_PIC pScanPic = pScanPaperTask->vecScanPic[i];
		CScanMgrDlg* pDlg = static_cast<CScanMgrDlg*>(pScanPic->pNotifyDlg);

		pST_PicInfo pPic = new ST_PicInfo;
		pPic->strPicName = pScanPic->strPicName;
		pPic->strPicPath = pScanPic->strPicPath;
		pPic->pSrcScanPic = pScanPic;
		if (pScanPaperTask->bDoubleScan)
		{
			pPic->nPicModelIndex = pScanPaperTask->nModelPaperID * 2 + i;	//设置图片是属于模板的第几页
			pPic->nPicOldModelIndex = pPic->nPicModelIndex;
		}
		else
		{
			pPic->nPicModelIndex = i;
			pPic->nPicOldModelIndex = pPic->nPicModelIndex;
		}
		std::stringstream ssTmp;
		ssTmp << "HandleScanPicTask: i = " << i << ", pPic->strPicName = " << pPic->strPicName << "\npScanPic->nOrder = " << pScanPic->nOrder
			<< ",pScanPaperTask->vecScanPic.size = " << pScanPaperTask->vecScanPic.size() << ",pScanPic->nStudentID = " << pScanPic->nStudentID;
		OutputDebugStringA(ssTmp.str().c_str());

		if (pScanPic->nOrder == 1)	//(pScanPic->nOrder + 1) % 2 == 0	第1、3、5...页的时候创建新的试卷信息，如果是多页模式时，每一张试卷创建一个考生信息，最后根据准考证号合并考生
		{
			char szStudentName[30] = { 0 };
			sprintf_s(szStudentName, "S%d", pScanPic->nStudentID);
			pCurrentPaper = new ST_PaperInfo;
			pCurrentPaper->nIndex = pScanPic->nStudentID;
			pCurrentPaper->strStudentInfo = szStudentName;
			pCurrentPaper->pModel = _pModel_;
			pCurrentPaper->pPapers = pScanPaperTask->pPapersInfo;
			pCurrentPaper->pSrcDlg = pScanPaperTask->nSrcDlgType == 1 ? pScanPic->pNotifyDlg : pDlg->GetScanMainDlg();		//m_pDlg;
			pCurrentPaper->lPic.push_back(pPic);
			pCurrentPaper->lSrcScanPaper.push_back(pScanPaperTask);

			if (!pScanPaperTask->bCanRecog)
				pCurrentPaper->nPaginationStatus = 0;	//没有识别到页码，不能参与识别，设置问题卷，人工确认后再识别
			else
				pCurrentPaper->nPaginationStatus = 1;	//识别完页码，可以识别，不能确定具体属于哪个学生(默认)

			static_cast<pPAPERSINFO>(pScanPaperTask->pPapersInfo)->fmlPaper.lock();
			static_cast<pPAPERSINFO>(pScanPaperTask->pPapersInfo)->lPaper.push_back(pCurrentPaper);
			static_cast<pPAPERSINFO>(pScanPaperTask->pPapersInfo)->fmlPaper.unlock();
		}
		else
		{
			pCurrentPaper->lPic.push_back(pPic);
		}
		pPic->pPaper = pCurrentPaper;

		if (pScanPaperTask->nSrcDlgType == 0)
		{
			pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
			pResult->bScanOK = false;
			pResult->nState = 1;			//标识正在扫描
			pResult->nPaperId = pScanPaperTask->nPaperID;
			pResult->nPicId = i + 1;
			pResult->pPaper = pCurrentPaper;
			pResult->matShowPic = pScanPic->mtPic.clone();
			pResult->strPicName = pScanPic->strPicName;
			pResult->strPicPath = pScanPic->strPicPath;
			pResult->strResult = "获得图像";
			pResult->strResult.append(pScanPic->strPicName);

			TRACE("%s\n", pResult->strResult.c_str());
			pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);
		}
	}
	//添加到识别任务列表
	if (_pModel_ && _pCurrExam_->nModel == 0 && _nScanAnswerModel_ != 2)	//网阅模式下的试卷才加入识别队列, 扫描主观题答案不加入识别
	{
		pRECOGTASK pTask = new RECOGTASK;
		pTask->pPaper = pCurrentPaper;
		g_lRecogTask.push_back(pTask);
	}
	return true;
}

bool CRecognizeThread::HandleTask(pRECOGTASK pTask)
{
	//进行任务识别
	pMODELINFO pModelInfo = NULL;
	std::map<pMODEL, pMODELINFO>::iterator it = _mapModel.find(pTask->pPaper->pModel);		//pTask->pModel
	if (it == _mapModel.end())
	{
		pModelInfo = new MODELINFO;
		_mapModel.insert(std::map<pMODEL, pMODELINFO>::value_type(pTask->pPaper->pModel, pModelInfo));	//pTask->pModel

		std::string strLog = "加载模板";
		g_pLogger->information(strLog);

		pModelInfo->pModel = pTask->pPaper->pModel;		//pTask->pModel;
	#if 0	//可以不需要
		bool bResult = LoadModel(pModelInfo);
		if (!bResult)
			return bResult;
	#endif
	}
	else
		pModelInfo = it->second;

	PaperRecognise(pTask->pPaper, pModelInfo);
	bool bRecogAllPic = true;
	for (auto objPic : pTask->pPaper->lPic)
	{
		if (objPic->nRecoged < 2)
		{
			bRecogAllPic = false;
			break;
		}
		if (objPic->pSrcScanPic)
		{
			objPic->pSrcScanPic->mtPic.release();	//释放已经识别过的图片内存，防止内存持续增加
		}
	}
	if (bRecogAllPic)
	{
		TRACE("----->设置试卷(%s)识别完成\n", pTask->pPaper->strStudentInfo.c_str());
		pTask->pPaper->bRecogComplete = true;
	}
	if(g_lScanPaperTask.size() == 0 && g_lRecogTask.size() == 0 && _nScanStatus_ != 1)	//整袋都识别完成后，合并扫描试卷
		MergeScanPaper(static_cast<pPAPERSINFO>(pTask->pPaper->pPapers), pModelInfo->pModel);

	return true;
}
void CRecognizeThread::sharpenImage1(const cv::Mat &image, cv::Mat &result)
{
	//创建并初始化滤波模板
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = _nSharpKernel_;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//对图像进行滤波
	cv::filter2D(image, result, image.depth(), kernel);
}

bool CRecognizeThread::LoadModel(pMODELINFO pModelInfo)
{
	USES_CONVERSION;
	for (int i = 0; i < pModelInfo->pModel->nPicNum; i++)
	{
		std::string strModelPicPath = g_strModelSavePath + "\\" + pModelInfo->pModel->strModelName + "\\" + pModelInfo->pModel->vecPaperModel[i]->strModelPicName;

		cv::Mat matSrc;
		try
		{
			matSrc = cv::imread(strModelPicPath);
		}
		catch (cv::Exception& exc)
		{
			std::string strLog = "打开文件失败: " + exc.msg;
			g_pLogger->information(strLog);		
			return false;
		}
#ifdef PIC_RECTIFY_TEST
		Mat dst;
		Mat rotMat;
		PicRectify(matSrc, dst, rotMat);
		Mat matImg;
		if (dst.channels() == 1)
			cvtColor(dst, matImg, CV_GRAY2BGR);
		else
			matImg = dst;
#else
		Mat matImg = matSrc;
#endif
		pModelInfo->vecMatSrc.push_back(matImg);
	}
	return true;
}

void CRecognizeThread::PaperRecognise(pST_PaperInfo pPaper, pMODELINFO pModelInfo)
{
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		clock_t start_pic, end_pic;
		start_pic = clock();

		if ((*itPic)->nRecoged)		//已经识别过，不再识别
			continue;

		int nPic = 0;
		if (pModelInfo->pModel->nUsePagination)
		{
			nPic = (*itPic)->nPicModelIndex;
			if (pModelInfo->pModel->nUsePagination && pPaper->nPaginationStatus == 0)
			{
				pPaper->bIssuePaper = true;
				(*itPic)->nRecoged = 2;			//先临时设置识别完成标识，后面在人工确认完成后需要重新识别
				continue;
			}
			}
		else
			nPic = i;

		(*itPic)->nRecoged = 1;
		if (nPic >= pModelInfo->pModel->vecPaperModel.size())
		{
			(*itPic)->nRecoged = 2;
			continue;
		}

		int nCount = pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.size() + pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.size()
			+ pModelInfo->pModel->vecPaperModel[nPic]->lPagination.size() + pModelInfo->pModel->vecPaperModel[nPic]->lABModel.size() + pModelInfo->pModel->vecPaperModel[nPic]->lCourse.size()
			+ pModelInfo->pModel->vecPaperModel[nPic]->lQK_CP.size() + pModelInfo->pModel->vecPaperModel[nPic]->lWJ_CP.size() + pModelInfo->pModel->vecPaperModel[nPic]->lGray.size()
			+ pModelInfo->pModel->vecPaperModel[nPic]->lWhite.size() + pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.size() + pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.size()
			+ pModelInfo->pModel->vecPaperModel[nPic]->lElectOmr.size();
		if (!nCount)	//如果当前模板试卷没有校验点就不需要进行试卷打开操作，直接下一张试卷
		{
			(*itPic)->nRecoged = 2;
			continue;
		}
		
		std::string strPicFileName = (*itPic)->strPicName;
		Mat matCompSrcPic;
		bool bOpenSucc = false;
		for (int j = 0; j < 3; j++)
		{
			if (!bOpenSucc)
			{
				try
				{
					Poco::File fPic(CMyCodeConvert::Gb2312ToUtf8((*itPic)->strPicPath));
					if (!fPic.exists())
						continue;
					matCompSrcPic = imread((*itPic)->strPicPath);			//imread((*itPic)->strPicPath);
					bOpenSucc = true;
					break;
				}
				catch (cv::Exception& exc)
				{
					Sleep(500);
				}
			}			
		}
		if (!bOpenSucc)
		{
			HandleWithErrPaper(pPaper);
			std::string strLog = "几次打开文件都失败2: " + (*itPic)->strPicPath;
			g_pLogger->information(strLog);

			(*itPic)->nRecoged = 2;
			continue;
		}
		
#ifdef PIC_RECTIFY_TEST	//图像旋转纠正测试
		Mat matDst;
		Mat matCompPic;
		Mat rotMat;
		PicRectify(matCompSrcPic, matDst, rotMat);
		if (matDst.channels() == 1)
			cvtColor(matDst, matCompPic, CV_GRAY2BGR);
		else
			matCompPic = matDst;
#else
		Mat matCompPic = matCompSrcPic;
#endif

		clock_t end1_pic = clock();

	#ifdef TEST_New_RecogClass
		if (!RecogPaper(nPic, matCompPic, *itPic, pModelInfo->pModel))
		{
			if (g_nOperatingMode != 1)
			{
				HandleWithErrPaper(pPaper);
				break;									//找到这张试卷有问题点，不进行下一张试卷的检测
			}
		}
		continue;
	#endif

		bool bFind = false;
		
		if (g_nOperatingMode == 1)
		{
			bool bResult;
			if(pModelInfo->pModel->nUseWordAnchorPoint)
				bResult = RecogCharacter(nPic, matCompPic, *itPic, pModelInfo);
			else
				bResult = RecogFixCP(nPic, matCompPic, *itPic, pModelInfo);

		#ifdef TEST_LostCorner
			RecogLostCorner(nPic, matCompPic, *itPic, pModelInfo);
		#endif

		#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			//bResult = PicTransfer(nPic, matCompPic, (*itPic)->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, inverseMat);
			bResult = GetFixPicTransfer(nPic, matCompPic, *itPic, pModelInfo->pModel, inverseMat);
		#endif

			bResult = RecogHHead(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogVHead(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogABModel(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogCourse(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogQKCP(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogWJCP(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogGrayCP(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogWhiteCP(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogSN(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogOMR(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogElectOmr(nPic, matCompPic, *itPic, pModelInfo);
		}
		else
		{
			bool bResult;
			if (pModelInfo->pModel->nUseWordAnchorPoint)
				bResult = RecogCharacter(nPic, matCompPic, *itPic, pModelInfo);
			else
				bResult = RecogFixCP(nPic, matCompPic, *itPic, pModelInfo);

// 			bool bResult = RecogFixCP(nPic, matCompPic, *itPic, pModelInfo);
// 
// 		#ifdef USE_TESSERACT
// 			bResult = RecogCharacter(nPic, matCompPic, *itPic, pModelInfo);
// 		#endif

		#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			//if (bResult) bResult = PicTransfer(nPic, matCompPic, (*itPic)->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, inverseMat);
			if (bResult) bResult = GetFixPicTransfer(nPic, matCompPic, *itPic, pModelInfo->pModel, inverseMat);
		#endif

			if(bResult) bResult = RecogHHead(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogVHead(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogABModel(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogCourse(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogQKCP(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogWJCP(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogGrayCP(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogWhiteCP(nPic, matCompPic, *itPic, pModelInfo);
			if (bResult) bResult = RecogSN(nPic, matCompPic, *itPic, pModelInfo);
			if (bResult) bResult = RecogOMR(nPic, matCompPic, *itPic, pModelInfo);
			if (bResult) bResult = RecogElectOmr(nPic, matCompPic, *itPic, pModelInfo);
			if(!bResult) bFind = true;
			if (bFind)
			{
				HandleWithErrPaper(pPaper);
				break;									//找到这张试卷有问题点，不进行下一张试卷的检测
			}
		}

		(*itPic)->nRecoged = 2;

		end_pic = clock();
		TRACE("试卷 %s 打开时间: %d, 识别总时间: %d\n", strPicFileName.c_str(), end1_pic - start_pic, end_pic - start_pic);
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "试卷 %s 打开时间: %d, 识别总时间: %d\n", strPicFileName.c_str(), end1_pic - start_pic, end_pic - start_pic);
		g_pLogger->information(szLog);
	}
#ifdef TEST_New_RecogClass
	return;
#endif

	//统计识别信息到试卷袋中
	pPAPERSINFO pPapers = static_cast<pPAPERSINFO>(pPaper->pPapers);
	pPapers->fmOmrStatistics.lock();
	pPapers->nOmrDoubt += pPaper->nOmrDoubt;			//统计试卷袋的Omr怀疑数量
	pPapers->nOmrNull += pPaper->nOmrNull;				//统计试卷袋的omr为空数量
	pPapers->nOmrSingleToMulti += pPaper->nOmrSingleToMulti;	//统计试卷袋的单选题识别成多选的数量
	if (pPaper->bHasOmrDoubt) pPapers->nOmrDoubtSnCounts++;		//统计试卷袋的存在omr怀疑的试卷数量
	if (pPaper->bHasOmrNull) pPapers->nOmrNullSnCounts++;		//统计试卷袋的存在omr为空的试卷数量
	if (pPaper->bHasSingleToMulti) pPapers->nSingleToMultiSnCounts++;	//统计试卷袋的存在omr单选识别成多选的试卷数量
	if (pPaper->bOmrIssue)	pPapers->nOmrIssueSnCounts++;		//统计属于Omr问题卷的考生数量
	pPapers->fmOmrStatistics.unlock();

#ifdef PrintRecogLog	//test log
	std::string strPaperLog = "试卷(";
	strPaperLog.append(pPaper->strStudentInfo);
	strPaperLog.append(")识别结果: ");

	int nNullCount = 0;
	int nDoubtCount = 0;
	int nEqualCount = 0;

	char szSN[100] = { 0 };
	sprintf_s(szSN, "SN(%s), ", pPaper->strSN.c_str());
	strPaperLog.append(szSN);

// 	int nOmrCount = 0;
// 	for (int k = 0; k < pModelInfo->pModel->vecPaperModel.size(); k++)
// 	{
// 		nOmrCount += pModelInfo->pModel->vecPaperModel[k]->lOMR2.size();
// 	}
// 	int nPapersCount = (static_cast<pPAPERSINFO>(pPaper->pPapers))->lPaper.size() + (static_cast<pPAPERSINFO>(pPaper->pPapers))->lIssue.size();
// 	char szStatisticsInfo[300] = { 0 };
// 	sprintf_s(szStatisticsInfo, "\n统计信息: omrDoubt = %.2f(%d/%d), omrNull = %.2f(%d/%d), zkzhNull = %.2f(%d/%d)\n", (float)(static_cast<pPAPERSINFO>(pPaper->pPapers))->nOmrDoubt / nOmrCount, (static_cast<pPAPERSINFO>(pPaper->pPapers))->nOmrDoubt, nOmrCount, \
// 			  (float)(static_cast<pPAPERSINFO>(pPaper->pPapers))->nOmrNull / nOmrCount, (static_cast<pPAPERSINFO>(pPaper->pPapers))->nOmrNull, nOmrCount, \
// 			  (float)(static_cast<pPAPERSINFO>(pPaper->pPapers))->nSnNull / nPapersCount, (static_cast<pPAPERSINFO>(pPaper->pPapers))->nSnNull, nPapersCount);
// 	strPaperLog.append(szStatisticsInfo);

	OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
	for (; itOmr != pPaper->lOmrResult.end(); itOmr++)
	{
		//数量统计
		if (itOmr->nDoubt == 0)
			nEqualCount++;
		else if (itOmr->nDoubt == 1)
			nDoubtCount++;
		else
			nNullCount++;

		char szSingle[10] = { 0 };
		if (itOmr->nSingle == 0)
			strcpy_s(szSingle, "单");
		else
			strcpy_s(szSingle, "多");
		
		int nPrintOmrVal = 0;
	#ifdef PrintAllOmrVal
		nPrintOmrVal = 1;
	#else
		nPrintOmrVal = itOmr->nDoubt;
	#endif
		std::string strItemLog;
		if (nPrintOmrVal)	//itOmr->nDoubt
		{
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				char szTmp[100] = { 0 }; 
				sprintf_s(szTmp, "%c,识别=%.3f(R/S=%.1f/%.1f),Succ:%d. ", itRect->nAnswer + 65, \
					itRect->fRealValuePercent, itRect->fRealValue, itRect->fStandardValue, itRect->fRealValuePercent > itRect->fStandardValuePercent);
				strItemLog.append(szTmp);
			}
			strItemLog.append("\n");
			RECTLIST::iterator itRect2 = itOmr->lSelAnswer.begin();
			for (; itRect2 != itOmr->lSelAnswer.end(); itRect2++)
			{
				char szTmp[200] = { 0 };
				sprintf_s(szTmp, "%c,密度=%.3f(%.3f/%.3f), ", itRect2->nAnswer + 65, \
						  itRect2->fRealDensity / itRect2->fStandardDensity, itRect2->fRealDensity, itRect2->fStandardDensity);
				strItemLog.append(szTmp);
			}
			strItemLog.append("\n");
			RECTLIST::iterator itRect3 = itOmr->lSelAnswer.begin();
			for (; itRect3 != itOmr->lSelAnswer.end(); itRect3++)
			{
				char szTmp[200] = { 0 };
				sprintf_s(szTmp, "%c,灰度=%.3f(%.3f-%.3f), ", itRect3->nAnswer + 65, \
						  itRect3->fRealMeanGray - itRect3->fStandardMeanGray, itRect3->fRealMeanGray, itRect3->fStandardMeanGray);
				strItemLog.append(szTmp);
			}
		}

		//++++++++	test	++++++++
		std::vector<pRECTINFO> vecItemsDensityDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDensityDiff;
		calcOmrDensityDiffVal(itOmr->lSelAnswer, vecItemsDensityDesc, vecOmrItemDensityDiff);
		strItemLog.append("\n[");
		for (int i = 0; i < vecOmrItemDensityDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.5f ", vecOmrItemDensityDiff[i].szVal, vecOmrItemDensityDiff[i].fDiff);
			strItemLog.append(szTmp);
		}
		strItemLog.append("]");

		float fDensityThreshold = 0.0;
		strItemLog.append("密度选中阀值:[");
		for (int i = 0; i < vecOmrItemDensityDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.5f ", vecOmrItemDensityDiff[i].szVal, _dDiffThread_Fix_ + fDensityThreshold * 0.5);
			strItemLog.append(szTmp);
			if ((vecOmrItemDensityDiff[i].fDiff >= _dDiffThread_Fix_ + fDensityThreshold * 0.5))
				fDensityThreshold += vecOmrItemDensityDiff[i].fDiff;
		}
		strItemLog.append("]");

		//test日志
		float fDensityMeanPer = 0.0;
		for (int i = 0; i < vecItemsDensityDesc.size(); i++)
			fDensityMeanPer += vecItemsDensityDesc[i]->fRealValuePercent;
		fDensityMeanPer = fDensityMeanPer / vecItemsDensityDesc.size();

		char szTmp2[40] = { 0 };
		sprintf_s(szTmp2, "密度平均值:%.3f, ", fDensityMeanPer);
		strItemLog.append(szTmp2);

		strItemLog.append("与密度平均值差值:[");
		for (int i = 0; i < vecItemsDensityDesc.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%c:%.5f ", vecItemsDensityDesc[i]->nAnswer + 65, vecItemsDensityDesc[i]->fRealValuePercent - fDensityMeanPer);
			strItemLog.append(szTmp);
		}
		strItemLog.append("]");

		std::vector<pRECTINFO> vecItemsGrayDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemGrayDiff;
		calcOmrGrayDiffVal(itOmr->lSelAnswer, vecItemsGrayDesc, vecOmrItemGrayDiff);
		strItemLog.append("\n[");
		for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.3f ", vecOmrItemGrayDiff[i].szVal, vecOmrItemGrayDiff[i].fDiff);
			strItemLog.append(szTmp);
		}
		strItemLog.append("]");

		float fMeanGrayDiff = 0.0;
		for (int i = 0; i < vecItemsGrayDesc.size(); i++)
		{
			fMeanGrayDiff += (vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray);
		}
		fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayDesc.size();
		char szTmp1[40] = { 0 };
		sprintf_s(szTmp1, "平均灰度差:%.3f, ", fMeanGrayDiff);
		strItemLog.append(szTmp1);
		strItemLog.append("灰度选中的阀值[");
		float fThreld = 0.0;
		float fGrayDiffLast = 0.0;		//对上一次判断选中的选项对下一个选项选中判断的增益
		for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
		{
			float fGrayThresholdGray = vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray - fMeanGrayDiff;
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.3f ", vecOmrItemGrayDiff[i].szVal, _dDiffThread_3_ + fGrayThresholdGray + fGrayDiffLast);
			strItemLog.append(szTmp);
			fGrayDiffLast += abs(fGrayThresholdGray) / 2;
		}
		strItemLog.append("]");
		//--------------------------
		
		char szOmrItem[3060] = { 0 };
		if (nPrintOmrVal)	//itOmr->nDoubt
			sprintf_s(szOmrItem, "%d(%s):%s[%s -- %s -- %s] Doubt(%d)\t==>%s\n", itOmr->nTH, szSingle, itOmr->strRecogVal.c_str(), itOmr->strRecogVal1.c_str(), itOmr->strRecogVal2.c_str(), itOmr->strRecogVal3.c_str(), itOmr->nDoubt, strItemLog.c_str());	//szItemInfo
// 		else
// 			sprintf_s(szOmrItem, "%d(%s):%s[%s -- %s -- %s] Doubt(%d)\n", itOmr->nTH, szSingle, itOmr->strRecogVal.c_str(), itOmr->strRecogVal1.c_str(), itOmr->strRecogVal2.c_str(), itOmr->strRecogVal3.c_str(), itOmr->nDoubt);

		strPaperLog.append(szOmrItem);
	}
	int nCount = pPaper->lOmrResult.size();
	if (nCount)
	{
		std::string strCorrectPerInfo = Poco::format("%s总数[%u],空值%d(%.2f%%),怀疑%d(%.2f%%),无怀疑%d(%.2f%%)", pPaper->strStudentInfo, pPaper->lOmrResult.size(), nNullCount, (double)nNullCount / nCount * 100, \
													 nDoubtCount, (double)nDoubtCount / nCount * 100, nEqualCount, (double)nEqualCount / nCount * 100);

		g_pLogger->information(strCorrectPerInfo);
	}
	g_pLogger->information(strPaperLog);
#endif
}

inline bool CRecognizeThread::Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	Mat matCompRoi;
	Rect rt = rc.rt;
	bool bResult = false;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matCompPic.cols)
		{
			rt.width = matCompPic.cols - rt.x;
		}
		if (rt.br().y > matCompPic.rows)
		{
			rt.height = matCompPic.rows - rt.y;
		}

 		matCompRoi = matCompPic(rt);

		Mat imag_src, img_comp;
		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);
		cv::GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//_nGauseKernel_
		SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

		const int channels[1] = { 0 };
		const float* ranges[1];
		const int histSize[1] = { 1 };
		float hranges[2];
#if 1	//for test

		if (rc.eCPType != WHITE_CP)
		{
			hranges[0] = g_nRecogGrayMin;
			hranges[1] = static_cast<float>(rc.nThresholdValue);
			ranges[0] = hranges;
		}
		else
		{
			hranges[0] = static_cast<float>(rc.nThresholdValue);
			hranges[1] = g_nRecogGrayMax_White;	//255		//256时可统计完全空白的点，即RGB值为255的完全空白点;255时只能统计到RGB为254的值，255的值统计不到
			ranges[0] = hranges;
		}
#else
		if (rc.eCPType != WHITE_CP && rc.eCPType != OMR)
		{
			hranges[0] = g_nRecogGrayMin;
			hranges[1] = static_cast<float>(rc.nThresholdValue);
			ranges[0] = hranges;
		}
		else if (rc.eCPType == OMR)
		{
			hranges[0] = g_nRecogGrayMin_OMR;
			hranges[1] = g_RecogGrayMax_OMR;
			ranges[0] = hranges;
		}
		else
		{
			hranges[0] = static_cast<float>(rc.nThresholdValue);
			hranges[1] = g_nRecogGrayMax_White;	//255		//256时可统计完全空白的点，即RGB值为255的完全空白点;255时只能统计到RGB为254的值，255的值统计不到
			ranges[0] = hranges;
		}
#endif
		MatND src_hist, comp_hist;
		cv::calcHist(&matCompRoi, 1, channels, Mat(), comp_hist, 1, histSize, ranges, false);

		double maxValComp = 0;
		double minValComp = 0;

		//找到直方图中的最大值和最小值
		cv::minMaxLoc(comp_hist, &minValComp, &maxValComp, 0, 0);
		int nSize = comp_hist.rows;

		// 设置最大峰值为图像高度的90%
		int hpt = static_cast<int>(0.9 * 256);		//histSize
		for (int h = 0; h < nSize; h++)	//histSize
		{
// 			float binValSrc = src_hist.at<float>(h);
// 			int intensitySrc = static_cast<int>(binValSrc*hpt / maxValSrc);
//			TRACE("h = %d. binValSrc = %f. intensitySrc = %d\n", h, binValSrc, intensitySrc);

			float binValComp = comp_hist.at<float>(h);
			int intensityComp = static_cast<int>(binValComp*hpt / maxValComp);

			rc.fRealValue = binValComp;
			rc.fRealArea = rc.rt.area();
			rc.fRealDensity = rc.fRealValue / rc.fRealArea;
			

			float fStandardCompare = (rc.fRealArea / rc.fStandardArea) * rc.fStandardValue;
			if (binValComp == 0 && rc.fStandardValue == 0)
				rc.fRealValuePercent = 1.0;
			else if (rc.fStandardValue == 0)
				rc.fRealValuePercent = 1.0;
			else
				rc.fRealValuePercent = binValComp / rc.fStandardValue;
			bResult = true;
			break;
		}

#if 1
		MatND mean;
		MatND stddev;
		meanStdDev(matCompRoi, mean, stddev);

		IplImage *src;
		src = &IplImage(mean);
		rc.fRealMeanGray = cvGetReal2D(src, 0, 0);

		IplImage *src2;
		src2 = &IplImage(stddev);
		rc.fRealStddev = cvGetReal2D(src2, 0, 0);
#else
		MatND src_hist2;
		const int histSize2[1] = { 256 };	//rc.nThresholdValue - g_nRecogGrayMin
		const float* ranges2[1];
		float hranges2[2];
		hranges2[0] = 0;
		hranges2[1] = 255;
		ranges2[0] = hranges2;
		cv::calcHist(&matCompRoi, 1, channels, Mat(), src_hist2, 1, histSize2, ranges2, true, false);
		int nCount = 0;
		for (int i = 0; i < 256; i++)
		{
			nCount += i * src_hist2.at<float>(i);
		}
		rc.fRealMeanGray = nCount / rc.fRealArea;
#endif
	}
	catch (cv::Exception &exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "CRecognizeThread::Recog error. detail: %s\n", exc.msg);
		//g_pLogger->information(szLog);
		TRACE(szLog);
	}
	
	return bResult;
}

bool CRecognizeThread::RecogCharacter(int nPic, cv::Mat & matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("图片%s\n", pPic->strPicName);
	if (pModelInfo->pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() == 0)
	{
		strLog = Poco::format("图片%s没有文字定位点需要识别", pPic->strPicName);
		g_pLogger->information(strLog);
		return true;
	}

#ifdef USE_TESSERACT
	if(NULL == m_pTess)
		InitCharacterRecog();
#endif

	CHARACTER_ANCHOR_AREA_LIST::iterator itBigRecogCharRt = pModelInfo->pModel->vecPaperModel[nPic]->lCharacterAnchorArea.begin();
	for (int i = 0; itBigRecogCharRt != pModelInfo->pModel->vecPaperModel[nPic]->lCharacterAnchorArea.end(); i++, itBigRecogCharRt++)
	{
		pST_CHARACTER_ANCHOR_AREA pstBigRecogCharRt = *itBigRecogCharRt;

		std::vector<Rect>RectCompList;
		try
		{
			if (pstBigRecogCharRt->rt.x < 0) pstBigRecogCharRt->rt.x = 0;
			if (pstBigRecogCharRt->rt.y < 0) pstBigRecogCharRt->rt.y = 0;
			if (pstBigRecogCharRt->rt.br().x > matCompPic.cols)
			{
				pstBigRecogCharRt->rt.width = matCompPic.cols - pstBigRecogCharRt->rt.x;
			}
			if (pstBigRecogCharRt->rt.br().y > matCompPic.rows)
			{
				pstBigRecogCharRt->rt.height = matCompPic.rows - pstBigRecogCharRt->rt.y;
			}

			Mat matCompRoi;
			matCompRoi = matCompPic(pstBigRecogCharRt->rt);		//真坐标

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			switch (pModelInfo->pModel->vecPaperModel[nPic]->nPicSaveRotation)
			{
				case 1:	break;
				case 3:			//图像需要调整到正常的视觉方向(作答方向)，则模板是右旋90，则实际图片需左旋90
					{
						cv::Mat dst;
						transpose(matCompRoi, dst);	//左旋90，镜像 
						flip(dst, matCompRoi, 0);	//左旋90，模板图像需要右旋90，原图即需要左旋90
					}
					break;
				case 2:			//图像需要调整到正常的视觉方向(作答方向)，则模板是左旋90，则实际图片需右旋90
					{
						cv::Mat dst;
						transpose(matCompRoi, dst);	//左旋90，镜像 
						flip(dst, matCompRoi, 1);	//右旋90，模板图像需要左旋90，原图即需要右旋90
					}
					break;
				case 4:
					{
						cv::Mat dst;
						transpose(matCompRoi, dst);	//左旋90，镜像 
						cv::Mat dst2;
						flip(dst, dst2, 1);
						cv::Mat dst5;
						transpose(dst2, dst5);
						flip(dst5, matCompRoi, 1);	//右旋180
					}
					break;
				default:
					break;
			}
			
			GaussianBlur(matCompRoi, matCompRoi, cv::Size(pstBigRecogCharRt->nGaussKernel, pstBigRecogCharRt->nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi, matCompRoi, pstBigRecogCharRt->nSharpKernel);

			//double dThread = threshold(matCompRoi, matCompRoi, pstBigRecogCharRt->nThresholdValue, 255, THRESH_OTSU | THRESH_BINARY);
			cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 19, 20);	//blockSize, constValue

			CCoordinationConvert convertObj(matCompPic);	//坐标转换对象
			cv::Rect rtShowRect = convertObj.GetShowFakePosRect(pstBigRecogCharRt->rt, pModelInfo->pModel->vecPaperModel[nPic]->nPicSaveRotation);
		#ifdef USE_TESSERACT
			m_pTess->SetImage((uchar*)matCompRoi.data, matCompRoi.cols, matCompRoi.rows, 1, matCompRoi.cols);

			std::string strWhiteList;
			for (auto itChar : pstBigRecogCharRt->vecCharacterRt)
				strWhiteList.append(itChar->strVal);
			m_pTess->SetVariable("tessedit_char_whitelist", CMyCodeConvert::Gb2312ToUtf8(strWhiteList).c_str());

			m_pTess->Recognize(0);

			tesseract::ResultIterator* ri = m_pTess->GetIterator();
			tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;	//RIL_WORD
			if (ri != 0)
			{
				int nIndex = 1;

				pST_CHARACTER_ANCHOR_AREA pstRecogCharacterRt = new ST_CHARACTER_ANCHOR_AREA();
				pstRecogCharacterRt->nIndex = pstBigRecogCharRt->nIndex;
				pstRecogCharacterRt->nThresholdValue = pstBigRecogCharRt->nThresholdValue;
				pstRecogCharacterRt->nGaussKernel = pstBigRecogCharRt->nGaussKernel;
				pstRecogCharacterRt->nSharpKernel = pstBigRecogCharRt->nSharpKernel;
				pstRecogCharacterRt->nCannyKernel = pstBigRecogCharRt->nCannyKernel;
				pstRecogCharacterRt->nDilateKernel = pstBigRecogCharRt->nDilateKernel;
				pstRecogCharacterRt->nCharacterConfidence = pstBigRecogCharRt->nCharacterConfidence;
				pstRecogCharacterRt->rt = pstBigRecogCharRt->rt;	//真坐标

				convertObj.SetPicRect(matCompPic.rows, matCompPic.cols);	//改变参照图像的大小，后面计算从假坐标到真坐标时，基准图像的大小已经变化

				//重复字临时登记列表，后面删除所有重复的字
				std::vector<std::string> vecRepeatWord;
				do
				{
					const char* word = ri->GetUTF8Text(level);
					float conf = ri->Confidence(level);
					if (word && strcmp(word, " ") != 0 && conf >= pstRecogCharacterRt->nCharacterConfidence)
					{
						int x1, y1, x2, y2;
						ri->BoundingBox(level, &x1, &y1, &x2, &y2);
						Point start, end;
						start.x = rtShowRect.x + x1;
						start.y = rtShowRect.y + y1;
						end.x = rtShowRect.x + x2;
						end.y = rtShowRect.y + y2;
						Rect rtSrc(start, end);		//假坐标

						pST_CHARACTER_ANCHOR_POINT pstCharRt = new ST_CHARACTER_ANCHOR_POINT();
						pstCharRt->nIndex = nIndex;
						pstCharRt->fConfidence = conf;
						pstCharRt->rc.eCPType = CHARACTER_AREA;
						pstCharRt->rc.rt = convertObj.GetSrcSaveRect(rtSrc, pModelInfo->pModel->vecPaperModel[nPic]->nPicSaveRotation);
						pstCharRt->rc.nTH = pstRecogCharacterRt->nIndex;	//记录下当前文字属于第几个大文字识别区
						pstCharRt->rc.nAnswer = nIndex;						//记录下当前文字属于当前文字识别区中的第几个识别的文字

						pstCharRt->rc.nThresholdValue = pstBigRecogCharRt->nThresholdValue;
						pstCharRt->rc.nGaussKernel = pstBigRecogCharRt->nGaussKernel;
						pstCharRt->rc.nSharpKernel = pstBigRecogCharRt->nSharpKernel;
						pstCharRt->rc.nCannyKernel = pstBigRecogCharRt->nCannyKernel;
						pstCharRt->rc.nDilateKernel = pstBigRecogCharRt->nDilateKernel;

						pstCharRt->strVal = CMyCodeConvert::Utf8ToGb2312(word);

						//**********	需要删除所有重复的字，保证识别的文字没有重复字
						for (auto item : pstRecogCharacterRt->vecCharacterRt)
							if (item->strVal == pstCharRt->strVal)
							{
								vecRepeatWord.push_back(item->strVal);
								break;
							}

						pstRecogCharacterRt->vecCharacterRt.push_back(pstCharRt);
						nIndex++;
					}
				} while (ri->Next(level));
				
				//要做字的重复检查，去除重复的字，即统一识别区中，不能存在重复的字，否则可能影响取字
				for (int i = 0; i < vecRepeatWord.size(); i++)
				{
					std::vector<pST_CHARACTER_ANCHOR_POINT>::iterator itCharAnchorPoint = pstRecogCharacterRt->vecCharacterRt.begin();
					for (; itCharAnchorPoint != pstRecogCharacterRt->vecCharacterRt.end(); )
						if ((*itCharAnchorPoint)->strVal == vecRepeatWord[i])
						{
							pST_CHARACTER_ANCHOR_POINT pstCharRt = *itCharAnchorPoint;
							itCharAnchorPoint = pstRecogCharacterRt->vecCharacterRt.erase(itCharAnchorPoint);
							SAFE_RELEASE(pstCharRt);
						}
						else
							itCharAnchorPoint++;
				}

				if (pstRecogCharacterRt->vecCharacterRt.size() > 0)
				{
					pPic->lCharacterAnchorArea.push_back(pstRecogCharacterRt);

					int nSize = pstRecogCharacterRt->vecCharacterRt.size();
					if (nSize > 2)
					{
						//检查当前识别区中距离最远的两个点
						int nX_min = 0, nX_Max = 0;				//X轴的最小值、最大值
						int nX_minIndex = 0, nX_maxIndex = 0;	//X轴的最小值、最大值对应的标签，即第几个
						int nY_min = 0, nY_Max = 0;				//Y轴的最小值、最大值
						int nY_minIndex = 0, nY_maxIndex = 0;	//Y轴的最小值、最大值对应的标签，即第几个
						for (int i = 0; i < nSize; i++)
						{
							if (nX_min > pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.x)
							{
								nX_min = pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.x;
								nX_minIndex = i;
							}
							if (nX_Max < pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.x)
							{
								nX_Max = pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.x;
								nX_maxIndex = i;
							}
							if (nY_min > pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.y)
							{
								nY_min = pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.y;
								nY_minIndex = i;
							}
							if (nY_Max < pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.y)
							{
								nY_Max = pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.y;
								nY_maxIndex = i;
							}
						}
						int nXDist = nX_Max - nX_min;
						int nYDist = nY_Max - nY_min;
						if (nXDist > nYDist)
						{
							pstRecogCharacterRt->arryMaxDist[0] = nX_minIndex;
							pstRecogCharacterRt->arryMaxDist[1] = nX_maxIndex;
						}
						else
						{
							pstRecogCharacterRt->arryMaxDist[0] = nY_minIndex;
							pstRecogCharacterRt->arryMaxDist[1] = nY_maxIndex;
						}

// 						std::vector<pST_CHARACTER_ANCHOR_POINT> vecTmpCharacterRt(pstRecogCharacterRt->vecCharacterRt);
// 						std::sort(vecTmpCharacterRt.begin(), vecTmpCharacterRt.end(), [](pST_CHARACTER_ANCHOR_POINT st1, pST_CHARACTER_ANCHOR_POINT st2)
// 						{
// 							return st1->rc.rt.x < st2->rc.rt.x;
// 						});
// 						int nXStart, nXEnd;
// 						nXStart = vecTmpCharacterRt[0]->nIndex;
// 						nXEnd = vecTmpCharacterRt[nSize - 1]->nIndex;
// 						int nXDist = abs(vecTmpCharacterRt[nSize - 1]->rc.rt.x - vecTmpCharacterRt[0]->rc.rt.x); //X轴最远距离
// 
// 						std::sort(vecTmpCharacterRt.begin(), vecTmpCharacterRt.end(), [](pST_CHARACTER_ANCHOR_POINT st1, pST_CHARACTER_ANCHOR_POINT st2)
// 						{
// 							return st1->rc.rt.y > st2->rc.rt.y;
// 						});
// 						int nYStart, nYEnd;
// 						nYStart = vecTmpCharacterRt[0]->nIndex;
// 						nYEnd = vecTmpCharacterRt[nSize - 1]->nIndex;
// 						int nYDist = abs(vecTmpCharacterRt[nSize - 1]->rc.rt.x - vecTmpCharacterRt[0]->rc.rt.x); //X轴最远距离
// 
// 						if (nXDist > nYDist)
// 						{
// 							pstBigRecogCharRt->arryMaxDist[0] = nXStart;
// 							pstBigRecogCharRt->arryMaxDist[1] = nXEnd;
// 						}
// 						else
// 						{
// 							pstBigRecogCharRt->arryMaxDist[0] = nYStart;
// 							pstBigRecogCharRt->arryMaxDist[1] = nYEnd;
// 						}
					}
					else if(nSize == 2)
					{
						pstRecogCharacterRt->arryMaxDist[0] = pstRecogCharacterRt->vecCharacterRt[0]->nIndex;
						pstRecogCharacterRt->arryMaxDist[1] = pstRecogCharacterRt->vecCharacterRt[1]->nIndex;
					}
					else
					{
						pstRecogCharacterRt->arryMaxDist[0] = pstRecogCharacterRt->vecCharacterRt[0]->nIndex;
						pstRecogCharacterRt->arryMaxDist[1] = -1;
					}
				}
				else
					SAFE_RELEASE(pstRecogCharacterRt);
			}
		#endif
		}
		catch (...)
		{
			std::string strLog2 = Poco::format("识别文字定位区域(%d)异常\n", i);
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

// 			pPic->bFindIssue = true;
// 			pPic->lIssueRect.push_back(stBigRecogCharRt);
			if (g_nOperatingMode == 2)
			{
				bResult = false;						//找到问题点
				break;
			}
		}

	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别文字定位区域失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();

	std::string strRecogCharacter;
	for (auto item : pPic->lCharacterAnchorArea)
	{
		for (auto item2 : item->vecCharacterRt)
			strRecogCharacter.append(item2->strVal);
		strRecogCharacter.append("\t");
	}
	std::string strTime = Poco::format("识别文字定位区域时间: %dms, 识别文字: %s\n", (int)(end - start), strRecogCharacter);
	strLog.append(strTime);

#if 1	 //重置定点，在识别出来的文字中选2个作为定点
	if (!GetPicFix(nPic, pPic, pModelInfo->pModel))
	{
		std::string strGetAnchorPoint = "\n获取图片的文字定位点失败\n";
		strLog.append(strGetAnchorPoint);
	}
	cv::Mat matTmp = matCompPic.clone();
	for(auto item : pPic->lFix)
		cv::rectangle(matTmp, item.rt, CV_RGB(255, 0, 0), 2);
#endif
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
// 	if (pModelInfo->pModel->nHasHead != 0)	//有同步头的，不需要进行定点识别
// 		return bResult;

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.begin();
	for (int i = 0; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.end(); i++, itCP++)
	{
		RECTINFO rc = *itCP;

		std::vector<Rect>RectCompList;
		try
		{
		#if 1
			float fModelW = pModelInfo->pModel->vecPaperModel[nPic]->nPicW;
			float fModelH = pModelInfo->pModel->vecPaperModel[nPic]->nPicH;
			int nRealW = matCompPic.cols;
			int nRealH = matCompPic.rows;

			cv::Rect rtTmp;
			rtTmp.x = rc.rt.x * nRealW / fModelW;
			rtTmp.y = rc.rt.y * nRealH / fModelH;
			rtTmp.width = rc.rt.width * nRealW / fModelW;
			rtTmp.height = rc.rt.height * nRealH / fModelH;

			if (rtTmp.x < 0) rtTmp.x = 0;
			if (rtTmp.y < 0) rtTmp.y = 0;
			if (rtTmp.br().x > matCompPic.cols)
			{
				rtTmp.width = matCompPic.cols - rtTmp.x;
			}
			if (rtTmp.br().y > matCompPic.rows)
			{
				rtTmp.height = matCompPic.rows - rtTmp.y;
			}
			cv::Mat matCompRoi;
			matCompRoi = matCompPic(rtTmp);
		#else
			if (rc.rt.x < 0) rc.rt.x = 0;
			if (rc.rt.y < 0) rc.rt.y = 0;
			if (rc.rt.br().x > matCompPic.cols)
			{
				rc.rt.width = matCompPic.cols - rc.rt.x;
			}
			if (rc.rt.br().y > matCompPic.rows)
			{
				rc.rt.height = matCompPic.rows - rc.rt.y;
			}
			cv::Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);
		#endif

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

			int nRealThreshold = 150;
			RECTLIST::iterator itFix = pModelInfo->pModel->vecPaperModel[nPic]->lFix.begin();
			for (int j = 0; itFix != pModelInfo->pModel->vecPaperModel[nPic]->lFix.end(); j++, itFix++)
			{
				if (j == i)
				{
					nRealThreshold = itFix->nThresholdValue;
					break;
				}
			}

#ifdef USES_GETTHRESHOLD_ZTFB
			const int channels[1] = { 0 };
			const int histSize[1] = { nRealThreshold };	//150
			float hranges[2] = { 0, nRealThreshold };	//150
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = nRealThreshold;		//150
			if (nCount > 0)
			{
				float fMean = (float)nSum / nCount;		//均值

				for (int h = 0; h < hist.rows; h++)	//histSize
				{
					float binVal = hist.at<float>(h);

					nDevSum += pow(h - fMean, 2)*binVal;
				}
				float fStdev = sqrt(nDevSum / nCount);
				nThreshold = fMean + 2 * fStdev;
				if (fStdev > fMean)
					nThreshold = fMean + fStdev;
			}		

			if (nThreshold > nRealThreshold) nThreshold = nRealThreshold;	//150
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别	Size(_nDilateKernel_, _nDilateKernel_)
			dilate(matCompRoi, matCompRoi, element);

#if 1
			//		std::vector<std::vector<cv::Point> > vecContours;		//轮廓信息存储
			//		m_vecContours.clear();
			//		std::vector<cv::Mat> vecContours;
			//		cv::findContours(matCompRoi.clone(), vecContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);	//hsvRe.clone()		//CV_RETR_EXTERNAL	//CV_CHAIN_APPROX_SIMPLE

			// 		vector<Mat> vecContours;
			// 		Mat hierarchy;
			// 		findContours(matCompRoi, vecContours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
			// 
			// 		std::vector<Rect>RectCompList;
			// 		for (int i = 0; i < vecContours.size(); i++)
			// 		{
			// 			cv::Mat matTmp = cv::Mat(vecContours[i]);
			// 			Rect rm = cv::boundingRect(matTmp);
			// 
			// 			// 			Rect rm = cv::boundingRect(vecContours[i]);
			// 			// 			vecContours[i].release();
			// 
			// 			RectCompList.push_back(rm);
			// 			matTmp.release();
			// 		}

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm = rm + rtTmp.tl();	// rc.rt.tl();
				RectCompList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);
#else
			//		std::vector<std::vector<cv::Point> > vecContours;		//轮廓信息存储
			m_vecContours.clear();
			//		std::vector<cv::Mat> vecContours;
			cv::findContours(matCompRoi.clone(), m_vecContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);	//hsvRe.clone()		//CV_RETR_EXTERNAL	//CV_CHAIN_APPROX_SIMPLE

			//		bool bResult = false;
			std::vector<Rect>RectCompList;
			for (int i = 0; i < m_vecContours.size(); i++)
			{
				Rect rm = cv::boundingRect(cv::Mat(m_vecContours[i]));
				//			Rect rm = cv::boundingRect(cv::Mat(vecContours[i]));

				// 			std::vector<cv::Point> v;
				// 			m_vecContours[i].swap(v);
				// 			if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50 || rm.area() < 70)
				// 			{
				// 				TRACE("抛弃不符合条件的矩形块,(%d, %d, %d, %d)\n", rm.x, rm.y, rm.width, rm.height);
				// 				continue;
				// 			}
				RectCompList.push_back(rm);

				//			m_vecContours[i].clear();
				//			bResult = true;
			}
#endif
		}
		catch (cv::Exception& exc)
		{
			std::string strLog2 = Poco::format("识别定点(%d)异常: %s\n", i, exc.msg.c_str());
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc); 
			if (g_nOperatingMode == 2)
			{
				bResult = false;						//找到问题点
				break;
			}
		}

		std::string strLog2;	//临时日志，记录矩形具体识别结果
		bool bFindRect = false;
		if(RectCompList.size() == 0)
			bFindRect = true;
		else
		{
			std::sort(RectCompList.begin(), RectCompList.end(), SortByArea);
			Rect& rtFix = RectCompList[0];

			RECTINFO rcFix;
			RECTLIST::iterator itFix = pModelInfo->pModel->vecPaperModel[nPic]->lFix.begin();
			for (int j = 0; itFix != pModelInfo->pModel->vecPaperModel[nPic]->lFix.end(); j++, itFix++)
			{
				if (j == i)
				{
					rcFix = *itFix;
					break;
				}
			}

			bool bOnlyOne = false;		//只有一个矩形，需要判断面积和灰度，但是比例可以降低
			bool bFind = false;
			
			//通过灰度值来判断
			for (int k = 0; k < RectCompList.size(); k++)
			{
				RECTINFO rcTmp = rcFix;
				rcTmp.rt = RectCompList[k];

				//根据定点左上点与右下点位置判断是否在试卷的边线上，如果在，则可能是折角或者边上有损坏
				cv::Point pt1 = RectCompList[k].tl();
				cv::Point pt2 = RectCompList[k].br();
				int nDiff = 4;	//与图像边界的距离间隔在这个值之内，认为属于边界线上
				if (pt1.x < nDiff || pt1.y < nDiff || matCompPic.cols - pt2.x < nDiff || matCompPic.rows - pt2.y < nDiff)
				{
					TRACE("矩形(%d,%d,%d,%d)位置距离边线太近，可能是折角或损坏\n", RectCompList[k].x, RectCompList[k].y, RectCompList[k].width, RectCompList[k].height);
					continue;
				}

				Recog(nPic, rcTmp, matCompPic, pPic, pModelInfo);
				float fArea = rcTmp.fRealArea / rcTmp.fStandardArea;
				float fDensity = rcTmp.fRealDensity / rcTmp.fStandardDensity;
				float fWper = (float)rcTmp.rt.width / rcFix.rt.width;			//查找的矩形的宽度与模板对应定点的宽度之比
				float fHper = (float)rcTmp.rt.height / rcFix.rt.height;			//查找的矩形的宽度与模板对应定点的高度之比
				std::string strTmpLog = Poco::format("第%d个矩形:area=%f, Density=%f\t", k, (double)fArea, (double)fDensity);
				strLog2.append(strTmpLog);
				if ((bOnlyOne && fArea > 0.4 && fArea < 2.5 && fDensity > rcTmp.fStandardValuePercent * 0.9 && fWper < 2.0 && fWper > 0.4 && fHper < 2.0 && fHper > 0.4) || \
					(fArea > 0.5 && fArea < 2.0 && fDensity > rcTmp.fStandardValuePercent && fWper < 2.0 && fWper > 0.4 && fHper < 2.0 && fHper > 0.4))	//fArea > 0.7 && fArea < 1.5 && fDensity > 0.6
				{
					bFind = true;
					rtFix = RectCompList[k];
					break;
				}
			}
			
			if (!bFind)
				bFindRect = true;
			else
			{
				// 			rtFix.x = rtFix.x + rc.rt.x;
				// 			rtFix.y = rtFix.y + rc.rt.y;

				RECTINFO rcFixInfo = rc;
				rcFixInfo.nTH = i;			//这是属于模板上定点列表的第几个
				rcFixInfo.rt = rtFix;
				pPic->lFix.push_back(rcFixInfo);
				TRACE("定点矩形: (%d,%d,%d,%d)\n", rtFix.x, rtFix.y, rtFix.width, rtFix.height);
			}
		}
		if (bFindRect)
		{
			std::string strLog3 = Poco::format("识别定点(%d)失败 -- %s\n", i, strLog2);
			strLog.append(strLog3);
			bResult = false;						//找到问题点
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
			if (g_nOperatingMode == 2)
				break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别定点失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别定点时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	if (pModelInfo->pModel->nHasHead == 0)
		return true;

	std::string strErrDesc;
	m_vecH_Head.clear();
	RECTLIST::iterator itRoi = pModelInfo->pModel->vecPaperModel[nPic]->lSelHTracker.begin();
	for (; itRoi != pModelInfo->pModel->vecPaperModel[nPic]->lSelHTracker.end(); itRoi++)
	{
		RECTINFO rc = *itRoi;
#if 1
		std::vector<Rect>RectCompList;
		try
		{
			if (rc.rt.x < 0) rc.rt.x = 0;
			if (rc.rt.y < 0) rc.rt.y = 0;
			if (rc.rt.br().x > matCompPic.cols)
			{
				rc.rt.width = matCompPic.cols - rc.rt.x;
			}
			if (rc.rt.br().y > matCompPic.rows)
			{
				rc.rt.height = matCompPic.rows - rc.rt.y;
			}

			Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

#ifdef USES_GETTHRESHOLD_ZTFB
			const int channels[1] = { 0 };
			const int histSize[1] = { 150 };
			float hranges[2] = { 0, 150 };
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = 150;
			if (nCount > 0)
			{
				float fMean = (float)nSum / nCount;		//均值

				for (int h = 0; h < hist.rows; h++)	//histSize
				{
					float binVal = hist.at<float>(h);

					nDevSum += pow(h - fMean, 2)*binVal;
				}
				float fStdev = sqrt(nDevSum / nCount);
				nThreshold = fMean + 2 * fStdev;
				if (fStdev > fMean)
					nThreshold = fMean + fStdev;
			}
			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			//模板图像的水平同步头平均长宽
			RECTLIST::iterator itBegin = pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.begin();
			RECTINFO rcFist = *itBegin;
			RECTINFO rcSecond = *(++itBegin);
			
			int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
			int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;
#if 0	//test
			float fOffset = 0.2;
			nMid_minW = rcSecond.rt.width * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_maxW = rcSecond.rt.width * (1 + fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_minH = rcSecond.rt.height * (1 - fOffset);		//同上
			nMid_maxH = rcSecond.rt.height * (1 + fOffset);		//同上

			nHead_minW = rcFist.rt.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nHead_maxW = rcFist.rt.width * (1 + fOffset);		//同上
			nHead_minH = rcFist.rt.height * (1 - fOffset);		//同上
			nHead_maxH = rcFist.rt.height * (1 + fOffset);		//同上
#else
			if (pModelInfo->pModel->nType == 1)
			{
				int nMid_modelW = rcSecond.rt.width;
				int nMid_modelH = rcSecond.rt.height;
				int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
				nMidInterW = 3;
				nMidInterH = 3;
				nHeadInterW = 4;
				nHeadInterH = 4;
				nMid_minW = nMid_modelW - nMidInterW;
				nMid_maxW = nMid_modelW + nMidInterW;
				nMid_minH = nMid_modelH - nMidInterH;
				nMid_maxH = nMid_modelH + nMidInterH;

				nHead_minW = rcFist.rt.width - nHeadInterW;
				nHead_maxW = rcFist.rt.width + nHeadInterW;
				nHead_minH = rcFist.rt.height - nHeadInterH;
				nHead_maxH = rcFist.rt.height + nHeadInterH;

// 				float fOffset = 0.1;
// 				float fPer_W, fPer_H;	//模板第二个点与第一个点的宽、高的比例，用于最小值控制
// 				fPer_W = 0.5;
// 				fPer_H = 0.25;
// 				int nMid_modelW = rcSecond.rt.width + 2;		//加2是因为制卷模板框框没有经过查边框运算，经过查边框后，外框会包含整个矩形，需要加上上下各1个单位的线宽
// 				int nMid_modelH = rcSecond.rt.height + 2;
// 				if (nMid_modelW < rcFist.rt.width * fPer_W + 0.5)	nMid_modelW = rcFist.rt.width * fPer_W + 0.5;
// 				if (nMid_modelH < rcFist.rt.height * fPer_H + 0.5)	nMid_modelH = rcFist.rt.height * fPer_H + 0.5;
// 				nMid_minW = nMid_modelW * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 				nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 				nMid_minH = nMid_modelH * (1 - fOffset);				//同上
// 				nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//同上
// 
// 				nHead_minW = rcFist.rt.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 				nHead_maxW = rcFist.rt.width * (1 + fOffset * 4) + 0.5;		//同上
// 				nHead_minH = rcFist.rt.height * (1 - fOffset);				//同上
// 				nHead_maxH = rcFist.rt.height * (1 + fOffset * 4) + 0.5;	//同上
			}
			else
			{
				float fOffset_mid = 0.4;
				nMid_minW = rcSecond.rt.width * (1 - fOffset_mid);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nMid_maxW = rcSecond.rt.width * (1 + fOffset_mid);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nMid_minH = rcSecond.rt.height * (1 - fOffset_mid);		//同上
				nMid_maxH = rcSecond.rt.height * (1 + fOffset_mid);		//同上

				float fOffset_Head = 0.3;
				nHead_minW = rcFist.rt.width * (1 - fOffset_Head);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nHead_maxW = rcFist.rt.width * (1 + fOffset_Head);		//同上
				nHead_minH = rcFist.rt.height * (1 - fOffset_Head);		//同上
				nHead_maxH = rcFist.rt.height * (1 + fOffset_Head);		//同上
			}
#endif

			int nYSum = 0;
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm.x = rm.x + rc.rt.x;
				rm.y = rm.y + rc.rt.y;
				
				if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
				{
					if (!(rm.width > nHead_minW && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//排除第一个或最后一个大的同步头
					{
						TRACE("过滤水平同步头(%d,%d,%d,%d), 要求范围W:[%d,%d], H[%d,%d], 参考大小(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
						continue;
					}
					else
					{
						TRACE("首尾水平同步头(即定位点同步头)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
					}
				}
				RectCompList.push_back(rm);
				nYSum += rm.y;
			}
			cvReleaseMemStorage(&storage);
//			int nYMean = nYSum / RectCompList.size();
		}
		catch (cv::Exception& exc)
		{
			std::string strLog = "识别水平同步头异常: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;
			break;
		}
		if (RectCompList.size() == 0)
		{
			bResult = false;
			strErrDesc = "水平同步头数量为0.";
		}
		else
		{
			//二次过滤
			std::sort(RectCompList.begin(), RectCompList.end(), SortByPositionX2);
			std::vector<Rect>::iterator itHead = RectCompList.begin();
			for (int i = 0; itHead != RectCompList.end(); i++)
			{
				RECTINFO rcHead1;
				RECTLIST::iterator itModelHead = pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.begin();
				for (int j = 0; itModelHead != pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.end(); j++, itModelHead++)
				{
					if (j == i)
					{
						rcHead1 = *itModelHead;
						break;
					}
				}
				RECTINFO rcTmp = rcHead1;
				rcTmp.rt = RectCompList[i];
				Recog(nPic, rcTmp, matCompPic, pPic, pModelInfo);
				if (rcTmp.fRealArea / rcTmp.fStandardArea < 0.75 || rcTmp.fRealDensity / rcTmp.fStandardDensity < 0.8)
				{
					itHead = RectCompList.erase(itHead);
					i = i - 1;
				}
				else
					itHead++;
			}
			for (int i = 0; i < RectCompList.size(); i++)
			{
				RECTINFO rcHead;
				rcHead.rt = RectCompList[i];
				
				rcHead.nGaussKernel = rc.nGaussKernel;
				rcHead.nSharpKernel = rc.nSharpKernel;
				rcHead.nCannyKernel = rc.nCannyKernel;
				rcHead.nDilateKernel = rc.nDilateKernel;

				rcHead.eCPType = H_HEAD;
				m_vecH_Head.push_back(rcHead);
				//++ for test
				pPic->lNormalRect.push_back(rcHead);
				//--
			}
			std::sort(m_vecH_Head.begin(), m_vecH_Head.end(), SortByPositionX);
		}
		if(m_vecH_Head.size() != pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.size())
		{
			bResult = false;
			pPic->bFindIssue = true;
			for(int i = 0; i < m_vecH_Head.size(); i++)
				pPic->lIssueRect.push_back(m_vecH_Head[i]);

			strErrDesc = Poco::format("水平同步头数量为%u, 与模板水平同步头数量(%u)不一致", m_vecH_Head.size(), pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.size());
		}
#else
		GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);	//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic].lFix, rc.rt);
		bool bFindRect = Recog(nPic, rc, matCompPic, pPic, pModelInfo);

		if (bFindRect)
		{
			bResult = false;						//找到问题点
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
		else
			pPic->lNormalRect.push_back(rc);
#endif
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别水平同步头失败, 原因: %s, 图片名: %s\n", strErrDesc.c_str(), pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	if (pModelInfo->pModel->nHasHead == 0)
		return true;

	std::string strErrDesc;
	m_vecV_Head.clear();
	RECTLIST::iterator itRoi = pModelInfo->pModel->vecPaperModel[nPic]->lSelVTracker.begin();
	for (; itRoi != pModelInfo->pModel->vecPaperModel[nPic]->lSelVTracker.end(); itRoi++)
	{
		RECTINFO rc = *itRoi;
#if 1
		std::vector<Rect>RectCompList;
		try
		{
			if (rc.rt.x < 0) rc.rt.x = 0;
			if (rc.rt.y < 0) rc.rt.y = 0;
			if (rc.rt.br().x > matCompPic.cols)
			{
				rc.rt.width = matCompPic.cols - rc.rt.x;
			}
			if (rc.rt.br().y > matCompPic.rows)
			{
				rc.rt.height = matCompPic.rows - rc.rt.y;
			}

			Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//_nGauseKernel_
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

#ifdef USES_GETTHRESHOLD_ZTFB
			const int channels[1] = { 0 };
			const int histSize[1] = { 150 };
			float hranges[2] = { 0, 150 };
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = 150;
			if (nCount > 0)
			{
				float fMean = (float)nSum / nCount;		//均值

				for (int h = 0; h < hist.rows; h++)	//histSize
				{
					float binVal = hist.at<float>(h);

					nDevSum += pow(h - fMean, 2)*binVal;
				}
				float fStdev = sqrt(nDevSum / nCount);
				nThreshold = fMean + 2 * fStdev;
				if (fStdev > fMean)
					nThreshold = fMean + fStdev;
			}
			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

#if 1
			//模板图像的水平同步头平均长宽
			RECTLIST::iterator itBegin = pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.begin();
			RECTINFO rcFist = *itBegin;
			RECTINFO rcSecond = *(++itBegin);

			int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
			int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;
#if 0	//test
			float fOffset = 0.2;
			nMid_minW = rcSecond.rt.width * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_maxW = rcSecond.rt.width * (1 + fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_minH = rcSecond.rt.height * (1 - fOffset);		//同上
			nMid_maxH = rcSecond.rt.height * (1 + fOffset);		//同上

			nHead_minW = rcFist.rt.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nHead_maxW = rcFist.rt.width * (1 + fOffset);		//同上
			nHead_minH = rcFist.rt.height * (1 - fOffset);		//同上
			nHead_maxH = rcFist.rt.height * (1 + fOffset);		//同上
#else
			if (pModelInfo->pModel->nType == 1)
			{
				int nMid_modelW = rcSecond.rt.width;
				int nMid_modelH = rcSecond.rt.height;
				int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
				nMidInterW = 3;
				nMidInterH = 3;
				nHeadInterW = 4;
				nHeadInterH = 4;
				nMid_minW = nMid_modelW - nMidInterW;
				nMid_maxW = nMid_modelW + nMidInterW;
				nMid_minH = nMid_modelH - nMidInterH;
				nMid_maxH = nMid_modelH + nMidInterH;

				nHead_minW = rcFist.rt.width - nHeadInterW;
				nHead_maxW = rcFist.rt.width + nHeadInterW;
				nHead_minH = rcFist.rt.height - nHeadInterH;
				nHead_maxH = rcFist.rt.height + nHeadInterH;
// 				float fOffset = 0.1;
// 				float fPer_W, fPer_H;	//模板第二个点与第一个点的宽、高的比例，用于最小值控制
// 				fPer_W = 0.5;
// 				fPer_H = 0.25;
// 				int nMid_modelW = rcSecond.rt.width + 2;		//加2是因为制卷模板框框没有经过查边框运算，经过查边框后，外框会包含整个矩形，需要加上上下各1个单位的线宽
// 				int nMid_modelH = rcSecond.rt.height + 2;
// 				if (nMid_modelW < rcFist.rt.width * fPer_W + 0.5)	nMid_modelW = rcFist.rt.width * fPer_W + 0.5;
// 				if (nMid_modelH < rcFist.rt.height * fPer_H + 0.5)	nMid_modelH = rcFist.rt.height * fPer_H + 0.5;
// 				nMid_minW = nMid_modelW * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 				nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 				nMid_minH = nMid_modelH * (1 - fOffset);				//同上
// 				nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//同上
// 
// 				nHead_minW = rcFist.rt.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 				nHead_maxW = rcFist.rt.width * (1 + fOffset * 4) + 0.5;		//同上
// 				nHead_minH = rcFist.rt.height * (1 - fOffset);				//同上
// 				nHead_maxH = rcFist.rt.height * (1 + fOffset * 4) + 0.5;	//同上
			}
			else
			{
				float fOffset_mid = 0.4;
				nMid_minW = rcSecond.rt.width * (1 - fOffset_mid);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nMid_maxW = rcSecond.rt.width * (1 + fOffset_mid);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nMid_minH = rcSecond.rt.height * (1 - fOffset_mid);		//同上
				nMid_maxH = rcSecond.rt.height * (1 + fOffset_mid);		//同上

				float fOffset_Head = 0.3;
				nHead_minW = rcFist.rt.width * (1 - fOffset_Head);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nHead_maxW = rcFist.rt.width * (1 + fOffset_Head);		//同上
				nHead_minH = rcFist.rt.height * (1 - fOffset_Head);		//同上
				nHead_maxH = rcFist.rt.height * (1 + fOffset_Head);		//同上
			}
#endif
			int nXMidSum = 0;	//x轴中线坐标总和
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm.x = rm.x + rc.rt.x;
				rm.y = rm.y + rc.rt.y;

				if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
				{
					if (!(rm.width > nHead_minW && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//排除第一个或最后一个大的同步头
					{
						TRACE("过滤垂直同步头(%d,%d,%d,%d), 要求范围W:[%d,%d], H[%d,%d], 参考大小(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
						continue;
					}
					else
					{
						TRACE("首尾垂直同步头(即定位点同步头)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
					}
				}
				RectCompList.push_back(rm);
				nXMidSum += (rm.x + rm.width / 2);
			}
			cvReleaseMemStorage(&storage);
			int nXMean = nXMidSum / RectCompList.size();

			//根据坐标位置过滤
// 			int nDeviation = 10;
// 			int xMin = nXMean - (rcFist.rt.width / 2 + nDeviation);
// 			int xMax = nXMean + (rcFist.rt.width / 2 + nDeviation);
// 			TRACE("垂直同步头中线X = %d, 偏差[%d,%d]\n", nXMean, xMin, xMax);
// 			std::vector<Rect>::iterator itHead = RectCompList.begin();
// 			for (int i = 0; itHead != RectCompList.end(); i++)
// 			{
// 				if(itHead->x < xMin || itHead->x > xMax)
// 					itHead = RectCompList.erase(itHead);
// 				else if(itHead->x > nXMean && itHead->br().x > xMax)
// 					itHead = RectCompList.erase(itHead);
// 				else
// 					itHead++;
// 			}



#else
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm.x = rm.x + rc.rt.x;
				rm.y = rm.y + rc.rt.y;
				if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50 || rm.area() < 70)
				{
					continue;
				}
				RectCompList.push_back(rm);
			}
#endif
		}
		catch (cv::Exception& exc)
		{
			std::string strLog = "识别垂直同步头异常: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;
			break;
		}
		if (RectCompList.size() == 0)
		{
			bResult = false;
			strErrDesc = "垂直同步头数量为0.";
		}
		else
		{
			//二次过滤
			std::sort(RectCompList.begin(), RectCompList.end(), SortByPositionY2);
			std::vector<Rect>::iterator itHead = RectCompList.begin();
			for (int i = 0; itHead != RectCompList.end(); i++)
			{
				RECTINFO rcHead1;
				RECTLIST::iterator itModelHead = pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.begin();
				for (int j = 0; itModelHead != pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.end(); j++, itModelHead++)
				{
					if (j == i)
					{
						rcHead1 = *itModelHead;
						break;
					}
				}
				RECTINFO rcTmp = rcHead1;
				rcTmp.rt = RectCompList[i];
				Recog(nPic, rcTmp, matCompPic, pPic, pModelInfo);
				if (rcTmp.fRealArea / rcTmp.fStandardArea < 0.7 || rcTmp.fRealDensity / rcTmp.fStandardDensity < 0.8)
				{
					itHead = RectCompList.erase(itHead);
					i = i - 1;
				}
				else
					itHead++;
			}
			for (int i = 0; i < RectCompList.size(); i++)
			{
				RECTINFO rcHead;
				rcHead.rt = RectCompList[i];

				rcHead.nGaussKernel = rc.nGaussKernel;
				rcHead.nSharpKernel = rc.nSharpKernel;
				rcHead.nCannyKernel = rc.nCannyKernel;
				rcHead.nDilateKernel = rc.nDilateKernel;

				rcHead.eCPType = V_HEAD;
				m_vecV_Head.push_back(rcHead);
				//++ for test
				pPic->lNormalRect.push_back(rcHead);
				//--
			}
			std::sort(m_vecV_Head.begin(), m_vecV_Head.end(), SortByPositionY);
		}
		if(m_vecV_Head.size() != pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.size())
		{
			bResult = false;
			pPic->bFindIssue = true;
			for(int i = 0; i < m_vecV_Head.size(); i++)
				pPic->lIssueRect.push_back(m_vecV_Head[i]);

			strErrDesc = Poco::format("垂直同步头数量为%u, 与模板垂直同步头数量(%u)不一致", m_vecV_Head.size(), pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.size());
		}
#else
		GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt); //GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic].lFix, rc.rt);
		bool bFindRect = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bFindRect)
		{
			bResult = false;						//找到问题点
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
		else
			pPic->lNormalRect.push_back(rc);
#endif
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别垂直同步头失败, 原因: %s, 图片名: %s\n", strErrDesc.c_str(), pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("识别AB卷型\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lABModel.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lABModel.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bFindRect = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bFindRect)
		{
			bResult = false;						//找到问题点
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
		else
			pPic->lNormalRect.push_back(rc);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别AB卷型失败, 图片名: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("识别科目\n");
	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lCourse.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lCourse.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)				//**************	如果科目是由多个校验点组合构成，需要另外考虑	*******************
			{
				pPic->lNormalRect.push_back(rc);
				bResult = true;
				continue;
			}
			else
			{
				char szLog[MAX_PATH] = { 0 };
				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				strLog.append(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			strLog.append(szLog);
			TRACE(szLog);
		}
		
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//找到问题点
			break;		//发现问题点时，继续判断后面的点，不停止扫描
		}
	}
	if (!bResult)
	{
		(static_cast<pST_PaperInfo>(pPic->pPaper))->bRecogCourse = false;
		pPic->bRecogCourse = false;
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别科目失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
//		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别科目校验点时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("识别缺考\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lQK_CP.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lQK_CP.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				((pST_PaperInfo)pPic->pPaper)->nQKFlag = 1;			//设置学生缺考
				pPic->nQKFlag = 1;
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}
		
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//找到问题点
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别缺考失败, 图片名: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("识别违纪\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lWJ_CP.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lWJ_CP.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				((pST_PaperInfo)pPic->pPaper)->nWJFlag = 1;			//设置学生违纪
				pPic->nWJFlag = 1;
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//找到问题点
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别违纪失败, 图片名: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("识别灰度点\n");

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lGray.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lGray.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				pPic->lNormalRect.push_back(rc);
				bResult = true;
				continue;
			}
			else
			{
				char szLog[MAX_PATH] = { 0 };
				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				strLog.append(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			strLog.append(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//找到问题点
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别灰度校验点失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别灰度校验点时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lWhite.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lWhite.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				pPic->lNormalRect.push_back(rc);
				bResult = true;
				continue;
			}
			else
			{
				char szLog[MAX_PATH] = { 0 };
				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				g_pLogger->information(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//找到问题点
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别空白校验点失败, 图片名: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	if (pModelInfo->pModel->nZkzhType == 2)
		bResult = RecogSn_code(nPic, matCompPic, pPic, pModelInfo);
	else
		bResult = RecogSn_omr(nPic, matCompPic, pPic, pModelInfo);
	return bResult;
}

bool CRecognizeThread::RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	int nNullCount = 0;
	int nDoubtCount = 0;
	int nEqualCount = 0;
	int nSingleToMultiCount = 0;	//单选识别为多选且无怀疑的数量
	int nNullCount_1 = 0;	//第一种方法识别出的空值
	int nNullCount_2 = 0;	//第二种方法识别出的空值
	int nNullCount_3 = 0;	//第三种方法识别出的空值

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecOmr;
	OMRLIST::iterator itOmr = pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.begin();
	for (; itOmr != pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.end(); itOmr++)
	{
		pOMR_QUESTION pOmrQuestion = &(*itOmr);
		
		OMR_RESULT omrResult;
		omrResult.nTH = pOmrQuestion->nTH;
		omrResult.nSingle = pOmrQuestion->nSingle;
		omrResult.nPageId = nPic + 1;

		std::vector<int> vecVal_calcHist;		//直方图灰度计算的识别结果
		std::vector<int> vecVal_threshold;		//二值化计算的识别结果
		RECTLIST::iterator itOmrItem = pOmrQuestion->lSelAnswer.begin();
		for (; itOmrItem != pOmrQuestion->lSelAnswer.end(); itOmrItem++)
		{
			RECTINFO rc = *itOmrItem;

			if (pModelInfo->pModel->nHasHead)
			{
				if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
				{
					bResult = false;
					pPic->bFindIssue = true;
					pPic->lIssueRect.push_back(rc);
					break;
				}
				rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
				rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
				rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
				rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
			}
			else
				GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);

			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > _dAnswerSure_DensityFix_)	//rc.fStandardValuePercent
				{
					vecVal_calcHist.push_back(rc.nAnswer);
				}
			}

// 			bool bResult_Recog2 = RecogVal(nPic, rc, matCompPic, pPic, pModelInfo);
// 			if (bResult_Recog2)
// 			{
// 				vecVal_threshold.push_back(rc.nAnswer);
// 			}
			omrResult.lSelAnswer.push_back(rc);
			
		#ifdef PrintWordRecogPoint
			if (pModelInfo->pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() > 0 && pPic->lModelWordFix.size() > 0)
			{

				VEC_NEWRTBY2FIX vecNewRt;
				RECTLIST::iterator itFix = pPic->lFix.begin();
				RECTLIST::iterator itModelFix = pPic->lModelWordFix.begin();
				itFix++;
				itModelFix++;
				for (int i = 1; itFix != pPic->lFix.end(); itFix++, itModelFix++, i++)
				{
					RECTLIST lTmpFix, lTmpModelFix;
					lTmpFix.push_back(pPic->lFix.front());
					lTmpModelFix.push_back(pPic->lModelWordFix.front());

					lTmpFix.push_back(*itFix);
					lTmpModelFix.push_back(*itModelFix);

					ST_NEWRTBY2FIX stNewRt;
					stNewRt.nFirstFix = 0;
					stNewRt.nSecondFix = i;
					stNewRt.rt = itOmrItem->rt;
					GetPosition(lTmpFix, lTmpModelFix, stNewRt.rt);
					vecNewRt.push_back(stNewRt);
				}
				for (auto newRt : vecNewRt)
				{
					RECTINFO rcTmp;
					rcTmp = rc;
					rcTmp.rt = newRt.rt;

					pPic->lCalcRect.push_back(rcTmp);
				}
			}
		#endif
			
		#ifdef PaintOmrSnRect	//打印OMR、SN位置
			pPic->lNormalRect.push_back(rc);
		#endif
		}

	#if 1
		std::string strRecogAnswer;
		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDiff;
		calcOmrDensityDiffVal(omrResult.lSelAnswer, vecItemsDesc, vecOmrItemDiff);

		float fCompThread = 0.0;		//密度间隔达到要求时，第一个选项的密度必须达到的要求
		float fDiffThread = 0.0;		//选项可能填涂的可能密度梯度阀值
		float fDiffExit = 0;			//密度的梯度递减太快时，可以认为后面选项没有填涂，此时的密度梯度阀值
		if (pModelInfo->pModel->nHasHead)
		{
			fCompThread = _dCompThread_Head_;
			fDiffThread = _dDiffThread_Head_;
			fDiffExit = _dDiffExit_Head_;
		}
		else
		{
			fCompThread = _dCompThread_Fix_;
			fDiffThread = _dDiffThread_Fix_;
			fDiffExit = _dDiffExit_Fix_;
		}

		int nFlag = -1;
		float fThreld = 0.0;
		float fDensityThreshold = 0.0;
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
			//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
			float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.5;
			if ((vecOmrItemDiff[i].fDiff >= fDiffThread + fDensityThreshold * 0.5))
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
				fDensityThreshold += vecOmrItemDiff[i].fDiff;
			}
		}
		if (nFlag >= 0)
		{
			//++判断全选的情况
			if (nFlag == vecOmrItemDiff.size() - 1)
			{
				if (vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent >= fCompThread + fDiffExit)	//如果密度最低的选项，它的密度大于“最低比较密度 + 最大退出密度差”，则认为全选
					fThreld = vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent;
			}
			//--
			RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
			for (; itItem != omrResult.lSelAnswer.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					char szVal[10] = { 0 };
					sprintf_s(szVal, "%c", itItem->nAnswer + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else if (vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent >= fCompThread + fDiffExit)	//如果密度最低的选项，它的密度大于“最低比较密度 + 最大退出密度差”，则认为全选
		{
			fThreld = vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent;

			RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
			for (; itItem != omrResult.lSelAnswer.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					char szVal[10] = { 0 };
					sprintf_s(szVal, "%c", itItem->nAnswer + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else if (vecItemsDesc[0]->fRealValuePercent >= fCompThread && vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent < fCompThread + fDiffExit)
		{
			int nFlag = -1;
			float fThreld = 0.0;
			for (int i = 0; i < vecOmrItemDiff.size(); i++)
			{
				//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
				//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
				float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.1;
				if ((vecOmrItemDiff[i].fDiff >= fDiffThread && vecOmrItemDiff[i].fFirst > fCompThread) ||
					(vecOmrItemDiff[i].fDiff >= fDiffThread + fDiff && vecOmrItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
				{
					nFlag = i;
					fThreld = vecOmrItemDiff[i].fFirst;
					if (vecOmrItemDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_calcHist.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
						break;
				}
			}
			if (nFlag >= 0)
			{
				RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
				for (; itItem != omrResult.lSelAnswer.end(); itItem++)
				{
					if (itItem->fRealValuePercent >= fThreld)
					{
						char szVal[2] = { 0 };
						sprintf_s(szVal, "%c", itItem->nAnswer + 65);
						strRecogAnswer1.append(szVal);
					}
				}
			}
			else if (vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent > fCompThread && vecItemsDesc[vecOmrItemDiff.size()]->fRealMeanGray < (_dCompThread_3_ + _dDiffExit_3_ + _dAnswerSure_) / 2)	//全选，最低密度>比较密度，同时此选项的灰度<(比较灰度 + 灰度答案确认值 + 灰度退出密度值)/2
			{
				fThreld = vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent;

				RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
				for (; itItem != omrResult.lSelAnswer.end(); itItem++)
				{
					if (itItem->fRealValuePercent >= fThreld)
					{
						char szVal[10] = { 0 };
						sprintf_s(szVal, "%c", itItem->nAnswer + 65);
						strRecogAnswer1.append(szVal);
					}
				}
			}
			else
			{
				for (int i = 0; i < vecVal_calcHist.size(); i++)
				{
					char szVal[5] = { 0 };
					sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else
		{
			for (int i = 0; i < vecVal_calcHist.size(); i++)
			{
				char szVal[10] = { 0 };
				sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
				strRecogAnswer1.append(szVal);
			}
		}
	#else
		std::string strRecogAnswer;
		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDiff;
		calcOmrDensityDiffVal(omrResult.lSelAnswer, vecItemsDesc, vecOmrItemDiff);

		float fCompThread = 0.0;		//灰度间隔达到要求时，第一个选项的灰度必须达到的要求
		float fDiffThread = 0.0;		//选项可能填涂的可能灰度梯度阀值
		float fDiffExit = 0;			//灰度的梯度递减太快时，可以认为后面选项没有填涂，此时的灰度梯度阀值
		if (pModelInfo->pModel->nHasHead)
		{
			fCompThread = _dCompThread_Head_;
			fDiffThread = _dDiffThread_Head_;
			fDiffExit = _dDiffExit_Head_;
		}
		else
		{
			fCompThread = _dCompThread_Fix_;
			fDiffThread = _dDiffThread_Fix_;
			fDiffExit = _dDiffExit_Fix_;
		}

		int nFlag = -1;
		float fThreld = 0.0;
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
			//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
			float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.1;
			if ((vecOmrItemDiff[i].fDiff >= fDiffThread && vecOmrItemDiff[i].fFirst > fCompThread) ||
				(vecOmrItemDiff[i].fDiff >= fDiffThread + fDiff && vecOmrItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
				if (vecOmrItemDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_calcHist.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
					break;
			}
		}
		if (nFlag >= 0)
		{
			RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
			for (; itItem != omrResult.lSelAnswer.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					char szVal[2] = { 0 };
					sprintf_s(szVal, "%c", itItem->nAnswer + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else
		{
			for (int i = 0; i < vecVal_calcHist.size(); i++)
			{
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
				strRecogAnswer1.append(szVal);
			}
		}
	#endif

	#ifdef Test_RecogOmr3
		RecogVal_Omr2(nPic, matCompPic, pPic, pModelInfo, omrResult);
		RecogVal_Omr3(nPic, matCompPic, pPic, pModelInfo, omrResult);

		std::string strRecogAnswer2 = omrResult.strRecogVal2;
		std::string strRecogAnswer3 = omrResult.strRecogVal3;

		if (strRecogAnswer1 == "") nNullCount_1++;
		if (strRecogAnswer2 == "") nNullCount_2++;
		if (strRecogAnswer3 == "") nNullCount_3++;

		int nDoubt = 0;
		if (strRecogAnswer1 == strRecogAnswer2 && strRecogAnswer1 == strRecogAnswer3)	//为空判断时，方法1与方法3准确度更高
		{
			if (strRecogAnswer1 == "")
			{
				strRecogAnswer = strRecogAnswer1;
				nDoubt = 2;
				nNullCount++;

// 				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
// 				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrNull++;
// 				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
			}
			else
			{
				strRecogAnswer = strRecogAnswer1;
				nDoubt = 0;
				nEqualCount++;
			}
		}
		else
		{
			if (strRecogAnswer1 == "")
			{
				if (strRecogAnswer2 == strRecogAnswer3)
				{
					strRecogAnswer = strRecogAnswer3;
					nDoubt = 0;
					nEqualCount++;
				}
				else
				{
					//方法2、方法3两种方法有一种方法判断非空
					if (strRecogAnswer2 == "" && strRecogAnswer3 != "")			
					{
						strRecogAnswer = strRecogAnswer3;
						nDoubt = 1;
						nDoubtCount++;

// 						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
// 						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrDoubt++;
// 						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
					}
					else if (strRecogAnswer3 == "" && strRecogAnswer2 != "")
					{
						strRecogAnswer = strRecogAnswer3;
						nDoubt = 2;
						nEqualCount++;
					}
					else if (strRecogAnswer2 != "" && strRecogAnswer3 != "")
					{
						strRecogAnswer = strRecogAnswer3;
						nDoubt = 1;
						nDoubtCount++;

// 						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
// 						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrDoubt++;
// 						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
					}
				}
			}
			else
			{
				if (strRecogAnswer1 == strRecogAnswer2 || strRecogAnswer1 == strRecogAnswer3)
				{
					strRecogAnswer = strRecogAnswer1;
					nDoubt = 0;
					nEqualCount++;
				}
				else if ((strRecogAnswer1 != strRecogAnswer2) && (strRecogAnswer2 == strRecogAnswer3 && strRecogAnswer3 != ""))		//2017.10.20
				{
					strRecogAnswer = strRecogAnswer3;
					nDoubt = 0;
					nEqualCount++;
				}
				else
				{
					strRecogAnswer = strRecogAnswer1;
					nDoubt = 1;
					nDoubtCount++;

// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrDoubt++;
// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
				}
			}
		}
	#else
		//++ test	测试整题选项进行二值化识别
		RecogVal_Omr2(nPic, matCompPic, pPic, pModelInfo, omrResult);
		//--
		std::string strRecogAnswer2 = omrResult.strRecogVal2;

		if (strRecogAnswer1 == "") nNullCount_1++;
		if (strRecogAnswer2 == "") nNullCount_2++;

		int nDoubt = 0;
		if (strRecogAnswer1 == "" && strRecogAnswer2 == "")
		{
			nDoubt = 2;
			nNullCount++;

			(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
			(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrNull++;
			(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
		}
		else
		{
			if (strRecogAnswer1 == strRecogAnswer2)
			{
				nDoubt = 0;
				nEqualCount++;
			}
			else
			{
				nDoubt = 1;
				nDoubtCount++;

				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrDoubt++;
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
			}
		}		
	#endif
		omrResult.nDoubt = nDoubt;
		omrResult.strRecogVal = strRecogAnswer;
		omrResult.strRecogVal1 = strRecogAnswer1;
		omrResult.strRecogVal2 = strRecogAnswer2;
		omrResult.strRecogVal3 = strRecogAnswer3;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lOmrResult.push_back(omrResult);
		pPic->lOmrResult.push_back(omrResult);

		if (nDoubt == 0 && omrResult.nSingle == 0 && omrResult.strRecogVal.size() > 1)
			nSingleToMultiCount++;
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别OMR失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}

	int nCount = pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.size();

// 	std::stringstream ss;
// 	ss << "总数" << nCount << ",空值" << nNullCount << "(" << (double)nNullCount / nCount * 100 << "%)[No.1=" << nNullCount_1 << "(" << (double)nNullCount_1 / nCount * 100 << "%),No.2=" << nNullCount_2
// 		<< "(" << (double)nNullCount_2 / nCount * 100 << "%)],怀疑" << nDoubtCount << "(" << (double)nDoubtCount / nCount * 100 << "%),无怀疑" << nEqualCount << "(" << (double)nEqualCount / nCount * 100
// 		<< "%)";
// 	std::string strTmp = ss.str();

	pST_PaperInfo pPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	if (nDoubtCount > 0 && !pPaper->bHasOmrDoubt)
	{
		pPaper->bHasOmrDoubt = true;
		pPaper->nOmrDoubt += nDoubtCount;
	}
	if (nNullCount > 0 && !pPaper->bHasOmrNull)
	{
		pPaper->bHasOmrNull = true;
		pPaper->nOmrNull += nNullCount;
	}
	if (nSingleToMultiCount > 0 && !pPaper->bHasSingleToMulti)
	{
		pPaper->bHasSingleToMulti = true;
		pPaper->nOmrSingleToMulti += nSingleToMultiCount;
	}
	if (pPaper->bHasOmrDoubt || pPaper->bHasOmrNull || pPaper->bHasSingleToMulti)	pPaper->bOmrIssue = true;

	if (bResult && nCount)
	{
		char szStatistics[150] = { 0 };
		sprintf_s(szStatistics, "图片(%s)选项总数[%d],空值%d(%.2f%%)[No.1=%d(%.2f%%),No.2=%d(%.2f%%), No.3=%d(%.2f%%)],怀疑%d(%.2f%%),无怀疑%d(%.2f%%)", pPic->strPicName.c_str(), nCount, nNullCount, (float)nNullCount / nCount * 100, \
				  nNullCount_1, (float)nNullCount_1 / nCount * 100, nNullCount_2, (float)nNullCount_2 / nCount * 100, nNullCount_3, (float)nNullCount_3 / nCount * 100, \
				  nDoubtCount, (float)nDoubtCount / nCount * 100, nEqualCount, (float)nEqualCount / nCount * 100);
		
		strLog.append(szStatistics);
	}	
	end = clock();
	std::string strTime = Poco::format("识别Omr时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;

	std::vector<int> vecOmr;
	ELECTOMR_LIST::iterator itElectOmr = pModelInfo->pModel->vecPaperModel[nPic]->lElectOmr.begin();
	for (; itElectOmr != pModelInfo->pModel->vecPaperModel[nPic]->lElectOmr.end(); itElectOmr++)
	{
		pELECTOMR_QUESTION pOmrQuestion = &(*itElectOmr);

		ELECTOMR_QUESTION omrResult;
		omrResult.nPageId = nPic + 1;
		omrResult.sElectOmrGroupInfo = pOmrQuestion->sElectOmrGroupInfo;

		std::vector<int> vecVal_calcHist;		//直方图灰度计算的识别结果
		std::vector<int> vecVal_threshold;		//二值化计算的识别结果
		RECTLIST::iterator itOmrItem = pOmrQuestion->lItemInfo.begin();
		for (; itOmrItem != pOmrQuestion->lItemInfo.end(); itOmrItem++)
		{
			RECTINFO rc = *itOmrItem;

			if (pModelInfo->pModel->nHasHead)
			{
				if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
				{
					bResult = false;
					pPic->bFindIssue = true;
					pPic->lIssueRect.push_back(rc);
					break;
				}
				rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
				rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
				rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
				rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
			}
			else
				GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);

			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > rc.fStandardValuePercent)
				{
					vecVal_calcHist.push_back(rc.nAnswer);
				}
			}

// 			bool bResult_Recog2 = RecogVal(nPic, rc, matCompPic, pPic, pModelInfo);
// 			if (bResult_Recog2)
// 			{
// 				vecVal_threshold.push_back(rc.nAnswer);
// 			}
			omrResult.lItemInfo.push_back(rc);

#ifdef PaintOmrSnRect	//打印OMR、SN位置
			pPic->lNormalRect.push_back(rc);
#endif
		}

#if 1
		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDiff;
		calcOmrDensityDiffVal(omrResult.lItemInfo, vecItemsDesc, vecOmrItemDiff);

		float fCompThread = 0.0;		//灰度间隔达到要求时，第一个选项的灰度必须达到的要求
		float fDiffThread = 0.0;		//选项可能填涂的可能灰度梯度阀值
		float fDiffExit = 0;			//灰度的梯度递减太快时，可以认为后面选项没有填涂，此时的灰度梯度阀值
		if (pModelInfo->pModel->nHasHead)
		{
			fCompThread = _dCompThread_Head_;
			fDiffThread = _dDiffThread_Head_;
			fDiffExit = _dDiffExit_Head_;
		}
		else
		{
			fCompThread = _dCompThread_Fix_;
			fDiffThread = _dDiffThread_Fix_;
			fDiffExit = _dDiffExit_Fix_;
		}

		int nFlag = -1;
		float fThreld = 0.0;
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
			//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
			float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.1;
			if ((vecOmrItemDiff[i].fDiff >= fDiffThread && vecOmrItemDiff[i].fFirst > fCompThread) ||
				(vecOmrItemDiff[i].fDiff >= fDiffThread + fDiff && vecOmrItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
				if (vecOmrItemDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_calcHist.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
					break;
			}
		}
		if (nFlag >= 0)
		{
			RECTLIST::iterator itItem = omrResult.lItemInfo.begin();
			for (; itItem != omrResult.lItemInfo.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					char szVal[2] = { 0 };
					sprintf_s(szVal, "%c", itItem->nAnswer + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else
		{
			for (int i = 0; i < vecVal_calcHist.size(); i++)
			{
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
				strRecogAnswer1.append(szVal);
			}
		}
#else	//一下是直接通过阀值判断是否选中，可用，对填涂不规范并且不清晰的情况不够理想
		std::string strRecogAnswer1;
		for (int i = 0; i < vecVal_calcHist.size(); i++)
		{
			char szVal[5] = { 0 };
			sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
			strRecogAnswer1.append(szVal);
		}
#endif
		
		std::string strRecogAnswer2 = strRecogAnswer1;		//目前第二种方法不可用
		int nDoubt = 0;
		if (strRecogAnswer1 == "" && strRecogAnswer2 == "")
			nDoubt = 2;
		else
		{
			if (strRecogAnswer1 == strRecogAnswer2)
				nDoubt = 0;
			else
				nDoubt = 1;
		}

		omrResult.nDoubt = nDoubt;
		omrResult.strRecogResult = strRecogAnswer1;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lElectOmrResult.push_back(omrResult);
		pPic->lElectOmrResult.push_back(omrResult);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别选做题OMR失败, 图片名: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}

	return bResult;
}

bool CRecognizeThread::RecogLostCorner(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	//检测试卷的4个角
 	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.begin();
// 	for (int i = 0; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.end(); i++, itCP++)
	int nRtW = 150;
	int nRtH = 150;
	for(int i = 0; i < 4; i++)
	{
		RECTINFO rc = *itCP;

		std::vector<Rect>RectCompList;
		try
		{
			float fModelW = pModelInfo->pModel->vecPaperModel[nPic]->nPicW;
			float fModelH = pModelInfo->pModel->vecPaperModel[nPic]->nPicH;
			int nRealW = matCompPic.cols;
			int nRealH = matCompPic.rows;

			cv::Rect rtTmp;
			rtTmp.width = nRtW * nRealW / fModelW;
			rtTmp.height = nRtH * nRealH / fModelH;
			rtTmp.x = i % 2 == 0 ? 0 : (nRealW - rtTmp.width - 1);
			rtTmp.y = i / 2 == 0 ? 0 : (nRealH - rtTmp.height - 1);

			if (rtTmp.x < 0) rtTmp.x = 0;
			if (rtTmp.y < 0) rtTmp.y = 0;
			if (rtTmp.br().x > matCompPic.cols)
			{
				rtTmp.width = matCompPic.cols - rtTmp.x;
			}
			if (rtTmp.br().y > matCompPic.rows)
			{
				rtTmp.height = matCompPic.rows - rtTmp.y;
			}
			cv::Mat matCompRoi;
			matCompRoi = matCompPic(rtTmp);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

			int nRealThreshold = 100;

#ifdef USES_GETTHRESHOLD_ZTFB
			const int channels[1] = { 0 };
			const int histSize[1] = { nRealThreshold };	//150
			float hranges[2] = { 0, nRealThreshold };	//150
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = nRealThreshold;		//150
			if (nCount > 0)
			{
				float fMean = (float)nSum / nCount;		//均值

				for (int h = 0; h < hist.rows; h++)	//histSize
				{
					float binVal = hist.at<float>(h);

					nDevSum += pow(h - fMean, 2)*binVal;
				}
				float fStdev = sqrt(nDevSum / nCount);
				nThreshold = fMean + 2 * fStdev;
				if (fStdev > fMean)
					nThreshold = fMean + fStdev;
			}

			if (nThreshold > nRealThreshold) nThreshold = nRealThreshold;	//150
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别	Size(_nDilateKernel_, _nDilateKernel_)
			dilate(matCompRoi, matCompRoi, element);

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm = rm + rtTmp.tl();	// rc.rt.tl();
				RectCompList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);
		}
		catch (cv::Exception& exc)
		{
			std::string strLog2 = Poco::format("识别缺角折叠(%d)异常: %s\n", i, exc.msg.c_str());
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
			if (g_nOperatingMode == 2)
			{
				bResult = false;						//找到问题点
				break;
			}
		}

		RECTINFO rcLostCorner;		//缺角矩形区域
		std::string strLog2;	//临时日志，记录矩形具体识别结果
		bool bFindRect = false;
		if (RectCompList.size() > 0)
		{
			std::sort(RectCompList.begin(), RectCompList.end(), [](cv::Rect& rt1, cv::Rect& rt2)
			{
				return rt1.area() > rt2.area() ? true : (rt1.area() < rt2.area() ? false : (rt1.x > rt2.x ? true : false));
			});
			rcLostCorner.rt = RectCompList[0];

			bool bFind = false;

			//通过灰度值来判断
			for (int k = 0; k < RectCompList.size(); k++)
			{
				RECTINFO rcTmp = rc;
				rcTmp.rt = RectCompList[k];
				rcTmp.nThresholdValue = 100;
				rcTmp.fStandardArea = rcTmp.rt.area();

				//根据定点左上点与右下点位置判断是否在试卷的边线上，如果在，则可能是折角或者边上有损坏
				cv::Point pt1 = RectCompList[k].tl();
				cv::Point pt2 = RectCompList[k].br();
				int nDiff = 10;	//与图像边界的距离间隔在这个值之内，认为属于边界线上
				if (pt1.x < nDiff || pt1.y < nDiff || matCompPic.cols - pt2.x < nDiff || matCompPic.rows - pt2.y < nDiff)
				{
					TRACE("矩形(%d,%d,%d,%d)位置距离边线太近，可能是折角或损坏\n", RectCompList[k].x, RectCompList[k].y, RectCompList[k].width, RectCompList[k].height);

					Recog(nPic, rcTmp, matCompPic, pPic, pModelInfo);
 					float fDensity = rcTmp.fRealValue / (nRtW * nRtH);
					if (fDensity >= 0.25 && rcTmp.fRealDensity > 0.4)	//要求满足要求的黑色区域达到框选角落的1/4以上
					{
						bFind = true;
						rcLostCorner.rt = RectCompList[k];
						rcLostCorner.nTH = i;					//标识属于第几个角落
						break;
					}
				}
			}

			bFindRect = bFind;
		}
		if (bFindRect)
		{
			std::string strLog3 = Poco::format("检测缺角折角(%d)完成 -- %s\n", i, strLog2);
			strLog.append(strLog3);
			bResult = false;						//找到问题点
			pPic->lLostCorner.push_back(rcLostCorner);
			if (g_nOperatingMode == 2)
				break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "可能存在缺角折角, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("检测缺角折角时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogVal(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	Mat matSrcRoi, matCompRoi;
	Rect rt = rc.rt;
	bool bResult = false;
	try
	{
		matCompRoi = matCompPic(rt);

		Mat imag_src, img_comp;
		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		//图片二值化
		threshold(matCompRoi, matCompRoi, 240, 255, THRESH_BINARY_INV);				//200, 255
#if 0
		//确定腐蚀和膨胀核的大小
		Mat element = getStructuringElement(MORPH_RECT, Size(4, 4));	//Size(4, 4)
		//膨胀操作
		dilate(matCompRoi, matCompRoi, element);
		
		Mat element2 = getStructuringElement(MORPH_RECT, Size(15, 15));	//Size(4, 4)
		//腐蚀操作1
		erode(matCompRoi, matCompRoi, element2);
		
		Mat element3 = getStructuringElement(MORPH_RECT, Size(4, 4));	//Size(4, 4)
		//腐蚀操作2
		erode(matCompRoi, matCompRoi, element3);
		
		Mat element4 = getStructuringElement(MORPH_RECT, Size(5, 5));	//Size(4, 4)
		dilate(matCompRoi, matCompRoi, element4);
		
#else
	#if 0
		//确定腐蚀和膨胀核的大小
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
		//腐蚀操作1
		erode(matCompRoi, matCompRoi, element);
	#else
		//确定腐蚀和膨胀核的大小
		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));	//Size(4, 4)
		//腐蚀操作1
		erode(matCompRoi, matCompRoi, element);

		//确定腐蚀和膨胀核的大小
		Mat element2 = getStructuringElement(MORPH_RECT, Size(3, 3));	//Size(4, 4)
		//腐蚀操作2
		erode(matCompRoi, matCompRoi, element2);

		//膨胀操作
		dilate(matCompRoi, matCompRoi, element2);
		//膨胀操作
		dilate(matCompRoi, matCompRoi, element);
	#endif
#endif
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		std::vector<Rect>RectCompList;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			RectCompList.push_back(rm);

// 			cv::Rect rtTmp = rm;
// 			rtTmp.x += rt.x;
// 			rtTmp.y += rt.y;
// 			rectangle(matCompPic, rtTmp, CV_RGB(255, 0, 0));
		}
		cvReleaseMemStorage(&storage);
		if (RectCompList.size() == 0)
			bResult = false;
		else
			bResult = true;
	}
	catch (cv::Exception &exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "CRecognizeThread::RecogVal error. detail: %s\n", exc.msg);
		g_pLogger->information(szLog);
		TRACE(szLog);
	}

	return bResult;
}

inline bool CRecognizeThread::RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, RECTLIST& lSelInfo, std::string& strResult)
{
	Mat matCompRoi;
	
	bool bResult = false;

	//omr框的大小高度
	RECTLIST::iterator itItem = lSelInfo.begin();
	RECTLIST::reverse_iterator itEndItem = lSelInfo.rbegin();

 	cv::Point pt1, pt2;
	if (itItem->eCPType == SN)
	{
		switch (itItem->nRecogFlag)
		{
			case 10:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 9:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
			case 6:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 5:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
		}
	}
	else if (itItem->eCPType == OMR)
	{
		switch (itItem->nRecogFlag)
		{
			case 42:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 41:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 38:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
			case 37:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
			case 26:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 25:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 22:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
			case 21:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
		}
	}
// 	pt1 = itItem->rt.tl() - cv::Point(3, 3);
// 	pt2 = itEndItem->rt.br() + cv::Point(3, 3);
	Rect rt = cv::Rect(pt1, pt2);	//ABCD整个题目的选项区

	RECTLIST::iterator itFirst = itItem++;
#if 0
	int nSpace = 0;		//选项间的空格距离(可能是宽，可能是高)
	switch (itItem->nRecogFlag)	//获取选项间的空白区域宽度或高度
	{
		case 42:	//101010
			nSpace = itItem->rt.tl().y - itFirst->rt.br().y;
			break;
		case 41:	//101001
			nSpace = itItem->rt.tl().y - itFirst->rt.br().y;
			break;
		case 38:	//100110
			nSpace = itFirst->rt.tl().y - itItem->rt.br().y;
			break;
		case 37:	//100101
			nSpace = itFirst->rt.tl().y - itItem->rt.br().y;
			break;
		case 26:
			nSpace = itItem->rt.tl().x - itFirst->rt.br().x;
			break;
		case 25:
			nSpace = itItem->rt.tl().x - itFirst->rt.br().x;
			break;
		case 22:
			nSpace = itFirst->rt.tl().x - itItem->rt.br().x;
			break;
		case 21:
			nSpace = itFirst->rt.tl().x - itItem->rt.br().x;
			break;
		default:
			nSpace = max(abs(itFirst->rt.br().x - itItem->rt.tl().x), abs(itFirst->rt.br().y - itItem->rt.tl().y));
			break;
	}
#endif
	int nOmrMinW, nOmrMinH, nAreaMin;
	nOmrMinW = itFirst->rt.width * 0.4;
	nOmrMinH = itFirst->rt.height * 0.4;
	nAreaMin = itFirst->rt.area() * 0.3;
	//根据大小、面积先过滤一下可能框选到题号的情况

	try
	{
		cv::Point ptNew1, ptNew2;
		if (!pModelInfo->pModel->nHasHead)	//无同步头时，进行坐标重定位
		{
			if (rt.x < 0) rt.x = 0;
			if (rt.y < 0) rt.y = 0;
			if (rt.br().x > matCompPic.cols)
			{
				rt.width = matCompPic.cols - rt.x;
			}
			if (rt.br().y > matCompPic.rows)
			{
				rt.height = matCompPic.rows - rt.y;
			}
			matCompRoi = matCompPic(rt);

			cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			//图片二值化
			threshold(matCompRoi, matCompRoi, _nThreshold_Recog2_, 255, THRESH_BINARY_INV);				//200, 255

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			bool bFirst = true;
			std::vector<Rect> RectBaseList;
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				cv::Rect rm = aRect;

				if (rm.width < nOmrMinW || rm.height < nOmrMinH || rm.area() < nAreaMin)		//************	针对中括号的选项会存在问题	************
					continue;

				//需要去除可能包含的情况


				if (bFirst)
				{
					bFirst = false;
					ptNew1 = rm.tl() + pt1;
					ptNew2 = rm.br() + pt1;
				}
				if (itItem->nRecogFlag >= 37)	//选项为竖向的
				{
					if (rm.tl().y + pt1.y < ptNew1.y)	ptNew1 = rm.tl() + pt1;
					if (rm.br().y + pt1.y > ptNew2.y)	ptNew2 = rm.br() + pt1;
				}
				else  //选项为横向的
				{
					if (rm.tl().x + pt1.x < ptNew1.x)	ptNew1 = rm.tl() + pt1;
					if (rm.br().x + pt1.x > ptNew2.x)	ptNew2 = rm.br() + pt1;
				}
				RectBaseList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);

			if (RectBaseList.size() <= lSelInfo.size())	//识别出的矩形区个数和选项一致，可能有删除的情况，还是去新的起点与原起点的中间值
			{
				ptNew1 = (pt1 + ptNew1) / 2;
				ptNew2 = (pt2 + ptNew2) / 2;
			}

// 			if (itItem->nRecogFlag >= 37)
// 				std::sort(RectBaseList.begin(), RectBaseList.end(), SortByPositionY2);
// 			else
// 				std::sort(RectBaseList.begin(), RectBaseList.end(), SortByPositionX2);
		}
		else
		{
			ptNew1 = pt1;
			ptNew2 = pt2;
		}
		//根据新的坐标点计算新选项区矩形的填涂情况
//		matCompRoi.deallocate();
		matCompRoi = matCompPic(cv::Rect(ptNew1, ptNew2));
		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

// 		GaussianBlur(matCompRoi, matCompRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
// 		sharpenImage1(matCompRoi, matCompRoi);

		//++先获取均值和标准差，再计算新的二值化阀值	2017.4.27
		const int channels[1] = { 0 };
		const int histSize[1] = { _nThreshold_Recog2_ };
		float hranges[2] = { 0, _nThreshold_Recog2_ };
		const float* ranges[1];
		ranges[0] = hranges;
		MatND hist;
		calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		int nThreshold = _nThreshold_Recog2_;
		if (nCount > 0)
		{
			float fMean = (float)nSum / nCount;		//均值

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);
			nThreshold = fMean + fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;
		}

		if (nThreshold > _nThreshold_Recog2_) nThreshold = _nThreshold_Recog2_;
		//--

		//图片二值化
		threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY_INV);				//200, 255

		//这里进行开闭运算
		//确定腐蚀和膨胀核的大小
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
		//腐蚀操作
		erode(matCompRoi, matCompRoi, element);

		IplImage ipl_img2(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage2 = cvCreateMemStorage(0);
		CvSeq* contour2 = 0;

		//提取轮廓  
		cvFindContours(&ipl_img2, storage2, &contour2, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		int nMaxArea = 0;
		std::vector<Rect>RectCompList;
		for (int iteratorIdx = 0; contour2 != 0; contour2 = contour2->h_next, iteratorIdx++)
		{
			CvRect aRect = cvBoundingRect(contour2, 0);
			cv::Rect rm = aRect;
			rm.x += ptNew1.x;
			rm.y += ptNew1.y;
			if (rm.area() > nMaxArea)
				nMaxArea = rm.area();

			RectCompList.push_back(rm);
		}
		cvReleaseMemStorage(&storage2);

		if (RectCompList.size() > 1)
		{
			int nMinArea = nMaxArea * 0.3;				//在所有二值化识别出来的矩形中再过滤一遍，根据最大识别矩形的面积
			std::vector<Rect>::iterator itRect = RectCompList.begin();
			for (; itRect != RectCompList.end();)
			{
				if (itRect->area() < nMinArea)
					itRect = RectCompList.erase(itRect);
				else
					itRect++;
			}
		}		

		//接下来根据位置信息判断abcd
		float fThreod;
		if (pModelInfo->pModel->nHasHead)
			fThreod = _dCompThread_Head_;
		else
			fThreod = _dCompThread_Fix_;

		std::string strRecogAnswer;
		if (RectCompList.size())
		{
			if (itItem->eCPType == OMR && itItem->nRecogFlag >= 37)	//选项竖直排列
			{
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().y > itItem->rt.tl().y && itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
						{
							char szVal[5] = { 0 };
							sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 							if (strTmpVal.find(szVal) == std::string::npos)
// 								strTmpVal.append(szVal);

							if (strRecogAnswer.find(szVal) == std::string::npos)
								strRecogAnswer.append(szVal);
						}
					}
				}

				//下面逻辑和上面有重复，先测试上面的，没问题就可以删除下面
// 				if (strTmpVal.length())
// 				{
// 					RECTLIST::iterator itItem = lSelInfo.begin();
// 					for (; itItem != lSelInfo.end(); itItem++)
// 					{
// 						for (int i = 0; i < RectCompList.size(); i++)
// 						{
// 							if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().x > itItem->rt.tl().x)
// 							{
// 								if (itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
// 								{
// 									char szVal[2] = { 0 };
// 									sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 									if (strRecogAnswer.find(szVal) == std::string::npos)
// 										strRecogAnswer.append(szVal);
// 								}
// 							}
// 						}
// 					}
// 				}

				//以下可用
// 				RECTLIST::iterator itItem = lSelInfo.begin();
// 				for (; itItem != lSelInfo.end(); itItem++)
// 				{
// 					for (int i = 0; i < RectCompList.size(); i++)
// 					{
// 						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().x > itItem->rt.tl().x)
// 						{
// 							float fThreod = 1.0;
// 							if (RectCompList.size() == 1)		//当只识别出只有一个选项时，降低灰度值标准
// 								fThreod = 0.95;
// 
// 							if (itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
// 							{
// 								char szVal[2] = { 0 };
// 								sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 								if (strRecogAnswer.find(szVal) == std::string::npos)
// 									strRecogAnswer.append(szVal);
// 							}
// 						}
// 					}
// 				}
			}
			else if(itItem->eCPType == OMR && itItem->nRecogFlag < 37)	//选项横向排列
			{
#if 1
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x && itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
						{
							char szVal[5] = { 0 };
							sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 							if (strTmpVal.find(szVal) == std::string::npos)
// 								strTmpVal.append(szVal);

							if (strRecogAnswer.find(szVal) == std::string::npos)
								strRecogAnswer.append(szVal);
						}
					}
				}

// 				if (strTmpVal.length())
// 				{
// 					RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
// 					for (; itItem != omrResult.lSelAnswer.end(); itItem++)
// 					{
// 						for (int i = 0; i < RectCompList.size(); i++)
// 						{
// 							if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x)
// 							{
// 								if (itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
// 								{
// 									char szVal[2] = { 0 };
// 									sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 									if (strRecogAnswer.find(szVal) == std::string::npos)
// 										strRecogAnswer.append(szVal);
// 								}
// 							}
// 						}
// 					}
// 				}
#else
				RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
				for (; itItem != omrResult.lSelAnswer.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x)
						{
							float fThreod = 1.0;
							if (!bFind)								//当单个选项识别出多个小识别块时
							{
								bFind = true;
								if (RectCompList.size() == 1)		//当只识别出只有一个选项时，降低灰度值标准
									fThreod = 0.95;
							}
							else
							{
								fThreod = 0.95;
							}
							
							if (itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
							{
								char szVal[5] = { 0 };
								sprintf_s(szVal, "%c", itItem->nAnswer + 65);
								if (strRecogAnswer.find(szVal) == std::string::npos)
									strRecogAnswer.append(szVal);
							}
						}
					}
				}
#endif
			}
			else if (itItem->eCPType == SN && itItem->nRecogFlag >= 9)	//竖直排列
			{
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().y > itItem->rt.tl().y && itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
						{
							char szVal[5] = { 0 };
							sprintf_s(szVal, "%d", itItem->nSnVal);

							if (strRecogAnswer.find(szVal) == std::string::npos)
								strRecogAnswer.append(szVal);
						}
					}
				}
			}
			else if (itItem->eCPType == SN && itItem->nRecogFlag < 9)
			{
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x && itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
						{
							char szVal[5] = { 0 };
							sprintf_s(szVal, "%d", itItem->nSnVal);

							if (strRecogAnswer.find(szVal) == std::string::npos)
								strRecogAnswer.append(szVal);
						}
					}
				}
			}
		}
		strResult = strRecogAnswer;
	}
	catch (cv::Exception &exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "CRecognizeThread::RecogVal2 error. detail: %s\n", exc.msg);
		g_pLogger->information(szLog);
		TRACE(szLog);
	}

	return bResult;
}

int CRecognizeThread::calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff)
{
	//下面将所有选项识别灰度值降序排列并相邻比较
	RECTLIST::iterator itItem = rectList.begin();
	for (; itItem != rectList.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), SortByItemDensity);

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsDesc[i]->nAnswer + 65), (char)(vecItemsDesc[j]->nAnswer + 65));
			stDiff.fDiff = vecItemsDesc[i]->fRealValuePercent - vecItemsDesc[j]->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}

	return 1;
}

int CRecognizeThread::calcSnDensityDiffVal(pSN_ITEM pSn, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff)
{
	RECTLIST::iterator itItem = pSn->lSN.begin();
	for (; itItem != pSn->lSN.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), SortByItemDensity);

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%d_%d", vecItemsDesc[i]->nSnVal, vecItemsDesc[j]->nSnVal);
			stDiff.fDiff = vecItemsDesc[i]->fRealValuePercent - vecItemsDesc[j]->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
	return 1;
}

int CRecognizeThread::calcOmrGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemGrayDiff)
{
	RECTLIST::iterator itItem = rectList.begin();
	for (; itItem != rectList.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), SortByItemGray);

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsDesc[i]->nAnswer + 65), (char)(vecItemsDesc[j]->nAnswer + 65));
			stDiff.fDiff = abs(vecItemsDesc[i]->fRealMeanGray - vecItemsDesc[j]->fRealMeanGray);
			stDiff.fFirst = vecItemsDesc[i]->fRealMeanGray;
			stDiff.fSecond = vecItemsDesc[j]->fRealMeanGray;
			vecOmrItemGrayDiff.push_back(stDiff);
		}
	}
	return 1;
}

bool CRecognizeThread::Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult_Recog = false;

	if (!pModelInfo->pModel->nHasHead)	//同步头模式不需要判断是否矩形区框选到了其他区域
	{
		try
		{
			Mat matCompRoi;

			if (rc.rt.x < 0) rc.rt.x = 0;
			if (rc.rt.y < 0) rc.rt.y = 0;
			if (rc.rt.br().x > matCompPic.cols)
			{
				rc.rt.width = matCompPic.cols - rc.rt.x;
			}
			if (rc.rt.br().y > matCompPic.rows)
			{
				rc.rt.height = matCompPic.rows - rc.rt.y;
			}
			matCompRoi = matCompPic(rc.rt);

			cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			//图片二值化
			// 局部自适应阈值的图像二值化
			int blockSize = 25;		//25
			int constValue = 10;
			cv::Mat local;
//			cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
			cv::threshold(matCompRoi, matCompRoi, rc.nThresholdValue, 255, THRESH_BINARY);
			cv::Canny(matCompRoi, matCompRoi, 0, _nCannyKernel_, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));	//Size(6, 6)	普通空白框可识别
			dilate(matCompRoi, matCompRoi, element);

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			int nMaxArea = 0;
			cv::Rect rt;
			int i = 0;
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				cv::Rect rm = aRect;

				if (rm.area() > nMaxArea)
				{
					nMaxArea = rm.area();
					rt = rm;
					rt.x += rc.rt.x;
					rt.y += rc.rt.y;
				}
				i++;
			}
			cvReleaseMemStorage(&storage);

#if 1

#else		//这里先不进行选项框的裁剪(裁剪目的是放在选项框到题目或其他选项的矩形区)，起码这里是有问题的，选项有填涂的时候，如果矩形区全都框选到，会识别出多个矩形，这个需要解决
			if (i > 1)
				rc.rt = rt;
#endif
		}
		catch (cv::Exception &exc)
		{
			bResult_Recog = false;
		}
	}
	bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
	return bResult_Recog;
}

bool CRecognizeThread::RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult)
{
	return RecogVal2(nPic, matCompPic, pPic, pModelInfo, omrResult.lSelAnswer, omrResult.strRecogVal2);
}

bool CRecognizeThread::RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal)
{
	std::string strResult;
	RecogVal2(nPic, matCompPic, pPic, pModelInfo, pSn->lSN, strResult);
	char* p = NULL;
	for (int i = 0; i < strResult.length(); i++)
	{
		p = const_cast<char*>(strResult.c_str() + i);
		vecItemVal.push_back(atoi(p));
	}
	return true;
}

bool CRecognizeThread::RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecSN;

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	if (pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.size() == 0)
	{
		strLog = Poco::format("图片%s没有考号需要识别", pPic->strPicName);
		g_pLogger->information(strLog);
		return true;
	}

	SNLIST::iterator itSN = pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.begin();
	for (; itSN != pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.end(); itSN++)
	{
		pSN_ITEM pSnItem = *itSN;

		pSN_ITEM pSn = new SN_ITEM;
		pSn->nItem = pSnItem->nItem;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lSnResult.push_back(pSn);

		std::vector<int> vecItemVal;
		RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
		for (; itSnItem != pSnItem->lSN.end(); itSnItem++)
		{
			RECTINFO rc = *itSnItem;

			if (pModelInfo->pModel->nHasHead)
			{
				if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
				{
					bResult = false;
					pPic->bFindIssue = true;
					pPic->lIssueRect.push_back(rc);
					break;
				}
				rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
				rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
				rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
				rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
			}
			else
				GetRecogPosition(nPic, pPic, pModelInfo->pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
#if 1
			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > rc.fStandardValuePercent)
					vecItemVal.push_back(rc.nSnVal);
			}
			char szTmp[300] = { 0 };
			sprintf_s(szTmp, "图片名: %s, SN: 第%d位, 选项=%d, 识别实际比例=%.3f, val=%.2f, 识别标准=%.3f, val=%.2f, 是否成功:%d\n", pPic->strPicName.c_str(), \
					  pSnItem->nItem, rc.nSnVal, rc.fRealValuePercent, rc.fRealValue, rc.fStandardValuePercent, rc.fStandardValue, rc.fRealValuePercent > rc.fStandardValuePercent);
//			TRACE(szTmp);
#else
			bool bResult_Recog = RecogVal(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog)
			{
				vecItemVal.push_back(rc.nSnVal);
			}
#endif
			pSn->lSN.push_back(rc);

		#ifdef PrintWordRecogPoint
			if (pModelInfo->pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() > 0 && pPic->lModelWordFix.size() > 0)
			{
				VEC_NEWRTBY2FIX vecNewRt;
				RECTLIST::iterator itFix = pPic->lFix.begin();
				RECTLIST::iterator itModelFix = pPic->lModelWordFix.begin();
				itFix++;
				itModelFix++;
				for (int i = 1; itFix != pPic->lFix.end(); itFix++, itModelFix++, i++)
				{
					RECTLIST lTmpFix, lTmpModelFix;
					lTmpFix.push_back(pPic->lFix.front());
					lTmpModelFix.push_back(pPic->lModelWordFix.front());

					lTmpFix.push_back(*itFix);
					lTmpModelFix.push_back(*itModelFix);

					ST_NEWRTBY2FIX stNewRt;
					stNewRt.nFirstFix = 0;
					stNewRt.nSecondFix = i;
					stNewRt.rt = itSnItem->rt;
					GetPosition(lTmpFix, lTmpModelFix, stNewRt.rt);
					vecNewRt.push_back(stNewRt);
				}
				for (auto newRt : vecNewRt)
				{
					RECTINFO rcTmp;
					rcTmp = rc;
					rcTmp.rt = newRt.rt;

					pPic->lCalcRect.push_back(rcTmp);
				}
			}
		#endif
		#ifdef PaintOmrSnRect	//打印OMR、SN位置
			pPic->lNormalRect.push_back(rc);
		#endif
		}
		if (!bResult)
			break;

#if 1	//根据选项差值判断选中
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecSnItemDiff;
		calcSnDensityDiffVal(pSn, vecItemsDesc, vecSnItemDiff);

		float fCompThread = 0.0;		//灰度间隔达到要求时，第一个选项的灰度必须达到的要求
		float fDiffThread = 0.0;		//选项可能填涂的可能灰度梯度阀值
		float fDiffExit = 0;			//灰度的梯度递减太快时，可以认为后面选项没有填涂，此时的灰度梯度阀值
		if (pModelInfo->pModel->nHasHead)
		{
			fCompThread = _dCompThread_Head_;
			fDiffThread = _dDiffThread_Head_;
			fDiffExit = _dDiffExit_Head_;
		}
		else
		{
			fCompThread = _dCompThread_Fix_;
			fDiffThread = _dDiffThread_Fix_;
			fDiffExit = _dDiffExit_Fix_;
		}

		int nFlag = -1;
		float fThreld = 0.0;
#if 1
		vecItemVal.clear();
		if(vecItemsDesc[0]->fRealValuePercent > fCompThread)
		{
			vecItemVal.push_back(vecItemsDesc[0]->nSnVal);
		}
#else
		for (int i = 0; i < vecSnItemDiff.size(); i++)
		{
			//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
			//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
			float fDiff = (fCompThread - vecSnItemDiff[i].fFirst) * 0.1;
			if ((vecSnItemDiff[i].fDiff >= fDiffThread && vecSnItemDiff[i].fFirst > fCompThread) ||
				(vecSnItemDiff[i].fDiff >= fDiffThread + fDiff && vecSnItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecSnItemDiff[i].fFirst;
				if (vecSnItemDiff[i].fDiff > fDiffExit)	//灰度值变化较大，直接退出
					break;
			}
		}

		if (nFlag >= 0)
		{
			vecItemVal.clear();
			RECTLIST::iterator itItem = pSn->lSN.begin();
			for (; itItem != pSn->lSN.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					vecItemVal.push_back(itItem->nSnVal);
				}
			}
		}
#endif
		if (vecItemVal.size() != 1)
		{
			char szTmpLog[300] = { 0 };
			sprintf_s(szTmpLog, "第%d位SN[", pSn->nItem);
			for (int i = 0; i < vecSnItemDiff.size(); i++)
			{
				char szTmp[15] = { 0 };
				sprintf_s(szTmp, "%s:%.5f ", vecSnItemDiff[i].szVal, vecSnItemDiff[i].fDiff);
				strcat_s(szTmpLog, szTmp);
			}
			strcat_s(szTmpLog, "]\n");
			strLog.append(szTmpLog);
		}
#endif

		if (vecItemVal.size() == 1)
		{
			pSn->nRecogVal = vecItemVal[0];
			vecSN.push_back(vecItemVal[0]);
			for(auto itSn : pSn->lSN)
				if (itSn.nSnVal == pSn->nRecogVal)
				{
					pSn->rt = itSn.rt;
					break;
				}
		}
		else
		{
		#if 1	//第二种ZKZH识别方法 test
			std::vector<int> vecItemVal2;
			RecogVal_Sn2(nPic, matCompPic, pPic, pModelInfo, pSn, vecItemVal2);
		#endif

			if (vecItemVal.size() == 0 && vecItemVal2.size() == 1)
			{
				pSn->nRecogVal = vecItemVal2[0];
				vecSN.push_back(vecItemVal2[0]);
				for (auto itSn : pSn->lSN)
					if (itSn.nSnVal == pSn->nRecogVal)
					{
						pSn->rt = itSn.rt;
						break;
					}
			}
			else
			{
				std::vector<int> vecItemVal3;
				RecogVal_Sn3(nPic, matCompPic, pPic, pModelInfo, pSn, vecItemVal3);
				if (vecItemVal3.size())
				{
					pSn->nRecogVal = vecItemVal3[0];
					vecSN.push_back(vecItemVal3[0]);
					for (auto itSn : pSn->lSN)
						if (itSn.nSnVal == pSn->nRecogVal)
						{
							pSn->rt = itSn.rt;
							break;
						}
				}
				else
				{
					bRecogAll = false;
					char szVal[21] = { 0 };
					for (int i = 0; i < vecItemVal.size(); i++)
					{
						char szTmp[3] = { 0 };
						sprintf_s(szTmp, "%d ", vecItemVal[i]);
						strcat(szVal, szTmp);
					}
					char szLog[MAX_PATH] = { 0 };
					sprintf_s(szLog, "识别准考证号第%d位失败,识别出结果%d位(%s), 图片名: %s\n", pSnItem->nItem, vecItemVal.size(), szVal, pPic->strPicName.c_str());
					strLog.append(szLog);
					TRACE(szLog);

					vecSN.push_back(-1);
				}
			}
		}
	}
	if (bRecogAll && vecSN.size() > 0)
	{
		for (int i = 0; i < vecSN.size(); i++)
		{
			char szTmp[5] = { 0 };
			itoa(vecSN[i], szTmp, 10);
			(static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.append(szTmp);
		}
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别准考证号完成(%s), 图片名: %s\n", (static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.c_str(), pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
		(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
	}
	else
	{
		bool bAllEmpty = true;
		for (int i = 0; i < vecSN.size(); i++)
		{
			char szTmp[5] = { 0 };
			if (vecSN[i] >= 0)
			{
				itoa(vecSN[i], szTmp, 10);
				bAllEmpty = false;
			}
			else
				szTmp[0] = '#';		//未识别出来的或者识别到多个的用#代替，后面做模糊查找
			if (!bAllEmpty)			//只要识别到一部分，就将此识别到的结果放入模糊搜索字段中
				(static_cast<pST_PaperInfo>(pPic->pPaper))->strRecogSN4Search.append(szTmp);
		}
		(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
	}
	if (!bRecogAll)
	{
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
	}
	if (!bResult)
	{
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();

		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别准考证号失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别考号时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	bool bResult = true;
	SNLIST::iterator itSN = pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.begin();
	for (; itSN != pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.end(); itSN++)
	{
		pSN_ITEM pSnItem = *itSN;
		
		pSN_ITEM pSn = new SN_ITEM;
		pSn->nItem = 0;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lSnResult.push_back(pSn);

		RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
		for (; itSnItem != pSnItem->lSN.end(); itSnItem++)
		{
			RECTINFO rc = *itSnItem;
			pSn->lSN.push_back(rc);

			try
			{
				if (rc.rt.x < 0) rc.rt.x = 0;
				if (rc.rt.y < 0) rc.rt.y = 0;
				if (rc.rt.br().x > matCompPic.cols)
					rc.rt.width = matCompPic.cols - rc.rt.x;
				if (rc.rt.br().y > matCompPic.rows)
					rc.rt.height = matCompPic.rows - rc.rt.y;

				Mat matCompRoi;
				matCompRoi = matCompPic(rc.rt);

				cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

				string strTypeName;
				string strResult = GetQR(matCompRoi, strTypeName);

				if (strResult.empty() && _nUseOcrRecogSn_)
				{
					RecogSN_code_Character(matCompPic, rc, pPic, pModelInfo->pModel->vecPaperModel[nPic]->nPicSaveRotation, strResult);
				}

				std::string strTmpLog;
				if (strResult != "")
				{
					strTmpLog = "识别准考证号完成(" + strResult + "), 图片名: " + pPic->strPicName;
					(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
				}
				else
				{
					strTmpLog = "识别准考证号失败, 图片名:" + pPic->strPicName;
//					bResult = false;
				}
				(static_cast<pST_PaperInfo>(pPic->pPaper))->strSN = strResult;
				strLog.append(strTmpLog);
			}
			catch (cv::Exception& exc)
			{
				std::string strTmpLog = "识别二维码或条码失败(" + pPic->strPicName + "): " + exc.msg;
				strLog.append(strTmpLog);
				break;
			}
			catch (...)
			{
				std::string strTmpLog = "识别二维码或条码失败(" + pPic->strPicName + "), 未知异常.\n";
				strLog.append(strTmpLog);
				break;
			}
		}
	}
	if ((static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.empty())
	{
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
	}
	end = clock();
	std::string strTime = Poco::format("识别考号时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogSN_code_Character(cv::Mat& matCompPic, RECTINFO rc, pST_PicInfo pPic, int nPicSrcRotation, std::string& strRecogSN)
{
	clock_t sT, eT, mT1, mT2, mT3;
	sT = clock();

	//提取条码下面的考号
	Mat matCompRoi;
	matCompRoi = matCompPic(rc.rt);

	cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);
	GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);
	sharpenImage1(matCompRoi, matCompRoi);
	int blockSize = 25;		//25
	int constValue = 10;
	cv::Mat local;
	//cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);

	threshold(matCompRoi, matCompRoi, 130, 255, THRESH_BINARY);

	cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);
#if 1
	Mat element = getStructuringElement(MORPH_RECT, Size(4, 4));	//Size(6, 6)	普通空白框可识别
	dilate(matCompRoi, matCompRoi, element);

	Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(2, 2));
	//erode(matCompRoi, matCompRoi, element_Anticlutter);

	IplImage ipl_img(matCompRoi);

	std::vector<Rect>RectCompList;
	//the parm. for cvFindContours  
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;

	Mat matTmpShow;
	matTmpShow = matCompPic(rc.rt);
	//提取轮廓  
	cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
	{
		CvRect aRect = cvBoundingRect(contour, 0);
		Rect rm = aRect;
		int nMaxW = rc.rt.width * 0.3;
		int nMaxH = rc.rt.height * 0.5;
		if (rm.width < 15 || rm.height < 20 ||rm.x > rc.rt.width * 0.5 || rm.y > rc.rt.height * 0.5)
			continue;
		if (rm.width > nMaxW || rm.height > nMaxH || rm.height < rm.width * 1.5 || rm.width < 20 || rm.height > rm.width * 6 || \
			rm.x > rc.rt.width * 0.5 || rm.y > rc.rt.height * 0.5)
		{
			Mat matTmpGray = matCompPic(rm + rc.rt.tl());
			MatND mean;
			MatND stddev;
			meanStdDev(matTmpGray, mean, stddev);

			IplImage *src;
			src = &IplImage(mean);
			float fRealMeanGray = cvGetReal2D(src, 0, 0);
			rectangle(matTmpShow, rm, CV_RGB(255, 0, 0), 2);
			continue;
		}

		rm = rm + rc.rt.tl();	// rc.rt.tl();
		RectCompList.push_back(rm);
	}
	cvReleaseMemStorage(&storage);
#else
	Mat element = getStructuringElement(MORPH_RECT, Size(4, 4));	//Size(6, 6)	普通空白框可识别
	dilate(matCompRoi, matCompRoi, element);
	IplImage ipl_img(matCompRoi);

	std::vector<Rect>RectCompList;
	//the parm. for cvFindContours  
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	//提取轮廓  
	cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
	{
		CvRect aRect = cvBoundingRect(contour, 0);
		Rect rm = aRect;
		int nMaxW = rc.rt.width * 0.3;
		int nMaxH = rc.rt.height * 0.5;
		if(rm.width > nMaxW || rm.height > nMaxH || rm.height < rm.width * 5 || rm.width < 10)
			continue;

		rm = rm + rc.rt.tl();	// rc.rt.tl();
		RectCompList.push_back(rm);
	}
	cvReleaseMemStorage(&storage);
#endif
	if (RectCompList.size() == 0)
		return false;

	Mat matNameRoi;
	matNameRoi = matCompPic(RectCompList[0]);
	
	cvtColor(matNameRoi, matNameRoi, CV_BGR2GRAY);
// 	GaussianBlur(matZkzhRoi, matZkzhRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);
// 	sharpenImage1(matZkzhRoi, matZkzhRoi);

	cv::adaptiveThreshold(matNameRoi, matNameRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, 10);	//blockSize, constValue

	mT1 = clock();

	switch (nPicSrcRotation)
	{
		case 1:	break;
		case 3:			//图像需要调整到正常的视觉方向(作答方向)，则模板是右旋90，则实际图片需左旋90
			{
				cv::Mat dst;
				transpose(matNameRoi, dst);	//左旋90，镜像 
				flip(dst, matNameRoi, 0);	//左旋90，模板图像需要右旋90，原图即需要左旋90
			}
			break;
		case 2:			//图像需要调整到正常的视觉方向(作答方向)，则模板是左旋90，则实际图片需右旋90
			{
				cv::Mat dst;
				transpose(matNameRoi, dst);	//左旋90，镜像 
				flip(dst, matNameRoi, 1);	//右旋90，模板图像需要左旋90，原图即需要右旋90
			}
			break;
		case 4:
			{
				cv::Mat dst;
				transpose(matNameRoi, dst);	//左旋90，镜像 
				cv::Mat dst2;
				flip(dst, dst2, 1);
				cv::Mat dst5;
				transpose(dst2, dst5);
				flip(dst5, matNameRoi, 1);	//右旋180
			}
			break;
		default:
			break;
	}

	mT2 = clock();

	std::vector<std::string> vecWord;
#ifdef USE_TESSERACT
	try
	{
		m_pTess->SetImage((uchar*)matNameRoi.data, matNameRoi.cols, matNameRoi.rows, 1, matNameRoi.cols);
		m_pTess->Recognize(0);

		tesseract::ResultIterator* ri = m_pTess->GetIterator();
		tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;	//RIL_WORD	RIL_SYMBOL
		if (ri != 0)
		{
			do
			{
				const char* word = ri->GetUTF8Text(level);
				float conf = ri->Confidence(level);
				if (word && strcmp(word, " ") != 0 && conf >= 0.5)
				{
					vecWord.push_back(CMyCodeConvert::Utf8ToGb2312(word));
					strRecogSN.append(CMyCodeConvert::Utf8ToGb2312(word));
				}
			} while (ri->Next(level));
		}
// 		m_pTess->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);	//PSM_SINGLE_BLOCK
// 		m_pTess->SetImage((uchar*)matNameRoi.data, matNameRoi.cols, matNameRoi.rows, 1, matNameRoi.cols);
// 		char* out = m_pTess->GetUTF8Text();
// 		strRecogSN = CMyCodeConvert::Utf8ToGb2312(out);
	}
	catch (...)
	{
		std::string strTmpLog;
		strTmpLog = "识别ocr异常.\n";
	}
#endif
	mT3 = clock();

	bool bFind = false;
	int nMaxTime = vecWord.size() > 5 ? 5 : vecWord.size();
	for (int i = 0; i <= nMaxTime; i++)
	{
		STUDENT_LIST lResult;
		std::string strTable = Poco::format("T%d_%d", _pModel_->nExamID, _pModel_->nSubjectID);
		if (m_pStudentMgr && !strRecogSN.empty() && strRecogSN != "%" && m_pStudentMgr->SearchStudent(strTable, strRecogSN, 1, lResult))
		{
			if (lResult.size() >= 1 && lResult.size() <= 2)
			{
				bFind = true;
				STUDENT_LIST::iterator itStudent = lResult.begin();
				strRecogSN = itStudent->strZkzh;
				break;
			}
		}
		strRecogSN = "";
		for (int j = 0; j < vecWord.size(); j++)
		{
			if (j != i)
				strRecogSN.append(vecWord[j]);
			else
				strRecogSN.append("%");
		}
	}
	if (!bFind)
	{
		strRecogSN = "";
		for (int j = 0; j < vecWord.size(); j++)
			strRecogSN.append(vecWord[j]);
		(static_cast<pST_PaperInfo>(pPic->pPaper))->strRecogSN4Search = strRecogSN;
	}
	eT = clock();

	USES_CONVERSION;
	TRACE(_T("考号识别(文字识别): %s, %d:%d:%d:%d == %d\n"), A2T(strRecogSN.c_str()), (int)(mT1 - sT), (int)(mT2 - mT1), (int)(mT3 - mT2), (int)(eT - mT3), (int)(eT - sT));
	return true;
}

bool CRecognizeThread::RecogVal_ElectOmr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pELECTOMR_QUESTION pOmrQuestion, OMR_RESULT& omrResult)
{
	try
	{
		RECTLIST::iterator itOmrItem = pOmrQuestion->lItemInfo.begin();
		for (; itOmrItem != pOmrQuestion->lItemInfo.end(); itOmrItem++)
		{
			RECTINFO rc = *itOmrItem;

			cv::Point pt1, pt2;
			pt1 = rc.rt.tl() - cv::Point(3, 3);
			pt2 = rc.rt.br() + cv::Point(3, 3);
			Mat matCompRoi;
			matCompRoi = matCompPic(cv::Rect(pt1, pt2));
			cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			//图片二值化
			threshold(matCompRoi, matCompRoi, _nThreshold_Recog2_, 255, THRESH_BINARY_INV);				//200, 255

			//这里进行开闭运算
			//确定腐蚀和膨胀核的大小
			Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
			//腐蚀操作
			erode(matCompRoi, matCompRoi, element);

			IplImage ipl_img2(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage2 = cvCreateMemStorage(0);
			CvSeq* contour2 = 0;

			//提取轮廓  
			cvFindContours(&ipl_img2, storage2, &contour2, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			std::vector<Rect>RectCompList;
			for (int iteratorIdx = 0; contour2 != 0; contour2 = contour2->h_next, iteratorIdx++)
			{
				CvRect aRect = cvBoundingRect(contour2, 0);
				cv::Rect rm = aRect;
				rm.x += pt1.x;
				rm.y += pt1.y;

				RectCompList.push_back(rm);
			}
			cvReleaseMemStorage(&storage2);
		}
	}
	catch (cv::Exception& exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "CRecognizeThread::RecogVal_ElectOmr2 error. detail: %s\n", exc.msg);
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return true;
}

void CRecognizeThread::InitCharacterRecog()
{
#ifdef USE_TESSERACT
	m_pTess = new tesseract::TessBaseAPI();
	m_pTess->Init(NULL, "chi_sim", tesseract::OEM_DEFAULT);
#endif
}

void CRecognizeThread::MergeScanPaper(pPAPERSINFO pPapers, pMODEL pModel)
{
	if (!pPapers) return ;
	if (pModel->nUsePagination == 0) return ;

	//全部识别完成才进行合并操作
	bool bRecogComplete = true;
	for (auto objPaper : pPapers->lPaper)
	{
		if (!objPaper->bRecogComplete)
		{
			bRecogComplete = false;
			break;
		}
	}
	if (!bRecogComplete) return ;

	TRACE("开始进行试卷合并。。。\n");
	pPapers->nPaperScanMergerStatus = 1;

	//多页模式时，每个考生的试卷可能是乱的，需要把每张试卷的合并到对应考生
	pPAPERSINFO pNewPapers = new PAPERSINFO();
	pNewPapers->strPapersName = pPapers->strPapersName;
	pNewPapers->strPapersDesc = pPapers->strPapersDesc;
	pNewPapers->nOmrDoubt = pPapers->nOmrDoubt;
	pNewPapers->nOmrNull = pPapers->nOmrNull;
	PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
	for (; itPaper != pPapers->lPaper.end(); )
	{
		pST_PaperInfo pCurrentPaper = *itPaper;
		bool bFindSN = false;
		pST_PaperInfo pNewPaper = NULL;
		for (auto objPaper : pNewPapers->lPaper)
		{
			if (!objPaper->strSN.empty() && objPaper->strSN == pCurrentPaper->strSN && objPaper != pCurrentPaper)
			{
				bFindSN = true;
				pNewPaper = objPaper;
				break;
			}
		}
		if (!bFindSN)
		{
			//将此试卷信息放入新试卷袋中
			pNewPapers->lPaper.push_back(pCurrentPaper);
			//pCurrentPaper->pPapers = pNewPapers;
			if(pCurrentPaper->nPaginationStatus == 1)
				pCurrentPaper->nPaginationStatus = 2;
			pCurrentPaper->nIndex = pNewPapers->lPaper.size();
			pCurrentPaper->strStudentInfo = Poco::format("S%d", pCurrentPaper->nIndex);

			itPaper = pPapers->lPaper.erase(itPaper);
		}
		else
		{
			//试卷的信息合并到新试卷中
			if (!pNewPaper->bIssuePaper)	pNewPaper->bIssuePaper = pCurrentPaper->bIssuePaper;
			if (!pNewPaper->bModifyZKZH)	pNewPaper->bModifyZKZH = pCurrentPaper->bModifyZKZH;
			if (!pNewPaper->bReScan)		pNewPaper->bReScan = pCurrentPaper->bReScan;
			if (!pNewPaper->bRecogCourse)	pNewPaper->bRecogCourse = pCurrentPaper->bRecogCourse;
			pNewPaper->nPicsExchange += pCurrentPaper->nPicsExchange;
			pNewPaper->nPaginationStatus = 2;
			if (pNewPaper->nQKFlag == 0)	pNewPaper->nQKFlag = pCurrentPaper->nQKFlag;
			if (pNewPaper->nWJFlag == 0)	pNewPaper->nWJFlag = pCurrentPaper->nWJFlag;
			pNewPaper->nZkzhInBmkStatus = pCurrentPaper->nZkzhInBmkStatus;
			//pNewPaper->strRecogSN4Search
			//pNewPaper->lSnResult

			SCAN_PAPER_LIST::iterator itSrcScanPaper = pCurrentPaper->lSrcScanPaper.begin();
			for (; itSrcScanPaper != pCurrentPaper->lSrcScanPaper.end();)
			{
				pST_SCAN_PAPER pScanPaper = *itSrcScanPaper;
				pNewPaper->lSrcScanPaper.push_back(pScanPaper);
				itSrcScanPaper = pCurrentPaper->lSrcScanPaper.erase(itSrcScanPaper);
			}
			for (auto omrObj : pCurrentPaper->lOmrResult)
				pNewPaper->lOmrResult.push_back(omrObj);
			for (auto electOmrObj : pCurrentPaper->lElectOmrResult)
				pNewPaper->lElectOmrResult.push_back(electOmrObj);

			PIC_LIST::iterator itPic = pCurrentPaper->lPic.begin();
			for (; itPic != pCurrentPaper->lPic.end(); )
			{
				pST_PicInfo pPic = *itPic;
				itPic = pCurrentPaper->lPic.erase(itPic);

				pPic->pPaper = pNewPaper;
				bool bInsert = false;
				PIC_LIST::iterator itNewPic = pNewPaper->lPic.begin();
				for (; itNewPic != pNewPaper->lPic.end(); itNewPic++)
				{
					if ((*itNewPic)->nPicModelIndex > pPic->nPicModelIndex)
					{
						bInsert = true;
						pNewPaper->lPic.insert(itNewPic, pPic);
						break;
					}
				}
				if (!bInsert)
					pNewPaper->lPic.push_back(pPic);
			}
			if (pCurrentPaper->strSN.empty())
				pNewPapers->nSnNull++;

			itPaper++;
		}
	}
	pNewPapers->nPaperCount = pNewPapers->lPaper.size();
	
	PAPER_LIST::iterator itPaper1 = pPapers->lPaper.begin();
	for (; itPaper1 != pPapers->lPaper.end();)
	{
		pST_PaperInfo pPaper = *itPaper1;
		SAFE_RELEASE(pPaper);
		itPaper1 = pPapers->lPaper.erase(itPaper1);
	}
	PAPER_LIST::iterator itPaper2 = pPapers->lIssue.begin();
	for (; itPaper2 != pPapers->lIssue.end();)
	{
		pST_PaperInfo pPaper = *itPaper2;
		SAFE_RELEASE(pPaper);
		itPaper2 = pPapers->lIssue.erase(itPaper2);
	}
	pPapers->nPaperCount = pNewPapers->nPaperCount;
	pPapers->nRecogErrCount = pNewPapers->nRecogErrCount;
	pPapers->strPapersName = pNewPapers->strPapersName;
	pPapers->strPapersDesc = pNewPapers->strPapersDesc;
	pPapers->nOmrDoubt = pNewPapers->nOmrDoubt;
	pPapers->nOmrNull = pNewPapers->nOmrNull;
	PAPER_LIST::iterator itPaper3 = pNewPapers->lPaper.begin();
	for (; itPaper3 != pNewPapers->lPaper.end(); )
	{
		pST_PaperInfo pCurrentPaper = *itPaper3;
		pPapers->lPaper.push_back(pCurrentPaper);
		itPaper3 = pNewPapers->lPaper.erase(itPaper3);
	}
	PAPER_LIST::iterator itPaper4 = pNewPapers->lIssue.begin();
	for (; itPaper4 != pNewPapers->lIssue.end(); )
	{
		pST_PaperInfo pCurrentPaper = *itPaper4;
		pPapers->lPaper.push_back(pCurrentPaper);
		itPaper4 = pNewPapers->lIssue.erase(itPaper4);
	}
	//memcpy(pPapers, pNewPapers, sizeof(PAPERSINFO));
	//pNewPapers->lPaper.clear();
	//pNewPapers->lIssue.clear();
	SAFE_RELEASE(pNewPapers);

	//试卷合法性检测，即检查测每个考生的每张试卷是否都是合法的
	PAPER_LIST::iterator itPaperValid = pPapers->lPaper.begin();
	for (; itPaperValid != pPapers->lPaper.end(); itPaperValid++)
	{
		pST_PaperInfo pCurrentPaper = *itPaperValid;
		if(pCurrentPaper->nPaginationStatus == 0) continue;		//没有识别到页码的不检查试卷合法性，在人工设置完页码后再做处理
		if (pCurrentPaper->lPic.size() != pModel->vecPaperModel.size())
			pCurrentPaper->nPaginationStatus = 3;
		else
		{
			//图片的页码是否存在重复的
			int nLastPicIndex = -1;
			PIC_LIST::iterator itPic = pCurrentPaper->lPic.begin();		//图片在插入的时候已经初步排序
			for (; itPic != pCurrentPaper->lPic.end(); itPic++)
			{
				pST_PicInfo pPic = *itPic;
				if (nLastPicIndex == pPic->nPicModelIndex)
				{
					pCurrentPaper->nPaginationStatus = 4;
					break;
				}
				nLastPicIndex = pPic->nPicModelIndex;
			}
		}
	}

	//图片重命名
	std::string strLog;
	PAPER_LIST::iterator itReNamePaper = pPapers->lPaper.begin();
	for (; itReNamePaper != pPapers->lPaper.end(); itReNamePaper++)
	{
		pST_PaperInfo pCurrentPaper = *itReNamePaper;

		PIC_LIST::iterator itPic = pCurrentPaper->lPic.begin();
		for (; itPic != pCurrentPaper->lPic.end(); itPic++)
		{
			pST_PicInfo pPic = *itPic;
			std::string strNewPicName = Poco::format("%s_%d.jpg", pCurrentPaper->strStudentInfo, pPic->nPicModelIndex + 1);
			int nPos = pPic->strPicPath.rfind('\\') + 1;
			std::string strBasePath = pPic->strPicPath.substr(0, nPos);
			std::string strNewPath = strBasePath + strNewPicName;
			if (strNewPicName != pPic->strPicName)
			{
				try
				{
					Poco::File fNewPic(CMyCodeConvert::Gb2312ToUtf8(pPic->strPicPath));
					fNewPic.renameTo(CMyCodeConvert::Gb2312ToUtf8(strNewPath));
					pPic->strPicPath = strNewPath;
					pPic->strPicName = strNewPicName;
				}
				catch (Poco::Exception& exc)
				{
					std::string strTmpLog;
					strTmpLog = Poco::format("原始图片(%s)重命名为(%s)失败, 原因: %s\n", pPic->strPicName, strNewPicName, std::string(exc.what()));
					strLog.append(strTmpLog);
				}
			}
		}
	}
	pPapers->nPaperScanMergerStatus = 2;

	if(!strLog.empty())
		g_pLogger->information(strLog);
}

bool CRecognizeThread::RecogPagination(pST_SCAN_PAPER pScanPaperTask, pMODEL pModel)
{
	TRACE("识别页码\n");
	clock_t start, end;
	start = clock();

	bool bResult = true;
	if (pModel->nUsePagination == 0)
	{
		for (int i = 0; i < pScanPaperTask->vecScanPic.size(); i++)
			pScanPaperTask->vecScanPic[i]->nModelPicID = i;
		return true;
	}

// 	if (pModel->vecPaperModel.size() <= 2)	//模板图片数量 <= 2，必须用计算这张试卷属于模板的第几张
// 	{
// 		for (int i = 0; i < pScanPaperTask->vecScanPic.size(); i++)
// 			pScanPaperTask->vecScanPic[i]->nModelPicID = i;
// 		return true;
// 	}

	std::stringstream ssLog;
	if (pScanPaperTask->vecScanPic.size() > 2)
	{
		ssLog << "扫描试卷(P" << pScanPaperTask->nPaperID << ")的图片数(" << pScanPaperTask->vecScanPic.size() << "页)超过2页, 不进行判断\n";
		TRACE(ssLog.str().c_str());
		return true;
	}
	ssLog << "开始判断试卷(P" << pScanPaperTask->nPaperID << ")属于模板的第几张试卷:\n";

	bool bFindPagination = false;	//是否找到页码属于第几页试卷
	//识别每页的定点，然后根据透视变换反向计算实际页码点的位置
	for(int i = 0; i < pScanPaperTask->vecScanPic.size(); i++)
	{
		pST_SCAN_PIC pScanPic = pScanPaperTask->vecScanPic[i];

// 		if (bFindPagination)
// 		{
// 			bFindPagination = false;
// 			pScanPic->nModelPicID = pScanPaperTask->vecScanPic[i - 1]->nModelPicID + 1;
// 			ssLog << "已经判断到试卷的第" << i << "页属于用模板的第" << pScanPaperTask->vecScanPic[i - 1]->nModelPicID + 1 << "页, 故该试卷的第" << i + 1 \\
// 				<< "页属于模板的第" << pScanPic->nModelPicID + 1 << "页\n";
// 			continue;
// 		}

		for (int j = 0; j < pModel->vecPaperModel.size(); j++)
		{
			clock_t sT1, sT2, sT3, sT4, sT5, sT6, eT1;
			sT1 = clock();

			RECTLIST lFixResult;
			RecogFixCP2(j, pScanPic->mtPic, NULL, pModel, lFixResult);
			sT2 = clock();
		#if 1
			if (lFixResult.size() == 0)
			{
				ssLog << "检测试卷的第" << i + 1 << "页是否属于用模板的第" << j + 1 << "页时，检测到定点数 = 0, 不需要后续比较，直接检测模板的下一页\n";
				continue;
			}

			COmrRecog omrReocgObj;
			cv::Mat rot_mat_inv = omrReocgObj.GetRotMat(lFixResult, pModel->vecPaperModel[j]->lFix);
			sT3 = clock();

			ssLog<<"获取定点和旋转矩阵完毕. "<< sT2 - sT1 << ":" << sT3 - sT2 << "(定:旋)ms\n";

			int nRecogPagination = 0;	//已经识别到的页码点
			RECTLIST::iterator itPage = pModel->vecPaperModel[j]->lPagination.begin();
			for (int k = 0; itPage != pModel->vecPaperModel[j]->lPagination.end(); itPage++, k++)
			{
				RECTINFO rc = *itPage;
				cv::Rect rtPageCode = rc.rt;
				cv::Rect rtRealPic = omrReocgObj.GetRealRtFromModel(rtPageCode, lFixResult, pModel->vecPaperModel[j]->lFix, rot_mat_inv);
				rc.rt = rtRealPic;
				bool bResult_Recog = Recog(j, rc, pScanPic->mtPic, NULL, NULL);
				if (bResult_Recog)
				{
					if (rc.fRealValuePercent >= rc.fStandardValuePercent)
					{
						nRecogPagination++;
					}
					else
					{
						char szLog[MAX_PATH] = { 0 };
						sprintf_s(szLog, "试卷(第%d页)尝试识别模板(第%d页)的页码点失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", i + 1, j + 1, rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
						//ssLog << szLog;
						TRACE(szLog);
					}
				}
			}
			sT4 = clock();
			if (nRecogPagination == pModel->vecPaperModel[j]->lPagination.size())
			{
				bFindPagination = true;
				pScanPaperTask->nModelPaperID = j / 2;
				pScanPic->nModelPicID = j;
				if(i % 2 == 0)		//设置这张试卷的另一页的图片属于模板的第几页
					pScanPaperTask->vecScanPic[i + 1]->nModelPicID = j + 1;
				else
					pScanPaperTask->vecScanPic[i - 1]->nModelPicID = j - 1;
				ssLog << "检测到试卷的第" << i + 1 << "页属于用模板的第" << j + 1 << "页, 下一页试卷不需要判断, " << sT4 - sT3 << "ms\n";
				break;
			}
			else
			{
				ssLog << "检测试卷的第" << i + 1 << "页是否属于用模板的第" << j + 1 << "页时，检测到页码" << nRecogPagination << "个，实际模板上有" 
					<< pModel->vecPaperModel[j]->lPagination.size() << "个，不符合条件，进行下一页模板比较. " << sT4 - sT3 << "ms\n";
			}
		#else
			//透视变换
			std::vector<cv::Point2f> vecFixPt;
			RECTLIST::iterator itCP = pModel->vecPaperModel[j]->lFix.begin();
			for (; itCP != pModel->vecPaperModel[j]->lFix.end(); itCP++)
			{
				cv::Point2f pt;
				pt.x = itCP->rt.x + itCP->rt.width / 2;
				pt.y = itCP->rt.y + itCP->rt.height / 2;
				vecFixPt.push_back(pt);
			}
			std::vector<cv::Point2f> vecFixNewPt;
			RECTLIST::iterator itCP2 = lFixResult.begin();
			for (; itCP2 != lFixResult.end(); itCP2++)
			{
				cv::Point2f pt;
				pt.x = itCP2->rt.x + itCP2->rt.width / 2;
				pt.y = itCP2->rt.y + itCP2->rt.height / 2;
				vecFixNewPt.push_back(pt);
			}

			cv::Point2f srcTri[4];
			cv::Point2f dstTri[4];
			cv::Mat warp_mat(3, 3, CV_32FC1);		//warp_mat(2, 3, CV_32FC1);
			cv::Mat warp_dst, warp_rotate_dst;
			for (int i = 0; i < vecFixPt.size(); i++)
			{
				srcTri[i] = vecFixNewPt[i];
				dstTri[i] = vecFixPt[i];
			}
			sT3 = clock();
			warp_mat = cv::getPerspectiveTransform(srcTri, dstTri);
			sT4 = clock();

			//cv::Mat matPerspective = pScanPic->mtPic.clone();
			//cv::warpPerspective(matPerspective, matPerspective, warp_mat, matPerspective.size(), 1, 0, cv::Scalar(255, 255, 255));
			//反向计算模板上点对应的实际图像的点
			cv::Mat warp_mat_inv = warp_mat.inv();	//逆矩阵

			sT5 = clock();

			std::vector<Point2f> vecPoint;
			RECTLIST::iterator itPage = pModel->vecPaperModel[j]->lPagination.begin();
			for(int k = 0; itPage != pModel->vecPaperModel[j]->lPagination.end(); itPage++, k++)
			{
				Point2f pt1 = (*itPage).rt.tl();
				Point2f pt2 = (*itPage).rt.br();
				Point2f p1 = Point2f(0, 0);
				Point2f p2 = Point2f(0, 0);
				p1.x = warp_mat_inv.ptr<double>(0)[0] * pt1.x + warp_mat_inv.ptr<double>(0)[1] * pt1.y + warp_mat_inv.ptr<double>(0)[2];
				p1.y = warp_mat_inv.ptr<double>(1)[0] * pt1.x + warp_mat_inv.ptr<double>(1)[1] * pt1.y + warp_mat_inv.ptr<double>(1)[2];

				p2.x = warp_mat_inv.ptr<double>(0)[0] * pt2.x + warp_mat_inv.ptr<double>(0)[1] * pt2.y + warp_mat_inv.ptr<double>(0)[2];
				p2.y = warp_mat_inv.ptr<double>(1)[0] * pt2.x + warp_mat_inv.ptr<double>(1)[1] * pt2.y + warp_mat_inv.ptr<double>(1)[2];
				vecPoint.push_back(p1);
				vecPoint.push_back(p2);

				//cv::rectangle(matPerspective, (*itPage).rt, CV_RGB(255, 0, 0), 2);
			}
			eT1 = clock();
			std::string strTmpLog = Poco::format("各阶段耗时: %d:%d:%d:%d:%d, all: %dms\n", (int)(sT2 - sT1), (int)(sT3 - sT2), (int)(sT4 - sT3), (int)(sT5 - sT4), (int)(eT1 - sT5), (int)(eT1 - sT1));
			TRACE(strTmpLog.c_str());

			//标出关键点
			for (int m = 0; m < vecPoint.size(); m++)
			{
				circle(pScanPic->mtPic, vecPoint[m], 2, Scalar(255, 0, 0));
				//circle(matPerspective, vecPoint[m], 2, Scalar(255, 0, 0));
			}
		#endif
		}
		//在当前页已经发现页码匹配了，不需要进行第二页的页码检查
		if (bFindPagination)
			break;
		else
			ssLog << "试卷的第" << i + 1 << "页在模板上没有发现匹配的页面，开始下一页试卷在模板上的比较\n";
	}
	end = clock();
	if (!bFindPagination)
	{
		//根据页码查找此试卷属于模板的第几张试卷失败
		pScanPaperTask->bCanRecog = false;		//设置不可识别，在人工确认后再识别
		bResult = false;
		ssLog << "判断完成, 试卷(P" << pScanPaperTask->nPaperID << ")无法确定属于模板的第几张试卷, 此试卷不参与识别，后续人工判定再识别. " << end - start << "ms\n";
	}
	else
		ssLog<< "判断完成, 试卷(P" << pScanPaperTask->nPaperID << ")属于模板的第" << pScanPaperTask->nModelPaperID + 1 << "张试卷. " << end - start << "ms\n";

	g_pLogger->information(ssLog.str());
	TRACE(ssLog.str().c_str());
	return bResult;
}

bool CRecognizeThread::RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult)
{
	std::vector<int> vecVal_AnswerSuer;
	RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
	for (; itItem != omrResult.lSelAnswer.end(); itItem++)
	{
		if (itItem->fRealMeanGray < _dAnswerSure_)
		{
			vecVal_AnswerSuer.push_back(itItem->nAnswer);
		}
	}

	std::string strRecogAnswer;
	std::vector<pRECTINFO> vecItemsGrayDesc;
	std::vector<ST_ITEM_DIFF> vecOmrItemGrayDiff;
	calcOmrGrayDiffVal(omrResult.lSelAnswer, vecItemsGrayDesc, vecOmrItemGrayDiff);

	float fCompThread = 0.0;		//灰度间隔达到要求时，第一个选项的灰度必须达到的要求
	float fDiffThreshold = 0.0;		//选项可能填涂的可能灰度梯度阀值
	float fDiffExit = 0;			//灰度的梯度递减太快时，可以认为后面选项没有填涂，此时的灰度梯度阀值

	fCompThread = _dCompThread_3_;
	fDiffThreshold = _dDiffThread_3_;
	fDiffExit = _dDiffExit_3_;


#if 1		//灰度差值满足后，加上单项Omr的灰度需要比标准的小
	float fMeanGrayDiff = 0.0;
	for (int i = 0; i < vecItemsGrayDesc.size(); i++)
	{
		fMeanGrayDiff += (vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray);
	}
	fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayDesc.size();

	int nFlag = -1;
	float fThreld = 0.0;
	float fGrayDiffLast = 0.0;		//对上一次判断选中的选项对下一个选项选中判断的增益
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		float fGrayThresholdGray = vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray - fMeanGrayDiff;
		if (vecOmrItemGrayDiff[i].fDiff >= fDiffThreshold + fGrayThresholdGray + fGrayDiffLast)		//vecOmrItemGrayDiff[i].fDiff >= fDiffThreshold + fGrayThresholdGray
		{
			nFlag = i;
			fThreld = vecOmrItemGrayDiff[i].fFirst;
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
				break;
		}
		fGrayDiffLast += abs(fGrayThresholdGray) / 2;
	}
	if (nFlag >= 0)
	{
		//++判断全都选中的情况
		if (nFlag == vecOmrItemGrayDiff.size() - 1)
		{
			if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)
			{
				fThreld = vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
			}
		}
		//--
		RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
		for (; itItem != omrResult.lSelAnswer.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				char szVal[2] = { 0 };
				sprintf_s(szVal, "%c", itItem->nAnswer + 65);
				strRecogAnswer.append(szVal);
			}
		}
	}
	else if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)		//++判断全都选中的情况
	{
		fThreld = vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
		RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
		for (; itItem != omrResult.lSelAnswer.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				char szVal[2] = { 0 };
				sprintf_s(szVal, "%c", itItem->nAnswer + 65);
				strRecogAnswer.append(szVal);
			}
		}
	}
	else
	{
		for (int i = 0; i < vecVal_AnswerSuer.size(); i++)
		{
			char szVal[5] = { 0 };
			sprintf_s(szVal, "%c", vecVal_AnswerSuer[i] + 65);
			strRecogAnswer.append(szVal);
		}
	}
#else
	int nFlag = -1;
	float fThreld = 0.0;
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		if ((vecOmrItemGrayDiff[i].fDiff >= fDiffThreshold && vecOmrItemGrayDiff[i].fFirst < fCompThread))
		{
			nFlag = i;
			fThreld = vecOmrItemGrayDiff[i].fFirst;
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
				break;
		}
	}
	if (nFlag >= 0)
	{
		RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
		for (; itItem != omrResult.lSelAnswer.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				char szVal[2] = { 0 };
				sprintf_s(szVal, "%c", itItem->nAnswer + 65);
				strRecogAnswer.append(szVal);
			}
		}
	}
	else
	{
		for (int i = 0; i < vecVal_AnswerSuer.size(); i++)
		{
			char szVal[5] = { 0 };
			sprintf_s(szVal, "%c", vecVal_AnswerSuer[i] + 65);
			strRecogAnswer.append(szVal);
		}
	}
#endif
	omrResult.strRecogVal3 = strRecogAnswer;
	return true;
}

bool CRecognizeThread::RecogVal_Sn3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal)
{
	std::vector<int> vecVal_AnswerSuer;
	RECTLIST::iterator itItem = pSn->lSN.begin();
	for (; itItem != pSn->lSN.end(); itItem++)
	{
		if (itItem->fRealMeanGray < _dAnswerSure_)
		{
			vecVal_AnswerSuer.push_back(itItem->nSnVal);
		}
	}

	std::string strRecogAnswer;
	std::vector<pRECTINFO> vecItemsGrayDesc;
	std::vector<ST_ITEM_DIFF> vecOmrItemGrayDiff;
	calcOmrGrayDiffVal(pSn->lSN, vecItemsGrayDesc, vecOmrItemGrayDiff);

	float fCompThread = 0.0;		//灰度间隔达到要求时，第一个选项的灰度必须达到的要求
	float fDiffThread = 0.0;		//选项可能填涂的可能灰度梯度阀值
	float fDiffExit = 0;			//灰度的梯度递减太快时，可以认为后面选项没有填涂，此时的灰度梯度阀值

	fCompThread = _dCompThread_3_;
	fDiffThread = _dDiffThread_3_;
	fDiffExit = _dDiffExit_3_;
#if 1
	float fMeanGrayDiff = 0.0;
	for (int i = 0; i < vecItemsGrayDesc.size(); i++)
	{
		fMeanGrayDiff += (vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray);
	}
	fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayDesc.size();

	int nFlag = -1;
	float fThreld = 0.0;
	float fGrayDiffLast = 0.0;		//对上一次判断选中的选项对下一个选项选中判断的增益
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		float fGrayThresholdGray = vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray - fMeanGrayDiff;
		if ((vecOmrItemGrayDiff[i].fDiff >= fDiffThread + fGrayThresholdGray + fGrayDiffLast))
		{
			nFlag = i;
			fThreld = vecOmrItemGrayDiff[i].fFirst;
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
				break;
		}
		fGrayDiffLast += abs(fGrayThresholdGray) / 2;
	}
	if (nFlag >= 0)
	{
		if (nFlag == vecOmrItemGrayDiff.size() - 1)
		{
			if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)
			{
				fThreld = vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
			}
		}
		RECTLIST::iterator itItem = pSn->lSN.begin();
		for (; itItem != pSn->lSN.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				vecItemVal.push_back(itItem->nSnVal);
			}
		}
	}
	else if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)		//++判断全都选中的情况
	{
		fThreld = vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
		RECTLIST::iterator itItem = pSn->lSN.begin();
		for (; itItem != pSn->lSN.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				vecItemVal.push_back(itItem->nSnVal);
			}
		}
	}
	else
	{
		vecItemVal = vecVal_AnswerSuer;
	}
#else
	int nFlag = -1;
	float fThreld = 0.0;
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		if ((vecOmrItemGrayDiff[i].fDiff >= fDiffThread && vecOmrItemGrayDiff[i].fFirst < fCompThread))
		{
			nFlag = i;
			fThreld = vecOmrItemGrayDiff[i].fFirst;
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
				break;
		}
	}
	if (nFlag >= 0)
	{
		RECTLIST::iterator itItem = pSn->lSN.begin();
		for (; itItem != pSn->lSN.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				vecItemVal.push_back(itItem->nSnVal);
			}
		}
	}
	else
	{
		vecItemVal = vecVal_AnswerSuer;
	}
#endif
	return true;
}

void CRecognizeThread::HandleWithErrPaper(pST_PaperInfo pPaper)
{
	pPaper->bIssuePaper = true;				//标识此学生试卷属于问题试卷
	pPAPERSINFO pPapers = static_cast<pPAPERSINFO>(pPaper->pPapers);

	pPapers->fmlPaper.lock();
	PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
	for (; itPaper != pPapers->lPaper.end();)
	{
		if (*itPaper == pPaper)
		{
			itPaper = pPapers->lPaper.erase(itPaper);
			break;
		}
		else
			itPaper++;
	}
	pPapers->fmlPaper.unlock();

	pPapers->fmlIssue.lock();
	bool bFind = false;
	PAPER_LIST::iterator itIssuePaper = pPapers->lIssue.begin();
	for (; itIssuePaper != pPapers->lIssue.end();)
	{
		if (*itIssuePaper == pPaper)
		{
			bFind = true;
			break;
		}
		else
			itIssuePaper++;
	}
	if (!bFind)
	{
		pPapers->lIssue.push_back(pPaper);
		pPapers->nRecogErrCount++;
	}
	pPapers->fmlIssue.unlock();

	(static_cast<CDialog*>(pPaper->pSrcDlg))->SendMessage(MSG_ERR_RECOG, (WPARAM)pPaper, (LPARAM)pPapers);		//PostMessageW
}

bool CRecognizeThread::RecogFixCP2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, RECTLIST& lFixResult)
{
	bool bResult = true;

	clock_t start, end;
	start = clock();
	std::string strLog;
//	strLog = Poco::format("开始识别页面%d的定点，图片%s\n", nPic, pPic->strPicName);

	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lSelFixRoi.begin();
	for (int i = 0; itCP != pModel->vecPaperModel[nPic]->lSelFixRoi.end(); i++, itCP++)
	{
		RECTINFO rc = *itCP;

		std::vector<Rect>RectCompList;
		try
		{
			float fModelW = pModel->vecPaperModel[nPic]->nPicW;
			float fModelH = pModel->vecPaperModel[nPic]->nPicH;
			int nRealW = matCompPic.cols;
			int nRealH = matCompPic.rows;

			cv::Rect rtTmp;
			rtTmp.x = rc.rt.x * nRealW / fModelW;
			rtTmp.y = rc.rt.y * nRealH / fModelH;
			rtTmp.width = rc.rt.width * nRealW / fModelW;
			rtTmp.height = rc.rt.height * nRealH / fModelH;

			if (rtTmp.x < 0) rtTmp.x = 0;
			if (rtTmp.y < 0) rtTmp.y = 0;
			if (rtTmp.br().x > matCompPic.cols)
				rtTmp.width = matCompPic.cols - rtTmp.x;
			if (rtTmp.br().y > matCompPic.rows)
				rtTmp.height = matCompPic.rows - rtTmp.y;

			cv::Mat matCompRoi;
			matCompRoi = matCompPic(rtTmp);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

			int nRealThreshold = 150;
			RECTLIST::iterator itFix = pModel->vecPaperModel[nPic]->lFix.begin();
			for (int j = 0; itFix != pModel->vecPaperModel[nPic]->lFix.end(); j++, itFix++)
			{
				if (j == i)
				{
					nRealThreshold = itFix->nThresholdValue;
					break;
				}
			}

			const int channels[1] = { 0 };
			const int histSize[1] = { nRealThreshold };	//150
			float hranges[2] = { 0, nRealThreshold };	//150
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = nRealThreshold;		//150
			if (nCount > 0)
			{
				float fMean = (float)nSum / nCount;		//均值

				for (int h = 0; h < hist.rows; h++)	//histSize
				{
					float binVal = hist.at<float>(h);

					nDevSum += pow(h - fMean, 2)*binVal;
				}
				float fStdev = sqrt(nDevSum / nCount);
				nThreshold = fMean + 2 * fStdev;
				if (fStdev > fMean)
					nThreshold = fMean + fStdev;
			}

			if (nThreshold > nRealThreshold) nThreshold = nRealThreshold;	//150
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);

			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别	Size(_nDilateKernel_, _nDilateKernel_)
			dilate(matCompRoi, matCompRoi, element);

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm = rm + rtTmp.tl();	// rc.rt.tl();
				RectCompList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);
		}
		catch (cv::Exception& exc)
		{
			std::string strLog2 = Poco::format("识别定点(%d)异常: %s\n", i, exc.msg.c_str());
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

// 			if (g_nOperatingMode == 2)
// 			{
// 				bResult = false;						//找到问题点
// 				break;
// 			}
		}

		std::string strLog2;	//临时日志，记录矩形具体识别结果
		bool bFindRect = false;
		if (RectCompList.size() == 0)
			bFindRect = true;
		else
		{
			std::sort(RectCompList.begin(), RectCompList.end(), SortByArea);
			Rect& rtFix = RectCompList[0];

			RECTINFO rcFix;
			RECTLIST::iterator itFix = pModel->vecPaperModel[nPic]->lFix.begin();
			for (int j = 0; itFix != pModel->vecPaperModel[nPic]->lFix.end(); j++, itFix++)
			{
				if (j == i)
				{
					rcFix = *itFix;
					break;
				}
			}

			bool bOnlyOne = false;		//只有一个矩形，需要判断面积和灰度，但是比例可以降低
			bool bFind = false;

			//通过灰度值来判断
			for (int k = 0; k < RectCompList.size(); k++)
			{
				RECTINFO rcTmp = rcFix;
				rcTmp.rt = RectCompList[k];

				//根据定点左上点与右下点位置判断是否在试卷的边线上，如果在，则可能是折角或者边上有损坏
				cv::Point pt1 = RectCompList[k].tl();
				cv::Point pt2 = RectCompList[k].br();
				int nDiff = 4;	//与图像边界的距离间隔在这个值之内，认为属于边界线上
				if (pt1.x < nDiff || pt1.y < nDiff || matCompPic.cols - pt2.x < nDiff || matCompPic.rows - pt2.y < nDiff)
				{
					TRACE("矩形(%d,%d,%d,%d)位置距离边线太近，可能是折角或损坏\n", RectCompList[k].x, RectCompList[k].y, RectCompList[k].width, RectCompList[k].height);
					continue;
				}

				Recog(nPic, rcTmp, matCompPic, pPic, NULL);
				float fArea = rcTmp.fRealArea / rcTmp.fStandardArea;
				float fDensity = rcTmp.fRealDensity / rcTmp.fStandardDensity;
				float fWper = (float)rcTmp.rt.width / rcFix.rt.width;			//查找的矩形的宽度与模板对应定点的宽度之比
				float fHper = (float)rcTmp.rt.height / rcFix.rt.height;			//查找的矩形的宽度与模板对应定点的高度之比
				std::string strTmpLog = Poco::format("第%d个矩形:area=%f, Density=%f\t", k, (double)fArea, (double)fDensity);
				strLog2.append(strTmpLog);
				if ((bOnlyOne && fArea > 0.4 && fArea < 2.5 && fDensity > rcTmp.fStandardValuePercent * 0.9 && fWper < 2.0 && fWper > 0.4 && fHper < 2.0 && fHper > 0.4) || \
					(fArea > 0.5 && fArea < 2.0 && fDensity > rcTmp.fStandardValuePercent && fWper < 2.0 && fWper > 0.4 && fHper < 2.0 && fHper > 0.4))	//fArea > 0.7 && fArea < 1.5 && fDensity > 0.6
				{
					bFind = true;
					rtFix = RectCompList[k];
					break;
				}
			}

			if (!bFind)
				bFindRect = true;
			else
			{
				RECTINFO rcFixInfo = rc;
				rcFixInfo.nTH = i;			//这是属于模板上定点列表的第几个
				rcFixInfo.rt = rtFix;
				lFixResult.push_back(rcFixInfo);
				//TRACE("定点矩形: (%d,%d,%d,%d)\n", rtFix.x, rtFix.y, rtFix.width, rtFix.height);
			}
		}
		if (bFindRect)
		{
			std::string strLog3 = Poco::format("识别定点(%d)失败 -- %s\n", i, strLog2);
			strLog.append(strLog3);
			bResult = false;						//找到问题点
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别页面%d的定点失败\n", nPic);
		strLog.append(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别页面%d的定点时间: %dms\n", nPic, (int)(end - start));
	strLog.append(strTime);
	
	return bResult;
}
