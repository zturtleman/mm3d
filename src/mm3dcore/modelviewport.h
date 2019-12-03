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


#ifndef __MVIEWPORT_H
#define __MVIEWPORT_H

#include "texwidget.h" //ScrollWidget
#include "tool.h" //Tool::Parent

class ModelViewport : public ScrollWidget
{				
public:

	class Parent;

	Parent *const parent;

	ModelViewport(Parent*);

	void draw(int frameX, int frameY, int width, int height);  //NEW	

	void render() //animexportwin
	{
		m_rendering = true; draw(0,0,0,0);
		m_rendering = false;
	}

	//Note: These Qt events are now made to take
	//Tool::ButtonState values to be independent.
	virtual void getXY(int &x, int &y);
	virtual bool mousePressEvent(int bt, int bs, int x, int y);
	virtual void mouseReleaseEvent(int bt, int bs, int x, int y);
	virtual void mouseMoveEvent(int bs, int x, int y);
	virtual void wheelEvent(int wh, int bs, int x, int y);
	virtual bool keyPressEvent(int bt, int bs, int x, int y);
		
	/*REFERENCE
	static int constructButtonState(QMouseEvent *e)
	{
		int button = 0;
		if(e->modifiers()&Qt::ShiftModifier)
		button|=Tool::BS_Shift;
		if(e->modifiers()&Qt::AltModifier)
		button|=Tool::BS_Alt;
		if(e->modifiers()&Qt::ControlModifier)
		button|=Tool::BS_Ctrl;
		switch(m_activeButton)
		{
		case Qt::LeftButton: return button|Tool::BS_Left;
		case Qt::MidButton: return button|Tool::BS_Middle;
		case Qt::RightButton: return button|Tool::BS_Right;
		}
		return button;
	}*/

public:

	//NOTE: These are Model::m_canvasDrawMode
	//and Model::m_perspectiveDrawMode.
	enum ViewOptionsE
	{
		ViewWireframe,
		ViewFlat,
		ViewSmooth,
		ViewTexture,
		ViewAlpha
	};

	enum MouseOperationE
	{
		MO_None,
		MO_Tool,
		MO_Pan,
		MO_PanButton,
		MO_Rotate,
		MO_RotateButton,
	};

	struct ViewStateT
	{
		Tool::ViewE direction;
		double zoom;
		double rotation[3];
		double translation[3];
		void copy_transformation(ViewStateT &cp)
		{
			memcpy(rotation,cp.rotation,3+3);
		}

		operator bool(){ return zoom!=0; }
		ViewStateT(){ memset(this,0x00,sizeof(*this)); }
	};

	void frameArea(bool lock, double x1, double y1, double z1, double x2, double y2, double z2);
		
	// Tool::Parent methods		
	void getParentXYValue(int x, int y, double &xval, double &yval,bool selected);
	void getRawParentXYValue(int x, int y, double &xval, double &yval)
	{
		xval =  x/(double)m_viewportWidth*m_width-m_width/2;
		yval = y/(double)m_viewportHeight*m_height-m_height/2;
	}

public:

	//RENAME ME
	void viewChangeEvent(Tool::ViewE dir);
	void setViewState(const ModelViewport::ViewStateT &viewState);
	void getViewState(ModelViewport::ViewStateT &viewState);

	Tool::ViewE getView(){ return m_view; }

protected:
	
	friend class Parent;

	//ScrollWidget methods
	virtual void updateViewport(int=0); 
	virtual void rotateViewport(double x, double y, double z=0);

	void updateMatrix(); //NEW
	
	bool updateBackground();
	
	void drawGridLines();
	void drawOrigin();
	void drawBackground();
	void freeBackground() //UNUSED
	{
		//Removing ~ModelViewport since it's hard to coordinate
		//this with OpenGL context switching.
		glDeleteTextures(1,&m_backgroundTexture);
		m_backgroundTexture = 0;
	}

	double getUnitWidth();

	MouseOperationE m_operation;

	Tool::ViewE m_view;

	//2019: m_projMatrix do perspective correct selection.
	//Ideally this matrix would be used by all tools, but
	//other tools must use the ortho matrix for right now.
	Matrix m_viewMatrix,m_invMatrix;
	Matrix m_projMatrix,m_unprojMatrix;

	double m_rotX;
	double m_rotY;
	double m_rotZ;
	double m_width;
	double m_height;
	double m_unitWidth;

	Texture *m_background;
	GLuint m_backgroundTexture;
	std::string m_backgroundFile;

	bool m_rendering; //animexportwin

	//REMOVE ME?
	std::array<int,2> m_scrollStartPosition; //QPoint
};

//WORK-IN-PROGRESS
class ModelViewport::Parent : public Tool::Parent
{
	//This class hoists a lot of code that does
	//not depend on how the UI is realized from
	//viewpanel.cc.

	friend class ModelViewport;

protected:

	Parent();

	static void initializeGL(Model*),checkGlErrors(Model*);

public:

	//NOTE: lock is new.
	void frameArea(bool lock, double x1, double y1, double z1, double x2, double y2, double z2)
	{
		for(int i=0;i<viewsN;i++) ports[i].frameArea(lock,x1,y1,z1,x2,y2,z2);
	}

public: //slots:

	virtual void getXY(int&,int&) = 0;
	virtual void viewChangeEvent(ModelViewport&) = 0;
	virtual void zoomLevelChangedEvent(ModelViewport&) = 0;	
	void viewportSaveStateEvent(int,const ModelViewport::ViewStateT&);
	void viewportRecallStateEvent(ModelViewport&,int);

//protected:
		
	//NOTE: Misfit has 3x3 but for good
	//grief that's impractical.
	enum{ portsN=3*2 };
	ModelViewport ports[portsN];

	bool views1x2;
	int viewsM;
	int viewsN;
	//ViewBar::ModelView *views[portsN];	

	GLuint m_scrollTextures[2]; //REMOVE ME	
	void freeOverlay() //UNUSED
	{
		//Removing ~Parent since it's hard to coordinate
		//this with OpenGL context switching.
		glDeleteTextures(1,m_scrollTextures);
		m_scrollTextures[0] = 0;
	}

	int m_focus;
	
	//NOTE: This is used with Ctrl+1 through 9 to save/recall
	//states by viewportSaveStateEvent/viewportRecallStateEvent.
	enum{ user_statesN='9'-'1'+1 };
	ModelViewport::ViewStateT user_states[user_statesN];
		
		/*ModelViewport imports*/

	inline void drawTool(ModelViewport *p)
	{
		int swap = m_focus;
		m_focus = p-ports; tool->draw(swap==m_focus);
		m_focus = swap;

	}

	// Tool::Parent methods
	virtual Tool::ViewE getView()
	{
		return ports[m_focus].m_view; 
	}
	virtual void getParentXYValue(int x, int y, double &xval, double &yval, bool selected)
	{
		ports[m_focus].getParentXYValue(x,y,xval,yval,selected);
	}
	virtual void getRawParentXYValue(int x, int y, double &xval, double &yval)
	{
		ports[m_focus].getRawParentXYValue(x,y,xval,yval);
	}
	virtual const Matrix &getParentViewMatrix()const
	{
		if(tool->m_type==Tool::TT_SelectTool)
		return ports[m_focus].m_projMatrix; 
		return ports[m_focus].m_viewMatrix; 
	}
	virtual const Matrix &getParentViewInverseMatrix()const
	{
		if(tool->m_type==Tool::TT_SelectTool)
		return ports[m_focus].m_unprojMatrix; 
		return ports[m_focus].m_invMatrix; 
	}
	virtual double _getParentZoom()const //TESTING
	{
		return ports[m_focus].m_zoom;
	} 
	virtual void getXYZ(int x, int y, double*,double*,double*);
};

#endif // __MVIEWPORT_H
