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

#include "mm3dtypes.h" //PCH

#include "weld.h"

#include "model.h"
#include "glmath.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"

// For unweld
typedef struct _NewVertices_t
{
	int index;
	int v[3];
	bool selected[3];
} NewVerticesT;

typedef std::vector<NewVerticesT> NewVerticesList;

void weldSelectedVertices(Model *model, double tolerance)
{
	int dummy1 = 0;
	int dummy2 = 0;
	weldSelectedVertices(model,tolerance,dummy1,dummy2);
}

void weldSelectedVertices(Model *model, double tolerance, int &unweldnum, int &weldnum)
{
	int_list vert;
	model->getSelectedVertices(vert);

	std::map<int,int> welded;

	int weldSources = 0;
	int weldTargets = 0;

	int_list::iterator it;
	int_list::iterator it2;

	// Find vertices to weld and store in "welded"
	for(it = vert.begin(); it!=vert.end(); it++)
	{
		bool match = false;
		it2 = it;
		it2++;
		for(; it2!=vert.end(); it2++)
		{
			double a[3];
			double b[3];

			model->getVertexCoords(*it,a);
			model->getVertexCoords(*it2,b);

			double d = distance(a[0],a[1],a[2],b[0],b[1],b[2]);

			if(d<tolerance&&welded.find(*it2)==welded.end())
			{
				log_debug("weld vertices %d and %d\n",*it,*it2);
				welded[*it2] = *it;
				match = true;
				weldSources++;
			}
		}

		if(match)
		{
			weldTargets++;
		}
	}

	// Move triangles to welded vertices
	for(int t = 0; t<model->getTriangleCount(); t++)
	{
		int v[3];
		bool changeVertices = false;
		for(int i = 0; i<3; i++)
		{
			v[i] = model->getTriangleVertex(t,i);
			if(welded.find(v[i])!=welded.end())
			{
				v[i] = welded[v[i]];
				changeVertices = true;
			}
		}

		if(changeVertices)
		{
			model->setTriangleVertices(t,v[0],v[1],v[2]);
		}
	}

	// Delete orphaned vertices
	std::map<int,int>::reverse_iterator mit;

	model->deleteFlattenedTriangles();
	model->deleteOrphanedVertices();

	unweldnum = weldSources+weldTargets;
	weldnum = weldTargets;
}

void unweldSelectedVertices(Model *model)
{
	int dummy1 = 0;
	int dummy2 = 0;
	unweldSelectedVertices(model,dummy1,dummy2);
}

void unweldSelectedVertices(Model *model, int &unweldnum, int &weldnum)
{
	int_list vert;
	model->getSelectedVertices(vert);

	NewVerticesList nvl;
	int added = 0;

	std::map<int,int> vertCount;

	int_list::iterator it;

	for(it = vert.begin(); it!=vert.end(); it++)
	{
		vertCount[*it] = 0;
	}

	for(int t = 0; t<model->getTriangleCount(); t++)
	{
		NewVerticesT nv;
		nv.index = t;

		bool changeVertices = false;

		for(int i = 0; i<3; i++)
		{
			nv.v[i] = model->getTriangleVertex(t,i);

			if(vertCount.find(nv.v[i])!=vertCount.end())
			{
				nv.selected[i] = true;
				vertCount[nv.v[i]]++;

				if(vertCount[nv.v[i]]>1)
				{
					changeVertices = true;

					double coords[3];

					model->getVertexCoords(nv.v[i],coords);
					int temp  = model->addVertex(coords[0],coords[1],coords[2]);

					//infl_list inf;
					//model->getVertexInfluences(nv.v[i],inf);
					const infl_list &inf = model->getVertexInfluences(nv.v[i]);
					for(infl_list::const_iterator it = inf.begin();
							it!=inf.end(); ++it)
					{
						model->addVertexInfluence(temp,
								it->m_boneId,it->m_type,it->m_weight);
					}

					nv.v[i] = temp;
					added++;
					unweldnum++;
				}
				else
				{
					weldnum++;
				}
			}
			else
			{
				nv.selected[i] = false;
			}
		}

		nvl.push_back(nv);
	}

	NewVerticesList::iterator nvit;
	for(nvit = nvl.begin(); nvit!=nvl.end(); nvit++)
	{
		model->setTriangleVertices((*nvit).index,
				(*nvit).v[0],(*nvit).v[1],(*nvit).v[2]);
	}

	int_list tri;
	model->getSelectedTriangles(tri);

	model->unselectAll();
	model->unselectAll();
	for(it = tri.begin(); it!=tri.end(); it++)
	{
		model->selectTriangle(*it);
	}

	for(nvit = nvl.begin(); nvit!=nvl.end(); nvit++)
	{
		for(int i = 0; i<3; i++)
		{
			if((*nvit).selected[i])
			{
				model->selectVertex((*nvit).v[i]);
			}
		}
	}

	unweldnum += weldnum;
}
