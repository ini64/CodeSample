#include "StdAfx.h"
#include "GiThreadMgr.h"
#include "afxinet.h"

#include "Lock.h"
#include "MemoryPool.h"

#include "GisMath.h"
#include "GiGeometryMgr.h"

#include "ThreadData_grid.h"
#include "ThreadData_geoms.h"
#include "ThreadData_image.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>


DWORD WINAPI GiThreadMgr_ThreadFuntion(IN LPVOID vThreadHandle)
{
	GiThreadMgr_Data * pData = (GiThreadMgr_Data *)vThreadHandle;

	TRACE("GiThreadMgr_ThreadFuntion %d thread 시작 \n" , pData->idx);

	GiThreadMgr * pMgr = (GiThreadMgr *)pData->Parent;
	//CInternetSession * pSession = new CInternetSession(_T("KGIS_IMAGE_MAP_SESSION"));

	GiGeometryMgr * geometryMgr = new GiGeometryMgr();
	HttpDownMgr * httpDown = new HttpDownMgr();
	

	CString strSession;
	strSession.Format(_T("GITHREAD_MGR_SESSION %d"), pData->idx);

	HINTERNET hInternetSession = InternetOpen(strSession, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 
		INTERNET_FLAG_DONT_CACHE| INTERNET_FLAG_NO_AUTO_REDIRECT|INTERNET_FLAG_NO_COOKIES|INTERNET_FLAG_NO_UI); 


	DWORD dwTime = 1000 * 60;
	InternetSetOption (hInternetSession, INTERNET_OPTION_CONNECT_TIMEOUT,         &dwTime, sizeof(DWORD));
	InternetSetOption (hInternetSession, INTERNET_OPTION_RECEIVE_TIMEOUT,         &dwTime, sizeof(DWORD));
	InternetSetOption (hInternetSession, INTERNET_OPTION_SEND_TIMEOUT,            &dwTime, sizeof(DWORD));
	InternetSetOption (hInternetSession, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &dwTime, sizeof(DWORD));
	InternetSetOption (hInternetSession, INTERNET_OPTION_CONTROL_SEND_TIMEOUT,    &dwTime, sizeof(DWORD));
	InternetSetOption (hInternetSession, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,    &dwTime, sizeof(DWORD));
	InternetSetOption (hInternetSession, INTERNET_OPTION_DATA_SEND_TIMEOUT,       &dwTime, sizeof(DWORD));

	while(1)
	{

		//ThreadData는 message를 보내다 에러 난 경우를 제외하고 절대 지우지 않는다 ^^
		ThreadData * workingData = pMgr->Get(pData->idx);

		if(NULL == workingData )
		{
			DWORD ret = WaitForMultipleObjects(2, pData->hAndle, FALSE, INFINITE);

			if(WAIT_OBJECT_0 + 1 == ret){
				pData->ReSetEvent(1); break;
			}
			else if(WAIT_FAILED == ret)continue;
			else if(WAIT_TIMEOUT == ret)continue;
		}
		else
		{
		
			pMgr->UpdateThreadDataTime(pData->idx, dwTime);

			switch(workingData->type)
			{
			case thread_data_image:
					//ThreadData_image * image = dynamic_cast<ThreadData_image *>(workingData);
					pMgr->ImageWorking(hInternetSession, pData, dynamic_cast<ThreadData_image *>(workingData), httpDown);
				break;

			case thread_data_grid:
				pMgr->GridWorking(hInternetSession, pData, dynamic_cast<ThreadData_grid *>(workingData), httpDown, geometryMgr);
				break;

			case thread_data_geoms:
				//TRACE("thread_data_geoms \n");

				pMgr->GeomWorking(hInternetSession, pData, dynamic_cast<ThreadData_geoms *>(workingData), httpDown, geometryMgr);
				break;
			default:
				break;
			}

			pMgr->UpdateThreadDataTime(pData->idx, 0);
		}
	}


	delete geometryMgr;
	delete httpDown;

	InternetCloseHandle(hInternetSession);

	TRACE("%d thread 종료 \n" , pData->idx);
	//delete pData;

	return 2;
}

bool GiThreadMgr::ImageWorking(HINTERNET hInternetSession, GiThreadMgr_Data * pData, ThreadData_image * workingData, 
							   HttpDownMgr * httpDownMgr )
{
	if(NULL == workingData)return false;

	//LPBYTE pReadBuf = GetMemoryPool();
	//while(NULL == pReadBuf)
	//{
	//	//사용가능 메모리 없으면 0.1초 대기
	//	SleepEx(100, TRUE);
	//	pReadBuf = GetMemoryPool();
	//}

	LPBYTE pReadBuf = NULL;

	CString * httpInfo = new CString();
	httpInfo->Format(_T("http://%s:%d/%s"), workingData->ip,  workingData->port, workingData->url);
	PostMessage(pData->hWnd, WM_DEBUG_MESSAGE, (WPARAM)httpInfo, 0);

	int dataSize = 0;
	if( httpDownMgr->Get_HttpSendRequest(hInternetSession, 
		workingData->ip, workingData->port, workingData->url, _T("GET"), workingData->cache, 
		&pReadBuf, dataSize) )
	{
		
		CString httpReadData((TCHAR *)pReadBuf, dataSize);
		
		if(dataSize > 0 && httpReadData.Find(_T("<HTML>")) < 0 && httpReadData.Find(_T("<html>")) < 0) 
		{
			workingData->image = new CxImage;
			if(workingData->image->Decode(pReadBuf, dataSize, workingData->imageType))
			{
				if(PostMessage(pData->hWnd, WM_GITHREAD_MGR_EVENT, (WPARAM)workingData, 0))
				{
					delete pReadBuf;
					return true;
				}
			}
		}
		delete pReadBuf;
		
	}

	if(NULL != workingData->image)delete workingData->image;

	if(PostMessage(pData->hWnd, WM_GITHREAD_MGR_EVENT, (WPARAM)workingData, 0))
	{
		return false;
	}

	//delete workingData;

	return false;
}
bool GiThreadMgr::GridWorking(HINTERNET hInternetSession, GiThreadMgr_Data * pData, ThreadData_grid * workingData, 
							  HttpDownMgr * httpDownMgr, GiGeometryMgr * geometryMgr)
{


	if(NULL == workingData)return false;

	LPBYTE pReadBuf = NULL;
	bool bParsIng = true;

	int dataSize = 0;

	DWORD dwTime = GetTickCount();
 	//if( httpDownMgr->Get_InternetOpenUrl(hInternetSession, 
 	//	workingData->ip, workingData->url, workingData->cache, 
 	//	&pReadBuf, dataSize) )
 	//{

 	if( httpDownMgr->Get_HttpSendRequest(hInternetSession, 
 			workingData->ip, workingData->port, workingData->url, _T("GET"), workingData->cache, 
 			&pReadBuf, dataSize) )
 	{
		//TRACE("다운로드 %s %d\n", workingData->layerId, GetTickCount() - dwTime);

		if(dataSize > 0)
		{
			//geometryMgr->GetGeom(pReadBuf, dataSize, workingData);
			dwTime = GetTickCount();
			bParsIng = geometryMgr->GetGeom(pReadBuf, dataSize, workingData);
			//TRACE("파싱 %s %d\n", workingData->layerId, GetTickCount() - dwTime);
		}
		//TRACE("데이터 사이즈 %s %d\n", workingData->layerId, workingData->vec.size());

		
		delete pReadBuf;
		
		if(PostMessage(pData->hWnd, WM_GITHREAD_MGR_EVENT, (WPARAM)workingData, 0))
		{
			return true;
		}
		
	}

	//SetMemoryPool(pReadBuf);
	BOOST_FOREACH(GiGeometry * p, workingData->vec)
	{
		delete p;
	}

	if(PostMessage(pData->hWnd, WM_GITHREAD_MGR_EVENT, (WPARAM)workingData, 0))
	{
		return false;
	}

	//delete workingData;
	
	return false;
}
bool GiThreadMgr::GeomWorking(HINTERNET hInternetSession, GiThreadMgr_Data * pData, ThreadData_geoms * workingData, 
							  HttpDownMgr * httpDownMgr, GiGeometryMgr * geometryMgr)
{
	if(NULL == workingData)
	{
		TRACE("변환실패 \n");
		return false;
	}

	LPBYTE pReadBuf = NULL;

	int dataSize = 0;

	//TRACE("네트워크 진입 %s %s \n" , workingData->ip, workingData->url);

	bool bParsIng = true;

// 	if( httpDownMgr->Get_InternetOpenUrl(hInternetSession, 
// 		workingData->ip, workingData->url, workingData->cache, 
// 		&pReadBuf, dataSize) )
// 	{

	if( httpDownMgr->Get_HttpSendRequest(hInternetSession, 
		workingData->ip, workingData->port, workingData->url, _T("GET"), workingData->cache, 
		&pReadBuf, dataSize) )
	{
		if(dataSize > 0)
		{
			bParsIng = geometryMgr->GetGeom(pReadBuf, dataSize, workingData);
			
		}

		delete pReadBuf;

		TRACE("eventgeom %d \n", workingData->vec.size());
		if(workingData->vec.size() > 0 && bParsIng)
		{
			if(PostMessage(pData->hWnd, WM_GITHREAD_MGR_EVENT, (WPARAM)workingData, 0))
			{
				return true;
			}
		}
	}

	//SetMemoryPool(pReadBuf);
	BOOST_FOREACH(GiGeometry * p, workingData->vec)
	{
		delete p;
	}
	delete workingData;

	return false;
}




GiThreadMgr::GiThreadMgr(int nThread, HWND h)
{
	InitializeCriticalSection(&m_cs);

	m_httpDownMgr = new HttpDownMgr();
	m_geometryMgr = new GiGeometryMgr();

	hWnd = h;

	DWORD dwThreadID;
	for(int i = 0 ; i < nThread ; i++)
	{
		GiThreadMgr_Data * pThreadData = new GiThreadMgr_Data(i, &m_cs, h, this);
		pThreadData->hThread = CreateThread(NULL, 0, GiThreadMgr_ThreadFuntion, pThreadData, 0, &dwThreadID);

		m_threadData.push_back(pThreadData);
	}

	CString strSession;
	strSession.Format(_T("GITHREAD_MGR_BLOCKING_SESSION"));

	m_hInternetSession = InternetOpen(strSession, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 
		INTERNET_FLAG_DONT_CACHE| INTERNET_FLAG_NO_AUTO_REDIRECT|INTERNET_FLAG_NO_COOKIES|INTERNET_FLAG_NO_UI); 


	DWORD dwTime = 1000*60;
	InternetSetOption (m_hInternetSession, INTERNET_OPTION_CONNECT_TIMEOUT,         &dwTime, sizeof(DWORD));
	InternetSetOption (m_hInternetSession, INTERNET_OPTION_RECEIVE_TIMEOUT,         &dwTime, sizeof(DWORD));
	InternetSetOption (m_hInternetSession, INTERNET_OPTION_SEND_TIMEOUT,            &dwTime, sizeof(DWORD));
	InternetSetOption (m_hInternetSession, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &dwTime, sizeof(DWORD));
	InternetSetOption (m_hInternetSession, INTERNET_OPTION_CONTROL_SEND_TIMEOUT,    &dwTime, sizeof(DWORD));
	InternetSetOption (m_hInternetSession, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,    &dwTime, sizeof(DWORD));
	InternetSetOption (m_hInternetSession, INTERNET_OPTION_DATA_SEND_TIMEOUT,       &dwTime, sizeof(DWORD));
}

GiThreadMgr::~GiThreadMgr(void)
{
	EnterCriticalSection(&m_cs);

	BOOST_FOREACH(ThreadData * p, m_lWorkingList)
	{
		delete p;
	}
	m_lWorkingList.clear();
	LeaveCriticalSection(&m_cs);

	BOOST_FOREACH(GiThreadMgr_Data * threadMgrData, m_threadData)
	{
		threadMgrData->SetEvent(1);
		WaitForSingleObject(threadMgrData->hThread, INFINITE);
		CloseHandle(threadMgrData->hThread);

		delete threadMgrData;
	}
	m_threadData.clear();

	delete m_httpDownMgr; 
	delete m_geometryMgr;

	InternetCloseHandle(m_hInternetSession);
	
	/*Lock pLock(&m_cs);	

	list<ThreadData *>::iterator it = m_lWorkingList.begin();
	while(it != m_lWorkingList.end())
	{
		delete (*it);
		it++;
	}*/

	DeleteCriticalSection(&m_cs);
}

//void GiThreadMgr::SetDRect(DRect * rt)
//{
//	Lock pLock(&m_cs);	
//	m_drt = *rt;
//}
//
//void GiThreadMgr::GetDRect(DRect * rt)
//{
//	Lock pLock(&m_cs);	
//	*rt = m_drt;
//}

ThreadData * GiThreadMgr::Get()
{
	Lock pLock(&m_cs);	

	if(m_lWorkingList.size() == 0)return NULL;

	vector<ThreadData *>::iterator it = m_lWorkingList.begin();
	ThreadData * p = (*it);
	m_lWorkingList.erase(it);

	TRACE("획득 아이템 %s %s \n" , p->layerId, p->key);

	return p;
}

ThreadData * GiThreadMgr::Get(DWORD i)
{
	Lock pLock(&m_cs);	
	ThreadData * p = NULL;

	if(m_mWorkingList[i].size() != 0)
	{
		vector<ThreadData *>::iterator it = m_mWorkingList[i].begin();
		p = (*it);
		m_mWorkingList[i].erase(it);

		TRACE("획득 아이템 %s %s \n" , p->layerId, p->key);
	}
	else
	{
		//TRACE("획득 아이템 없음 \n" );
	}
	return p;
}

bool GiThreadMgr::Set(ThreadData * p)
{
	Lock pLock(&m_cs);
	m_lWorkingList.push_back(p);
	return true;

	/*if(m_lWorkingList.end() == find_if(m_lWorkingList.begin(), m_lWorkingList.end(), HttpThreadMgrUrlCheck(&p->strCacheKey)))
	{
	m_lWorkingList.push_back(p);
	return true;
	}
	else
	{
	delete p;
	}
	return false;*/
}

bool GiThreadMgr::GeomWorkingBlocking(ThreadData_geoms * workingData)
{
	if(NULL == workingData)
	{
		TRACE("변환실패 \n");
		return false;
	}

	LPBYTE pReadBuf = NULL;

	int dataSize = 0;

	//TRACE("네트워크 진입 %s %s \n" , workingData->ip, workingData->url);

	bool bParsIng = true;

	//if( m_httpDownMgr->Get_InternetOpenUrl(m_hInternetSession, 
	//	workingData->ip, workingData->url, workingData->cache, 
	//	&pReadBuf, dataSize) )
	//{

	if( m_httpDownMgr->Get_HttpSendRequest(m_hInternetSession, 
		workingData->ip, workingData->port, workingData->url, _T("GET"), workingData->cache, 
		&pReadBuf, dataSize) )
	{
		if(dataSize > 0)
		{
			bParsIng = m_geometryMgr->GetGeom(pReadBuf, dataSize, workingData);

		}

		delete pReadBuf;

		return true;
	}
	return false;
}


// void GiThreadMgr::DeleteData()
// {
// 	Lock pLock(&m_cs);
// 
// 	BOOST_FOREACH(ThreadData * data, m_lWorkingList) 
// 	{
// 		delete data;
// 	}
// 	m_lWorkingList.clear();
// 
// 	typedef map<DWORD, vector<ThreadData *> > data;
// 
// 	BOOST_FOREACH(data::value_type & hashWork, m_mWorkingList) 
// 	{
// 		BOOST_FOREACH(ThreadData * data, hashWork.second) 
// 		{
// 			delete data;
// 		}
// 	}
// 	m_mWorkingList.clear();
// 
// 	TRACE("전체삭제 \n");
// }

void GiThreadMgr::DeleteData(thread_data_type type, LPCTSTR key)
{
	Lock pLock(&m_cs);

	vector<ThreadData *>::iterator it = find_if(m_lWorkingList.begin(), m_lWorkingList.end(), ThreadDataCheck(type, key));
	while(it != m_lWorkingList.end())
	{
		delete (*it);
		m_lWorkingList.erase(it);

		it = find_if(m_lWorkingList.begin(), m_lWorkingList.end(), ThreadDataCheck(type, key));
	}

	typedef map<DWORD, vector<ThreadData *> > data;
	
	BOOST_FOREACH(data::value_type & hashWork, m_mWorkingList) 
	{
		vector<ThreadData *>::iterator it = find_if(hashWork.second.begin(), hashWork.second.end(), ThreadDataCheck(type, key));
		while(it != hashWork.second.end())
		{
			delete (*it);
			hashWork.second.erase(it);

			it = find_if(hashWork.second.begin(), hashWork.second.end(), ThreadDataCheck(type, key));
		}
	}
}

void GiThreadMgr::DeleteLayer(thread_data_type type, LPCTSTR layerId)
{
	Lock pLock(&m_cs);

	vector<ThreadData *>::iterator it = find_if(m_lWorkingList.begin(), m_lWorkingList.end(), ThreadDataCheckSameLayer(type, layerId));
	while(it != m_lWorkingList.end())
	{
		delete (*it);
		m_lWorkingList.erase(it);

		it = find_if(m_lWorkingList.begin(), m_lWorkingList.end(), ThreadDataCheckSameLayer(type, layerId));
	}

	typedef map<DWORD, vector<ThreadData *> > data;

	BOOST_FOREACH(data::value_type & hashWork, m_mWorkingList) 
	{
		vector<ThreadData *>::iterator it = find_if(hashWork.second.begin(), hashWork.second.end(), ThreadDataCheckSameLayer(type, layerId));
		while(it != hashWork.second.end())
		{
			delete (*it);
			hashWork.second.erase(it);

			it = find_if(hashWork.second.begin(), hashWork.second.end(), ThreadDataCheckSameLayer(type, layerId));
		}
	}
	if(thread_data_image == type)
	{
		TRACE("삭제: %s \n", layerId);
	}
}
void GiThreadMgr::UpdateThreadDataTime(DWORD idx, DWORD time)
{
	Lock pLock(&m_cs);

	list<GiThreadMgr_Data *>::iterator it = find_if(m_threadData.begin(), m_threadData.end(), GiThreadMgr_Data_find(idx));
	if(m_threadData.end() != it)
	{
		GiThreadMgr_Data * threadData = (*it);
		threadData->time = GetTickCount() + time;
	}
}

int GiThreadMgr::GetWorkingSize()
{
	Lock pLock(&m_cs);
	return (int)m_lWorkingList.size();
}

void GiThreadMgr::Run()
{
	Lock pLock(&m_cs);

#ifdef _DEBUG

	m_mWorkingList[0].insert(m_mWorkingList[0].begin(), m_lWorkingList.begin(), m_lWorkingList.end());
	m_lWorkingList.clear();

	list<GiThreadMgr_Data *>::iterator it = find_if(m_threadData.begin(), m_threadData.end(), GiThreadMgr_Data_find(0));
	if(m_threadData.end() != it)
	{
		GiThreadMgr_Data * threadData = (*it);
		threadData->SetEvent(0);
	}

#else
	m_threadData.sort(GiThreadMgr_Data_sort());
	
	while(1)
	{
		BOOST_FOREACH(GiThreadMgr_Data * threadMgrData, m_threadData)
		{
			if(m_lWorkingList.size() == 0)break;
			
			vector<ThreadData *>::iterator it = m_lWorkingList.begin();
			m_mWorkingList[threadMgrData->idx].push_back(*it);

			//TRACE("thread %d \n", threadMgrData->idx);

			m_lWorkingList.erase(it);

		}
		if(m_lWorkingList.size() == 0)break;
	}

	typedef map<DWORD, vector<ThreadData *> > data;
	BOOST_FOREACH(data::value_type & threadData, m_mWorkingList)
	{
		if(threadData.second.size() > 0)
		{
			list<GiThreadMgr_Data *>::iterator it = find_if(m_threadData.begin(), m_threadData.end(), GiThreadMgr_Data_find(threadData.first));
			if(m_threadData.end() != it)
			{
				GiThreadMgr_Data * threadData = (*it);
				threadData->SetEvent(0);
			}
		}
	}
#endif



//	for(DWORD i = 0 ; i < m_lWorkingList.size(); i++)
//	{
//#ifdef _DEBUG
//		DWORD workingIdx = 0;
//#else
//		DWORD workingIdx = i % vThread.size();
//#endif
//		m_mWorkingList[workingIdx].push_back(m_lWorkingList.at(i));
//	}
//	m_lWorkingList.clear();
//
////#ifdef _DEBUG
////	vThread.at(0)->SetEvent(0);
////
////#else
//	//DWORD thread = min(vThread.size(), m_lWorkingList.size());
//
//#ifdef _DEBUG
//	
//	vThread.at(0)->SetEvent(0);
//
//#else
//
//	for(DWORD i = 0 ; i < vThread.size() ; i++)
//	{
//		vThread.at(i)->SetEvent(0);
//	}
//
//#endif
//
////#endif
}


LPBYTE GiThreadMgr::GetMemoryPool()
{
	return m_MemoryPool.Get();
}

void GiThreadMgr::SetMemoryPool(LPBYTE p)
{
	m_MemoryPool.Set(p);
}

// bool GiThreadMgr::SymbolWorking(HINTERNET hInternetSession, GiThreadMgr_Data * pData, ThreadData_symbol * workingData, 
// 								HttpDownMgr * httpDownMgr )
// {
// 	if(NULL == workingData)return false;
// 	LPBYTE pReadBuf = NULL;
// 
// 	int dataSize = 0;
// 	if( httpDownMgr->Get_HttpSendRequest(hInternetSession, 
// 		workingData->ip, workingData->url, _T("GET"), workingData->cache, 
// 		&pReadBuf, dataSize) )
// 	{
// 		CString httpReadData((TCHAR *)pReadBuf, dataSize);
// 		if(httpReadData.Find(_T("<HTML>")) > 0 && httpReadData.Find(_T("<html>")) > 0) return false;
// 
// 		if(dataSize > 0)
// 		{
// 			workingData->image = new CxImage;
// 			if(workingData->image->Decode(pReadBuf, dataSize, workingData->imageType))
// 			{
// 				if(PostMessage(pData->hWnd, WM_GITHREAD_MGR_EVENT, (WPARAM)workingData, 0))
// 				{
// 					delete pReadBuf;
// 					return true;
// 				}
// 			}
// 		}
// 		delete pReadBuf;
// 
// 	}
// 
// 	if(NULL != workingData->image)delete workingData->image;
// 	workingData->image = NULL;
// 	delete workingData;
// 
// 	return false;
// }