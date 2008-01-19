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


#include "vertextool.h"

#include "model.h"
#include "msg.h"
#include "log.h"
#include "modelstatus.h"

#include "pixmap/vertextool.xpm"

#include <QObject>
#include <QApplication>

VertexTool::VertexTool()
{
   m_vertex.pos.type = Model::PT_Point;
}

VertexTool::~VertexTool()
{
}


void VertexTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   double coord[3] = {0,0,0};

   parent->getParentXYValue( x, y, coord[0], coord[1], true );

   m_vertex = addPosition( parent, Model::PT_Vertex, NULL, 
         coord[0], coord[1], coord[2] );
   model->setVertexFree( m_vertex.pos.index, true );

   model->unselectAll();
   model->selectVertex( m_vertex.pos.index );

   parent->updateAllViews();

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Vertex created" ).toUtf8() );
}

void VertexTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   if ( m_vertex.pos.type == Model::PT_Vertex )
   {
      double coord[3] = {0,0,0};

      parent->getParentXYValue( x, y, coord[0], coord[1] );

      movePosition( parent, m_vertex.pos, 
            coord[0], coord[1], coord[2] );

      parent->updateAllViews();
   }
}

void VertexTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
}

const char ** VertexTool::getPixmap()
{
   return (const char **) vertextool_xpm;
}

const char * VertexTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Vertex" );
}

