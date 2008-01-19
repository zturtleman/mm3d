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


#include "ellipsetool.h"

#include "pixmap/ellipsetool.xpm"

#include "model.h"
#include "glmath.h"
#include "log.h"
#include "modelstatus.h"

#include <math.h>
#include <vector>

#include <QMainWindow>
#include <QApplication>

using std::vector;
using std::list;

EllipsoidTool::EllipsoidTool()
   : m_smoothness( 2 ),
     m_isSphere( false ),
     m_fromCenter( false )
{
}

EllipsoidTool::~EllipsoidTool()
{
}

void EllipsoidTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   log_debug( "ellipse activated\n" );
   m_widget = new EllipsoidToolWidget( this, mainwin );
#ifdef HAVE_QT4
   //mainwin->addDockWindow( m_widget, DockBottom );
#endif
   m_widget->show();
}

void EllipsoidTool::deactivated()
{
   m_widget->close();
}

void EllipsoidTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   model->unselectAll();

   // create rough 12-vertex sphere

   // Make top and bottom vertices
   ToolCoordT v1 = addPosition( parent, Model::PT_Vertex, NULL, 
         0, 1, 0 );
   ToolCoordT v2 = addPosition( parent, Model::PT_Vertex, NULL, 
         0, -1, 0 );

   // Make center vertices
   double offset = sin( 30 * PIOVER180 );
   double adjust = cos( 30 * PIOVER180 );

   vector<int> top;
   vector<int> bot;

   ToolCoordT v;
   double xoff;
   double zoff;

   for ( int t = 0; t < 5; t++ )
   {
      // upper vertex
      xoff = sin ( ( t * 72) * PIOVER180 ) * adjust;
      zoff = cos ( ( t * 72) * PIOVER180 ) * adjust;

      v = addPosition( parent, Model::PT_Vertex, NULL,
            xoff, offset, zoff );
      top.push_back( v.pos.index );

      // lower vertex
      xoff = sin ( ( t * 72 + 36) * PIOVER180 ) * adjust;
      zoff = cos ( ( t * 72 + 36) * PIOVER180 ) * adjust;

      v = addPosition( parent, Model::PT_Vertex, NULL,
            xoff, -offset, zoff );
      bot.push_back( v.pos.index );
   }

   // Create top and bottom faces
   model->selectTriangle( model->addTriangle( v1.pos.index, top.back(),  top.front() ) );
   model->selectTriangle( model->addTriangle( v2.pos.index, bot.front(), bot.back()  ) );

   vector<int>::iterator it1;
   vector<int>::iterator it2;
   vector<int>::reverse_iterator rit1;
   vector<int>::reverse_iterator rit2;

   for ( it1 = top.begin(); ; it1++ )
   {
      it2 = it1;
      it2++;

      if ( it2 == top.end() )
      {
         break;
      }

      model->selectTriangle( model->addTriangle( v1.pos.index, *it1, *it2 ) );
   }

   for ( rit1 = bot.rbegin(); ; rit1++ )
   {
      rit2 = rit1;
      rit2++;

      if ( rit2 == bot.rend() )
      {
         break;
      }

      model->selectTriangle( model->addTriangle( v2.pos.index, *rit1, *rit2 ) );
   }

   model->selectTriangle( model->addTriangle( top[0], bot[0], top[1] ) );
   model->selectTriangle( model->addTriangle( top[1], bot[1], top[2] ) );
   model->selectTriangle( model->addTriangle( top[2], bot[2], top[3] ) );
   model->selectTriangle( model->addTriangle( top[3], bot[3], top[4] ) );
   model->selectTriangle( model->addTriangle( top[4], bot[4], top[0] ) );

   model->selectTriangle( model->addTriangle( bot[1], top[1], bot[0] ) );
   model->selectTriangle( model->addTriangle( bot[2], top[2], bot[1] ) );
   model->selectTriangle( model->addTriangle( bot[3], top[3], bot[2] ) );
   model->selectTriangle( model->addTriangle( bot[4], top[4], bot[3] ) );
   model->selectTriangle( model->addTriangle( bot[0], top[0], bot[4] ) );

   // create smooth sphere
   for ( unsigned i = 0; i < m_smoothness; i++ )
   {
      model->subdivideSelectedTriangles();
   }

   list<Model::Position> posList;
   model->getSelectedPositions( posList );

   m_vertices.clear();
   // TODO add makeSelectedToolCoordList?
   makeToolCoordList( parent, m_vertices, posList );

   ToolCoordList::iterator vit;
   for ( vit = m_vertices.begin(); vit != m_vertices.end(); vit++ )
   {
      normalize3( (*vit).oldCoords );
   }

   double pos[3] = {0,0,0};
   double rad[3] = {0,0,0};

   parent->getParentXYValue( x, y, pos[0], pos[1], true );

   m_startX = pos[0];
   m_startY = pos[1];

   updateVertexCoords( parent, pos[0], pos[1], pos[2], rad[0], rad[1], rad[2] );

   parent->updateAllViews();

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Ellipsoid created" ).toUtf8() );
}

void EllipsoidTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   m_vertices.clear();
}

void EllipsoidTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   double pos[3] = {0,0,0};
   double rad[3] = {0,0,0};

   parent->getParentXYValue( x, y, pos[0], pos[1] );

   if ( m_isSphere )
   {
      double sphereRadius = 0;
      double a, b;

      a = fabs((m_startX - pos[0]) / 2);
      b = fabs((m_startY - pos[1]) / 2);
      sphereRadius = (a < b) ? a : b;

      rad[0] = rad[1] = rad[2] = sphereRadius;
   }
   else
   {
      rad[0] = fabs(m_startX - pos[0]) / 2;
      rad[1] = fabs(m_startY - pos[1]) / 2;
      rad[2] = ( rad[0] < rad[1] ) ? rad[0] : rad[1];
   }

   if ( m_fromCenter )
   {
      pos[0] = m_startX;
      pos[1] = m_startY;

      rad[0] *= 2.0;
      rad[1] *= 2.0;
      rad[2] *= 2.0;
   }
   else
   {
      pos[0] = (pos[0] + m_startX) / 2;
      pos[1] = (pos[1] + m_startY) / 2;
   }

   updateVertexCoords( parent, pos[0], pos[1], pos[2], rad[0], rad[1], rad[2] );

   parent->updateAllViews();
}

const char ** EllipsoidTool::getPixmap()
{
   return (const char **) ellipsetool_xpm;
}

void EllipsoidTool::updateVertexCoords( Tool::Parent * parent, double x, double y, double z,
      double xrad, double yrad, double zrad )
{
   ToolCoordList::iterator it;

   // TODO it would probably be considerably faster to copy the matrix
   // logic into this function
   for ( it = m_vertices.begin(); it != m_vertices.end(); it++ )
   {
      movePosition( parent, (*it).pos, 
            (*it).oldCoords[0]*xrad + x, 
            (*it).oldCoords[1]*yrad + y, 
            (*it).oldCoords[2]*zrad + z );
   }
}

void EllipsoidTool::setSmoothnessValue( int newValue )
{
   m_smoothness = newValue;
}

void EllipsoidTool::setSphere( bool o )
{
   m_isSphere = o;
}

void EllipsoidTool::setCenter( bool o )
{
   m_fromCenter = o;
}

const char * EllipsoidTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Ellipsoid" );
}

