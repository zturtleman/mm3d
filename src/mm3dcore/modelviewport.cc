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

#include "model.h"
#include "toolbox.h"
#include "tool.h"
#include "glmath.h"

#include "log.h"
#include "modelstatus.h"
#include "texmgr.h"
#include "modelviewport.h"
#include "mm3dport.h"

#include "translate.h"

//https://github.com/zturtleman/mm3d/issues/99
//1.2?
//This seems unrelated to 1.2 used to add margins to
//the view. It must correspond to gluPerspective FOV.
static const double modelviewport_persp_factor = 2*1.2;

void ModelViewport::getXY(int &x, int &y)
{
	parent->getXY(x,y); //circular dependency
}

ModelViewport::ModelViewport(Parent *parent)
	:
parent(parent),
m_operation(MO_None),
m_view(Tool::ViewFront),
m_rotX(),m_rotY(),m_rotZ(),
m_width(),m_height(),
m_unitWidth(1),
m_background(),
m_backgroundTexture(),
m_rendering()
{
	m_zoom = 32;

	m_interactive = true;

	//https://github.com/zturtleman/mm3d/issues/97
	//m_farOrtho = 1000000; //???
	//m_nearOrtho = 0.001; //???
	m_farOrtho = 5000; //???
	m_nearOrtho = -5000; //???
	
	//setAutoBufferSwap(false);

	//setFocusPolicy(Qt::WheelFocus);
	//setMinimumSize(220,180);
   
	//setAcceptDrops(true);
	//setMouseTracking(true); //QWidget

	//setCursor(Qt::ArrowCursor);
}

void ModelViewport::updateMatrix() //NEW
{
	if(m_view<Tool::ViewOrtho
	 &&m_view>Tool::ViewPerspective)
	{
		m_rotX = m_rotY = m_rotZ = 0; 		
		switch(m_view)
		{
		case Tool::ViewBack: m_rotY = 180; break;
		case Tool::ViewLeft: m_rotY = -90; break; //90
		case Tool::ViewRight: m_rotY = 90; break; //-90
		case Tool::ViewTop: m_rotX = -90; break;
		case Tool::ViewBottom: m_rotX = 90; break;
		}
	}

	m_viewMatrix.loadIdentity();

	if(m_view<Tool::ViewOrtho
	&&m_view>Tool::ViewPerspective) //NEW
	{
		m_viewMatrix.setRotationInDegrees(-m_rotX,-m_rotY,-m_rotZ);
	}
	else m_viewMatrix.setRotationInDegrees(m_rotX,m_rotY,m_rotZ);

	double z = -m_scroll[2];
	//https://github.com/zturtleman/mm3d/issues/99
	//NEW: Misfit's persp mode hadn't originally allowed tool work.
	const double aspect = (double)m_viewportWidth/m_viewportHeight;
	if(1&&m_view<=Tool::ViewPerspective)
	{
		if(aspect<=1)
		z-=m_zoom*modelviewport_persp_factor/aspect;
		else
		z-=m_zoom*modelviewport_persp_factor;
	}
	//YUCK: Need to keep ortho matrix even for perspective.
	//m_viewMatrix.setTranslation(-m_scroll[0],-m_scroll[1],-z);
	m_viewMatrix.setTranslation(-m_scroll[0],-m_scroll[1],-m_scroll[2]);

	m_invMatrix = m_viewMatrix.getInverse();

	//NEW: It's probably incorrect to apply a perspective 
	//matrix, but it would produce better selection results.
	if(m_view<=Tool::ViewPerspective)
	{
		double s,t; //updateViewport gets these of whack.
		aspect<1?t=(s=m_zoom)/aspect:s=(t=m_zoom)*aspect;
		m_width = s*2; m_height = t*2;

		//gluPerspective(45,aspect,m_zoom*0.1,m_zoom*1000);
		const double f = 1/tan(PI/8); //45/2
		const double znear = m_zoom*0.1, zfar = m_zoom*1000;
		double proj[4][4] = {};		
		proj[0][0] = f/aspect;
		proj[1][1] = f;
		//Reverse back-face sense from gluPerspective.
		//proj[2][2] = (zfar+znear)/(znear-zfar);
		proj[2][2] = (zfar+znear)/(zfar-znear);
		proj[2][3] = -1; 
		//Reverse back-face sense from gluPerspective.
		//proj[3][2] = 2*zfar*znear/(znear-zfar);
		proj[3][2] = 2*zfar*znear/(zfar-znear);
		
		//Scale to getRawParentXYValue coordinate system.
		proj[0][0]*=m_width/2;
		proj[1][1]*=m_height/2;

		//2019: This is equivalent to doing
		//glLoadMatrix(this);
		//glMultMatrix(lhs);
		//m_viewMatrix.postMultiply(*(Matrix*)proj);
		std::swap(z,m_viewMatrix.getMatrix()[14]); 
		((Matrix*)proj)->postMultiply(m_viewMatrix);
		std::swap(z,m_viewMatrix.getMatrix()[14]); 
		//memcpy(&m_viewMatrix,proj,sizeof(proj));
		memcpy(&m_projMatrix,proj,sizeof(proj));

		m_unprojMatrix = m_projMatrix.getInverse();
	}
	else
	{
		m_projMatrix = m_viewMatrix;
		m_unprojMatrix = m_invMatrix;
	}	

	parent->updateView();
}

static int modelviewport_opts(int drawMode)
{		
	int o = Model::DO_TEXTURE|Model::DO_SMOOTHING;
	/*if(config.get("ui_render_bad_textures",false))
	o|=Model::DO_BADTEX;
	if(config.get("ui_render_backface_cull",false))
	o|=Model::DO_BACKFACECULL;*/
	switch(drawMode)
	{
	case ModelViewport::ViewFlat: o = Model::DO_NONE; break;
	case ModelViewport::ViewSmooth: o = Model::DO_SMOOTHING; break;
	case ModelViewport::ViewAlpha: o|=Model::DO_ALPHA; break;
	}
	return o;
}

void ModelViewport::draw(int x, int y, int w, int h)
{	
	if(1) //Parent::initializeGL(parent->getModel());
	{
		glEnable(GL_DEPTH_TEST);
	}

	Model *model = parent->getModel();

	if(m_rendering)
	{
		assert(!w);
		x = m_viewportX; w = m_viewportWidth;
		y = m_viewportY; h = m_viewportHeight;
	}
	else
	{
		//NEW: Trying to have tools in perspective.
		//HACK: Adapt to new size... assuming user
		//interaction support is all that's needed.
		bool cmp = w!=m_viewportWidth||h!=m_viewportHeight;
		{
			m_viewportX = x; m_viewportWidth = w;
			m_viewportY = y; m_viewportHeight = h;
		}
		//NOTE: Ortho seems to shape invariant.
		if(cmp&&m_view<=Tool::ViewPerspective)
		{
			updateMatrix();
		}
	}

	if(x<0||y<0||w<=0||h<=0) return;
	
	glScissor(x,y,w,h); //NEW

	//if(m_inOverlay) //???
	//void setViewportDraw()
	double aspect = (double)w/h;
	{
		//m_inOverlay = false; //???

		glViewport(x,y,w,h);

		glMatrixMode(GL_PROJECTION); glLoadIdentity();
				
		double s,t;
		aspect<1?t=(s=m_zoom)/aspect:s=(t=m_zoom)*aspect;
		
		if(m_view>Tool::ViewPerspective)
		{
			glOrtho(m_scroll[0]-s,m_scroll[0]+s,
			m_scroll[1]-t,m_scroll[1]+t,m_nearOrtho,m_farOrtho);
		}
		else //gluPerspective(45,aspect,m_zoom*0.002,m_zoom*2000); //GLU
		{
			gluPerspective(45,aspect,m_zoom*0.1,m_zoom*1000); //GLU
		}

		m_width = s*2; m_height = t*2;
				
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	}

	if(m_rendering) //animexportwin? 
	{
		//??? Undoing old focus effect?
		//glClearColor(130.0/256.0,200.0/256.0,200.0/256.0,1);		

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //NEW
	}	

	float viewPoint[4] = { 0,0,0,1 };

	if(m_view>=Tool::ViewOrtho
	 ||m_view<=Tool::ViewPerspective)
	{
		glEnable(GL_LIGHT0);
		
		viewPoint[0] = (float)m_scroll[0]; 
		viewPoint[1] = (float)m_scroll[1]; 
		viewPoint[2] = (float)m_scroll[2]; 
		if(m_view<=Tool::ViewPerspective)
		{
			//https://github.com/zturtleman/mm3d/issues/99
			if(aspect<=1)
			viewPoint[2]+=m_zoom*modelviewport_persp_factor/aspect;
			else
			viewPoint[2]+=m_zoom*modelviewport_persp_factor;

			glTranslatef(-viewPoint[0],-viewPoint[1],-viewPoint[2]);
		}
		else 
		{
			viewPoint[2]+=m_zoom*2;

			//https://github.com/zturtleman/mm3d/issues/97
			//glTranslatef(0,0,-m_farOrtho/2);
			glTranslatef(0,0,-(m_farOrtho+m_nearOrtho)/2);
		}

		glRotated(m_rotZ,0,0,1);
		glRotated(m_rotY,0,1,0);
		glRotated(m_rotX,1,0,0);

		Matrix m; //???
		m.setRotationInDegrees(0,0,-m_rotZ);
		m.apply(viewPoint);
		m.setRotationInDegrees(0,-m_rotY,0);
		m.apply(viewPoint);
		m.setRotationInDegrees(-m_rotX,0,0);
		m.apply(viewPoint);
	}
	else
	{
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHT1);

		//https://github.com/zturtleman/mm3d/issues/97
		viewPoint[0] = 0;
		viewPoint[1] = 0;
		viewPoint[2] = (m_farOrtho+m_nearOrtho)/2; //500000;

		glTranslatef(-viewPoint[0],-viewPoint[1],-viewPoint[2]);

		Matrix m; //???
		switch(m_view)
		{
		case Tool::ViewFront:
			// do nothing
			break;
		case Tool::ViewBack:
			glRotatef(180,0,1,0);
			m.setRotationInDegrees(0,-180,0);
			break;
		case Tool::ViewLeft:
			glRotatef(90,0,1,0); //-90
			m.setRotationInDegrees(0,-90,0); //90
			break;
		case Tool::ViewRight:
			glRotatef(-90,0,1,0); //90
			m.setRotationInDegrees(0,90,0); //-90
			break;
		case Tool::ViewTop:
			glRotatef(90,1,0,0);
			m.setRotationInDegrees(-90,0,0);
			break;
		case Tool::ViewBottom:
			glRotatef(-90,1,0,0);
			m.setRotationInDegrees(90,0,0);
			break;
		//default:
			//log_error("Unknown ViewDirection: %d\n",m_view);
			//swapBuffers();
			//return;
		}
		m.apply(viewPoint);
	}

	glColor3f(0.7f,0.7f,0.7f); //???

	int drawMode;
	bool drawSelections = true;	
	if(m_view<=Tool::ViewPerspective)
	{			
		drawMode = model->getPerspectiveDrawMode();
		if(drawMode!=ViewWireframe)
		{
			//drawSelections = config.get("ui_render_3d_selections",false);
			drawSelections = model->getDrawSelection();
		}
	}
	else
	{
		drawMode = model->getCanvasDrawMode();

		drawBackground(); // Draw background
		
		//glClear(GL_DEPTH_BUFFER_BIT); //???
	}
	glEnable(GL_DEPTH_TEST);
	if(drawMode!=ViewWireframe)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		if(drawSelections)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1,1);
		}

		//ContextT was because every view was its own OpenGL context.
		//model->draw(opt,static_cast<ContextT>(this),viewPoint);
		model->draw(modelviewport_opts(drawMode),nullptr,viewPoint);

		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	glDisable(GL_LIGHTING);
	if(drawSelections)
	{
		//TESTING
		//Trying to create a visual cue.
		bool dl = true;
		bool dv = !parent->tool->isNullTool();
		if(dl) model->drawLines();
		if(dv) model->drawVertices();

		glDisable(GL_DEPTH_TEST);
		model->drawJoints();
	}
	else glDisable(GL_DEPTH_TEST);
	
	model->drawPoints();
	model->drawProjections();

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	if(!m_rendering) //animexportwin?
	{
		drawOrigin();
	}

	glEnable(GL_DEPTH_TEST);
	
	if(!m_rendering) //animexportwin?
	{
		//2019: I'm moving this to after so it isn't
		//blended behind drawLines.
		drawGridLines();
	}
	
	glDisable(GL_DEPTH_TEST);

	//TODO: Probably ought to break this up into two
	//stages, one in model space, one in screen space.
	//The SelectTool should draw in screen space, so
	//it can not use _getParentZoom.
	if(m_view>Tool::ViewPerspective
	 ||model->getDrawSelection()) //TESTING
	{
		parent->drawTool(this);
	}
		
	if(!m_rendering) //if(this->hasFocus())
	{
		//m_inOverlay = true; //???
		drawOverlay(parent->m_scrollTextures);
	}
	//2019: Testing correctness of updateMatrix w/ projection.
	#if 0 && defined(_DEBUG)
	if(m_view<=Tool::ViewPerspective)
	{
		//TODO: Would rather do this to fill a selection buffer.

		glTranslated(w/2,h/2,0);
		glBegin(GL_POINTS);			
		double ww = w/m_width;
		double hh = h/m_height;
		for(int i=0,iN=model->getVertexCount();i<iN;i++)
		{
			//I think gluProject neglects to mention homogeneous
			//divide.
			//https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluProject.xml
			Vector v;
			model->getVertexCoords(i,v.getVector());
			m_projMatrix.apply(v);
			v[0]/=v[3]; v[1]/=v[3];
			glVertex2d(v[0]*ww,v[1]*hh);
		}
		glEnd();
		glLoadIdentity(); //HACK
	}
	#endif

	//REMOVE ME
	//ModelViewport needs to be separate 
	//from the UI.
	Parent::checkGlErrors(model);

	//swapBuffers();
}

void ModelViewport::drawGridLines()
{
	Model *model = parent->getModel();
	Model::ViewportUnits &vu = model->getViewportUnits();

	if(m_view<=Tool::ViewPerspective
	 ||m_view>=Tool::ViewOrtho)
	{
		//double inc = config.get("ui_3dgrid_inc",4.0);
		double inc = vu.inc3d;
		//double max = config.get("ui_3dgrid_count",6)*inc;
		double max = vu.lines3d*inc;
		double x,y,z;

		glColor3f(0.55f,0.55f,0.55f);

		glBegin(GL_LINES);

		//if(config.get("ui_3dgrid_xy",false))
		if(vu.xyz3d&4)
		{
			for(x=-max;x<=max;x+=inc)
			{
				glVertex3d(x,-max,0); glVertex3d(x,+max,0);
			}
			for(y=-max;y<=max;y+=inc)
			{
				glVertex3d(-max,y,0); glVertex3d(+max,y,0);
			}
		}
		//if(config.get("ui_3dgrid_xz",true))
		if(vu.xyz3d&2)
		{
			for(x=-max;x<=max;x+=inc)
			{
				glVertex3d(x,0,-max); glVertex3d(x,0,+max);
			}
			for(z=-max;z<=max;z+=inc)
			{
				glVertex3d(-max,0,z); glVertex3d(+max,0,z);
			}
		}
		//if(config.get("ui_3dgrid_yz",false))
		if(vu.xyz3d&1)
		{
			for(y=-max;y<=max;y+=inc)
			{
				glVertex3d(0,y,-max); glVertex3d(0,y,+max);
			}
			for(z=-max;z<=max;z+=inc)
			{
				glVertex3d(0,-max,z); glVertex3d(0,+max,z);
			}
		}

		glEnd();
	}
	else
	{
		//May be 0. 
		m_unitWidth = getUnitWidth();
		double uw = m_unitWidth;
		if(uw<=0) return;

		glColor3f(0.55f,0.55f,0.55f);

		glBegin(GL_LINES);
		
		double xRangeMin = m_scroll[0];
		double xRangeMax = m_scroll[0];
		double yRangeMin = m_scroll[1];
		double yRangeMax = m_scroll[1];

		switch(m_view)
		{
		case Tool::ViewBack: 
		
		//2019: CHANGING SENSE.
		//case Tool::ViewLeft:
		case Tool::ViewRight:

			xRangeMin*=-1; xRangeMax*=-1; break;
			
		case Tool::ViewTop: 
			
			yRangeMin*=-1; yRangeMax*=-1; break;
		}

		xRangeMin-=m_width/2; yRangeMin-=m_height/2;
		xRangeMax+=m_width/2; yRangeMax+=m_height/2;
		
		double x,xStart = uw*(int)(xRangeMin/uw);
		double y,yStart = uw*(int)(yRangeMin/uw);
		
		//NEW: Fix sometimes visible rounding error?
		xStart-=uw; xRangeMax+=uw;
		yStart-=uw; yRangeMax+=uw;

		//TODO: Don't use double for this? Draw in 2D?
		switch(m_view)
		{
		case Tool::ViewFront: case Tool::ViewBack:
			
			for(x=xStart;x<xRangeMax;x+=uw)
			{
				glVertex3d(x,yRangeMin,0);
				glVertex3d(x,yRangeMax,0);
			}
			for(y=yStart;y<yRangeMax;y+=uw)
			{
				glVertex3d(xRangeMin,y,0);
				glVertex3d(xRangeMax,y,0);
			}
			break;

		case Tool::ViewLeft: case Tool::ViewRight:
			
			for(x=xStart;x<xRangeMax;x+=uw)
			{
				glVertex3d(0,yRangeMin,x);
				glVertex3d(0,yRangeMax,x);
			}
			for(y=yStart;y<yRangeMax;y+=uw)
			{
				glVertex3d(0,y,xRangeMin);
				glVertex3d(0,y,xRangeMax);
			}
			break;

		case Tool::ViewTop: case Tool::ViewBottom:
			
			for(x=xStart;x<xRangeMax;x+=uw)
			{
				glVertex3d(x,0,yRangeMin);
				glVertex3d(x,0,yRangeMax);
			}
			for(y=yStart;y<yRangeMax;y+=uw)
			{
				glVertex3d(xRangeMin,0,y);
				glVertex3d(xRangeMax,0,y);
			}
			break;

		//default: 
		//	log_error("Unhandled view direction: %d\n",m_view);
		//	break;
		}

		glEnd();
			
		/*REMOVE ME
		if(config.get("ui_render_text",false))
		{
			glColor3f(0.35f,0.35f,0.35f);
			glRasterPos3f(xRangeMin,yRangeMin,0);
			switch(m_view)
			{
			case Tool::ViewFront: case Tool::ViewBack:
			
				glRasterPos3f(xRangeMax,yRangeMin,0); break;

			case Tool::ViewLeft: case Tool::ViewRight:
			
				glRasterPos3f(0,yRangeMin,xRangeMin); break;

			case Tool::ViewTop: case Tool::ViewBottom:
			
				glRasterPos3f(xRangeMin,0,yRangeMin); break;
			}

			// Broken with Qt4+non-accelerated nVidia card (possibly other configs as
			// well). That is why it is disabled by default. You can set ui_render_text
			// to a non-zero value in your mm3drc file to enable text rendering.
			QString text; text.sprintf("%g",uw);
			renderText(2,this->height()-12,text,QFont("Sans",10));
		}*/
	}
}
void ModelViewport::drawOrigin()
{
	glDisable(GL_DEPTH_TEST);

	glBegin(GL_LINES);

	double scale = m_zoom/10.0;

	if(m_view<=Tool::ViewPerspective)
	{
		double x = m_scroll[0];
		double y = m_scroll[1];
		double z = m_scroll[2]+m_zoom;
		scale = sqrt(x*x+y*y+z*z)/10.0;
	}

	glColor3f(1,0,0);
	glVertex3f(0,0,0); glVertex3d(scale,0,0);
	glColor3f(0,1,0);
	glVertex3f(0,0,0); glVertex3d(0,scale,0);
	glColor3f(0,0,1);
	glVertex3f(0,0,0); glVertex3d(0,0,scale);

	glEnd();

	glEnable(GL_DEPTH_TEST);
}
void ModelViewport::drawBackground()
{	
	if(!updateBackground()) return;

	Model *model = parent->getModel();
		
	//https://github.com/zturtleman/mm3d/issues/102	
	if(Texture::FORMAT_RGBA==m_background->m_format)
	glEnable(GL_BLEND);

	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);

	int index = m_view-1;
		
	float w = (float)m_background->m_origWidth;
	float h = (float)m_background->m_origHeight;
	float dimMax = std::max(w,h);

	float cenX,cenY,cenZ;
	model->getBackgroundCenter(index,cenX,cenY,cenZ);

	glBindTexture(GL_TEXTURE_2D,m_backgroundTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	//SIMPLIFY ME
	//SIMPLIFY ME
	//SIMPLIFY ME
	float minX,minY,minZ;
	float maxX,maxY,maxZ;
	float normX = 0, normY = 0, normZ = 0;
	float scale = model->getBackgroundScale(index);
	//TODO: Assuming this wants background
	//to slice through 0,0.
	//https://github.com/zturtleman/mm3d/issues/97
	//float ortho = m_farOrtho/2-0.1f; //???
	//float ortho = -0.1f; //???
	float ortho = m_farOrtho-0.01f; //???
	switch(m_view)
	{
	case Tool::ViewFront:
		minZ  =  maxZ = -ortho;
		minX  = -scale *(w/dimMax)+cenX;
		maxX  =  scale *(w/dimMax)+cenX;
		minY  = -scale *(h/dimMax)+cenY;
		maxY  =  scale *(h/dimMax)+cenY;
		normZ =  1;
		break;

	case Tool::ViewBack:
		minZ  =  maxZ = ortho;
		minX  =  scale *(w/dimMax)+cenX;
		maxX  = -scale *(w/dimMax)+cenX;
		minY  = -scale *(h/dimMax)+cenY;
		maxY  =  scale *(h/dimMax)+cenY;
		normZ = -1;
		break;

	//2019: CHANGING SENSE
	case Tool::ViewLeft: //case Tool::ViewRight:
		minX  =  maxX = ortho;
		minZ  = -scale *(w/dimMax)+cenZ;
		maxZ  =  scale *(w/dimMax)+cenZ;
		minY  = -scale *(h/dimMax)+cenY;
		maxY  =  scale *(h/dimMax)+cenY;
		normX =  1;
		break;
	//2019: CHANGING SENSE.
	case Tool::ViewRight: //case Tool::ViewLeft:
		minX  =  maxX = -ortho;
		minZ  =  scale *(w/dimMax)+cenZ;
		maxZ  = -scale *(w/dimMax)+cenZ;
		minY  = -scale *(h/dimMax)+cenY;
		maxY  =  scale *(h/dimMax)+cenY;
		normX = -1;
		break;

	case Tool::ViewTop:
		minY  =  maxY = -(m_farOrtho/2-0.1f);
		minX  = -scale *(w/dimMax)+cenX;
		maxX  =  scale *(w/dimMax)+cenX;
		minZ  =  scale *(h/dimMax)+cenZ;
		maxZ  = -scale *(h/dimMax)+cenZ;
		normY =  1;
		break;

	case Tool::ViewBottom:
		minY  =  maxY = m_farOrtho/2-0.1f;
		minX  = -scale *(w/dimMax)+cenX;
		maxX  =  scale *(w/dimMax)+cenX;
		minZ  = -scale *(h/dimMax)+cenZ;
		maxZ  =  scale *(h/dimMax)+cenZ;
		normY = -1;
		break;
	}

	if(m_view==Tool::ViewLeft||m_view==Tool::ViewRight)
	{
		glNormal3f(normX,normY,normZ);
		glTexCoord2f(0,0);
		glVertex3f(minX, minY,minZ);
		glNormal3f(normX,normY,normZ);
		glTexCoord2f(0,1);
		glVertex3f(maxX, maxY,minZ);
		glNormal3f(normX,normY,normZ);
		glTexCoord2f(1,1);
		glVertex3f(maxX, maxY,maxZ);
		glNormal3f(normX,normY,normZ);
		glTexCoord2f(1,0);
		glVertex3f(minX, minY,maxZ);
	}
	else
	{
		glNormal3f(normX,normY,normZ);
		glTexCoord2f(0,0);
		glVertex3f(minX, minY,minZ);
		glNormal3f(normX,normY,normZ);
		glTexCoord2f(1,0);
		glVertex3f(maxX, minY,minZ);
		glNormal3f(normX,normY,normZ);
		glTexCoord2f(1,1);
		glVertex3f(maxX, maxY,maxZ);
		glNormal3f(normX,normY,normZ);
		glTexCoord2f(0,1);
		glVertex3f(minX, maxY,maxZ);
	}

	glEnd();

	//https://github.com/zturtleman/mm3d/issues/88
	glDisable(GL_TEXTURE_2D);

	//https://github.com/zturtleman/mm3d/issues/102
	glDisable(GL_BLEND);
}
  
double ModelViewport::getUnitWidth()
{
	Model *model = parent->getModel();
	Model::ViewportUnits &vu = model->getViewportUnits();

	if(m_view>=Tool::ViewOrtho
	 ||m_view<=Tool::ViewPerspective)
	{
		//return config.get("ui_3dgrid_inc",4.0);
		return vu.inc3d;
	}

	//double unitWidth = config.get("ui_grid_inc",4.0);
	double unitWidth = vu.inc;

	double ratio;
	int scale_min,scale_max;

	// Note early return for fixed width ('case 2:')
	//switch(config.get("ui_grid_mode",0))
	switch(vu.grid)
	{
	default:
	case vu.BinaryGrid: //0  // Binary
		ratio = 2;
		scale_min = 4; scale_max = 16;
		break;
	case vu.DecimalGrid:  //1 // Decimal
		ratio = 10;
		scale_min = 2; scale_max = 60;
		break;
	case vu.FixedGrid:  //2 // Fixed
		return unitWidth;
		break;
	}

	double maxDimension = m_width>m_height?m_width:m_height;
	while(maxDimension/unitWidth>scale_max)
	{
		unitWidth*=ratio;
	}
	while(maxDimension/unitWidth<scale_min)
	{
		unitWidth/=ratio;
	}
	return unitWidth;
}

bool ModelViewport::updateBackground()
{
	Model *model = parent->getModel();
	const char *file = model->getBackgroundImage(m_view-1);	
	bool ret = *file!='\0';
	bool diff = file!=m_backgroundFile; 
	if(diff) m_backgroundFile = file;	
	if(!diff||!ret) return ret;
		
	if(!m_backgroundTexture) glGenTextures(1,&m_backgroundTexture);

	m_background = TextureManager::getInstance()->getTexture(file,false,false);
	if(!m_background)
	{
		//TRANSLATE("LowLevel","Could not load background %1"))
		model_status(model,StatusError,STATUSTIME_LONG,
		TRANSLATE("LowLevel","Could not load background %s"),file);
		m_background = TextureManager::getInstance()->getDefaultTexture(file);
	}
	glBindTexture(GL_TEXTURE_2D,m_backgroundTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP); //NEW
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP); //NEW

	GLuint format = m_background->m_format;
	format = format==Texture::FORMAT_RGBA?GL_RGBA:GL_RGB;
	
	//https://github.com/zturtleman/mm3d/issues/85
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	/*FIX ME //Mipmaps option?
	glTexImage2D(GL_TEXTURE_2D,0,format,
	m_background->m_width,m_background->m_height,0,
	format,GL_UNSIGNED_BYTE,m_background->m_data.data());*/
	gluBuild2DMipmaps(GL_TEXTURE_2D,format, //GLU
	m_background->m_width,m_background->m_height,							
	format,GL_UNSIGNED_BYTE,m_background->m_data.data()); return true;
}

void ModelViewport::updateViewport(int how)
{
	if(how=='z') //HACK
	{
		parent->zoomLevelChangedEvent(*this);
	
		//REMOVE ME (Invaliding cursor position?)
		m_unitWidth = getUnitWidth();
		char str[80];
		snprintf(str,sizeof(str),"Units: %g",m_unitWidth);
		model_status(parent->getModel(),StatusNormal,STATUSTIME_NONE,str);
	}

	updateMatrix();
}

void ModelViewport::wheelEvent(int wh, int bs, int x, int y)
{
	//bool rotate = (e->modifiers()&Qt::ControlModifier)!=0;
	bool rotate = (bs&Tool::BS_Ctrl)!=0;
	if(wh>0) rotate?rotateClockwise():zoomIn();	
	else rotate?rotateCounterClockwise():zoomOut();
}
bool ModelViewport::mousePressEvent(int bt, int bs, int x, int y)
{
	//printf("press = %d\n",e->button());
	//if(m_activeButton!=Qt::NoButton)
	if(m_activeButton)
	{
		//e->ignore(); //???
		return true;
	}

	//e->accept(); //???
	m_activeButton = bt; //e->button();
	
	int w = m_viewportWidth; //this->width();
	int h = m_viewportHeight; //this->height();

	//bool rotate = (e->modifiers()&Qt::ControlModifier)!=0;
	bool rotate = (bs&Tool::BS_Ctrl)!=0;
	
	if(pressOverlayButton(x,y,rotate))	
	if(m_overlayButton==ScrollButtonPan)
	{
		m_scrollStartPosition = {x,y};
		m_operation = rotate?MO_Rotate:MO_Pan;
	}
	else m_operation = rotate?MO_RotateButton:MO_PanButton;
	else 
	{
		//m_operation = MO_None;
		if(m_operation!=MO_None) return true; //NEW

		//if(e->button()==Qt::MidButton)
		if(bt==Tool::BS_Middle)
		{
			m_operation = MO_Pan;
			m_scrollStartPosition = {x,y};
		}
		else 
		{
			bool persp = m_view<=Tool::ViewPerspective
			&&!parent->getModel()->getDrawSelection();
			bool def = persp||parent->tool->isNullTool();

			//Misift BS_Right did nothing.
			//if(def&&bt==Tool::BS_Left||bs&Tool::BS_Ctrl)
			if(def||bs&Tool::BS_Ctrl)
			{
				m_operation = MO_Rotate;
				m_scrollStartPosition = {x,y};
			}
			else
			{
				m_operation = MO_Tool;
				Tool *tool = parent->tool;
				tool->mouseButtonDown(bt|bs,x-m_viewportX,y-m_viewportY);
			}
		}
	}

	return m_operation!=MO_None;
}
void ModelViewport::mouseMoveEvent(int bs, int x, int y)
{
	int w = m_viewportWidth;
	int h = m_viewportHeight;
	
	if(m_autoOverlay)
	{
		//NOTE: This can't catch when the mouse moves 
		//out of bounds if the cursor is clipped.
		int xx = x-m_viewportX;
		int yy = y-m_viewportY;
		int cmp = xx>=0&&yy>=0&&xx<w&&yy<h?2:1;
		if(cmp!=m_autoOverlay) 
		{
			m_autoOverlay = cmp; parent->updateView();
		}
	}

	//NEW
	if(!m_activeButton) return;

	//EXPERIMENTAL
	//This test filters out multi-pan/rotate via shift.
	bool multipanning = bs&Tool::BS_Shift;
	if(m_operation!=MO_Rotate&&m_operation!=MO_Pan)
	multipanning = false;

	if(!multipanning) if(m_view>Tool::ViewPerspective)
	{
		Vector pos;
		getParentXYValue(x,y,pos[0],pos[1],true);
		m_invMatrix.apply(pos);
		for(int i=0;i<3;i++) 
		if(fabs(pos[i])<0.000001) pos[i] = 0;

		model_status(parent->getModel(),StatusNormal,STATUSTIME_NONE,
		"Units: %g  (%g,%g,%g)",m_unitWidth,pos[0],pos[1],pos[2]);
	}
	else
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_NONE,
		"Units: %g",m_unitWidth); //getUnitWidth());
	}

	if(m_operation==MO_Rotate)
	{
		double xDiff = m_scrollStartPosition[0]-x;
		double yDiff = m_scrollStartPosition[1]-y;

		double rotY = xDiff/w*PI*2;
		double rotX = yDiff/h*PI*2;

		rotateViewport(rotX,-rotY);

		m_scrollStartPosition = {x,y};
	}
	else if(m_operation==MO_Pan)
	{
		//printf("adjusting translation\n");
		
		double xDiff = m_scrollStartPosition[0]-x;
		xDiff*=m_width/w;		
		double yDiff = m_scrollStartPosition[1]-y;
		yDiff*=m_height/h;
	
		m_scroll[0]+=xDiff;
		m_scroll[1]+=yDiff;

		m_scrollStartPosition = {x,y};

		updateMatrix();
	}
	else if(m_operation==MO_Tool)
	{			
		bs|=m_activeButton; //NEW

		//printf("tool mouse event\n");
		Tool *tool = parent->tool;
		tool->mouseButtonMove(bs,x-m_viewportX,y-m_viewportY);
	}
}
void ModelViewport::mouseReleaseEvent(int bt, int bs, int x, int y)
{
	Model *model = parent->getModel();

	//printf("release = %d\n",e->button());
	//if(e->button()!=m_activeButton) return;
	if(bt!=m_activeButton) return;
	
	if(m_overlayButton==ScrollButtonMAX)
	{
		if(m_operation==MO_Tool)
		{
			Tool *tool = parent->tool;
			tool->mouseButtonUp(bt|bs,x-m_viewportX,y-m_viewportY);
			model->operationComplete(TRANSLATE("Tool",tool->getName(0)));

			//REMOVE ME
			//Getting the count is inefficient for this purpose. 
			//Why is this even here?
			/*???
			if(model->getSelectedProjectionCount())
			{
				model->setDrawProjections(true); parent->updateView();
			}
			//???
			if(model->getSelectedBoneJointCount())
			{
				model->setDrawJoints(true); parent->updateView();
			}*/
			assert(model->getDrawJoints()||!model->getSelectedBoneJointCount());
			assert(model->getDrawProjections()||!model->getSelectedProjectionCount());
		}
	}
	else
	{
		m_overlayButton = ScrollButtonMAX;

		//m_scrollTimer->stop();
		extern void scrollwidget_stop_timer(); //REMOVE ME
		scrollwidget_stop_timer();	 

		model_status(model,StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("LowLevel","Use the middle mouse button to drag/pan the viewport"));
	}
	m_activeButton = 0; //Qt::NoButton;
	m_operation = MO_None;
}

bool ModelViewport::keyPressEvent(int bt, int bs, int x, int y)
{
	//bool ctrl = (e->modifiers()&Qt::ControlModifier)!=0;
	bool ctrl = (bs&Tool::BS_Ctrl)!=0;

	if(!m_activeButton) switch(bt)
	{
	//case Qt::Key_Equal: case Qt::Key_Plus:
	case '=': case '+':
	
		ctrl?rotateClockwise():zoomIn(); break;

	//case Qt::Key_Minus: case Qt::Key_Underscore:
	case '-': case '_':
	
		ctrl?rotateCounterClockwise():zoomOut(); break;
	
	//ODD ASSIGNMENT?
	//case Qt::Key_QuoteLeft: //???
	case '`':
	{
		Tool::ViewE newDir = Tool::ViewPerspective;
		if(m_view<=newDir) newDir = Tool::ViewOrtho;

		//emit viewDirectionChanged(newDir);					
		//emit(this,viewDirectionChanged,newDir);
		viewChangeEvent(newDir);
		break;
	}
	//ODD ASSIGNMENT?
	//case Qt::Key_Backslash:
	case '\\':
	{
		Tool::ViewE newDir = Tool::ViewPerspective;
		switch(m_view)
		{
		case Tool::ViewFront: newDir = Tool::ViewBack; break;
		case Tool::ViewBack: newDir = Tool::ViewFront; break;
		case Tool::ViewRight: newDir = Tool::ViewLeft; break;
		case Tool::ViewLeft: newDir = Tool::ViewRight; break;
		case Tool::ViewTop: newDir = Tool::ViewBottom; break;
		case Tool::ViewBottom: newDir = Tool::ViewTop; break;
		case Tool::ViewPerspective: newDir = Tool::ViewOrtho; break;
		case Tool::ViewOrtho: newDir = Tool::ViewPerspective; break;
		}

		//emit viewDirectionChanged(newDir);					
		//emit(this,viewDirectionChanged,newDir);
		viewChangeEvent(newDir);
		break;
	}

	case Tool::BS_Up: //Qt::Key_Up:
		ctrl?rotateUp():scrollUp();
		break;
	case Tool::BS_Down: //Qt::Key_Down
		ctrl?rotateDown():scrollDown();		
		break;
	case Tool::BS_Left: //Qt::Key_Left
		ctrl?rotateLeft():scrollLeft();
		break;
	case Tool::BS_Right: //Qt::Key_Right
		ctrl?rotateRight():scrollRight();
		break;

	case '0':	
		
		m_scroll[0] = m_scroll[1] = m_scroll[2] = 0;
		updateMatrix();
		break;

	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':

		if(ctrl)
		{
			log_debug("set viewport %d\n",bt-'1');

			ViewStateT viewState;
			getViewState(viewState);			

			//emit viewportSaveState(bt-'1',viewState);
			//emit(this,viewportSaveState,bt-'1',viewState);
			parent->viewportSaveStateEvent(bt-'1',viewState);
		}
		else
		{
			log_debug("viewport recall %d\n",bt-'1');
			
			//emit viewportRecallState(bt-'1');
			//emit(this,viewportRecallState,bt-'1');
			parent->viewportRecallStateEvent(*this,bt-'1');
		}
		break;

	default: return false;
	/*default:
		
		QGLWidget::keyPressEvent(e);
		return;*/
	}

	//e->accept(); //???
	return true;
}
void ModelViewport::getViewState(ModelViewport::ViewStateT &viewState)
{				
	viewState.direction = m_view;
	viewState.zoom = m_zoom;
	viewState.rotation[0] = m_rotX;
	viewState.rotation[1] = m_rotY;
	viewState.rotation[2] = m_rotZ;
	viewState.translation[0] = m_scroll[0];
	viewState.translation[1] = m_scroll[1];
	viewState.translation[2] = m_scroll[2];
}

void ScrollWidget::rotateUp()
{
	rotateViewport(-15*PIOVER180,0,0);
}
void ScrollWidget::rotateDown()
{
	rotateViewport(15*PIOVER180,0,0);
}
void ScrollWidget::rotateLeft()
{
	rotateViewport(0,-15*PIOVER180,0);
}
void ScrollWidget::rotateRight()
{
	rotateViewport(0,15*PIOVER180,0);
}
void ScrollWidget::rotateClockwise()
{
	rotateViewport(0,0,-15*PIOVER180);
}
void ScrollWidget::rotateCounterClockwise()
{
	rotateViewport(0,0,15*PIOVER180);
}
void ModelViewport::rotateViewport(double rotX, double rotY, double rotZ)
{
	if(fabs(rotX)<=0.00001
	 &&fabs(rotY)<=0.00001
	 &&fabs(rotZ)<=0.00001) return;
	 	
	if(m_view>Tool::ViewPerspective&&m_view<Tool::ViewOrtho)
	{
		//emit viewDirectionChanged(Tool::ViewOrtho);
		//emit(this,viewDirectionChanged,Tool::ViewOrtho);
		viewChangeEvent(Tool::ViewOrtho);
	}

	double rot[3] = { m_rotX*PIOVER180,m_rotY*PIOVER180,m_rotZ*PIOVER180 };

	Matrix mcur,mcurinv;
	mcur.setRotation(rot);
	mcurinv = mcur.getInverse();
	mcur.inverseRotateVector(m_scroll);

	Vector xvec(1,0,0,0);
	Vector yvec(0,1,0,0);
	Vector zvec(0,0,1,0);

	Matrix mx,my,mz; //???

	zvec = zvec*mcurinv;
	mz.setRotationOnAxis(zvec.getVector(),rotZ);
	yvec = yvec*mcurinv;
	my.setRotationOnAxis(yvec.getVector(),rotY);
	xvec = xvec*mcurinv;
	mx.setRotationOnAxis(xvec.getVector(),rotX);

	mcur = mx*mcur;
	mcur = my*mcur;
	mcur = mz*mcur;
	mcur.getRotation(rot);
	m_rotX = rot[0]/PIOVER180;
	m_rotY = rot[1]/PIOVER180;
	m_rotZ = rot[2]/PIOVER180;

	//m_rotY += xDiff *PIOVER180 *14.0; //???
	//m_rotX += yDiff *PIOVER180 *14.0; //???

	mcur.apply3(m_scroll);

	// And finally,update the view
	updateMatrix();
}

void ModelViewport::setViewState(const ViewStateT &viewState)
{
	m_view = viewState.direction;
	m_zoom = viewState.zoom;
	m_rotX = viewState.rotation[0];
	m_rotY = viewState.rotation[1];
	m_rotZ = viewState.rotation[2];
	m_scroll[0] = viewState.translation[0];
	m_scroll[1] = viewState.translation[1];
	m_scroll[2] = viewState.translation[2];
		
	updateMatrix();

	parent->viewChangeEvent(*this); //NEW
	parent->zoomLevelChangedEvent(*this); //NEW
}

void ModelViewport::viewChangeEvent(Tool::ViewE dir)
{
	log_debug("viewChangeEvent(%d)\n",dir);

	if(dir==m_view) return;

	if(m_view<=Tool::ViewPerspective)
	{
		Matrix m;
		m.setRotationInDegrees(0,0,-m_rotZ);
		m.apply3(m_scroll);
		m.setRotationInDegrees(0,-m_rotY,0);
		m.apply3(m_scroll);
		m.setRotationInDegrees(-m_rotX,0,0);
		m.apply3(m_scroll);
	}
	else m_invMatrix.apply3(m_scroll); //FIX ME: Try to stabilize?

	log_debug("center point = %f,%f,%f\n",m_scroll[0],m_scroll[1],m_scroll[2]);

	bool toFree = false;
	bool isFree = dir<=Tool::ViewPerspective||dir>=Tool::ViewOrtho;

	switch(m_view)
	{
	case Tool::ViewFront: case Tool::ViewBack:
	case Tool::ViewRight: case Tool::ViewLeft: 
	case Tool::ViewTop: case Tool::ViewBottom: 
		
		toFree = isFree;
	}

	if(toFree) //???
	{
		log_debug("inverting rotation\n");
		m_rotX = -m_rotX;
		m_rotY = -m_rotY;
		m_rotZ = -m_rotZ;
	}
	else if(!isFree)
	{
		m_rotX = m_rotY = m_rotZ = 0;

		switch(dir)
		{
		case Tool::ViewBack: m_rotY = 180; break;
		case Tool::ViewLeft: m_rotY = -90; break; //90
		case Tool::ViewRight: m_rotY = 90; break; //-90
		case Tool::ViewTop: m_rotX = -90; break;
		case Tool::ViewBottom: m_rotX = 90; break;
		}
	}

	Matrix m; if(isFree&&!toFree)
	{
		m.setRotationInDegrees(m_rotX,m_rotY,m_rotZ);
	}
	else m.setRotationInDegrees(-m_rotX,-m_rotY,-m_rotZ);

	if(toFree) m = m.getInverse();

	m.apply3(m_scroll);
	log_debug("after point = %f,%f,%f\n",m_scroll[0],m_scroll[1],m_scroll[2]);

	m_view = dir;
	
	updateMatrix();

	parent->viewChangeEvent(*this); //NEW
}

void ModelViewport::getParentXYValue(int x, int y, double &xval, double &yval, bool selected)
{
	Model *model = parent->getModel();

	getRawParentXYValue(x,y,xval,yval);

	double maxDist = 4.1/m_viewportWidth*m_width;

	Model::ViewportUnits &vu = model->getViewportUnits();

	//if(config.get("ui_snap_vertex",false))
	if(vu.snap&vu.VertexSnap)
	{
		// snap to vertex

		double curDist = maxDist;
		int i,iN,curIndex = -1;
		Model::PositionTypeE curType = Model::PT_Vertex;
		const Matrix &mat = m_viewMatrix;
		double coord[3] = { 0,0,0 };
		double saveCoord[3] = { 0,0,0 };

		auto f = [&](Model::PositionTypeE pt)
		{
			mat.apply3(coord);
			coord[0]+=mat.get(3,0);
			coord[1]+=mat.get(3,1);
			coord[2]+=mat.get(3,2);

			double xdiff = coord[0]-xval;
			double ydiff = coord[1]-yval;
			double dist = distance(xdiff,ydiff,0.0,0.0);

			if(dist<curDist)
			{
				curDist = dist;
				curIndex = i;
				curType = pt;
				saveCoord[0] = coord[0];
				saveCoord[1] = coord[1];
				saveCoord[2] = coord[2];
			}
		};

		iN = model->getVertexCount();
		for(i=0;i<iN;i++)
		if(selected||!model->isVertexSelected(i))
		{
			model->getVertexCoords(i,coord);

			f(Model::PT_Vertex);
		}

		iN = model->getBoneJointCount();
		for(i=0;i<iN;i++)		
		if(selected||!model->isBoneJointSelected(i))
		{
			model->getBoneJointCoords(i,coord);

			f(Model::PT_Joint);
		}

		iN = model->getPointCount();
		for(i=0;i<iN;i++)		
		if(selected||!model->isPointSelected(i))
		{
			model->getPointCoords(i,coord);

			f(Model::PT_Point);
		}

		iN = model->getProjectionCount();
		for(i=0;i<iN;i++)
		if(selected||!model->isProjectionSelected(i))
		{
			model->getProjectionCoords(i,coord);

			f(Model::PT_Projection);
		}

		if(curIndex>=0)
		{
			xval = saveCoord[0]; yval = saveCoord[1];
			maxDist = 0;
		}
	}

	//if(config.get("ui_snap_grid",false))
	if(vu.snap&vu.UnitSnap)
	if(m_view<Tool::ViewOrtho)		
	{
		// snap to grid

		double fudge = 0.5;
		double x = xval+m_scroll[0];
		if(x<0) fudge = -0.5;

		int mult = (int)(x/m_unitWidth+fudge);
		double val = mult*m_unitWidth;

		if(fabs(x-val)<maxDist)
		{
			xval = val-m_scroll[0];
		}

		fudge = 0.5;
		double y = yval+m_scroll[1];
		if(y<0) fudge = -0.5;

		mult = (int)(y/m_unitWidth+fudge);
		val = mult*m_unitWidth;

		if(fabs(y-val)<maxDist) yval = val-m_scroll[1];
	}
}

void ModelViewport::frameArea(bool lock, double x1, double y1, double z1, double x2, double y2, double z2)
{
	if(lock) //2019
	//FIX ME
	//This needs more thought if ortho/persp views are
	//to be locked. I think probably everything should
	//be locked always unless the Home function should
	//apply to one viewport.
	//if(m_view<Tool::ViewOrtho
	// &&m_view>Tool::ViewPerspective)
	{
		//HACK: This is just avoiding locking if 
		//there is less than two 2D views. 

		lock = false;
		
		for(int i=0;i<parent->viewsN;i++)
		{
			ModelViewport &other = parent->ports[i];

			if(other.m_view<Tool::ViewOrtho
			 &&other.m_view>Tool::ViewPerspective)
			{
				//if(this!=&other)
				if(other.m_view!=m_view)
				{
					//Could do more analysis at this point.
					//Or mutual locking can a user managed 
					//deal.

					//Looks best at start up.
					//NOTE: At time of writing this, every
					//use case uses lock.
					lock = true;
				}
			}
		}
	}

	m_scroll[0] = (x1+x2)/2;
	m_scroll[1] = (y1+y2)/2;
	m_scroll[2] = (z1+z2)/2;

	double width = std::max(fabs(x1-x2),0.0001667);
	double height = std::max(fabs(y1-y2),0.0001667);
	double depth = std::max(fabs(z1-z2),0.0001667);

	Vector bounds(width,height,depth); 
	
	bounds.scale(1.2);

	m_viewMatrix.apply3(m_scroll);
	m_viewMatrix.apply3(bounds);

	width = fabs(bounds[0]); height = fabs(bounds[1]);

	//NEW: Make zoom consistent for all viewports? Certainly
	//looks better?
	if(lock) width = std::max(width,fabs(bounds[2]));

	m_zoom = std::max(width,height)/2;	

	//QString zoomStr;
	//zoomStr.sprintf("%f",m_zoom);	
	//emit zoomLevelChanged(zoomStr);
	//emit(this,zoomLevelChanged,m_zoom);
	parent->zoomLevelChangedEvent(*this);

	updateMatrix(); //NEW
}

ModelViewport::Parent::Parent()
	:
	ports{this,this,this,this,this,this}, //C++11
	views1x2(),viewsM(),viewsN(),
	m_scrollTextures(),
	m_focus() 
{	
	ports[0].initOverlay(m_scrollTextures);

	//Let derived class call this in case timing is an issue.
	//initializeGL(getModel());
}

void ModelViewport::Parent::initializeGL(Model *m)
{		
	//NOTE: ViewPanel calls initializeGL to initialize its OpenGL context.
	//Ideally ViewPanel is a thin UI class. This gets some code out of it. 

	glEnable(GL_SCISSOR_TEST); //NEW

	//TODO: Get from Model settings.
	int backColor[3] = {130,200,200};

	glShadeModel(GL_SMOOTH);
	glClearColor
	(backColor[0]/255.0f
	,backColor[1]/255.0f
	,backColor[2]/255.0f,1);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

	{
		GLfloat ambient[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
		GLfloat diffuse[]  = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat position[] = { 0.0f, 0.0f, 1.0f, 0.0f };

		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
		glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
		glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
		glLightfv(GL_LIGHT0,GL_POSITION,position);
		//glEnable(GL_LIGHT0);
	}

	{
		GLfloat ambient[]  = { 0.8f, 0.4f, 0.4f, 1.0f };
		GLfloat diffuse[]  = { 0.9f, 0.5f, 0.5f, 1.0f };
		GLfloat position[] = { 0.0f, 0.0f, 1.0f, 0.0f };

		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
		glLightfv(GL_LIGHT1,GL_AMBIENT,ambient);
		glLightfv(GL_LIGHT1,GL_DIFFUSE,diffuse);
		glLightfv(GL_LIGHT1,GL_POSITION,position);
		//glEnable(GL_LIGHT1);
	}

	{
		GLint texSize = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&texSize);
		log_debug("max texture size is %dx%d\n",texSize,texSize);
	}

	checkGlErrors(m);

	if(1) //#define MM3D_ENABLEALPHA
	{
		//NOTE: model_draw.cc controls GL_BLEND.

		//https://github.com/zturtleman/mm3d/issues/102
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		if(!glIsEnabled(GL_BLEND))
		{
			log_warning("alpha not supported\n");
			//glDisable(GL_BLEND); //???
			glGetError(); // clear errors
		}
		glDisable(GL_BLEND); //NEW
	}
}
void ModelViewport::Parent::checkGlErrors(Model *m)
{
	const char *e; switch(glGetError())
	{
	case 0: return;
	case GL_INVALID_VALUE:
	e = TRANSLATE_NOOP("LowLevel","OpenGL error = Invalid Value"); break;
	case GL_INVALID_ENUM: 
	e = TRANSLATE_NOOP("LowLevel","OpenGL error = Invalid Enum"); break;
	case GL_INVALID_OPERATION: 
	e = TRANSLATE_NOOP("LowLevel","OpenGL error = Invalid Operation"); break;
	case GL_STACK_OVERFLOW: 
	e = TRANSLATE_NOOP("LowLevel","OpenGL error = Stack Overflow"); break;
	case GL_STACK_UNDERFLOW:
	e = TRANSLATE_NOOP("LowLevel","OpenGL error = Stack Underflow"); break;
	case GL_OUT_OF_MEMORY: 
	e = TRANSLATE_NOOP("LowLevel","OpenGL error = Out Of Memory"); break;
	default: 
	e = TRANSLATE_NOOP("LowLevel","OpenGL error = Unknown"); break;
	}
	model_status(m,StatusNormal,STATUSTIME_NONE,e);
}

void ModelViewport::Parent::getXYZ(int x, int y, double *xx, double *yy, double *zz)
{
	ModelViewport &mvp = ports[m_focus];
	Vector vec;
	mvp.getRawParentXYValue(x,y,vec[0],vec[1]);
	mvp.m_invMatrix.apply(vec);
	*xx = vec[0]; *yy = vec[1]; *zz = vec[2];
}

void ModelViewport::Parent::viewportSaveStateEvent(int slot, const ModelViewport::ViewStateT &viewState)
{
	if(slot>=0&&slot<user_statesN) user_states[slot] = viewState;
}
void ModelViewport::Parent::viewportRecallStateEvent(ModelViewport &mvp, int slot)
{
	if(slot>=0&&slot<user_statesN&&user_states[slot])
	{
		int i = &mvp-ports;		
		ports[&mvp-ports].setViewState(user_states[slot]);
	}
}