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

#include "model.h"

#include "log.h"
#include "msg.h"
#include "modelstatus.h"

//ISSUES
//https://github.com/zturtleman/mm3d/issues/49

struct JointWin : Win
{
	void submit(control*);

	JointWin(Model *model)
		:
	Win("Joints"),model(model),
	joint(main,"",id_item),
	nav(main),
	name(nav,"Rename",id_name),
	del(nav,"Delete",id_delete),
	sel(main),f1_ok_cancel(main)
	{
		joint.expand();
		nav.expand_all().proportion();
		sel.nav.expand_all();
		
		active_callback = &JointWin::submit;

		submit(main);
	}

	Model *model;

	struct selection_group
	{
		selection_group(node *main)
			:
		nav(main,"Selection"),
		infl(nav,"Select Joint Vertices"),
		uninfl(nav,"Select Unassigned Vertices"),
		assign(nav,"Assign Selected to Joint"),
		append(nav,"Add Selected to Joint")
		{}

		panel nav;
		button infl,uninfl,assign,append;
	};

	dropdown joint;
	row nav;
	button name,del;
	selection_group sel;
	f1_ok_cancel_panel f1_ok_cancel;
};

void JointWin::submit(control *c)
{
	//ISSUES
	//https://github.com/zturtleman/mm3d/issues/49

	int id = c->id(); switch(id)
	{
	case id_name: //"Rename"

		if(id_ok==EditBox(&joint.selection()->text(),
		::tr("Rename joint","window title"),::tr("Enter new joint name:")))
		{
			model->setBoneJointName((int)joint,joint.selection()->c_str());
		}
		break;

	case id_delete: //"Delete"

		model->deleteBoneJoint((int)joint); 

		joint.clear(); //break; //NEW

	case id_init:
	
		for(int i=0;i<model->getBoneJointCount();i++)
		{
			joint.add_item(i,model->getBoneJointName(i));
		}
		if(!joint.empty())
		{
			int_list l; //REMOVE ME
			model->getSelectedBoneJoints(l);
			if(!l.empty())
			joint.select_id(l.front());

			goto id_item;
		}
		else //NEW: Maybe last joint was deleted?
		{
			disable(); f1_ok_cancel.nav.enable();
		}
		break;

	case id_item: id_item: 
		
		model->unselectAllBoneJoints();
		model->selectBoneJoint((int)joint);
		break;
	
	default: //"Selection"

		switch(int bt=(button*)c-&sel.infl)
		{
		case 0: //"Select Joint Vertices"
		case 1: //"Select Unassigned Vertices"
		{
			model->unselectAllVertices();

			infl_list::const_iterator it;
			for(int j=joint,i=0,iN=model->getVertexCount();i<iN;i++)
			{
				const infl_list &l = model->getVertexInfluences(i);				
				if(bt==0) for(it=l.begin();it!=l.end();it++)
				{
					if(it->m_boneId==j)
					{
						select: model->selectVertex(i); break;
					}
				}
				else if(l.empty()) goto select;
			}
			for(int j=joint,i=0,iN=model->getPointCount();i<iN;i++)
			{
				const infl_list &l = model->getPointInfluences(i);
				if(bt==0) for(it=l.begin();it!=l.end();it++)
				{
					if(it->m_boneId==j)
					{
						select2: model->selectPoint(i); break;
					}
				}
				else if(l.empty()) goto select2;
			}
			//https://github.com/zturtleman/mm3d/issues/90
			//DecalManager::getInstance()->modelUpdated(model); //???
			model->updateObservers();
			break;
		}
		case 2: //"Assign Selected to Joint"
		case 3: //"Add Selected to Joint"
		{
			pos_list pl; model->getSelectedPositions(pl);
			int j = joint;
			log_debug("%sing %d objects to joint %d\n",bt==2?"ass":"add",pl.size(),j);
			for(pos_list::iterator it=pl.begin();it!=pl.end();it++)
			{
				if(bt==2) model->setPositionBoneJoint(*it,j);
				if(bt==3) model->addPositionInfluence(*it,j,Model::IT_Custom,1);
			}
			break;
		}}
		break;

	case id_ok:

		log_debug("Joint changes complete\n");
		model->operationComplete(::tr("Joint changes","operation complete"));
		break;

	case id_cancel:

		log_debug("Joint changes canceled\n");
		model->undoCurrent();
		break;
	}

	basic_submit(id);
}

extern void jointwin(Model *m){ JointWin(m).return_on_close(); }
