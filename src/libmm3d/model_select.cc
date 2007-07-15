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

#include "model.h"
#include "log.h"

#ifdef MM3D_EDIT

#include "modelundo.h"

// FIXME I really need to put this glmath
static void _calculateNormal( double * normal,
      double * a, double * b, double * c )
{
   normal[0] = a[1] * (b[2] - c[2]) + b[1] * (c[2] - a[2]) + c[1] * (a[2] - b[2]);
   normal[1] = a[2] * (b[0] - c[0]) + b[2] * (c[0] - a[0]) + c[2] * (a[0] - b[0]);
   normal[2] = a[0] * (b[1] - c[1]) + b[0] * (c[1] - a[1]) + c[0] * (a[1] - b[1]);

   normalize3( normal );
}

void Model::setSelectionMode( Model::SelectionModeE m )
{
   if ( m != m_selectionMode )
   {
      MU_SelectionMode * undo = new MU_SelectionMode;
      undo->setSelectionMode( m, m_selectionMode );
      sendUndo( undo );

      /*
      switch ( m )
      {
         case SelectVertices:
            unselectAllTriangles();
            unselectAllGroups();
            break;
         case SelectTriangles:
            if ( m_selectionMode == SelectVertices )
            {
               selectTrianglesFromVertices();
            }
            unselectAllGroups();
            break;
         case SelectGroups:
            if ( m_selectionMode == SelectVertices )
            {
               selectTrianglesFromVertices();
            }
            if ( m_selectionMode == SelectVertices || m_selectionMode == SelectTriangles )
            {
               selectGroupsFromTriangles();
            }
            break;
         case SelectJoints:
            unselectAllVertices();
            unselectAllTriangles();
            unselectAllGroups();
            break;
         default:
            break;
      }
      */
      m_selectionMode = m;
   }
}

bool Model::selectVertex( unsigned v )
{
   if ( v < m_vertices.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_vertices[v]->m_selected;
      m_vertices[ v ]->m_selected = true;

      MU_Select * undo = new MU_Select( SelectVertices );
      undo->setSelectionDifference( v, true, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::selectTriangle( unsigned t )
{
   if ( t < m_triangles.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_triangles[t]->m_selected;
      m_triangles[ t ]->m_selected = true;
      bool o = setUndoEnabled( false );
      //selectVerticesFromTriangles();
      selectVertex( m_triangles[t]->m_vertexIndices[0] );
      selectVertex( m_triangles[t]->m_vertexIndices[1] );
      selectVertex( m_triangles[t]->m_vertexIndices[2] );
      setUndoEnabled( o );

      MU_Select * undo = new MU_Select( SelectTriangles );
      undo->setSelectionDifference( t, true, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::selectGroup( unsigned m )
{
   if ( m >= 0 && m < m_groups.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_groups[m]->m_selected;
      m_groups[ m ]->m_selected = true;
      bool o = setUndoEnabled( false );
      selectTrianglesFromGroups();
      setUndoEnabled( o );

      MU_Select * undo = new MU_Select( SelectGroups );
      undo->setSelectionDifference( m, true, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::selectBoneJoint( unsigned j )
{
   if ( j >= 0 && j < m_joints.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_joints[j]->m_selected;
      m_joints[ j ]->m_selected = true;

      MU_Select * undo = new MU_Select( SelectJoints );
      undo->setSelectionDifference( j, true, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::selectPoint( unsigned p )
{
   if ( p >= 0 && p < m_points.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_points[p]->m_selected;
      m_points[ p ]->m_selected = true;

      MU_Select * undo = new MU_Select( SelectPoints );
      undo->setSelectionDifference( p, true, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::selectProjection( unsigned p )
{
   if ( p >= 0 && p < m_projections.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_projections[p]->m_selected;
      m_projections[ p ]->m_selected = true;

      MU_Select * undo = new MU_Select( SelectProjections );
      undo->setSelectionDifference( p, true, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unselectVertex( unsigned v )
{
   if ( v < m_vertices.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_vertices[v]->m_selected;
      m_vertices[ v ]->m_selected = false;

      MU_Select * undo = new MU_Select( SelectVertices );
      undo->setSelectionDifference( v, false, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unselectTriangle( unsigned t )
{
   if ( t < m_triangles.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_triangles[t]->m_selected;
      m_triangles[ t ]->m_selected = false;
      bool o = setUndoEnabled( false );
      selectVerticesFromTriangles();
      setUndoEnabled( o );

      MU_Select * undo = new MU_Select( SelectTriangles );
      undo->setSelectionDifference( t, false, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unselectGroup( unsigned m )
{
   LOG_PROFILE();

   if ( m >= 0 && m < m_groups.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_groups[m]->m_selected;
      m_groups[ m ]->m_selected = false;

      bool o = setUndoEnabled( false );

      list<int> tris = getGroupTriangles( m );
      list<int>::iterator it;
      for ( it = tris.begin(); it != tris.end(); it++ )
      {
         m_triangles[ *it ]->m_selected = false;
      }
      selectVerticesFromTriangles();

      setUndoEnabled( o );

      MU_Select * undo = new MU_Select( SelectGroups );
      undo->setSelectionDifference( m, false, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unselectBoneJoint( unsigned j )
{
   if ( j < m_joints.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_joints[j]->m_selected;
      m_joints[ j ]->m_selected = false;

      MU_Select * undo = new MU_Select( SelectJoints );
      undo->setSelectionDifference( j, false, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unselectPoint( unsigned p )
{
   if ( p < m_points.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_points[p]->m_selected;
      m_points[ p ]->m_selected = false;

      MU_Select * undo = new MU_Select( SelectPoints );
      undo->setSelectionDifference( p, false, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unselectProjection( unsigned p )
{
   if ( p < m_projections.size() )
   {
      m_changeBits |= SelectionChange;

      bool old = m_projections[p]->m_selected;
      m_projections[ p ]->m_selected = false;

      MU_Select * undo = new MU_Select( SelectProjections );
      undo->setSelectionDifference( p, false, old );
      sendUndo( undo );

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::isVertexSelected( unsigned v )
{
   if ( v < m_vertices.size() )
   {
      return m_vertices[v]->m_selected;
   }
   else
   {
      return false;
   }
}

bool Model::isTriangleSelected( unsigned v )
{
   if ( v < m_triangles.size() )
   {
      return m_triangles[v]->m_selected;
   }
   else
   {
      return false;
   }
}

bool Model::isGroupSelected( unsigned v )
{
   if ( v < m_groups.size() )
   {
      return m_groups[v]->m_selected;
   }
   else
   {
      return false;
   }
}

bool Model::isBoneJointSelected( unsigned j )
{
   if ( j < m_joints.size() )
   {
      return m_joints[j]->m_selected;
   }
   else
   {
      return false;
   }
}

bool Model::isPointSelected( unsigned p )
{
   if ( p < m_points.size() )
   {
      return m_points[p]->m_selected;
   }
   else
   {
      return false;
   }
}

bool Model::isProjectionSelected( unsigned p )
{
   if ( p < m_projections.size() )
   {
      return m_projections[p]->m_selected;
   }
   else
   {
      return false;
   }
}

bool Model::selectVerticesInVolumeMatrix( bool select, const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test )
{
   LOG_PROFILE();

   beginSelectionDifference();

   if ( x1 > x2 )
   {
      double temp = x2;
      x2 = x1;
      x1 = temp;
   }

   if ( y1 > y2 )
   {
      double temp = y2;
      y2 = y1;
      y1 = temp;
   }

   Vector vert;

   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->m_selected != select )
      {
         if ( m_animationMode == Model::ANIMMODE_FRAME )
         {
            vert.setAll( (*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[ v ]->m_coord, 3 );
         }
         else
         {
            vert.setAll( m_vertices[v]->m_drawSource, 3 );
         }
         vert[3] = 1.0;

         viewMat.apply( vert );

         if (  m_vertices[v]->m_visible
               && vert[0] >= x1 && vert[0] <= x2 
               && vert[1] >= y1 && vert[1] <= y2 )
         {
            if ( test )
               m_vertices[v]->m_selected = test->shouldSelect( m_vertices[v] ) ? select : m_vertices[v]->m_selected;
            else
               m_vertices[v]->m_selected = select;
         }
      }
   }
   endSelectionDifference();

   return true;
}

bool Model::selectTrianglesInVolumeMatrix( bool select, const Matrix & viewMat, double x1, double y1, double x2, double y2, bool connected, SelectionTest * test )
{
   LOG_PROFILE();

   beginSelectionDifference();

   unsigned i;
   for ( i = 0; i < m_vertices.size(); i++ )
   {
      m_vertices[i]->m_marked2 = false;
   }

   for ( i = 0; i < m_triangles.size(); i++ )
   {
      m_triangles[i]->m_marked2 = false;
   }

   if ( x1 > x2 )
   {
      double temp = x2;
      x2 = x1;
      x1 = temp;
   }

   if ( y1 > y2 )
   {
      double temp = y2;
      y2 = y1;
      y1 = temp;
   }

   unsigned t;
   for ( t = 0; t < m_triangles.size(); t++ )
   {
      Triangle * tri = m_triangles[t];
      if ( tri->m_selected != select 
            && tri->m_visible
            && !test || (test && test->shouldSelect( tri ) )) 
      {
         bool above = false;
         bool below = false;

         int v;
         Vertex *vert[3];
         double tCords[3][3];


         // 0. Assign vert to triangle's verticies 
         // 1. Check for vertices within the selection volume in the process
         for ( v = 0; v < 3; v++ )
         {
            unsigned vertId = tri->m_vertexIndices[v];
            vert[v] = m_vertices[ vertId ];
            if ( m_animationMode == Model::ANIMMODE_FRAME )
            {
               FrameAnimVertex * avert= (*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[ vertId ];
               tCords[v][0] = avert->m_coord[0]; 
               tCords[v][1] = avert->m_coord[1]; 
               tCords[v][2] = avert->m_coord[2]; 
               tCords[v][3] = 1.0;

               viewMat.apply( tCords[v] );
            }
            else
            {
               tCords[v][0] = vert[v]->m_drawSource[0]; 
               tCords[v][1] = vert[v]->m_drawSource[1]; 
               tCords[v][2] = vert[v]->m_drawSource[2]; 
               tCords[v][3] = 1.0;

               viewMat.apply( tCords[v] );
            }
         }
         for ( v = 0; v < 3; v++ )
         {
            if (  tCords[v][0] >= x1 && tCords[v][0] <= x2 
                  && tCords[v][1] >= y1 && tCords[v][1] <= y2 )
            {
               // A vertex of the triangle is within the selection area
               tri->m_selected = select;

               vert[0]->m_marked2 = true;
               vert[1]->m_marked2 = true;
               vert[2]->m_marked2 = true;
               goto next_triangle; // next triangle
            }
         }

         // 2. Find intersections between triangle edges and selection edges
         // 3. Also, check to see if the selection box is completely within triangle

         double m[3];
         double b[3];
         double *coord[3][2];

         m[0] = (tCords[0][1] - tCords[1][1]) / (tCords[0][0] - tCords[1][0]) ;
         coord[0][0] = tCords[0];
         coord[0][1] = tCords[1];
         m[1] = (tCords[0][1] - tCords[2][1]) / (tCords[0][0] - tCords[2][0]) ;
         coord[1][0] = tCords[0];
         coord[1][1] = tCords[2];
         m[2] = (tCords[1][1] - tCords[2][1]) / (tCords[1][0] - tCords[2][0]) ;
         coord[2][0] = tCords[1];
         coord[2][1] = tCords[2];

         b[0] = tCords[0][1] - ( m[0] * tCords[0][0] );
         b[1] = tCords[2][1] - ( m[1] * tCords[2][0] );
         b[2] = tCords[2][1] - ( m[2] * tCords[2][0] );

         for ( int line = 0; line < 3; line++ )
         {
            double y;
            double x;
            double xmin;
            double xmax;
            double ymin;
            double ymax;

            if ( coord[line][0][0] < coord[line][1][0] )
            {
               xmin = coord[line][0][0];
               xmax = coord[line][1][0];
            }
            else
            {
               xmin = coord[line][1][0];
               xmax = coord[line][0][0];
            }

            if ( coord[line][0][1] < coord[line][1][1] )
            {
               ymin = coord[line][0][1];
               ymax = coord[line][1][1];
            }
            else
            {
               ymin = coord[line][1][1];
               ymax = coord[line][0][1];
            }

            if ( x1 >= xmin && x1 <= xmax )
            {
               y = m[line] * x1 + b[line];
               if ( y >= y1 && y <= y2 )
               {
                  tri->m_selected = select;

                  vert[0]->m_marked2 = true;
                  vert[1]->m_marked2 = true;
                  vert[2]->m_marked2 = true;
                  goto next_triangle; // next triangle
               }

               if ( y > y1 )
               {
                  above = true;
               }
               if ( y < y1 )
               {
                  below = true;
               }
            }

            if ( x2 >= xmin && x2 <= xmax )
            {
               y = m[line] * x2 + b[line];
               if ( y >= y1 && y <= y2 )
               {
                  tri->m_selected = select;

                  vert[0]->m_marked2 = true;
                  vert[1]->m_marked2 = true;
                  vert[2]->m_marked2 = true;
                  goto next_triangle; // next triangle
               }
            }

            if ( y1 >= ymin && y1 <= ymax )
            {
               if ( coord[line][0][0] == coord[line][1][0] )
               {
                  if ( coord[line][0][0] >= x1 && coord[line][0][0] <= x2 )
                  {
                     tri->m_selected = select;

                     vert[0]->m_marked2 = true;
                     vert[1]->m_marked2 = true;
                     vert[2]->m_marked2 = true;
                     goto next_triangle; // next triangle
                  }
               }
               else
               {
                  x = (y1 - b[line]) / m[line];
                  if ( x >= x1 && x <= x2 )
                  {
                     tri->m_selected = select;

                     vert[0]->m_marked2 = true;
                     vert[1]->m_marked2 = true;
                     vert[2]->m_marked2 = true;
                     goto next_triangle; // next triangle
                  }
               }
            }

            if ( y2 >= ymin && y2 <= ymax )
            {
               if ( coord[line][0][0] == coord[line][1][0] )
               {
                  if ( coord[line][0][0] >= x1 && coord[line][0][0] <= x2 )
                  {
                     tri->m_selected = select;

                     vert[0]->m_marked2 = true;
                     vert[1]->m_marked2 = true;
                     vert[2]->m_marked2 = true;
                     goto next_triangle; // next triangle
                  }
               }
               else
               {
                  x = (y2 - b[line]) / m[line];
                  if ( x >= x1 && x <= x2 )
                  {
                     tri->m_selected = select;

                     vert[0]->m_marked2 = true;
                     vert[1]->m_marked2 = true;
                     vert[2]->m_marked2 = true;
                     goto next_triangle; // next triangle
                  }
               }
            }
         }

         if ( above && below )
         {
            // There was an intersection above and below the selection area,
            // This means we're inside the triangle, so add it to our selection list
            tri->m_selected = select;

            vert[0]->m_marked2 = true;
            vert[1]->m_marked2 = true;
            vert[2]->m_marked2 = true;
            goto next_triangle; // next triangle
         }

next_triangle:
         ; // because we need a statement after a label
      }
   }

   if ( connected )
   {
      bool found = true;
      while ( found )
      {
         found = false;
         for ( t = 0; t < m_triangles.size(); t++ )
         {
            int count = 0;
            for ( unsigned v = 0; v < 3; v++ )
            {
               if ( m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_marked2 )
               {
                  count++;
               }
            }

            if ( count > 0 && 
                  (count < 3 || m_triangles[t]->m_selected != select) ) 
            {
               found = true;

               m_triangles[t]->m_selected = select;

               for ( unsigned v = 0; v < 3; v++ )
               {
                  m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_marked2 = true;
               }
            }
         }
      }
   }

   selectVerticesFromTriangles();

   endSelectionDifference();
   return true;
}

bool Model::selectGroupsInVolumeMatrix( bool select, const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test )
{
   LOG_PROFILE();

   beginSelectionDifference();

   selectTrianglesInVolumeMatrix( select, viewMat, x1, y1, x2, y2, false, test );

   if ( select )
   {
      selectGroupsFromTriangles( false );
   }
   else
   {
      selectGroupsFromTriangles( true );
   }

   endSelectionDifference();

   return true;
}

bool Model::selectBoneJointsInVolumeMatrix( bool select, const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test )
{
   LOG_PROFILE();

   beginSelectionDifference();

   if ( x1 > x2 )
   {
      double temp = x2;
      x2 = x1;
      x1 = temp;
   }

   if ( y1 > y2 )
   {
      double temp = y2;
      y2 = y1;
      y1 = temp;
   }

   Vector vec;

   for ( unsigned j = 0; j < m_joints.size(); j++ )
   {
      Joint * joint = m_joints[j];

      if ( joint->m_selected != select && joint->m_visible )
      {
         vec[0] = joint->m_final.get( 3, 0 );
         vec[1] = joint->m_final.get( 3, 1 );
         vec[2] = joint->m_final.get( 3, 2 );
         vec[3] = 1.0;

         viewMat.apply( vec );

         if ( vec[ 0 ] >= x1 && vec[ 0 ] <= x2 
               && vec[ 1 ] >= y1 && vec[ 1 ] <= y2 )
         {
            if ( test )
               joint->m_selected = test->shouldSelect( joint ) ? select : joint->m_selected;
            else
               joint->m_selected = select;
         }
      }
   }
   endSelectionDifference();

   return true;
}

bool Model::selectPointsInVolumeMatrix( bool select, const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test )
{
   LOG_PROFILE();

   beginSelectionDifference();

   if ( x1 > x2 )
   {
      double temp = x2;
      x2 = x1;
      x1 = temp;
   }

   if ( y1 > y2 )
   {
      double temp = y2;
      y2 = y1;
      y1 = temp;
   }

   Vector vec;

   for ( unsigned p = 0; p < m_points.size(); p++ )
   {
      Point * point = m_points[p];

      if ( point->m_selected != select 
            && point->m_visible )
      {
         if ( m_animationMode == Model::ANIMMODE_FRAME )
         {
            vec.setAll( (*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_framePoints)[ p ]->m_trans, 3 );
         }
         else
         {
            vec.setAll( point->m_drawSource, 3 );
         }
         vec[3] = 1.0;

         viewMat.apply( vec );

         if ( vec[ 0 ] >= x1 && vec[ 0 ] <= x2 
               && vec[ 1 ] >= y1 && vec[ 1 ] <= y2 )
         {
            if ( test )
               point->m_selected = test->shouldSelect( point ) ? select : point->m_selected;
            else
               point->m_selected = select;
         }
      }
   }
   endSelectionDifference();

   return true;
}

bool Model::selectProjectionsInVolumeMatrix( bool select, const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test )
{
   LOG_PROFILE();

   beginSelectionDifference();

   if ( x1 > x2 )
   {
      double temp = x2;
      x2 = x1;
      x1 = temp;
   }

   if ( y1 > y2 )
   {
      double temp = y2;
      y2 = y1;
      y1 = temp;
   }

   if ( m_animationMode == ANIMMODE_NONE )
   {
      for ( unsigned p = 0; p < m_projections.size(); p++ )
      {
         TextureProjection * proj = m_projections[p];

         if ( proj->m_selected != select )
         {
            Vector pos(  proj->m_pos );
            Vector up(   proj->m_upVec );
            Vector seam( proj->m_seamVec );

            Vector left;  // perpendicular to up and seam vectors

            double upMag = up.mag3();

            up.normalize3();
            seam.normalize3();

            // Because up and seam are vectors from pos, we can assume
            // that pos is at the origin. The math works out the same.
            double orig[3] = { 0, 0, 0 };
            _calculateNormal( left.getVector(), orig, up.getVector(), seam.getVector() );

            up.scale3( upMag );
            left.scale3( upMag );

            viewMat.apply( pos );
            viewMat.apply3( up );
            viewMat.apply3( seam );
            viewMat.apply3( left );

            bool selectable = false;

            if ( proj->m_type == TPT_Plane )
            {
               bool above = false;
               bool below = false;

               int v;
               double tCords[4][3];

               // 0. Assign vert to triangle's verticies 
               // 1. Check for vertices within the selection volume in the process
               for ( v = 0; v < 4; v++ )
               {
                  double x = (v == 0 || v == 3) ? -1.0 : 1.0;
                  double y = (v >= 2) ? -1.0 : 1.0;

                  tCords[v][0] = pos[0] + up[0] * y + left[0] * x;
                  tCords[v][1] = pos[1] + up[1] * y + left[1] * x;
                  tCords[v][2] = pos[2] + up[2] * y + left[2] * x;

                  log_debug( "vertex %d: %f,%f,%f\n", v, 
                        tCords[v][0], tCords[v][1], tCords[v][2] );
               }

               for ( v = 0; v < 4; v++ )
               {
                  if (  tCords[v][0] >= x1 && tCords[v][0] <= x2 
                        && tCords[v][1] >= y1 && tCords[v][1] <= y2 )
                  {
                     // A vertex of the square is within the selection area
                     selectable = true;
                  }

                  log_debug( "xform: %d: %f,%f,%f\n", v, 
                        tCords[v][0], tCords[v][1], tCords[v][2] );
               }

               // 2. Find intersections between triangle edges and selection edges
               // 3. Also, check to see if the selection box is completely within triangle

               double m[4];
               double b[4];
               double *coord[4][2];

               m[0] = (tCords[0][1] - tCords[1][1]) / (tCords[0][0] - tCords[1][0]) ;
               coord[0][0] = tCords[0];
               coord[0][1] = tCords[1];
               m[1] = (tCords[1][1] - tCords[2][1]) / (tCords[1][0] - tCords[2][0]) ;
               coord[1][0] = tCords[1];
               coord[1][1] = tCords[2];
               m[2] = (tCords[2][1] - tCords[3][1]) / (tCords[2][0] - tCords[3][0]) ;
               coord[2][0] = tCords[2];
               coord[2][1] = tCords[3];
               m[3] = (tCords[3][1] - tCords[0][1]) / (tCords[3][0] - tCords[0][0]) ;
               coord[3][0] = tCords[3];
               coord[3][1] = tCords[0];

               b[0] = tCords[0][1] - ( m[0] * tCords[0][0] );
               b[1] = tCords[1][1] - ( m[1] * tCords[1][0] );
               b[2] = tCords[2][1] - ( m[2] * tCords[2][0] );
               b[3] = tCords[3][1] - ( m[3] * tCords[3][0] );


               for ( int line = 0; !selectable && line < 4; line++ )
               {
                  log_debug( "line %d:   m = %f   b = %f   x = %f   y = %f\n", 
                        line, m[line], b[line], coord[line][0][0], coord[line][0][1] );
                  double y;
                  double x;
                  double xmin;
                  double xmax;
                  double ymin;
                  double ymax;

                  if ( coord[line][0][0] < coord[line][1][0] )
                  {
                     xmin = coord[line][0][0];
                     xmax = coord[line][1][0];
                  }
                  else
                  {
                     xmin = coord[line][1][0];
                     xmax = coord[line][0][0];
                  }

                  if ( coord[line][0][1] < coord[line][1][1] )
                  {
                     ymin = coord[line][0][1];
                     ymax = coord[line][1][1];
                  }
                  else
                  {
                     ymin = coord[line][1][1];
                     ymax = coord[line][0][1];
                  }

                  if ( x1 >= xmin && x1 <= xmax )
                  {
                     y = m[line] * x1 + b[line];
                     if ( y >= y1 && y <= y2 )
                     {
                        selectable = true;
                     }

                     if ( y > y1 )
                     {
                        above = true;
                     }
                     if ( y < y1 )
                     {
                        below = true;
                     }
                  }

                  if ( !selectable && x2 >= xmin && x2 <= xmax )
                  {
                     y = m[line] * x2 + b[line];
                     if ( y >= y1 && y <= y2 )
                     {
                        selectable = true;
                     }
                  }

                  bool vertical = ( fabs( coord[line][0][0] - coord[line][1][0] ) < 0.0001 );
                  if ( !selectable && y1 >= ymin && y1 <= ymax )
                  {
                     if ( vertical )
                     {
                        if ( coord[line][0][0] >= x1 && coord[line][0][0] <= x2 )
                        {
                           selectable = true;
                        }
                     }
                     else
                     {
                        x = (y1 - b[line]) / m[line];
                        if ( x >= x1 && x <= x2 )
                        {
                           selectable = true;
                        }
                     }
                  }

                  if ( !selectable && y2 >= ymin && y2 <= ymax )
                  {
                     if ( vertical )
                     {
                        if ( coord[line][0][0] >= x1 && coord[line][0][0] <= x2 )
                        {
                           selectable = true;
                        }
                     }
                     else
                     {
                        x = (y2 - b[line]) / m[line];
                        if ( x >= x1 && x <= x2 )
                        {
                           selectable = true;
                        }
                     }
                  }
               }

               if ( above && below )
               {
                  // There was an intersection above and below the selection area,
                  // This means we're inside the square, so add it to our selection list
                  selectable = true;
               }
            }
            else
            {
               int i = 0;
               int iMax = 0;

               double radius = up.mag3();

               double diffX = up[0];
               double diffY = up[1];

               if ( proj->m_type == TPT_Cylinder )
               {
                  radius = radius / 3;

                  i    = -1;
                  iMax =  1;
               }

               for ( ; i <= iMax && !selectable; i++ )
               {
                  double x = pos[ 0 ];
                  double y = pos[ 1 ];

                  x += diffX * (double) i;
                  y += diffY * (double) i;

                  // check if center is inside selection region
                  bool inx = ( x >= x1 && x <= x2 );
                  bool iny = ( y >= y1 && y <= y2 );

                  selectable = ( inx && iny );

                  if ( !selectable )
                  {
                     // check if lines passes through radius
                     if ( inx )
                     {
                        if ( fabs( y - y1 ) < radius
                              || fabs( y - y2 ) < radius )
                        {
                           selectable = true;
                        }
                     }
                     else if ( iny )
                     {
                        if ( fabs( x - x1 ) < radius
                              || fabs( x - x2 ) < radius )
                        {
                           selectable = true;
                        }
                     }
                     else
                     {
                        // line did not pass through radius, see if all bounding region 
                        // points are within radius
                        double diff[2];

                        diff[0] = fabs( x - x1 );
                        diff[1] = fabs( y - y1 );

                        if ( sqrt( diff[0]*diff[0] + diff[1]*diff[1] ) < radius )
                        {
                           selectable = true;
                        }

                        diff[0] = fabs( x - x2 );
                        diff[1] = fabs( y - y1 );

                        if ( sqrt( diff[0]*diff[0] + diff[1]*diff[1] ) < radius )
                        {
                           selectable = true;
                        }

                        diff[0] = fabs( x - x1 );
                        diff[1] = fabs( y - y2 );

                        if ( sqrt( diff[0]*diff[0] + diff[1]*diff[1] ) < radius )
                        {
                           selectable = true;
                        }

                        diff[0] = fabs( x - x2 );
                        diff[1] = fabs( y - y2 );

                        if ( sqrt( diff[0]*diff[0] + diff[1]*diff[1] ) < radius )
                        {
                           selectable = true;
                        }
                     }
                  }
               }
            }

            if ( selectable )
            {
               if ( test )
                  proj->m_selected = test->shouldSelect( proj ) ? select : proj->m_selected;
               else
                  proj->m_selected = select;
            }
         }
      }
   }

   endSelectionDifference();
   return true;
}

bool Model::selectInVolumeMatrix( const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test )
{
   LOG_PROFILE();

   switch ( m_selectionMode )
   {
      case SelectVertices:
         return selectVerticesInVolumeMatrix ( true, viewMat, x1, y1, x2, y2, test );
         break;
      case SelectTriangles:
         return selectTrianglesInVolumeMatrix ( true, viewMat, x1, y1, x2, y2, false, test );
         break;
      case SelectConnected:
         return selectTrianglesInVolumeMatrix ( true, viewMat, x1, y1, x2, y2, true, test );
         break;
      case SelectGroups:
         return selectGroupsInVolumeMatrix ( true, viewMat, x1, y1, x2, y2, test );
         break;
      case SelectJoints:
         return selectBoneJointsInVolumeMatrix ( true, viewMat, x1, y1, x2, y2, test );
         break;
      case SelectPoints:
         return selectPointsInVolumeMatrix ( true, viewMat, x1, y1, x2, y2, test );
         break;
      case SelectProjections:
         return selectProjectionsInVolumeMatrix ( true, viewMat, x1, y1, x2, y2, test );
         break;
      default:
         break;
   }
   return true;
}

bool Model::unselectInVolumeMatrix( const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test )
{
   LOG_PROFILE();

   switch ( m_selectionMode )
   {
      case SelectVertices:
         return selectVerticesInVolumeMatrix ( false, viewMat, x1, y1, x2, y2, test );
         break;
      case SelectTriangles:
         return selectTrianglesInVolumeMatrix ( false, viewMat, x1, y1, x2, y2, false, test );
         break;
      case SelectConnected:
         return selectTrianglesInVolumeMatrix ( false, viewMat, x1, y1, x2, y2, true, test );
         break;
      case SelectGroups:
         return selectGroupsInVolumeMatrix ( false, viewMat, x1, y1, x2, y2, test );
         break;
      case SelectJoints:
         return selectBoneJointsInVolumeMatrix ( false, viewMat, x1, y1, x2, y2, test );
         break;
      case SelectPoints:
         return selectPointsInVolumeMatrix ( false, viewMat, x1, y1, x2, y2, test );
         break;
      case SelectProjections:
         return selectProjectionsInVolumeMatrix ( false, viewMat, x1, y1, x2, y2, test );
         break;
      default:
         break;
   }
   return true;
}

void Model::selectVerticesFromTriangles()
{
   LOG_PROFILE();

   unselectAllVertices();

   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_selected )
      {
         for ( int v = 0; v < 3; v++ )
         {
            m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_selected = true;
         }
      }
   }
}

void Model::selectTrianglesFromGroups()
{
   LOG_PROFILE();

   unselectAllTriangles();

   for ( unsigned g = 0; g < m_groups.size(); g++ )
   {
      if ( m_groups[g]->m_selected )
      {
         for ( unsigned t = 0; t < m_groups[g]->m_triangleIndices.size(); t++ )
         {
            if ( m_triangles[ m_groups[g]->m_triangleIndices[t] ]->m_visible )
            {
               m_triangles[ m_groups[g]->m_triangleIndices[t] ]->m_selected = true;
            }
         }
      }
   }

   selectVerticesFromTriangles();
}

void Model::selectTrianglesFromVertices( bool all )
{
   LOG_PROFILE();

   unselectAllTriangles();

   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_visible )
      {
         int count = 0;
         for ( int v = 0; v < 3; v++ )
         {
            if ( m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_selected )
            {
               count++;
            }

         }
         if ( all )
         {
            if ( count == 3 )
            {
               m_triangles[t]->m_selected = true;
            }
         }
         else
         {
            if ( count > 0 )
            {
               m_triangles[t]->m_selected = true;
            }
         }
      }
   }

   // Unselect vertices who don't have a triangle selected
   if ( all )
   {
      unselectAllVertices();
      selectVerticesFromTriangles();
   }
}

void Model::selectGroupsFromTriangles( bool all )
{
   LOG_PROFILE();

   unselectAllGroups();

   for ( unsigned g = 0; g < m_groups.size(); g++ )
   {
      unsigned count = 0;
      for ( unsigned t = 0; t < m_groups[g]->m_triangleIndices.size(); t++ )
      {
         if ( m_triangles[ m_groups[g]->m_triangleIndices[t] ]->m_selected )
         {
            count++;
         }
      }

      if ( all )
      {
         if ( count == m_groups[g]->m_triangleIndices.size() )
         {
            m_groups[g]->m_selected = true;
         }
         else
         {
            m_groups[g]->m_selected = false;
         }
      }
      else
      {
         if ( count > 0 )
         {
            m_groups[g]->m_selected = true;
         }
         else
         {
            m_groups[g]->m_selected = false;
         }
      }
   }

   // Unselect vertices who don't have a triangle selected
   unselectAllTriangles();
   selectTrianglesFromGroups();
}

bool Model::invertSelection()
{
   LOG_PROFILE();

   beginSelectionDifference();
   switch ( m_selectionMode )
   {
      case SelectVertices:
         for ( unsigned v = 0; v < m_vertices.size(); v++ )
         {
            if ( m_vertices[v]->m_visible )
            {
               m_vertices[v]->m_selected = m_vertices[v]->m_selected ? false : true;
            }
         }
         break;

      case SelectTriangles:
         for ( unsigned t = 0; t < m_triangles.size(); t++ )
         {
            if ( m_triangles[t]->m_visible )
            {
               m_triangles[t]->m_selected = m_triangles[t]->m_selected ? false : true;
            }
         }
         selectVerticesFromTriangles();
         break;

      case SelectGroups:
         for ( unsigned g = 0; g < m_groups.size(); g++ )
         {
            m_groups[g]->m_selected = m_groups[g]->m_selected ? false : true;
         }
         selectTrianglesFromGroups();
         break;

      case SelectJoints:
         for ( unsigned j = 0; j < m_joints.size(); j++ )
         {
            m_joints[j]->m_selected = m_joints[j]->m_selected ? false : true;
         }
         break;

      case SelectPoints:
         for ( unsigned p = 0; p < m_points.size(); p++ )
         {
            m_points[p]->m_selected = m_points[p]->m_selected ? false : true;
         }
         break;

      default:
         break;
   }
   endSelectionDifference();

   return true;
}

void Model::beginSelectionDifference()
{
   LOG_PROFILE();

   if ( !m_selecting )
   {
      m_selecting = true;

      m_changeBits |= SelectionChange;

      if ( m_undoEnabled )
      {
         unsigned t;
         for ( t = 0; t < m_vertices.size(); t++ )
         {
            m_vertices[t]->m_marked = m_vertices[t]->m_selected;
         }
         for ( t = 0; t < m_triangles.size(); t++ )
         {
            m_triangles[t]->m_marked = m_triangles[t]->m_selected;
         }
         for ( t = 0; t < m_groups.size(); t++ )
         {
            m_groups[t]->m_marked = m_groups[t]->m_selected;
         }
         for ( t = 0; t < m_joints.size(); t++ )
         {
            m_joints[t]->m_marked = m_joints[t]->m_selected;
         }
         for ( t = 0; t < m_points.size(); t++ )
         {
            m_points[t]->m_marked = m_points[t]->m_selected;
         }
         for ( t = 0; t < m_projections.size(); t++ )
         {
            m_projections[t]->m_marked = m_projections[t]->m_selected;
         }
      }
   }
}

void Model::endSelectionDifference()
{
   LOG_PROFILE();

   m_selecting = false;

   {
      MU_Select * undo = new MU_Select( SelectVertices );
      for ( unsigned t = 0; t < m_vertices.size(); t++ )
      {
         if ( m_vertices[t]->m_selected != m_vertices[t]->m_marked )
         {
            undo->setSelectionDifference( t, m_vertices[t]->m_selected, m_vertices[t]->m_marked );
         }
      }
      if ( undo->diffCount() > 0 )
      {
         sendUndo( undo );
      }
      else
      {
         undo->release();
      }
   }
   {
      MU_Select * undo = new MU_Select( SelectTriangles );
      for ( unsigned t = 0; t < m_triangles.size(); t++ )
      {
         if ( m_triangles[t]->m_selected != m_triangles[t]->m_marked )
         {
            undo->setSelectionDifference( t, m_triangles[t]->m_selected, m_triangles[t]->m_marked );
         }
      }
      if ( undo->diffCount() > 0 )
      {
         sendUndo( undo );
      }
      else
      {
         undo->release();
      }
   }
   {
      MU_Select * undo = new MU_Select( SelectGroups );
      for ( unsigned t = 0; t < m_groups.size(); t++ )
      {
         if ( m_groups[t]->m_selected != m_groups[t]->m_marked )
         {
            undo->setSelectionDifference( t, m_groups[t]->m_selected, m_groups[t]->m_marked );
         }
      }
      if ( undo->diffCount() > 0 )
      {
         sendUndo( undo );
      }
      else
      {
         undo->release();
      }
   }
   {
      MU_Select * undo = new MU_Select( SelectJoints );
      for ( unsigned t = 0; t < m_joints.size(); t++ )
      {
         if ( m_joints[t]->m_selected != m_joints[t]->m_marked )
         {
            undo->setSelectionDifference( t, m_joints[t]->m_selected, m_joints[t]->m_marked );
         }
      }
      if ( undo->diffCount() > 0 )
      {
         sendUndo( undo );
      }
      else
      {
         undo->release();
      }
   }
   {
      MU_Select * undo = new MU_Select( SelectPoints );
      for ( unsigned t = 0; t < m_points.size(); t++ )
      {
         if ( m_points[t]->m_selected != m_points[t]->m_marked )
         {
            undo->setSelectionDifference( t, m_points[t]->m_selected, m_points[t]->m_marked );
         }
      }
      if ( undo->diffCount() > 0 )
      {
         sendUndo( undo );
      }
      else
      {
         undo->release();
      }
   }
   {
      MU_Select * undo = new MU_Select( SelectProjections );
      for ( unsigned t = 0; t < m_projections.size(); t++ )
      {
         if ( m_projections[t]->m_selected != m_projections[t]->m_marked )
         {
            undo->setSelectionDifference( t, m_projections[t]->m_selected, m_projections[t]->m_marked );
         }
      }
      if ( undo->diffCount() > 0 )
      {
         sendUndo( undo );
      }
      else
      {
         undo->release();
      }
   }
}

void Model::getSelectedPositions( list< Position > & positions )
{
   unsigned count;
   unsigned t;

   positions.clear();

   count = m_vertices.size();
   for ( t = 0; t < count; t++ )
   {
      if ( m_vertices[t]->m_selected )
      {
         Position p;
         p.type = PT_Vertex;
         p.index = t;
         positions.push_back( p );
      }
   }

   count = m_joints.size();
   for ( t = 0; t < count; t++ )
   {
      if ( m_joints[t]->m_selected )
      {
         Position p;
         p.type = PT_Joint;
         p.index = t;
         positions.push_back( p );
      }
   }

   count = m_points.size();
   for ( t = 0; t < count; t++ )
   {
      if ( m_points[t]->m_selected )
      {
         Position p;
         p.type = PT_Point;
         p.index = t;
         positions.push_back( p );
      }
   }

   count = m_projections.size();
   for ( t = 0; t < count; t++ )
   {
      if ( m_projections[t]->m_selected )
      {
         Position p;
         p.type = PT_Projection;
         p.index = t;
         positions.push_back( p );
      }
   }
}

void Model::getSelectedVertices( list<int> & vertices )
{
   vertices.clear();

   for ( unsigned t = 0; t < m_vertices.size(); t++ )
   {
      if ( m_vertices[t]->m_selected )
      {
         vertices.push_back( t );
      }
   }
}

void Model::getSelectedTriangles( list<int> & triangles )
{
   triangles.clear();

   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_selected )
      {
         triangles.push_back( t );
      }
   }
}

void Model::getSelectedGroups( list<int> & groups )
{
   groups.clear();

   for ( unsigned t = 0; t < m_groups.size(); t++ )
   {
      if ( m_groups[t]->m_selected )
      {
         groups.push_back( t );
      }
   }
}

void Model::getSelectedBoneJoints( list<int> & joints )
{
   joints.clear();

   for ( unsigned t = 0; t < m_joints.size(); t++ )
   {
      if ( m_joints[t]->m_selected )
      {
         joints.push_back( t );
      }
   }
}

void Model::getSelectedPoints( list<int> & points )
{
   points.clear();

   for ( unsigned t = 0; t < m_points.size(); t++ )
   {
      if ( m_points[t]->m_selected )
      {
         points.push_back( t );
      }
   }
}

void Model::getSelectedProjections( list<int> & projections )
{
   projections.clear();

   for ( unsigned t = 0; t < m_projections.size(); t++ )
   {
      if ( m_projections[t]->m_selected )
      {
         projections.push_back( t );
      }
   }
}

unsigned Model::getSelectedVertexCount()
{
   unsigned c = m_vertices.size();
   unsigned count = 0;

   for ( unsigned v = 0; v < c; v++ )
   {
      if ( m_vertices[v]->m_selected )
      {
         count++;
      }
   }

   return count;
}

unsigned Model::getSelectedTriangleCount()
{
   unsigned c = m_triangles.size();
   unsigned count = 0;

   for ( unsigned v = 0; v < c; v++ )
   {
      if ( m_triangles[v]->m_selected )
      {
         count++;
      }
   }

   return count;
}

unsigned Model::getSelectedBoneJointCount()
{
   unsigned c = m_joints.size();
   unsigned count = 0;

   for ( unsigned v = 0; v < c; v++ )
   {
      if ( m_joints[v]->m_selected )
      {
         count++;
      }
   }

   return count;
}

unsigned Model::getSelectedPointCount()
{
   unsigned c = m_points.size();
   unsigned count = 0;

   for ( unsigned v = 0; v < c; v++ )
   {
      if ( m_points[v]->m_selected )
      {
         count++;
      }
   }

   return count;
}

unsigned Model::getSelectedProjectionCount()
{
   unsigned c = m_projections.size();
   unsigned count = 0;

   for ( unsigned v = 0; v < c; v++ )
   {
      if ( m_projections[v]->m_selected )
      {
         count++;
      }
   }

   return count;
}

bool Model::parentJointSelected( int joint )
{
   while ( m_joints[joint]->m_parent >= 0 )
   {
      joint = m_joints[joint]->m_parent;

      if ( m_joints[joint]->m_selected )
      {
         return true;
      }
   }
   return false;
}

bool Model::directParentJointSelected( int joint )
{
   int p = m_joints[joint]->m_parent;
   if ( p >= 0 )
   {
      return m_joints[ p ]->m_selected;
   }
   return false;
}

bool Model::getSelectedBoundingRegion( double *x1, double *y1, double *z1, double *x2, double *y2, double *z2 )
{
   if ( x1 && y1 && z1 && x2 && y2 && z2 )
   {
      int visible = 0;
      bool havePoint = false;
      *x1 = *y1 = *z1 = *x2 = *y2 = *z2 = 0.0;

      for ( unsigned v = 0; v < m_vertices.size(); v++ )
      {
         if ( m_vertices[v]->m_visible && m_vertices[v]->m_selected )
         {
            if ( havePoint )
            {
               if ( m_vertices[v]->m_coord[0] < *x1 )
               {
                  *x1 = m_vertices[v]->m_coord[0];
               }
               if ( m_vertices[v]->m_coord[0] > *x2 )
               {
                  *x2 = m_vertices[v]->m_coord[0];
               }
               if ( m_vertices[v]->m_coord[1] < *y1 )
               {
                  *y1 = m_vertices[v]->m_coord[1];
               }
               if ( m_vertices[v]->m_coord[1] > *y2 )
               {
                  *y2 = m_vertices[v]->m_coord[1];
               }
               if ( m_vertices[v]->m_coord[2] < *z1 )
               {
                  *z1 = m_vertices[v]->m_coord[2];
               }
               if ( m_vertices[v]->m_coord[2] > *z2 )
               {
                  *z2 = m_vertices[v]->m_coord[2];
               }
            }
            else
            {
               *x1 = *x2 = m_vertices[v]->m_coord[0];
               *y1 = *y2 = m_vertices[v]->m_coord[1];
               *z1 = *z2 = m_vertices[v]->m_coord[2];
               havePoint = true;
            }
            visible++;
         }
      }

      for ( unsigned j = 0; j < m_joints.size(); j++ )
      {
         if ( m_joints[j]->m_selected )
         {
            double coord[3];
            m_joints[j]->m_absolute.getTranslation( coord );

            if ( havePoint )
            {
               if ( coord[0] < *x1 )
               {
                  *x1 = coord[0];
               }
               if ( coord[0] > *x2 )
               {
                  *x2 = coord[0];
               }
               if ( coord[1] < *y1 )
               {
                  *y1 = coord[1];
               }
               if ( coord[1] > *y2 )
               {
                  *y2 = coord[1];
               }
               if ( coord[2] < *z1 )
               {
                  *z1 = coord[2];
               }
               if ( coord[2] > *z2 )
               {
                  *z2 = coord[2];
               }
            }
            else
            {
               *x1 = *x2 = coord[0];
               *y1 = *y2 = coord[1];
               *z1 = *z2 = coord[2];
               havePoint = true;
            }

            visible++;
         }
      }

      for ( unsigned p = 0; p < m_points.size(); p++ )
      {
         if ( m_points[p]->m_selected )
         {
            double coord[3];
            coord[0] = m_points[p]->m_trans[0];
            coord[1] = m_points[p]->m_trans[1];
            coord[2] = m_points[p]->m_trans[2];

            if ( havePoint )
            {
               if ( coord[0] < *x1 )
               {
                  *x1 = coord[0];
               }
               if ( coord[0] > *x2 )
               {
                  *x2 = coord[0];
               }
               if ( coord[1] < *y1 )
               {
                  *y1 = coord[1];
               }
               if ( coord[1] > *y2 )
               {
                  *y2 = coord[1];
               }
               if ( coord[2] < *z1 )
               {
                  *z1 = coord[2];
               }
               if ( coord[2] > *z2 )
               {
                  *z2 = coord[2];
               }
            }
            else
            {
               *x1 = *x2 = coord[0];
               *y1 = *y2 = coord[1];
               *z1 = *z2 = coord[2];
               havePoint = true;
            }

            visible++;
         }
      }

      return ( visible != 0 ) ? true : false;
   }

   return false;
}

bool Model::unselectAllVertices()
{
   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      m_vertices[ v ]->m_selected = false;
   }
   return true;
}

bool Model::unselectAllTriangles()
{
   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      m_triangles[ t ]->m_selected = false;
   }
   return true;
}

bool Model::unselectAllGroups()
{
   for ( unsigned m = 0; m < m_groups.size(); m++ )
   {
      m_groups[ m ]->m_selected = false;
   }
   return true;
}

bool Model::unselectAllBoneJoints()
{
   for ( unsigned j = 0; j < m_joints.size(); j++ )
   {
      m_joints[ j ]->m_selected = false;
   }
   return true;
}

bool Model::unselectAllPoints()
{
   for ( unsigned p = 0; p < m_points.size(); p++ )
   {
      m_points[ p ]->m_selected = false;
   }
   return true;
}

bool Model::unselectAllProjections()
{
   for ( unsigned p = 0; p < m_projections.size(); p++ )
   {
      m_projections[ p ]->m_selected = false;
   }
   return true;
}

bool Model::unselectAll()
{
   LOG_PROFILE();

   beginSelectionDifference();

   unselectAllVertices();
   unselectAllTriangles();
   unselectAllGroups();
   unselectAllBoneJoints();
   unselectAllPoints();
   unselectAllProjections();

   endSelectionDifference();
   return true;
}

void Model::selectFreeVertices()
{
   setSelectionMode( Model::SelectVertices );
   beginSelectionDifference();

   unsigned vcount = m_vertices.size();
   unsigned v = 0;

   for ( v = 0; v < vcount; v++ )
   {
      m_vertices[ v ]->m_marked = m_vertices[ v ]->m_free;
   }

   unsigned tcount = m_triangles.size();

   for ( unsigned t = 0; t < tcount; t++ )
   {
      m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_marked = false;
      m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_marked = false;
      m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_marked = false;
   }

   for ( v = 0; v < vcount; v++ )
   {
      if ( m_vertices[ v ]->m_marked )
      {
         selectVertex( v );
      }
      else
      {
         unselectVertex( v );
      }
   }

   endSelectionDifference();
}

#endif // MM3D_EDIT
