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


#include "projectionwin.h"
#include "decalmgr.h"
#include "helpwin.h"
#include "textureframe.h"
#include "texwidget.h"
#include "decalmgr.h"

#include <QtGui/QInputDialog>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QShortcut>

#include <list>

using std::list;

#include "model.h"
#include "decalmgr.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"

ProjectionWin::ProjectionWin( Model * model, QWidget * parent, ViewPanel * viewPanel )
   : QDialog( parent ),
     m_viewPanel( viewPanel ),
     m_undoCount( 0 ),
     m_redoCount( 0 ),
     m_inUndo( false ),
     m_ignoreChange( false )
{
   setupUi( this );

   m_material->hide();
   m_materialLabel->hide();

   m_textureWidget = m_textureFrame->getTextureWidget();
   m_textureWidget->setInteractive( true );
   m_textureWidget->setMouseOperation( TextureWidget::MouseRange );
   m_textureWidget->setDrawVertices( false );
   m_textureWidget->setMouseTracking( true );
   connect( m_textureWidget, SIGNAL(updateRangeSignal()), this, SLOT(rangeChangedEvent()) );
   connect( m_textureWidget, SIGNAL(updateRangeDoneSignal()), this, SLOT(applyProjectionEvent()) );
   connect( m_textureWidget, SIGNAL(updateSeamSignal(double,double)), this, SLOT(seamChangedEvent(double,double)) );
   connect( m_textureWidget, SIGNAL(updateSeamDoneSignal()), this, SLOT(applyProjectionEvent()) );
   connect( m_textureWidget, SIGNAL(zoomLevelChanged(QString)), this, SLOT(zoomLevelChangedEvent(QString)) );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   QShortcut * undo = new QShortcut( QKeySequence( tr("CTRL+Z", "Undo shortcut")), this );
   connect( undo, SIGNAL(activated()), this, SLOT(undoEvent()) );
   QShortcut * redo = new QShortcut( QKeySequence( tr("CTRL+Y", "Redo shortcut")), this );
   connect( redo, SIGNAL(activated()), this, SLOT(redoEvent()) );

   // can't do this until constructor is done (because of Model::Observer interface)
   //setModel( model );
}

ProjectionWin::~ProjectionWin()
{
}

void ProjectionWin::refreshProjectionDisplay()
{
   m_textureWidget->clearCoordinates();

   if ( m_projection->count() > 0 )
   {
      int proj = m_projection->currentIndex();
      unsigned tcount = m_model->getTriangleCount();
      for ( unsigned t = 0; t < tcount; t++ )
      {
         int p = m_model->getTriangleProjection( t );
         if ( p == proj )
         {
            int g = m_model->getTriangleGroup( t );
            if ( g >= 0 )
            {
               int material = m_model->getGroupTextureId( g );
               if ( material >= 0 )
               {
                  m_textureFrame->textureChangedEvent( material + 1 ); // TODO bah, off-by-one, I need to fix this

                  addProjectionTriangles();
                  return;
               }
            }
         }
      }
   }
   m_textureWidget->updateGL();
   m_textureFrame->textureChangedEvent( -1 );
   DecalManager::getInstance()->modelUpdated( m_model );
}

void ProjectionWin::modelChanged( int changeBits )
{
   if ( !m_ignoreChange )
   {
      if ( !m_inUndo )
      {
         m_undoCount = 0;
         m_redoCount = 0;
      }

      // TODO need some way to re-select the projection we were looking at
      if ( isVisible() )
      {
         int projCount = m_model->getProjectionCount();
         if ( projCount != m_projection->count() )
         {
            // A projection was added or deleted, we need to select a new
            // projection and re-initialize everything. 
            initWindow();
         }
         else
         {
            // a change to projection itself, or a non-projection change, just 
            // re-initialize the projection display for the current projection
            int p = m_projection->currentIndex();
            m_projection->setItemText( p, QString::fromUtf8( m_model->getProjectionName( p )) );
            m_type->setCurrentIndex( m_model->getProjectionType( p ) );
            // TODO material/test pattern?
            addProjectionTriangles();
         }
      }
   }
}

void ProjectionWin::addProjectionTriangles()
{
   m_textureWidget->clearCoordinates();

   int p = getSelectedProjection();

   double uv[4] = { 0, 0, 0, 0 };
   m_model->getProjectionRange( p,
         uv[0], uv[1], uv[2], uv[3] );
   m_textureWidget->setRange( 
         uv[0], uv[1], uv[2], uv[3] );

   unsigned tcount = m_model->getTriangleCount();
   for ( unsigned t = 0; t < tcount; t++ )
   {
      if ( m_model->getTriangleProjection( t ) == p )
      {
         int verts[3] = { 0, 0, 0 };
         for ( int i = 0; i < 3; i++ )
         {
            float u = 1.0;
            float v = 1.0;
            m_model->getTextureCoords( t, i, u, v );
            verts[i] = m_textureWidget->addVertex( u, v );
         }

         m_textureWidget->addTriangle( 
               verts[0], verts[1], verts[2] );
      }
   }
   m_textureWidget->updateGL();
   DecalManager::getInstance()->modelUpdated( m_model );
}

void ProjectionWin::setModel( Model * model )
{
   m_undoCount = 0;
   m_redoCount = 0;

   if ( model != m_model )
   {
      model->addObserver( this );
   }

   m_model = model;
   m_textureFrame->setModel( model );

   if ( isVisible() )
   {
      initWindow();
   }
}

void ProjectionWin::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_projectionwin.html", true );
   win->show();
}

void ProjectionWin::undoEvent()
{
   if ( m_undoCount > 0 )
   {
      m_inUndo = true;
      m_model->undo();
      m_undoCount--;
      m_inUndo = false;
   }
}

void ProjectionWin::redoEvent()
{
   if ( m_undoCount < m_redoCount )
   {
      m_inUndo = true;
      m_model->redo();
      m_undoCount++;
      m_inUndo = false;
   }
}

void ProjectionWin::show()
{
   setModel( m_model );
   
   if ( !isVisible() )
   {
      // If we are visible, setModel already did this
      initWindow();
   }
   QDialog::show();
}

void ProjectionWin::initWindow()
{
   m_projection->clear();

   if ( m_model )
   {
      unsigned pcount = m_model->getProjectionCount();
      bool enabled = true;
      if ( pcount == 0 )
      {
         enabled = false;
      }
      m_type->setEnabled( enabled );
      m_material->setEnabled( enabled );
      m_textureFrame->setEnabled( enabled );
      m_addFacesButton->setEnabled( enabled );
      m_removeFacesButton->setEnabled( enabled );
      m_renameButton->setEnabled( enabled );
      m_zoomInput->setEnabled( enabled );
      m_zoomInButton->setEnabled( enabled );
      m_zoomOutButton->setEnabled( enabled );
      m_applyButton->setEnabled( enabled );
      m_resetButton->setEnabled( enabled );

      if ( enabled )
      {
         for ( unsigned p = 0; p < pcount; p++ )
         {
            m_projection->insertItem( p, QString::fromUtf8( m_model->getProjectionName(p) ) );
         }

         bool found = false;

         for ( unsigned p = 0; p < pcount; p++ )
         {
            if ( m_model->isProjectionSelected( p ) )
            {
               found = true;
               m_projection->setCurrentIndex( p );
               projectionIndexChangedEvent( p );
               break;
            }
         }

         if ( !found )
         {
            unsigned tcount = m_model->getTriangleCount();
            for ( unsigned t = 0; t < tcount; t++ )
            {
               if ( m_model->isTriangleSelected( t ) )
               {
                  int p = m_model->getTriangleProjection( t );
                  if ( p >= 0 )
                  {
                     m_projection->setCurrentIndex( p );
                     projectionIndexChangedEvent( p );
                     break;
                  }
               }
            }
         }
      }
   }
}

void ProjectionWin::closeEvent( QCloseEvent * e )
{
   e->ignore();
   hide();
}

void ProjectionWin::zoomIn()
{
   m_textureWidget->zoomIn();
}

void ProjectionWin::zoomOut()
{
   m_textureWidget->zoomOut();
}

void ProjectionWin::typeChangedEvent( int type )
{
   int p = getSelectedProjection();

   if ( p >= 0 )
   {
      m_model->setProjectionType( p, type );
      //m_model->applyProjection( p );
      operationComplete( tr( "Set Projection Type", "operation complete" ).toUtf8() );
   }

   refreshProjectionDisplay();
}

void ProjectionWin::addFacesEvent()
{
   int p = getSelectedProjection();

   unsigned tcount = m_model->getTriangleCount();
   for ( unsigned t = 0; t < tcount; t++ )
   {
      if ( m_model->isTriangleSelected( t ) )
      {
         m_model->setTriangleProjection( t, p );
      }
   }
   m_model->applyProjection( p );
   operationComplete( tr( "Set Triangle Projection", "operation complete" ).toUtf8() );
   refreshProjectionDisplay();
}

void ProjectionWin::removeFacesEvent()
{
   unsigned tcount = m_model->getTriangleCount();
   for ( unsigned t = 0; t < tcount; t++ )
   {
      if ( m_model->isTriangleSelected( t ) )
      {
         m_model->setTriangleProjection( t, -1 );
      }
   }
   operationComplete( tr( "Set Triangle Projection", "operation complete" ).toUtf8() );
   refreshProjectionDisplay();
}

void ProjectionWin::applyProjection()
{
   int p = getSelectedProjection();

   if ( p >= 0 )
   {
      m_model->applyProjection( p );
   }
   addProjectionTriangles();
}

void ProjectionWin::applyProjectionEvent()
{
   applyProjection();
   operationComplete( tr("Apply Projection", "operation complete").toUtf8() );
}

void ProjectionWin::resetClickedEvent()
{
   int p = getSelectedProjection();

   m_model->setProjectionRange( p, 0.0, 0.0, 1.0, 1.0 );
   operationComplete( tr( "Reset UV Coordinates", "operation complete" ).toUtf8() );
   addProjectionTriangles();
   //applyProjectionEvent();
}

void ProjectionWin::renameClickedEvent()
{
   int p = getSelectedProjection();

   if ( p >= 0 )
   {
      bool ok = false;
      QString projName = QInputDialog::getText( this, tr("Rename projection", "window title"), tr("Enter new point name:"), QLineEdit::Normal, QString::fromUtf8( m_model->getProjectionName( p )), &ok );
      if ( ok )
      {
         m_model->setProjectionName( p, projName.toUtf8() );
         m_projection->setItemText( p, projName );
         operationComplete( tr( "Rename Projection", "operation complete" ).toUtf8() );
      }
   }
}

void ProjectionWin::projectionIndexChangedEvent( int newIndex )
{
   int type = m_model->getProjectionType( newIndex );
   m_type->setCurrentIndex( type );

   double uv[4] = { 0, 0, 0, 0 };
   m_model->getProjectionRange( newIndex, 
         uv[0], uv[1], uv[2], uv[3] );

   m_textureWidget->setRange( uv[0], uv[1], uv[2], uv[3] );

   refreshProjectionDisplay();
}

void ProjectionWin::zoomChangeEvent()
{
   double zoom = atof( m_zoomInput->text().toLatin1() );
   if ( zoom < 0.00001 )
   {
      zoom = 1;
   }
   m_textureWidget->setZoomLevel( zoom );
}

void ProjectionWin::zoomLevelChangedEvent( QString zoomStr )
{
   m_zoomInput->setText( zoomStr );
}

void ProjectionWin::rangeChangedEvent()
{
   int p = getSelectedProjection();

   double uv[4] = { 0, 0, 0, 0 };
   m_textureWidget->getRange( uv[0], uv[1], uv[2], uv[3] );
   m_model->setProjectionRange( p, uv[0], uv[1], uv[2], uv[3] );

   addProjectionTriangles();
}

void ProjectionWin::seamChangedEvent( double xDiff, double yDiff )
{
   if ( fabs( xDiff ) > 0.0 )
   {
      int p = getSelectedProjection();

      double up[4] = { 0, 0, 0, 1 };
      double seam[4] = { 0, 0, 0, 1 };

      m_model->getProjectionUp( p, up );
      m_model->getProjectionSeam( p, seam );

      Matrix m;
      m.setRotationOnAxis( up, xDiff );
      m.apply3( seam );

      m_model->setProjectionSeam( p, seam );

      addProjectionTriangles();
   }
}

int ProjectionWin::getSelectedProjection()
{
   if ( m_projection->count() > 0 )
   {
      return m_projection->currentIndex();
   }
   return -1;
}

void ProjectionWin::operationComplete( const char * opname )
{
   m_undoCount++;
   m_redoCount = m_undoCount;

   m_ignoreChange = true;
   m_model->operationComplete( opname );
   m_ignoreChange = false;
}

