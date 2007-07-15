/*  Misfit Model 3D
 * 
 *  Copyright (c) 2005 Johannes Kroll
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

#ifndef __SNAPCMD_H
#define __SNAPCMD_H

#include "command.h"

class SnapCommand: public Command
{
  public:
      SnapCommand();
      virtual ~SnapCommand();

      int getCommandCount() { return 5; }
      const char * getPath();
      const char * getName( int arg );

      // If you want a keybinding for your command you must set keyBinding
      // and return true.  The value of keyBinding is a key code as defined
      // in qnamespace.h (part of the Qt library).
      bool getKeyBinding( int arg, int & keyBinding ) { return false; };

      // activated means that the user wants to run the command specified in arg
      // on the specified model.
      //
      // A return value of 'true' means that the model was modified by the
      // command.  'false' means the model was unmodified.  An unmodified
      // model may be the result of a canceled dialog box, or an empty
      // selection set.
      bool activated( int arg, Model * model );

      // The following functions determine how the tools are grouped in
      // menus.

      // Does this command operate on primitives (vertices, triangles, joints)?
      bool isPrimitive() { return true; };

      // Does this command operate on groups of polygons?
      bool isGroup() { return false; };
};

#endif // __SNAPCMD_H
