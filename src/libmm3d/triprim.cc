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


#include "triprim.h"

#include "model.h"
#include "log.h"
#include <list>

class SimpleTriPrim : public TriPrim
{
   public:
      SimpleTriPrim();
      virtual ~SimpleTriPrim() {};

      bool findPrimitives( Model * model, bool grouped = true );
      void resetList() { m_triNumber = 0; }; // Reset to start of list
      TriPrimTypeE getNextPrimitive( TriangleVertexList & tvl );

   protected:
      Model * m_model;
      int m_triNumber;
};

SimpleTriPrim::SimpleTriPrim()
   : m_model( NULL ),
     m_triNumber( 0 )
{
}

bool SimpleTriPrim::findPrimitives( Model * model, bool grouped )
{
   m_model = model;
   return true;
}

TriPrim::TriPrimTypeE SimpleTriPrim::getNextPrimitive( TriPrim::TriangleVertexList & tvl )
{
   tvl.clear();
   int count = m_model->getTriangleCount();
   log_debug( "getting triangle %d of %d\n", m_triNumber + 1, count );
   if ( m_model != NULL && m_triNumber < m_model->getTriangleCount() )
   {
      TriPrim::TriangleVertexT tv;
      for ( unsigned v = 0; v < 3; v++ )
      {
         tv.vert = m_model->getTriangleVertex( m_triNumber, v );
         m_model->getTextureCoords( m_triNumber, v, tv.s, tv.t );
         tvl.push_back( tv );
      }
      m_triNumber++;
      return TRI_PRIM_FAN;
   }
   else
   {
      return TRI_PRIM_NONE;
   }
}

static TriPrim * _newSimpleTriPrimFunc()
{
   return new SimpleTriPrim();
}

TriPrim::NewTriPrimFunc TriPrim::s_newFunc = _newSimpleTriPrimFunc;

TriPrim::TriPrim()
{
}

TriPrim::~TriPrim()
{
}

void TriPrim::registerTriPrimFunction( NewTriPrimFunc newFunc )
{
   if ( newFunc )
   {
      s_newFunc = newFunc;
   }
   else
   {
      s_newFunc = _newSimpleTriPrimFunc;
   }
}

TriPrim * TriPrim::newTriPrim()
{
   if ( s_newFunc )
   {
      return s_newFunc();
   }
   else
   {
      return NULL;
   }
}

