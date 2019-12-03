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
#include "log.h"
#include "misc.h"
#include "msg.h"

#include "filedatadest.h"

struct PaintTextureWin : Win
{
	void submit(int);

	PaintTextureWin(Model *model)
		:
	Win("Paint Texture"),
	model(model),
	shelf1(main),
	polygons(shelf1,"Polygons:"),
	vertices(shelf1,"Vertices"),
	shelf2(main),
	width(shelf2,"Save Size:",'X'),width_v(width),
	height(shelf2,"x",'Y'),height_v(height),
	save(shelf2,"Save...",id_browse), //"Save Texture..."
	ok(main),

	//2019: Putting image on bottom so if it's larger
	//than the screen the interface is not off screen.

	scene(main,id_scene),texture(scene)
	{
		scene.space(1,1,4,1);

		//HACK?
		//DON'T USE canvas'S SPECIAL ALIGNMENT MODE
		//SINCE IT EXPANDS. COULD USE lock TOO, BUT
		//CANVASES assert ON lock UNTIL IMPLEMENTED.
		scene.calign(); 

		shelf2.expand(); save.ralign();		

		active_callback = &PaintTextureWin::submit;

		submit(id_init);
	}

	Model *model;

	row shelf1;
	dropdown polygons,vertices;
	row shelf2;
	textbox width,height;
	dropdown width_v,height_v;
	button save;
	f1_ok_panel ok;
	canvas scene;

	Widget texture;
};
void PaintTextureWin::submit(int id)
{
	if(id!=id_browse) switch(id)
	{
	case id_init:
	{
		texture.setModel(model);
		texture.setSolidBackground(true);

		polygons.select_id(3).add_item(1,"Edges") //DM_Edges
		.add_item(2,"Filled").add_item(3,"Filled and Edges");

		vertices.add_item(0,"Hidden").add_item(1,"Visible");
		
		width.edit(64,512,8192).update_area();
		height.edit(64,512,8192).compact(width.active_area<0>(-2));
		for(int x=1024;x>=64;x/=2)
		{
			auto i = new li::item(x);
			i->text().format("%d",x); width_v.add_item(i);
		}
		height_v.reference(width_v);

		int_list l;
		int_list::iterator it,itt;
		model->getSelectedTriangles(l);
		itt = l.end();

		int material = -1;

		//FIX ME (LOOKS BOGUS)
		//https://github.com/zturtleman/mm3d/issues/54
		for(it=l.begin();it<itt;it++)
		{
			//FIX ME: getTriangleGroup is dumb!!
			int g = model->getTriangleGroup(/*l.front()*/*it); //???
			int m = model->getGroupTextureId(g);
			if(m>=0)
			{
				texture.setTexture(m);
				material = m;
				//break;
			}
			break; //getTriangleGroup workaround.
		}
		if(material==-1)
		{
			log_error("no group selected\n");
		}

		//addTriangles();
		{
			texture.clearCoordinates();
			
			int v = 0; float s,t;
			for(it=l.begin();it<itt;it++,v+=3)
			{
				for(int i=0;i<3;i++)
				{
					model->getTextureCoords(*it,i,s,t);
					texture.addVertex(s,t);					
				}
				texture.addTriangle(v,v+1,v+2);
			}
		}

		if(material<0)
		{
			texture.setTexture(-1);
		}
		else if(Texture*tex=model->getTextureData(material))		
		{
			//int x = 64; while(x<tex->m_width) x*=2;
			//int y = 64; while(y<tex->m_height) y*=2;
			//width.select_id(x); height.select_id(y);
			width.set_int_val(tex->m_width);
			height.set_int_val(tex->m_height);
		}

		//break;
	}
	case 'X': case 'Y':
	{	
		int w = width, h = height;
		scene.offset_dims(&w,&h); 
		scene.lock(w,h);
		//break;
	}
	default:
	
		texture.setDrawMode((Widget::DrawModeE)polygons.int_val());
		
		//FIX ME
		//TextureWidget::draw artificially couples 
		//MouseRange to m_drawVertices.
		texture.setMouseOperation(Widget::MouseRange);
		texture.setDrawVertices(vertices&1);

		texture.sizeOverride(width,height);
		texture.updateWidget();
		//break;

	case id_ok: return basic_submit(id);

	case id_scene:

		texture.draw(scene.x(),scene.y(),scene.width(),scene.height());
		return;
	}

	const char *modelFile = model->getFilename();
	FileBox file = config.get("ui_model_dir");
	if(*modelFile) //???
	{
		std::string fullname,fullpath,basename; //REMOVE US
		normalizePath(modelFile,fullname,fullpath,basename);
		file = fullpath.c_str();
	}
		
	file.locate("Save PNG",::tr("File name for saved texture?"),true);
	if(file.empty()) 	
	return log_debug("save frame buffer canceled\n");

	int x,y,w,h; texture.getGeometry(x,y,w,h); 
	int il = glutext::glutCreateImageList();
	int*pb =*glutext::glutLoadImageList(il,w,h,false);

	file.push_back('\0');
	size_t buf = file.size();

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	if(!pb) assert(pb);
	else glReadPixels(x,y,pb[0],pb[1],pb[2],pb[3],(void*&)pb[4]);

	if(!file.render(il,"PNG")
	||!FileDataDest(file.c_str()).writeBytes(&file[buf],file.size()-buf))
	{
		msg_error("%s\n%s",::tr("Could not write file: "),file.c_str());
	}

	glutext::glutDestroyImageList(il);
}
extern void painttexturewin(Model *model)
{
	if(!model->getSelectedTriangleCount())
	{
		msg_info(::tr("You must select faces first.\n"
		"Use the 'Select Faces' tool.",
		"Notice that user must have faces selected to open 'paint texture' window"));
	}
	else PaintTextureWin(model).return_on_close(); 
}