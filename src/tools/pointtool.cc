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

#include "pixmap/pointtool.xpm"

#include "model.h"
#include "msg.h"
#include "log.h"
#include "modelstatus.h"

struct PointTool : Tool
{
	PointTool():Tool(TT_Creator){}
		
	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Create Point"); 
	}

	virtual const char **getPixmap(int){ return pointtool_xpm; }

	virtual const char *getKeymap(int){ return "F12"; }

	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

		ToolCoordT m_point;
};

extern Tool *pointtool(){ return new PointTool; }

void PointTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1],true);

 	// Find a unique name for the projection
	char name[64] = "Point ";
	int nameN = sizeof("Point ")-1;
	int num = 0;
	int iN = model->getPointCount();
	for(int i=0;i<iN;i++)
	{
		const char *cmp = model->getPointName(i);
		if(!memcmp(cmp,name,nameN))
		num = std::max(num,atoi(cmp+nameN));
	}
	sprintf(name+nameN,"%d",num+1);

	m_point = addPosition(Model::PT_Point,name,pos[0],pos[1],0);

	model->unselectAll();
	model->selectPoint(m_point);
	
	model_status(model,StatusNormal,STATUSTIME_SHORT,
	TRANSLATE("Tool","Point created"));

	parent->updateAllViews();
}
void PointTool::mouseButtonMove(int buttonState, int x, int y)
{
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	movePosition(m_point.pos,pos[0],pos[1],0);

	parent->updateAllViews();
}

