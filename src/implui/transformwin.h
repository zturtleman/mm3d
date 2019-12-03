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


#ifndef __TRANSFORMWIN_H__
#define __TRANSFORMWIN_H__

#include "mm3dtypes.h" //PCH
#include "win.h"

struct TransformWin : Win
{
	void submit(control*);

	TransformWin(class MainWin &model)
		:
	Win("Transform Model"),model(model),
	tab(main,id_item),
	translate(tab),rotate(tab),scale(tab),matrix(tab),
	scope(main,"Apply to:",id_subitem),
	ok(main)
	{
		active_callback = &TransformWin::submit;

		submit(main); //id_init
	}

	class MainWin &model;

	struct translate_tab
	{
		translate_tab(node *frame)
			:
		nav(frame),
		x(nav,"X"),y(nav,"Y"),z(nav,"Z"),
		translate(nav,"Translate",id_apply)
		{
			for(textbox*c=&x;c<=&z;c++)
			c->edit(0.0);
			translate.ralign();
		}

		panel nav;
		textbox x,y,z;		
		button translate;
	};
	struct rotate_tab
	{	
		rotate_tab(node *frame)
			:
		nav(frame),
		Euler(nav,"Euler angles",false), //"Euler Angles"
		rotate1(nav,"Rotate",id_apply),
		aa(nav.inl(bi::etched)), //"Quaternion"
		rotate2(nav,"Rotate",id_apply),
		x(Euler,"X"),y(Euler,"Y"),z(Euler,"Z"),
		ax(aa),ay(aa),az(aa),angle(aa.inl,"Angle")
		{
			ax.name("Axis").place(bottom);
			for(textbox*c=&x;c<=&angle;c++)
			c->edit(0.0);
			angle.place(bottom);
			rotate1.ralign();
			rotate2.align();
		}

		panel nav;
		panel Euler;
		button rotate1;
		panel aa;
		button rotate2;
		textbox x,y,z,ax,ay,az,angle;		
	};
	struct scale_tab
	{	
		scale_tab(node *frame)
			:
		nav(frame),
		x(nav,"X"),y(nav,"Y"),z(nav,"Z"),
		scale(nav,"Scale",id_apply)
		{
			nav.calign();
			for(textbox*c=&x;c<=&z;c++)
			c->edit(1.0);
			scale.ralign();
		}

		panel nav;
		textbox x,y,z;
		button scale;
	};
	struct matrix_tab
	{	
		matrix_tab(node *frame)
			:
		nav(frame),nav2(nav),
		multiply(nav,"Apply Matrix",id_apply),
		title(nav,"(bottom row is translation)")
		{
			title.calign();
			for(int i=0;i<4;i++)
			{
				for(int j=0;j<4;j++)
				{
					textbox &mji = m[j][i];					
					mji.set_parent(nav2);
					mji.edit(i==j?1.0:0.0);
				}
				if(i<3) c[i].set_parent(nav2);
			}
			multiply.expand();
		}

		panel nav,nav2;
		textbox m[4][4];
		column c[3];		
		button multiply;
		titlebar title;
	};

	ui::tab tab;
	translate_tab translate;
	rotate_tab rotate;
	scale_tab scale;
	matrix_tab matrix;
	multiple scope;
	f1_ok_panel ok;
	
	void open(){ show(); }

	// Transform window events
	void translateEvent();
	void rotateEvent(control*);
	void rotateEulerEvent(){ rotateEvent(rotate.rotate1); }
	void rotateQuaternionEvent(){ rotateEvent(rotate.rotate2); }
	void scaleEvent();
	void matrixEvent();
	bool matrixIsUndoable(const class Matrix &m);
	bool warnNoUndo(bool undoable);
	void applyMatrix(const Matrix &m, utf8 action);
};

#endif // __TRANSFORMWIN_H__
