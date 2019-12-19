#include "StdAfx.h"
#include "ThreadData_image.h"

ThreadData_image::ThreadData_image(bool cacheUse) : 
image(NULL), 
imageType(CXIMAGE_FORMAT_UNKNOWN),
scale(-1),
gridX(-1),
gridY(-1)
{
	type = thread_data_image;
	image = NULL;
	cache = cacheUse;
}

ThreadData_image::~ThreadData_image(void)
{
	/*if(NULL != image)
	{
		delete image;
		image = NULL;
	}*/
}
