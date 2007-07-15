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


#ifndef __CAPCMD_H
#define __CAPCMD_H

#include "command.h"

#include <list>

class CapCommand : public Command
{
   public:
      CapCommand();
      virtual ~CapCommand();

      int getCommandCount() { return 1; };
      const char * getPath();
      const char * getName( int arg );

      bool activated( int arg, Model * model );
      bool isPrimitive() { return true; };

   protected:
      void getConnected( 
            Model * model, int vert,
            std::list<int> & conList, 
            std::list<int> & triList);
      void addToList( std::list<int> & l, int ignore, int val );
      int createMissingTriangle( Model * model, unsigned int v, 
            std::list<int> & conList, std::list<int> & triList );
      int triangleCount( Model * model, 
            unsigned int v1, unsigned int v2,
            std::list<int> & triList, int & tri );
};

#endif // __CAPCMD_H
