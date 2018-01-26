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
#include "selectconnectedtool.h"
#include "bounding.h"
#include "decalmgr.h"
#include "log.h"
#include "modelstatus.h"

#include "pixmap/selectconnectedtool.xpm"

#include <stdio.h>
#include <QtCore/QObject>
#include <QtWidgets/QApplication>
#include <QtGui/QKeySequence>

SelectConnectedTool::SelectConnectedTool()
   : m_boundingBox( NULL),
     m_tracking( false ),
     m_unselect( false ),
     m_startX( 0 ),
     m_startY( 0 ),
     m_x1( 0.0 ),
     m_y1( 0.0 ),
     m_selectionMode( Model::SelectConnected )
{
}

SelectConnectedTool::~SelectConnectedTool()
{
}

void SelectConnectedTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   if ( m_tracking )
   {
      return;
   }

   parent->getModel()->setSelectionMode( Model::SelectConnected );

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

void SelectConnectedTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
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
      parent->getRawParentXYValue( x, y, x2, y2 );
      if ( m_unselect )
      {
         model->unselectInVolumeMatrix( m_mat, x1, y1, x2, y2 );
      }
      else
      {
         model->selectInVolumeMatrix( m_mat, x1, y1, x2, y2 );
      }

      DecalManager::getInstance()->removeDecal( m_boundingBox );
      m_boundingBox = NULL;

      parent->updateAllViews();
      model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Selection complete" ).toUtf8() );
   }
}

void SelectConnectedTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
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

const char ** SelectConnectedTool::getPixmap()
{
   return (const char **) selectconnectedtool_xpm;
}

const char * SelectConnectedTool::getPath()
{
   return TOOLS_SELECT_MENU;
}

const char * SelectConnectedTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Select Connected Mesh" );
}

bool SelectConnectedTool::getKeyBinding( int arg, int & keyBinding )
{
   keyBinding = Qt::Key_C;
   return true;
}

