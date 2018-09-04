/*  Maverick Model 3D
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


#include "toolbox.h"
#include "log.h"

#include "pixmap/nulltool.xpm"

using std::list;

Toolbox::ToolFuncList Toolbox::s_newFuncs;
NullTool  Toolbox::s_nullTool;

Toolbox::Toolbox()
   : m_current( &s_nullTool ),
     m_toolIt( m_tools.begin() )
{
}

Toolbox::~Toolbox()
{
   ToolList::iterator it;
   for ( it = m_tools.begin(); it != m_tools.end(); it++ )
   {
      (*it)->release();
   }
   m_tools.clear();

   // Subtract 1 because of the statically allocated s_nullTool;
   log_debug( "active tool count: %d\n", ::Tool::s_allocated - 1 );
}

bool Toolbox::registerTool( ::Tool * tool )
{
   if ( tool )
   {
      m_tools.push_back( tool );
      return true;
   }
   else
   {
      return false;
   }
}

bool Toolbox::registerToolFunction( NewToolFunc newToolFunc )
{
   if ( newToolFunc )
   {
      s_newFuncs.push_back( newToolFunc );
      return true;
   }
   else
   {
      return false;
   }
}

void Toolbox::registerAllTools()
{
   ToolFuncList::iterator it = s_newFuncs.begin();
   while ( it != s_newFuncs.end() )
   {
      (*it)( this ); // Call tool function to get registered tools
      it++;
   }
}

::Tool * Toolbox::getFirstTool()
{
   m_toolIt = m_tools.begin();
   if ( m_toolIt != m_tools.end() )
   {
      return *m_toolIt;
   }
   else
   {
      return NULL;
   }
}

::Tool * Toolbox::getNextTool()
{
   if ( m_toolIt != m_tools.end() )
   {
      m_toolIt++;
   }
   if ( m_toolIt != m_tools.end() )
   {
      return *m_toolIt;
   }
   else
   {
      return NULL;
   }
}

::Tool * Toolbox::getCurrentTool()
{
   return m_current;
}

void Toolbox::setCurrentTool( ::Tool * tool )
{
   if ( tool )
   {
      m_current = tool;
   }
   else
   {
      m_current = &s_nullTool;
   }
}

const char ** NullTool::getPixmap()
{
   return (const char **) nulltool_xpm;
}
