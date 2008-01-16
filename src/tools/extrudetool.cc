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


#include "extrudetool.h"
#include "pixmap/extrudetool.xpm"
#include "glmath.h"
#include "model.h"
#include "modelstatus.h"

#include <math.h>
#include <qobject.h>
#include <qapplication.h>

#include "log.h"

ExtrudeTool::ExtrudeTool()
{
}

ExtrudeTool::~ExtrudeTool()
{
}

void ExtrudeTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   m_x = 0.0;
   m_y = 0.0;

   parent->getParentXYValue( x, y, m_x, m_y, true );

   m_allowX = true;
   m_allowY = true;

   m_viewInverse = parent->getParentViewInverseMatrix();

   m_model = parent->getModel();
   extrudeEvent();
}

void ExtrudeTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   /*
   Model * model = parent->getModel();
   unsigned pcount = model->getProjectionCount();
   for ( unsigned p = 0; p < pcount; p++ )
   {
      if ( model->isProjectionSelected( p ) )
      {
         model->applyProjection(p);
      }
   }
   */
   parent->updateAllViews();

   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Extrude complete" ).utf8() );
}

void ExtrudeTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   Matrix m;

   double x2 = m_x;
   double y2 = m_y;

   parent->getParentXYValue( x, y, x2, y2 );

   if ( buttonState & BS_Shift )
   {
      if ( m_allowX && m_allowY )
      {
         double ax = fabs( x2 - m_x );
         double ay = fabs( y2 - m_y );

         if ( ax > ay )
         {
            m_allowY = false;
         }
         if ( ay > ax )
         {
            m_allowX = false;
         }
      }
   }

   if ( !m_allowX )
   {
      x2 = m_x;
   }
   if ( !m_allowY )
   {
      y2 = m_y;
   }

   double v[4] = { x2 - m_x, y2 - m_y, 0.0, 1.0 };

   m_x = x2;
   m_y = y2;

   m_viewInverse.apply3( v );

   m.set( 3, 0, v[0] );
   m.set( 3, 1, v[1] );
   m.set( 3, 2, v[2] );

   parent->getModel()->translateSelected( m );
   parent->updateAllViews();
}

const char ** ExtrudeTool::getPixmap()
{
   return (const char **) extrudetool_xpm;
}

void ExtrudeTool::activated( int argc, Model * model, QMainWindow * mainwin )
{
   model_status( model, StatusNormal, STATUSTIME_NONE, qApp->translate( "Tool", "Tip: Hold shift to restrict movement to one dimension" ).utf8() );
}

const char * ExtrudeTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Extrude" );
}

// Extrude methods

void ExtrudeTool::extrudeEvent()
{
   m_sides.clear();
   m_evMap.clear();

   model_status( m_model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Extrude complete").utf8() );

   list<int> faces;
   m_model->getSelectedTriangles( faces );
   list<int> vertices;
   m_model->getSelectedVertices( vertices );
   if ( faces.empty() )
   {
      model_status( m_model, StatusError, STATUSTIME_LONG, qApp->translate( "Tool", "Must have faces selected to extrude" ).utf8() );
   }
   else
   {
      // Find edges (sides with count==1)
      list<int>::iterator it;

      for ( it = faces.begin(); it != faces.end(); it++ )
      {
         unsigned v[3];

         for ( int t = 0; t < 3; t++ )
         {
            v[t] = m_model->getTriangleVertex( *it, t );
         }
         for ( int t = 0; t < (3 - 1); t++ )
         {
            addSide( v[t], v[t+1] );
         }
         addSide( v[0], v[2] );
      }

      // make extruded vertices and create a map from old vertices
      // to new vertices
      for ( it = vertices.begin(); it != vertices.end(); it++ )
      {
         double coord[3];
         m_model->getVertexCoords( *it, coord );
         unsigned i = m_model->addVertex( coord[0], coord[1], coord[2] );
         m_evMap[*it] = i;

         log_debug( "added vertex %d for %d at %f,%f,%f\n", m_evMap[*it], *it, coord[0], coord[1], coord[2] );
      }

      // Add faces for edges
      for ( it = faces.begin(); it != faces.end(); it++ )
      {
         unsigned v[3];

         for ( int t = 0; t < 3; t++ )
         {
            v[t] = m_model->getTriangleVertex( *it, t );
         }
         for ( int t = 0; t < (3 - 1); t++ )
         {
            if ( sideIsEdge( v[t], v[t+1] ) )
            {
               makeFaces( v[t], v[t+1] );
            }
         }
         if ( sideIsEdge( v[2], v[0] ) )
         {
            makeFaces( v[2], v[0] );
         }
      }

      // Map selected faces onto extruded vertices
      for ( it = faces.begin(); it != faces.end(); it++ )
      {
         unsigned tri = *it;

         int v1 = m_model->getTriangleVertex( tri, 0 );
         int v2 = m_model->getTriangleVertex( tri, 1 );
         int v3 = m_model->getTriangleVertex( tri, 2 );

         /*
         // TODO widget for back-facing
         if ( m_backFaceCheckbox->isChecked() )
         {
            int newTri = m_model->addTriangle( v1, v2, v3 );
            m_model->invertNormals( newTri );
         }
         */

         log_debug( "face %d uses vertices %d,%d,%d\n", *it, v1, v2, v3 );

         m_model->setTriangleVertices( tri,
               m_evMap[ m_model->getTriangleVertex( tri, 0 ) ],
               m_evMap[ m_model->getTriangleVertex( tri, 1 ) ],
               m_evMap[ m_model->getTriangleVertex( tri, 2 ) ] );

         log_debug( "moved face %d to vertices %d,%d,%d\n", *it,
               m_model->getTriangleVertex( tri, 0 ),
               m_model->getTriangleVertex( tri, 1 ),
               m_model->getTriangleVertex( tri, 2 ) );

      }

      // Update face selection

      ExtrudedVertexMap::iterator evit;
      for ( evit = m_evMap.begin(); evit != m_evMap.end(); evit++ )
      {
         m_model->unselectVertex( (*evit).first );
         m_model->selectVertex( (*evit).second );
      }

      m_model->deleteOrphanedVertices();
      model_status( m_model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Extruding selected faces" ).utf8() );
   }
}

void ExtrudeTool::makeFaces( unsigned a, unsigned b )
{
   unsigned a2 = m_evMap[a];
   unsigned b2 = m_evMap[b];

   m_model->addTriangle( b, b2, a2 );
   m_model->addTriangle( a2, a, b );
}

void ExtrudeTool::addSide( unsigned a, unsigned b )
{
   // Make sure a < b to simplify comparison below
   if ( b < a )
   {
      unsigned c = a;
      a = b;
      b = c;
   }

   // Find existing side (if any) and increment count
   SideList::iterator it;
   for ( it = m_sides.begin(); it != m_sides.end(); it++ )
   {
      if ( (*it).a == a && (*it).b == b )
      {
         (*it).count++;
         log_debug( "side (%d,%d) = %d\n", a, b, (*it).count );
         return;
      }
   }

   // Not found, add new side with a count of 1
   SideT s;
   s.a = a;
   s.b = b;
   s.count = 1;

   log_debug( "side (%d,%d) = %d\n", a, b, s.count );
   m_sides.push_back( s );
}

bool ExtrudeTool::sideIsEdge( unsigned a, unsigned b )
{
   // Make sure a < b to simplify comparison below
   if ( b < a )
   {
      unsigned c = a;
      a = b;
      b = c;
   }

   SideList::iterator it;
   for ( it = m_sides.begin(); it != m_sides.end(); it++ )
   {
      if ( (*it).a == a && (*it).b == b )
      {
         if ( (*it).count == 1 )
         {
            return true;
         }
         else
         {
            return false;
         }
      }
   }
   return false;
}

