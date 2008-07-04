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


#include "config.h"
#include "menuconf.h"
#include "selectfacetool.h"
#include "bounding.h"
#include "decalmgr.h"
#include "log.h"
#include "modelstatus.h"

#include "pixmap/selectfacetool.xpm"

#include <stdio.h>

#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QKeySequence>

class NormalTest : public Model::SelectionTest
{
   public:
      NormalTest( Model * model, const Matrix & mat )
      {
         m_model = model;

         m_mat = mat;
      }

      // These should go into one of the Vector classes
      static double dot( const double v1[3], const double v2[3] )
      {
         return (v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]);
      }

      static double * cross( const double v1[3], const double v2[3], double result[3])
      {
         result[0] = v1[1] * v2[2] - v1[2] * v2[1];
         result[1] = v1[0] * v2[2] - v1[2] * v2[0];
         result[2] = v1[0] * v2[1] - v1[1] * v2[0];

         return result;
      }

      bool shouldSelect( void * element )
      {
         Model::Triangle * tri = static_cast<Model::Triangle *>( element );
         if (tri)
         {
            double v0[3];
            m_model->getVertexCoords( tri->m_vertexIndices[0], v0 );
            m_mat.apply3( v0 );
            double v1[3];
            m_model->getVertexCoords( tri->m_vertexIndices[1], v1 );
            m_mat.apply3( v1 );
            double v2[3];
            m_model->getVertexCoords( tri->m_vertexIndices[2], v2 );
            m_mat.apply3( v2 );

            v1[0] -= v0[0]; v1[1] -= v0[1]; v1[2] -= v0[2];
            v2[0] -= v0[0]; v2[1] -= v0[1]; v2[2] -= v0[2];
            double normal[3];
            cross( v1, v2, normal );

            double vec[3] = { 0.0, 0.0, 1.0 };

            return ( dot( vec, normal ) > 0.0 );
         }
         else
         {
            return false;
         }
      }

   private:
      Model * m_model;
      Matrix  m_mat;
};


SelectFaceTool::SelectFaceTool()
   : m_boundingBox( NULL),
     m_tracking( false ),
     m_unselect( false ),
     m_includeBackfacing( true ),
     m_startX( 0 ),
     m_startY( 0 ),
     m_x1( 0.0 ),
     m_y1( 0.0 ),
     m_selectionMode( Model::SelectTriangles ),
     m_widget( NULL )
{
}

SelectFaceTool::~SelectFaceTool()
{
}

void SelectFaceTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   m_widget = new SelectFaceToolWidget( this, mainwin );
   m_widget->show();
}

void SelectFaceTool::deactivated()
{
   m_widget->close();
}

void SelectFaceTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   if ( m_tracking )
   {
      return;
   }

   parent->getModel()->setSelectionMode( Model::SelectTriangles );

   m_boundingBox = new BoundingBox();
   DecalManager::getInstance()->addDecalToParent( m_boundingBox, parent );

   if ( buttonState & BS_Right )
   {
      m_unselect = true;
   }
   else
   {
      m_unselect = false;
   }

   m_tracking = true;
   m_startX = x;
   m_startY = y;

   m_x1 = 0.0;
   m_y1 = 0.0;

   parent->getRawParentXYValue( x, y, m_x1, m_y1 );
   m_mat = parent->getParentViewMatrix();

   if ( ! m_unselect && ! (buttonState & BS_Shift) )
   {
      parent->getModel()->unselectAll();
   }

   parent->updateAllViews();
   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Starting selection" ).toUtf8() );
}

void SelectFaceTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   if ( m_unselect )
   {
      if ( buttonState & BS_Left )
      {
         // We're waiting for the right button
         return;
      }
   }
   else
   {
      if ( buttonState & BS_Right )
      {
         // We're waiting for the left button
         return;
      }
   }

   if ( m_tracking )
   {
      m_tracking = false;

      double x1 = m_x1;
      double y1 = m_y1;
      double x2 = 0.0;
      double y2 = 0.0;

      Model * model = parent->getModel();
      Model::SelectionTest * test = NULL;
      if ( !m_includeBackfacing )
      {
         test = new NormalTest( model, m_mat );
      }

      parent->getRawParentXYValue( x, y, x2, y2 );
      if ( m_unselect )
      {
         model->unselectInVolumeMatrix( m_mat, x1, y1, x2, y2, test );
      }
      else
      {
         model->selectInVolumeMatrix( m_mat, x1, y1, x2, y2, test );
      }

      DecalManager::getInstance()->removeDecal( m_boundingBox );
      m_boundingBox = NULL;

      parent->updateAllViews();
      model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Selection complete" ).toUtf8() );
   }
}

void SelectFaceTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   if ( m_tracking )
   {
      double x1 = m_x1;
      double y1 = m_y1;
      double x2 = 0.0;
      double y2 = 0.0;

      parent->getRawParentXYValue( x, y, x2, y2 );
      m_boundingBox->setMatrixBounds( m_mat, x1, y1, x2, y2 );
      parent->updateView();
   }
}

const char ** SelectFaceTool::getPixmap()
{
   return (const char **) selectfacetool_xpm;
}

const char * SelectFaceTool::getPath()
{
   return TOOLS_SELECT_MENU;
}

const char * SelectFaceTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Select Faces" );
}

bool SelectFaceTool::getKeyBinding( int arg, int & keyBinding )
{
   keyBinding = Qt::Key_F;
   return true;
}

void SelectFaceTool::setBackfacingValue( bool newValue )
{
   m_includeBackfacing = newValue;
   log_debug( "includeBackfacing = %s\n", newValue ? "true" : "false" );
}


