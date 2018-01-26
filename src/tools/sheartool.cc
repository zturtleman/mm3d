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


#include "sheartool.h"

#include "model.h"
#include "modelstatus.h"
#include "pixmap/sheartool.xpm"
#include "log.h"

#include <QtCore/QObject>
#include <QtWidgets/QApplication>
#include <math.h>

ShearTool::ShearTool()
{
}

ShearTool::~ShearTool()
{
}

const char * ShearTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Shear" );
}

bool ShearTool::getKeyBinding( int arg, int & keyBinding )
{
   return false;
}

void ShearTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   m_positionCoords.clear();

   Model * model = parent->getModel();
   list<Model::Position> posList;

   model->getSelectedPositions( posList );

   ToolCoordList::iterator it;

   m_positionCoords.clear();

   makeToolCoordList( parent, m_positionCoords, posList );

   double cminX = 0;
   double cminY = 0;
   double cmaxX = 0;
   double cmaxY = 0;

   bool setFirst = false;

   for ( it = m_positionCoords.begin(); it != m_positionCoords.end(); it++ )
   {
      // update range
      if ( !setFirst )
      {
         cminX = (*it).oldCoords[0];
         cminY = (*it).oldCoords[1];
         cmaxX = (*it).oldCoords[0];
         cmaxY = (*it).oldCoords[1];
         setFirst = true;
      }
      else
      {
         if ( (*it).oldCoords[0] < cminX ) { cminX = (*it).oldCoords[0]; }
         if ( (*it).oldCoords[0] > cmaxX ) { cmaxX = (*it).oldCoords[0]; }
         if ( (*it).oldCoords[1] < cminY ) { cminY = (*it).oldCoords[1]; }
         if ( (*it).oldCoords[1] > cmaxY ) { cmaxY = (*it).oldCoords[1]; }
      }
   }

   double curX = 0;
   double curY = 0;
   
   double minX = 0;
   double minY = 0;
   double maxX = 0;
   double maxY = 0;

   m_startLengthX = 0;
   m_startLengthY = 0;

   parent->getParentXYValue( x, y, curX, curY, true );

   minX = fabs( cminX - curX );
   minY = fabs( cminY - curY );
   maxX = fabs( cmaxX - curX );
   maxY = fabs( cmaxY - curY );

   if ( minX > maxX )
   {
      if ( minX > minY )
      {
         if ( minX > maxY )
         {
            m_axis = 1;
            m_far  = cminX;
            m_orig = curY;
         }
         else
         {
            m_axis = 0;
            m_far  = cmaxY;
            m_orig = curX;
         }
      }
      else
      {
         // minY > cminX
         if ( minY > maxY )
         {
            m_axis = 0;
            m_far  = cminY;
            m_orig = curX;
         }
         else
         {
            m_axis = 0;
            m_far  = cmaxY;
            m_orig = curX;
         }
      }
   }
   else
   {
      // maxX > minX
      if ( maxX > minY )
      {
         if ( maxX > maxY )
         {
            m_axis = 1;
            m_far  = cmaxX;
            m_orig = curY;
         }
         else
         {
            m_axis = 0;
            m_far  = cmaxY;
            m_orig = curX;
         }
      }
      else
      {
         // minY > maxX
         if ( minY > maxY )
         {
            m_axis = 0;
            m_far  = cminY;
            m_orig = curX;
         }
         else
         {
            m_axis = 0;
            m_far  = cmaxY;
            m_orig = curX;
         }
      }
   }

   m_startLengthX = distance( m_far, 0, curX, 0 );
   m_startLengthY = distance( m_far, 0, curY, 0 );

   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Starting shear on selected primitives" ).toUtf8() );
}

void ShearTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   double curX = 0;
   double curY = 0;

   parent->getParentXYValue( x, y, curX, curY );

   ToolCoordList::iterator it;

   if ( m_axis == 0 )
   {
      double offset = curX - m_orig;
      for( it = m_positionCoords.begin(); it != m_positionCoords.end(); it++ )
      {
         double x = (*it).oldCoords[0];
         double y = (*it).oldCoords[1];
         double z = (*it).oldCoords[2];

         x = x + (offset * (fabs( y - m_far) / m_startLengthY) );

         movePosition( parent, (*it).pos, x, y, z );
      }
   }
   else
   {
      double offset = curY - m_orig;
      for( it = m_positionCoords.begin(); it != m_positionCoords.end(); it++ )
      {
         double x = (*it).oldCoords[0];
         double y = (*it).oldCoords[1];
         double z = (*it).oldCoords[2];

         y = y + (offset * (fabs(x - m_far) / m_startLengthX) );

         movePosition( parent, (*it).pos, x, y, z );
      }
   }

   parent->updateAllViews();
}

void ShearTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Shear complete" ).toUtf8() );
}

const char ** ShearTool::getPixmap()
{
   return (const char **) sheartool_xpm;
}

double ShearTool::distance( const double & x1, const double & y1, const double & x2, const double & y2 )
{
   double xDiff = x2 - x1;
   double yDiff = y2 - y1;
   return sqrt( xDiff*xDiff + yDiff*yDiff );
}

double ShearTool::max( double a, double b )
{
   return ( a > b ) ? a : b;
}
