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
#include "log.h"
#include "modelstatus.h"


#include "extrudecmd.h" //INAPPROPRIATE?

struct ExtrudeWin : Win
{
	void submit(int);

	ExtrudeWin(Model *model)
		:
	Win("Extrude"),model(model),
	nav(main,"Extrude options"),
	x(nav,"X",'X'), //"X:"
	y(nav,"Y",'Y'),
	z(nav,"Z",'Z'),
	make_back_faces(nav,"Make back faces",'?'), //"Make Back Faces"
	extrude(nav,"E&xtrude",id_apply),
	ok(main)
	{
		active_callback = &ExtrudeWin::submit;

		submit(id_init);
	}

	class Model *model;

	panel nav;
	textbox x,y,z;
	boolean make_back_faces;
	button extrude;
	f1_ok_panel ok;
		  	
	ExtrudeImpl impl; 
};
void ExtrudeWin::submit(int id)
{
	switch(id)
	{
	case id_init:

		x.edit(0.0); 
		y.edit(0.0);
		z.edit(0.0);
		make_back_faces.set(config.get("ui_extrude_makebackfaces",false));
		break;

	case '?':

		config.set("ui_extrude_makebackfaces",(bool)make_back_faces);
		break;

	case id_apply:

		model_status(model,StatusNormal,STATUSTIME_SHORT,::tr("Extrude complete"));

		impl.extrude(model,x,y,z,make_back_faces);

		model->operationComplete(::tr("Extrude","operation complete"));
		
		return; //Notice: Ok/Cancel semantics don't apply.
	}

	basic_submit(id);
}

extern void extrudewin(Model *model)
{
	ExtrudeWin(model).return_on_close();
}

