#pragma once

#include "ThreadData.h"
#include "FindLayerDisplay.h"
#include "GeometryCreateData.h"

class GiGeometry;
class DLL_EXPORT ThreadData_geoms : public ThreadData
{
public:
	ThreadData_geoms(void);
	~ThreadData_geoms(void);

	//map<CString, list<find_layer_display_data> >  geomInfo; 
	//map<CString, CString> symbolType;

	GeometryCreateData m_geometryCreateData;

	vector<GiGeometry *> vec;
};
