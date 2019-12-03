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


#ifndef __VIEWWIN_H__
#define __VIEWWIN_H__

#include "toolbox.h"
#include "sidebar.h"
#include "viewpanel.h"

class MainWin : Model::Observer
{	
public:

	// Model::Observer method
	virtual void modelChanged(int changeBits);
		
	MainWin(Model *model=nullptr); ~MainWin();

	static bool quit();

	static bool open(utf8 file, MainWin *window=nullptr);

	static bool save(Model *model, bool expdir=false);

	static void merge(Model *model, bool anims=false);
		
	void undo(),redo();

	bool save_work();
	bool save_work_prompt();
		
	bool reshape(int x=10000, int y=10000);

	void frame(int globally=1); //Model::OS_Global

	template<class Functor>
	static void each(const Functor &f)
	{
		extern std::vector<MainWin*> viewwin_list;
		for(auto ea:viewwin_list) f(*ea);
	}

public: //FINISH US

	void run_script(const char *filename);

	/*VESTIGIAL??? NO-OP?
	These are id_anim_rotate and id_anim_transl? 
	https://github.com/zturtleman/mm3d/issues/16
	void animSetRotEvent(),animSetTransEvent();*/

public:

	//NOTE: I'm adding these because ui::main is
	//easily confused with main as shorthand for
	//MainWin. This way it's synonymous with its
	//model.
	operator Model*(){ return model; }
	Model *operator->(){ return model; }	
	
	Model *const model;
		
	const int glut_window_id;
	
	Toolbox toolbox;	
	ViewPanel views; SideBar sidebar; //IN Z-ORDER

	void open_texture_window();	
	void open_animation_system();
	void sync_animation_system();
	void open_animation_window();
	void open_transform_window();
	void open_projection_window();
	void perform_menu_action(int);

private:
	
	void _init_menu_toolbar();	

	Model *_swap_models(Model*);

	bool _window_title_asterisk;
	void _rewrite_window_title();

	friend void viewwin_close_func();
	struct AnimWin *_animation_win;
	struct TransformWin *_transform_win;
	struct ProjectionWin *_projection_win;
	struct TextureCoordWin *_texturecoord_win;
	
	//These menus have state.
	//NOTE: It's pointless to separate them as long
	//as the global "config" file holds their state.
	int _view_menu,_rops_menu,_anim_menu,_menubar;

	friend void viewwin_toolboxfunc(int);
	int _prev_tool,_curr_tool;
	int _prev_mode;
	int _prev_view,_curr_view;
	void _view(int i, void (ViewPanel::*mf)());
};

#endif // __VIEWWIN_H__
