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
#include "msg.h"

#include "texmgr.h"
#include "texturecoord.h" //Widget

struct BackgroundWin : Win
{
	void submit(int);

	BackgroundWin(Model *model)
		:
	Win("Select Background Image"),
	model(model),
	tab(main,id_item),
	scene(tab,id_scene),
	browse_nav(tab),
	source(browse_nav,"",id_source),
	browse(browse_nav,"...",id_browse),
	f1_ok_cancel(main),
	texture(scene,false)
	{
		tab.lock(false,320);

		tab.add_item("Front").add_item("Back");
		tab.add_item("Left").add_item("Right");
		tab.add_item("Top").add_item("Bottom");
		
		browse_nav.expand().space(0);
		source.expand(); 
		browse.ralign().span(0).drop(source.drop());

		active_callback = &BackgroundWin::submit;

		submit(id_init);
	}

	Model *model;		

	ui::tab tab;
	canvas scene;
	row browse_nav; 
	textbox source;
	button browse;
	f1_ok_cancel_panel f1_ok_cancel;

	Widget texture;
};
void BackgroundWin::submit(int id)
{
	switch(id)
	{
	case id_init: 
	
		texture.setModel(model);
		//break;

	case id_item:

		source.text() = model->getBackgroundImage((int)tab);
		
		/*if(0) case id_delete:
		{
		source.text().clear();	
		}*/
		if(0) case id_browse:
		{
		std::string verb = "Open: ";
		//FIX ME 
		//"all model formats" doesn't include parentheses ???
		verb+=::tr("All Supported Formats (","all texture formats");
		verb+=TextureManager::getInstance()->getAllReadTypes();
		verb+=')';

		std::string file = config.get("ui_background_dir");
		if(file.empty()) file = ".";
		file = FileBox(file,verb,::tr("Open texture image"));
		if(file.empty()) 
		break;						
		source.text() = file;
		config.set("ui_background_dir",file,file.rfind('/'));		
		}
		//break;

	case id_source:
			
		model->setBackgroundImage((int)tab,source.c_str());
		
		if(source.text().empty())
		{
			texture.setTexture(-1,nullptr);
		}
		else if(Texture*t=TextureManager::getInstance()->getTexture(source))
		{	
			texture.setTexture(-1,t);
		}
		else
		{
			Texture::ErrorE e = TextureManager::getInstance()->getLastError();
			msg_error("%s\n%s",source.c_str(),
			e==Texture::ERROR_NONE?::tr("Could not open file"):textureErrStr(e));			
		}
		
		source.set_text(source); //Finally, set the text properly.

		break;

	case id_scene:

		texture.draw(scene.x(),scene.y(),scene.width(),scene.height());
		break;

	case id_ok:

		model->operationComplete(::tr("Background Image","operation complete"));
		break;

	case id_cancel:

		model->undoCurrent();
		break;
	}

	basic_submit(id);
}

extern void backgroundwin(Model *m){ BackgroundWin(m).return_on_close(); }
