#pragma once

#include "wininet.h"
/*
��ü���� �������� ����α��� ����ó���� �����ϰ� ���������
������ �� ���� ������ ��� ����� ������ �״� ���� ������ �ŵ��� ���ؼ�
�켱 ��Ʈ��ũ �����̳� ��Ÿ �⽺�� ����� ó���ϴ� ��Ÿ�Ϸ� ���� ���̴�
*/

typedef enum {
	//wms�� �̹��� �޾ƿ��� ó��
	thread_data_image = 0,
	//�Ϲ� �ݰ��� �־ ������ �޾� ����ó��
	thread_data_grid = 1,
	//id ������ ������ �޾� ���� ó��
	thread_data_geoms = 2
	//symbol �̹��� �޾ƿ��� ó��
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
