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


#ifndef __TOOLBOX_H
#define __TOOLBOX_H

//------------------------------------------------------------------
// About the Toolbox
//------------------------------------------------------------------
//
// The Toolbox is a class that is responsible for keeping track of
// tools for a view window.  Tools are interactive operations that
// receive mouse events in a viewport and modify the model as 
// appropriate, usually operating on selected vertices or bone joints.
//
// To create a new tool, you must derive a class from the Tool class (in 
// tool.h).  You also need a function that will register a new instance
// of that class with any new toolbox that is created.  This function
// takes a Toolbox pointer as an argument and does not have a return 
// value.  If you have a class called MyTool, and an you would register it
// as follows:
//
//    PLUGIN_API bool plugin_init()
//    {
//       Toolbox::registerToolFunction( _newMyToolFunc );
//       return true;
//    }
//   
//    static void _newMyToolFunc( Toolbox * tb )
//    {
//       MyTool * mt = new MyTool();
//       tb->registerTool( mt );
//    }
//
// This will add myTool to the tool list and it will be available in
// the toolbar as well as through Maverick Model 3D's menu system.  Tools
// that are statically linked to MM3D are registered in stdtools.cc.  Tools
// that are dynamically added to MM3D via plugins can be registered
// in the plugin_init function of a plugin.
//
// See tool.h for instructions on creating a class derived from Tool.

#include "tool.h"
#include <list>

class Toolbox;


// This class is here so that we can always assume we have a tool selected,
// even if there isn't one (we can always play with the current tool without
// worrying about NULL pointers).  The NullTool is effectively one big "No-op".
class NullTool : public ::Tool
{
   public:
      NullTool() {};
      virtual ~NullTool() {};

      int getToolCount() { return 1; };
      const char * getName( int arg ) { return ""; };
      void activated( int arg, Model * model, QMainWindow * mainwin ) {};

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y ) {};
      void mouseButtonUp(   Parent * parent, int buttonState, int x, int y ) {};
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y ) {};
      const char ** getPixmap();

   protected:
};

class Toolbox
{
   public:
      Toolbox();
      virtual ~Toolbox();

      typedef void (*NewToolFunc)( Toolbox * );

      typedef std::list< ::Tool * > ToolList;
      typedef std::list< NewToolFunc > ToolFuncList;

      // This is how you register your new tool.
      static bool registerToolFunction( NewToolFunc newToolFunc );

      // When newToolFunc is called, create any new tools and register each
      // with this function.  The Toolbox will delete the tools when it is
      // finished, so be sure you create a new tool instance for each
      // call to your newToolFunc.
      bool registerTool( ::Tool * tool );

      void registerAllTools();

      int getToolCount() { return m_tools.size(); };

      void   setCurrentTool( ::Tool * );
      ::Tool * getCurrentTool();

      // Yes, I know I should do an iterator instead
      ::Tool * getFirstTool();
      ::Tool * getNextTool();

   protected:
      static ToolFuncList s_newFuncs;
      static NullTool s_nullTool;

      ::Tool * m_current;

      ToolList m_tools;
      ToolList::iterator m_toolIt;
};

#endif // __TOOLBOX_H
