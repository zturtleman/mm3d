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

#include "pixmap/torustool.xpm"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "weld.h"

struct TorusTool : Tool
{
	TorusTool():Tool(TT_Creator,1,TOOLS_CREATE_MENU),m_inverted()
	{
		m_segments = m_sides = 8; //config defaults
		m_width = 50;
		m_circle = m_center = false;
	}
		
	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Create Torus");
	}

	virtual const char **getPixmap(int){ return torustool_xpm; }

	virtual const char *getKeymap(int){ return "F9"; }

	virtual void activated(int)
	{
		parent->addInt(true,&m_segments,TRANSLATE_NOOP("Param","Segments"),3,32); //100
		parent->addInt(true,&m_sides,TRANSLATE_NOOP("Param","Sides"),3,32); //100
		parent->addDouble(true,&m_width,TRANSLATE_NOOP("Param","Width"),0,100);
		parent->addBool(true,&m_circle,TRANSLATE_NOOP("Param","Circle"));
		parent->addBool(true,&m_center,TRANSLATE_NOOP("Param","From Center","Checkbox that indicates if torus is created from center or from far corner"));
	}

	virtual void mouseButtonDown(int buttonState, int x, int y);	
	virtual void mouseButtonMove(int buttonState, int x, int y);
	
	virtual void mouseButtonUp(int buttonState, int x, int y)
	{
		weldSelectedVertices(parent->getModel());		
		parent->updateAllViews();		
	}	
		int m_segments;
		int m_sides;
		double m_width;
		bool m_circle;
		bool m_center;

		bool m_created; //NEW
		bool m_inverted;
		ToolCoordList m_vertices;

		double m_diameter;

		double m_startX,m_startY;
	
	void updateDimensions(double xdiff, double ydiff, double zdiff);
};

extern Tool *torustool(){ return new TorusTool; }

void TorusTool::mouseButtonDown(int buttonState, int x, int y)
{
	m_created = m_inverted = false;

	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1],true);
	m_startX = pos[0]; m_startY = pos[1];
}
void TorusTool::mouseButtonMove(int buttonState, int x, int y)
{	
	Model *model = parent->getModel();

	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	double xdiff = pos[0]-m_startX;
	double ydiff = pos[1]-m_startY;
	if(m_circle)
	{
		double diff = sqrt(fabs(xdiff*ydiff));
		ydiff = ydiff<0?-diff:diff;
		xdiff = xdiff<0?-diff:diff;
	}
	double zdiff = std::max(fabs(xdiff),fabs(ydiff))*m_diameter*0.25;
 	
	//NEW: Don't degenerate, as drawVertices now has trouble with
	//that, and it's just not cool.
	if(!zdiff) return;

	if(!m_created)
	{
		m_created = true;

		m_vertices.clear();
 	
		m_diameter = m_width/100;
		double rad = 1-m_diameter/2;

		const int selected = model->getTriangleCount();
	
		int iN = m_segments, jN = m_sides;

		for(int i=0;i<=iN;i++)
		{
			double angle = PI*2*i/iN;
			double rx = cos(angle);
			double ry = sin(angle);
			for(int j=0;j<=jN;j++)
			{
				angle = PI*2*j/jN;
				double c = cos(angle)*m_diameter/2;
				double cz = sin(angle);	
				double cx = (1+rx*rad+rx*c)/2;
				double cy = (1+ry*rad+ry*c)/2;
			 
				m_vertices.push_back
				(addPosition(Model::PT_Vertex,cx,cy,cz));
			}

			if(i)
			{
				int vc = model->getVertexCount();
				int vbase1 = vc-(jN+1)*2;
				int vbase2 = vc-(jN+1);

				for(int j=0;j<jN;j++)
				{
					model->addTriangle(vbase1+j,vbase1+j+1,vbase2+j);
					model->addTriangle(vbase1+j+1,vbase2+j+1,vbase2+j);
				}
			}
		}
	
		model->unselectAll();
		for(int i=model->getTriangleCount();i-->selected;)
		model->selectTriangle(i);

		updateDimensions(0,0,0);

		parent->updateAllViews();

		model_status(model,StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("Tool","Torus created"));
	}

	updateDimensions(xdiff,ydiff,zdiff);

	if(m_inverted!=(xdiff<0&&ydiff<0||xdiff>0&&ydiff>0))
	{
		m_inverted = !m_inverted;

		for(int i=model->getTriangleCount();i-->0;)		
		if(model->isTriangleSelected(i))
		model->invertNormals(i);
	}

	parent->updateAllViews();
}
void TorusTool::updateDimensions(double xdiff, double ydiff, double zdiff)
{
	double centerX = m_startX, centerY = m_startY; if(m_center)
	{
		centerX-=xdiff; centerY-=ydiff; xdiff*=2; ydiff*=2; zdiff*=2;
	}
	for(auto&ea:m_vertices) movePosition
	(ea.pos,centerX+xdiff*ea.coords[0],centerY+ydiff*ea.coords[1],zdiff*ea.coords[2]);		
}