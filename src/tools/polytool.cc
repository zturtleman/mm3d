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


#include "polytool.h"

#include "model.h"
#include "msg.h"
#include "log.h"
#include "modelstatus.h"

#include "pixmap/polytool.xpm"

#include <QtGui/QMainWindow>
#include <QtGui/QApplication>

PolyTool::PolyTool()
   : m_model( NULL ),
     m_type( 0 ),
     m_widget( NULL )
{
   m_lastVertex.pos.type = Model::PT_Point;
   m_lastVertex.pos.index = 0;
}

PolyTool::~PolyTool()
{
}

void PolyTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   m_model = model;
   m_widget = new PolyToolWidget( this, mainwin );
   m_widget->show();
}

void PolyTool::deactivated()
{
   m_model->deleteOrphanedVertices();
   m_widget->close();
}

void PolyTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   m_model = model;

   if ( buttonState == BS_Left )
   {
      m_verts.clear();

      IntList selected;
      model->getSelectedVertices( selected );
      IntList::iterator it;
      for ( it = selected.begin(); m_verts.size() < 3 && it != selected.end(); it++ )
      {
         m_verts.push_back( *it );
      }

      double coord[3] = {0,0,0};

      // Technically that last argument could be true, I chose false
      // because for the poly tool you'd end up creating a flat triangle 
      // that would just get deleted
      parent->getParentXYValue( x, y, coord[0], coord[1], false );

      m_lastVertex = addPosition( parent, Model::PT_Vertex, NULL,
            coord[0], coord[1], coord[2] );

      if ( m_verts.size() == 3 )
      {
         if ( m_type == 0 )
         {
            // Fan
            int v = m_verts.front();
            model->unselectVertex( v );

            m_verts.pop_front();
            m_verts.push_back( m_lastVertex.pos.index );
         }
         else
         {
            // Strip
            it = m_verts.begin();
            it++;
            int v = *it;
            m_verts.erase( it );
            model->unselectVertex( v );

            m_verts.push_back( m_lastVertex.pos.index );
         }
      }
      else
      {
         m_verts.push_back( m_lastVertex.pos.index );
      }

      if ( m_verts.size() == 3 )
      {
         it = m_verts.begin();
         int v1 = *it; it++;
         int v2 = *it; it++;
         int v3 = *it; it++;

         int t = model->addTriangle( v1, v2, v3 );

         model->beginSelectionDifference();
         //model->unselectAllTriangles();

         model->selectVertex( m_lastVertex.pos.index );
         //model->selectTriangle( t );

         const Matrix & viewMatrix = parent->getParentViewInverseMatrix();
         Vector viewNorm( 0, 0, 1 );

         viewMatrix.show();
         viewNorm.transform3( viewMatrix );

         float norm[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
         model->calculateNormals();
         model->getNormal( t, 0, norm );
         Vector triNorm( norm[0], norm[1], norm[2] );

         log_debug( "view normal is %f %f %f\n",
               (float) viewNorm[0],
               (float) viewNorm[1],
               (float) viewNorm[2] );
         log_debug( "triangle normal is %f %f %f\n",
               (float) triNorm[0],
               (float) triNorm[1],
               (float) triNorm[2] );

         double d = viewNorm.dot3( triNorm );
         log_debug( "dot product is %f\n", (float) d );

         if( d < 0 )
         {
            model->invertNormals( t );
         }
      }
      else
      {
         model->beginSelectionDifference();
         model->selectVertex( m_lastVertex.pos.index );
      }

      model->endSelectionDifference();

      parent->updateAllViews();
   }
}

void PolyTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   if ( buttonState == BS_Left )
   {
      if ( m_lastVertex.pos.type == Model::PT_Vertex )
      {
         double coord[3] = {0,0,0};

         // I chose false for the same reason as above
         parent->getParentXYValue( x, y, coord[0], coord[1], false );

         movePosition( parent, m_lastVertex.pos, coord[0], coord[1], coord[2] );

         parent->updateAllViews();
      }
   }
}

void PolyTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
}

const char ** PolyTool::getPixmap()
{
   return (const char **) polytool_xpm;
}

void PolyTool::setTypeValue( int newValue )
{
   m_type = newValue;
}

const char * PolyTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Polygon" );
}

