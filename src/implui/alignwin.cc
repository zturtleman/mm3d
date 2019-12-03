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

//FIX ME
//https://github.com/zturtleman/mm3d/issues/68
//#include "align.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"

		
struct AlignWin : Win
{	
	void submit(int);

	AlignWin(Model *model)
	    :
	Win("Align Selection"),
	model(model),
	nav(main),
	x(nav,"Align &X Now"),
	y(nav,"Align &Y Now"),
	z(nav,"Align &Z Now"),f1_ok_cancel(main)
	{
		active_callback = &AlignWin::submit;
	}

	Model *model;

	struct align_group
	{
		align_group(node *main, const char (&l)[13])
			:
		nav(main,""),mult(nav),
		value(nav),apply(nav,l,l[7])
		{	
			nav.name().assign(l,8);
			mult.add_item("Align minimum").add_item("Align center");
			mult.add_item("Align maximum").set_int_val(1); 
			value.edit(0.0).expand();
		}
					
		panel nav;
		multiple mult;
		textbox value; 
		button apply;		
	};

	row nav;
	align_group x,y,z; 
	f1_ok_cancel_panel f1_ok_cancel;
};
void AlignWin::submit(int i)
{	
	switch(i)
	{
	case 'X': case 'Y': case 'Z':
	{
		align_group *g = &x+i-'X';		
		log_debug("aligning %c on %s\n",i,g->value.c_str());
		extern void align_selected(int,Model*,int,double); //align.cc
		align_selected(i,model,g->mult,g->value);

		//Note: This is like an Apply button.
		//https://github.com/zturtleman/mm3d/issues/90
		//DecalManager::getInstance()->modelUpdated(model); //???
		model->updateObservers();

		model_status(model,StatusNormal,STATUSTIME_SHORT,::tr("Align %c"),i);
		return;
	}
	case id_ok:
		
		log_debug("Alignment complete\n");
		model->operationComplete(::tr("Align Selected","operation complete"));		
		break;

	case id_cancel:

		log_debug("Alignment canceled\n");
		model->undoCurrent();
		break;
	}
	basic_submit(i);
}

extern void alignwin(Model *model)
{
	AlignWin(model).return_on_close();
}

