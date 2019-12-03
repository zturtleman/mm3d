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


#ifndef __TEXTURECOORD_H__
#define __TEXTURECOORD_H__

#include "mm3dtypes.h" //PCH
#include "win.h"

#include "model.h"
#include "texwidget.h"

struct Win::Widget //YUCK
	:
TextureWidget,
TextureWidget::Parent
{
	//TODO: Expose/rethink these.
	using TextureWidget::m_vertices;
	using TextureWidget::m_triangles;	
	void move(double x, double y)
	{
		moveSelectedVertices
		(x/getUvWidth(),y/getUvHeight());
	}

	canvas &c;
	virtual void updateWidget(){ c.redraw(); }	
	Widget(canvas &c, bool border=true)
	:TextureWidget(this),c(c)
	{
		if(border)
		{
			int &p = PAD_SIZE;
			//c.space(p,0,p,p,p); p = 0;
			//c.space(-1,-1);
			c.space(1,0,1);
			p = 0;
			c.style(~0xf00);
		}
	}

	virtual void getXY(int &x, int &y)
	{
		x = c.x(x); y = c.y(y);
	}

	virtual bool mousePressSignal(int)
	{
		//Need to focus it so other controls 
		//don't consume arrow keys.
		if(c!=event.get_active()) c.activate();
		return true;
	}
};

struct TextureCoordWin : Win
{
	void init(),submit(control*);
	
	TextureCoordWin(class MainWin &model)
		:
	Win("Texture Coordinates",&texture),
	model(model),
	viewbar(main),
	white(viewbar,"",id_white),
	red(viewbar,"",id_red),
	zoom(viewbar),
	scene(main,id_scene),
	toolbar(main),
	ccw(toolbar,"Turn CCW"), //Rotate CCW
	cw(toolbar,"Turn CW"), //Rotate CW
	uflip(toolbar,"U Flip"), //H Flip
	vflip(toolbar,"V Flip"),	
		f1(main),	
	mouse(main.inl,"Mouse Tool",id_item),	
	scale_sfc(main,"Scale from center"),
	scale_kar(main,"Keep aspect ratio"),	
//		void1(main), //RESTORE US?
	pos(main,"Position",bi::none), //"Position"
	u(pos,"U",'U'),
	v(pos,"V",'V'),
	dimensions(pos,"Dimensions"),
//		void2(main), //RESTORE US?
	map(main,"Map Scheme",id_subitem),
	map_reset(main,"Reset Coordinates",id_subitem),
	ok(main),
	texture(scene)
	{
		viewbar.expand();
		pos.expand();
		u.edit(0.0).expand(); 
		v.edit(0.0).expand();
		u.compact().sspace<left>({v.compact()},false);
		dimensions.expand().place(bottom);
		map_reset.expand();

		active_callback = &TextureCoordWin::submit;

		init(),submit(main);
	}

	class MainWin &model;

	enum{ id_white=0xffffff, id_red=0xff0000 };

	row viewbar;	
	dropdown white,red;
	Win::zoom_set zoom;
	canvas scene;
	row toolbar;
	button ccw,cw,uflip,vflip;
	f1_titlebar f1;
	multiple mouse;
	boolean scale_sfc,scale_kar;
//	canvas void1;
	panel pos;
	spinbox u,v;
	textbox dimensions;
//	canvas void2;
	multiple map;
	button map_reset;
	ok_button ok;

	struct : Widget
	{
		using Widget::Widget; //C++11

		TextureCoordWin &win()
		{
			return *(TextureCoordWin*)c.ui();
		}		
		virtual void zoomLevelChangedSignal()
		{
			double z = win().texture.getZoomLevel();
			win().zoom.value.set_float_val(z);
		}
		virtual void updateCoordinatesSignal()
		{
			win().updateTextureCoordsDone(false);
		}
		virtual void updateSelectionDoneSignal()
		{
			win().updateSelectionDone();
		}
		virtual void updateCoordinatesDoneSignal()
		{
			win().updateTextureCoordsDone(true);
		}
		   
	}texture;

	virtual Widget *widget() //setInteractive?
	{
		return &texture; 
	}

	void open(); //???
	
	void setModel();
	void modelChanged(int changeBits);

protected:		
		 
	void openModel();

	void mapReset();
		
	void operationComplete(const char*);
	void updateSelectionDone();
	void updateTextureCoordsDone(bool done=true);

	void setTextureCoordsEtc(bool); //NEW

	bool m_ignoreChange;
	
	int_list trilist; //NEW: Reuse buffer.

	double centerpoint[2]; //UV textboxes.
};

#endif //__TEXTURECOORD_H__
