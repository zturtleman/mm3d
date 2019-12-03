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

#include "pixmap/rectangletool.xpm"

#include "log.h"
#include "model.h"
#include "modelstatus.h"

struct RectangleTool : Tool
{
	RectangleTool():Tool(TT_Creator,1,TOOLS_CREATE_MENU){}
		
	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Create Rectangle");
	}

	virtual const char **getPixmap(int){ return rectangletool_xpm; }
 
	virtual const char *getKeymap(int){ return "F5"; }

	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

		double m_x,m_y;

		ToolCoordT m_v1,m_v2,m_v3,m_v4;
};

extern Tool *rectangletool(){ return new RectangleTool; }

void RectangleTool::mouseButtonDown(int buttonState, int x, int y)
{
	parent->getParentXYValue(x,y,m_x,m_y);

	Model *model = parent->getModel();

	//log_debug("model has %d vertices\n",model->getVertexCount()); //???

	for(int i=0;i<4;i++)
	(&m_v1)[i] = addPosition(Model::PT_Vertex,m_x,m_y,0);

	//log_debug("last new vertex: %d\n",m_v4.pos.index); //???

	model->addTriangle(m_v1,m_v2,m_v4);
	model->addTriangle(m_v4,m_v3,m_v1);

	model->unselectAll();
	for(int i=0;i<4;i++)
	model->selectVertex((&m_v1)[i]);
	
	model_status(model,StatusNormal,STATUSTIME_SHORT,
	TRANSLATE("Tool","Rectangle created"));

	parent->updateAllViews();
}
void RectangleTool::mouseButtonMove(int buttonState, int x, int y)
{
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	movePosition(m_v2.pos,m_x,pos[1],0);
	movePosition(m_v3.pos,pos[0],m_y,0);
	movePosition(m_v4.pos,pos[0],pos[1],0);

	parent->updateAllViews();
}
