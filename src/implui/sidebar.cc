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

#include "sidebar.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"

#include "viewwin.h"
#include "modelstatus.h"

//REMOVE US
//This is replacing old functionality.
//I think at most it's an optimization
//as is. I think Qt was the source of 
//the problem. But possibly Model too?
static int sidebar_updating = 0;
//REMOVE US
struct sidebar_updater //RAII
{
	sidebar_updater(){ sidebar_updating++; }

	~sidebar_updater(){ sidebar_updating--; }
};

SideBar::SideBar(class MainWin &model)
	:
Win(model.glut_window_id,subpos_right),
//NOTE: Compilers squeal if "this" is passed
//in the initializer's list.
anim_panel(model.sidebar,model),
prop_panel(model.sidebar,model),
bool_panel(model.sidebar,model)
{
	clip(true,false,true);

	dropdown *ll[] = //HACK
	{
	&anim_panel.animation,
	&prop_panel.group.group.menu,
	&prop_panel.group.material.menu,
	&prop_panel.group.projection.menu,
	&prop_panel.infl.i0.joint,
	&prop_panel.infl.i1.joint,
	&prop_panel.infl.i2.joint,
	&prop_panel.infl.i3.joint,
	};
	//for(size_t i=std::size(ll);i-->0;)
	for(size_t i=sizeof(ll)/sizeof(*ll);i-->0;)
	{
		ll[i]->lock(ll[i]->span(),false);
	}
	anim_panel.nav.set(config.get("ui_anim_sidebar",true));
	bool_panel.nav.set(config.get("ui_bool_sidebar",true));
	prop_panel.nav.set(config.get("ui_prop_sidebar",true));

	active_callback = &SideBar::submit;

	basic_submit(id_init);
}
void SideBar::submit(control *c)
{
	void *v = c; 
	if(v>=&anim_panel&&v<&anim_panel+1)
	{
		if(c!=anim_panel.nav) anim_panel.submit(c);
		else config.set("ui_anim_sidebar",(bool)*c);
	}
	if(v>=&bool_panel&&v<&bool_panel+1)
	{
		int i = *c;
		if(c!=bool_panel.nav) bool_panel.submit(c);
		else config.set("ui_bool_sidebar",(bool)*c);
	}
	if(v>=&prop_panel&&v<&prop_panel+1)
	{
		if(c!=prop_panel.nav) prop_panel.submit(c);
		else config.set("ui_prop_sidebar",(bool)*c);
	}
}

void SideBar::AnimPanel::submit(control *c)
{
	switch(int id=c->id())
	{		
	case id_init:

		//refresh_list(); //Won't do.
		break;
	
	case id_animate_play: 
	case id_animate_pause:
	case id_animate_settings:

		model.perform_menu_action(id);
		break;		

	case id_item:
	case id_subitem:

		model.open_animation_system();
		break;
	}
}
void SideBar::AnimPanel::refresh_list()
{
	//See AnimWin::Impl::Impl.
	//It's easier to let the animation window manage this since
	//it's opaque.
	dropdown *r = animation.reference();
	if(!r) r = &animation;

	sep_animation = model->getAnimCount(Model::ANIMMODE_SKELETAL);
	new_animation = model->getAnimCount(Model::ANIMMODE_FRAME);
	new_animation+=sep_animation;

	r->clear();
	r->add_item(-1,"<None>"); //2019
	for(int i=0;i<sep_animation;i++)
	r->add_item(i,model->getAnimName(Model::ANIMMODE_SKELETAL,i));
	for(int i=sep_animation;i<new_animation;i++)
	r->add_item(i,model->getAnimName(Model::ANIMMODE_FRAME,i-sep_animation));
	r->add_item(new_animation,::tr("<New Animation>"));

	auto m = model->getAnimationMode();
	int a = model->getCurrentAnimation();
	if(m==Model::ANIMMODE_FRAME) a+=sep_animation;
	
	if(r!=&animation)
	{
		animation.clear();
		animation.reference(*r);
	}
	animation.select_id(m?a:-1);

	//This is for AnimSetWin id_ok closure.
	//It would probably be more kosher for
	//AnimPanel to monitor Model::Observer.
	model.sync_animation_system();
}

void SideBar::BoolPanel::submit(control *c)
{
	if(c==op)
	{
		utf8 str = nullptr; switch(op)
		{
		case op_union:
		//B replaces "Selected" to shorten the button.
		str = ::tr("Fuse With B","boolean operation");
		break;
		case op_union2:
		str = ::tr("Union With B","boolean operation");
		break;
		case op_subtract:
		str = ::tr("Subtract B","boolean operation");
		break;
		case op_intersect:
		str = ::tr("Intersect With B","boolean operation");
		break;
		}
		button_b.set_name(str);
	}
	else if(c==button_a)
	{
		int_list tris;
		model->getSelectedTriangles(tris);
		if(!tris.empty())
		{
			button_b.enable();
			//status.text().format("Object: %d Faces",(int)tris.size());
			//status.set_text(status);
			model->clearMarkedTriangles();			
			for(int_list::iterator it=tris.begin();it!=tris.end();it++)
			{
				model->setTriangleMarked(*it,true);
			}
		}
		else init();
	}
	else if(c==button_b)
	{	
		int i,iN = model->getTriangleCount();

		int_list al,bl; for(i=0;i<iN;i++)
		{
			if(model->isTriangleSelected(i))
			{
				bl.push_back(i);
			}
			else if(model->isTriangleMarked(i))
			{
				al.push_back(i);
			}
		}
		utf8 str; if(!al.empty()&&!bl.empty())
		{			
			Model::BooleanOpE e = Model::BO_Union; 
			switch(op)
			{
			case op_union2: e = Model::BO_UnionRemove;
			case op_union:
			str = ::tr("Union","boolean operation");
			break;
			case op_subtract: e = Model::BO_Subtraction; 
			str = ::tr("Subtraction","boolean operation");
			break;
			case op_intersect: e = Model::BO_Intersection; 
			str = ::tr("Intersection","boolean operation");
			break;
			}
			model->booleanOperation(e,al,bl);
			model->operationComplete(str);
			init();
			//model.views.modelUpdatedEvent(); //???
		}
		else 
		{
			if(!bl.empty()) 
			str = ::tr("Object A triangles are still selected");
			else 
			str = ::tr("You must have at least once face selected");

			model_status(model,StatusError,STATUSTIME_LONG,str);
		}
	}
}
void SideBar::BoolPanel::init()
{
	//status.set_name(::tr("Select faces to set",
	//"Select faces to set as 'A' Object in boolean operation"));
	button_b.disable();
}


void SideBar::PropPanel::submit(control *c)
{
	sidebar_updater raii; //REMOVE ME

	int id = c->id(); switch(id)
	{
	case id_init:

		pos.dimensions.disable();

		proj.menu.add_item(0,"Cylinder");
		proj.menu.add_item(1,"Sphere");
		proj.menu.add_item(2,"Plane");
			
		return;

	case id_name:

		name.change(); break;

	case 'X': case 'Y': case 'Z':

		c>=&rot.nav?rot.change():pos.change();
		break;

	case -id_projection_settings:

		if(c>proj.props_base::nav) return proj.change();
		//break;

	case -id_group_settings: 
	case -id_material_settings:
	
		group.submit(-id); break;

	case id_group_settings: 
	case id_material_settings: 
	case id_projection_settings:

		return model.perform_menu_action(id);

	case '0': case '1': case '2': case '3':
	
		infl.submit(c); break;
	}		

	//model.views.modelUpdatedEvent(); //???
}

	///// PROPERTIES //// PROPERTIES //// PROPERTIES ////

void SideBar::PropPanel::modelChanged()
{
	modelChanged(Model::ChangeAll);
}
void SideBar::PropPanel::modelChanged(int changeBits)
{	
	if(!changeBits){ assert(changeBits); return; }

	if(sidebar_updating) return; //REMOVE ME

	if(nav.hidden()) return;

	//log_debug("modelChanged()\n"); //???
			
	bool show;
	if(show=!model->getAnimationMode()
	&&model->getSelectedBoneJointCount()
	 +model->getSelectedPointCount()==1)
	name.change(changeBits);
	name.nav.set_hidden(!show);

	// Position should always be visible
	pos.change(changeBits);
	 	
	// Only allow points in None and Frame
	// Only allow joints in None and Skel
	int iM = model->getSelectedPointCount();
	int iN = model->getSelectedBoneJointCount();
	if(show= 
	(iM==1&&iN==0&&model->getAnimationMode()!=Model::ANIMMODE_SKELETAL)
	||(iN==1&&iM==0&&model->getAnimationMode()!=Model::ANIMMODE_FRAME))
	rot.change(changeBits);
	rot.nav.set_hidden(!show);

	show = false;
	iN = model->getTriangleCount();
	for(int i=0;i<iN;i++)
	if(model->isTriangleSelected(i)) //FIX ME
	{
		show = true;
		group.change(changeBits);
		break; 
	}	
	group.nav.set_hidden(!show);

	show = false;
	iN = model->getProjectionCount();
	for(int i=0;i<iN;i++)
	if(model->isProjectionSelected(i))
	{
		show = true;
		proj.change(changeBits);
		break; 
	}
	proj.props_base::nav.set_hidden(!show);

	show = false;
	// Only show influences if there are influences to show
	if(model->getBoneJointCount()&&!model->getAnimationMode())
	{
		iN = model->getPointCount();
		if(!show) for(int i=0;i<iN;i++)
		{
			if(model->isPointSelected(i))
			{
				show = true; break;
			}
		}
		iN = model->getVertexCount();
		if(!show) for(int i=0;i<iN;i++)
		{
			if(model->isVertexSelected(i))
			{
				show = true; break;
			}
		}

		if(show) infl.change(changeBits);
	}
	infl.nav.set_hidden(!show);
}

void SideBar::PropPanel::name_props::change(int changeBits)
{
	utf8 str = nullptr;
	int iN = model->getBoneJointCount();
	if(!str) for(int i=0;i<iN;i++)
	if(model->isBoneJointSelected(i))
	{
		if(!changeBits)		
		model->setBoneJointName(i,str=name);
		else str = model->getBoneJointName(i);
		break;
	}

	iN = model->getPointCount();
	if(!str) for(int i=0;i<iN;i++)
	if(model->isPointSelected(i))
	{
		if(!changeBits)			
		model->setPointName(i,str=name);			
		else str = model->getPointName(i);
		break;
	}

	if(changeBits&&str&&str!=name.text())
	{
		name.set_text(str);
	}

	if(!changeBits)
	model->operationComplete(::tr("Rename","operation complete"));
}

void SideBar::PropPanel::pos_props::change(int changeBits)
{	
	if(changeBits) // Update coordinates in text fields	
	{
		//FIX ME
		//2019: Assuming something like this should be done.
		//But I'm not sure what is the correct flags to use.
		//if(~changeBits&Model::SelectionVertices)
		//return;

		pos_list l;
		model->getSelectedPositions(l);

		//WORRIED THIS IS IMPRECISE
		double init = l.empty()?0.0:DBL_MAX;
		double cmin[3] = {+init,+init,+init};
		double cmax[3] = {-init,-init,-init};
	
		pos_list::iterator itt,it;
		for(it=l.begin(),itt=l.end();it<itt;it++)
		{
			double coords[3];
			model->getPositionCoords(*it,coords);
			for(int i=0;i<3;i++)
			cmin[i] = std::min(cmin[i],coords[i]);
			for(int i=0;i<3;i++)
			cmax[i] = std::max(cmax[i],coords[i]);
		}
		for(int i=0;i<3;i++)
		{
			centerpoint[i] = (cmin[i]+cmax[i])/2;
		}

		x.set_float_val(centerpoint[0]);
		y.set_float_val(centerpoint[1]);
		z.set_float_val(centerpoint[2]);
	
		//TODO: Add tooltip... make editable?
		//dimensions.text().format("%g,%g,%g",cmax[0]-cmin[0],cmax[1]-cmin[1],cmax[2]-cmin[2]);
		dimensions.text().clear();
		for(int i=0;;i++)
		{
			enum{ width=5 };
			char buf[32]; 
			int len = snprintf(buf,sizeof(buf),"%.6f",fabs(cmax[i]-cmin[i])); //-0
			if(char*dp=strrchr(buf,'.'))
			{
				while(len>width&&buf+len>dp)
				{
					dp--; len--;
				}
				while(buf+len>dp&&buf[len]=='0')
				{
					dp--; len--;
				}
				if(dp==buf+len) len--;
			}
			//while(len<width) buf[len++] = ' ';
			dimensions.text().append(buf,len);
			if(i==2) break;
			dimensions.text().push_back(',');
			dimensions.text().push_back(' ');
		}
		dimensions.set_text(dimensions);
	}
	else // Change model based on text field input
	{
		//WORRIED THIS IS IMPRECISE
		double trans[3],coords[3] = {x,y,z};		
		for(int i=0;i<3;i++)
		{
			trans[i] = coords[i]-centerpoint[i];
			centerpoint[i] = coords[i];
		}

		//FIX ME: Translate via vector.
		Matrix m;
		m.setTranslation(trans[0],trans[1],trans[2]);
		model->translateSelected(m);

		model->operationComplete(::tr("Set Position","operation complete"));
	}
}

void SideBar::PropPanel::rot_props::change(int changeBits)
{
	bool found = false; double rad[3] = {};

	Model::AnimationModeE mode = model->getAnimationMode();

	if(changeBits) // Update coordinates in text fields
	{	
		if(!found) 
		{
			int iN = model->getPointCount();
			for(int i=0;i<iN;i++)
			if(model->isPointSelected(i))
			{
				model->getPointRotation(i,rad);

				found = true; break;
			}
		}		
		if(!found) 
		if(!mode||mode==Model::ANIMMODE_SKELETAL)
		{
			int iN = model->getBoneJointCount();
			for(int i=0;i<iN;i++)
			if(model->isBoneJointSelected(i))
			if(mode)
			{
				int anim = model->getCurrentAnimation();
				int frame = model->getCurrentAnimationFrame();
				if(model->getSkelAnimKeyframe(anim,frame,i,true,rad[0],rad[1],rad[2]))
				{
					found = true; break;
				}
			}
			else
			{
				Matrix rm;
				model->getBoneJointRelativeMatrix(i,rm);
				rm.getRotation(rad[0],rad[1],rad[2]);

				found = true; break;
			}
		}
		for(int i=0;i<3;i++) rad[i]/=PIOVER180;

		x.set_float_val(rad[0]);
		y.set_float_val(rad[1]);
		z.set_float_val(rad[2]);

		nav.enable(found);
	}
	else // Change model based on text field input
	{	
		rad[0] = x; rad[1] = y; rad[2] = z;

		for(int i=0;i<3;i++) rad[i]*=PIOVER180;

		if(!found)
		{
			int iN = model->getPointCount();
			for(int i=0;i<iN;i++)		
			if(model->isPointSelected(i))
			{
				model->setPointRotation(i,rad);

				found = true; break;
			}
		}
		if(!found)
		if(!mode||mode==Model::ANIMMODE_SKELETAL)
		{
			int iN = model->getBoneJointCount();
			for(int i=0;i<iN;i++)			
			if(model->isBoneJointSelected(i))
			{
				if(mode)
				{
					int anim = model->getCurrentAnimation();
					int frame = model->getCurrentAnimationFrame();

					model->setSkelAnimKeyframe(anim,frame,i,true,rad[0],rad[1],rad[2]);
					model->setCurrentAnimationFrame(frame); // Force re-animate
				}
				else
				{
					model->setBoneJointRotation(i,rad);
					model->setupJoints();
				}

				found = true; break;
			}
		}

		if(found)
		model->operationComplete(::tr("Set Rotation","operation complete"));
	}
}

void SideBar::PropPanel::group_props::change(int changeBits)
{
	if(0==(changeBits&(Model::AddOther|Model::SelectionChange)))
	{
		return; // Only change if group or selection change	
	}

	int iN = model->getGroupCount();
	group.menu.clear();
	group.menu.add_item(-1,::tr("<None>"));
	for(int i=0;i<iN;i++)
	group.menu.add_item(i,model->getGroupName(i));	
	group.menu.add_item(iN,::tr("<New>"));

	iN = model->getTextureCount();
	material.menu.clear();
	material.menu.add_item(-1,::tr("<None>"));
	for(int i=0;i<iN;i++)
	material.menu.add_item(i,model->getTextureName(i));

	iN = model->getProjectionCount();
	projection.menu.clear();
	projection.menu.add_item(-1,::tr("<None>"));
	for(int i=0;i<iN;i++)	
	projection.menu.add_item(i,model->getProjectionName(i));
	//projection.nav.enable(iN!=0); 
	projection.nav.set_hidden(iN==0); 

	int grp = -1, prj  = -1;
	iN = model->getTriangleCount();
	for(int i=0;i<iN;i++)
	if(model->isTriangleSelected(i))
	{
		if(grp==-1)
		{
			grp = model->getTriangleGroup(i);
			if(prj!=-1) break; //UNNECCESSARY
		}
		if(prj==-1)
		{
			prj = model->getTriangleProjection(i);
			if(grp!=-1) break; //UNNECCESSARY
		}
	}

	material.nav.enable(grp!=-1);
	group.menu.select_id(grp);		
	material.menu.select_id(model->getGroupTextureId(grp));
	projection.menu.select_id(prj);
}
void SideBar::PropPanel::group_props::submit(int id)
{
	if(id==id_material_settings)
	{	 		
		int g = group.menu;
		int m = material.menu; if(g>=0)
		{
			model->setGroupTextureId(g,m);
			model->operationComplete(::tr("Set Material","operation complete"));
		}
	}
	else if(id==id_projection_settings)
	{
		int p = projection.menu;

		int iN = model->getTriangleCount();
		for(int i=0;i<iN;i++)		
		if(model->isTriangleSelected(i))
		{
			model->setTriangleProjection(i,p);
		}

		if(projection.window.enable(p>=0).enabled())
		{
			model->applyProjection(p);
		}

		model->operationComplete(::tr("Set Projection","operation complete"));
	}
	else if(id==id_group_settings) 
	{
		int g = group.menu;		
		int iN = model->getGroupCount();
		if(g==-1)
		{
			material.nav.disable();
			iN = model->getTriangleCount();
			for(int i=0;i<iN;i++)		
			if(model->isTriangleSelected(i))
			{
				g = model->getTriangleGroup(i);
				if(g>=0)
				model->removeTriangleFromGroup(g,i);
			}
			model->operationComplete(::tr("Unset Group","operation complete"));
		}
		else if(g!=iN) new_group:
		{
			model->addSelectedToGroup(g);
			model->operationComplete(::tr("Set Group","operation complete"));

			material.menu.select_id(model->getGroupTextureId(g));
			material.nav.enable();
		}
		else if(!event.wheel_event)
		{	
			std::string groupName;
			if(id_ok==Win::EditBox(&groupName,::tr("New Group","Name of new group,window title"),::tr("Enter new group name:")))
			{
				model->addGroup(groupName.c_str());
				group.menu.selection()->text() = groupName;
				group.menu.add_item(iN+1,::tr("<New>"));
				goto new_group;
			}
		}
		else group.menu.select_id(iN-1); 
	}
	else assert(0);
}

void SideBar::PropPanel::proj_props::change(int changeBits)
{
	int iN = model->getProjectionCount();
	for(int i=0;i<iN;i++)
	if(model->isProjectionSelected(i))
	{
		if(changeBits) // Update projection fields
		{
			menu.select_id(model->getProjectionType(i));
			break;
		}
		else model->setProjectionType(i,menu);
	}

	if(!changeBits)
	model->operationComplete(::tr("Set Projection Type","operation complete"));
}

static void sidebar_update_weight_v(Widgets95::li &v, bool enable, int type, int weight)
{
	Widgets95::li::item *it[4] = {v.first_item()};
	for(int i=1;i<4;i++) it[i] = it[i-1]->next();

	it[0]->set_text(::tr("<Mixed>","multiple types of bone joint influence"));
	it[1]->set_text(::tr("Custom","bone joint influence"));
	it[2]->set_text(::tr("Auto","bone joint influence"));
	it[3]->set_text(::tr("Remainder","bone joint influence"));

	if(enable)
	{
		utf8 str = "%d"; switch(type)
		{
		case 0: str = ::tr("<Mixed>","multiple types of bone joint influence");
				break;
		case 2: str = ::tr("Auto: %d"); //::tr("Auto: %1").arg(weight);
				break;
		case 3: str = ::tr("Rem: %d"); //::tr("Rem: %1").arg(weight);
				break;
		}
		auto &ittt = it[type]->text();
		ittt.format(str,weight);
		it[type]->set_text(ittt);
	}
	v.set_int_val(type); //v.select_id(type); 

	if(enable!=v.enabled()) v.parent()->enable(enable);
}
void SideBar::PropPanel::infl_props::change(int changeBits)
{
	if(0==(changeBits&(Model::AddOther|Model::SelectionChange)))
	{
		return; // Only change if group or selection change	
	}
	
	index_group *groups = &i0;

	int bonesN = model->getBoneJointCount();

	bool enable = !model->getAnimationMode();

	//FIX ME
	//Don't rebuild this list if unchanged.
	for(int i=0;i<Model::MAX_INFLUENCES;i++) 
	{
		groups[i].joint.clear();
		groups[i].joint.enable(enable);
		groups[i].weight.disable(); 
	}
	if(enable) //NEW
	{
		groups[0].joint.add_item(-1,::tr("<None>"));
		for(int i=0;i<bonesN;i++)
		groups[0].joint.add_item(i,model->getBoneJointName(i));
		for(int i=1;i<Model::MAX_INFLUENCES;i++) 
		groups[i].joint.reference(groups[0].joint);
	}

	/*REMINDER: +1 must skip over <Mixed> in the menu. */
	// Update influence fields		
	JointCount def = {}; def.typeIndex = Model::IT_Auto+1;
	jcl.clear(); jcl.resize(bonesN,def);
	
	pos_list l; model->getSelectedPositions(l);
	pos_list::iterator it,itt = l.end();

	// for now just do a sum on which bone joints are used the most
	// TODO: may want to weight by influence later
	for(it=l.begin();it<itt;it++) switch(it->type)
	{
	case Model::PT_Vertex: case Model::PT_Point: //TODO: Filter out.
	
		const infl_list &ll = model->getPositionInfluences(*it);
		infl_list::const_iterator jt,jtt;
		for(jt=ll.begin(),jtt=ll.end();jt<jtt;jt++)		
		{
			int bone = jt->m_boneId;
			jcl[bone].count++;
			jcl[bone].weight+=(int)lround(jt->m_weight*100);
			jcl[bone].typeCount[jt->m_type]++;
		}
	}

	for(int bone=0;bone<bonesN;bone++)
	{
		int type = -1;
		for(int t=0;type!=0&&t<Model::IT_MAX;t++)		
		if(jcl[bone].typeCount[t]>0)
		{
			// If type index is unset,set it to the combo box
			// index for our type (off by one). If type index
			// is already set,it's mixed (index 0)
			type = type<0?t+1:0;
		}

		if(type>=0)
		{
			jcl[bone].typeIndex = type;
			jcl[bone].weight = (int)lround(jcl[bone].weight/(double)jcl[bone].count);
		}
		else jcl[bone].weight = 100;
	}

	for(int i=0;i<Model::MAX_INFLUENCES;i++)
	{
		int maxVal = 0, bone = -1;

		for(int b=0;b<bonesN;b++)		
		if(!jcl[b].inList&&jcl[b].count>maxVal)
		{
			bone = b; maxVal = jcl[b].count;
		}

		// No more influences; done.
		if(maxVal<=0) 
		{
			for(;i<Model::MAX_INFLUENCES;i++)
			{
				groups[i].joint.select_id(-1);
				groups[i].v.select_id(-1);
			}
			break; 
		}

		jcl[bone].inList = true;

		if(!enable) //NEW
		groups[i].joint.add_item(bone,model->getBoneJointName(bone));
		groups[i].joint.select_id(bone);
		groups[i].prev_joint = bone;

		log_debug("bone i %d is bone ID %d\n",i,bone); //???

		sidebar_update_weight_v(groups[i].v,true,jcl[bone].typeIndex,jcl[bone].weight);
	}
}
void SideBar::PropPanel::infl_props::submit(control *c)
{
	int index = c->id()-'0';

	index_group *groups = &i0, &group = groups[index];

	int j = group.joint; bool updateRemainders = false;

	pos_list l; model->getSelectedPositions(l);
	pos_list::iterator it,itt = l.end();

	if(c==group.joint) /* Joint selected? */
	{	
		if(group.joint)
		for(int i=0;i<Model::MAX_INFLUENCES;i++) if(i!=index)
		{
			int compile[4==Model::MAX_INFLUENCES]; (void)compile;

			if(groups[i].prev_joint==group.joint.int_val())
			{
				// trying to assign new joint when we already have that joint
				// in one of the edit boxes. Change the selection back. This
				// will cause some nasty bugs and probably confuse the user.
				// Change the selection back.	
				return (void)group.joint.select_id(group.prev_joint);
			}
		}

		int new_joint = group.joint;
		int old_joint = group.prev_joint;
		group.prev_joint = new_joint--; old_joint--;

		if(old_joint>0)
		{
			for(it=l.begin();it<itt;it++)			
			model->removePositionInfluence(*it,old_joint);
			jcl[old_joint].count = 0;
		}
		int weight = 0; if(new_joint>0)
		{
			jcl[new_joint].count = l.size();

			for(it=l.begin();it<itt;it++)
			{
				double w = model->calculatePositionInfluenceWeight(*it,new_joint);

				log_debug("influence = %f\n",w); //???

				model->addPositionInfluence(*it,new_joint,Model::IT_Auto,w);

				weight+=(int)lround(w*100);
			}

			if(!l.empty())			
			jcl[new_joint].weight = (int)lround(weight/(double)l.size());			
			/*REMINDER: +1 must skip over <Mixed> in the menu. */
			jcl[new_joint].typeIndex = Model::IT_Auto+1;
		}		
		model->operationComplete(::tr("Change Joint Assignment","operation complete"));

		bool enable = new_joint>=0;
		weight = enable?jcl[new_joint].weight:0;		
		sidebar_update_weight_v(group.v,enable,enable?jcl[new_joint].typeIndex:-1,weight);
	}
	else if(c==group.weight) /* Weight text edited? */
	{
		double weight = std::max(0,std::min<int>(100,group.weight))/100.0;

		log_debug("setting joint %d weight to %f\n",j,weight); //???

		for(it=l.begin();it<itt;it++)
		{
			model->setPositionInfluenceType(*it,j,Model::IT_Custom);
			model->setPositionInfluenceWeight(*it,j,weight);
		}
		model->operationComplete(::tr("Change Influence Weight","operation complete"));
	}
	else if(c==group.v) /* Weight mode selected? */
	{
		Model::InfluenceTypeE type = Model::IT_Auto;
		switch(group.v)
		{
		case 0: return; //<Mixed> // Not really a valid selection
		case 1: type = Model::IT_Custom; break;
		case 3: type = Model::IT_Remainder; break;
		}

		log_debug("setting joint %d type to %d\n",j,(int)type); //???

		int weight = 0; for(it=l.begin();it<itt;it++)
		{
			model->setPositionInfluenceType(*it,j,type);
			if(type==Model::IT_Auto)
			{
				double w = model->calculatePositionInfluenceWeight(*it,j);
				model->setPositionInfluenceWeight(*it,j,w);
				weight+=(int)lround(w*100);
			}
			else
			{
				model->setPositionInfluenceWeight(*it,j,(double)jcl[j].weight/100);
			}
		}
		model->operationComplete(::tr("Change Influence Type","operation complete"));

		if(type==Model::IT_Auto)
		{
			jcl[j].weight = (int)lround(weight/(double)jcl[j].count);
		}

		jcl[j].typeIndex = type+1;

		sidebar_update_weight_v(group.v,true,jcl[j].typeIndex,jcl[j].weight);
	}

	for(int i=0;i<Model::MAX_INFLUENCES;i++)	
	if(groups[i].v.int_val()-1==Model::IT_Remainder)
	if(j>=0&&j<(int)model->getBoneJointCount()) //???
	{
		log_debug("getting remainder weight for joint %d\n",j); //???
		
		int count = 0; double weight = 0;
		for(it=l.begin();it<itt;it++) switch(it->type)
		{
		case Model::PT_Vertex: case Model::PT_Point: //TODO: Filter out.
			
			const infl_list &ll = model->getPositionInfluences(*it);
			infl_list::const_iterator jt,jtt;
			for(jt=ll.begin(),jtt=ll.end();jt<jtt;jt++)
			if(jt->m_type==Model::IT_Remainder&&j==(*jt).m_boneId)
			{
				weight+=(int)lround(jt->m_weight*100);
				count++;
			}
		}

		int w = count?(int)lround(weight/(double)count):0;
		
		log_debug("updating box %d with remaining weight %d\n",i,w); //???

		assert(group.v.enabled());
		sidebar_update_weight_v(group.v,true,group.v,w);
	}
}