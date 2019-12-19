#pragma once

#include "ThreadData.h"
#include "FindLayerDisplay.h"
#include "GeometryCreateData.h"

class GiGeometry;
class DLL_EXPORT ThreadData_grid : public ThreadData
{
public:
	ThreadData_grid(void);
	~ThreadData_grid(void);

	/*map<CString, list<find_layer_display_data> >  geomInfo; 
	map<CString, CString> symbolType;*/

	GeometryCreateData m_geometryCreateData;

	vector<GiGeometry *> vec;
};
