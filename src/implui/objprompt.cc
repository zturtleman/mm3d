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
#include "objfilter.h"

struct ObjPrompt : Win
{
	void submit(int);

	ObjPrompt(Model *model, ObjFilter::ObjOptions *obj)
		:
	Win("OBJ Filter Options"),
	model(model),obj(obj),
	normals(main,"&Save Normals"),
	decimal_places(main),f1_ok_cancel(main)
	{
		normals.set();

		active_callback = &ObjPrompt::submit;

		submit(id_init);
	}

	Model *model; ObjFilter::ObjOptions *obj;

	struct decimal_group
	{
		decimal_group(node *frame)
			:
		vertex(frame,"&Vertex Decimal Places\t"),
		texture(frame,"&Texture Decimal Places\t"),
		normal(frame,"&Normal Decimal Places\t")
		{
			
			vertex.sspace<left>({texture,normal});
		}

		struct value : spinbox 
		{
			value(node *frame, utf8 name)
				:
			spinbox(frame,name)
			{
				edit(6).limit(1,6).expand();
			}
		};

		value vertex,texture,normal;
	};

	boolean normals;
	decimal_group decimal_places;
	f1_ok_cancel_panel f1_ok_cancel;
};
void ObjPrompt::submit(int id)
{
	switch(id)
	{
	case id_init:

		normals.set(obj->m_saveNormals);
		decimal_places.vertex.set_int_val(obj->m_places);
		decimal_places.texture.set_int_val(obj->m_texPlaces);
		decimal_places.normal.set_int_val(obj->m_normalPlaces);
		break;

	case id_ok:

		obj->m_saveNormals = normals;
		obj->m_places = decimal_places.vertex;
		obj->m_texPlaces = decimal_places.texture;
		obj->m_normalPlaces = decimal_places.normal;
		break;
	}
	basic_submit(id);
}
 
extern bool objprompt(Model *model, ModelFilter::Options *o)
{
	auto obj = o->getOptions<ObjFilter::ObjOptions>();
	return obj&&id_ok==ObjPrompt(model,obj).return_on_close();
}

