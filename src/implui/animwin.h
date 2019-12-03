/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2008 Kevin Worcester
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


#ifndef __ANIMWIN_H__
#define __ANIMWIN_H__

#include "mm3dtypes.h" //PCH
#include "win.h"

struct AnimWin : Win
{
	void submit(int);
	
	void open(bool undo);

	void setModel(){ open(true); }

	AnimWin(class MainWin &model, int menu)
		:
	Win(::tr("Animator")), //::tr("Animations")
	menu(menu),
	model(model),impl(),
	shelf1(main),shelf2(main),
	invisible_f1_and_close(main)
	{	
		hide();

		invisible_f1_and_close.nav.set_hidden();

		active_callback = &AnimWin::submit;

		submit(id_init);
	}
	virtual ~AnimWin();
	
	int menu;

	class MainWin &model;

	struct Impl; Impl *impl;

	enum /*Animation controls*/
	{		
		id_anim_fps=1000,
		id_anim_loop,
		id_anim_frames,
	};
	struct shelf1_group
	{
		shelf1_group(node *main)
			:
		nav(main),
		animation(nav,"Animation",id_item), //"Animations"
		del(nav,"Delete",id_delete), //"X"
		fps(nav,"FPS",id_anim_fps),
		loop(nav,"Loop",id_anim_loop),		
		frames(nav,"Frame&s",id_anim_frames) //Not Frame!
		{
			nav.expand();
			loop.space<top>(3);

			//HACK: Force single increment over
			//huge INT_MAX range.
			frames.spinner->set_speed();
		}

		row nav;
		dropdown animation;
		button del;
		textbox fps;
		boolean loop;
		spinbox frames; 
	};
	struct shelf2_group
	{
		shelf2_group(node *main)
			:
		nav(main),
		media_nav(nav),
		play(media_nav,"Play",id_animate_play),
		stop(media_nav,"Stop",id_animate_pause),		
		timeline(nav,"Frame: 000",id_bar) 
		{	
			nav.expand();
			media_nav.space(2); timeline.expand();
		}

		row nav;
		row media_nav;
		button play,stop;
		bar timeline;
	};
	shelf1_group shelf1;
	shelf2_group shelf2;
	f1_ok_panel invisible_f1_and_close;
};

#endif  // __ANIMWIN_H_INC__
