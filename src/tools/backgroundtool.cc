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

#include "pixmap/bgmovetool.xpm"
#include "pixmap/bgscaletool.xpm"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "glmath.h"

struct BackgroundTool : Tool
{
	BackgroundTool():Tool(TT_Other,2,TOOLS_BACKGROUND_MENU)
	{}
	
	virtual const char *getName(int arg)
	{	
		switch(arg)
		{
		default: assert(0);
		case 0: return TRANSLATE_NOOP("Tool","Scale Background Image"); 
		case 1: return TRANSLATE_NOOP("Tool","Move Background Image"); 
		}
	}

	virtual const char *getKeymap(int arg)
	{	
		//NOTE: I imagine using your left hand to reach across the 
		//keyboard to do this, depending on what is most intuitive.
		return arg?"Ctrl+Back":"Shift+Back";
	}

	virtual const char **getPixmap(int arg)
	{
		return arg?bgmovetool_xpm:bgscaletool_xpm; 
	}

	virtual void activated(int arg)
	{
		m_op = arg; 

		model_status(parent->getModel(),StatusNormal,STATUSTIME_NONE,arg?
		TRANSLATE("Tool","Move background image"):
		TRANSLATE("Tool","Scale background image"));
	}
 
	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);
	
	//REMOVE ME
	virtual void mouseButtonUp(int buttonState, int x, int y)
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,m_op?
		TRANSLATE("Tool","Background move complete"):
		TRANSLATE("Tool","Scale background image"));
	}

		int m_op;

		double m_x,m_y,m_z;

		double m_startLength,m_startScale;
};

Tool *backgroundtool(){ return new BackgroundTool; }

void BackgroundTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();
	int index = parent->getView()-1;
	if((unsigned int)index<Model::MAX_BACKGROUND_IMAGES) //NEW
	{
		parent->getXYZ(x,y,&m_x,&m_y,&m_z);

		if(!m_op) //Scaling?
		{
			float cenX,cenY,cenZ;
			model->getBackgroundCenter(index,cenX,cenY,cenZ);
			m_startScale  = model->getBackgroundScale(index);
			m_startLength = distance((double)cenX,(double)cenY,(double)cenZ,m_x,m_y,m_z);
			m_startLength = std::max(m_startLength,0.0001);

			//log_debug("center (%f,%f,%f) mouse (%f,%f,%f)\n",cenX,cenY,cenZ,tempX,tempY,tempZ);
			//log_debug("starting background scale with length %f,scale %f\n",m_startLength,m_startScale);
		}
	}
	else index = -1;

	const char *msg = nullptr; 
	if(m_op==0&&index>=0) msg = TRANSLATE("Tool","Scaling background image");
	if(m_op==0&&index<0) msg = TRANSLATE("Tool","Cannot scale background from 3D view");
	if(m_op==1&&index>=0) msg = TRANSLATE("Tool","Moving background image");
	if(m_op==1&&index<0) msg = TRANSLATE("Tool","Cannot move background from 3D view");
	model_status(model,StatusNormal,STATUSTIME_SHORT,msg);
}
void BackgroundTool::mouseButtonMove(int buttonState, int x, int y)
{
	Model *model = parent->getModel();
	int index = parent->getView()-1;
	if((unsigned)index<Model::MAX_BACKGROUND_IMAGES) //NEW
	{
		double newX,newY,newZ;
		parent->getXYZ(x,y,&newX,&newY,&newZ);

		float cenX,cenY,cenZ;
		model->getBackgroundCenter(index,cenX,cenY,cenZ);

		if(m_op) //Moving?
		{
			cenX+=(float)(newX-m_x);
			cenY+=(float)(newY-m_y);
			cenZ+=(float)(newZ-m_z);
			model->setBackgroundCenter(index,cenX,cenY,cenZ);

			m_x = newX;
			m_y = newY;
			m_z = newZ;
		}
		else //Scaling?
		{
			double length = distance((double)cenX,(double)cenY,(double)cenZ,newX,newY,newZ);
			double scale  = length*m_startScale/m_startLength;

			//log_debug("scale with length %f,to scale %f\n",length,scale);

			model->setBackgroundScale(index,(float)scale);
		}
	}

	parent->updateAllViews();
}

