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

#include "pluginmgr.h"

struct PluginWin : Win
{
	void submit(int);

	PluginWin()
		:
	Win("Plugins"),
	table(main),header(table,id_sort),
	c1(header,"Plugin"),
	c2(header,"Version"),
	c3(header,"Description"),
	c4(header,"Status"),
	ok(main)
	{
		c1.span()*=3; c3.span()*=4;

		active_callback = &PluginWin::submit;

		submit(id_init);
	}

	listbox table;
	listbar header;
	listbar::item c1,c2,c3,c4;
	f1_ok_panel ok;
};
void PluginWin::submit(int id)
{
	if(id==id_init)
	{
		PluginManager *pmgr = PluginManager::getInstance();

		int_list plist = pmgr->getPluginIds();
		int_list::iterator it;
		for(it=plist.begin();it!=plist.end();it++)
		{
			//FIX THIS IS CRAP
			utf8 name = pmgr->getPluginName(*it);
			utf8 vers = pmgr->getPluginVersion(*it);
			utf8 desc = pmgr->getPluginDescription(*it);
			utf8 stat = "Unknown status code";
			switch(pmgr->getPluginStatus(*it))
			{
			case PluginManager::PluginActive:
			stat = "Active"; break;
			case PluginManager::PluginUserDisabled:
			stat = "Disabled by user"; break;
			case PluginManager::PluginVersionDisabled:
			stat = "Disabled (incompatible version)"; break;
			case PluginManager::PluginNotPlugin: //???
			stat = "Not a plugin"; break;
			case PluginManager::PluginError:
			stat = "Error"; break;
			}
			auto *i = new li::item(*it);
			i->text().format(&"%s\0%s\0%s\0%s",name,vers,desc,stat);
			table.add_item(i);
		}
	}
	else if(id==id_sort)
	{
		header.sort_items([&](li::item *a, li::item *b)
		{
			utf8 row1[4]; a->text().c_row(row1);
			utf8 row2[4]; b->text().c_row(row2);
			//https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92338
			return strcmp(row1[(int)header],row2[(int)header])<0;
		});
	}
	basic_submit(id);

}
extern void pluginwin(){ PluginWin().return_on_close(); }
