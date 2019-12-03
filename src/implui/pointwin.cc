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

struct PointWin : Win //UNUSED
{
	//2019: This window isn't available, but 
	//I feel like it may be worth preserving.

	void submit(int);

	PointWin(Model *model)
		:
	Win("Points"),model(model),
	point(main,"",id_item),
	nav(main),
	name(nav,"Rename",id_name),
	del(nav,"Delete",id_delete),
	joint(main,"Bone Joint",id_subitem)
	{
		//Just a guess at what it looks like. 

		point.expand();
		joint.style(bi::etched).expand();

		active_callback = &PointWin::submit;

		submit(id_init);
	}

	Model *model;

	row nav;
	button name,del;
	dropdown point,joint;
};
void PointWin::submit(int id)
{
	switch(id)
	{
	case id_init:
	{
		int iN = model->getPointCount();
		for(int i=0;i<iN;i++)
		point.add_item(i,model->getPointName(i));
	
		iN = model->getBoneJointCount();
		joint.add_item(0,::tr("<None>"));
		for(int i=0;i<iN;i++)
		joint.add_item(i+1,model->getBoneJointName(i));

		int_list l;
		model->getSelectedPoints(l);
		if(!l.empty()) point.select_id(l.front());
		//break;
	}
	case id_item:

		if(point.selection())
		{
			model->unselectAllPoints();
			model->selectPoint((int)point);			
			joint.select_id(model->getPointBoneJoint((int)point)+1);
			//https://github.com/zturtleman/mm3d/issues/90
			//DecalManager::getInstance()->modelUpdated(model); //???
			model->updateObservers();
		}
		else disable(); break;

	case id_subitem:

		//Looks erroneous.
		//if(index>=0&&index<m_pointJoint->count())
		model->setPointBoneJoint((int)point,joint.int_val()-1);
		break;

	case id_delete:

		model->deletePoint((int)point);
		break;

	case id_name:
	{
		std::string name = model->getPointName((int)point);
		if(id_ok==EditBox(&name,::tr("Rename point","window title"),::tr("Enter new point name:")))
		{
			model->setPointName((int)point,name.c_str());
			point.selection()->text() = name;
		}
		break;
	}
	case id_ok:

		log_debug("Point changes complete\n");
		model->operationComplete(::tr("Point changes","operation complete"));
		break;
	
	case id_cancel:

		log_debug("Point changes canceled\n");
		model->undoCurrent();
		break;
	}

	basic_submit(id);
}

