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


#include "cmdmgr.h"
#include "log.h"

CommandManager * CommandManager::s_instance = NULL;

CommandManager::CommandManager()
{
}

CommandManager::~CommandManager()
{
   log_debug( "CommandManager releasing %d commands\n", m_commands.size() );
   CommandList::iterator it;
   for ( it = m_commands.begin(); it != m_commands.end(); it++ )
   {
      (*it)->release();
   }
   m_commands.clear();
}

CommandManager * CommandManager::getInstance()
{
   if ( s_instance == NULL )
   {
      s_instance = new CommandManager();
   }

   return s_instance;
}

void CommandManager::release()
{
   if ( s_instance != NULL )
   {
      delete s_instance;
      s_instance = NULL;
   }
}

bool CommandManager::registerCommand( Command * newCommand )
{
   if ( newCommand )
   {
      log_debug( "registering command '%s'\n", newCommand->getName( 0 ) );
      m_commands.push_back( newCommand );
      return true;
   }
   else
   {
      return false;
   }
}

Command * CommandManager::getFirstCommand()
{
   m_commandIt = m_commands.begin();

   if ( m_commandIt != m_commands.end() )
   {
      return *m_commandIt;
   }
   else
   {
      return NULL;
   }
}

Command * CommandManager::getNextCommand()
{
   if ( m_commandIt != m_commands.end() )
   {
      m_commandIt++;
   }

   if ( m_commandIt != m_commands.end() )
   {
      return *m_commandIt;
   }
   else
   {
      return NULL;
   }
}

