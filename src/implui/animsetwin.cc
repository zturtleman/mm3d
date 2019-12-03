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


#include "log.h"
#include "msg.h"

//MM3D's table is not sortable. It needs more work
//I think to be sortable. unsort must go obviously.
enum{ animsetwin_sort=0 };

struct AnimSetWin : Win
{
	void submit(int);

	AnimSetWin(MainWin &model)
		:
	Win("Animation Sets"),model(model),
	type(main,"",id_item),
	table(main,"",id_subitem),
	header(table,animsetwin_sort,id_sort),
		name_col(header,"Name"),
		fps_col(header,"FPS"),
		frames_col(header,"Frames"),
		loop(name_col,"Loop",id_check),
	nav1(main),
		up(nav1,"&Up",id_up),
		down(nav1,"&Down",id_down),
	nav2(main),
		add(nav2,"&New",id_new),
		name(nav2,"&Rename",id_name),
		del(nav2,"&Delete",id_delete),
	nav3(main),
		copy(nav3,"&Copy",id_copy),
		split(nav3,"&Split",id_split),
		join(nav3,"&Join",id_join),
		merge(nav3,"&Merge",id_merge),
	convert(main,"&Convert To Frame Animation",id_convert),
	f1_ok_cancel(main)	
	{	
		(checkbox_item::cbcb&) //YUCK
		table.user_cb = cbcb;

		type.expand();		
		nav1.ralign();
		nav2.expand_all().proportion();
		nav3.expand_all().proportion();
		convert.expand();

		//The columns should support the window.
		//table.expand();
		loop.ctrl_tab_navigate();
		name_col.span()*=2;
		//NOTE: The end column auto-extends, so
		//it's better to have the short name as
		//the middle column.
		fps_col.span() = frames_col.span() = 0;

		active_callback = &AnimSetWin::submit;

		submit(id_init);
	}

	MainWin &model;
	Model::AnimationModeE mode;

	dropdown type; listbox table; //MERGE US?
	listbar header;
	multiple::item name_col,fps_col,frames_col;
	boolean loop; 
	row nav1; button up,down;
	row nav2; button add,name,del;
	row nav3; button copy,split,join,merge;
	button convert;
	f1_ok_cancel_panel f1_ok_cancel;

	void refresh();

	checkbox_item *new_item(int id)
	{
		utf8 n = model->getAnimName(mode,id);		
		double fps = model->getAnimFPS(mode,id);
		int frames = model->getAnimFrameCount(mode,id);
		auto i = new checkbox_item(id,nullptr);
		i->check(model->getAnimLooping(mode,id));
		i->text().format(&"%s\0%g\0%d",n,fps,frames);
		return i;
	}

	static void cbcb(int impl, checkbox_item &it)
	{
		assert(impl&it.impl_checkbox);
		auto w = (AnimSetWin*)it.list().ui();
		w->model->setAnimLooping(w->mode,it.id(),it.checkbox());
	}

	//HACK: Since the columns can be sorted it's
	//necessary to undo it. This should probably
	//throw up a prompt.
	bool unsort()
	{
		if(!animsetwin_sort) return true;

		if(header.text().empty()) return true;
				
			//Prompt?

		header.clear();
		header.sort_items([&](li::item *a, li::item *b)
		{
			return a->id()<b->id();
		});		

		return true;
	}
};

struct AnimEditWin : EditWin
{
	AnimEditWin(int id, li::item *i)
		:
	EditWin((char*)nullptr,
	id==id_name?"Rename Animation":"New Animation",
	"New name:",0,0,&AnimEditWin::submit_cb,false),
	nav2(nav),
	fps(nav2,"FPS"),frames(nav2,"Frames")
	{
		double c1 = 30; int c2 = 1; if(i)
		{
			//It's a problem if merely "focused".
			//i->select();
			assert(i->multisel());
			utf8 row[3]; i->text().c_row(row);
			edit.set_text(row[0]);
			c1 = strtod(row[1],nullptr);
			c2 = atoi(row[2]);
		}
		else if(id==id_name)
		{
			//Must be multi-selection, so don't
			//give impression all names will be
			//reassigned.
			edit.disable();
		}
		fps.edit<double>(1,c1,120);
		frames.edit<int>(1,c2,INT_MAX);

		//HACK: Cleared boxes mean don't change.
		//This is essential for multi-selection.
		if(!i) fps.text().clear();
		if(!i) frames.text().clear();

		submit(main); //YUCK
	}
	void submit_cb(control *c) //YUCK
	{
		//FIX ME: How to extend EditWin requires
		//more consideration. This is a big mess.

		if(c==main) //id_init
		{
			fps.place(bottom);
			fps.compact(ok_cancel.cancel.span());
			frames.place(bottom);
			frames.compact(ok_cancel.ok.span());
		}
		basic_submit(c->id());
	}
	
	row nav2;
	textbox fps,frames;
};

struct AnimConvertWin : Win
{
	void submit(int);

	AnimConvertWin(AnimSetWin &owner)
		:
	Win("Convert To Frame"),owner(owner),
	table(main,"Convert Skeletal to Frame:",id_item),
		header(table,id_sort),
	f1_ok_cancel(main)
	{
		active_callback = &AnimConvertWin::submit;

		submit(id_init);
	}

	AnimSetWin &owner;

	listbox table;
	listbar header;
	f1_ok_cancel_panel f1_ok_cancel;
};
void AnimConvertWin::submit(int id)
{
	switch(id)
	{
	case id_init:
 
		//Assuming table will be auto-sized.
		header.add_item("Skeletal Animation"); //200
		header.add_item("Frame Animation"); //200
		header.add_item("Frame Count"); //30
		owner.table^[&](li::multisel ea)
		{
			auto row = new li::item(ea->id());

			utf8 name = ea->text().c_str();
			row->text().format(&"%s\0%s\0%d",name,name,
			owner.model->getAnimFrameCount(owner.mode,ea->id()));
			
			table.add_item(row);
		};
		f1_ok_cancel.ok_cancel.ok.name("Convert");
		break;

	case id_sort:

		header.sort_items([&](li::item *a, li::item *b)
		{
			utf8 row1[3]; a->text().c_row(row1);
			utf8 row2[3]; b->text().c_row(row2);
			switch(int col=header)
			{
			case 2: return atoi(row1[col])<atoi(row2[col]);
			default: return strcmp(row1[col],row2[col])<0;
			}
		});
		break;
	
	case id_item:
	
		switch(int col=header)
		{
		case 0: col = 1; default:

			textbox modal(table);
			if(2==col) modal.edit<int>(1,INT_MAX);
			if(modal.move_into_place(table.outline(),col)) 
			modal.return_on_enter();
		}
		break;
	
	case id_ok:

		table^[&](li::allitems ea)
		{
			utf8 row[3]; ea->text().c_row(row);
			owner.model->convertAnimToFrame(owner.mode,ea->id(),row[1],atoi(row[2]));
		};
		break;
	}

	basic_submit(id);
}

void AnimSetWin::submit(int id)
{
	switch(id)
	{
	case id_init:
	
		type.add_item(0,::tr("Skeletal Animation"));
		type.add_item(1,::tr("Frame Animation"));

		//NEW: Select current animation?
		if(auto m=model->getAnimationMode())
		{
			//HACK: Intialize table, then select.
			type.select_id(m-1);
			int n = model->getCurrentAnimation();
			submit(id_item);
			table.outline(n);
			table.find_line_ptr()->select();
			table.show_line(n); //UNTESTED
			break;
		}
		else
		{
			int a = model->getAnimCount(Model::ANIMMODE_SKELETAL);
			int b = model->getAnimCount(Model::ANIMMODE_FRAME);
			type.select_id(a?0:b?1:0);
		}
		
		//break; /*FALLING THROUGH*/
	
	case id_item:
	{
		Model::AnimationModeE cmp = mode;
		switch(type)
		{
		case 1: mode = Model::ANIMMODE_FRAME; 
		break;
		default: mode = Model::ANIMMODE_SKELETAL;
		}
		if(cmp!=mode) table.clear();
		
		refresh(); break;
	}
	case id_check:

		table^[&](li::multisel ea){ ea->check(loop); };
		break;

	case id_sort:
	
		if(animsetwin_sort)
		header.sort_items([&](li::item *a, li::item *b)
		{
			utf8 row1[3]; a->text().c_row(row1);
			utf8 row2[3]; b->text().c_row(row2);
			switch(int col=header)
			{			
			case 0: return strcmp(row1[col],row2[col])<0;
			case 1: return atoi(row1[col])<atoi(row2[col]);
			case 2: return strtod(row1[2],0)<strtod(row2[2],0);
			}
		});
		break;
	
	case id_subitem: 
	
		switch(event.get_click())
		{
		default: //Spacebar????
		case 2: id = id_name; break; //Double-click?
		case 1:

			//This test implements Windows Explorer
			//select, wait, single-click, to rename 
			//logic.
			if(event.listbox_item_rename(true,true))
			{
				textbox modal(table); switch(header)
				{
				case 1: modal.edit<double>(1,120); break;
				case 2: modal.edit<int>(1,INT_MAX); break;
				}

				if(modal.move_into_place())
				if(modal.return_on_enter()) switch(header)
				{
				case 0: model->setAnimName(mode,(int)table,modal); break;
				case 1: model->setAnimFPS(mode,(int)table,modal); break;
				case 2: model->setAnimFrameCount(mode,(int)table,(int)modal); break;
				}
			}
			return;		
		}
		//break;
	
	case id_name: case id_new: //Append to back?
	{
		int j = 0;
		li::item *it = nullptr;
		table^[&](li::multisel ea)
		{
			j++; it = ea;
		};		
		if(!j&&id==id_name) //NEW
		{
			//Should probably disable Delete/Rename then?
			return;
		}
		if(j>1) it = nullptr;
		AnimEditWin e(id,it);		
		if(id_ok!=e.return_on_close())
		return;
		utf8 c_str = e.edit.c_str();
		j = -1; table^[&](li::multisel ea)
		{
			if(id==id_new)
			{
				if(j==-1) j = ea->id(); ea->unselect();
			}
			else //id_name
			{
				if(*c_str)
				model->setAnimName(mode,ea->id(),c_str);
				if(!e.fps.text().empty())
				model->setAnimFPS(mode,ea->id(),e.fps);
				if(!e.frames.text().empty())
				model->setAnimFrameCount(mode,ea->id(),(int)e.frames);

				auto &t = ea->text();
				t.replace(0,t.find('\0'),c_str);
			}
		};
		if(id==id_new)
		{
			int i = model->addAnimation(mode,c_str);
			if(i==-1) break;

			if(!e.fps.text().empty())
			model->setAnimFPS(mode,i,e.fps);
			if(!e.frames.text().empty())
			model->setAnimFrameCount(mode,i,(int)e.frames);
			if(j==-1) j = i;
			model->moveAnimation(mode,i,j); //NEW

			unsort(); //YUCK

			table.outline(j);
			table.add_item((new_item(j))->select());

			refresh();
		}	
		else table.redraw(); //HACK
		break;
	}	
	case id_up: case id_down:
	case id_delete:
	case id_copy: case id_split: case id_join: case id_merge:
	{
		unsort(); //YUCK

		int a = -1, b = 0; table^[&](li::multisel ea)
		{
			switch(id)
			{
			case id_delete:

				model->deleteAnimation(mode,ea->id());
				table.erase(ea);
				break;

			case id_up:

				table.insert_item(ea,ea->prev());
				break;

			case id_down:

				table.insert_item(ea,ea->next(),behind);

				//HACK: Infinite loop condition.
				ea->id() = ~ea->id(); ea->unselect();
				break;

			case id_copy:

				unsort(); //YUCK

				model->moveAnimation(mode, //NEW
				model->copyAnimation(mode,ea->id(),*ea),ea->id()+1);
				table.insert(ea->next(),new_item(ea->id()+1)); 
				break;

			case id_split:
								
				a = ea->id(); b = model->getAnimFrameCount(mode,a); if(b<2)
				{
					InfoBox(::tr("Cannot Split","Cannot split animation window title"),
					::tr("Must have at least 2 frames to split","split animation"),id_ok);
				}
				else //SIMPLIFY ME
				{	
					std::string name;
					name = ::tr("Split","'Split' refers to splitting an animation into two separate animations");
					name.push_back(' '); //MERGE US 
					name.append(model->getAnimName(mode,a));
					name.push_back(' '); //MERGE US
					name.append(::tr("at frame number","the frame number where the second (split)animation begins"));

					int split = b/2;
					if(id_ok!=EditBox(&split,::tr("Split at frame","Split animation frame window title"),name.c_str(),2,b))
					return;
					
					name = model->getAnimName(mode,a);
					name.push_back(' ');
					name.append(::tr("split"));
					if((b=model->splitAnimation(mode,a,name.c_str(),split))<0)
					return;
		
					a++; model->moveAnimation(mode,b,a); //NEW

					ea = ea->next(); //HACK
					{
						table.insert(ea,new_item(a));
					}							
					for(;ea;ea.item=ea->next()) //HACK: Increment the Ids.
					{
						ea->id() = ++a;
					}
				}
				break;
			
			case id_merge: case id_join: 
			
				if(a==-1)
				{
					a = ea->id(); break;
				}
				b+=ea->id();
				{
					bool ok = false; switch(id)
					{
					case id_join: ok = model->joinAnimations(mode,a,b); break;
					case id_merge: ok = model->mergeAnimations(mode,a,b); break;
					}
					if(ok){ b--; table.erase(ea); }
				}
				b-=ea->id(); 
				break;
			}
		};
		if(id==id_up||id==id_down)
		{
			a = 0; table^[&](li::allitems ea)
			{
				b = ea->id(); if(b<0) //HACK
				{
					ea->id() = b = ~b; ea->select();
				}								
				model->moveAnimation(mode,b,a++);
			};
		}
		refresh(); break;
	}
	case id_convert:
	{
		AnimConvertWin(*this).return_on_close();
		break;
	}
	case id_ok:
			
		model->operationComplete(::tr("Animation changes","operation complete"));
	
		//It would probably be more kosher for
		//AnimPanel to monitor Model::Observer.
		model.sidebar.anim_panel.refresh_list();
		break;

	case id_cancel:

		model->undoCurrent();
		break;
	}

	basic_submit(id);
}
void AnimSetWin::refresh()
{
	int iN = 0; if(!table.empty())
	{
		unsort();
		table^[&](li::allitems ea){ ea->id() = iN++; };
	}
	else if(iN=model->getAnimCount(mode))
	{
		main_panel()->enable();
		if(mode==Model::ANIMMODE_SKELETAL)
		{
			merge.enable(); convert.enable();
		}

		for(int i=0;i<iN;i++)
		table.add_item(new_item(i));

		//MM3D's table is not sortable. It needs more work
		//I think to be sortable. unsort must go obviously.
		if(!animsetwin_sort)
		{
			//header.disable(); loop.enable(); 
		}
	}
	else
	{
		main_panel()->disable();
		type.enable();
		add.enable();
		f1_ok_cancel.nav.enable();
	}
}

extern void animsetwin(MainWin &m){ AnimSetWin(m).return_on_close(); }