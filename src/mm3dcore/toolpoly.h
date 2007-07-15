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


#ifndef __TOOLPOLY_H
#define __TOOLPOLY_H

#include <vector>

class Model;

class ToolPolygons
{
   public:
      ToolPolygons();
      virtual ~ToolPolygons();

      void clear();

      void setModel( Model * model );
      void startCoordinates( double xdim, double ydim, double zdim );
      void updateDimensions( double xdim, double ydim, double zdim );

      int addVertex( double x, double y, double z );
      int addTriangle( unsigned v1, unsigned v2, unsigned v3 );

      void selectVertices();
      void selectTriangles();

      void invertNormals();
      bool areNormalsInverted() { return m_inverted; };

      struct _Vertex_t
      {
         unsigned v;
         double   coords[3]; 
      };
      typedef struct _Vertex_t VertexT;
      typedef std::vector< VertexT > VertexList;

      Model * m_model;
      bool    m_inverted;
      double  m_start[3];
      double  m_dim[3];

      VertexList            m_toolVertices;
      std::vector<unsigned> m_toolTriangles;
};

#endif // __TOOLPOLY_H
