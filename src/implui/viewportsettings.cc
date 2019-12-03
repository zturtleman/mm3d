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

struct ViewportSettings : Win
{	
	void submit(int);

	ViewportSettings(Model *model)
		:
	Win("Viewport Settings"),model(model),
	ortho(main),persp(main),f1_ok_cancel(main)
	{
		active_callback = &ViewportSettings::submit;

		submit(id_init);
	}

	Model *model;

	struct ortho_group
	{	
		ortho_group(node *main)
			:
		nav(main,"Canvas Grid"),
		unit(nav,"Default Grid Unit\t"),
		mult(nav)
		{
			//unit.edit(4.0);
			//mult
			//.add_item("Binary Grid")
			//.add_item("Decimal Grid")
			//.add_item("Fixed Grid");
		}

		panel nav;
		textbox unit; multiple mult;
	};
	struct persp_group
	{
		persp_group(node *main)
			:
		nav(main,"3D Grid"),
		unit(nav,"Default Grid Unit\t"),
		lines(nav,"Grid Lines\t"),
		xy(nav,"X/Y Plane"),
		xz(nav,"X/Z Plane"),
		yz(nav,"Y/Z Plane")
		{
			//unit.edit(4.0); lines.edit(6);
			//xz.set();
		}

		panel nav;
		textbox unit,lines;
		boolean xy,xz,yz;
	};
	ortho_group ortho;
	persp_group persp;
	f1_ok_cancel_panel f1_ok_cancel;	
};
void ViewportSettings::submit(int i)
{
	Model::ViewportUnits &vu = model->getViewportUnits();

	switch(i)
	{
	case id_init:

		/*
		ortho.unit.edit(config.get("ui_grid_inc",4.0));
		ortho.mult.select_id(config.get("ui_grid_mode",0))
		.add_item("Binary Grid")
		.add_item("Decimal Grid")
		.add_item("Fixed Grid");
		persp.unit.edit(config.get("ui_3dgrid_inc",4.0));
		persp.lines.edit(config.get("ui_3dgrid_count",6));
		persp.xy.set(config.get("ui_3dgrid_xy",false));
		persp.xz.set(config.get("ui_3dgrid_xz",true));
		persp.yz.set(config.get("ui_3dgrid_yz",false));*/		
		ortho.unit.edit(0.00001,vu.inc,100000.0);
		ortho.mult.select_id(vu.grid)
		.add_item("Binary Grid")
		.add_item("Decimal Grid")
		.add_item("Fixed Grid");
		persp.unit.edit(0.00001,vu.inc3d,100000.0);
		persp.lines.edit(1,vu.lines3d,1000);
		persp.xy.set(vu.xyz3d&4);
		persp.xz.set(vu.xyz3d&2);
		persp.yz.set(vu.xyz3d&1);
		break;

	case id_ok:

		config.set("ui_grid_inc",(double)ortho.unit);
		config.set("ui_grid_mode",(int)ortho.mult);
		config.set("ui_3dgrid_inc",(float)persp.unit);
		config.set("ui_3dgrid_count",(int)persp.lines);
		config.set("ui_3dgrid_xy",(bool)persp.xy);
		config.set("ui_3dgrid_xz",(bool)persp.xz);
		config.set("ui_3dgrid_yz",(bool)persp.yz);
		vu.inc = ortho.unit;
		vu.grid = ortho.mult;
		vu.inc3d = persp.unit;
		vu.lines3d = persp.lines;
		vu.xyz3d = 0;
		if(persp.xy) vu.xyz3d|=4;
		if(persp.xz) vu.xyz3d|=2;
		if(persp.yz) vu.xyz3d|=1;
		break;
	}

	basic_submit(i);
}
extern void viewportsettings(Model *model)
{
	ViewportSettings(model).return_on_close(); 
}

