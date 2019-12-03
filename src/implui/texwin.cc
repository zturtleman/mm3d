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
#include "texturecoord.h" //Widget
#include "texture.h"
#include "texmgr.h"
#include "log.h"

#include "msg.h"

struct TextureWin : Win
{
	void submit(int);

	TextureWin(Model *model)
		:
	Win("Materials"),
	model(model),
	material(main,"",id_item),
	nav(main),	
	add(nav,"New",id_new), //"New Material..."
	name(nav,"Rename",id_name),
	del(nav,"Delete",id_delete),	
	column2(nav,"",'2'),
	column_nav(main),
	//Happens to be 100 x 100.
	scene(column_nav,id_scene),	
	column1(column_nav,"",'1'),			
	r(column_nav.inl,"Red\t",-1,+1,0.0),
	g(column_nav,"Green\t",-1,+1,0.0),
	b(column_nav,"Blue\t",-1,+1,0.0),
	a(column_nav,"Alpha\t",-1,+1,0.0),
	void1(column_nav),
	wrap_nav(column_nav),
	wrap_x(wrap_nav,"",'X'),
	wrap_y(wrap_nav,"",'Y'),
	browse_nav(main),
	source(browse_nav,"",id_source),
	browse(browse_nav,"...",id_browse),
	f1_ok_cancel(main),
	texture(scene,false)
	{	
		//This happens to be its size.
		scene.lock(100,100);

		//HACK: Lining up picture with
		//scrollbar bottoms.
		void1.space<top>(1);

		nav.expand();
		column2.ralign().space<top>(5);

		browse_nav.expand().space(0);
		source.expand();
		browse.ralign().span(0).drop(source.drop());

		active_callback = &TextureWin::submit;

		submit(id_init);
	}
		
	Model *model;
	
	dropdown material;
	row nav;
	button add,name,del;
	panel column_nav;	
	canvas scene;
	row browse_nav;
	textbox source; button browse;
	dropdown column1;
	dropdown column2;
	slider_value r,g,b,a;
	canvas void1;
	row wrap_nav;
	dropdown wrap_x,wrap_y;
	f1_ok_cancel_panel f1_ok_cancel;

	Widget texture;

	void material_selected();
	void source_texture(int);
	void new_material_or_name(int);
	void refresh_column2(int='2');
};

extern void texwin(Model *m)
{
	TextureWin(m).return_on_close(); 
}

void TextureWin::submit(int id)
{
	int m = material; switch(id)
	{
	case id_init:
	{
		column_nav.lock(false,true);

		texture.setModel(model);

		material.expand();

		int iN = model->getTextureCount();
		for(int i=0;i<iN;i++)				
		material.add_item(i,model->getTextureName(i));

		wrap_x.add_item(0,"Wrap X").add_item(1,"Clamp X");
		wrap_y.add_item(0,"Wrap Y").add_item(1,"Clamp Y");
		
		column1.select_id(config.get("ui_texwin_preview_index",false))
		.add_item(false,"Flat Preview")
		.add_item(true,"3D Preview");
		texture.set3d((bool)column1); 
		
		column2.select_id(1) // Diffuse
		.add_item(0,"Ambient").add_item(1,"Diffuse")
		.add_item(2,"Specular").add_item(3,"Emissive")
		.add_item(4,"Shininess");

		int_list l; model->getSelectedTriangles(l);
		for(int_list::iterator it=l.begin(),itt=l.end();it<itt;it++)	
		if(int g=model->getTriangleGroup(*it)+1)
		{
			material.select_id(model->getGroupTextureId(g-1));
			break;
		}
		//break;
	}
	case id_item:

		material_selected();		
		break;
	
	case '1': 
		
		texture.set3d((bool)column1); 
		config.set("ui_texwin_preview_index",(bool)column1);		
		break;

	case '2': case id_value: 
		
		refresh_column2(id); break;

	case id_new: case id_name:

		new_material_or_name(id);
		break;

	case id_delete:
	
		model->deleteTexture(m);
		material.delete_item(m);
		material.select_id(0);
		material_selected();
		break;

	case id_source: case id_browse:

		source_texture(id); break;

	case 'X':
		
		model->setTextureSClamp(m,wrap_x&1);
		texture.setSClamp(wrap_x&1);
		break;
		
	case 'Y': 
		
		model->setTextureTClamp(m,wrap_y&1);
		texture.setTClamp(wrap_y&1);
		break;

	case id_scene:

		texture.draw(scene.x(),scene.y(),scene.width(),scene.height());
		break;

	case id_ok:
		
		model->operationComplete(::tr("Texture changes"));		
		break;

	case id_cancel:

		model->undoCurrent();
		break;
	}

	basic_submit(id);
}

void TextureWin::source_texture(int id)
{
	if(id==id_browse)
	{
		std::string verb = "Open: ";
		//"all model formats" doesn't include parentheses ???
		//verb.append(::tr("All Supported Formats (")); //Parenthesis?
		verb+=::tr("All Supported Formats (","all texture formats");
		verb+=TextureManager::getInstance()->getAllReadTypes();
		verb+=')';

		std::string file = config.get("ui_texture_dir");
		if(file.empty()) file = "."; 
		file = FileBox(file,verb,::tr("Open texture image"));

		if(file.empty()) return;

		source.set_text(file); 
		config.set("ui_texture_dir",file,file.rfind('/'));
	}

	int m = material;

	//NOTE: Explicit removal.
	if(source.text().empty())
	{
		model->removeMaterialTexture(m);
		log_debug("removed texture from material %d\n",m); //???
		material_selected();
	}
	else if(Texture*tex=TextureManager::getInstance()->getTexture(source.c_str()))
	{
		model->setMaterialTexture(m,tex);
		log_debug("changed texture %d to %s\n",m,source.c_str());
		texture.setTexture(m);
	}
	else
	{
		Texture::ErrorE e = TextureManager::getInstance()->getLastError();
		utf8 err = e==Texture::ERROR_NONE?::tr("Could not open file"):textureErrStr(e);
		msg_error("%s\n%s",source.c_str(),err);
	}
}

void TextureWin::new_material_or_name(int id)
{
	utf8 title = "Rename material";
	std::string name; int m; if(id==id_name)
	{
		title = "New material";
		m = material; name = model->getTextureName(m);		
	}
	
	if(id_ok==EditBox(&name,
	::tr(title,"window title"),::tr("Enter new material name:")))
	{
		if(id==id_name)
		{	
			model->setTextureName(m,name.c_str());
			material.selection()->text() = name;		
		}
		else if(id==id_new)
		{
			if(material.empty()) enable();

			m = model->addColorMaterial(name.c_str());
			material.add_item(m,name);
			material.select_id(m);
			material_selected();
			log_debug("added %s as %d\n",name.c_str(),m); //???
		}
		else assert(0);
	}
}

void TextureWin::material_selected()
{
	int m; if(material.empty())
	{
		disable();
		add.enable();
		f1_ok_cancel.nav.enable();

		m = -1;
	}
	else
	{
		m = material; 
		
		refresh_column2();

		texture.setSClamp(1&wrap_x.select_id(model->getTextureSClamp(m)));
		texture.setTClamp(1&wrap_y.select_id(model->getTextureTClamp(m)));
	}	
	texture.setTexture(m);
	source.set_text(model->getTextureFilename(m));
}

void TextureWin::refresh_column2(int id)
{
	int m = material; if(id=='2')
	{
		float val[4] = {};

		bool err = false; switch(column2)
		{
		case 0: err = model->getTextureAmbient(m,val); break;
		case 1: err = model->getTextureDiffuse(m,val); break;
		case 2: err = model->getTextureSpecular(m,val); break;
		case 3: err = model->getTextureEmissive(m,val); break;
		case 4: err = model->getTextureShininess(m,val[0]); break;
		}
		if(!err)
		log_error("could not get lighting values for %d\n",m); //???

		bool hide = 4==column2.int_val();
		if(hide) val[0]/=100;
		r.init(::tr(hide?"Shininess":"Red"),hide?0:-1,1,val[0]);
		r.value.space<0>(g.value.space<0>()); //HACK
		for(int i=hide?4:1;i<4;i++) (&r)[i].set(val[i]);
		for(int i=1;i<4;i++) (&r)[i].nav.set_hidden(hide);
	}
	else //id_value
	{
		assert(id_value==id);

		float val[4] = { r.value,g.value,b.value,a.value };

		switch(column2)
		{
		case 0: model->setTextureAmbient(m,val); break;
		case 1: model->setTextureDiffuse(m,val); break;
		case 2: model->setTextureSpecular(m,val); break;
		case 3: model->setTextureEmissive(m,val); break;
		case 4: model->setTextureShininess(m,val[0]*100); break;
		}
		//https://github.com/zturtleman/mm3d/issues/90
		//DecalManager::getInstance()->modelUpdated(model); //???
		model->updateObservers();

		texture.updateWidget();
	}
}
