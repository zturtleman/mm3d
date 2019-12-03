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

#include "viewwin.h"

ViewBar::ViewBar(MainWin &model)
	:
Win(model.glut_window_id,
this==&model.views.bar1?top:bottom),
model(model)
{
	active_callback = &ViewBar::submit;
	
	if(top==subpos())
	{
		exterior_row.set_parent(this);
		portside_row.set_parent(this);
	}
	else
	{
		portside_row.set_parent(this);
		exterior_row.set_parent(this);		
	}
	exterior_row.expand();
	portside_row.space(0,1,2);
}

void ViewBar::submit(control *cc)
{		
	//Find the ModelView in particular as
	//a column in front of the containing
	//panel.

	if(model.views.params.nav==cc->parent())
	{
		model.views.tool->updateParam(cc->live_ptr());
		if(cc->user)
		config.set(model.views.param_config_key((utf8)cc->user),cc);
		return;
	}

	control *c = cc;
	while(c->parent()!=portside_row
		&&c->parent()!=exterior_row)
	{
		c = (control*)c->parent();				
	}
	if(ModelView*mv=c->prev<ModelView>())  
	{
		return mv->submit(cc->id());	
	}
	else if(c==cc) switch(cc->id())
	{
	case id_bar:
	if(int n=cc->int_val()+1)
	model.sidebar.anim_panel.frame.set_int_val(n).execute_callback();
	}
}
static std::vector<Tool::ViewE> mview_new;
void ViewBar::ModelView::init_view_dropdown()
{
	auto *ref = view.reference();
	auto &v = ref?*ref:view;
	v.clear();
	v.add_item(Tool::ViewPerspective,"&Perspective"); 
	for(auto ea:mview_new) if(ea<-1)
	{
		char n[] = "Persp (&0)"; n[8]+=-ea;
		v.add_item((int)ea,n);
	}
	//&B is below T. F/R is left of K/L.
	v.add_item(1,"&Front").add_item(2,"Bac&k");
	v.add_item(3,"&Left").add_item(4,"&Right");
	v.add_item(5,"&Top").add_item(6,"&Bottom");
	v.add_item(Tool::ViewOrtho,"&Orthographic");
	for(auto ea:mview_new) if(ea>-1)
	{
		char n[] = "Ortho (&1)"; n[8]+=ea-Tool::ViewOrtho;
		v.add_item((int)ea,n);
	}v.add_item(-1,"<&New View>"); //EXPERIMENTAL

	if(!ref) return; //YUCK
	auto &vp = ((ViewBar*)ui())->model.views;
	for(int i=0;i<vp.viewsN;i++)
	{
		auto &vv = vp.views[i]->view;
		if(&vv==ref) continue;
		vv.clear();
		vv.reference(*ref).select_id(vv);
	}		
}
void ViewBar::ModelView::submit(int id)
{	
	switch(id)
	{
	case id_init:
	
		if(!view.reference()) init_view_dropdown();

		if(top==ui()->subpos())
		{
			//nav.cspace_all<top>().space<bottom>(2); 
			view.space<top>(3);
			zoom.value.space<top>(4).space<right>(3);
		}
		else 
		{
			//nav.space<top>(1);
			view.space<top>(4); 
			zoom.value.space<top>(4).space<right>(3);
		}
		break;

	case id_item:
	
		switch(int e=view)
		{
		case -1:
		
			struct E : Win
			{
				E(int *lv):
				Win("Which projection?"),
				value(main,"",lv),
				persp(value,"Perspective"),
				ortho(value,"Orthographic"),
				ok_cancel(main)
				{
					int a = -1;
					int b = Tool::ViewOrtho;
					for(auto ea:mview_new)
					{
						a = std::min<int>(a,ea);
						b = std::max<int>(b,ea);
					}
					persp.id(a-1); ortho.id(b+1);
					a = a>-9; 
					b = b-Tool::ViewOrtho<8;					
					if(!a) persp.disable();
					if(!b) ortho.disable();
					if(!a&&!b) ok_cancel.ok.disable();
					else value.select_id(a?persp.id():ortho.id());
				}
				multiple value;
				multiple::item persp,ortho;
				ok_cancel_panel ok_cancel;
			};
			if(event.wheel_event
			||id_ok!=E(&e).return_on_close())
			{			
				view.select_id(port.getView());
				return;
			}
			mview_new.push_back((Tool::ViewE)e);
			MainWin::each([](MainWin &ea)
			{
				ea.views->init_view_dropdown(); 
			});
			//break;
		
		default: port.viewChangeEvent((Tool::ViewE)e); 
		}
		break;
	
	case '=': port.setZoomLevel(zoom.value); break;
	case '+': port.zoomIn(); break;
	case '-': port.zoomOut(); break;
	}
}

