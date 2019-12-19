#include "StdAfx.h"
#include "ThreadData_geoms.h"
#include "GiGeometry.h"

ThreadData_geoms::ThreadData_geoms(void)
{
	type = thread_data_geoms;
}

ThreadData_geoms::~ThreadData_geoms(void)
{
	//BOOST_FOREACH(GiGeometry * geom, vec)
	//{
	//	delete geom;
	//}
	//vec.clear();
}
