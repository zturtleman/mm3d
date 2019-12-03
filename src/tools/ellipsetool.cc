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

#include "pixmap/ellipsetool.xpm"

#include "model.h"
#include "log.h"
#include "modelstatus.h"

struct EllipseTool : Tool
{	
	EllipseTool():Tool(TT_Creator,1,TOOLS_CREATE_MENU)
	{
		m_smoothness = 2; //config defaults
		m_isSphere = m_fromCenter = false;
	}
				  		
	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Create Ellipsoid");
	}

	virtual const char **getPixmap(int){ return ellipsetool_xpm; }

	virtual const char *getKeymap(int){ return "F7"; }

	virtual void activated(int)
	{
		parent->addInt(true,&m_smoothness,TRANSLATE_NOOP("Param","Smoothness"),0,5);		
		parent->addBool(true,&m_isSphere,TRANSLATE_NOOP("Param","Sphere"));
		parent->addBool(true,&m_fromCenter,TRANSLATE_NOOP("Param","From Center"));
	}

	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);
		
		int m_smoothness;
		bool m_isSphere, m_fromCenter;

		bool m_created; //NEW
		ToolCoordList m_vertices;
		double m_startX,m_startY;
		
	void updateVertexCoords
	(double x, double y, double z, double xrad, double yrad, double zrad);
};

extern Tool *ellipsetool(){ return new EllipseTool; }

void EllipseTool::mouseButtonDown(int buttonState, int x, int y)
{
	m_created = false;
	
	double pos[3];
	parent->getParentXYValue(x,y,pos[0],pos[1],true);
	m_startX = pos[0]; m_startY = pos[1];
}
void EllipseTool::mouseButtonMove(int buttonState, int x, int y)
{
	double pos[2],rad[3];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	rad[0] = fabs(m_startX-pos[0])/2;
	rad[1] = fabs(m_startY-pos[1])/2;		
	rad[2] = std::min(rad[0],rad[1]);

	//NEW: Don't degenerate, as drawVertices now has trouble with
	//that, and it's just not cool.
	if(!rad[2]) return;

	if(!m_created)
	{
		m_created = true;

		Model *model = parent->getModel();

		const int select = model->getTriangleCount();
	
		// create rough 12-vertex sphere

		// Make top and bottom vertices
		auto v1 = addPosition(Model::PT_Vertex,0,1,0);
		auto v2 = addPosition(Model::PT_Vertex,0,-1,0);

		// Make center vertices
		double offset = sin(30*PIOVER180);
		double adjust = cos(30*PIOVER180);

		enum{ tN=5 }; int top[tN],bot[tN];
		double xoff,zoff; for(int t=0;t<tN;t++)
		{
			// upper vertex
			xoff = sin((t*72)*PIOVER180)*adjust;
			zoff = cos((t*72)*PIOVER180)*adjust;
			top[t] = addPosition(Model::PT_Vertex,nullptr,xoff,offset,zoff);

			// lower vertex
			xoff = sin((t*72+36)*PIOVER180)*adjust;
			zoff = cos((t*72+36)*PIOVER180)*adjust;
			bot[t] = addPosition(Model::PT_Vertex,nullptr,xoff,-offset,zoff);
		}

		// Create top and bottom faces
		model->addTriangle(v1,top[tN-1],top[0]);
		model->addTriangle(v2,bot[0],bot[tN-1]); 
		for(int t=1;t<tN;t++) model->addTriangle(v1,top[t-1],top[t]);	
		for(int t=tN-1;t-->0;) model->addTriangle(v2,bot[t+1],bot[t]);	

		model->addTriangle(top[0],bot[0],top[1]);
		model->addTriangle(top[1],bot[1],top[2]);
		model->addTriangle(top[2],bot[2],top[3]);
		model->addTriangle(top[3],bot[3],top[4]);
		model->addTriangle(top[4],bot[4],top[0]);

		model->addTriangle(bot[1],top[1],bot[0]);
		model->addTriangle(bot[2],top[2],bot[1]);
		model->addTriangle(bot[3],top[3],bot[2]);
		model->addTriangle(bot[4],top[4],bot[3]);
		model->addTriangle(bot[0],top[0],bot[4]);

		model->unselectAll();
		for(int i=model->getTriangleCount();i-->select;)
		model->selectTriangle(i);

		// create smooth sphere
		for(int i=0;i<m_smoothness;i++)
		model->subdivideSelectedTriangles();

		// TODO add makeSelectedToolCoordList?
		m_vertices.clear();
		pos_list posList;
		model->getSelectedPositions(posList);
		makeToolCoordList(m_vertices,posList);
		for(auto&ea:m_vertices) normalize3(ea.coords);

		model_status(model,StatusNormal,STATUSTIME_SHORT,TRANSLATE("Tool","Ellipsoid created"));
	}

	if(m_isSphere) rad[0] = rad[1] = rad[2];

	if(m_fromCenter)
	{
		pos[0] = m_startX;
		pos[1] = m_startY;
		rad[0]*=2; rad[1]*=2; rad[2]*=2;
	}
	else
	{
		pos[0] = (pos[0]+m_startX)/2;
		pos[1] = (pos[1]+m_startY)/2;
	}

	updateVertexCoords(pos[0],pos[1],0,rad[0],rad[1],rad[2]);

	parent->updateAllViews();
} 
void EllipseTool::updateVertexCoords
(double x, double y, double z, double xrad, double yrad, double zrad)
{
	for(auto&ea:m_vertices) movePosition
	(ea.pos,ea.coords[0]*xrad+x,ea.coords[1]*yrad+y,ea.coords[2]*zrad+z);
}

