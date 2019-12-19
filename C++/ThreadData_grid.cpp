#include "StdAfx.h"
#include "ThreadData_grid.h"

ThreadData_grid::ThreadData_grid(void)
{
	type = thread_data_grid;
}

ThreadData_grid::~ThreadData_grid(void)
{
	//BOOST_FOREACH(GiGeometry * geom, vec)
	//{
	//	delete geom;
	//}
	//vec.clear();
}
