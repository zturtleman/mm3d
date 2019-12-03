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

#include "tool.h"
#include "menuconf.h" //TOOLS_CREATE_MENU

#include "pixmap/cylindertool.xpm"

#include "model.h"
#include "glmath.h"
#include "log.h"
#include "modelstatus.h"

struct CylinderTool : Tool
{
	CylinderTool():Tool(TT_Creator,1,TOOLS_CREATE_MENU),m_inverted()
	{	
		m_segments = 4; m_sides = 8; //config defaults
		m_width = m_scale = 100;
	}
		
	virtual const char *getName(int arg)
	{
		return TRANSLATE_NOOP("Tool","Create Cylinder");
	}

	virtual const char **getPixmap(int){ return cylindertool_xpm; }

	virtual const char *getKeymap(int){ return "F8"; }

	virtual void activated(int)
	{
		parent->addInt(true,&m_segments,TRANSLATE_NOOP("Param","Segments"),1,32); //100
		parent->addInt(true,&m_sides,TRANSLATE_NOOP("Param","Sides"),3,32); //100
		parent->addDouble(true,&m_width,TRANSLATE_NOOP("Param","Width"),0,100);
		parent->addDouble(true,&m_scale,TRANSLATE_NOOP("Param","Scale"),0,100);
	}
	
	virtual void mouseButtonDown(int buttonState, int x, int y);	
	virtual void mouseButtonMove(int buttonState, int x, int y);
		
		int m_segments, m_sides;
		double m_width, m_scale;

		bool m_inverted;
		ToolCoordList m_vertices;
		double m_startX,m_startY;

	void updateVertexCoords
	(double x, double y, double z, double x2, double y2, double z2); 		
};

extern Tool *cylindertool(){ return new CylinderTool; }

void CylinderTool::mouseButtonDown(int buttonState, int x, int y)
{	
	m_vertices.clear(); m_inverted = false;

	Model *model = parent->getModel();
	
	const int select = model->getTriangleCount();

	const int iN = m_segments, jN = m_sides;

	double s = m_scale/100, l_s = 1-s;

	// Create external cylinder
	for(int i=0;i<=iN;i++)
	{
		double vx = (double)i/iN;		
		double scale = vx*l_s+s; //lerp
		for(int j=0;j<jN;j++)
		{
			double theta = 2*PI*j/jN;
			double vy = cos(theta)*scale;
			double vz = sin(theta)*scale;
			ToolCoordT ev = addPosition(Model::PT_Vertex,vx,vy,vz);
			m_vertices.push_back(ev);
		}

		auto vb = m_vertices.data();
		if(i) for(int j=0;j<jN;j++)
		{
			int next = j+1; if(next>=jN) next = 0;

			int w = i*jN+next;
			int x = (i-1)*jN+j;
			int y = (i-1)*jN+next;
			int z = i*jN+j;
			model->addTriangle(vb[x],vb[y],vb[z]);
			model->addTriangle(vb[y],vb[w],vb[z]);
		}
	}

	int off = (unsigned)m_vertices.size();

	if(m_width>=100) // No internal cylinder
	{
		off-=jN;

		// Add ends
		ToolCoordT ev1 = addPosition(Model::PT_Vertex,0,0,0);
		ToolCoordT ev2 = addPosition(Model::PT_Vertex,1,0,0);
		m_vertices.push_back(ev1);
		m_vertices.push_back(ev2);
		auto vb = m_vertices.data();
		for(int j=0;j<jN;j++)
		{
			int next = j+1; if(next>=jN) next = 0;
			model->addTriangle(ev1.pos.index,m_vertices[next],m_vertices[j]);
		}
		for(int j=0;j<jN;j++)
		{
			int next = j+1; if(next>=jN) next = 0;
			model->addTriangle(ev2.pos.index,vb[off+j],vb[off+next]);
		}
	}
	else // Create internal cylinder
	{
		double rad = std::min(1-m_width/100,0.999);
		for(int i=0;i<=iN;i++)
		{
			double vx = (double)i/iN;
			double scale = vx*l_s+s; //lerp
			scale*=rad;
			for(int j=0;j<jN;j++)
			{
				double theta = 2*PI*j/jN;
				double vy = cos(theta)*scale;
				double vz = sin(theta)*scale;
				ToolCoordT ev = addPosition(Model::PT_Vertex,vx,vy,vz);
				m_vertices.push_back(ev);
			}

			auto vb = m_vertices.data();
			if(i) for(int j=0;j<jN;j++)
			{
				int next = j+1; if(next>=jN) next = 0;

				int w = off+i*jN+next;
				int x = off+(i-1)*jN+j;
				int y = off+(i-1)*jN+next;
				int z = off+i*jN+j;
				model->addTriangle(vb[y],vb[x],vb[z]);
				model->addTriangle(vb[y],vb[z],vb[w]);
			}
		}

		// Add ends
		auto vb = m_vertices.data();
		int off2 = (unsigned)m_vertices.size()-jN;
		for(int j=0;j<jN;j++)
		{
			int next = j+1; if(next>=jN) next = 0;
			model->addTriangle(vb[next],vb[j],vb[off+j]);
			model->addTriangle(vb[next],vb[off+j],vb[off+next]);
		}
		for(int j=0;j<jN;j++) //NEW: Don't interleave cap polygons.
		{
			int next = j+1; if(next>=jN) next = 0;
			model->addTriangle(vb[off-jN+j],vb[off-jN+next],vb[off2+j]);
			model->addTriangle(vb[off-jN+next],vb[off2+next],vb[off2+j]);
		}
	}

	model->unselectAll();
	for(int i=model->getTriangleCount();i-->select;)
	model->selectTriangle(i);

	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1],true);	
	m_startX = pos[0];
	m_startY = pos[1];
	updateVertexCoords(pos[0],pos[1],0,0,0,0);

	parent->updateAllViews();

	model_status(model,StatusNormal,STATUSTIME_SHORT,TRANSLATE("Tool","Cylinder created"));
}
void CylinderTool::mouseButtonMove(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	double rad[3],pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);	
	rad[0] = pos[0]-m_startX;
	rad[1] = //???
	rad[2] = fabs(m_startY-pos[1])/2;
	pos[0] = m_startX;
	pos[1] = (m_startY+pos[1])/2;

	if(m_inverted==(rad[0]>=0))
	{
		m_inverted = !m_inverted;

		int_list selectedList;
		model->getSelectedTriangles(selectedList);
		for(int ea:selectedList) model->invertNormals(ea);
	}

	updateVertexCoords(pos[0],pos[1],0,rad[0],rad[1],rad[2]);

	parent->updateAllViews();
}
void CylinderTool::updateVertexCoords
(double x, double y, double z, double xrad, double yrad, double zrad)
{
	for(auto&ea:m_vertices) movePosition
	(ea.pos,ea.coords[0]*xrad+x,ea.coords[1]*yrad+y,ea.coords[2]*zrad+z);
}


