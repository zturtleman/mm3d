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


#ifndef __COMMAND_H
#define __COMMAND_H

#include "mm3dtypes.h"
#include "translate.h"

//------------------------------------------------------------------
// About the Command class
//------------------------------------------------------------------
//
// The Command class is a virtual base class for all MMM3D commands.
//
// The methods you need to implement in order to add a new command are 
// noted below.
//
// Generally commands only operate on selected vertices,faces,or bone
// joints; though there can be exceptions to this,exceptions should be 
// rare.
//
// You can open a Qt dialog box from a command if you need more complex
// input from the user.  See the ExtrudeCommand for an undocumented 
// example.  Dialog boxes must be modal.  Non-modal dialogs will not
// be notified of model changes so they cannot update themselves 
// accordingly.  Also non-modal dialogs are inconsistent with MMM3D's 
// general behavior and may be confusing to the user.

class Command
{
public:

	const int m_args;

	const char *const m_path;
	
	//NOTE: Tool has Tool::s_allocated? 
	Command(int count=1, const char *path=nullptr)
	:m_args(count),m_path(path)
	{}
	virtual ~Command()
	{}

	int getCommandCount(){ return m_args; }

	// Returns the path to the item in the popup menu. If empty,it is in 
	// the menu at the top level,otherwise it is in submenus listed in the
	// path. Each sub menu is separated by the pipe '|' character.
	const char *getPath(){ return m_path; }
				
	// Is this a place-holder for a menu separator?
	bool isSeparator(){ return !m_args; }

	//UNUSED
	// Does this command operate on primitives (vertices,triangles,joints)?
	//bool isPrimitive(){ return m_type==CT_Primitive; }

	//UNUSED
	// Does this command operate on groups of polygons?
	//bool isGroup(){ return m_type==CT_Group; }

	// It is a good idea to override this if you implement
	// a command as a plugin.
	virtual void release(){ delete this; }

	// Returns the name of the command number 'arg'.  In the case of a
	// single command you can ignore arg.  If you have a command group,
	// arg=0 means the group name and 1-n are your command numbers.
	//
	// This is the name as it appears in the menu system.
	virtual const char *getName(int arg) = 0;

	// Replace keycfg.cc
	virtual const char *getKeymap(int arg){ return ""; }
		
	// activated means that the user wants to run the command specified in arg
	// on the specified model.
	//
	// A return value of 'true' means that the model was modified by the
	// command.  'false' means the model was unmodified.  An unmodified
	// model may be the result of a canceled dialog box,or an empty
	// selection set.
	virtual bool activated(int arg, Model *model) = 0;
};

#endif // __COMMAND_H
