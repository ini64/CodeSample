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

	//메모리에 설정될 key값
	CString hashName;

	int scale;
	int gridX;
	int gridY;
};


