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


#ifndef __TRIPRIM_H
#define __TRIPRIM_H

#include <vector>

class Model;

class TriPrim
{
   public:
      enum _TriPrimType_e
      {
         TRI_PRIM_STRIP,
         TRI_PRIM_FAN,
         TRI_PRIM_NONE
      };
      typedef enum _TriPrimType_e TriPrimTypeE;

      struct _TriangleVertex_t
      {
         int   vert;
         float s;
         float t;
      };
      typedef struct _TriangleVertex_t TriangleVertexT;
      typedef std::vector<TriangleVertexT> TriangleVertexList;
      typedef TriPrim * (*NewTriPrimFunc)();

      TriPrim();
      virtual ~TriPrim();

      // grouped:
      //    true  = textures and texture coordinates must match
      //    false = ignore texture information
      //
      // Returns:
      //    true  = found strips, fans
      //    false = error parsing triangle data
      virtual bool findPrimitives( Model * model, bool grouped = true ) = 0;

      virtual void resetList() = 0; // Reset to start of list
      virtual TriPrimTypeE getNextPrimitive( TriangleVertexList & tvl ) = 0;

      static void registerTriPrimFunction( NewTriPrimFunc newFunc );
      static TriPrim * newTriPrim();

   protected:

      static NewTriPrimFunc s_newFunc;
};

#endif // __TRIPRIM_H
