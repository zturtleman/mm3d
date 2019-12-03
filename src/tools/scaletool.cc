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
#include "glmath.h" //distance

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "pixmap/scaletool.xpm"

enum ScaleProportionE
{
	ST_ScaleFree=0,
	ST_ScaleProportion2D,
	ST_ScaleProportion3D,
};

enum ScalePointE
{
	ST_ScalePointCenter=0,
	ST_ScalePointFar,
};

struct ScaleTool : Tool
{
	ScaleTool():Tool(TT_Other)
	{
		m_proportion = m_point = 0; //config details
	}

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Scale");
	}

	virtual const char *getKeymap(int){ return "S"; }

	virtual const char **getPixmap(int){ return scaletool_xpm; }

	virtual void activated(int)
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_NONE,
		TRANSLATE("Tool","Tip: Hold shift to restrict scaling to one dimension"));

		const char *e[3+1] = 
		{
		TRANSLATE("Tool","Free","Free scaling option"),
		TRANSLATE("Tool","Keep Aspect 2D","2D scaling aspect option"),
		TRANSLATE("Tool","Keep Aspect 3D","3D scaling aspect option"),
		};
		parent->addEnum(true,&m_proportion,TRANSLATE("Tool","Proportion"),e);

		const char *f[2+1] = 
		{
		TRANSLATE("Tool","Center","Scale from center"),
		TRANSLATE("Tool","Far Corner","Scale from far corner"),
		};
		parent->addEnum(true,&m_point,TRANSLATE("Tool","Point"),f);
	}

	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

	//REMOVE ME
	virtual void mouseButtonUp(int buttonState, int x, int y)
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("Tool","Scale complete"));
	}
		int m_proportion;
		int m_point;		

		double m_x,m_y;
		bool m_allowX,m_allowY;

		double m_pointX;
		double m_pointY;
		double m_pointZ;

		double m_startLengthX;
		double m_startLengthY;

		//double m_projScale;
		//int_list m_projList;
		std::vector<std::pair<int,double>> m_projList;

		ToolCoordList m_positionCoords;
};

extern Tool *scaletool(){ return new ScaleTool; }

void ScaleTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	m_allowX = m_allowY = true;
			
	pos_list posList;
	model->getSelectedPositions(posList);
	m_positionCoords.clear();
	makeToolCoordList(m_positionCoords,posList);

	double min[3] = {+DBL_MAX,+DBL_MAX,+DBL_MAX}; 
	double max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX};

	m_projList.clear(); for(auto&ea:m_positionCoords)
	{
		for(int i=0;i<3;i++)
		{
			min[i] = std::min(min[i],ea.coords[i]);
			max[i] = std::max(max[i],ea.coords[i]);
		}

		if(ea.pos.type==Model::PT_Projection)
		{
			//log_debug("found projection %d\n",ea.pos.index);
			m_projList.push_back(std::make_pair(ea.pos.index,model->getProjectionScale(ea)));
		}
	}

	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1],true);
	m_x = pos[0];
	m_y = pos[1];
	if(m_point==ST_ScalePointFar)
	{
		//NOTE: This looks wrong, but seems to not matter.
		m_pointZ = min[2];

		//DUPLICATES texwidget.cc.

		//NOTE: sqrt not required.
		double minmin = distance(min[0],min[1],pos[0],pos[1]);
		double minmax = distance(min[0],max[1],pos[0],pos[1]);
		double maxmin = distance(max[0],min[1],pos[0],pos[1]);
		double maxmax = distance(max[0],max[1],pos[0],pos[1]);

		//Can this be simplified?
		if(minmin>minmax)
		{
			if(minmin>maxmin)
			{
				if(minmin>maxmax)
				{
					m_pointX = min[0]; m_pointY = min[1];
				}
				else
				{
					m_pointX = max[0]; m_pointY = max[1];
				}
			}
			else same: // maxmin>minmin
			{
				if(maxmin>maxmax)
				{
					m_pointX = max[0]; m_pointY = min[1];
				}
				else
				{
					m_pointX = max[0]; m_pointY = max[1];
				}
			}
		}
		else // minmax>minmin
		{
			if(minmax>maxmin)
			{
				if(minmax>maxmax)
				{
					m_pointX = min[0]; m_pointY = max[1];
				}
				else
				{
					m_pointX = max[0]; m_pointY = max[1];
				}
			}
			else goto same; // maxmin>minmax
		}

		m_startLengthX = fabs(m_pointX-pos[0]);
		m_startLengthY = fabs(m_pointY-pos[1]);
	}
	else
	{
		m_pointX = (max[0]-min[0])/2+min[0];
		m_pointY = (max[1]-min[1])/2+min[1];
		m_pointZ = (max[2]-min[2])/2+min[2];

		m_startLengthX = fabs(m_pointX-pos[0]);
		m_startLengthY = fabs(m_pointY-pos[1]);
	}

	model_status(model,StatusNormal,STATUSTIME_SHORT,
	TRANSLATE("Tool","Scaling selected primitives"));
}

void ScaleTool::mouseButtonMove(int buttonState, int x, int y)
{
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	//Should tools be responsible for this?
	if(buttonState&BS_Shift&&m_allowX&&m_allowY)
	{
		double ax = fabs(pos[0]-m_x);
		double ay = fabs(pos[1]-m_y);

		if(ax>ay) m_allowY = false;
		if(ay>ax) m_allowX = false;
	}

	if(!m_allowX) pos[0] = m_x;
	if(!m_allowY) pos[1] = m_y;

	double spX = m_pointX;
	double spY = m_pointY;
	double spZ = m_pointZ;

	double lengthX = fabs(spX-pos[0]);
	double lengthY = fabs(spY-pos[1]);

	for(auto&ea:m_positionCoords)
	{
		double x = ea.coords[0]-spX;
		double y = ea.coords[1]-spY;
		double z = ea.coords[2]-spZ;

		double xper = m_startLengthX<=0.00006?1:lengthX/m_startLengthX;
		double yper = m_startLengthY<=0.00006?1:lengthY/m_startLengthY;

		if(m_proportion==ST_ScaleFree)
		{
			x*=xper; y*=yper;
		}
		else
		{
			double max = std::max(xper,yper);
			x*=max;
			y*=max;
			if(m_proportion==ST_ScaleProportion3D)			
			z*=max;
		}

		movePosition(ea.pos,x+spX,y+spY,z+spZ);
	}

	if(!m_projList.empty())
	{
		//log_debug("setting scale\n");
		double startLen = distance(m_startLengthX,m_startLengthY,0.0,0.0);
		double len = distance(lengthX,lengthY,0.0,0.0);
		double diff = len/startLen;
		//log_debug("new scale = %f\n",diff*m_projScale);

		//NEW: Why not all???
		for(auto&ea:m_projList)
		parent->getModel()->setProjectionScale(ea.first,ea.second*diff);
	}

	parent->updateAllViews();
}
