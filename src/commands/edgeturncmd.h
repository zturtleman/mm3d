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


#ifndef __EDGETURNCMD_H
#define __EDGETURNCMD_H

#include "command.h"

class EdgeTurnCommand : public Command
{
   public:
      EdgeTurnCommand();
      virtual ~EdgeTurnCommand();

      int getCommandCount() { return 1; };
      const char * getPath();
      const char * getName( int arg );

      bool activated( int arg, Model * model );
      bool isPrimitive() { return true; };

   protected:
      bool canTurnEdge( Model * model, int tri1, int tri2,
            unsigned int & edge_v1, unsigned int & edge_v2, unsigned int & tri1_v, unsigned int & tri2_v );
      void getTriangleVertices( Model * model, int tri,
            unsigned int edge_v1, unsigned int edge_v2, unsigned int tri_v,
            int & a, int & b, int & c );
};

#endif // __EDGETURNCMD_H
