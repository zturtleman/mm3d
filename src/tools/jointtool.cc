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

#include "pixmap/jointtool.xpm"

#include "model.h"
#include "modelstatus.h"
#include "msg.h"
#include "log.h"

struct JointTool : Tool
{
	JointTool():Tool(TT_Creator){}

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Create Bone Joint"); 
	}

	virtual const char **getPixmap(int){ return jointtool_xpm; }

	virtual const char *getKeymap(int){ return "F10"; }
	
	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

		ToolCoordT m_joint;
};

extern Tool *jointtool(){ return new JointTool; }

void JointTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	double pos[2];
	parent->getParentXYValue(x,y,pos[0],pos[1],true);

	const Matrix &mat = parent->getParentViewMatrix();

	int p = -1;
	double pDist = DBL_MAX;
	for(int i=model->getBoneJointCount();i-->0;)
	{
		double coords[4];
		model->getBoneJointCoords(i,coords);
		coords[3] = 1;
		mat.apply(coords);

		double dist = distance(pos[0],pos[1],coords[0],coords[1]);
		if(dist<pDist)
		{
			p = i; pDist = dist;
		}
	}

	// Find a unique name for the joint
	char name[64] = "Joint ";
	int nameN = sizeof("Joint ")-1;
	int num = 0;
	int iN = model->getBoneJointCount();
	for(int i=0;i<iN;i++)
	{
		const char *cmp = model->getBoneJointName(i);
		if(!memcmp(cmp,name,nameN))
		num = std::max(num,atoi(cmp+nameN));
	}
	sprintf(name+nameN,"%d",num+1);

	m_joint = addPosition(Model::PT_Joint,name,pos[0],pos[1],0,0,0,0,p);

	model->unselectAll();
	model->selectBoneJoint(m_joint);
	
	model_status(model,StatusNormal,STATUSTIME_SHORT,p>=0?
	TRANSLATE("Tool","Joint created"):
	TRANSLATE("Tool","Root joint created"));	

	parent->updateAllViews();
}
void JointTool::mouseButtonMove(int buttonState, int x, int y)
{
	if(m_joint.pos.type==Model::PT_Joint)
	{
		double pos[2];
		parent->getParentXYValue(x,y,pos[0],pos[1]);

		movePosition(m_joint.pos,pos[0],pos[1],0);

		parent->updateAllViews();
	}
}

 

