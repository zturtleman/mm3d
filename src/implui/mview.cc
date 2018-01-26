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


#include "mview.h"
#include "modelviewport.h"
#include "viewpanel.h"
#include "log.h"
#include "decalmgr.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtGui/QImage>

ModelView::ModelView( Toolbox * toolbox, QWidget * parent )
   : QWidget( parent ),
     m_toolbox( toolbox )
{
   setupUi( this );

   QString zoomStr;
   zoomStr.sprintf( "%f", m_modelView->getZoomLevel() );
   m_zoomInput->setText( zoomStr );

   connect( m_modelView, SIGNAL(viewDirectionChanged(int)), this, SLOT(setViewDirection(int)));

   ViewPanel * panel = dynamic_cast<ViewPanel *>(parent);
   if ( panel )
   {
      connect( m_modelView, SIGNAL(modelUpdated()), panel, SLOT(modelUpdatedEvent()));
   }
   else
   {
      log_error( "cast failed, not connecting\n" );
   }
}

ModelView::~ModelView()
{
   DecalManager::getInstance()->unregisterToolParent( m_modelView );
}

void ModelView::freeTextures()
{
   m_modelView->freeTextures();
}

void ModelView::setViewDirection( int dir )
{
   m_viewInput->setCurrentIndex( dir );
   m_modelView->viewChangeEvent( dir );
}

void ModelView::zoomLevelEnterEvent()
{
   m_modelView->setZoomLevel( m_zoomInput->text().toDouble() );
}

void ModelView::zoomInEvent()
{
   m_modelView->zoomIn();
}

void ModelView::zoomOutEvent()
{
   m_modelView->zoomOut();
}

void ModelView::setModel( Model * model )
{
   if ( model == NULL  )
   {
      DecalManager::getInstance()->unregisterToolParent( m_modelView );
   }

   m_modelView->setModel( model );
   m_modelView->setToolbox( m_toolbox );

   if ( model )
   {
      DecalManager::getInstance()->registerToolParent( m_modelView );
   }
}

void ModelView::updateView()
{
   m_modelView->updateView();
}

unsigned ModelView::getViewDirection()
{
   return m_viewInput->currentIndex();
}

QString ModelView::getViewDirectionLabel()
{
   return m_viewInput->currentText();
}

void ModelView::copyContentsToTexture( Texture * tex )
{
   m_modelView->copyContentsToTexture( tex );
}

void ModelView::updateCaptureGL()
{
   m_modelView->updateCaptureGL();
}

QImage ModelView::grabFrameBuffer( bool withAlpha )
{
   return m_modelView->grabFrameBuffer( withAlpha );
}

