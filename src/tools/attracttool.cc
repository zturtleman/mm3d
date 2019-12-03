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

#include "menuconf.h"
#include "tool.h"

#include "pixmap/atrfartool.xpm"
#include "pixmap/atrneartool.xpm"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "glmath.h"

struct AttractTool : Tool
{
	//Can this use a Near/Far option instead? Right/Left?

	AttractTool():Tool(TT_Other,2,TOOLS_ATTRACT_MENU)
	{}

	virtual const char *getName(int arg)
	{	
		switch(arg)
		{
		default: assert(0);
		case 0: return TRANSLATE_NOOP("Tool","Attract Near"); 
		case 1: return TRANSLATE_NOOP("Tool","Attract Far"); 
		}
	}

	virtual const char *getKeymap(int arg)
	{	
		//Note: These are basically translation 
		//tools. Y is beside T and looks like a
		//broken T.
		return arg?"Shift+Y":"Y";
	}

	virtual const char **getPixmap(int arg)
	{
		return arg?atrfartool_xpm:atrneartool_xpm; 
	}

	virtual void activated(int arg){ m_op = arg; }

	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

	//REMOVE ME
	virtual void mouseButtonUp(int buttonState, int x, int y)
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,m_op?
		TRANSLATE("Tool","Attract far complete"):
		TRANSLATE("Tool","Attract near complete"));
	}
		int m_op;

		double m_minDistance;
		double m_sepDistance;

		double m_startX;
		double m_startY;

		ToolCoordList m_positionCoords;
};

Tool *attracttool(){ return new AttractTool; }

void AttractTool::mouseButtonDown(int buttonState, int x, int y)
{	
	Model *model = parent->getModel();

	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1],true);	
	m_startX = pos[0];
	m_startY = pos[1];

	pos_list pl;
	model->getSelectedPositions(pl); 
	m_positionCoords.clear();
	makeToolCoordList(m_positionCoords,pl);

	double min = +DBL_MAX;
	double max = -DBL_MAX;
	for(auto&ea:m_positionCoords)
	{
		double d = distance(pos[0],pos[1],ea.coords[0],ea.coords[1]);
		ea.dist = d;

		min = std::min(min,d); max = std::max(max,d);
	}
	m_minDistance = min; m_sepDistance = max-min;
	
	model_status(model,StatusNormal,STATUSTIME_SHORT,m_op?
	TRANSLATE("Tool","Attracting far selected primitives"):
	TRANSLATE("Tool","Attracting near selected primitives"));
}
void AttractTool::mouseButtonMove(int buttonState, int x, int y)
{
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	double lengthX = pos[0]-m_startX;
	double lengthY = pos[1]-m_startY;

	double min = m_minDistance;
	double sep = m_sepDistance;
	if(sep>=0.00001f) if(!m_op)
	for(auto&ea:m_positionCoords)
	{
		double t = 1-(ea.dist-min)/sep;
		movePosition(ea.pos,ea.coords[0]+lengthX*t,ea.coords[1]+lengthY*t,ea.coords[2]);		
	}
	else for(auto&ea:m_positionCoords)
	{
		double t = (ea.dist-min)/sep;
		movePosition(ea.pos,ea.coords[0]+lengthX*t,ea.coords[1]+lengthY*t,ea.coords[2]);
	}

	parent->updateAllViews();
}




