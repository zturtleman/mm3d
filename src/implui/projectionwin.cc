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
#include "win.h"

#include "projectionwin.h"
#include "viewwin.h"


#include "model.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"

void ProjectionWin::init()
{
	m_ignoreChange = false;

	type.add_item(0,"Cylinder");
	type.add_item(1,"Sphere");
	type.add_item(2,"Plane");

	// TODO handle undo of select/unselect?
	// Can't do this until after constructor is done because of observer interface
	//setModel(model);
	texture.setModel(model);
	texture.setInteractive(true);
	texture.setMouseOperation(Widget::MouseRange);
	texture.setDrawVertices(false);
	//texture.setMouseTracking(true); //QWidget //???
}
void ProjectionWin::submit(int id)
{
	int p = projection; switch(id)
	{	
	case id_item:
	{
		type.select_id(model->getProjectionType(p));

		double x,y,xx,yy;
		model->getProjectionRange(p,x,y,xx,yy);
		texture.setRange(x,y,xx,yy);

		goto refresh;
	}
	case id_subitem:
	
		model->setProjectionType(p,type);
		operationComplete(::tr("Set Projection Type","operation complete"));
		goto refresh;	
	
	case '+': texture.zoomIn(); break;
	case '-': texture.zoomOut(); break;
	case '=': texture.setZoomLevel(zoom.value); break;
		
	case id_name:
	{
		std::string name = model->getProjectionName(p);
		if(id_ok==EditBox(&name,::tr("Rename projection","window title"),::tr("Enter new point name:")))		
		{
			model->setProjectionName(p,name.c_str());
			projection.selection()->text() = name;
			operationComplete(::tr("Rename Projection","operation complete"));
		}
		break;
	}
	case id_apply: //???
		
		updateDone(); //applyProjectionEvent(); 
		break;

	case id_reset:
	
		model->setProjectionRange(p,0,0,1,1);

		//updateDone();
		operationComplete(::tr("Reset UV Coordinates","operation complete"));
		addProjectionTriangles();
		break;

	case id_remove: p = -1; 
	case id_append:
	
		for(int i=model->getTriangleCount();i-->0;)
		{
			if(model->isTriangleSelected(i))
			{
				//FIX ME: This is one Undo per triangle!
				model->setTriangleProjection(i,p);
			}
		}
		if(p!=-1) model->applyProjection(p);		

		operationComplete(::tr("Set Triangle Projection","operation complete"));
	
		refresh: 
		refreshProjectionDisplay();
		break;

	case id_scene:

		texture.draw(scene.x(),scene.y(),scene.width(),scene.height());
		break;

	case id_ok: case id_close:

		event.close_ui_by_create_id(); //Help?

		//I guess this model saves users' work?
		hide(); return;
	}

	basic_submit(id);
}

void ProjectionWin::open()
{
	setModel();
	
	// If we are visible, setModel already did this
	if(hidden())
	{
		//REMINDER: GLX needs to show before
		//it can use OpenGL.
		show(); openModel();
	}
}
void ProjectionWin::setModel()
{				  
	texture.setModel(model);

	if(!hidden()) openModel();
}
void ProjectionWin::modelChanged(int changeBits)
{
	if(m_ignoreChange) return;
	
	// TODO need some way to re-select the projection we were looking at
	if(!hidden())
	if(model->getProjectionCount()!=projection.find_line_count())
	{
		// A projection was added or deleted, we need to select a new
		// projection and re-initialize everything. 
		openModel();
	}
	else
	{
		// a change to projection itself,or a non-projection change,just 
		// re-initialize the projection display for the current projection
		int p = projection;
		projection.selection()->text() = model->getProjectionName(p);
		type.select_id(model->getProjectionType(p));		
		addProjectionTriangles();
	}
}
void ProjectionWin::openModel()
{
	projection.select_id(-1).clear();

	int iN = model?model->getProjectionCount():0;	
	if(!enable(iN!=0).enabled())
	{
		ok.nav.enable(); return;
	}
	
	for(int i=0;i<iN;i++)
	projection.add_item(i,model->getProjectionName(i));

	int p = -1;
	for(int i=0;i<iN;i++)
	if(model->isProjectionSelected(i))
	{
		p = i; break;
	}
	iN = model->getTriangleCount();
	if(p==-1)
	for(int i=0;i<iN;i++)
	if(model->isTriangleSelected(i))
	{
		int pp = model->getTriangleProjection(i);
		if(pp<0) continue;
		p = pp; break;
	}	
	projection.select_id(p==-1?0:p); submit(id_item);
}

void ProjectionWin::refreshProjectionDisplay()
{
	texture.clearCoordinates();

	if(!projection.empty())
	{			
		int iN = model->getTriangleCount();
		int proj = projection;
		for(int i=0;i<iN;i++)
		{
			//FIX ME: getTriangleGroup is dumb!!
			//FIX ME: getTriangleGroup is dumb!!
			//FIX ME: getTriangleGroup is dumb!!
			//FIX ME: getTriangleGroup is dumb!!
			int p = model->getTriangleProjection(i);
			int g = model->getTriangleGroup(i);
			int material = model->getGroupTextureId(g);
			if(p==proj&&g>=0&&material>=0)
			{
				texture.setTexture(material); 
				return addProjectionTriangles();
			}
		}
	}
	texture.setTexture(-1);

	//REFERENCE
	//DecalManager::getInstance()->modelUpdated(model); //???
}

void ProjectionWin::addProjectionTriangles()
{
	texture.clearCoordinates();

	int p = projection;

	double x,y,xx,yy;
	model->getProjectionRange(p,x,y,xx,yy);
	texture.setRange(x,y,xx,yy);

	float s,t;
	int iN = model->getTriangleCount();
	for(int i=0,v=0;i<iN;i++,v+=3)	
	if(p==model->getTriangleProjection(i))
	{
		for(int j=0;j<3;j++)
		{				
			model->getTextureCoords(i,j,s,t);
			texture.addVertex(s,t);
		}
		texture.addTriangle(v,v+1,v+2);
	}
	texture.updateWidget();

	//REFERENCE
	//DecalManager::getInstance()->modelUpdated(model); //???
}

void ProjectionWin::rangeChanged()
{
	double x,y,xx,yy;
	texture.getRange(x,y,xx,yy);
	model->setProjectionRange((int)projection,x,y,xx,yy);

	addProjectionTriangles();
}
void ProjectionWin::seamChanged(double xDiff, double yDiff)
{
	if(xDiff==0) return; //Zero-divide?
		
	int p = projection;

	double up[4] = { 0,0,0,1 };
	double seam[4] = { 0,0,0,1 };

	model->getProjectionUp(p,up);
	model->getProjectionSeam(p,seam);

	Matrix m;
	m.setRotationOnAxis(up,xDiff);
	m.apply3(seam);

	model->setProjectionSeam(p,seam);

	addProjectionTriangles();
}
void ProjectionWin::updateDone()
{
	int p = projection; if(p==-1) return; //Dragging?

	model->applyProjection(p); 
	
	addProjectionTriangles();

	operationComplete(::tr("Apply Projection","operation complete"));
}
void ProjectionWin::operationComplete(const char *opname)
{	
	m_ignoreChange = true;
	{
		model->operationComplete(opname);
	}
	m_ignoreChange = false;
}
