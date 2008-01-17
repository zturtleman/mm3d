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

#include "contextpanel.h"

#include "viewpanel.h"
#include "contextname.h"
#include "contextposition.h"
#include "contextrotation.h"
#include "contextgroup.h"
#include "contextinfluences.h"
#include "contextprojection.h"

#include "log.h"

#include <QLayout>
#include <QSpacerItem>
#include <QContextMenuEvent>
#include <QCloseEvent>

ContextPanel::ContextPanel( QMainWindow * parent,
      ViewPanel * panel, ContextPanelObserver * ob )
   : QDockWidget( tr( "Properties", "Window title" ), parent ),
     m_model( NULL ),
     m_observer( ob ),
     m_panel( panel ),
     m_mainWidget( new QWidget( parent ) ),
     m_spacer( NULL )
{
   setWidget( m_mainWidget );
   m_layout = new QBoxLayout( QBoxLayout::TopToBottom, m_mainWidget );
}

ContextPanel::~ContextPanel()
{
}

void ContextPanel::setModel( Model * model )
{
   ContextWidgetList::iterator it;
   for ( it = m_widgets.begin(); it!= m_widgets.end(); it++ )
   {
      delete *it;
   }
   m_widgets.clear();

   if ( m_spacer )
   {
      delete m_spacer;
      m_spacer = NULL;
   }

   if ( m_model != model )
   {
      model->addObserver( this );
   }

   m_model = model;
   modelChanged( Model::ChangeAll );
}

void ContextPanel::modelChanged( int changeBits )
{
   if ( this->isVisible() )
   {
      log_debug( "modelChanged()\n" );

      setUpdatesEnabled( false );

      ContextWidgetList::iterator it;

      // If the panel caused the change, update all 
      // panels and return
      for ( it = m_widgets.begin(); it!= m_widgets.end(); it++ )
      {
         if ( (*it)->isUpdating() )
         {
            for ( it = m_widgets.begin(); it!= m_widgets.end(); it++ )
            {
               (*it)->modelChanged( changeBits );
            }

            setUpdatesEnabled( true );
            return;
         }
      }

      // If selection didn't change, just send an update
      if ( !(changeBits & (Model::SelectionChange | Model::AnimationMode)) )
      {
         for ( it = m_widgets.begin(); it!= m_widgets.end(); it++ )
         {
            (*it)->modelChanged( changeBits );
         }

         setUpdatesEnabled( true );
         return;
      }

      log_debug( "deleting and re-creating (%08X)\n", changeBits );

      // Otherwise, delete panels and recreate
      // TODO: may optimize by not deleting and re-creating, need to
      // be smarter about changes
      for ( it = m_widgets.begin(); it!= m_widgets.end(); it++ )
      {
         delete *it;
      }
      m_widgets.clear();

      {
         // FIXME only allow points in None and Frame
         // FIXME only allow joints (for keyframe) in Skel
         if ( (m_model->getSelectedBoneJointCount() 
                  + m_model->getSelectedPointCount()) == 1 )
         {
            ContextName * name = new ContextName( m_mainWidget );
            name->setModel( m_model );
            m_layout->addWidget( name );
            name->show();

            connect( name, SIGNAL(panelChange()), m_panel, SLOT(modelUpdatedEvent()));

            m_widgets.push_back( name );
         }
      }

      // Position should always be visible
      ContextPosition * pos = new ContextPosition( m_mainWidget );
      pos->setModel( m_model );
      m_layout->addWidget( pos );
      pos->show();

      connect( pos, SIGNAL(panelChange()), m_panel, SLOT(modelUpdatedEvent()));

      m_widgets.push_back( pos );

      {
         // FIXME only allow points in None and Frame
         // FIXME only allow joints (for keyframe) in Skel
         if ( (m_model->getSelectedBoneJointCount() 
                  + m_model->getSelectedPointCount()) == 1 )
         {
            ContextRotation * rot = new ContextRotation( m_mainWidget );
            rot->setModel( m_model );
            m_layout->addWidget( rot );
            rot->show();

            connect( rot, SIGNAL(panelChange()), m_panel, SLOT(modelUpdatedEvent()));

            m_widgets.push_back( rot );
         }
      }

      unsigned int tcount = m_model->getTriangleCount();
      for ( unsigned int t = 0; t < tcount; t++ )
      {
         if ( m_model->isTriangleSelected( t ) )
         {
            ContextGroup * grp = new ContextGroup( m_mainWidget, m_observer );
            grp->setModel( m_model );
            m_layout->addWidget( grp );
            grp->show();

            connect( grp, SIGNAL(panelChange()), m_panel, SLOT(modelUpdatedEvent()));

            m_widgets.push_back( grp );
            break;
         }
      }

      unsigned int pcount = m_model->getProjectionCount();
      for ( unsigned int p = 0; p < pcount; p++ )
      {
         if ( m_model->isProjectionSelected( p ) )
         {
            ContextProjection * prj = new ContextProjection( m_mainWidget, m_observer );
            prj->setModel( m_model );
            m_layout->addWidget( prj );
            prj->show();

            connect( prj, SIGNAL(panelChange()), m_panel, SLOT(modelUpdatedEvent()));

            m_widgets.push_back( prj );
            break;
         }
      }

      // Only show influences if there are influences to show
      if ( m_model->getBoneJointCount() > 0 && m_model->getAnimationMode() == Model::ANIMMODE_NONE )
      {
         bool showInfluences = false;

         unsigned pointCount = m_model->getPointCount();
         for ( unsigned n = 0; !showInfluences && n < pointCount; n++ )
         {
            if ( m_model->isPointSelected( n ) )
            {
               showInfluences = true;
            }
         }

         unsigned vcount = m_model->getVertexCount();
         for ( unsigned v = 0; !showInfluences && v < vcount; v++ )
         {
            if ( m_model->isVertexSelected( v ) )
            {
               showInfluences = true;
            }
         }

         if ( showInfluences )
         {
            ContextInfluences * prj = new ContextInfluences( m_mainWidget );
            prj->setModel( m_model );
            m_layout->addWidget( prj );
            prj->show();

            connect( prj, SIGNAL(panelChange()), m_panel, SLOT(modelUpdatedEvent()));

            m_widgets.push_back( prj );
         }
      }
      m_spacer = new QSpacerItem( 0, 0, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );
      setUpdatesEnabled( true );
   }
}

void ContextPanel::show()
{
   QDockWidget::show();

   // this is causing a segfault on dock/undock because the widgets
   // are being destroyed and re-created. The solution is to call setModel
   // explicitly whenever the window is shown (this only happens in viewwin.cc)
   //setModel( m_model ) 
}

void ContextPanel::close()
{
   hide(); // Do hide instead
}

void ContextPanel::hide()
{
   log_debug( "ContextPanel::hide()\n" );
   QDockWidget::hide();
}

void ContextPanel::contextMenuEvent( QContextMenuEvent * e )
{
   e->ignore();
}

void ContextPanel::closeEvent( QCloseEvent * e )
{
   emit panelHidden();
}

