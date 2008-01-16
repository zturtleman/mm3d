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


#include "scaletool.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "pixmap/scaletool.xpm"

#include <math.h>

#include <qobject.h>
#include <qapplication.h>
#include <qwidget.h>

ScaleTool::ScaleTool()
{
}

ScaleTool::~ScaleTool()
{
}

const char * ScaleTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Scale" );
}

bool ScaleTool::getKeyBinding( int arg, int & keyBinding )
{
   return false;
}

void ScaleTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   m_positionCoords.clear();

   m_allowX = true;
   m_allowY = true;

   Model * model = parent->getModel();
   list<Model::Position> posList;

   model->getSelectedPositions( posList );

   m_positionCoords.clear();
   makeToolCoordList( parent, m_positionCoords, posList );

   ToolCoordList::iterator it;

   bool firstSet = false;

   for ( it = m_positionCoords.begin(); it != m_positionCoords.end(); it++ )
   {
      // update range
      if ( !firstSet )
      {
         m_minX = (*it).oldCoords[0];
         m_minY = (*it).oldCoords[1];
         m_minZ = (*it).oldCoords[2];
         m_maxX = (*it).oldCoords[0];
         m_maxY = (*it).oldCoords[1];
         m_maxZ = (*it).oldCoords[2];
         firstSet = true;
      }
      else
      {
         if ( (*it).oldCoords[0] < m_minX ) { m_minX = (*it).oldCoords[0]; }
         if ( (*it).oldCoords[0] > m_maxX ) { m_maxX = (*it).oldCoords[0]; }
         if ( (*it).oldCoords[1] < m_minY ) { m_minY = (*it).oldCoords[1]; }
         if ( (*it).oldCoords[1] > m_maxY ) { m_maxY = (*it).oldCoords[1]; }
         if ( (*it).oldCoords[2] < m_minZ ) { m_minZ = (*it).oldCoords[2]; }
         if ( (*it).oldCoords[2] > m_maxZ ) { m_maxZ = (*it).oldCoords[2]; }
      }
   }

   double curX = 0;
   double curY = 0;
   
   m_startLengthX = 0;
   m_startLengthY = 0;

   parent->getParentXYValue( x, y, curX, curY, true );

   m_x = curX;
   m_y = curY;

   if ( m_point == ST_ScalePointFar )
   {
      double minmin = 0;
      double minmax = 0;
      double maxmin = 0;
      double maxmax = 0;

      m_farZ = m_minZ;

      minmin = distance( m_minX, m_minY, curX, curY );
      minmax = distance( m_minX, m_maxY, curX, curY );
      maxmin = distance( m_maxX, m_minY, curX, curY );
      maxmax = distance( m_maxX, m_maxY, curX, curY );

      if ( minmin > minmax )
      {
         if ( minmin > maxmin )
         {
            if ( minmin > maxmax )
            {
               m_farX = m_minX;
               m_farY = m_minY;
            }
            else
            {
               m_farX = m_maxX;
               m_farY = m_maxY;
            }
         }
         else
         {
            // maxmin > minmin
            if ( maxmin > maxmax )
            {
               m_farX = m_maxX;
               m_farY = m_minY;
            }
            else
            {
               m_farX = m_maxX;
               m_farY = m_maxY;
            }
         }
      }
      else
      {
         // minmax > minmin
         if ( minmax > maxmin )
         {
            if ( minmax > maxmax )
            {
               m_farX = m_minX;
               m_farY = m_maxY;
            }
            else
            {
               m_farX = m_maxX;
               m_farY = m_maxY;
            }
         }
         else
         {
            // maxmin > minmax
            if ( maxmin > maxmax )
            {
               m_farX = m_maxX;
               m_farY = m_minY;
            }
            else
            {
               m_farX = m_maxX;
               m_farY = m_maxY;
            }
         }
      }

      m_startLengthX = fabs( m_farX - curX );
      m_startLengthY = fabs( m_farY - curY );
   }
   else
   {
      m_centerX = (m_maxX - m_minX) / 2.0 + m_minX;
      m_centerY = (m_maxY - m_minY) / 2.0 + m_minY;
      m_centerZ = (m_maxZ - m_minZ) / 2.0 + m_minZ;

      m_startLengthX = fabs( m_centerX - curX );
      m_startLengthY = fabs( m_centerY - curY );
   }

   m_projList.clear();
   // remove projections (special case)
   for ( it = m_positionCoords.begin(); it != m_positionCoords.end(); it++ )
   {
      log_debug( "checking for projection\n" );
      if ( (*it).pos.type == Model::PT_Projection )
      {
         log_debug( "found projection %d\n", (*it).pos.index );
         m_projList.push_back( (*it).pos.index );
      }
   }
   while ( !m_positionCoords.empty() && m_positionCoords.back().pos.type == Model::PT_Projection )
   {
      log_debug( "removing projection\n" );
      m_positionCoords.pop_back();
   }
   if ( !m_projList.empty() )
   {
      log_debug( "getting scale\n" );
      m_projScale = model->getProjectionScale( m_projList.front() );
   }

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Scaling selected primitives" ).utf8() );
}

void ScaleTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   LOG_PROFILE();

   double curX = m_x;
   double curY = m_y;

   parent->getParentXYValue( x, y, curX, curY );

   if ( buttonState & BS_Shift )
   {
      if ( m_allowX && m_allowY )
      {
         double ax = fabs( curX - m_x );
         double ay = fabs( curY - m_y );

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
      curX = m_x;
   }
   if ( !m_allowY )
   {
      curY = m_y;
   }

   double spX = ( m_point == ST_ScalePointFar ) ? m_farX : m_centerX;
   double spY = ( m_point == ST_ScalePointFar ) ? m_farY : m_centerY;
   double spZ = ( m_point == ST_ScalePointFar ) ? m_farZ : m_centerZ;

   double lengthX = distance( spX, 0, curX, 0 );
   double lengthY = distance( spY, 0, curY, 0 );

   ToolCoordList::iterator it;

   for( it = m_positionCoords.begin(); it != m_positionCoords.end(); it++ )
   {
      double x = (*it).oldCoords[0];
      double y = (*it).oldCoords[1];
      double z = (*it).oldCoords[2];

      x -= spX;
      y -= spY;
      z -= spZ;

      double xper = (lengthX / m_startLengthX);
      if ( m_startLengthX <= 0.00006 )
      {
         xper = 1.0;
      }
      double yper = (lengthY / m_startLengthY);
      if ( m_startLengthY <= 0.00006 )
      {
         yper = 1.0;
      }

      if ( m_proportion == ST_ScaleFree )
      {
         x *= xper;
         y *= yper;
      }
      else
      {
         if ( xper > yper )
         {
            x *= xper;
            y *= xper;
            if ( m_proportion == ST_ScaleProportion3D )
            {
               z *= xper;
            }
         }
         else
         {
            x *= yper;
            y *= yper;
            if ( m_proportion == ST_ScaleProportion3D )
            {
               z *= yper;
            }
         }
      }

      x += spX;
      y += spY;
      z += spZ;

      movePosition( parent, (*it).pos, x, y, z );
   }

   if ( !m_projList.empty() )
   {
      log_debug( "setting scale\n" );
      double startLen = sqrt( m_startLengthX * m_startLengthX + m_startLengthY * m_startLengthY);
      double len      = sqrt( lengthX * lengthX + lengthY * lengthY);
      double diff = len / startLen;
      parent->getModel()->setProjectionScale( m_projList.front(), m_projScale * diff );
      log_debug( "new scale = %f\n", (float) m_projScale * diff );
   }

   parent->updateAllViews();
}

void ScaleTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   if ( !m_projList.empty() )
   {
      //parent->getModel()->applyProjection( m_projList.front() );
      //parent->updateAllViews();
   }
   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Scale complete" ).utf8() );
}

const char ** ScaleTool::getPixmap()
{
   return (const char **) scaletool_xpm;
}

double ScaleTool::distance( double x1, double y1, double x2, double y2 )
{
   double xDiff = x2 - x1;
   double yDiff = y2 - y1;
   return sqrt( xDiff*xDiff + yDiff*yDiff );
}

double ScaleTool::max( double a, double b )
{
   return ( a > b ) ? a : b;
}

void ScaleTool::setProportionValue( int newValue )
{
   m_proportion = newValue;
}

void ScaleTool::setPointValue( int newValue )
{
   m_point = newValue;
}

void ScaleTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   model_status( model, StatusNormal, STATUSTIME_NONE, qApp->translate( "Tool", "Tip: Hold shift to restrict scaling to one dimension" ).utf8() );
   m_widget = new ScaleToolWidget( this, mainwin );
#ifdef HAVE_QT4
   //mainwin->addDockWindow( m_widget, DockBottom );
#endif
   m_widget->show();
}

void ScaleTool::deactivated()
{
   m_widget->close();
}

