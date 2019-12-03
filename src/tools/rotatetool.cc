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
#include "pixmap/rotatetool.xpm"
#include "glmath.h"

#include "rotatepoint.h"
#include "log.h"
#include "modelstatus.h"

struct RotateTool : Tool
{
	RotateTool():Tool(TT_Other){}

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Rotate");
	}

	virtual const char *getKeymap(int){ return "R"; }

	virtual const char **getPixmap(int){ return rotatetool_xpm; }

	void activated2(); 

	virtual void activated(int)
	{
		activated2();
		parent->addDouble(false,&m_rotatePoint.x,TRANSLATE_NOOP("Param","X"));
		parent->groupParam();
		parent->addDouble(false,&m_rotatePoint.y,TRANSLATE_NOOP("Param","Y"));
		parent->groupParam();
		parent->addDouble(false,&m_rotatePoint.z,TRANSLATE_NOOP("Param","Z"));
	}
	virtual void updateParam(void*)
	{
		//TODO: Update parent?
		//DecalManager::getInstance()->modelUpdated(parent->getModel());
		parent->updateAllViews();
	}
	virtual void deactivated()
	{
		parent->updateAllViews();
	}
	
	int mouse2(int buttonState, int x, int y);

	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

	//REMOVE ME
	virtual void mouseButtonUp(int buttonState, int x, int y)
	{	
		if(buttonState&BS_Left)	
		model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("Tool","Rotate complete"));
	
		parent->updateAllViews();
	}

	virtual void draw(bool){ m_rotatePoint.draw(); }
		
		double m_mouse2_angle;

		struct RotatePoint
		{	
			void draw()
			{
				//HACK: The crosshairs draw on top of each
				//other. Note, assuming the previous value.
				glEnable(GL_DEPTH_TEST);
				glDepthRange(0,0);
				glDepthFunc(GL_LESS);
				{
					//glColor3f(0,1,0);
					glEnable(GL_COLOR_LOGIC_OP);
					glColor3ub(0x80,0x80,0x80);
					glLogicOp(GL_XOR);
					glTranslated(x,y,z);
					rotatepoint_draw_manip(scale); //0.25f
					glTranslated(-x,-y,-z);
					glDisable(GL_COLOR_LOGIC_OP);
				}
				glDepthRange(0,1);
				glDepthFunc(GL_LEQUAL);
			}

			double x,y,z,w; float scale;

			operator double*(){ return &x; }	

		}m_rotatePoint;
	
	void getRotateCoords();
};

extern Tool *rotatetool(){ return new RotateTool; }

void RotateTool::activated2()
{
	Model *model = parent->getModel();

	if(model->getAnimationMode()!=Model::ANIMMODE_SKELETAL)
	{
		//FIX ME
		//Min/max/scale the pivot accordingly.
		//https://github.com/zturtleman/mm3d/issues/89
		pos_list l;
		model->getSelectedPositions(l);
		double min[3] = {+DBL_MAX,+DBL_MAX,+DBL_MAX}; 
		double max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX}; 
		for(auto&ea:l) 
		{
			double coords[3];
			model->getPositionCoords(ea,coords);
			for(int i=0;i<3;i++)
			{
				min[i] = std::min(min[i],coords[i]);
				max[i] = std::max(max[i],coords[i]);
			}
		}
		for(int i=0;i<3;i++)
		m_rotatePoint[i] = (min[i]+max[i])/2;
		m_rotatePoint.scale = distance(min,max)/6;
	}
	else //??? //REMOVE ME //???
	{
		m_rotatePoint.scale = 0.25f; //Old default???

		m_rotatePoint.x = m_rotatePoint.y = m_rotatePoint.z = 0;
	}
	m_rotatePoint.w = 1;		

	//FIX ME
	//Should show decal in all views.
//	DecalManager::getInstance()->addDecalToModel(&m_rotatePoint,model);
	parent->updateAllViews(); //NEW
		
	model_status(model,StatusNormal,STATUSTIME_NONE,
	TRANSLATE("Tool","Tip: Hold shift to rotate in 15 degree increments"));
}

int RotateTool::mouse2(int buttonState, int x, int y)
{
	Model *model = parent->getModel();
	
	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1]);

	int ret = 0; if(buttonState&BS_Left)
	{
		ret = 1;

		double coords[4];
		memcpy(coords,m_rotatePoint,sizeof(coords));
		coords[3] = 1; //???
		parent->getParentViewMatrix().apply(coords);

		double xDiff = pos[0]-coords[0];
		double yDiff = pos[1]-coords[1];
		double angle = rotatepoint_diff_to_angle(xDiff,yDiff);
		if(buttonState&BS_Shift) 
		angle = rotatepoint_adjust_to_nearest(angle,15);

		m_mouse2_angle = angle;
	}
	else if(buttonState&BS_Right
	&&model->getAnimationMode()!=Model::ANIMMODE_SKELETAL)
	{
		ret = 2;

		m_rotatePoint.x = pos[0];
		m_rotatePoint.y = pos[1];
		m_rotatePoint.z = 0;
		m_rotatePoint.w = 1; //???
		parent->getParentViewInverseMatrix().apply(m_rotatePoint); 
		parent->updateParams();
	}

	parent->updateAllViews(); return ret;
}
void RotateTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	if(model->getAnimationMode()==Model::ANIMMODE_SKELETAL)
	{
		int iN = model->getBoneJointCount();
		for(int i=0;i<iN;i++)		
		if(model->isBoneJointSelected(i))
		{
			model->getBoneJointCoords(i,m_rotatePoint);
			parent->updateParams();
			break;
		}
	}
	 
	if(int mode=mouse2(buttonState,x,y))
	{
		const char *msg;
		if(mode==1) msg = TRANSLATE("Tool","Rotating selected primitives");	
		if(mode!=1) msg = TRANSLATE("Tool","Setting rotation point");
		model_status(model,StatusNormal,STATUSTIME_SHORT,msg);
	}
}
void RotateTool::mouseButtonMove(int buttonState, int x, int y)
{
	double angle = m_mouse2_angle;
	if(1==mouse2(buttonState,x,y))
	{
		double vec[4] = { 0,0,1,1 };
		parent->getParentViewInverseMatrix().apply3(vec);
		Matrix m;
		m.setRotationOnAxis(vec,m_mouse2_angle-angle);

		parent->getModel()->rotateSelected(m,m_rotatePoint);
	}
}

