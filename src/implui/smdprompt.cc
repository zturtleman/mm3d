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

#include "smdfilter.h"

struct SmdPrompt : Win
{
	void submit(int);
	
	SmdPrompt(Model *model, SmdFilter::SmdOptions *smd)
		:
	Win("SMD Filter Options"),
	model(model),smd(smd),
	type(main,"Model Type",id_item),
	points_as_joints(main,"Save Points as Bone Joints"),
    format(main,"Vertex Format",id_subitem),
	animation(main,"Animations"),
	f1_ok_cancel(main)
	{
		type.add_item("Reference").add_item("Animation");
		type.row_pack().style(bi::etched).expand();

		format.add_item("GoldSrc (Single bone joint influence)");
		format.add_item("Source (Multiple bone joint influences)");
		format.style(bi::etched).expand();

		animation.expand();

		active_callback = &SmdPrompt::submit;

		submit(id_init);
	}

	Model *model;
	SmdFilter::SmdOptions *smd;

	multiple type;	
	boolean points_as_joints;
	multiple format;
	listbox animation; 
	f1_ok_cancel_panel f1_ok_cancel; 
};
void SmdPrompt::submit(int id)
{
	switch(id)
	{
	case id_init:

		type.select_id(!smd->m_saveMeshes);
		points_as_joints.set(smd->m_savePointsJoint);
		format.select_id(smd->m_multipleVertexInfluences);
		for(int i=0,iN=model->getAnimCount(Model::ANIMMODE_SKELETAL);i<iN;i++)
		{
			animation.add_item(i,model->getAnimName(Model::ANIMMODE_SKELETAL,i));
		}
		//break;

	case id_item:

		animation.enable(!format.enable(!type).enabled());
		break;

	case id_ok:

		smd->m_saveMeshes = !type;
		smd->m_savePointsJoint = points_as_joints;
		smd->m_multipleVertexInfluences = format&1;
		smd->m_animations.clear();
		if(!smd->m_saveMeshes)
		{
			if(!animation.empty())
			{
				smd->m_animations.push_back((int)animation);
			}
		}
		else smd->m_animations.push_back(UINT_MAX); //???
		break;
	}
	basic_submit(id);
}
 
extern bool smdprompt(Model *model, ModelFilter::Options *o)
{
	auto smd = o->getOptions<SmdFilter::SmdOptions>();
	return id_ok==SmdPrompt(model,smd).return_on_close();
}
