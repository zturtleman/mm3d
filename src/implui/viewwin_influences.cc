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

#include "viewwin.h"

#include "viewpanel.h"
#include "model.h"
#include "modelstatus.h"
//#include "jointwin.h"
//#include "autoassignjointwin.h"
#include "log.h"

struct AutoAssignJointWin : Win
{
	AutoAssignJointWin(int *lv, double *lv2)
		:
	Win("Auto-Assign Bone Joints"),
	selected(main,"Only assign to selected joints",lv),
		sep1(main),
	nav(main),
	single(nav,"Single"),
	sensitivity(nav,"",bar::horizontal,lv2),
	multi(nav,"Multiple"),
		sep2(main),
	f1_ok_cancel(main)
	{
		if(!*lv) selected.disable();

		nav.expand();
		single.space<top>(1); 
		sensitivity.set_range(0,100).expand(); 
		multi.space<top>(1).ralign();
	}

	boolean selected;
	titlebar sep1;
	row nav;	
	titlebar single; 
	bar sensitivity;
	titlebar multi;
	titlebar sep2;
	f1_ok_cancel_panel f1_ok_cancel;
};

static void viewwin_influences_jointAssignSelectedToJoint(Model *model)
{
	int_list j;

	model->getSelectedBoneJoints(j);
	if(j.empty())
	return model_status(model,StatusError,STATUSTIME_LONG,
	::tr("You must have at least one bone joint selected."));

	int_list::iterator it,jit;

	int_list vertList,pointList;
	model->getSelectedVertices(vertList);
	model->getSelectedPoints(pointList);

	log_debug("assigning %d vertices and %d points to joints\n",vertList.size(),pointList.size());
	//QString str = ::tr("Assigning %1 vertices and %2 points to joints").arg(vertList.size()).arg(pointList.size());
	//model_status(model,StatusNormal,STATUSTIME_SHORT,"%s",(const char *)str);
	utf8 str = ::tr("Assigning %d vertices and %d points to joints");
	model_status(model,StatusNormal,STATUSTIME_SHORT,str,vertList.size(),pointList.size());

	for(it = vertList.begin(); it!=vertList.end(); it++)	
	for(jit = j.begin(); jit!=j.end(); jit++)
	{
		double w = model->calculateVertexInfluenceWeight(*it,*jit);
		model->addVertexInfluence(*it,*jit,Model::IT_Auto,w);
	}

	for(it = pointList.begin(); it!=pointList.end(); it++)	
	for(jit = j.begin(); jit!=j.end(); jit++)
	{
		double w = model->calculatePointInfluenceWeight(*it,*jit);
		model->addPointInfluence(*it,*jit,Model::IT_Auto,w);
	}

	model->operationComplete(::tr("Assign Selected to Joint"));
}

static void viewwin_influences_jointAutoAssignSelected(Model *model)
{
	pos_list l;	
	model->getSelectedPositions(l); if(l.empty())
	{
		return model_status(model,StatusError,STATUSTIME_LONG,
		::tr("You must have at least one vertex or point selected."));
	}	 
	int selected = 0!=model->getSelectedBoneJointCount();		
	double sensitivity = 50; 
	if(id_ok==AutoAssignJointWin(&selected,&sensitivity).return_on_close())
	{
		sensitivity/=100;			

		log_debug("auto-assigning %p vertices and points to joints\n",l.size());

		for(pos_list::iterator it=l.begin();it!=l.end();it++)
		{
			model->autoSetPositionInfluences(*it,sensitivity,1&selected);
		}

		model->operationComplete(::tr("Auto-Assign Selected to Bone Joints"));
	}
}

static void viewwin_influences_jointRemoveInfluencesFromSelected(Model *model)
{
	pos_list posList;
	pos_list::iterator it;

	model->getSelectedPositions(posList);
	for(it = posList.begin(); it!=posList.end(); it++)
	{
		model->removeAllPositionInfluences(*it);
	}

	model->operationComplete(::tr("Remove All Influences from Selected"));
}

static void viewwin_influences_jointRemoveInfluenceJoint(Model *model)
{
	int_list jointList;
	int_list::iterator it;

	unsigned vcount = model->getVertexCount();
	unsigned pcount = model->getPointCount();

	model->getSelectedBoneJoints(jointList);

	for(it = jointList.begin(); it!=jointList.end(); it++)
	{
		for(unsigned v = 0; v<vcount; v++)
		{
			model->removeVertexInfluence(v,*it);
		}
		for(unsigned p = 0; p<pcount; p++)
		{
			model->removePointInfluence(p,*it);
		}
	}

	model->operationComplete(::tr("Remove Joint from Influencing"));
}

static void viewwin_influences_jointMakeSingleInfluence(Model *model)
{
	//FIX ME
	//Should be selection based.

	unsigned vcount = model->getVertexCount();
	unsigned pcount = model->getPointCount();
	int		bcount = model->getBoneJointCount();

	for(unsigned v = 0; v<vcount; v++)
	{
		int joint = model->getPrimaryVertexInfluence(v);
		
		if(joint>=0) for(int b = 0; b<bcount; b++)
		{
			if(b!=joint) model->removeVertexInfluence(v,b);
		}
	}
	for(unsigned p = 0; p<pcount; p++)
	{
		int joint = model->getPrimaryPointInfluence(p);
		
		if(joint>=0) for(int b = 0; b<bcount; b++)
		{
			if(b!=joint) model->removePointInfluence(p,b);
		}
	}

	model->operationComplete(::tr("Convert To Single Influence"));
}

static void viewwin_influences_jointSelectUnassignedVertices(Model *model)
{
	model->unselectAllVertices();

	model->beginSelectionDifference();

	unsigned vcount = model->getVertexCount();
	for(unsigned v = 0; v<vcount; v++)
	{
		//infl_list l;
		//model->getVertexInfluences(v,l);
		//if(l.empty())
		if(model->getVertexInfluences(v).empty())
		{
			model->selectVertex(v);
		}
	}

	model->endSelectionDifference();

	model->operationComplete(::tr("Select Unassigned Vertices"));
}

static void viewwin_influences_jointSelectUnassignedPoints(Model *model)
{
	model->unselectAllVertices();

	model->beginSelectionDifference();

	unsigned pcount = model->getPointCount();
	for(unsigned p = 0; p<pcount; p++)
	{
		//infl_list l;
		//model->getPointInfluences(p,l);
		//if(l.empty())
		if(model->getPointInfluences(p).empty())
		{
			model->selectPoint(p);
		}
	}

	model->endSelectionDifference();

	model->operationComplete(::tr("Select Unassigned Points"));
}

static void viewwin_influences_jointSelectInfluenceJoints(Model *model)
{
	model->beginSelectionDifference();

	//infl_list ilist;
	infl_list::const_iterator iit;

	pos_list posList;
	pos_list::iterator it;

	model->getSelectedPositions(posList);
	for(it=posList.begin();it!=posList.end();it++) switch(it->type)
	{
	case Model::PT_Vertex: case Model::PT_Point: //TODO: Filter out.
	
		//model->getPositionInfluences(*it,ilist);
		const infl_list &ilist = model->getPositionInfluences(*it);

		for(iit=ilist.begin();iit!=ilist.end();iit++)			
		{
			model->selectBoneJoint((*iit).m_boneId);
		}
	}

	model->endSelectionDifference();

	model->operationComplete(::tr("Select Joint Influences"));
}

static void viewwin_influences_jointSelectInfluencedVertices(Model *model)
{
	model->beginSelectionDifference();

	//infl_list ilist;
	infl_list::const_iterator iit;

	int_list jointList;
	int_list::iterator it;

	unsigned vcount = model->getVertexCount();

	model->getSelectedBoneJoints(jointList);
	for(it = jointList.begin(); it!=jointList.end(); it++)	
	for(unsigned v = 0; v<vcount; v++)
	{
		//model->getVertexInfluences(v,ilist);
		const infl_list &ilist = model->getVertexInfluences(v);

		for(iit = ilist.begin(); iit!=ilist.end(); iit++)
		{
			if(*it==iit->m_boneId) model->selectVertex(v);
		}
	}

	model->endSelectionDifference();

	model->operationComplete(::tr("Select Influences Vertices"));
}

static void viewwin_influences_jointSelectInfluencedPoints(Model *model)
{
	model->beginSelectionDifference();

	//infl_list ilist;
	infl_list::const_iterator iit;

	int_list jointList;
	int_list::iterator it;

	unsigned pcount = model->getBoneJointCount();

	model->getSelectedBoneJoints(jointList);
	for(it = jointList.begin(); it!=jointList.end(); it++)	
	for(unsigned p = 0; p<pcount; p++)
	{
		//model->getPointInfluences(p,ilist);
		const infl_list &ilist = model->getPointInfluences(p);

		for(iit = ilist.begin(); iit!=ilist.end(); iit++)
		{
			if(*it==iit->m_boneId) model->selectPoint(p);
		}
	}

	model->endSelectionDifference();
	model->operationComplete(::tr("Select Influenced Points"));
}

extern void viewwin_influences(Model *model, int id)
{
	switch(id)
	{
	case id_joint_attach_verts: 
		
		viewwin_influences_jointAssignSelectedToJoint(model);
		break;

	case id_joint_weight_verts: 
		
		viewwin_influences_jointAutoAssignSelected(model);
		break;

	case id_joint_remove_bones:

		viewwin_influences_jointRemoveInfluencesFromSelected(model);
		break;

	case id_joint_remove_selection: 
		
		viewwin_influences_jointRemoveInfluenceJoint(model);
		break;

	case id_joint_simplify: 
		
		viewwin_influences_jointMakeSingleInfluence(model);
		break;

	case id_joint_select_bones_of:
		
		viewwin_influences_jointSelectInfluenceJoints(model);
		break;

	case id_joint_select_verts_of: 
		
		viewwin_influences_jointSelectInfluencedVertices(model);
		break;

	case id_joint_select_points_of: 
		
		viewwin_influences_jointSelectInfluencedPoints(model);
		break;

	case id_joint_unnassigned_verts:
		
		viewwin_influences_jointSelectUnassignedVertices(model);
		break;

	case id_joint_unnassigned_points:
		
		viewwin_influences_jointSelectUnassignedPoints(model);
		break;
	}
}

