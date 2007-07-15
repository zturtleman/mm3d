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


#include "menuconf.h"
#include "atrfartool.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "pixmap/atrfartool.xpm"
#include "glmath.h"

#include <math.h>
#include <qobject.h>
#include <qapplication.h>

AttractFarTool::AttractFarTool()
{
}

AttractFarTool::~AttractFarTool()
{
}

const char * AttractFarTool::getPath()
{
   return TOOLS_ATTRACT_MENU;
}

const char * AttractFarTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Attract Far" );
}

bool AttractFarTool::getKeyBinding( int arg, int & keyBinding )
{
   return false;
}

void AttractFarTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   m_positionCoords.clear();

   Model * model = parent->getModel();

   list<Model::Position> positions;
   model->getSelectedPositions( positions );

   m_positionCoords.clear();
   makeToolCoordList( parent, m_positionCoords, positions );

   ToolCoordList::iterator it;

   bool firstSet = false;

   double curX = 0;
   double curY = 0;
   
   parent->getParentXYValue( x, y, curX, curY, true );

   for ( it = m_positionCoords.begin(); it != m_positionCoords.end(); it++ )
   {
      (*it).dist = distance( curX, curY, 
                       (*it).oldCoords[0], (*it).oldCoords[1] );

      // update range
      if ( !firstSet )
      {
         m_minDistance = (*it).dist;
         m_maxDistance = (*it).dist;
         firstSet = true;
      }
      else
      {
         if ( (*it).dist < m_minDistance ) { m_minDistance = (*it).dist; }
         if ( (*it).dist > m_maxDistance ) { m_maxDistance = (*it).dist; }
      }
   }

   m_startX = curX;
   m_startY = curY;

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Attracting far selected primitives" ).utf8() );
}

void AttractFarTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   LOG_PROFILE();

   double curX = 0;
   double curY = 0;

   parent->getParentXYValue( x, y, curX, curY );

   double lengthX = curX - m_startX;
   double lengthY = curY - m_startY;

   ToolCoordList::iterator it;

   for( it = m_positionCoords.begin(); it != m_positionCoords.end(); it++ )
   {
      double x = (*it).oldCoords[0];
      double y = (*it).oldCoords[1];
      double z = (*it).newCoords[2];

      if ( (m_maxDistance - m_minDistance) >= 0.00001f )
      {
         x += (lengthX * ( ((*it).dist - m_minDistance) / (m_maxDistance - m_minDistance) ) );
         y += (lengthY * ( ((*it).dist - m_minDistance) / (m_maxDistance - m_minDistance) ) );
      }
      else
      {
         x += lengthX;
         y += lengthY;
      }

      movePosition( parent, (*it).pos, x, y, z );
   }

   parent->updateAllViews();
}

void AttractFarTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Attract far complete" ).utf8() );
}

const char ** AttractFarTool::getPixmap()
{
   return (const char **) atrfartool_xpm;
}

double AttractFarTool::max( double a, double b )
{
   return ( a > b ) ? a : b;
}

