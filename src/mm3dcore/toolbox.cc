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

#include "toolbox.h"
#include "log.h"

#include "pixmap/nulltool.xpm"

// Concerned about initialization order of toolbox_null & toolbox_sep.
int Tool::s_allocated = 0; 

static struct ToolSeparator : Tool
{
public:

	ToolSeparator():Tool(TT_Separator,0)
	{
		s_allocated--;
	}

	const char *getName(int){ return nullptr; }

	const char **getPixmap(int){ return nullptr; }

	void mouseButtonDown(int buttonState, int x, int y){}
	void mouseButtonMove(int buttonState, int x, int y){}

	virtual void release(){} //NEW

}toolbox_sep;

// This class is here so that we can always assume we have a tool selected,
// even if there isn't one (we can always play with the current tool without
// worrying about nullptr pointers).
static struct NullTool : Tool
{
	NullTool():Tool(TT_NullTool,1)
	{
		s_allocated--;
	}

	//2019: This tool is actually in use by my fork.
	virtual const char *getName(int)
	{
		return "None"; 
	}
	virtual const char *getKeymap(int arg)
	{
		return "Esc"; //May want to rethink this at some point.
	}
	virtual const char **getPixmap(int)
	{
		return (const char**)nulltool_xpm; 
	}
	
	virtual void mouseButtonDown(int buttonState, int x, int y){}
	virtual void mouseButtonMove(int buttonState, int x, int y){}

	virtual void release(){} //NEW

}toolbox_null;

void Toolbox::setCurrentTool(Tool *tool)
{
	m_current = tool?tool:&toolbox_null;
}

Toolbox::Toolbox()
:m_current(&toolbox_null)
,m_toolIt(m_tools.begin())
{}
Toolbox::~Toolbox()
{
	ToolList::iterator it;
	for(it = m_tools.begin(); it!=m_tools.end(); it++)
	{
		(*it)->release();
	}
	m_tools.clear(); //REMOVE ME

	log_debug("active tool count: %d\n",Tool::s_allocated);
}

void Toolbox::addTool(Tool *tool, bool separate)
{
	if(auto p=tool->getPath())
	for(auto it=m_tools.rbegin();it!=m_tools.rend();it++)	
	if(auto q=(*it)->getPath())
	{
		if(q!=p&&strcmp(p,q)) continue;

		(const void*&)tool->m_path = q; //SIMPLIFY THINGS

		//HACK: Enable limited separators in submenus for
		//stdtools.cc macros.
		if(m_tools.back()->isSeparator()) 
		{
			it--; assert(it==m_tools.rbegin());
		}

		auto sep = m_tools.insert(it.base(),tool); 
		
		//NOTE: Plugin tools should separate the first tool.
		if(separate) m_tools.insert(sep,&toolbox_sep);

		return;
	}
	m_tools.push_back(tool); 
}
void Toolbox::addSeparator()
{
	m_tools.push_back(&toolbox_sep); 
}

static std::vector<void(*)(Toolbox*)> toolbox_registry;

void Toolbox::registerToolFunction(NewToolFunc f)
{	
	toolbox_registry.push_back(f);
}

void Toolbox::registerAllTools()
{
	assert(m_tools.empty()); //2019

	for(auto ea:toolbox_registry) ea(this);
}

Tool *Toolbox::getFirstTool()
{
	m_toolIt = m_tools.begin();
	return m_toolIt!=m_tools.end()?*m_toolIt:nullptr;
}

Tool *Toolbox::getNextTool()
{
	if(m_toolIt!=m_tools.end()) m_toolIt++;
	if(m_toolIt!=m_tools.end()) return *m_toolIt; return nullptr;
}
