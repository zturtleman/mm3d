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


#ifndef __COMMAND_H
#define __COMMAND_H

//------------------------------------------------------------------
// About the Command class
//------------------------------------------------------------------
//
// The Command class is a virtual base class for all Maverick Model 3D
// commands.
//
// The methods you need to implement in order to add a new command are 
// noted below.
//
// Generally commands only operate on selected vertices, faces, or bone
// joints; though there can be exceptions to this, exceptions should be 
// rare.
//
// You can open a Qt dialog box from a command if you need more complex
// input from the user.  See the ExtrudeCommand for an undocumented 
// example.  Dialog boxes must be modal.  Non-modal dialogs will not
// be notified of model changes so they cannot update themselves 
// accordingly.  Also non-modal dialogs are inconsistent with 
// Maverick Model 3D's general behavior and may be confusing to the user.

#include <stdlib.h>

class Model;

class Command
{
   public:
      Command();
      virtual ~Command();

      // It is a good idea to override this if you implement
      // a command as a plugin.
      virtual void release() { delete this; };

      // getCommandCount() returns the number of sub commands provided by 
      // this command.  This value should be either 1 (for a single command) 
      // or the number of commands you want plus 1.  A value of 1 will 
      // create a single entry in a menu to activate the command.  A value
      // greater than 1 will create a sub menu for each command number.
      //
      // If this number is greater than 1, the first command (0) is
      // the command group and will never be activated as a command.
      // If you wanted to provide commands Foo A, Foo B, and Foo C
      // as one command group, your command count would be 4 and the
      // commands would be set up as follows:
      //
      // 0 Foo
      // 1 Foo A
      // 2 Foo B
      // 3 Foo C
      //
      // In the menu system, "Foo" would appear as a menu item with a submenu.
      // The submenu items would be Foo A, Foo B, and Foo C.  The "Foo" item
      // is not actually a command, but simply the name of the group.
      //
      // See the FlipCommand for an undocumented example of a command group.
      virtual int getCommandCount() = 0;

      // Returns the name of the command number 'arg'.  In the case of a
      // single command you can ignore arg.  If you have a command group,
      // arg=0 means the group name and 1-n are your command numbers.
      //
      // This is the name as it appears in the menu system.
      virtual const char * getName( int arg ) = 0;

      // Returns the path to the item in the popup menu. If empty, it is in 
      // the menu at the top level, otherwise it is in submenus listed in the
      // path. Each sub menu is separated by the pipe '|' character.
      virtual const char * getPath() { return ""; };

      // If you want a keybinding for your command you must set keyBinding
      // and return true.  The value of keyBinding is a key code as defined
      // in qnamespace.h (part of the Qt library).
      virtual bool getKeyBinding( int arg, int & keyBinding ) { return false; };

      // activated means that the user wants to run the command specified in arg
      // on the specified model.
      //
      // A return value of 'true' means that the model was modified by the
      // command.  'false' means the model was unmodified.  An unmodified
      // model may be the result of a canceled dialog box, or an empty
      // selection set.
      virtual bool activated( int arg, Model * model ) = 0;

      // Is this a place-holder for a menu separator?
      virtual bool isSeparator() { return false; };

      // Does this command operate on primitives (vertices, triangles, joints)?
      virtual bool isPrimitive() { return false; };

      // Does this command operate on groups of polygons?
      virtual bool isGroup() { return false; };
};

class SeparatorCommand : public Command
{
   public:
      SeparatorCommand() {};
      virtual ~SeparatorCommand() {};

      int getCommandCount() { return 1; };

      const char * getName( int arg ) { return ""; };

      bool activated( int arg, Model * model ) { return false; };

      bool isSeparator() { return true; };
};
#endif // __COMMAND_H
