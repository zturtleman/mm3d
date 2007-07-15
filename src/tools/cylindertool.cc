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


#include "cylindertool.h"

#include "pixmap/cylindertool.xpm"

#include "model.h"
#include "glmath.h"
#include "log.h"
#include "modelstatus.h"
#include "mq3compat.h"

#include <math.h>

#include <qobject.h>
#include <qapplication.h>
#include <qwidget.h>

using std::vector;
using std::list;

CylinderTool::CylinderTool()
   : m_segments( 1 ),
     m_sides( 8 ),
     m_inverted( false )
{
}

CylinderTool::~CylinderTool()
{
}

void CylinderTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   log_debug( "cylinder activated\n" );
   m_widget = new CylinderToolWidget( this, mainwin );
#ifdef HAVE_QT4
   //mainwin->addDockWindow( m_widget, DockBottom );
#endif
   m_widget->show();
}

void CylinderTool::deactivated()
{
   m_widget->close();
}

void CylinderTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   model->unselectAll();

   double pos[3] = {0,0,0};
   double rad[3] = {0,0,0};

   m_inverted = false;

   parent->getParentXYValue( x, y, pos[0], pos[1], true );

   m_startX = pos[0];
   m_startY = pos[1];

   unsigned triStart = model->getTriangleCount();

   unsigned points = m_sides;
   ToolCoordT ev;
   unsigned s;
   unsigned r;

   // Create external cylinder
   for ( s = 0; s <= m_segments; s++ )
   {
      double scale = (double) m_scale / 100.0;
      scale = (double) m_scale / 100.0;
      scale = (1.0 - scale) * ((double) s / (double) m_segments) 
         //* (sqrt( 10000.0 * (double) s / (double) m_segments ) / 100.0)
         //* ( sin( ((double) s / (double) m_segments) * PI / 2.0 ) )
         + scale;
      for ( r = 0; r < points; r++ )
      {
         double vx = (double) s / (double) (m_segments);
         double vy = cos( ((double) r / (double) points) * PI * 2.0 ) * scale;
         double vz = sin( ((double) r / (double) points) * PI * 2.0 ) * scale;
         ev = addPosition( parent, Model::PT_Vertex, NULL, 
               vx, vy, vz );
         m_vertices.push_back( ev );
      }

      if ( s > 0 )
      {
         for ( r = 0; r < points; r++ )
         {
            unsigned next = r + 1;
            if ( next >= points )
            {
               next = 0;
            }

            model->addTriangle( m_vertices[((s-1)*points) + r].pos.index, 
                  m_vertices[ ((s-1)*points) + next].pos.index, 
                  m_vertices[ (s*points) + r ].pos.index );
            model->addTriangle( m_vertices[((s-1)*points) + next].pos.index, 
                  m_vertices[ (s*points) + next].pos.index, 
                  m_vertices[ (s*points) + r ].pos.index );
         }
      }
   }

   unsigned off = m_vertices.size();

   // Create internal cylinder
   if ( m_width < 100 )
   {
      double rad = 1.0 - (double) m_width / 100.0;
      if ( rad > 0.999 )
      {
         rad = 0.999;
      }
      for ( s = 0; s <= m_segments; s++ )
      {
         double scale = (double) m_scale / 100.0;
         scale = (1.0 - scale) * ((double) s / (double) m_segments) + scale;
         for ( r = 0; r < points; r++ )
         {
            double vx = (double) s / (double) (m_segments);
            double vy = cos( ((double) r / (double) points) * PI * 2.0 ) * rad * scale;
            double vz = sin( ((double) r / (double) points) * PI * 2.0 ) * rad * scale;
            ev = addPosition( parent, Model::PT_Vertex, NULL, 
                  vx, vy, vz );
            m_vertices.push_back( ev );
         }

         if ( s > 0 )
         {
            for ( r = 0; r < points; r++ )
            {
               unsigned next = r + 1;
               if ( next >= points )
               {
                  next = 0;
               }
               model->addTriangle( m_vertices[ off + ((s-1)*points) + next ].pos.index, 
                     m_vertices[ off + ((s-1)*points) + r ].pos.index, 
                     m_vertices[ off + (s*points) + r ].pos.index );
               model->addTriangle( m_vertices[ off + ((s-1)*points) + next ].pos.index, 
                     m_vertices[ off + (s*points) + r ].pos.index, 
                     m_vertices[ off + (s*points) + next ].pos.index );
            }
         }
      }

      // Add ends
      unsigned off2 = m_vertices.size() - points;
      for ( r = 0; r < points; r++ )
      {
         unsigned next = r + 1;
         if ( next >= points )
         {
            next = 0;
         }
         model->addTriangle( m_vertices[ next ].pos.index, m_vertices[ r ].pos.index, m_vertices[ off + r ].pos.index );
         model->addTriangle( m_vertices[ next ].pos.index, m_vertices[ off + r ].pos.index, m_vertices[ off + next ].pos.index );

         model->addTriangle( m_vertices[ (off - points) + r ].pos.index, m_vertices[ (off - points) + next ].pos.index, m_vertices[ off2 + r ].pos.index );
         model->addTriangle( m_vertices[ (off - points) + next ].pos.index, m_vertices[ off2 + next ].pos.index, m_vertices[ off2 + r ].pos.index );
      }
   }
   else // No internal cylinder
   {
      off = m_vertices.size() - points;

      // Add ends
      ev = addPosition( parent, Model::PT_Vertex, NULL,
            0, 0, 0 );

      for ( r = 0; r < points; r++ )
      {
         unsigned next = r + 1;
         if ( next >= points )
         {
            next = 0;
         }
         model->addTriangle( ev.pos.index, m_vertices[ next ].pos.index, m_vertices[ r ].pos.index );
      }
      m_vertices.push_back( ev );

      ev = addPosition( parent, Model::PT_Vertex, NULL,
            1, 0, 0 );
      m_vertices.push_back( ev );

      for ( r = 0; r < points; r++ )
      {
         unsigned next = r + 1;
         if ( next >= points )
         {
            next = 0;
         }
         model->addTriangle( ev.pos.index, m_vertices[ off + r].pos.index, m_vertices[ off + next].pos.index );
      }
      m_vertices.push_back( ev );

   }

   unsigned triEnd = model->getTriangleCount();

   for ( unsigned t = triStart; t < triEnd; t++ )
   {
      model->selectTriangle( t );
   }

   updateVertexCoords( parent, pos[0], pos[1], pos[2], rad[0], rad[1], rad[2] );

   parent->updateAllViews();

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Cylinder created" ).utf8() );
}

void CylinderTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   m_vertices.clear();
}

void CylinderTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   double pos[3] = {0,0,0};
   double rad[3] = {0,0,0};

   parent->getParentXYValue( x, y, pos[0], pos[1] );

   bool doInvert = false;

   rad[0] = (pos[0] - m_startX);
   rad[1] = fabs(m_startY - pos[1]) / 2;
   rad[2] = fabs(m_startY - pos[1]) / 2;

   pos[0] = m_startX;
   pos[1] = (pos[1] + m_startY) / 2;
   pos[2] = 0;

   if ( rad[0] < 0.0 && m_inverted == false )
   {
      doInvert = true;
   }
   else if ( rad[0] >= 0.0 && m_inverted == true )
   {
      doInvert = true;
   }

   if ( doInvert )
   {
      m_inverted = !m_inverted;
      list<int> selectedList;
      model->getSelectedTriangles( selectedList );
      list<int>::iterator it;
      for ( it = selectedList.begin(); it != selectedList.end(); it++ )
      {
         model->invertNormals( *it );
      }
   }

   updateVertexCoords( parent, pos[0], pos[1], pos[2], rad[0], rad[1], rad[2] );

   parent->updateAllViews();
}

const char ** CylinderTool::getPixmap()
{
   return (const char **) cylindertool_xpm;
}

void CylinderTool::updateVertexCoords( Tool::Parent * parent, double x, double y, double z,
      double xrad, double yrad, double zrad )
{
   ToolCoordList::iterator it;

   for ( it = m_vertices.begin(); it != m_vertices.end(); it++ )
   {
      movePosition( parent, (*it).pos, 
            (*it).oldCoords[0]*xrad + x, 
            (*it).oldCoords[1]*yrad + y, 
            (*it).oldCoords[2]*zrad + z );
   }
}

void CylinderTool::setSegmentsValue( int newValue )
{
   log_debug( "segments: %d\n", newValue );
   m_segments = newValue;
}

void CylinderTool::setSidesValue( int newValue )
{
   log_debug( "sides: %d\n", newValue );
   m_sides = newValue;
}

void CylinderTool::setWidthValue( int newValue )
{
   log_debug( "width: %d\n", newValue );
   m_width = newValue;
}

void CylinderTool::setScaleValue( int newValue )
{
   log_debug( "scale: %d\n", newValue );
   m_scale = newValue;
}

void CylinderTool::setShapeValue( int newValue )
{
   log_debug( "shape: %d\n", newValue );
   m_shape = newValue;
}

const char * CylinderTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Cylinder" );
}

