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

#include "ms3dfilter.h"

struct Ms3dPrompt : Win
{
	void submit(int);
	
	Ms3dPrompt(Model *model, Ms3dFilter::Ms3dOptions *ms3d)
		:
	Win("MS3D Filter Options"),
	model(model),ms3d(ms3d),
	vformat(main,"Vertex Format",id_item),
	extra_nav(main,"Subversion Options"),
	extra1(extra_nav,"Vertex Extra\t"),
	extra2(extra_nav,"Vertex Extra 2\t"),
	f1_ok_cancel(main)
	{
		vformat.style(bi::etched);
		vformat.add_item("Subversion 0 (Single bone joint influence)")
		.add_item("Subversion 1 (Multiple bone joints influences, weight scale 255)")
        .add_item("Subversion 2 (Multiple bone joints influences, weight scale 100)")
        .add_item("Subversion 3 (Multiple bone joints influences, weight scale 100)");

		extra1.expand().sspace<left>({extra2});
		extra2.expand();

		active_callback = &Ms3dPrompt::submit;

		submit(id_init);
	}

	Model *model; Ms3dFilter::Ms3dOptions *ms3d;

	multiple vformat;
	panel extra_nav; textbox extra1,extra2;
	f1_ok_cancel_panel f1_ok_cancel;
};
void Ms3dPrompt::submit(int id)
{
	switch(id)
	{
	case id_init:

		vformat.select_id(ms3d->m_subVersion);
		extra1.text().format("%X",ms3d->m_vertexExtra);
		extra2.text().format("%X",ms3d->m_vertexExtra2);
		//break;

	case id_item:

		extra1.enable(vformat.int_val()>=2);
		extra2.enable(vformat.int_val()>=3);	
		break;

	case id_ok:

		ms3d->m_subVersion = vformat;
		sscanf(extra1,"%X",&(ms3d->m_vertexExtra=0xffffffff));
		sscanf(extra2,"%X",&(ms3d->m_vertexExtra2=0xffffffff));
		break;
	}
	basic_submit(id);
}

extern bool ms3dprompt(Model *model, ModelFilter::Options *o)
{
	auto ms3d = o->getOptions<Ms3dFilter::Ms3dOptions>();
	return id_ok==Ms3dPrompt(model,ms3d).return_on_close();
}

