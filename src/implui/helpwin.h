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


#ifndef __HELPWIN_H__
#define __HELPWIN_H__

#include "sysconf.h"
#include "mm3dtypes.h" //PCH
#include "win.h"

struct HelpWin : Win
{
	void submit(int id)
	{
		switch(id)
		{
		case id_item:

			backward.enable(html.backwards());
			forward.enable(html.forwards());
			glutSetWindowTitle(html.name().c_str());
			break;

		case '<': html.set_url("<"); break;
		case '>': html.set_url(">"); break;
		case '^': html.set_url(getDocDirectory()+"/olh_index.html"); break;
		}
		return basic_submit(id);
	}

	HelpWin(utf8 document="index", utf8 ext=".html", utf8 prefix="/olh_")
		:
	Win("Help"),
	html(main,"",id_item),
	nav(main),
	backward(nav,"Back",'<'),
	forward(nav,"Forward",'>'),
	contents(nav,"Contents",'^'),
	ok(nav,"OK",id_ok)
	{
		ok.ralign(); //html_special_button

		//"tab" (tabular) is removing the 
		//vertical 3px spacing.
		main->space(1,0,1,1,1);
		html.style(bi::shadow).pos("tab");
		html.space<bottom>(1);
		nav.style(bi::shadow).pos("tab");
		nav.space(1).expand();

		std::string source = getDocDirectory();

		source.append(prefix);
		size_t i = source.size();
		source.append(document);
		for(;i<source.size();i++) 
		source[i] = (char)tolower(source[i]);
		source.append(ext);

		special_callback = html_special;
		keyboard_callback = html_keyboard;
		active_callback = &HelpWin::submit;
		
		//submit(id_init);
		html.set_url(source).measure(640,480);
	}

	//This is so you don't have to click to start
	//using keyboard.
	struct html_special_button : button
	{
		using button::button; //C++11

		virtual bool special_handler(int k, int m)
		{
			if(k==GLUT_KEY_UP||k==GLUT_KEY_DOWN)
			return true;
			return button::_special_handler(k,m);
		}
	};
	static bool html_special(ui *w, int k, int m)
	{
		switch(k)
		{
		case GLUT_KEY_UP: case GLUT_KEY_DOWN:
		case GLUT_KEY_HOME: case GLUT_KEY_END:
		case GLUT_KEY_PAGE_UP: case GLUT_KEY_PAGE_DOWN:
		return ((HelpWin*)w)->html.scrollbar->special_handler(k,m);
		default: return basic_special(w,k,m);
		}
	}
	static bool html_keyboard(ui *w, int k, int m)
	{
		switch(k)
		{
		case '\b': return ((HelpWin*)w)->html.set_url(m?">":"<");
		}
		return basic_keyboard(w,k,m);
	}

	ui::html html;
	row nav;
    html_special_button backward,forward,contents;    
    html_special_button ok;
};

#endif // __HELPWIN_H__
