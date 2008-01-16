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


#include "cubetool.h"
#include "pixmap/cubetool.xpm"

#include "model.h"
#include "weld.h"
#include "modelstatus.h"
#include "log.h"

#include <math.h>

#include <QMainWindow>
#include <QApplication>

using std::vector;

static void _cubify( bool isCube, double & coord, double & diff_d, double & diff_s1, double & diff_s2 )
{
   if ( isCube )
   {
      if ( fabs(diff_s1) > fabs(diff_s2) )
      {
         if ( (diff_s1 < 0 && diff_s2 > 0) 
               || (diff_s1 > 0 && diff_s2 < 0 ) )
         {
            diff_s2 = -diff_s1;
         }
         else
         {
            diff_s2 =  diff_s1;
         }
      }
      else
      {
         if ( (diff_s1 < 0 && diff_s2 > 0) 
               || (diff_s1 > 0 && diff_s2 < 0 ) )
         {
            diff_s1 = -diff_s2;
         }
         else
         {
            diff_s1 =  diff_s2;
         }
      }
   }

   if ( fabs(diff_s1) < fabs(diff_s2) )
   {
      coord  =  fabs(diff_s1/2.0);
      diff_d = -fabs(diff_s1);
   }
   else
   {
      coord  =  fabs(diff_s1/2.0);
      diff_d = -fabs(diff_s1);
   }
}

CubeTool::CubeTool()
   : m_tracking( false ),
     m_parent( NULL )
{
}

CubeTool::~CubeTool()
{
}

void CubeTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   if ( !m_tracking )
   {
      m_parent   = parent; // Keep track of which parent we're serving
      m_tracking = true;
      m_invertedNormals = false;

      m_x1 = 0.0;
      m_y1 = 0.0;
      m_z1 = 0.0;

      parent->getParentXYValue( x, y, m_x1, m_y1, true );

      Model * model = parent->getModel();

      model->unselectAll();

      double x1 = 0.0;
      double y1 = 0.0;
      double x2 = 0.0;
      double y2 = 0.0;
      double z  = 0.0;

      int xindex = 0;
      int yindex = 1;
      int zindex = 2;

      for ( unsigned side = 0; side < 6; side++ )
      {
         if ( side & 1 )
         {
            x1 = 1.0; x2 = 0.0;
            y1 = 1.0; y2 = 0.0;
            z  = 1.0;
         }
         else
         {
            x1 = 0.0; x2 = 1.0;
            y1 = 1.0; y2 = 0.0;
            z  = 0.0;
         }

         switch ( side )
         {
            case 0:
            case 1:
               xindex = 0;
               yindex = 1;
               zindex = 2;
               break;
            case 2:
            case 3:
               xindex = 2;
               yindex = 1;
               zindex = 0;
               break;
            case 4:
            case 5:
               xindex = 0;
               yindex = 2;
               zindex = 1;
               break;
            default:
               break;
         }

         double coord[3];
         for ( unsigned y = 0; y <= m_segments; y++ )
         {
            for ( unsigned x = 0; x <= m_segments; x++ )
            {
               coord[xindex] = x1 + ((x2 - x1) * (double) x / (double) m_segments);
               coord[yindex] = y1 + ((y2 - y1) * (double) y / (double) m_segments);
               coord[zindex] = z;
               ToolCoordT tc = addPosition( parent, Model::PT_Vertex, NULL,
                     coord[0], coord[1], coord[2] );
                  
               log_debug( "adding vertex %d at %f,%f,%f\n", tc.pos.index, tc.oldCoords[0], tc.oldCoords[1], tc.oldCoords[2] );

               m_vertices.push_back( tc );
            }

            if ( y > 0 )
            {
               int row1 = m_vertices.size() - (m_segments + 1) * 2;
               int row2 = m_vertices.size() - (m_segments + 1);
               for ( unsigned x = 0; x < m_segments; x++ )
               {
                  log_debug( "%d,%d,%d,%d\n", row1 + x, row1 + x + 1, row2 + x, row2 + x + 1 );
                  int t1 = model->addTriangle( m_vertices[ row2 + x ].pos.index, m_vertices[ row1 + x + 1].pos.index, m_vertices[ row1 + x ].pos.index );
                  int t2 = model->addTriangle( m_vertices[ row2 + x ].pos.index, m_vertices[ row2 + x + 1].pos.index, m_vertices[ row1 + x + 1].pos.index );
                  m_triangles.push_back( t1 );
                  m_triangles.push_back( t2 );

                  if ( side >= 2 )
                  {
                     model->invertNormals( t1 );
                     model->invertNormals( t2 );
                  }
               }
            }
         }
      }

      unsigned count = m_vertices.size();
      for ( unsigned sv = 0; sv < count; sv++ )
      {
         model->selectVertex( m_vertices[sv].pos.index );
      }

      updateVertexCoords( parent, m_x1, m_y1, m_z1,
            m_x1, m_y1, m_z1 );

      parent->updateAllViews();

      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Cube created" ).utf8() );
   }
}

void CubeTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   if ( parent != m_parent )
   {
      log_error( "Can't serve two parents at once\n" );
   }

   if ( m_tracking )
   {
      m_tracking = false;

      double x2 = 0.0;
      double y2 = 0.0;
      double z2 = 0.0;

      parent->getParentXYValue( x, y, x2, y2 );

      updateVertexCoords( parent, m_x1, m_y1, m_z1,
            x2, y2, z2 );

      weldSelectedVertices( parent->getModel() );
      parent->updateAllViews();

      m_vertices.clear();
      m_triangles.clear();
   }
}

void CubeTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   if ( parent != m_parent )
   {
      log_error( "Can't serve two parents at once\n" );
   }

   if ( m_tracking )
   {
      double x2 = 0.0;
      double y2 = 0.0;
      double z2 = 0.0;

      parent->getParentXYValue( x, y, x2, y2 );

      updateVertexCoords( parent, m_x1, m_y1, m_z1,
            x2, y2, z2 );

      parent->updateAllViews();
   }
}

void CubeTool::updateVertexCoords( Parent * parent, double x1, double y1, double z1,
      double x2, double y2, double z2 )
{
   Model * model = parent->getModel();
   bool invert = false;

   double xdiff = x2 - x1;
   double ydiff = y2 - y1;
   double zdiff = z2 - z1;

   _cubify( m_isCube, z1, zdiff, xdiff, ydiff );

   if ( y1 < y2 )
   {
      invert = !invert;
   }
   if ( x2 > x1 )
   {
      invert = !invert;
   }

   ToolCoordList::iterator it;

   for ( it = m_vertices.begin(); it != m_vertices.end(); it++ )
   {
      movePosition( parent, (*it).pos, 
            (*it).oldCoords[0]*xdiff + x1, 
            (*it).oldCoords[1]*ydiff + y1, 
            (*it).oldCoords[2]*zdiff + z1 );
   }

   if ( invert != m_invertedNormals )
   {
      unsigned count = m_triangles.size();
      for ( unsigned t = 0; t < count; t++ )
      {
         model->invertNormals( m_triangles[t] );
      }

      m_invertedNormals = invert;
   }
}

void CubeTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   m_widget = new CubeToolWidget( this, mainwin );
   m_widget->show();
}

void CubeTool::deactivated()
{
   m_widget->close();
}

const char ** CubeTool::getPixmap()
{
   return (const char **) cubetool_xpm;
}

void CubeTool::setCubeValue( bool newValue )
{
   m_isCube = newValue;
   log_debug( "isCube = %s\n", newValue ? "true" : "false" );
}

void CubeTool::setSegmentValue( int newValue )
{
   if ( newValue >= 1 )
   {
      m_segments = newValue;
   }
}

const char * CubeTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Cube" );
}

