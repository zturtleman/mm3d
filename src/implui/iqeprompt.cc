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
#include "iqefilter.h"

struct IqePrompt : Win
{
	void submit(int);

	IqePrompt(Model *model, IqeFilter::IqeOptions *iqe)
		:
	Win("IQE Filter Options"),
	model(model),iqe(iqe),
	a(main,"Save Meshes"),
    b(main,"Save Points as Bone Joints"),
    c(main,"Save Points in Animations"),
	d(main,"Save Skeleton"),
	e(main,"Save Animations"),
	animation(main),
	f1_ok_cancel(main)
	{
		animation.expand();

		active_callback = &IqePrompt::submit;

		submit(id_init);
	}

	Model *model;
	IqeFilter::IqeOptions *iqe;

	boolean a,b,c,d,e;
	listbox animation; 
	f1_ok_cancel_panel f1_ok_cancel; 
};
void IqePrompt::submit(int id)
{
	switch(id)
	{
	case id_init:

		//NOTE: THESE ARE ALWAYS true.
		a.set(iqe->m_saveMeshes);
		b.set(iqe->m_savePointsJoint);
		c.set(iqe->m_savePointsAnim);
		d.set(iqe->m_saveSkeleton);
		e.set(iqe->m_saveAnimations);
		for(int i=0,iN=model->getAnimCount(Model::ANIMMODE_SKELETAL);i<iN;i++)
		{
			auto *ii = new multisel_item(i,model->getAnimName(Model::ANIMMODE_SKELETAL,i));
			animation.add_item(ii->select());
		}
		break;

	case id_ok:

		iqe->m_saveMeshes = a;
		iqe->m_savePointsJoint = b;
		iqe->m_savePointsAnim = c;
		iqe->m_saveSkeleton = d;
		iqe->m_saveAnimations = e;
		iqe->m_animations.clear();
		animation^[&](li::multisel ea)
		{
			iqe->m_animations.push_back(ea->id());
		};
		break;
	}

	basic_submit(id);
}
extern bool iqeprompt(Model *model, ModelFilter::Options *o)
{
	auto iqe = o->getOptions<IqeFilter::IqeOptions>();
	return iqe&&id_ok==IqePrompt(model,iqe).return_on_close();
}
