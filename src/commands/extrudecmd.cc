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

#include "extrudecmd.h" //REMOVE ME
#include "command.h"
#include "model.h"
#include "log.h"

struct ExtrudeCommand : Command
{
	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Command","Extrude...");
	}

	//This is a historical shortcut. The tool is using X.
	//virtual const char *getKeymap(int){ return "Insert"; }
	virtual const char *getKeymap(int){ return "Ctrl+Shift+X"; }

	virtual bool activated(int, Model *model)
	{
		extern void extrudewin(Model*);
		extrudewin(model); return true;
	}
};

extern Command *extrudecmd(){ return new ExtrudeCommand; }

bool ExtrudeImpl::extrude(Model *model, double x, double y, double z, bool make_back_faces)
{
	m_sides.clear(); m_evMap.clear();
	
	int_list faces;
	model->getSelectedTriangles(faces);
	int_list vertices;
	model->getSelectedVertices(vertices);
	
	if(faces.empty()) return false;

	// Find edges (sides with count==1)
	int_list::iterator it;

	for(it = faces.begin(); it!=faces.end(); it++)
	{
		unsigned v[3];

		for(int t = 0; t<3; t++)
		{
			v[t] = model->getTriangleVertex(*it,t);
		}
		for(int t = 0; t<(3-1); t++)
		{
			_addSide(v[t],v[t+1]);
		}
		_addSide(v[0],v[2]);
	}

	// make extruded vertices and create a map from old vertices
	// to new vertices
	for(it = vertices.begin(); it!=vertices.end(); it++)
	{
		double coord[3];
		model->getVertexCoords(*it,coord);
		coord[0] += x;
		coord[1] += y;
		coord[2] += z;
		unsigned i = model->addVertex(coord[0],coord[1],coord[2]);
		m_evMap[*it] = i;

		log_debug("added vertex %d for %d at %f,%f,%f\n",m_evMap[*it],*it,coord[0],coord[1],coord[2]);
	}

	// Add faces for edges
	for(it = faces.begin(); it!=faces.end(); it++)
	{
		unsigned v[3];

		for(int t = 0; t<3; t++)
		{
			v[t] = model->getTriangleVertex(*it,t);
		}
		for(int t = 0; t<(3-1); t++)
		{
			if(_sideIsEdge(v[t],v[t+1]))
			{
				_makeFaces(model,v[t],v[t+1]);
			}
		}
		if(_sideIsEdge(v[2],v[0]))
		{
			_makeFaces(model,v[2],v[0]);
		}
	}

	// Map selected faces onto extruded vertices
	for(it = faces.begin(); it!=faces.end(); it++)
	{
		unsigned tri = *it;

		int v1 = model->getTriangleVertex(tri,0);
		int v2 = model->getTriangleVertex(tri,1);
		int v3 = model->getTriangleVertex(tri,2);
				
		if(make_back_faces)
		{
			int newTri = model->addTriangle(v1,v2,v3);
			model->invertNormals(newTri);
		}

		log_debug("face %d uses vertices %d,%d,%d\n",*it,v1,v2,v3);

		model->setTriangleVertices(tri,
				m_evMap[model->getTriangleVertex(tri,0)],
				m_evMap[model->getTriangleVertex(tri,1)],
				m_evMap[model->getTriangleVertex(tri,2)]);

		log_debug("moved face %d to vertices %d,%d,%d\n",*it,
				model->getTriangleVertex(tri,0),
				model->getTriangleVertex(tri,1),
				model->getTriangleVertex(tri,2));

	}

	// Update face selection

	ExtrudedVertexMap::iterator evit;
	for(evit = m_evMap.begin(); evit!=m_evMap.end(); evit++)
	{
		model->unselectVertex((*evit).first);
		model->selectVertex((*evit).second);
	}

	model->deleteOrphanedVertices(); return true;
}
void ExtrudeImpl::_makeFaces(Model *model, unsigned a, unsigned b)
{
	unsigned a2 = m_evMap[a];
	unsigned b2 = m_evMap[b];

	model->addTriangle(b,b2,a2);
	model->addTriangle(a2,a,b);
}
void ExtrudeImpl::_addSide(unsigned a, unsigned b)
{
	// Make sure a<b to simplify comparison below
	if(b<a)
	{
		unsigned c = a;
		a = b;
		b = c;
	}

	// Find existing side (if any)and increment count
	SideList::iterator it;
	for(it = m_sides.begin(); it!=m_sides.end(); it++)
	{
		if((*it).a==a&&(*it).b==b)
		{
			(*it).count++;
			log_debug("side (%d,%d)= %d\n",a,b,(*it).count);
			return;
		}
	}

	// Not found,add new side with a count of 1
	SideT s;
	s.a = a;
	s.b = b;
	s.count = 1;

	log_debug("side (%d,%d)= %d\n",a,b,s.count);
	m_sides.push_back(s);
}
bool ExtrudeImpl::_sideIsEdge(unsigned a, unsigned b)
{
	// Make sure a<b to simplify comparison below
	if(b<a)
	{
		unsigned c = a;
		a = b;
		b = c;
	}

	SideList::iterator it;
	for(it = m_sides.begin(); it!=m_sides.end(); it++)
	{
		if((*it).a==a&&(*it).b==b)
		{
			if((*it).count==1)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}