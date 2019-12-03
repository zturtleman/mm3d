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


struct MergeWin : Win
{
	void submit(control*);

	MergeWin(Model *model, Model *merge)
		:
	Win("Merge Model"),model(model),merge(merge),
	transformation_nav(main,"Merge Location"),
	rotation(transformation_nav,"Rotation\t"),
	translation(transformation_nav,"Translation\t"),
	opts_nav(main),
	incl_nav(opts_nav,"Merge Options"),
	texture(incl_nav,"Include textures"),
	animate(incl_nav,"Include animations"),
	animate2(opts_nav,"Animation Options"),
	f1_ok_cancel(main)
	{
		opts_nav.proportion();
		incl_nav.expand();
		texture.set();
		animate.set();
		animate2.expand();
		animate2.style(bi::etched);
		animate2.add_item(Model::AM_ADD,"Append animations");
		animate2.add_item(Model::AM_MERGE,"Merge if possible");

		active_callback = &MergeWin::submit;

		submit(main); //id_init
	}

	Model *model,*merge;

	struct transformation_group
	{
		transformation_group(node *frame, utf8 name)
			:
		nav(frame),
		label(nav,name),x(nav),y(nav),z(nav)
		{
			nav.space(2).ralign();

			label.space<top>(2).space<right>(4);

			x.edit(0.0); y.edit(0.0); z.edit(0.0);
		}

		row nav; titlebar label; textbox x,y,z;
	};
	panel transformation_nav;
	transformation_group rotation,translation;
	row opts_nav;
	panel incl_nav;
	boolean texture,animate;
	multiple animate2;
	f1_ok_cancel_panel f1_ok_cancel;
};
void MergeWin::submit(control *c)
{
	if(c==animate)
	{
		animate2.enable(animate);
	}
	else if(c==f1_ok_cancel.ok_cancel.ok)
	{
		Model::AnimationMergeE mode = Model::AM_NONE;
		if(animate)
		mode = Model::AnimationMergeE(animate2.int_val());
		double rot[3] = { rotation.x,rotation.y,rotation.z };
		double trans[3] = { translation.x,translation.y,translation.z };		
		model->mergeModels(merge,texture,mode,true,trans,rot);
		//model->operationComplete(::tr("Merge models"));				
		model->operationComplete(::tr("Merge models","operation complete"));
	}
	basic_submit(c->id());
}

extern void mergewin(Model *model, Model *merge)
{
	MergeWin(model,merge).return_on_close();
}
