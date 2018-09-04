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


#include "menuconf.h"
#include "bgmovetool.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "glmath.h"
#include "pixmap/bgmovetool.xpm"

#include <math.h>

BgMoveTool::BgMoveTool()
{
}

BgMoveTool::~BgMoveTool()
{
}

const char * BgMoveTool::getPath()
{
   return TOOLS_BACKGROUND_MENU;
}

const char * BgMoveTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Move Background Image" );
}

bool BgMoveTool::getKeyBinding( int arg, int & keyBinding )
{
   return false;
}

void BgMoveTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   LOG_PROFILE();
   Model * model = parent->getModel();

   int index = (int) parent->getViewDirection() - 1;

   if ( index >= 0 )
   {
      m_lastX = 0;
      m_lastX = 0;
      m_lastX = 0;

      parent->getXValue( x, y, &m_lastX );
      parent->getYValue( x, y, &m_lastY );
      parent->getZValue( x, y, &m_lastZ );

      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Moving background image" ).toUtf8() );
   }
   else
   {
      model_status( model, StatusError, STATUSTIME_SHORT, qApp->translate( "Tool", "Cannot move background from 3D view" ).toUtf8() );
   }
}

void BgMoveTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   LOG_PROFILE();

   Model * model = parent->getModel();

   int index = (int) parent->getViewDirection() - 1;

   if ( index >= 0 )
   {
      float cenX = 0.0f;
      float cenY = 0.0f;
      float cenZ = 0.0f;

      double newX = 0.0f;
      double newY = 0.0f;
      double newZ = 0.0f;

      parent->getXValue( x, y, &newX );
      parent->getYValue( x, y, &newY );
      parent->getZValue( x, y, &newZ );

      model->getBackgroundCenter( index, cenX, cenY, cenZ );
      cenX += (newX - m_lastX);
      cenY += (newY - m_lastY);
      cenZ += (newZ - m_lastZ);
      model->setBackgroundCenter( index, cenX, cenY, cenZ );

      m_lastX = newX;
      m_lastY = newY;
      m_lastZ = newZ;
   }

   parent->updateAllViews();
}

void BgMoveTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Background move complete" ).toUtf8() );
}

const char ** BgMoveTool::getPixmap()
{
   return (const char **) bgmovetool_xpm;
}

void BgMoveTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   model_status( model, StatusNormal, STATUSTIME_NONE, qApp->translate( "Tool", "Move background image" ).toUtf8() );
}

void BgMoveTool::deactivated()
{
   //m_widget->close();
}

