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

#include "texturecoord.h" //Widget

struct GroupWin : Win
{
	void submit(int);
	
	GroupWin(Model *model)
		:
	Win("Groups"),
	model(model),
	group(main,"",id_item),
	nav1(main),
	add(nav1,"New",id_new),
	name(nav1,"Rename",id_name),
	del(nav1,"Delete",id_delete),
	nav2(main),
		//TODO: What about add to selection?
	select(nav2,"Select Faces In Group",id_select), 
	deselect(nav2,"Unselect Faces In Group",-id_select),
	smooth(main),angle(main),
	nav3(main),
	scene(nav3,id_scene),	
		col1(nav3),
	material(nav3,"Material",id_subitem),
	faces(nav3,"Membership",bi::none), //"Faces"	
	set_faces(faces,"Assign Faces",id_assign), //"Assign As Group"
	add_faces(faces,"Add To Group",id_append), //"Add To Group"	
	f1_ok_cancel(main),
	texture(scene,false)
	{
		//texwin.cc's happens to be 100 x 100.
		scene.lock(100,100);

		smooth.space(3,2);

		nav2.expand_all().proportion();
		nav3.expand();
		col1.expand().space<top>(1);
		material.expand().place(bottom);

		active_callback = &GroupWin::submit;
		
		submit(id_init);
	}
	
	Model *model;

	dropdown group;
	row nav1;
	button add,name,del;
	row nav2;
	button select,deselect;
	bar smooth,angle; //spinbox?
	panel nav3;
	canvas scene;
	column col1;
	dropdown material;
	row faces;
	button set_faces,add_faces;
	f1_ok_cancel_panel f1_ok_cancel;

	Widget texture;

	void group_selected();
	void new_group_or_name(int);	
	bool smooth_differs(),angle_differs();
};
void GroupWin::submit(int id)
{
	enum{ id_smooth=1000,id_angle };

	switch(id)
	{
	case id_init:
	{	
		group.expand();

		int iN = model->getGroupCount();
		for(int i=0;i<iN;i++)
		group.add_item(i,model->getGroupName(i));

		iN = model->getTextureCount();
		material.add_item(-1,::tr("<None>"));
		for(int i=0;i<iN;i++)	
		material.add_item(i,model->getTextureName(i));
		texture.setModel(model);
		
		int_list l; model->getSelectedTriangles(l);
		for(int_list::iterator it=l.begin(),itt=l.end();it<itt;it++)	
		if(int g=model->getTriangleGroup(*it)+1)
		{
			group.select_id(g-1); break;
		}
		group_selected(); 
				
		smooth.set_range(0,255).id(id_smooth).expand();
		submit(id_smooth); //HACK
		angle.set_range(0,180).id(id_angle).expand();
		submit(id_angle); //HACK
		smooth.sspace<left>({angle}); //EXPERIMENTAL

		break;
	}	
	case id_scene:

		texture.draw(scene.x(),scene.y(),scene.width(),scene.height());
		break;

	case id_delete:
	case id_select: case -id_select: //Deselect?
	case id_assign: case id_append:
	case id_subitem:
	{
		int g = group; switch(id)
		{
		case id_delete:

			model->deleteGroup(g);	
			group.delete_item(g).select_id(0);
			group.select_id(0);
			group_selected();
			break;

		case id_select: 
		
			model->unselectAll(); //TODO: Disable option?
			model->selectGroup(g);			
			break;

		case -id_select: model->unselectGroup(g); break;		

		case id_assign: model->setSelectedAsGroup(g); break; 
		case id_append: model->addSelectedToGroup(g); break;

		case id_subitem:

			model->setGroupTextureId(g,material);
	
			texture.setTexture(material); break;			
		}
		//https://github.com/zturtleman/mm3d/issues/90
		//DecalManager::getInstance()->modelUpdated(model); //???
		model->updateObservers();
		break;
	}
	case id_smooth:

		//Obscuring this adds phantom positions to this slider.
		//smooth.name().format("%s%03d\t",::tr("Smoothness: "),(int)((val/255.0)*100));
		smooth.name().format("%s\t%03d",::tr("Smoothness: "),(int)smooth);
		break;

	case id_angle:

		angle.name().format("%s\t%03d",::tr("Max Angle: "),(int)angle);
		break;

	case id_new: case id_name:

		new_group_or_name(id);
		break;

	case id_item:
	case id_ok:

		//FIX ME: https://github.com/zturtleman/mm3d/issues/53
		if(!group.empty())
		if(smooth_differs()||angle_differs())		
		model->calculateNormals();
		if(id==id_item)		
		group_selected();
		if(id==id_ok)
		model->operationComplete(::tr("Group changes","operation complete"));		
		break;

	case id_cancel:

		model->undoCurrent();
		break;
	}

	basic_submit(id);
}
void GroupWin::group_selected()
{
	if(group.empty())
	{
		disable(); add.enable();
		f1_ok_cancel.nav.enable(); 
		material.select_id(0);
	}
	else
	{
		int g = group;
		smooth.set_int_val(model->getGroupSmooth(g)); 
		angle.set_int_val(model->getGroupAngle(g));	
		material.select_id(model->getGroupTextureId(g));
	}
	texture.setTexture(material);
}
void GroupWin::new_group_or_name(int id)
{
	utf8 title = "Rename group"; //NEW
	std::string name; int g; if(id==id_name)
	{
		title = "New group";
		g = group; name = model->getGroupName(g);		
	}
	if(id_ok==EditBox(&name,::tr(title,"window title"),
	::tr("Enter new group name:"),1,Model::MAX_GROUP_NAME_LEN))
	{
		if(id==id_name)
		{	
			model->setGroupName(g,name.c_str());
			group.selection()->text() = name;		
		}
		else if(id==id_new)
		{
			if(group.empty()) enable();

			g = model->addGroup(name.c_str());
			group.add_item(g,name);
			group.select_id(g);
			group_selected();
		}
		else assert(0);
	}
}
bool GroupWin::smooth_differs()
{
	int g = group;

	unsigned char yuck = 0xFF&smooth; //FIX ME

	//REMOVE ME: setGroupSmooth ought to return false.
	if(yuck==model->getGroupSmooth(g)) 
	return false;

	//FIX ME: https://github.com/zturtleman/mm3d/issues/53
	return model->setGroupSmooth(g,yuck);
	//Converting 255 to 100???
	//smooth.name().format("%s%03d",::tr("Smoothness: "),(int)((val/255.0)*100));
	//model->calculateNormals();
	//DecalManager::getInstance()->modelUpdated(model); //???
}
bool GroupWin::angle_differs()
{
	int g = group;

	unsigned char yuck = 0xFF&angle; //FIX ME

	//REMOVE ME: setGroupAngle ought to return false.
	if(yuck==model->getGroupAngle(g))
	return false;

	//FIX ME: https://github.com/zturtleman/mm3d/issues/53
	return model->setGroupAngle(g,yuck);
	//angle.name().format("%s%03d",::tr("Max Angle: "),val);
	//model->calculateNormals();
	//DecalManager::getInstance()->modelUpdated(model); //???
}

extern void groupwin(Model *m){ GroupWin(m).return_on_close(); }
