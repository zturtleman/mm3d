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


#include "toolpoly.h"

#include "model.h"

using std::vector;

ToolPolygons::ToolPolygons()
{
   clear();
}

ToolPolygons::~ToolPolygons()
{
}

void ToolPolygons::clear()
{
   m_model    = NULL;
   m_inverted = false;

   m_dim[0] = 0.0;
   m_dim[1] = 0.0;
   m_dim[2] = 0.0;

   m_toolVertices.clear();
   m_toolTriangles.clear();
}

void ToolPolygons::setModel( Model * model )
{
   clear();
   m_model = model;
}

void ToolPolygons::startCoordinates( double xdim, double ydim, double zdim )
{
   m_inverted = false;

   m_start[0] = xdim;
   m_start[1] = ydim;
   m_start[2] = zdim;

   if ( m_model )
   {
      VertexList::iterator it = m_toolVertices.begin();
      while ( it != m_toolVertices.end() )
      {
         m_model->moveVertex( (*it).v,
               m_start[0], m_start[1], m_start[2] );
         it++;
      }
   }
}

void ToolPolygons::updateDimensions( double xdim, double ydim, double zdim )
{
   m_dim[0] = xdim;
   m_dim[1] = ydim;
   m_dim[2] = zdim;

   if ( m_model )
   {
      VertexList::iterator it = m_toolVertices.begin();
      while ( it != m_toolVertices.end() )
      {
         m_model->moveVertex( (*it).v,
               m_start[0] + m_dim[0] * (*it).coords[0],
               m_start[1] + m_dim[1] * (*it).coords[1],
               m_start[2] + m_dim[2] * (*it).coords[2] );
         it++;
      }
   }
}

void ToolPolygons::selectVertices()
{
   if ( m_model )
   {
      VertexList::iterator it = m_toolVertices.begin();
      while ( it != m_toolVertices.end() )
      {
         m_model->selectVertex( (*it).v );
         it++;
      }
   }
}

void ToolPolygons::selectTriangles()
{
   if ( m_model )
   {
      unsigned count = m_toolTriangles.size();
      for ( unsigned t = 0; t < count; t++ )
      {
         m_model->selectTriangle( m_toolTriangles[ t ] );
      }
   }
}

int ToolPolygons::addVertex( double x, double y, double z )
{
   if ( m_model )
   {
      int v = m_model->addVertex( x, y, z );

      if ( v >= 0 )
      {
         VertexT vert;

         vert.v = v;
         vert.coords[0] = x;
         vert.coords[1] = y;
         vert.coords[2] = z;

         m_toolVertices.push_back( vert );
      }

      return v;
   }
   else
   {
      return -1;
   }
}

int ToolPolygons::addTriangle( unsigned v1, unsigned v2, unsigned v3 )
{
   if ( m_model )
   {
      int tri = m_model->addTriangle( v1, v2, v3 );

      if ( tri >= 0 )
      {
         m_toolTriangles.push_back( tri );
      }

      return tri;
   }
   else
   {
      return -1;
   }
}

void ToolPolygons::invertNormals()
{
   m_inverted = ! m_inverted;

   unsigned count = m_toolTriangles.size();
   for ( unsigned t = 0; t < count; t++ )
   {
      m_model->invertNormals( m_toolTriangles[t] );
   }
}
