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

#include "model.h"
#include "msg.h"
#include "log.h"
#include "modelstatus.h"

#include "pixmap/polytool.xpm"

struct PolyTool : Tool
{
	PolyTool():Tool(TT_Creator)
	{
		m_type = 0; //config defaults
	}

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Create Polygon");
	}

	virtual const char **getPixmap(int){ return polytool_xpm; }

	virtual const char *getKeymap(int){ return "Z"; }

	virtual void activated(int)
	{
		const char *e[2+1] = 
		{
		TRANSLATE_NOOP("Param","Strip","Triangle strip option"),
		TRANSLATE_NOOP("Param","Fan","Triangle fan option"),
		};
		parent->addEnum(true,&m_type,TRANSLATE_NOOP("Param","Poly Type"),e);
	}

	void mouseButtonDown(int buttonState, int x, int y);		
	void mouseButtonMove(int buttonState, int x, int y);
		
		int m_type;
		
		ToolCoordT m_lastVertex;

		//FIX ME
		//TODO: Add this and lift getParentXYValue(false)
		//constraint. Refer to MoveTool.
		//bool m_allowX,m_allowY;
};

extern Tool *polytool(){ return new PolyTool; }

void PolyTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	if(buttonState!=BS_Left) return;

	//FIX ME
	std::vector<int> selected;
	model->getSelectedVertices(selected);
	if(selected.size()>3) selected.resize(3);
	
	//TODO: Change to "true" and lift BS_Left constraint. 
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1],/*true*/false);
	m_lastVertex = addPosition(Model::PT_Vertex,pos[0],pos[1],0);

	if(3==selected.size())
	{
		int v = m_type?1:0;	
		model->unselectVertex(selected[v]);
		selected.erase(selected.begin()+v);
	}
	selected.push_back(m_lastVertex);

	int tri;
	if(3==selected.size())
	tri = model->addTriangle(selected[0],selected[1],selected[2]);		
	model->beginSelectionDifference();
	model->selectVertex(m_lastVertex);
	if(3==selected.size())
	{
		const Matrix &viewMatrix = 
		parent->getParentViewInverseMatrix();
		viewMatrix.show(); //???

		Vector viewNorm(0,0,1);
		viewNorm.transform3(viewMatrix);

		float fNorm[3];
		model->calculateNormals(); //FIX ME!
		model->getNormal(tri,0,fNorm);		
		Vector dNorm(fNorm[0],fNorm[1],fNorm[2]);

		log_debug("view normal is %f %f %f\n",viewNorm[0],viewNorm[1],viewNorm[2]);
		log_debug("triangle normal is %f %f %f\n",dNorm[0],dNorm[1],dNorm[2]);

		double d = viewNorm.dot3(dNorm);
		log_debug("dot product is %f\n",d);

		if(d<0) model->invertNormals(tri);
	}
	model->endSelectionDifference();

	parent->updateAllViews();	
}
void PolyTool::mouseButtonMove(int buttonState, int x, int y)
{
	if(buttonState==BS_Left)
	{
		//TODO: Change to "true" and lift BS_Left constraint. 
		double pos[2];
		parent->getParentXYValue(x,y,pos[0],pos[1],/*true*/false);
		movePosition(m_lastVertex.pos,pos[0],pos[1],0);

		parent->updateAllViews();
	}
}


