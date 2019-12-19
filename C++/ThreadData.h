#pragma once

#include "wininet.h"
/*
전체적인 디자인은 연산부까지 병렬처리를 생각하고 만들었지만
샘플을 좀 만들어서 돌려본 결과 연산부 병렬은 그닥 좋은 성과를 거두지 못해서
우선 네트워크 관련이나 기타 잡스런 놈들을 처리하는 스타일로 변경 중이다
*/

typedef enum {
	//wms쪽 이미지 받아오는 처리
	thread_data_image = 0,
	//일반 반경을 주어서 데이터 받아 오는처리
	thread_data_grid = 1,
	//id 값으로 데이터 받아 오는 처리
	thread_data_geoms = 2
	//symbol 이미지 받아오는 처리
} thread_data_type;


class ThreadData
{
public:
	ThreadData(void);
	virtual ~ThreadData(void);

	CString ip;
	INTERNET_PORT port;
	CString url;
	thread_data_type type;
	CString key;
	bool cache;
	CString layerId;
};


class ThreadDataCheck :public std::unary_function<ThreadData*,bool>
{
public:

	bool operator() (ThreadData * pData) const
	{
		if(m_type == pData->type)
		{
			if(m_key.Compare(pData->key) == 0 )return true;
		}
		return false;
	}
	CString m_key;
	thread_data_type m_type;
	ThreadDataCheck(thread_data_type type, LPCTSTR key):m_key(key),m_type(type){}
};

class ThreadDataCheckSameLayer :public std::unary_function<ThreadData*,bool>
{
public:
	bool operator() (ThreadData * pData) const
	{
		if(m_type == pData->type)
		{
			if(m_layerId.Compare(pData->layerId) == 0 )return true;
		}
		return false;
	}

	CString m_layerId;
	thread_data_type m_type;
	ThreadDataCheckSameLayer(thread_data_type type, LPCTSTR layerId):m_type(type),m_layerId(layerId){}
};
