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
#include "pointtool.h"

#include "model.h"
#include "msg.h"
#include "log.h"
#include "modelstatus.h"

#include "pixmap/pointtool.xpm"

#include <QtCore/QObject>
#include <QtWidgets/QApplication>

PointTool::PointTool()
{
   m_point.pos.type = Model::PT_Vertex;
}

PointTool::~PointTool()
{
}


void PointTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   double coord[3] = {0,0,0};

   parent->getParentXYValue( x, y, coord[0], coord[1], true );

   // Find a unique name for the point
   char name[32] = "Point 1";
   unsigned c = model->getPointCount();
   bool uniqueName = (c == 0) ? true : false;

   for ( unsigned i = 1; !uniqueName && i < 1000; i++ )
   {
      uniqueName = true;
      sprintf( name, "Point %d", i );
      for ( unsigned j = 0; j < c; j++ )
      {
         if ( strcmp( name, model->getPointName( j ) ) == 0 )
         {
            uniqueName = false;
            break;
         }
      }
   }

   // I give up, just call it "Point"
   if ( ! uniqueName )
   {
      strcpy( name, "Point" );
   }

   m_point = addPosition( parent, Model::PT_Point, name, 
         coord[0], coord[1], coord[2], 0, 0, 0 );

   model->unselectAll();
   model->selectPoint( m_point.pos.index );

   parent->updateAllViews();

   model_status( model, StatusNormal, STATUSTIME_SHORT, "%s", qApp->translate( "Tool", "Point created" ).toUtf8().data() );
}

void PointTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   if ( m_point.pos.type == Model::PT_Point )
   {
      double coord[3] = {0,0,0};

      parent->getParentXYValue( x, y, coord[0], coord[1] );

      movePosition( parent, m_point.pos, 
            coord[0], coord[1], coord[2] );

      parent->updateAllViews();
   }
}

void PointTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
}

const char ** PointTool::getPixmap()
{
   return (const char **) pointtool_xpm;
}

const char * PointTool::getPath()
{
   return TOOLS_CREATE_MENU;
}

const char * PointTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Point" );
}

