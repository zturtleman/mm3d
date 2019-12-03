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

#include "pixmap/movetool.xpm"

#include "model.h"
#include "modelstatus.h"
#include "log.h"

struct MoveTool : Tool
{
	MoveTool():Tool(TT_Other){}

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Move");
	}

	//2019: T is beside R and not on the right-hand side of
	//the keyboard. T for Translate. M for Material.
	virtual const char *getKeymap(int){ return "T"; } //"M"

	virtual const char **getPixmap(int){ return movetool_xpm; }	

	virtual void activated(int)
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_NONE,
		TRANSLATE("Tool","Tip: Hold shift to restrict movement to one dimension"));
	}
	
	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

	//REMOVE ME
	virtual void mouseButtonUp(int buttonState, int x, int y)
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("Tool","Move complete"));
		parent->updateAllViews();
	}		
		double m_x,m_y;

		bool m_allowX,m_allowY;
};

extern Tool *movetool(){ return new MoveTool; }

void MoveTool::mouseButtonDown(int buttonState, int x, int y)
{
	parent->getParentXYValue(x,y,m_x,m_y,true);

	m_allowX = m_allowY = true;
	
	model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,
	TRANSLATE("Tool","Moving selected primitives"));
}
void MoveTool::mouseButtonMove(int buttonState, int x, int y)
{	
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	if(buttonState&BS_Shift&&m_allowX&&m_allowY)
	{
		double ax = fabs(pos[0]-m_x);
		double ay = fabs(pos[1]-m_y);

		if(ax>ay) m_allowY = false;
		if(ay>ax) m_allowX = false;
	}

	if(!m_allowX) pos[0] = m_x;
	if(!m_allowY) pos[1] = m_y;

	double v[4] = { pos[0]-m_x,pos[1]-m_y,0,1 };

	m_x = pos[0]; m_y = pos[1];

	parent->getParentViewInverseMatrix().apply3(v);

	Matrix m; //???
	m.set(3,0,v[0]);
	m.set(3,1,v[1]);
	m.set(3,2,v[2]);

	//FIX ME: Translate via vector.
	parent->getModel()->translateSelected(m);
	parent->updateAllViews();
}

