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


struct MetaEditWin : Win
{
	void submit(int);

	MetaEditWin(li::item *kv)
		:
	Win("Edit Meta Data"),kv(kv),
	key(main,"Name\t"),
	val(main,"Value\t"),
	ok_cancel(main)
	{
		key.expand(); val.expand();

		utf8 row[2]; kv->text().c_row(row);
		key.set_text(row[0]); val.set_text(row[1]);		

		active_callback = &MetaEditWin::submit;

		submit(id_init);
	}

	li::item *kv;

	textbox key,val; 
	ok_cancel_panel ok_cancel;
};
void MetaEditWin::submit(int id)
{
	if(id==id_ok)		
	kv->text().format(&"%s\0%s",
	key.text().c_str(),val.text().c_str());
	basic_submit(id);
}

struct MetaWin : Win
{
	void submit(int);

	MetaWin(Model *model)
		:
	Win("Model Meta Data"),
		model(model),
	table(main,"",id_item),
		header(table,id_sort),
	nav(main),
	add(nav,"New",id_new),
	del(nav,"Delete",id_delete),
	f1_ok_cancel(main)
	{
		table.expand();
		nav.ralign();

		active_callback = &MetaWin::submit;

		submit(id_init);
	}

	Model *model;

	listbox table;
	listbar header;
	row nav;
	button add,del;
	f1_ok_cancel_panel f1_ok_cancel;

	void new_item(utf8 k, utf8 v)
	{
		auto *i = new li::item; 
		i->text().format(&"%s\0%s",k,v); 
		table.add_item(i);
	}
};
void MetaWin::submit(int id)
{
	switch(id)
	{
	case id_init:

		header.add_item("Name").add_item("Value");
		for(int i=0,iN=model->getMetaDataCount();i<iN;i++)
		{	
			Model::MetaData &md = model->getMetaData(i);
			new_item(md.key.c_str(),md.value.c_str());
		}
		break;

	case id_new:

		new_item(::tr("Name","meta value key name"),::tr("Value","meta value 'value'"));
		table.outline(table.find_line_count()-1);
		break;

	case id_delete:

		table.delete_item(table.find_line_ptr());
		break;

	case id_sort:
	
		header.sort_items([&](li::item *a, li::item *b)
		{
			utf8 row1[2]; a->text().c_row(row1);
			utf8 row2[2]; b->text().c_row(row2);
			//https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92338
			return strcmp(row1[(int)header],row2[(int)header])<0;
		});
		break;

	case id_item:

		switch(event.get_click())
		{
		default: //Spacebar???
		case 2: //Double-click? 
			
			MetaEditWin(table.find_line_ptr()).return_on_close();
			break;

		case 1:

			if(event.listbox_item_rename())
			{
				textbox modal(table);
				modal.move_into_place(); modal.return_on_enter();
			}
			break;
		}		
		break;

	case id_ok:

		model->clearMetaData();
		table^[&](li::allitems ea)
		{
			utf8 row[2]; ea->text().c_row(row);
			model->addMetaData(row[0],row[1]);
		};	
		model->operationComplete(::tr("Change meta data","operation complete"));
		break;

	case id_cancel:

		model->undoCurrent();
		break;
	}

	basic_submit(id);
}

extern void metawin(Model *m){ MetaWin(m).return_on_close(); }
