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

#include "pixmap/sheartool.xpm"

#include "model.h"
#include "modelstatus.h"
#include "log.h"

struct ShearTool : Tool
{
	ShearTool():Tool(TT_Other){}

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Shear");
	}

	virtual const char *getKeymap(int){ return "Shift+S"; }

	virtual const char **getPixmap(int){ return sheartool_xpm; }
	 
	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

	//REMOVE ME
	virtual void mouseButtonUp(int buttonState, int x, int y)
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("Tool","Shear complete"));
	}
		bool m_axis;

		double m_far;
		double m_orig;

		double m_startLength;

		ToolCoordList m_positionCoords;
};

extern Tool *sheartool(){ return new ShearTool; }

void ShearTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	pos_list posList;
	model->getSelectedPositions(posList);
	m_positionCoords.clear();
	makeToolCoordList(m_positionCoords,posList);

	double cminX = +DBL_MAX, cminY = +DBL_MAX;
	double cmaxX = -DBL_MAX, cmaxY = -DBL_MAX;

	for(auto&ea:m_positionCoords)
	{
		cminX = std::min(cminX,ea.coords[0]);
		cmaxX = std::max(cmaxX,ea.coords[0]);
		cminY = std::min(cminY,ea.coords[1]);
		cmaxY = std::max(cmaxY,ea.coords[1]);
	}

	double curX,curY;
	parent->getParentXYValue(x,y,curX,curY,true);

	double minX = fabs(cminX-curX);
	double minY = fabs(cminY-curY);
	double maxX = fabs(cmaxX-curX);
	double maxY = fabs(cmaxY-curY);

	if(minX>maxX)
	{
		if(minX>minY)
		{
			if(minX>maxY)
			{
				m_axis = 1;
				m_far = cminX;
				m_orig = curY;
			}
			else
			{
				m_axis = 0;
				m_far = cmaxY;
				m_orig = curX;
			}
		}
		else same: // minY>cminX
		{
			if(minY>maxY)
			{
				m_axis = 0;
				m_far = cminY;
				m_orig = curX;
			}
			else
			{
				m_axis = 0;
				m_far = cmaxY;
				m_orig = curX;
			}
		}
	}
	else // maxX>minX
	{
		if(maxX>minY)
		{
			if(maxX>maxY)
			{
				m_axis = 1;
				m_far = cmaxX;
				m_orig = curY;
			}
			else
			{
				m_axis = 0;
				m_far = cmaxY;
				m_orig = curX;
			}
		}
		else goto same;
	}

	m_startLength = fabs(m_far-(m_axis?curX:curY));

	model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,
	TRANSLATE("Tool","Starting shear on selected primitives"));
}
void ShearTool::mouseButtonMove(int buttonState, int x, int y)
{
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	int a = m_axis, b = !m_axis;

	double offset = pos[a]-m_orig;
	double length = m_startLength;

	for(auto&ea:m_positionCoords)
	{
		double xy[2] = { ea.coords[0],ea.coords[1] };

		xy[a]+=offset*fabs(xy[b]-m_far)/length;			

		movePosition(ea.pos,xy[0],xy[1],ea.coords[2]);
	}

	parent->updateAllViews();
}
