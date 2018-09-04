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
#include "rectangletool.h"
#include "pixmap/rectangletool.xpm"
#include "log.h"
#include "model.h"
#include "modelstatus.h"

#include <QtCore/QObject>
#include <QtWidgets/QApplication>

RectangleTool::RectangleTool()
   : m_tracking( false ),
     m_parent( NULL )
{
}

RectangleTool::~RectangleTool()
{
}

void RectangleTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   if ( !m_tracking )
   {
      m_parent   = parent; // Keep track of which parent we're serving
      m_tracking = true;

      m_x1 = 0.0;
      m_y1 = 0.0;

      parent->getParentXYValue( x, y, m_x1, m_y1 );

      Model * model = parent->getModel();

      model->unselectAll();

      log_debug( "model has %d vertices\n", model->getVertexCount() );

      m_v1 = addPosition( parent, Model::PT_Vertex, NULL,
            m_x1, m_y1, 0.0 );
      m_v2 = addPosition( parent, Model::PT_Vertex, NULL,
            m_x1, m_y1, 0.0 );
      m_v3 = addPosition( parent, Model::PT_Vertex, NULL,
            m_x1, m_y1, 0.0 );
      m_v4 = addPosition( parent, Model::PT_Vertex, NULL,
            m_x1, m_y1, 0.0 );

      log_debug( "last new vertex: %d\n", m_v4.pos.index );

      model->addTriangle( m_v1.pos.index, m_v2.pos.index, m_v4.pos.index );
      model->addTriangle( m_v4.pos.index, m_v3.pos.index, m_v1.pos.index );

      model->selectVertex( m_v1.pos.index );
      model->selectVertex( m_v2.pos.index );
      model->selectVertex( m_v3.pos.index );
      model->selectVertex( m_v4.pos.index );

      parent->updateAllViews();

      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Rectangle created" ).toUtf8() );
   }
}

void RectangleTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   m_tracking = false;
}

void RectangleTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   if ( parent != m_parent )
   {
      log_error( "Can't serve two parents at once\n" );
   }

   if ( m_tracking )
   {
      double x2 = 0.0;
      double y2 = 0.0;

      parent->getParentXYValue( x, y, x2, y2 );

      movePosition( parent, m_v2.pos, 
            m_x1, y2, 0.0 );
      movePosition( parent, m_v3.pos, 
            x2, m_y1, 0.0 );
      movePosition( parent, m_v4.pos, 
            x2, y2, 0.0 );

      parent->updateAllViews();
   }
}

const char ** RectangleTool::getPixmap()
{
   return (const char **) rectangletool_xpm;
}

const char * RectangleTool::getPath()
{
   return TOOLS_CREATE_MENU;
}

const char * RectangleTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Rectangle" );
}

