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

#include "viewwin.h"
 
static std::map<Model*,StatusObject*> statusbar_map;

extern StatusObject *statusbar_get_object(Model *model)
{
	auto it = statusbar_map.find(model);
	return it!=statusbar_map.end()?it->second:nullptr;
}

ViewBar::ViewBar::StatusBar::~StatusBar()
{
	statusbar_map.erase(m_model);
}

void ViewBar::StatusBar::setModel(Model *swap)
{
	if(m_model!=swap) //NEW
	{
		if(m_model) //NEW
		statusbar_map.erase(m_model);

		if(m_model=swap) //NEW
		statusbar_map[m_model] = this;

		if(swap) setStats(); //NEW
	}
}

static void statusbar_timer(int id)
{
	if(auto w=(ViewBar*)Widgets95::e::find_ui_by_window_id(id))
	w->model.views.status.timer_expired();
}
void ViewBar::StatusBar::timer_expired()
{
	if(m_queue.empty())
	{
		m_queueDisplay = false; return;
	}

	TextQueueItemT tqi = m_queue.front();
	m_queue.pop_front();

	setText(tqi.str.c_str());
	if(tqi.type==StatusError) text.select_all();

	m_queueDisplay = true;
		
	if(tqi.ms>0)
	{
		glutTimerFunc(tqi.ms,statusbar_timer,nav.ui()->glut_window_id());
	}
	else timer_expired(); //RECURSIVE
}
void ViewBar::StatusBar::setText(utf8 str)
{
	//This is causing a problem with the status bar 
	//constantly redrawing that slows down the main
	//window's dragging on Windows. The text in the
	//beginning prevents the bug from being noticed.
	if(str&&str!=text.text()) text.set_text(str); 
}
void ViewBar::StatusBar::addText(StatusTypeE type, int ms, const char *str)
{
	if(!m_queueDisplay)
	{	
		m_queueDisplay = true;

		setText(str);
		if(type==StatusError) text.select_all();

		glutTimerFunc(ms,statusbar_timer,nav.ui()->glut_window_id());
	}
	else //FIX ME (Looks like errors are hidden from user?)
	{
		// Clear non-errors first
		size_t max_queue_size = 2;
		bool removing = true;
		while(removing&&m_queue.size()>max_queue_size)
		{
			removing = false;
			for(auto it=m_queue.begin();it!=m_queue.end();)			
			if(it->type!=StatusError)
			{
				removing = true;
				it = m_queue.erase(it);				
			}
			else it++;
		}

		// If we still have more than max_queue_size in the queue,and the
		// new message is an error,start removing the oldest errors.
		if(m_queue.size()>max_queue_size)
		{
			if(type==StatusError)
			while(m_queue.size()>max_queue_size)
			m_queue.pop_front();
			else return;
		}

		TextQueueItemT tqi;
		tqi.str = str;
		tqi.ms = ms;
		tqi.type = type;
		m_queue.push_back(tqi);
	}
}
void ViewBar::StatusBar::setStats()
{	
	int d[6][2] = 
	{
		{m_model->getVertexCount(),m_model->getSelectedVertexCount()},
		{m_model->getTriangleCount(),m_model->getSelectedTriangleCount()},
		{m_model->getGroupCount()},
		{m_model->getBoneJointCount(),m_model->getSelectedBoneJointCount()},
		{m_model->getPointCount(),m_model->getSelectedPointCount()},
		{m_model->getTextureCount()},
	};

	//ViewPanel::modelUpdatedEvent calls setStats 
	//heavily/blindly.
	int compile[sizeof(d)==sizeof(_curstats)];	
	if(memcmp(_curstats,d,sizeof(d)))
	{
		memcpy(_curstats,d,sizeof(_curstats));
	}
	else return; (void)compile;

	utf8 s[6] = 
	{
		::tr("V:","Vertices status bar label"),
		::tr("F:","Faces status bar label"),
		::tr("G:","Groups status bar label"),
		::tr("B:","Bone Joints status bar label"),
		::tr("P:","Points status bar label"),
		::tr("M:","Materials status bar label"),
	};		
	char buf[33];	
	std::string &st = stats.name();
	st.clear();	
	for(int i=0;i<sizeof(s)/sizeof(*s);i++)
	{
		st+=s[i]; if(d[i][1])
		{
			snprintf(buf,sizeof(buf),"%d",d[i][1]);
			st+=buf;
			st+='/';
		}
		snprintf(buf,sizeof(buf),"%d",d[i][0]);
		st+=buf;
		st+=' ';
	}
	st.pop_back(); stats.repack();
}


