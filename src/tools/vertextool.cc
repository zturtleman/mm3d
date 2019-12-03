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

#include "pixmap/vertextool.xpm"

#include "model.h"
#include "msg.h"
#include "log.h"
#include "modelstatus.h"

struct VertexTool : Tool
{
	VertexTool():Tool(TT_Creator){}

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Create Vertex");
	}

	virtual const char **getPixmap(int){ return vertextool_xpm; }

	virtual const char *getKeymap(int){ return "Shift+Z"; }
	
	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

		ToolCoordT m_vertex;
};

extern Tool *vertextool(){ return new VertexTool; }

void VertexTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1],true);

	m_vertex = addPosition(Model::PT_Vertex,pos[0],pos[1],0);
	model->setVertexFree(m_vertex,true);

	model->unselectAll();
	model->selectVertex(m_vertex);
	
	model_status(model,StatusNormal,STATUSTIME_SHORT,
	TRANSLATE("Tool","Vertex created"));

	parent->updateAllViews();
}
void VertexTool::mouseButtonMove(int buttonState, int x, int y)
{
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	movePosition(m_vertex.pos,pos[0],pos[1],0);

	parent->updateAllViews();
}