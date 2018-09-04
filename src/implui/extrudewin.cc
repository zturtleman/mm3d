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


#include "extrudewin.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "decalmgr.h"
#include "3dmprefs.h"
#include "helpwin.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QShortcut>

using std::list;
using std::map;

ExtrudeWin::ExtrudeWin( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_model( model )
{
   setupUi( this );
   setModal( true );

   if ( g_prefs.exists( "ui_extrude_makebackfaces" ) )
   {
      int val = g_prefs( "ui_extrude_makebackfaces" ).intValue();
      if ( val != 0 )
      {
         m_backFaceCheckbox->setChecked( true );
      }
      else
      {
         m_backFaceCheckbox->setChecked( false );
      }
   }

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

ExtrudeWin::~ExtrudeWin()
{
}

void ExtrudeWin::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_commands.html#extrude", true );
   win->show();
}

void ExtrudeWin::absoluteExtrudeEvent()
{
   m_sides.clear();
   m_evMap.clear();

   model_status( m_model, StatusNormal, STATUSTIME_SHORT, tr("Extrude complete").toUtf8() );

   // get extrude arguments
   double x = m_xEdit->text().toDouble();
   double y = m_yEdit->text().toDouble();
   double z = m_zEdit->text().toDouble();

   list<int> faces;
   m_model->getSelectedTriangles( faces );
   list<int> vertices;
   m_model->getSelectedVertices( vertices );

   log_debug( "extruding %d faces on %f,%f,%f\n", faces.size(), x, y, z );

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
      coord[0] += x;
      coord[1] += y;
      coord[2] += z;
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

      if ( m_backFaceCheckbox->isChecked() )
      {
         int newTri = m_model->addTriangle( v1, v2, v3 );
         m_model->invertNormals( newTri );
      }

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

   m_model->operationComplete( tr( "Extrude", "operation complete" ).toUtf8() );
   DecalManager::getInstance()->modelUpdated( m_model );
}

void ExtrudeWin::normalExtrudeEvent()
{
   /*
   m_sides.clear();
   m_evMap.clear();

   double magnitude = m_normalEdit->text().toDouble();

   list<int> faces     = m_model->getSelectedTriangles();
   list<int> vertices  = m_model->getSelectedVertices();

   // Find edges (sides with count==1)
   list<int>::iterator it;
   
   for ( it = faces.begin(); it != faces.end(); it++ )
   {
      unsigned v[3];

      for ( int t = 0; t < 3; t++ )
      {
         v[t] = m_model->getTriangleVertex( *it, t );

         // Capture normal while we're here
         // Note that if normals aren't smoothed the extrude may not
         // have the intended effect
         VertexNormal vn;
         m_model->getNormal( *it, t, vn.val );
         m_vnMap[ v[t] ] = vn;
         log_debug( "vertex %d normals: %f,%f,%f\n", v[t], vn.val[0], vn.val[1], vn.val[2] );
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

      VertexNormal vn = m_vnMap[ *it ];
      coord[0] += vn.val[0] * magnitude;
      coord[1] += vn.val[1] * magnitude;
      coord[2] += vn.val[2] * magnitude;

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

      log_debug( "face %d uses vertices %d,%d,%d\n", *it,
         m_model->getTriangleVertex( tri, 0 ),
         m_model->getTriangleVertex( tri, 1 ),
         m_model->getTriangleVertex( tri, 2 ) );

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

   m_model->operationComplete();
   DecalManager::getInstance()->modelUpdated( m_model );
   */
}

void ExtrudeWin::backFacesChanged( bool o )
{
   g_prefs( "ui_extrude_makebackfaces" ) = (o ? 1 : 0);
}

void ExtrudeWin::makeFaces( unsigned a, unsigned b )
{
   unsigned a2 = m_evMap[a];
   unsigned b2 = m_evMap[b];

   m_model->addTriangle( b, b2, a2 );
   m_model->addTriangle( a2, a, b );
}

void ExtrudeWin::addSide( unsigned a, unsigned b )
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

bool ExtrudeWin::sideIsEdge( unsigned a, unsigned b )
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

