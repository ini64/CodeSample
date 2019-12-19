#pragma once

#include "HttpDownMgr.h"

#include "FindLayerDisplay.h"
#include "MemoryPool.h"
#include "threadData.h"


class DLL_EXPORT GiThreadMgr_Data
{
public:

	GiThreadMgr_Data(int i, CRITICAL_SECTION * cs, HWND h, IN LPVOID parent ) : 
	  cs(cs) , idx(i), hWnd(h), Parent(parent)

	  {
		  //���� ���¹��, �����ǰ� ���
		  hAndle[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
		  hAndle[1] = CreateEvent(NULL, TRUE, FALSE, NULL);

		  time = GetTickCount();
	  }

	  ~GiThreadMgr_Data()
	  {

	  }

	  void SetEvent(int idx)
	  {
		  ::SetEvent(hAndle[idx]);
	  }

	  void ReSetEvent(int idx)
	  {
		  ::ResetEvent(hAndle[idx]);
	  }

	  HANDLE GetHandle(int idx)
	  {
		  return hAndle[idx];
	  }
	  DWORD time;

	  //0�� �۾�, 1�� Ż��
	  HANDLE hAndle[2];
	  //�Ϸ��� �޼����� ����� �θ��� �ڵ�
	  HWND hWnd;
	  //����ȭ ��ü
	  CRITICAL_SECTION * cs;
	  //������ ��ȣ
	  DWORD idx;
	  //�ش� �������� �ڵ�
	  HANDLE hThread;
	  //�޴��� �ڵ�
	  IN LPVOID Parent;
};

//setting_sort


class GiThreadMgr_Data_sort :public std::binary_function<GiThreadMgr_Data*,GiThreadMgr_Data*, bool>
{
public:
	bool operator() (GiThreadMgr_Data * pNode1, GiThreadMgr_Data * pNode2) const
	{
		DWORD order1 = pNode1->time;
		DWORD order2 = pNode2->time;

		return order1 < order2;
	}
	GiThreadMgr_Data_sort(){}
};



class GiThreadMgr_Data_find  :public std::unary_function<GiThreadMgr_Data *,bool>
{
public:

	bool operator() (GiThreadMgr_Data * data) const
	{
		if(data->idx == m_idx)
		{
			//data->time = GetTickCount();
			return true;
		}
		return false;
	}
	DWORD m_idx;

	GiThreadMgr_Data_find(DWORD idx):m_idx(idx)
	{
	}
};


class GiGeometryMgr;
class ThreadData_grid;
class ThreadData_geoms;
class ThreadData_image;
class ThreadData_symbol;

DWORD WINAPI GiThreadMgr_ThreadFuntion(IN LPVOID vThreadHandle);
#define WM_GITHREAD_MGR_EVENT  WM_USER + 5

class DLL_EXPORT GiThreadMgr
{
	friend DWORD WINAPI GiThreadMgr_ThreadFuntion(IN LPVOID vThreadHandle);

	//bool GetGeomList(CString & textData, vector<GiGeometry *> * vec, 
	//	ThreadData * workingData, GiGeometryMgr * geometryMgr);
	//bool SymbolWorking(HINTERNET hInternetSession, GiThreadMgr_Data * pData, ThreadData_symbol * workingData, 
	//	HttpDownMgr * httpDownMgr);

	bool ImageWorking(HINTERNET hInternetSession, GiThreadMgr_Data * pData, ThreadData_image * workingData, 
		HttpDownMgr * httpDownMgr);

	bool GridWorking(HINTERNET hInternetSession, GiThreadMgr_Data * pData, ThreadData_grid * workingData, 
		HttpDownMgr * httpDownMgr, GiGeometryMgr * geometryMgr);

	bool GeomWorking(HINTERNET hInternetSession, GiThreadMgr_Data * pData, ThreadData_geoms * workingData, 
		HttpDownMgr * httpDownMgr, GiGeometryMgr * geometryMgr);
	

public:
	GiThreadMgr(int nThread, HWND h);
	~GiThreadMgr(void);

	//�����尡 �۾��� ���
	//list<ThreadData *> m_lWorkingList;
	vector<ThreadData *> m_lWorkingList;

	map<DWORD, vector<ThreadData *> > m_mWorkingList;

	HttpDownMgr * m_httpDownMgr; 
	GiGeometryMgr * m_geometryMgr;
	HINTERNET m_hInternetSession;

	//������ ��Ź� ���� ����
	list<GiThreadMgr_Data *> m_threadData;

	//��ü ������� ����ȭ ��ü
	CRITICAL_SECTION m_cs;

	//�θ� ������ �ڵ� (Ȥ�ó� ���� ������ ����)
	HWND hWnd;

	//void SetDRect(DRect * rt);
	//void GetDRect(DRect * rt);
	//DRect m_drt;

	ThreadData * Get();
	ThreadData * Get(DWORD i);
	bool Set(ThreadData * p);
	//bool Check(CString * p);

	//void DeleteData();
	void DeleteData(thread_data_type type, LPCTSTR key);
	void DeleteLayer(thread_data_type type, LPCTSTR layerId);

	void UpdateThreadDataTime(DWORD idx, DWORD time);

	int GetWorkingSize();
	void Run();

	MemoryPool m_MemoryPool;
	LPBYTE GetMemoryPool();
	void SetMemoryPool(LPBYTE p);

	bool GeomWorkingBlocking(ThreadData_geoms * workingData);
};
