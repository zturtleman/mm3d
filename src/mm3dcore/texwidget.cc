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

#include "glheaders.h"
#include "texwidget.h"

#include "tool.h" //ButtonState
#include "rotatepoint.h"
#include "glmath.h" //distance

#include "texture.h"
#include "model.h"
#include "log.h"
//#include "mm3dport.h"

#include "pixmap/arrow.xpm"
#include "pixmap/crosshairrow.xpm"

static struct
{
	enum{ SCROLL_SIZE = 16 };

	int x, y;
	int texIndex;
	float s1,t1;
	float s2,t2;
	float s3,t3;
	float s4,t4;

}scrollwidget[ScrollWidget::ScrollButtonMAX] =
{
	{ -18,-18,1,  0,0,  1,0,  1,1, 0,1  }, //Pan
	{ -52,-18,0,  0,1,  0,0,  1,0, 1,1  }, //Left
	{ -35,-18,0,  0,0,  0,1,  1,1, 1,0  }, //Right
	{ -18,-35,0,  0,0,  1,0,  1,1, 0,1  }, //Up
	{ -18,-52,0,  0,1,  1,1,  1,0, 0,0  }, //Down
};

static std::pair<int,ScrollWidget*> 
textwidget_scroll,textwidget_3dcube;
extern void scrollwidget_stop_timer() //REMOVE ME
{
	textwidget_scroll.first = 0;
}

ScrollWidget::ScrollWidget()
	:
m_activeButton(),
m_viewportX(),m_viewportY(),
m_viewportWidth(),m_viewportHeight(),
m_zoom(1),
m_scroll(),
m_nearOrtho(-1),m_farOrtho(1),
m_interactive(),
m_autoOverlay(),
m_overlayButton(ScrollButtonMAX)
{}
ScrollWidget::~ScrollWidget()
{
	textwidget_scroll.first = 0; //Timer business.
	textwidget_3dcube.first = 0;
}

void ScrollWidget::scrollUp()
{
	m_scroll[1]+=m_zoom*0.10; updateViewport('p');
}
void ScrollWidget::scrollDown()
{
	m_scroll[1]-=m_zoom*0.10; updateViewport('p');
}
void ScrollWidget::scrollLeft()
{
	m_scroll[0]-=m_zoom*0.10; updateViewport('p');
}
void ScrollWidget::scrollRight()
{
	m_scroll[0]+=m_zoom*0.10; updateViewport('p');
}

//#define VP_ZOOMSCALE 0.75
static const double texwidgit_zoom = 0.75;

void ScrollWidget::zoomIn()
{
	//Looks like a bug?
	//Using ModelViewport::zoomIn()
	//if(m_zoom/texwidgit_zoom>0.0001)
	setZoomLevel(m_zoom*texwidgit_zoom);
}
void ScrollWidget::zoomOut()
{
	setZoomLevel(m_zoom/texwidgit_zoom);
}
//0.0001,250000.0
const double ScrollWidget::zoom_min = 0.0001;
const double ScrollWidget::zoom_max = 250000;
void ScrollWidget::setZoomLevel(double z)
{
	z = std::min(std::max(z,zoom_min),zoom_max);

	if(m_interactive&&m_zoom!=z)
	{
		m_zoom = z;
		
		//QString zoomStr;
		//zoomStr.sprintf("%f",m_zoom);		
		//emit zoomLevelChanged(zoomStr);
		//emit(this,zoomLevelChanged,m_zoom);
		updateViewport('z');
	}
}

void ScrollWidget::drawOverlay(GLuint m_scrollTextures[2])
{	
	//Need to defer this because of GLX crumminess
	//(it won't bind a GL context until onscreen.)
	if(!*m_scrollTextures) initOverlay(m_scrollTextures);

	if(1==m_autoOverlay) return; //EXPERIMENTAL

	int w = m_viewportWidth;
	int h = m_viewportHeight;

	//setViewportOverlay();
	{
		//glViewport(0,0,(GLint)m_viewportWidth,(GLint)m_viewportHeight);

		glMatrixMode(GL_PROJECTION); glLoadIdentity();

		glOrtho(0,w,0,h,m_nearOrtho,m_farOrtho); //-1,1);

		//glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	}

	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);

	glEnable(GL_TEXTURE_2D);

	int sx = 0;
	int sy = 0;
	int size = scrollwidget->SCROLL_SIZE;

	for(int b=0;b<ScrollButtonMAX;b++)
	{
		auto *sbt = &scrollwidget[b];
		sx = sbt->x;
		sy = sbt->y;

		glBindTexture(GL_TEXTURE_2D,m_scrollTextures[sbt->texIndex]);

		glBegin(GL_QUADS);

		glTexCoord2f(sbt->s1,sbt->t1);
		glVertex3i(w+sx,h+sy,0);
		glTexCoord2f(sbt->s2,sbt->t2);
		glVertex3i(w+sx+size,h+sy,0);
		glTexCoord2f(sbt->s3,sbt->t3);
		glVertex3i(w+sx+size,h+sy+size,0);
		glTexCoord2f(sbt->s4,sbt->t4);
		glVertex3i(w+sx,h+sy+size,0);

		glEnd();
	}

	glDisable(GL_TEXTURE_2D);
}

void ScrollWidget::initOverlay(GLuint m_scrollTextures[2])
{	
	if(m_scrollTextures[0]){ assert(0); return; }

	const char **xpm[] = { arrow_xpm,crosshairrow_xpm };

	initTexturesUI(2,m_scrollTextures,(char***)xpm);
}

TextureWidget::TextureWidget(Parent *parent)
	: 
PAD_SIZE(6),
parent(parent),
m_scrollTextures(),
m_sClamp(),
m_tClamp(),
m_model(),
m_materialId(-1),
m_texture(),
m_glTexture(),
m_drawMode(DM_Edit),
m_drawVertices(true),
m_drawBorder(),
m_solidBackground(),
m_operation(MouseSelect),
m_scaleKeepAspect(),
m_scaleFromCenter(),
m_selecting(),
m_drawBounding(),
m_3d(),
m_buttons(),
m_xMin(0),
m_xMax(1),
m_yMin(0),
m_yMax(1),
m_xRotPoint(0.5),
m_yRotPoint(0.5),
m_linesColor(0xffffff),
m_selectionColor(0xff0000),
m_overrideWidth(),m_overrideHeight()
{
	m_scroll[0] = m_scroll[1] = 0.5;

	//setAutoBufferSwap(false); //Qt
}

void TextureWidget::setModel(Model *model)
{
	m_model = model;
	m_texture = nullptr;
	m_materialId = -1; //NEW
	clearCoordinates();
	//glDisable(GL_TEXTURE_2D); //???
}
void TextureWidget::clearCoordinates()
{
	m_vertices.clear(); m_triangles.clear();
	
	m_operations_verts.clear();
}

void TextureWidget::updateViewport(int how)
{
	if(how=='z') //HACK
	{
		parent->zoomLevelChangedSignal();
	}

	m_xMin = m_scroll[0]-m_zoom/2;
	m_xMax = m_scroll[0]+m_zoom/2;
	m_yMin = m_scroll[1]-m_zoom/2;
	m_yMax = m_scroll[1]+m_zoom/2;

	parent->updateWidget(); //updateGL();
}

void TextureWidget::drawTriangles()
{
	auto vb = m_vertices.data();

	for(auto&ea:m_triangles)
	{
		bool wrapLeft = false;
		bool wrapRight = false;
		bool wrapTop = false;
		bool wrapBottom = false;

		for(int ea2:ea.vertex)
		{
			auto &v = vb[ea2];
			glVertex3d(v.s,v.t,-0.5);

			if(m_drawMode!=DM_Edit) //paintexturewin.cc
			{
				if(v.s<0) wrapLeft = true;
				if(v.s>1) wrapRight = true;
				if(v.t<0) wrapBottom = true;
				if(v.t>1) wrapTop = true;
			}
		}

		if(m_drawMode!=DM_Edit) //paintexturewin.cc
		{
			if(wrapLeft) for(int ea2:ea.vertex)
			glVertex3d(vb[ea2].s+1,vb[ea2].t,-0.5);

			if(wrapRight) for(int ea2:ea.vertex)
			glVertex3d(vb[ea2].s-1,vb[ea2].t,-0.5);

			if(wrapBottom) for(int ea2:ea.vertex)
			glVertex3d(vb[ea2].s,vb[ea2].t+1,-0.5);

			if(wrapTop) for(int ea2:ea.vertex)
			glVertex3d(vb[ea2].s,vb[ea2].t-1,-0.5);
		}
	}
}

static void texwidget_3dcube_timer(int i)
{
	if(i!=textwidget_3dcube.first) return;
	
	//TODO: UI layer should implement this.
	ScrollWidget::setTimerUI(30,texwidget_3dcube_timer,i);

	//textwidget_3dcube.second->updateGL();
	void *s = textwidget_3dcube.second;
	((TextureWidget*)s)->parent->updateWidget();
}
void TextureWidget::set3d(bool o)
{
	int i = ++textwidget_3dcube.first;
	
	m_3d = o; if(o)
	{
		textwidget_3dcube.second = this;
		texwidget_3dcube_timer(i); 
	}
	else parent->updateWidget(); //updateGL();
}

void TextureWidget::setTexture(int materialId, Texture *texture)
{
	if(!texture&&materialId>=0) //NEW
	{
		//Removing need for TextureFrame middle class.
		texture = m_model->getTextureData(materialId);
	}

	m_materialId = materialId; m_texture = texture;

	m_sClamp = m_model->getTextureSClamp(materialId);
	m_tClamp = m_model->getTextureTClamp(materialId);

	/*This is annoying.
	m_zoom = 1;
	m_scroll[0] = m_scroll[1] = 0.5;
	*/

	updateViewport();

	if(m_texture&&m_glTexture) initTexture();	

	//resizeGL(this->width(),this->height()); //???

	parent->updateWidget(); //updateGL();
}
void TextureWidget::initTexture()
{
	if(!m_glTexture)
	glGenTextures(1,&m_glTexture);

	glBindTexture(GL_TEXTURE_2D,m_glTexture);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,/*GL_NEAREST*/GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,/*GL_NEAREST*/GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,m_sClamp?GL_CLAMP:GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,m_tClamp?GL_CLAMP:GL_REPEAT);

	GLuint format = m_texture->m_format==Texture::FORMAT_RGBA?GL_RGBA:GL_RGB;

	//https://github.com/zturtleman/mm3d/issues/85
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	/*FIX ME //Mipmaps option?
	glTexImage2D(GL_TEXTURE_2D,0,format,
	m_texture->m_width,m_texture->m_height,0,
	format,GL_UNSIGNED_BYTE,m_texture->m_data.data());*/
	gluBuild2DMipmaps(GL_TEXTURE_2D,format, //GLU
	m_texture->m_width,m_texture->m_height,							
	format,GL_UNSIGNED_BYTE,m_texture->m_data.data());
}

void TextureWidget::uFlipCoordinates()
{
	for(auto&ea:m_vertices) if(ea.selected)
	{
		ea.s = 1-ea.s;
	}
	
	parent->updateWidget(); //updateGL();
}
void TextureWidget::vFlipCoordinates()
{
	for(auto&ea:m_vertices) if(ea.selected)
	{
		ea.t = 1-ea.t;
	}
	
	parent->updateWidget(); //updateGL();
}

void TextureWidget::rotateCoordinatesCcw()
{
	for(auto&ea:m_vertices) if(ea.selected)
	{
		double swap = ea.t;
		ea.t = ea.s; ea.s = 1-swap;
	}

	parent->updateWidget(); //updateGL();
}
void TextureWidget::rotateCoordinatesCw()
{
	for(auto&ea:m_vertices) if(ea.selected)
	{
		double swap = ea.t;
		ea.t = 1-ea.s; ea.s = swap;
	}

	parent->updateWidget(); //updateGL();
}

void TextureWidget::addVertex(double s, double t)
{
	TextureVertexT v = {s,t,true};
	m_vertices.push_back(v);
	//return (unsigned)m_vertices.size()-1;
}

void TextureWidget::addTriangle(int v1, int v2, int v3)
{
	//SILLINESS
	//size_t sz = m_vertices.size();
	//if((size_t)v1<sz&&(size_t)v2<sz&&(size_t)v3<sz) //???
	{	
		TextureTriangleT t = {{v1,v2,v3}};
		m_triangles.push_back(t);

	//	return (unsigned)m_triangles.size()-1;
	}
	//else assert(0); return -1; //???
}

static void textwidget_scroll_timer(int id)
{
	if(id==textwidget_scroll.first)
	textwidget_scroll.second->pressOverlayButton(id<0);
}
void ScrollWidget::pressOverlayButton(bool rotate)
{
	switch(m_overlayButton)
	{
	case ScrollButtonPan: 

		default: return; //ScrollButtonMAX 

	case ScrollButtonLeft:
		if(rotate)
		rotateLeft(); else scrollLeft();
		break;
	case ScrollButtonRight:
		if(rotate) 
		rotateRight(); else scrollRight();
		break;
	case ScrollButtonUp:
		if(rotate) 
		rotateUp(); else scrollUp();
		break;
	case ScrollButtonDown:
		if(rotate)
		rotateDown(); else scrollDown();
		break;
	}
	
	//MAGIC: Communicate these to the timer.
	int id = 1+abs(textwidget_scroll.first); //C99
	if(rotate) id = -id;
	textwidget_scroll.first = id;
	textwidget_scroll.second = this;
	//TODO: UI layer should implement this.
	setTimerUI(300,textwidget_scroll_timer,id);
}
bool ScrollWidget::pressOverlayButton(int x, int y, bool rotate)
{	
	x-=m_viewportX; y-=m_viewportY; 

	int w = m_viewportWidth;
	int h = m_viewportHeight; 
	m_overlayButton = ScrollButtonMAX;
	for(int b=0;b<ScrollButtonMAX;b++)
	{
		int sx = scrollwidget[b].x;
		int sy = scrollwidget[b].y;
		if(x>=w+sx&&x<=w+sx+scrollwidget->SCROLL_SIZE
		 &&y>=h+sy&&y<=h+sy+scrollwidget->SCROLL_SIZE)
		{
			m_overlayButton = (ScrollButtonE)b;
			pressOverlayButton(rotate);
			break;
		}
	}
	return m_overlayButton!=ScrollButtonMAX;
}
bool TextureWidget::mousePressEvent(int bt, int bs, int x, int y)
{	
	if(!m_interactive) return false;

	if(!parent->mousePressSignal(bt)) return false;
			
	if(!m_buttons) //NEW
	{
		assert(!m_activeButton);

		if(pressOverlayButton(x,y,false)) 
		{
			m_activeButton = bt; //NEW

			return true;
		}
	}

	x-=m_x; y-=m_y; //NEW
	
	m_lastXPos = x; m_lastYPos = y; 	

	double s = getWindowXCoord(x);
	double t = getWindowYCoord(y);
	
	if(!m_buttons) m_activeButton = bt; //NEW	

	m_buttons|=bt; //e->button();

	if(bt!=m_activeButton) return true; //NEW

	m_constrain = 0;

	//if(e->button()&Qt::MidButton)
	if(bt==Tool::BS_Middle)
	{
		// We're panning
	}
	else switch(m_operation)
	{
	case MouseSelect:

		//if(!(e->button()&Qt::RightButton
		//||e->modifiers()&Qt::ShiftModifier))
		if(!(bt==Tool::BS_Right||bs&Tool::BS_Shift))
		{
			//clearSelected();
			for(auto&ea:m_vertices) ea.selected = false;
		}
		m_xSel1 = s; m_ySel1 = t;
		m_xSel2 = s; m_ySel2 = t; //NEW:
		//m_selecting = e->button()&Qt::RightButton?false:true;
		m_selecting = bs&Tool::BS_Right?false:true;
		m_drawBounding = true;
		break;

	case MouseScale:
			
		startScale(s,t);
		//break;

	case MouseMove:

		m_constrain = ~3;
		break;

	case MouseRotate:
	
		if(bt==Tool::BS_Right)
		{
			m_xRotPoint = s; m_yRotPoint = t;
		}
		else
		{
			m_xRotStart = s-m_xRotPoint;
			m_yRotStart = t-m_yRotPoint;

			double angle = rotatepoint_diff_to_angle(m_xRotStart*m_aspect,m_yRotStart);
				
			//if(e->modifiers()&Qt::ShiftModifier)
			if(bs&Tool::BS_Shift) angle = rotatepoint_adjust_to_nearest(angle,15);

			m_startAngle = angle;

			m_operations_verts.clear();
			for(auto&ea:m_vertices) if(ea.selected)
			{
				int v = &ea-m_vertices.data();
				TransVertexT r = {v,(ea.s-m_xRotPoint)*m_aspect,ea.t-m_yRotPoint};
				m_operations_verts.push_back(r);
			}
		}
		parent->updateWidget(); //updateGL();
		break;
	
	case MouseRange:
	
		setCursorUI(getRangeDirection(s,t,true));
		break;
	}	

	return true; //I guess??
}

void TextureWidget::mouseReleaseEvent(int bt, int bs, int x, int y)
{
	if(!m_interactive) return;

	m_buttons&=~bt; //e->button();

	if(bt!=m_activeButton) return; //NEW
		
	m_activeButton = 0;

	if(m_overlayButton!=ScrollButtonMAX)
	{
		m_overlayButton = ScrollButtonMAX;

		textwidget_scroll.first = 0; //Cancel timer.

		return;
	}	

	x-=m_x; y-=m_y; //UNUSED
	
	//if(e->button()&Qt::MidButton)
	if(bt==Tool::BS_Middle)
	{
		// We're panning
	}
	else switch(m_operation)
	{
	case MouseSelect:
		
		m_drawBounding = false;

		selectDone();
						
		//emit updateSelectionDoneSignal();
		//emit(this,updateSelectionDoneSignal);
		parent->updateSelectionDoneSignal();
		break;

	case MouseMove:
								
		//emit updateCoordinatesSignal();
		//emit(this,updateCoordinatesSignal);
		parent->updateCoordinatesSignal();
						
				case MouseRotate://2019
				case MouseScale: //2019
				/*2019: FALLING THROUGH

		//emit updateCoordinatesDoneSignal();
		//emit(this,updateCoordinatesDoneSignal);
		parent->updateCoordinatesDoneSignal();

		break;

	case MouseRotate:
						
		// Nothing to do here

		//emit updateCoordinatesDoneSignal();
		//emit(this,updateCoordinatesDoneSignal);
		parent->updateCoordinatesDoneSignal();

		break;

	case MouseScale:
						
		// Nothing to do here

				2019: FALLING THROUGH*/

		//emit updateCoordinatesDoneSignal();
		//emit(this,updateCoordinatesDoneSignal);
		parent->updateCoordinatesDoneSignal();
		break;

	case MouseRange:

		//if(m_buttons&Qt::LeftButton)
		if(bt==Tool::BS_Left)
		{
			if(m_dragAll||m_dragTop||m_dragBottom||m_dragLeft||m_dragRight)
			{
				//emit updateRangeDoneSignal();
				//emit(this,updateRangeDoneSignal);
				parent->updateRangeDoneSignal();
			}
		}
		else if(m_dragAll)
		{
			//emit updateSeamDoneSignal();
			//emit(this,updateSeamDoneSignal);
			parent->updateSeamDoneSignal();
		}
		break;
	}
}

void TextureWidget::mouseMoveEvent(int bs, int x, int y)
{
	if(!m_interactive) return;

	if(m_autoOverlay) //EXPERIMENTAL
	{
		//NOTE: This can't catch when the mouse moves 
		//out of bounds if the cursor is clipped.
		int cmp = x>=m_viewportX&&y>=m_viewportY
		&&x<m_viewportWidth&&y<m_viewportHeight?2:1;
		if(cmp!=m_autoOverlay)
		{
			m_autoOverlay = cmp; parent->updateWidget();
		}
	}
			
	x-=m_x; y-=m_y; 
		
	//bool shift = (e->modifiers()&Qt::ShiftModifier)!=0;
	bool shift = (bs&Tool::BS_Shift)!=0;
	if(shift&&m_constrain==~3) 
	{
		int ax = std::abs(x-m_lastXPos);
		int ay = std::abs(y-m_lastYPos);
		if(ax==ay) return; //NEW

		m_constrain = ax>ay?~1:~2;
	}
	if(m_constrain&1) x = m_lastXPos;
	if(m_constrain&2) y = m_lastYPos;

	double s,t,ds,dt;
	s = getWindowXCoord(x); ds = getWindowXDelta(x,m_lastXPos);
	t = getWindowYCoord(y); dt = getWindowYDelta(y,m_lastYPos);

	m_lastXPos = x; m_lastYPos = y;
	
	int bt = m_activeButton; //NEW

	if(m_overlayButton!=ScrollButtonMAX)
	{
		switch(m_overlayButton)
		{
		case ScrollButtonPan: pan:
		
			m_scroll[0]-=ds; m_xMin-=ds; m_xMax-=ds;
			m_scroll[1]-=dt; m_yMin-=dt; m_yMax-=dt;

			updateViewport(); break;
		}
	}
	else if(!bt) //!m_buttons
	{
		//This is peculiar to MouseRange alone.
		//updateCursorShape(x,y);
		if(m_operation==MouseRange)
		setRangeCursor(x,y);
	}
	else if(bt==Tool::BS_Middle) //m_buttons&Qt::MidButton
	{
		goto pan;
	}
	else switch(m_operation)
	{
	case MouseSelect:
		
		updateSelectRegion(s,t);
		break;

	case MouseMove:
  
		moveSelectedVertices(ds,dt);
							
		//emit updateCoordinatesSignal();
		//emit(this,updateCoordinatesSignal);
		parent->updateCoordinatesSignal();
		break;

	case MouseRotate:
	{
		s-=m_xRotPoint; t-=m_yRotPoint;
				
		double angle = rotatepoint_diff_to_angle(s*m_aspect,t);

		if(shift) angle = rotatepoint_adjust_to_nearest(angle,15);

		rotateSelectedVertices(angle-m_startAngle);

		//emit updateCoordinatesSignal();
		//emit(this,updateCoordinatesSignal);
		parent->updateCoordinatesSignal();
		break;
	}
	case MouseScale:

		scaleSelectedVertices(s,t);
							
		//emit updateCoordinatesSignal();
		//emit(this,updateCoordinatesSignal);
		parent->updateCoordinatesSignal();
		break;

	case MouseRange:
	
		//if(m_buttons&Qt::LeftButton)
		if(bt==Tool::BS_Left)
		{	
			if(m_dragLeft||m_dragAll)
			{
				m_xRangeMin+=ds;
				m_xRangeMax = std::max(m_xRangeMax,m_xRangeMin);
			}
			if(m_dragRight||m_dragAll)
			{
				m_xRangeMax+=ds;
				m_xRangeMin = std::min(m_xRangeMin,m_xRangeMax);
			}
			if(m_dragBottom||m_dragAll)
			{
				m_yRangeMin+=dt;
				m_yRangeMax = std::max(m_yRangeMax,m_yRangeMin);
			}
			if(m_dragTop||m_dragAll)
			{
				m_yRangeMax+=dt;
				m_yRangeMin = std::min(m_yRangeMin,m_yRangeMax);
			}

			if(m_dragAll||m_dragTop||m_dragBottom||m_dragLeft||m_dragRight)
			{
				//emit updateRangeSignal();
				//emit(this,updateRangeSignal);
				parent->updateRangeSignal();
			}
		}
		else if(m_dragAll) //???
		{		
			//emit updateSeamSignal(ds*-2*PI,dt*-2*PI);
			//emit(this,updateSeamSignal,ds*-2*PI,dt*-2*PI);
			parent->updateSeamSignal(ds*-2*PI,dt*-2*PI);
		}
		break;
	}
}

void TextureWidget::wheelEvent(int wh, int bs, int, int)
{
	if(m_interactive) if(wh>0) zoomIn(); else zoomOut();
}

bool TextureWidget::keyPressEvent(int bt, int bs, int, int)
{	
	if(m_interactive) switch(bt)
	{
	//case Qt::Key_Home:
	case Tool::BS_Left|Tool::BS_Special: //HOME
	
		//if(~e->modifiers()&Qt::ShiftModifier)
		if(~bs&Tool::BS_Shift)
		{
			m_scroll[0] = m_scroll[1] = 0.5;
			m_zoom = 1;
		}			
		else if(m_drawMode!=DM_Edit)
		{
			break;
		}
		else if(m_operation==MouseRange)
		{
			m_scroll[0] = (m_xRangeMax-m_xRangeMin)/2+m_xRangeMin;
			m_scroll[1] = (m_yRangeMax-m_yRangeMin)/2+m_yRangeMin;

			double xzoom = m_xRangeMax-m_xRangeMin;
			double yzoom = m_yRangeMax-m_yRangeMin;

			m_zoom = 1.10*std::max(xzoom,yzoom);
		}
		else
		{
			double xMin = +DBL_MAX, xMax = -DBL_MAX;
			double yMin = +DBL_MAX, yMax = -DBL_MAX;

			//for(size_t v=1;v<vcount;v++) //1???
			for(auto&ea:m_vertices) if(ea.selected)
			{
				xMin = std::min(xMin,ea.s);
				xMax = std::max(xMax,ea.s);
				yMin = std::min(yMin,ea.t);
				yMax = std::max(yMax,ea.t);
			}
			if(xMin!=DBL_MAX)
			{
				double xzoom = xMax-xMin;
				double yzoom = yMax-yMin;

				m_scroll[0] = xzoom/2+xMin;
				m_scroll[1] = yzoom/2+yMin;

				m_zoom = 1.10*std::max(xzoom,yzoom);
			}
		}
		updateViewport(); break;
	
	//case Qt::Key_Equal: case Qt::Key_Plus: 
	case '=': case '+': zoomIn(); break;

	//case Qt::Key_Minus: case Qt::Key_Underscore:
	case '-': case '_': zoomOut(); break;

	case '0':
		
		m_scroll[0] = m_scroll[1] = 0.5;
		updateViewport(); break;

	//case Qt::Key_Up:
	case Tool::BS_Up: scrollUp(); break;
	//case Qt::Key_Down:
	case Tool::BS_Down: scrollDown(); break;
	//case Qt::Key_Left:
	case Tool::BS_Left: scrollLeft(); break;
	//case Qt::Key_Right: 
	case Tool::BS_Right: scrollRight(); break;

	//default: QGLWidget::keyPressEvent(e); break;
	default: return false;
	}
	//else QGLWidget::keyPressEvent(e);
	return true;
}
 
void TextureWidget::moveSelectedVertices(double x, double y)
{
	for(auto&ea:m_vertices) if(ea.selected)
	{
		ea.s+=x; ea.t+=y;
	}
	
	parent->updateWidget(); //updateGL();
}

void TextureWidget::updateSelectRegion(double x, double y)
{
	m_xSel2 = x; m_ySel2 = y; 
	
	parent->updateWidget(); //updateGL();
}

void TextureWidget::selectDone()
{
	if(m_xSel1>m_xSel2)
	{
		double temp = m_xSel2;
		m_xSel2 = m_xSel1;
		m_xSel1 = temp;
	}
	if(m_ySel1>m_ySel2)
	{
		double temp = m_ySel2;
		m_ySel2 = m_ySel1;
		m_ySel1 = temp;
	}
	for(auto&ea:m_vertices)
	if(ea.s>=m_xSel1&&ea.s<=m_xSel2 
	 &&ea.t>=m_ySel1&&ea.t<=m_ySel2)
	{
		ea.selected = m_selecting;
	}
	
	parent->updateWidget(); //updateGL();
}

void TextureWidget::setRangeCursor(int x, int y)
{
	if(m_interactive)
	{
		int w = m_viewportWidth;
		int h = m_viewportHeight;

		int sx = 0;
		int sy = 0;
		int size = scrollwidget->SCROLL_SIZE;

		int bx = x;
		int by = /*h-*/y;

		ScrollButtonE button = ScrollButtonMAX;
		for(int b=0;b<ScrollButtonMAX;b++)
		{
			sx = scrollwidget[b].x;
			sy = scrollwidget[b].y;

			if(bx>=w+sx&&bx<=w+sx+size
			 &&by>=h+sy&&by<=h+sy+size)
			{
				button = (ScrollButtonE)b; 
				break;
			}
		}

		if(button==ScrollButtonMAX)		
		if(m_operation==MouseRange)
		{	
			double windowX = getWindowXCoord(x);
			double windowY = getWindowYCoord(y);						
			setCursorUI(getRangeDirection(windowX,windowY));
			return;
		}
	}

	setCursorUI();
}

int TextureWidget::getRangeDirection(double windowX, double windowY, bool press)
{
	if(!m_interactive) return -1;

	bool dragAll = false;
	bool dragTop = false;
	bool dragBottom = false;
	bool dragLeft = false;
	bool dragRight = false;

	double prox = 6.0/m_width*m_zoom;

	if(windowX>=m_xRangeMin-prox
	 &&windowX<=m_xRangeMax+prox
	 &&windowY>=m_yRangeMin-prox
	 &&windowY<=m_yRangeMax+prox)
	{
		if(fabs(m_xRangeMin-windowX)<=prox)
		{
			dragLeft = true;
		}
		if(fabs(m_xRangeMax-windowX)<=prox)
		{
			dragRight = true;
		}
		if(fabs(m_yRangeMin-windowY)<=prox)
		{
			dragBottom = true;
		}
		if(fabs(m_yRangeMax-windowY)<=prox)
		{
			dragTop = true;
		}

		if(dragLeft&&dragRight)
		{
			// The min and max are very close together,don't drag
			// both at one time.
			if(windowX<m_xRangeMin)
			{
				dragRight = false;
			}
			else if(windowX>m_xRangeMax)
			{
				dragLeft = false;
			}
			else
			{
				// We're in-between,don't drag either (top/bottom still okay)
				dragLeft = false;
				dragRight = false;
			}
		}
		if(dragTop&&dragBottom)
		{
			// The min and max are very close together,don't drag
			// both at one time.
			if(windowY<m_yRangeMin)
			{
				dragTop = false;
			}
			else if(windowY>m_yRangeMax)
			{
				dragBottom = false;
			}
			else
			{
				// We're in-between,don't drag either (left/right still okay)
				dragTop = false;
				dragBottom = false;
			}
		}

		if(!dragTop&&!dragBottom&&!dragLeft&&!dragRight)
		{
			if(windowX>m_xRangeMin&&windowX<m_xRangeMax
			 &&windowY>m_yRangeMin&&windowY<m_yRangeMax)
			{
				dragAll = true;
			}
		}
	}	

	if(press)
	{
		m_dragAll = dragAll;
		m_dragTop = dragTop;
		m_dragBottom = dragBottom;
		m_dragLeft = dragLeft;
		m_dragRight = dragRight;
	}

	if(dragLeft)
	{
		if(dragTop) return 45;

		if(dragBottom) return 315;

		return 0;
	}
	if(dragRight)
	{
		if(dragTop) return 135;

		if(dragBottom) return 225;

		return 180;
	}

	if(dragTop) return 90;

	if(dragBottom) return 270;

	return dragAll?360:-1;
}

void TextureWidget::getCoordinates(int tri, float *s, float *t)
{
	//if(t&&s&&(size_t)tri<m_triangles.size()) //???
	for(int ea:m_triangles[tri].vertex)
	{
		*s++ = (float)m_vertices[ea].s; 
		*t++ = (float)m_vertices[ea].t;
	}
}

void TextureWidget::saveSelectedUv()
{
	std::vector<int> selectedUv;
	int i,iN = (unsigned)m_vertices.size();
	for(i=0;i<iN;i++) if(m_vertices[i].selected)
	{
		selectedUv.push_back(i);
	}
	m_model->setSelectedUv(selectedUv);
}

void TextureWidget::restoreSelectedUv()
{		
	//clearSelected();
	for(auto&ea:m_vertices) ea.selected = false;

	std::vector<int> selectedUv;
	m_model->getSelectedUv(selectedUv);
	for(auto ea:selectedUv)
	{
		//NOTE: getSelectedUv IS NOT ACTUALLY PART OF THE MODEL DATA.
		//IT JUST SAVES/RESTORES TextureWidget's STATE.

		//ASSERT?
		//https://github.com/zturtleman/mm3d/issues/90
		//Model::appendUndo was added so this assert is
		//not hit. But I feel if it's hit again, it may
		//be because TextureCoordWin goes in and out of
		//hiding. When hidden it doesn't generate these.

		if((size_t)ea<m_vertices.size()) //???
		m_vertices[ea].selected = true; else assert(0);
	}
}

void TextureWidget::setRange(double xMin, double yMin, double xMax, double yMax)
{
	m_xRangeMin = xMin; m_yRangeMin = yMin; m_xRangeMax = xMax; m_yRangeMax = yMax;
}

void TextureWidget::getRange(double &xMin, double &yMin, double &xMax, double &yMax)
{
	xMin = m_xRangeMin; yMin = m_yRangeMin; xMax = m_xRangeMax; yMax = m_yRangeMax;
}

void TextureWidget::startScale(double x, double y)
{	
	double xMin = +DBL_MAX, xMax = -DBL_MAX;
	double yMin = +DBL_MAX, yMax = -DBL_MAX;

	m_operations_verts.clear();
	for(auto&ea:m_vertices) if(ea.selected)
	{
		xMin = std::min(xMin,ea.s);
		xMax = std::max(xMax,ea.s);
		yMin = std::min(yMin,ea.t);
		yMax = std::max(yMax,ea.t);

		TransVertexT s = {&ea-m_vertices.data(),ea.s,ea.t};
		m_operations_verts.push_back(s);
	}
	if(m_operations_verts.empty()) return; //NEW

	if(m_scaleFromCenter)
	{
		m_centerX = (xMax-xMin)/2+xMin;
		m_centerY = (yMax-yMin)/2+yMin;

		m_startLengthX = fabs(m_centerX-x);
		m_startLengthY = fabs(m_centerY-y);
	}
	else //From corner?
	{
		//DUPLICATES scaletool.cc.

		//NOTE: sqrt not required.
		double minmin = distance(x,y,xMin,yMin);
		double minmax = distance(x,y,xMin,yMax);
		double maxmin = distance(x,y,xMax,yMin);
		double maxmax = distance(x,y,xMax,yMax);

		//Can this be simplified?
		if(minmin>minmax)
		{
			if(minmin>maxmin)
			{
				if(minmin>maxmax)
				{
					m_farX = xMin; m_farY = yMin;
				}
				else
				{
					m_farX = xMax; m_farY = yMax;
				}
			}
			else same: // maxmin>minmin
			{
				if(maxmin>maxmax)
				{
					m_farX = xMax; m_farY = yMin;
				}
				else
				{
					m_farX = xMax; m_farY = yMax;
				}
			}
		}
		else // minmax>minmin
		{
			if(minmax>maxmin)
			{
				if(minmax>maxmax)
				{
					m_farX = xMin; m_farY = yMax;
				}
				else
				{
					m_farX = xMax; m_farY = yMax;
				}
			}
			else goto same; // maxmin>minmax			
		}

		m_startLengthX = fabs(x-m_farX);
		m_startLengthY = fabs(y-m_farY);
	}
}

void TextureWidget::rotateSelectedVertices(double angle)
{
	Matrix m;
	Vector rot(0,0,angle); m.setRotation(rot);
	
	double a = 1/m_aspect;
	for(auto&ea:m_operations_verts)
	{
		Vector vec(ea.x,ea.y,0);
		m.apply3(vec);
		m_vertices[ea.index].s = vec[0]*a+m_xRotPoint;
		m_vertices[ea.index].t = vec[1]+m_yRotPoint;
	}

	parent->updateWidget(); //updateGL();
}

void TextureWidget::scaleSelectedVertices(double x, double y)
{
	double xx = m_scaleFromCenter?m_centerX:m_farX;
	double yy = m_scaleFromCenter?m_centerY:m_farY;

	double s = m_startLengthX<0.00006?1:fabs(x-xx)/m_startLengthX;
	double t = m_startLengthY<0.00006?1:fabs(y-yy)/m_startLengthY;

	if(m_scaleKeepAspect) s = t = std::max(s,t);

	for(auto&ea:m_operations_verts) //lerp
	{
		m_vertices[ea.index].s = (ea.x-xx)*s+xx;
		m_vertices[ea.index].t = (ea.y-yy)*t+yy;
	}

	parent->updateWidget(); //updateGL();
}

void TextureWidget::draw(int x, int y, int w, int h)
{
	int ww = w, hh = h;

	//updateSize()
	{
		//enum{ PAD_SIZE=6 };

		x+=PAD_SIZE; w-=PAD_SIZE*2;
		y+=PAD_SIZE; h-=PAD_SIZE*2;

		m_x = x; m_y = y;

		if(!m_3d&&m_texture)
		{
			ww = m_overrideWidth;
			hh = m_overrideHeight;
			if(!ww) ww = m_texture->m_origWidth;
			if(!hh) hh = m_texture->m_origHeight;

			double s = (double)w/ww;
			double t = (double)h/hh;
			if(s>t) ww = (int)(ww*t); else ww = w;
			if(s<t) hh = (int)(hh*s); else hh = h;
			
			m_x+=(w-ww)/2; m_y+=(h-hh)/2;

			if(!m_interactive) 
			{
				x = m_x; w = ww; 
				y = m_y; h = hh;
			}
		}

		m_viewportX = x; m_viewportWidth = w;
		m_viewportY = y; m_viewportHeight = h;

		if(x<0||y<0||w<=0||h<=0) return;
	}

		//NEW: OpenGL can't backup its states.
		//ModelViewport might need to do this
		//if it wasn't its own OpenGL context.
		const int attribs 
		=GL_COLOR_BUFFER_BIT
		|GL_CURRENT_BIT
		|GL_DEPTH_BUFFER_BIT
		|GL_ENABLE_BIT
		|GL_LIGHTING_BIT
		|GL_POLYGON_BIT
		|GL_SCISSOR_BIT
		|GL_TEXTURE_BIT
		|GL_TRANSFORM_BIT
		|GL_VIEWPORT_BIT;
		glPushAttrib(attribs);

	//initializeGL()
	{
		// general set-up
		//glEnable(GL_TEXTURE_2D);

		//glShadeModel(GL_SMOOTH); //???
		//glClearColor(0.80f,0.80f,0.80f,1);

		/*NEW: Assuming depth-buffer not required.
		glDepthFunc(GL_LEQUAL);
		glClearDepth(1);*/

		//REMINDER:
		
		// set up lighting
		GLfloat ambient[] = { 0.8f,0.8f,0.8f,1 };
		GLfloat diffuse[] = { 1,1,1,1 };
		GLfloat position[] = { 0,0,3,0 };
		
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,false);
		glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
		glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
		glLightfv(GL_LIGHT0,GL_POSITION,position);

		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
		glEnable(GL_SCISSOR_TEST);
	}

	m_width = ww;
	m_height = hh;
	m_aspect = m_width/m_height; 
	
	double cx,cy,sMin,sMax,tMin,tMax;

	//setViewportDraw();
	{
		glScissor(x,y,w,h);
		glViewport(x,y,w,h); 

		glMatrixMode(GL_PROJECTION);		
		glPushMatrix();
		glLoadIdentity();

		//glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		if(!m_3d)
		{
			//Changing this so excess is rendered.
			//glOrtho(m_xMin,m_xMax,m_yMin,m_yMax,-1,1);
			cx = m_xMax+m_xMin, cy = m_yMax+m_yMin;
			double cw = m_xMax-m_xMin, ch = m_yMax-m_yMin;
			cw*=(double)w/ww; ch*=(double)h/hh;
			cx/=2; cy/=2; cw/=2; ch/=2;
			sMin = cx-cw;
			sMax = cx+cw;
			tMin = cy-ch;
			tMax = cy+ch;			

			//NOTE: This half-pixel calculation was for the
			//m_drawBounding effect to use later. My instinct
			//is it should be applied to the projection too, in
			//which case a half might be wrong for m_drawBounding.
			//The small value is for my Intel chipset. It might not
			//be universally applicable:
			//https://community.khronos.org/t/gl-nearest-frayed-edges-rounding-in-simple-image-editor/104696/3
			
			//This should be roughly a half pixel.
			//The shrinkage eliminates the Intel bug described in
			//the link. Note, the half-pixel offset isn't required.
			//But I assume if Intel's chipset wants to be offsetted
			//it expects a half pixel.
			cx = cw/w*0.95; cy = ch/h*0.95; //0.9995 is too little.
			
			//NOTE: I don't think offsetting a half-pixel matters much
			//here, since it's basically arbitrary. It should match if
			//w/h are the texture's width/height.
			glOrtho(sMin-cx,sMax-cx,tMin+cy,tMax+cy,-1,1);

			//I can't get this to render m_drawBounding square more 
			//than 95% of the time. I think the problem is scrolling
			//is not pegged to screenspace units. I mean, the corners
			//ideally are L shaped. It can be achieved by using screen
			//coordinates instead of UV coordinates, offset accordingly.
			//cx*=2; cy*=2; 
		}
		else gluPerspective(45,m_aspect,0.01,30); //GLU

		glMatrixMode(GL_MODELVIEW);		
		glPushMatrix(); 
		glLoadIdentity();
	}

	//log_debug("paintInternal()\n");
	//log_debug("(%f,%f)%f\n",m_scroll[0],m_scroll[1],m_zoom);

	/*FIX ME (glClearColor(0.80f,0.80f,0.80f,1))
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);*/

	//glLoadIdentity();
	//glEnable(GL_LIGHTING);

	if(m_texture&&!m_solidBackground)
	{		
		//Need to defer this because of GLX crumminess
		//(it won't bind a GL context until onscreen.)
		if(!m_glTexture) initTexture();

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,m_glTexture);
	}
	else glDisable(GL_TEXTURE_2D);

	if(m_solidBackground)
	{
		glColor3f(0,0,0);

		float black[4] = {};
		glMaterialfv(GL_FRONT,GL_AMBIENT,black);
		glMaterialfv(GL_FRONT,GL_DIFFUSE,black);
		glMaterialfv(GL_FRONT,GL_SPECULAR,black);
		glMaterialfv(GL_FRONT,GL_EMISSION,black);
		glMaterialf(GL_FRONT,GL_SHININESS,black[0]);
	}
	else if(m_materialId>=0)
	{
		glColor3f(1,1,1);

		float fval[4];
		m_model->getTextureAmbient(m_materialId,fval);
		glMaterialfv(GL_FRONT,GL_AMBIENT,fval);
		m_model->getTextureDiffuse(m_materialId,fval);
		glMaterialfv(GL_FRONT,GL_DIFFUSE,fval);
		m_model->getTextureSpecular(m_materialId,fval);
		glMaterialfv(GL_FRONT,GL_SPECULAR,fval);
		m_model->getTextureEmissive(m_materialId,fval);
		glMaterialfv(GL_FRONT,GL_EMISSION,fval);
		m_model->getTextureShininess(m_materialId,fval[0]);
		glMaterialf(GL_FRONT,GL_SHININESS,fval[0]);
	}
	else
	{
		float fval[4] = { 0.2f,0.2f,0.2f,1 };
		glMaterialfv(GL_FRONT,GL_AMBIENT,fval);
		fval[0] = fval[1] = fval[2] = 1;
		glMaterialfv(GL_FRONT,GL_DIFFUSE,fval);
		fval[0] = fval[1] = fval[2] = 0;
		glMaterialfv(GL_FRONT,GL_SPECULAR,fval);
		fval[0] = fval[1] = fval[2] = 0;
		glMaterialfv(GL_FRONT,GL_EMISSION,fval);
		glMaterialf(GL_FRONT,GL_SHININESS,0);

		if(!m_texture)
		{
			glColor3f(0,0,0); glDisable(GL_LIGHTING);
		}
		else glColor3f(1,1,1);
	}

	if(m_materialId>=0&&m_model
	&&m_model->getMaterialType(m_materialId)==Model::Material::MATTYPE_COLOR)
	{
		//REMOVE ME
		GLubyte r = m_model->getMaterialColor(m_materialId,0); //???
		GLubyte g = m_model->getMaterialColor(m_materialId,1); //???
		GLubyte b = m_model->getMaterialColor(m_materialId,2); //???
		glDisable(GL_TEXTURE_2D);
		//glDisable(GL_LIGHTING);
		glColor3ub(r,g,b);
	}

	if(m_3d&&m_materialId>=0)
	{	
		/*NEW: Don't assume depth-buffer. 
		//May need to enable back-face culling.
		glEnable(GL_DEPTH_TEST);*/
		glEnable(GL_CULL_FACE);
		//glEnable(GL_COLOR_MATERIAL);

		int ms = getElapsedTimeUI();
		float yRot = ms/4000.0f*360;
		float xRot = ms/8000.0f*360;

		glTranslatef(0,0,-5);
		glRotatef(yRot,0,1,0);
		glRotatef(xRot,1,0,0);

		glBegin(GL_QUADS);

		// Front
		//glColor3ub(255,0,0); //r
		glTexCoord2f(0,0); glNormal3f(0,0,1); glVertex3f(-1,-1,1);
		glTexCoord2f(1,0); glNormal3f(0,0,1); glVertex3f( 1,-1,1);
		glTexCoord2f(1,1); glNormal3f(0,0,1); glVertex3f( 1,+1,1);
		glTexCoord2f(0,1); glNormal3f(0,0,1); glVertex3f(-1,+1,1);

		// Back
		//glColor3ub(255,255,0); //y
		glTexCoord2f(0,0); glNormal3f(0,0,-1); glVertex3f( 1,-1,-1);
		glTexCoord2f(1,0); glNormal3f(0,0,-1); glVertex3f(-1,-1,-1);
		glTexCoord2f(1,1); glNormal3f(0,0,-1); glVertex3f(-1,+1,-1);
		glTexCoord2f(0,1); glNormal3f(0,0,-1); glVertex3f( 1,+1,-1);

		// Left
		//glColor3ub(0,255,0); //g
		glTexCoord2f(0,0); glNormal3f(1,0,0); glVertex3f(1,-1,+1);
		glTexCoord2f(1,0); glNormal3f(1,0,0); glVertex3f(1,-1,-1);
		glTexCoord2f(1,1); glNormal3f(1,0,0); glVertex3f(1,+1,-1);
		glTexCoord2f(0,1); glNormal3f(1,0,0); glVertex3f(1,+1,+1);

		// Right
		//glColor3ub(0,255,255); //c
		glTexCoord2f(0,0); glNormal3f(-1,0,0); glVertex3f(-1,-1,-1);
		glTexCoord2f(1,0); glNormal3f(-1,0,0); glVertex3f(-1,-1,+1);
		glTexCoord2f(1,1); glNormal3f(-1,0,0); glVertex3f(-1,+1,+1);
		glTexCoord2f(0,1); glNormal3f(-1,0,0); glVertex3f(-1,+1,-1);

		// Top
		//glColor3ub(0,0,255); //b*
		glTexCoord2f(0,1); glNormal3f(0,1,0); glVertex3f(-1,1,+1); //4
		glTexCoord2f(1,1); glNormal3f(0,1,0); glVertex3f(+1,1,+1); //3
		glTexCoord2f(1,0); glNormal3f(0,1,0); glVertex3f(+1,1,-1); //2
		glTexCoord2f(0,0); glNormal3f(0,1,0); glVertex3f(-1,1,-1); //1

		// Bottom
		//glColor3ub(255,0,255); //m*
		glTexCoord2f(0,1); glNormal3f(0,-1,0); glVertex3f(+1,-1,+1); //4
		glTexCoord2f(1,1); glNormal3f(0,-1,0); glVertex3f(-1,-1,+1); //3
		glTexCoord2f(1,0); glNormal3f(0,-1,0); glVertex3f(-1,-1,-1); //2
		glTexCoord2f(0,0); glNormal3f(0,-1,0); glVertex3f(+1,-1,-1); //1
																		
		glEnd();
		
		//glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glBegin(GL_QUADS);

		glTexCoord2d(sMin,tMin); glNormal3f(0,0,1);
		glVertex3d(sMin,tMin,0);
		glTexCoord2d(sMax,tMin); glNormal3f(0,0,1);
		glVertex3d(sMax,tMin,0);
		glTexCoord2d(sMax,tMax); glNormal3f(0,0,1);
		glVertex3d(sMax,tMax,0); 
		glTexCoord2d(sMin,tMax); glNormal3f(0,0,1);
		glVertex3d(sMin,tMax,0);

		glEnd();
	}

	//glLoadIdentity();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); 

	//I'm trying here to ensure the border is not drawn to only
	//2 of 4 sides when texturecoord.cc window is first opened.
	//I didn't check, but it's also possible it was obscuring
	//texture pixels on those edges.
	if(m_drawBorder)
	{
		//NOTE: This is to demonstrate the 0/1 edge of the UV space.
		//It should be repeated to form a grid.

		glColor3f(0.7f,0.7f,0.7f);
		glRectd(-cx,-cy,1+cx,1+cy);
	}

	useLinesColor();

	if(m_operation==MouseRange)
	{
		glColor3f(0.7f,0.7f,0.7f);
	}

	if(!m_triangles.empty()) switch(m_drawMode)
	{
	case DM_Edit:

		//TESTING
		//CAUTION: I don't know what the blend mode is
		//at this point?
		//https://github.com/zturtleman/mm3d/issues/95
		//useLinesColor SETS ALPHA, SHARED EDGES BLEND
		//TOGETHER TO BE BRIGHTER. THIS IS UNDESIRABLE.
		glEnable(GL_BLEND);
		//break;

	case DM_Edges:

		glBegin(GL_TRIANGLES);
		drawTriangles();
		glEnd();

		//TESTING
		glDisable(GL_BLEND); 
		break;

	case DM_Filled:
	case DM_FilledEdges:

		glColor3f(0,0,0.8f);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		glBegin(GL_TRIANGLES);
		drawTriangles();
		glEnd();

		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	
		if(DM_FilledEdges!=m_drawMode)
		break;

		glColor3f(1,1,1);
		glBegin(GL_TRIANGLES);
		drawTriangles();
		glEnd();
		break;
	}

	// TODO may want to make "draw points" a separate property
	if(m_operation==MouseRange) useLinesColor();

	/*Note, PaintTextureWin seems to enable MouseRange just so
	//that these vertices are not drawn by default. It's a bit
	//of a HACK obviously.
	*/
	// TODO may want to make "draw points" a separate property
	if(m_operation!=MouseRange||m_drawVertices)
	{
		glPointSize(3);
		glBegin(GL_POINTS);

		for(auto&ea:m_triangles)
		for(int ea2:ea.vertex)
		{
			auto &v = m_vertices[ea2];
			if(v.selected&&m_drawMode==DM_Edit)
			{
				useSelectionColor();
			}
			else useLinesColor();

			//glVertex3f((v.s-m_xMin)/m_zoom,(v.t-m_yMin)/m_zoom,-0.5);
			glVertex3d(v.s,v.t,-0.5);
		}

		glEnd();
	}

	if(m_drawBounding) //drawSelectBox()
	{
		glEnable(GL_COLOR_LOGIC_OP);

		//glColor3ub(255,255,255);
		glColor3ub(0x80,0x80,0x80);
		glLogicOp(GL_XOR);
		glRectd(m_xSel1,m_ySel1,m_xSel2,m_ySel2);
		glDisable(GL_COLOR_LOGIC_OP);
	}
	if(m_operation==MouseRange) //drawRangeBox()
	{		
		glColor3f(1,1,1);
		glRectd(m_xRangeMin-cx,m_yRangeMin-cy,m_xRangeMax+cx,m_yRangeMax+cy);
	}
	if(m_operation==MouseRotate) //drawRotationPoint()
	{
		glColor3f(0,1,0);

		double yoff = m_zoom*0.04;
		double xoff = yoff/m_aspect;
		glBegin(GL_QUADS);
		glVertex2d(m_xRotPoint-xoff,m_yRotPoint);
		glVertex2d(m_xRotPoint,m_yRotPoint-yoff);
		glVertex2d(m_xRotPoint+xoff,m_yRotPoint);
		glVertex2d(m_xRotPoint,m_yRotPoint+yoff);
		glEnd();
	}

	if(m_interactive) 
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		drawOverlay(m_scrollTextures);
	}

	//swapBuffers();

	//glPushAttrib doesn't cover
	//these states.
	glMatrixMode(GL_PROJECTION); 
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW); 
	glPopMatrix();
	glPopAttrib();
}
void TextureWidget::useLinesColor()
{
	//TESTING
	//https://github.com/zturtleman/mm3d/issues/95
	int c = m_linesColor;
	glColor4ub(c>>16&0xff,c>>8&0xff,c>>0&0xff,0x80);
}
void TextureWidget::useSelectionColor()
{
	int c = m_selectionColor;
	glColor3ub(c>>16&0xff,c>>8&0xff,c>>0&0xff);
}
void TextureWidget::sizeOverride(int width, int height)
{
	//m_textureWidget->resize(0,0);
	m_overrideWidth = width; m_overrideHeight = height;
	//updateSize();
}

int TextureWidget::getUvWidth()
{
	return m_texture?m_texture->m_width:1; 
}
int TextureWidget::getUvHeight()
{
	return m_texture?m_texture->m_height:1; 
}
