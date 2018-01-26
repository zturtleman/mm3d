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


#include "movetool.h"
#include "pixmap/movetool.xpm"
#include "glmath.h"
#include "model.h"
#include "modelstatus.h"

#include <math.h>
#include <QtCore/QObject>
#include <QtWidgets/QApplication>

#include "log.h"

MoveTool::MoveTool()
{
}

MoveTool::~MoveTool()
{
}

void MoveTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   m_x = 0.0;
   m_y = 0.0;

   parent->getParentXYValue( x, y, m_x, m_y, true );

   m_allowX = true;
   m_allowY = true;

   m_viewInverse = parent->getParentViewInverseMatrix();

   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Moving selected primitives" ).toUtf8() );
}

void MoveTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
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

   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Move complete" ).toUtf8() );
}

void MoveTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
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

const char ** MoveTool::getPixmap()
{
   return (const char **) movetool_xpm;
}

void MoveTool::activated( int argc, Model * model, QMainWindow * mainwin )
{
   model_status( model, StatusNormal, STATUSTIME_NONE, qApp->translate( "Tool", "Tip: Hold shift to restrict movement to one dimension" ).toUtf8() );
}

const char * MoveTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Move" );
}

