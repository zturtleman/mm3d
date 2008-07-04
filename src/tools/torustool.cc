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


#include "torustool.h"

#include "pixmap/torustool.xpm"

#include "model.h"
#include "glmath.h"
#include "log.h"
#include "modelstatus.h"
#include "glmath.h"
#include "weld.h"

#include <math.h>
#include <vector>

#include <QtGui/QMainWindow>
#include <QtGui/QApplication>

using std::vector;
using std::list;

TorusTool::TorusTool()
   : m_segments( 8 ),
     m_sides( 8 ),
     m_width( 50 ),
     m_circle( false ),
     m_center( false )
{
}

TorusTool::~TorusTool()
{
}

void TorusTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   log_debug( "torus activated\n" );
   m_widget = new TorusToolWidget( this, mainwin );
   m_widget->show();
}

void TorusTool::deactivated()
{
   m_widget->close();
}

void TorusTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   model->unselectAll();

   m_inverted = false;

   m_vertices.clear();

   double pos[3] = {0,0,0};

   parent->getParentXYValue( x, y, pos[0], pos[1], true );

   m_startX = pos[0];
   m_startY = pos[1];

   double rx = 0.0;
   double ry = 0.0;

   double cx = 0.0;
   double cy = 0.0;
   double cz = 0.0;

   m_diameter = (double) m_width / 100.0;
   double rad = 1.0 - (m_diameter / 2.0);

   size_t triBase = model->getTriangleCount();

   model->unselectAll();

   for ( unsigned t = 0; t <= m_segments; t++ )
   {
      double angle = (PI * 2) * ((double) t / (double) m_segments );
      rx = cos( angle );
      ry = sin( angle );

      for ( unsigned n = 0; n <= m_sides; n++ )
      {
         angle = (PI * 2) * ((double) n / (double) m_sides );
         double c = cos( angle ) * m_diameter * 0.5;
         cz = sin( angle );

         cx = (rx * rad) + (rx * c);
         cy = (ry * rad) + (ry * c);

         cx += 1.0;
         cy += 1.0;
         cx /= 2.0;
         cy /= 2.0;

         ToolCoordT v = addPosition( parent, Model::PT_Vertex, NULL,
                  cx, cy, cz );

         model->selectVertex( v.pos.index );
         m_vertices.push_back( v );
      }

      if ( t > 0 )
      {
         unsigned vbase1 = model->getVertexCount() - ((m_sides+1) * 2);
         unsigned vbase2 = model->getVertexCount() -  (m_sides+1);

         unsigned i;
         for ( i = 0; i < m_sides; i++ )
         {
            model->addTriangle( vbase1 + i,     vbase1 + i + 1, vbase2 + i );
            model->addTriangle( vbase1 + i + 1, vbase2 + i + 1, vbase2 + i );
         }
      }
   }

   size_t t = 0;
   size_t tcount = model->getTriangleCount();
   for ( t = triBase; t < tcount; t++ )
   {
      model->selectTriangle( t );
   }

   updateDimensions( parent, 0, 0, 0 );

   parent->updateAllViews();

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Torus created" ).toUtf8() );
}

void TorusTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();
   weldSelectedVertices( model );
   m_vertices.clear();
   parent->updateAllViews();
}

void TorusTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   double pos[3] = {0,0,0};

   Model * model = parent->getModel();

   parent->getParentXYValue( x, y, pos[0], pos[1] );

   double xdiff = pos[0] - m_startX;
   double ydiff = pos[1] - m_startY;
   double zdiff = 0;

   bool invert = false;

   if ( m_circle )
   {
      double diff = sqrt( fabs(xdiff * ydiff) );
      ydiff = (ydiff < 0.0) ? -diff : diff;
      xdiff = (xdiff < 0.0) ? -diff : diff;
   }

   zdiff = ( fabs( xdiff ) > fabs( ydiff ) ) 
      ? fabs( xdiff ) 
      : fabs( ydiff );
   zdiff *= (m_diameter * 0.25);

   updateDimensions( parent, xdiff, ydiff, zdiff );

   if ( ( xdiff < 0.0 && ydiff < 0.0 )
         || ( xdiff > 0.0 && ydiff > 0.0 ) )
   {
      invert = true;
   }

   if ( invert != m_inverted )
   {
      m_inverted = !m_inverted;

      size_t tcount = model->getTriangleCount();
      for ( size_t t = 0; t < tcount; t++ )
      {
         if ( model->isTriangleSelected( t ) )
         {
            model->invertNormals( t );
         }
      }
   }

   parent->updateAllViews();
}

const char ** TorusTool::getPixmap()
{
   return (const char **) torustool_xpm;
}

void TorusTool::updateDimensions( Tool::Parent * parent,
      double xdiff, double ydiff, double zdiff )
{
   ToolCoordList::iterator it = m_vertices.begin();

   double centerX = m_startX;
   double centerY = m_startY;

   if ( m_center )
   {
      centerX -= xdiff;
      centerY -= ydiff;

      xdiff *= 2.0;
      ydiff *= 2.0;
      zdiff *= 2.0;
   }

   while ( it != m_vertices.end() )
   {
      movePosition( parent, (*it).pos,
            centerX + xdiff * (*it).oldCoords[0],
            centerY + ydiff * (*it).oldCoords[1],
            0.0     + zdiff * (*it).oldCoords[2] );
      it++;
   }
}

void TorusTool::setSegmentsValue( int newValue )
{
   m_segments = newValue;
}

void TorusTool::setSidesValue( int newValue )
{
   m_sides = newValue;
}

void TorusTool::setWidthValue( int newValue )
{
   m_width = newValue;
}

void TorusTool::setCircleValue( bool newValue )
{
   m_circle = newValue;
}

void TorusTool::setCenterValue( bool newValue )
{
   m_center = newValue;
}

const char * TorusTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Torus" );
}

