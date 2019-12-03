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


#ifndef __SIDEBAR_H__
#define __SIDEBAR_H__

#include "mm3dtypes.h" //PCH
#include "win.h"

#include "model.h" 

struct SideBar : Win 
{	
	void submit(control*);

	SideBar(class MainWin&);
	 
	struct AnimPanel
	{
		void submit(control*);

		AnimPanel(SideBar &bar, MainWin &model)
			:
		model(model),
		media_nav(bar.main),
		play(media_nav,"",id_animate_play),
		stop(media_nav,"",id_animate_pause),
		nav(bar.main,"Animation"),
		animation(nav,"",id_item),
		frame_nav(nav),
		frame(frame_nav,"",id_subitem),
		window(frame_nav,"...",id_animate_settings)
		{
			//The "media bar" is proportional so
			//it rests beside the animation tool.

			media_nav.proportion().space(1,2,1,0,-1);
			play.expand().picture(pics[pic_play]);
			stop.expand().picture(pics[pic_stop]);

			animation.expand();
			frame_nav.expand().space(1);
			frame.name("Frame");
			frame.edit(0,0,0).compact().expand();						
			window.drop(frame.drop()).span(0).ralign();

			submit(bar.main); //id_init
		}
	
		MainWin &model;

		row media_nav;
		button play,stop;

		rollout nav;
		dropdown animation;
		row frame_nav;
		spinbox frame;
		button window;

	private:

		friend struct SideBar;
		friend struct AnimWin;
		friend struct AnimSetWin;
		int sep_animation;
		int new_animation;
		void refresh_list();
	};

	struct BoolPanel
	{
		void init(),submit(control*);

		BoolPanel(SideBar &bar, MainWin &model)
			:
		model(model),
		nav(bar.main,"Boolean"), //"Boolean Operation"
		op(nav,""), //"Operation"
		button_a(nav,"Set Object A"),
		//status(nav,""),
		button_b(nav)
		{
			//Maximizing use of space.
			//op.style(bi::etched).expand();
			//op.align();
			op.calign().space(2,0,0);
			op.add_item(0,"Fuse");
			op.add_item(1,"Union");
			col.set_parent(op);
			op.add_item(2,"Subtract"); //Subtraction
			op.add_item(3,"Intersect"); //Intersection

			button_a.expand();
			button_b.expand();

			init(); submit(op.select_id(1));
		}

		MainWin &model;

		enum{ op_union=0,op_union2, op_subtract, op_intersect, };

		rollout nav;
		multiple op;
		column col;
		button button_a;
		//titlebar status;
		button button_b;
	};

	struct PropPanel
	{
		void submit(control*);

		void modelChanged(); //RENAME ME
		void modelChanged(int changeBits);

		PropPanel(SideBar &bar, MainWin &model)
			:
		model(model),
		nav(bar.main,"Properties"),
		//NOTE: Compilers squeal if "this" is passed
		//in the initializer's list.
		name(bar.prop_panel),
		pos(bar.prop_panel),
		rot(bar.prop_panel),
		group(bar.prop_panel),
		proj(bar.prop_panel),
		infl(bar.prop_panel)
		{
			submit(bar.main); //id_init
		}
		
		MainWin &model;
		
		struct props_base
		{	
			props_base(PropPanel &p)
			:model(p.model),nav(p.nav)
			{
				nav.expand();
			}

			MainWin &model;

			panel nav;
		};

		struct nav_group
		{
			nav_group(node *frame, utf8 name, int id)
				:
			nav(frame),
			menu(nav,"",-id),window(nav,"...",id)
			{
				nav.name(name).expand().space(1);
				menu.expand();
				window.span(0).ralign();
				window.drop(menu.drop()+1);				
				window.space<top>(-1);
			}

			row nav;
			dropdown menu; button window;
		};
		
		struct name_props : props_base
		{
			void change(int=0);

			name_props(PropPanel &p)
				:
			props_base(p),name(nav,"",id_name)
			{
				nav.expand().name("Name");
				name.expand();
			}
			textbox name;
		};
		struct rot_props : props_base
		{
			void change(int=0);

			rot_props(PropPanel &p)
				:
			props_base(p),
			x(nav,"X",'X'),y(nav,"Y",'Y'),z(nav,"Z",'Z')
			{
				nav.name("Rotation").expand();

				//NOTE: THESE SUPPORT THE SIDEBAR WIDTH.
				for(int i=0;i<3;i++) 
				(&x+i)->edit(0.0).compact(60*2).ralign();
			}

			spinbox x,y,z;
		};
		struct pos_props : rot_props
		{
			void change(int=0);

			pos_props(PropPanel &p)
				:
			rot_props(p),
			dimensions(nav,"Dimensions")
			{
				nav.name("Position");
				dimensions.place(bottom).expand();
			}

			textbox dimensions;

			double centerpoint[3];
		};		
		struct group_props : props_base
		{
			void change(int);

			void submit(int);

			group_props(PropPanel &p)
				:
			props_base(p),
			group(nav,"Group",id_group_settings),
				//"Group Material:"
			material(nav,"Material",id_material_settings),
				//"Texture Projection:"
			projection(nav,"Projection",id_projection_settings)
			{
				nav.expand();
			}

			nav_group group, material, projection; 
		};
		struct proj_props : props_base, nav_group
		{
			void change(int=0);

			proj_props(PropPanel &p)
				:
			props_base(p),
			nav_group(props_base::nav,"Projection Type",id_projection_settings)
			{}
		};
		struct infl_props : props_base
		{	
			void change(int);

			void submit(control*);

			infl_props(PropPanel &p)
				:
			props_base(p),
			i0(nav,'0'),i1(nav,'1'),i2(nav,'2'),i3(nav,'3')
			{
				//i0.joint.place(bottom).name("Joint");
				//i0.weight.place(bottom).name("Weight");
				nav.name("Joints");
			}

			struct index_group
			{
				dropdown joint;
				int prev_joint;
				textbox weight; dropdown v;
				li::item vi[4];

				index_group(node *frame, int id)
					:
				joint(frame,"1",id),
				weight(frame,"",id),v(weight,id)
				{						
					joint.expand().name()[0]+=id-'0';
					joint.compact();
					weight.expand();
					for(int i=0;i<4;i++)
					{
						vi[i].id() = i; v.add_item(vi+i);
					}
				}
			};
			index_group i0,i1,i2,i3;

			struct JointCount
			{
			bool inList;
			int count;
			int typeCount[Model::IT_MAX];
			int typeIndex;
			int weight;
			};
			std::vector<JointCount> jcl;
		};

		rollout nav;
		name_props name;
		pos_props pos;
		rot_props rot;
		group_props group;		
		proj_props proj;
		infl_props infl;
	};

	AnimPanel anim_panel;
	BoolPanel bool_panel;
	PropPanel prop_panel;	
	
		/* Qt supplemental */
	
	void setModel()
	{
		anim_panel.refresh_list();
		bool_panel.init();
		prop_panel.modelChanged();
	}
	void modelChanged(int changeBits)
	{
		if(changeBits&Model::AddAnimation)
		anim_panel.refresh_list();
		prop_panel.modelChanged(changeBits);
	}
};

#endif //__SIDEBAR_H__
