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


#ifndef __VIEWPANEL_H__
#define __VIEWPANEL_H__

#include "mm3dtypes.h" //PCH
#include "win.h"
#include "mview.h"
#include "tool.h"
#include "texture.h"
#include "texwidget.h" //ScrollWidget

class ViewPanel : public ModelViewport::Parent
{
	friend class MainWin;
	ViewPanel(MainWin &model),~ViewPanel();

public:

	void setModel();

	Model *m_model; //REMOVE ME
	virtual Model *getModel() //REMOVE ME
	{
		return m_model; //return model;
	}

	ViewBar::ModelView *getModelView(unsigned index)
	{
		return index<(unsigned)viewsN?views[index]:nullptr; 
	}

public: //slots:

	//ModelViewport::Parent methods
	virtual void getXY(int&, int &y){ y = shape[1]-y; }
	virtual void viewChangeEvent(ModelViewport&);
	virtual void zoomLevelChangedEvent(ModelViewport&);

	void modelUpdatedEvent() //REMOVE ME
	{
		status.setStats(); //This is somewhat optimized.

		updateAllViews(); //Overkill?
	}

	//TODO: Add 3x1, 4x1, and have "tall" mean Nx1.
	void view1(){ _makeViews(1); }
	void view2(){ _makeViews(2); }
	void view1x2(){ config.set("ui_viewport_tall",0); _makeViews(1*2); }
	void view2x1(){ config.set("ui_viewport_tall",1); _makeViews(2*1); }
	void view2x2(){ _makeViews(2*2); } //void view2x3();
	void view3x2(){ _makeViews(3*2); } //void view3x3();	
	void rearrange(int);

//protected:

	void _deleteViews(),_makeViews(int),_defaultViews(int);

	MainWin &model;

	const int shape[2]; //REMOVE ME

	ViewBar bar1,bar2;

	ViewBar::ParamsBar params;
	Win::bar timeline;

	//enum{ portsN=3*2 };
	//ModelViewport ports[portsN];

	//bool views1x2;
	//size_t viewsM;
	//size_t viewsN;
	ViewBar::ModelView *views[portsN];	

	ViewBar::ModelView *operator->()
	{
		return views[0];
	}

	enum{ memoryN=1+2+2+4+6 };
	ModelViewport::ViewStateT memory[memoryN];
	void _save(int,int);
	bool _recall(int,int);

	//Model *m_model; //REMOVE ME
	//GLuint m_scrollTextures[2]; 

	//int m_focus;
	
	//NOTE: This is used with Ctrl+1 through 9 to save/recall
	//states by viewportSaveStateEvent/viewportRecallStateEvent.
	//enum{ user_statesN='9'-'1'+1 };
	//ModelViewport::ViewStateT user_states[user_statesN];

	ViewBar::StatusBar status;
		
		/*ModelViewport imports*/

	//std::array<int,3> m_backColor; //QColor

	// Tool::Parent methods
	virtual void addBool(bool,bool*,utf8);
	virtual void addInt(bool,int*,utf8,int,int);
	virtual void addDouble(bool,double*,utf8,double,double);
	virtual void addEnum(bool,int*,utf8,const char**);
	virtual void groupParam();
	virtual void updateParams();
	virtual void removeParams();
	virtual void updateView();
	virtual void update3dView(); //NOTE: This is limited to animation.
	virtual void updateAllViews();

	utf8 param_config_key(utf8);

	void draw();
};

#endif // __VIEWPANEL_H
