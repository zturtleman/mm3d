/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2007 Kevin Worcester
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 *
 *  See the COPYING file for full license text.
 */


#ifndef __CMDMGR_H
#define __CMDMGR_H

//------------------------------------------------------------------
// About the CommandManager
//------------------------------------------------------------------
//
// The CommandManager is a singleton which is responsible for keeping
// track of every command.  Commands are non-interactive operations which
// modify some part of the model, usually operating on selected vertices.
//
// If you create a class derived from the Command class (in command.h)
// and have an object of that class called myCommand, you can regiter that 
// command with the following statement:
//
//   CommandManager::getInstance()->registerCommand( myCommand );
//
// This will add myCommand to the command list and it will be available in
// MM3D's menu system.  Commands which are statically linked to MM3D
// are registered in stdcmd.cc.  Commands which are dynamically added to
// MM3D via plugins can be registered in the plugin_init function of a
// plugin.
//
// See command.h for instructions on creating a class derived from Command.

#include "command.h"
#include <list>

using std::list;

typedef list< Command * > CommandList;

class CommandManager
{
   public:
      CommandManager();
      virtual ~CommandManager();

      // Get the CommandManager instance.  You must use the return value
      // of this function to call CommandManager member functions.
      static CommandManager * getInstance();
      static void release();

      bool registerCommand( Command * newCommand );
      int getCommandCount() { return m_commands.size(); };

      // Yeah, it would be cleaner to use iterators--but I'm not going to.
      Command * getFirstCommand();
      Command * getNextCommand();

   protected:
      
      CommandList m_commands;
      CommandList::iterator m_commandIt;

      static CommandManager * s_instance;
};

#endif // __CMDMGR_H
