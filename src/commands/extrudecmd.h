/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2004-2007 Kevin Worcester
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License,or
 * (at your option)any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not,write to the Free Software
 * Foundation,Inc.,59 Temple Place-Suite 330,Boston,MA 02111-1307,
 * USA.
 *
 * See the COPYING file for full license text.
 */


#ifndef __EXTRUDECMD_H
#define __EXTRUDECMD_H

#include "mm3dtypes.h"

//MOVE ME
//2019: Shared implementation for ExtrudeWin.
//https://github.com/zturtleman/mm3d/issues/68
struct ExtrudeImpl
{
	struct SideT
	{
		unsigned a,b; int count;
	};

	typedef std::vector<SideT> SideList;
	typedef std::unordered_map<int,int> ExtrudedVertexMap;

	SideList m_sides;
	ExtrudedVertexMap m_evMap;
	
	bool extrude(Model*, double x, double y, double z, bool make_back_faces);
	void _makeFaces(Model*, unsigned a, unsigned b);
	void _addSide(unsigned a, unsigned b);
	bool _sideIsEdge(unsigned a, unsigned b);	
};

#endif // __EXTRUDECMD_H
