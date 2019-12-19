#pragma once

#include "ThreadData.h"
#include "ximage.h"

class DLL_EXPORT ThreadData_image : public ThreadData
{
public:
	ThreadData_image(bool cacheUse);
	~ThreadData_image(void);

	ENUM_CXIMAGE_FORMATS imageType;
	CxImage * image;
	//CString layer_id;

	//�޸𸮿� ������ key��
	CString hashName;

	int scale;
	int gridX;
	int gridY;
};


