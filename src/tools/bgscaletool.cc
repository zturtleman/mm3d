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
#include "bgscaletool.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "glmath.h"
#include "pixmap/bgscaletool.xpm"

#include <math.h>
#include <qobject.h>
#include <qapplication.h>

BgScaleTool::BgScaleTool()
{
}

BgScaleTool::~BgScaleTool()
{
}

const char * BgScaleTool::getPath()
{
   return TOOLS_BACKGROUND_MENU;
}

const char * BgScaleTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Scale Background Image" );
}

bool BgScaleTool::getKeyBinding( int arg, int & keyBinding )
{
   return false;
}

void BgScaleTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   LOG_PROFILE();
   Model * model = parent->getModel();

   int index = (int) parent->getViewDirection() - 1;

   if ( index >= 0 )
   {
      float cenX = 0.0f;
      float cenY = 0.0f;
      float cenZ = 0.0f;

      m_x = 0.0f;
      m_y = 0.0f;
      m_z = 0.0f;

      parent->getXValue( x, y, &m_x );
      parent->getYValue( x, y, &m_y );
      parent->getZValue( x, y, &m_z );

      float tempX = m_x;
      float tempY = m_y;
      float tempZ = m_z;

      model->getBackgroundCenter( index, cenX, cenY, cenZ );
      m_startScale  = model->getBackgroundScale( index );
      m_startLength = distance( cenX, cenY, cenZ, tempX, tempY, tempZ );
      if ( m_startLength < 0.0001f )
      {
         m_startLength = 0.0001f;
      }

      log_debug( "center (%f,%f,%f) mouse (%f,%f,%f)\n",
            cenX, cenY, cenZ, tempX, tempY, tempZ );
      log_debug( "starting background scale with length %f, scale %f\n", 
            m_startLength, m_startScale );

      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Scaling background image" ).utf8() );
   }
   else
   {
      model_status( model, StatusError, STATUSTIME_SHORT, qApp->translate( "Tool", "Cannot scale background from 3D view" ).utf8() );
   }
}

void BgScaleTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
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

      float tempX = newX;
      float tempY = newY;
      float tempZ = newZ;

      model->getBackgroundCenter( index, cenX, cenY, cenZ );
      float length = distance( cenX, cenY, cenZ, tempX, tempY, tempZ );
      float scale  = length * (m_startScale / m_startLength );

      log_debug( "scale with length %f, to scale %f\n", 
            length, scale );

      model->setBackgroundScale( index, scale );
   }

   parent->updateAllViews();
}

void BgScaleTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Background scale complete" ).utf8() );
}

const char ** BgScaleTool::getPixmap()
{
   return (const char **) bgscaletool_xpm;
}

void BgScaleTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   model_status( model, StatusNormal, STATUSTIME_NONE, qApp->translate( "Tool", "Scale background image" ).utf8() );
   //m_widget = new BgScaleToolWidget( this, mainwin );
}

void BgScaleTool::deactivated()
{
   //m_widget->close();
}

