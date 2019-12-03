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

#include "texturecoord.h"

#include "viewwin.h"
#include "log.h"

struct MapDirectionWin : Win
{
	MapDirectionWin(int *lv)
		:
	Win("Which direction?"),
	dir(main,"Set new texture coordinates from which direction?",lv),
	ok_cancel(main)
	{
		//MAGIC NUMBER
		dir.add_item("Front").add_item("Back");
		dir.add_item("Left").add_item("Right");
		dir.add_item("Top").add_item("Bottom");
	}

	multiple dir;
	ok_cancel_panel ok_cancel;
};

static int texturecoord_keyboard_accel[4] = {};
static bool texturecoord_keyboard(Widgets95::ui *ui, int k, int m)
{
	auto *w = (TextureCoordWin*)ui;

	if((unsigned)k<256) //YUCK
	{
		enum{ n=5 };
		//REMOVE ME (ELSEWHERE)
		//NOTE: Currently not invalidating.
		if(!texturecoord_keyboard_accel[0])
		{  
			utf8 kc[n] = {"tool_select_vertices",
			"tool_move","tool_rotate","tool_scale",
			"tool_select_faces"};
			for(int i=0;i<n;i++)
			{
				std::string &p = keycfg.get(kc[i]);
				for(char&ea:p) ea = toupper(ea);

				int kk = 0, mm = 0; if(!p.empty())
				{
					size_t sep = p.rfind('+');
					sep = ~sep?0:sep+1;
					if(sep!=p.size()-1)
					{
						//TODO: Symbol lookup?
					}
					else kk = tolower(p.back()); 

					if(~p.find("ALT")) mm|=GLUT_ACTIVE_ALT;
					if(~p.find("SHIFT")) mm|=GLUT_ACTIVE_SHIFT;
					if(~p.find('C'))
					if(~p.find("CTRL")
					 ||~p.find("CONTROL")
					 ||~p.find("CMD")
					 ||~p.find("COMMMAND")) mm|=GLUT_ACTIVE_CTRL;
				}
				texturecoord_keyboard_accel[i] = kk?kk|mm<<8:-1;
			}
		}
		int km = k|m<<8;
		for(int i=0;i<n;i++) 
		if(km==texturecoord_keyboard_accel[i]) 
		{
			switch(i) 
			{
			case 0: i = TextureWidget::MouseSelect; break;
			case 1: i = TextureWidget::MouseMove; break;
			case 2: i = TextureWidget::MouseRotate; break;
			case 3: i = TextureWidget::MouseScale; break;

			case 4: //HACK: Let F be used from this window.
				
				glutSetWindow(w->model.glut_window_id);				
				extern void viewwin_toolboxfunc(int);
				viewwin_toolboxfunc(1); //tool_select_faces?
				return false;
			}			
			w->submit(w->mouse.select_id(i));
			return false;
		}
	}

	return Win::basic_keyboard(ui,k,m);
}

void TextureCoordWin::open()
{
	setModel();
	
	// If visible, setModel already did this
	if(hidden())
	{
		//REMINDER: GLX needs to show before
		//it can use OpenGL.
		show(); openModel();
	}
}
void TextureCoordWin::setModel()
{
	texture.setModel(model);

	if(!hidden()) openModel();
}
void TextureCoordWin::modelChanged(int changeBits)
{
	if(changeBits&Model::SelectionFaces
	 ||changeBits&Model::MoveGeometry&&!m_ignoreChange)
	{		
		if(!hidden()) openModel();
	}
}
void TextureCoordWin::openModel()
{
	  ////POINT-OF-NO-RETURN////
	
	assert(!hidden());

	//REMINDER: saveSelectedUv/restoreSelectedUv
	//must be called so that they don't disagree.

	glutSetWindow(glut_window_id());

	model->getSelectedTriangles(trilist);
	for(int ea:trilist)
	{
		//FIX ME: getTriangleGroup is dumb!!
		int g = model->getTriangleGroup(ea);
		int m = model->getGroupTextureId(g);
		if(m>=0)
		{
			texture.setTexture(m); //break;
		}
		break; //getTriangleGroup workaround.
	}
		
	//openModelCoordinates();
	{	
		texture.clearCoordinates(); if(!trilist.empty())
		{
			int v = 0; float s,t;
			for(auto it=trilist.begin();it!=trilist.end();it++,v+=3)	
			{	
				for(int i=0;i<3;i++)
				{
					model->getTextureCoords(*it,i,s,t);
					texture.addVertex(s,t);
				}		
				texture.addTriangle(v,v+1,v+2);
			}
		}		
	}

	//NOTE: Some lines need to be switched around in order
	//for this to work.
	//https://github.com/zturtleman/mm3d/issues/91
	//if(m_inUndo)
	if(!model->getUndoEnabled())
	{
		texture.restoreSelectedUv();
	}
	else texture.saveSelectedUv();

	setTextureCoordsEtc(false); //NEW

	texture.updateWidget(); //REDUNDANT
}

void TextureCoordWin::init()
{
	m_ignoreChange = false;

	dimensions.disable();

	white.add_item(0,"Black")
    .add_item(0x0000ff,"Blue")
    .add_item(0x00ff00,"Green")
    .add_item(0x00ffff,"Cyan")
    .add_item(0xff0000,"Red") //id_red
    .add_item(0xff00ff,"Magenta")
    .add_item(0xffff00,"Yellow")
    .add_item(id_white,"White"); //0xffffff
	red.reference(white);
	int w = config.get("ui_texcoord_lines_color",+id_white);
	int r = config.get("ui_texcoord_selection_color",+id_red);
	if(w&&w<8) w = id_white; if(r&&r<8) r = id_red; //back-compat
	white.set_int_val(w); red.set_int_val(r);

	// TODO handle undo of select/unselect?
	// Can't do this until after constructor is done because of observer interface
	//setModel(model);
	texture.setModel(model);
	texture.setInteractive(true);
	texture.setDrawUvBorder(true); 

	mouse.select_id(+Widget::MouseSelect)
	.add_item(Widget::MouseSelect,"Select")
	.add_item(Widget::MouseMove,"Move")
	.add_item(Widget::MouseRotate,"Rotate")
	.add_item(Widget::MouseScale,"Scale");	
		
	scale_sfc.set(config.get("ui_texcoord_scale_center",true));
	scale_kar.set(config.get("ui_texcoord_scale_aspect",false));

	map.add_item(3,"Triangle").add_item(4,"Quad");
	map.add_item(0,"Group"); //Group???

	keyboard_callback = texturecoord_keyboard;
}
void TextureCoordWin::submit(control *c)
{
	int id = c->id(); switch(id)
	{
	case id_init: init2:

		texture.setLinesColor((int)white);
		texture.setSelectionColor((int)red);
		texture.setScaleFromCenter(scale_sfc);
		texture.setScaleKeepAspect(scale_kar);
		//break;

	case id_item: 

		texture.setMouseOperation
		((Widget::MouseOperationE)mouse.int_val());
		texture.updateWidget();
		break;

	case id_subitem:

		mapReset(); break;

	case id_white: 

		config.set("ui_texcoord_lines_color",(int)white);
	    goto init2;

	case id_red:

		config.set("ui_texcoord_selection_color",(int)red);
	    goto init2;

	case '+': texture.zoomIn(); break;
	case '-': texture.zoomOut(); break;
	case '=': texture.setZoomLevel(zoom.value); break;

	default:

		if(c>=ccw&&c<=uflip)
		{
			if(c==ccw) texture.rotateCoordinatesCcw();
			if(c==cw) texture.rotateCoordinatesCw();
			if(c==uflip) texture.uFlipCoordinates();
			if(c==vflip) texture.vFlipCoordinates();

			updateTextureCoordsDone();	
		}
		else if(c==scale_sfc)
		{
			config.set("ui_texcoord_scale_center",(bool)scale_sfc);
			goto init2;
		}
		else if(c==scale_kar)
		{
			config.set("ui_texcoord_scale_aspect",(bool)scale_kar);
			goto init2;
		}
		break;

	case id_scene:

		texture.draw(scene.x(),scene.y(),scene.width(),scene.height());
		break;

	case 'U': case 'V':
	
		texture.move(u.float_val()-centerpoint[0],v.float_val()-centerpoint[1]); 
		updateTextureCoordsDone();
		break;
	
	case id_ok: case id_close:

		event.close_ui_by_create_id(); //Help?

		//I guess this model saves users' work?
		hide(); return;
	}

	basic_submit(id);
}

//Transplanting this Model API to here.
//bool Model::getVertexCoords2d(unsigned vertexNumber,ProjectionDirectionE dir, double *coord)const
static void texturecoord_2d(Model *m, int v, int dir, double coord[2])
{
	//if(coord&&vertexNumber>=0&&(unsigned)vertexNumber<m_vertices.size())
	{
		//Mode::Vertex *vert = m_vertices[vertexNumber];

		double c3d[3]; m->getVertexCoords(v,c3d);
		switch(dir)
		{		
		case 0: //case ViewFront:
			coord[0] =  c3d[0]; coord[1] =  c3d[1];			
			break;
		case 1: //case ViewBack:
			coord[0] = -c3d[0]; coord[1] =  c3d[1];
			break;
		case 2: //case ViewLeft:
			coord[0] = -c3d[2]; coord[1] =  c3d[1];
			break;
		case 3: //case ViewRight:
			coord[0] =  c3d[2]; coord[1] =  c3d[1];
			break;
		case 4: //case ViewTop:
			coord[0] =  c3d[0]; coord[1] = -c3d[2];
			break;
		case 5: //case ViewBottom:
			coord[0] =  c3d[0]; coord[1] =  c3d[2];
			break;
		}
	}
}

void TextureCoordWin::mapReset()
{
	double min = 0, max = 1; //goto

	if(map)
	{
		texture.clearCoordinates();

		texture.addVertex(min,max);
	
		if(3==map.int_val())
		{
		texture.addVertex(min,min);
		texture.addVertex(max,min);
		texture.addTriangle(0,1,2);
		}
		else //4
		{
		texture.addVertex(max,max);
		texture.addVertex(min,min);
		texture.addVertex(max,min);
		texture.addTriangle(3,1,0);
		texture.addTriangle(0,2,3);
		}

	done: updateTextureCoordsDone();

		return texture.updateWidget();
	}
		
	if(trilist.empty()) //???
	return log_error("no triangles selected\n"); //???

	int direction;
	{
		float normal[3],total[3] = {};
		for(int ea:trilist)
		for(int i=0;i<3;i++)
		{
			model->getNormal(ea,i,normal);
			for(int i=0;i<3;i++)
			total[i]+=normal[i];
		}

		int index = 0;
		for(int i=1;i<3;i++)		
		if(fabs(total[i])>fabs(total[index]))
		index = i;
		switch(index)
		{
		case 0: direction = total[index]>=0.0?3:2; break;
		case 1: direction = total[index]>=0.0?4:5; break;
		case 2: direction = total[index]>=0.0?0:1; break;
		default:direction = 0;
			log_error("bad normal index: %d\n",index);			
		}
	}
	if(id_ok!=MapDirectionWin(&direction).return_on_close())
	return;

	//log_debug("mapGroup(%d)\n",direction); //???

	texture.clearCoordinates();

	double coord[2];
	double xMin = +DBL_MAX, x = -DBL_MAX;
	double yMin = +DBL_MAX, y = -DBL_MAX;
	for(int ea:trilist) for(int i=0;i<3;i++)
	{
		int v = model->getTriangleVertex(ea,i);
		texturecoord_2d(model,v,direction,coord);
		xMin = std::min(xMin,coord[0]); x = std::max(x,coord[0]);
		yMin = std::min(yMin,coord[1]); y = std::max(y,coord[1]);
	}		
	//log_debug("Bounds = (%f,%f)-(%f,%f)\n",xMin,yMin,xMax,yMax); //???

	x = 1/(x-xMin); y = 1/(y-yMin);
	int v = 0; for(int ea:trilist)
	{
		for(int i=0;i<3;i++)
		{
			int vv = model->getTriangleVertex(ea,i);
			texturecoord_2d(model,vv,direction,coord);
			texture.addVertex((coord[0]-xMin)*x,(coord[1]-yMin)*y);	
		}
		texture.addTriangle(v,v+1,v+2); v+=3;
	}

	goto done;
}

void TextureCoordWin::updateTextureCoordsDone(bool done)
{
	//NOTE: Could set the coords, but it would be different from
	//how the Position sidebar behaves.
	if(done) setTextureCoordsEtc(true);
	if(done) operationComplete(::tr("Move texture coordinates"));
}
void TextureCoordWin::updateSelectionDone()
{
	texture.saveSelectedUv();
	operationComplete(::tr("Select texture coordinates"));

	setTextureCoordsEtc(false); //NEW
}
void TextureCoordWin::operationComplete(const char *opname)
{
	m_ignoreChange = true; model->operationComplete(opname);
	m_ignoreChange = false;
}

void TextureCoordWin::setTextureCoordsEtc(bool setCoords)
{
	dimensions.redraw().text().clear();
	
	//int_list trilist;
	//model->getSelectedTriangles(trilist);

	float s[3],t[3];
		
	//NEW: Gathering stats based on sidebar.cc.
	float cmin[2] = {+FLT_MAX,+FLT_MAX};
	float cmax[2] = {-FLT_MAX,-FLT_MAX};
	
	int m = (unsigned)trilist.size();
	int n = map?(unsigned)texture.m_triangles.size():0;
	auto *tv = texture.m_vertices.data();
	for(int i=0;i<m;i++)
	{
		int tri = n?i%n:i;
		int *tt = texture.m_triangles[tri].vertex;
		int sel = 0;
		for(int j=0;j<3;j++) if(tv[tt[j]].selected)
		sel|=1<<j;
		if(!sel) continue;
		texture.getCoordinates(tri,s,t);
		int ea = trilist[i];
		for(int j=0;j<3;j++) if(sel&1<<j)
		{
			if(setCoords)
			model->setTextureCoords(ea,j,s[j],t[j]);

			cmin[0] = std::min(cmin[0],s[j]);
			cmin[1] = std::min(cmin[1],t[j]);
			cmax[0] = std::max(cmax[0],s[j]);
			cmax[1] = std::max(cmax[1],t[j]);
		}
	}	
	
	if(cmin[0]==FLT_MAX)
	{
		if(setCoords&&trilist.empty())
		log_error("no group selected\n"); 

		u.text().clear();
		v.text().clear();

		return;
	}

	for(int i=0;i<2;i++)
	{
		centerpoint[i] = (cmin[i]+cmax[i])/2;
	}
	u.set_float_val(centerpoint[0]*texture.getUvWidth());
	v.set_float_val(centerpoint[1]*texture.getUvHeight());
	
	//dimensions.text().format("%g,%g,%g",cmax[0]-cmin[0],cmax[1]-cmin[1],cmax[2]-cmin[2]);
	//dimensions.text().clear();
	for(int i=0;;i++)
	{
		enum{ width=5 };
		char buf[32]; 
		double dim = fabs(cmax[i]-cmin[i]); //-0
		dim*=i?texture.getUvHeight():texture.getUvWidth();
		int len = snprintf(buf,sizeof(buf),"%.6f",dim); 
		if(char*dp=strrchr(buf,'.'))
		{
			while(len>width&&buf+len>dp)
			{
				dp--; len--;
			}
			while(buf+len>dp&&buf[len]=='0')
			{
				dp--; len--;
			}
			if(dp==buf+len) len--;
		}
		//while(len<width) buf[len++] = ' ';
		dimensions.text().append(buf,len);
		if(i==1) break;
		dimensions.text().push_back(',');
		dimensions.text().push_back(' ');		
	}
	dimensions.set_text(dimensions);
}
