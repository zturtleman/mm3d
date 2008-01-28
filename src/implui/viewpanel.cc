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


#include "viewpanel.h"

#include "mview.h"
#include "toolbox.h"
#include "model.h"
#include "log.h"
#include "msg.h"
#include "modelviewport.h"
#include "3dmprefs.h"

#include <QtGui/QLayout>
#include <QtGui/QGridLayout>

ViewPanel::ViewPanel( Toolbox * toolbox, QWidget * parent )
   : QWidget( parent ),
     m_model( NULL ),
     m_viewCount( 4 ),
     m_tall( false ),
     m_toolbox( toolbox )
{
   if ( g_prefs.exists( "ui_viewport_count" ) )
   {
      m_viewCount = g_prefs( "ui_viewport_count" ).intValue();
      if ( m_viewCount < 1 || m_viewCount > 9 )
      {
         m_viewCount = 4;
      }
   }
   if ( g_prefs.exists( "ui_viewport_tall" ) )
   {
      int tall = g_prefs( "ui_viewport_tall" ).intValue();
      m_tall = ( tall != 0 );
   }

   makeViews();

   for ( unsigned s = 0; s < VIEWPORT_STATE_MAX; s++ )
   {
      m_viewState[s].rotation[0] = -1000.0;
   }

   show();
}

ViewPanel::~ViewPanel()
{
   log_debug( "deleting view panel\n" );
}

void ViewPanel::freeTextures()
{
   for ( unsigned t = 0; t < m_viewCount; t++ )
   {
      m_modelView[t]->freeTextures();
   }
}

void ViewPanel::setModel( Model * model )
{
   for ( unsigned t = 0; t < m_viewCount; t++ )
   {
      m_modelView[t]->setModel( model );
   }
   m_model = model;
}

void ViewPanel::modelUpdatedEvent()
{
   for ( unsigned t = 0; t < m_viewCount; t++ )
   {
      m_modelView[t]->updateView();
   }
}

void ViewPanel::frameArea( double x1, double y1, double z1, double x2, double y2, double z2 )
{
   for ( unsigned t = 0; t < m_viewCount; t++ )
   {
      m_modelView[t]->frameArea( x1, y1, z1, x2, y2, z2 );
   }
}

void ViewPanel::wireframeEvent()
{
   for ( unsigned i = 0; i < m_viewCount; i++ )
   {
      m_modelView[i]->wireframeEvent();
   }
}

void ViewPanel::flatEvent()
{
   for ( unsigned i = 0; i < m_viewCount; i++ )
   {
      m_modelView[i]->flatEvent();
   }
}

void ViewPanel::smoothEvent()
{
   for ( unsigned i = 0; i < m_viewCount; i++ )
   {
      m_modelView[i]->smoothEvent();
   }
}

void ViewPanel::textureEvent()
{
   for ( unsigned i = 0; i < m_viewCount; i++ )
   {
      m_modelView[i]->textureEvent();
   }
}

void ViewPanel::alphaEvent()
{
   for ( unsigned i = 0; i < m_viewCount; i++ )
   {
      m_modelView[i]->alphaEvent();
   }
}

void ViewPanel::canvasWireframeEvent()
{
   m_model->setCanvasDrawMode( ModelViewport::ViewWireframe );
   modelUpdatedEvent();
}

void ViewPanel::canvasFlatEvent()
{
   m_model->setCanvasDrawMode( ModelViewport::ViewFlat );
   modelUpdatedEvent();
}

void ViewPanel::canvasSmoothEvent()
{
   m_model->setCanvasDrawMode( ModelViewport::ViewSmooth );
   modelUpdatedEvent();
}

void ViewPanel::canvasTextureEvent()
{
   m_model->setCanvasDrawMode( ModelViewport::ViewTexture );
   modelUpdatedEvent();
}

void ViewPanel::canvasAlphaEvent()
{
   m_model->setCanvasDrawMode( ModelViewport::ViewAlpha );
   modelUpdatedEvent();
}

void ViewPanel::makeViews()
{
   g_prefs( "ui_viewport_count" ) = (int) m_viewCount;
   g_prefs( "ui_viewport_tall" ) = (int) m_tall ? 1 : 0;

   int width  = 3;
   int height = 3;

   m_gridLayout = new QGridLayout( this );
   m_gridLayout->setSpacing(0);
   m_gridLayout->setMargin(0);

   switch ( m_viewCount )
   {
      case 1:
         width  = 1;
         height = 1;
         break;
      case 2:
         if ( m_tall )
         {
            width  = 1;
            height = 2;
         }
         else
         {
            width  = 2;
            height = 1;
         }
         break;
      case 4:
         width  = 2;
         height = 2;
         break;
      case 6:
         if ( m_tall )
         {
            width  = 2;
            height = 3;
         }
         else
         {
            width  = 3;
            height = 2;
         }
         break;
      default:
         width  = 3;
         height = 3;
         break;
   }

   for ( unsigned t = 0; t < m_viewCount; t++ )
   {
      m_modelView[t] = new ModelView( m_toolbox, this );
      connect( m_modelView[t]->getModelViewport(), SIGNAL(viewportSaveState(int, const ModelViewport::ViewStateT & )), 
            this, SLOT(viewportSaveStateEvent(int, const ModelViewport::ViewStateT &)) );
      connect( m_modelView[t]->getModelViewport(), SIGNAL(viewportRecallState(int)), 
            this, SLOT(viewportRecallStateEvent(int)) );
      m_gridLayout->addWidget( m_modelView[t], t / width, t % width );
   }

   setModel( m_model );
   setDefaultViewDirections();

   if ( m_model )
   {
      double x1, y1, z1, x2, y2, z2;
      if ( m_model->getBoundingRegion( &x1, &y1, &z1, &x2, &y2, &z2 ) )
      {
         frameArea( x1, y1, z1, x2, y2, z2 );
      }
   }
}

void ViewPanel::deleteViews()
{
   for ( unsigned t = 0; t < m_viewCount; t++ )
   {
      delete m_modelView[t];
   }
   delete m_gridLayout;
}

void ViewPanel::setDefaultViewDirections()
{
   switch ( m_viewCount )
   {
      case 1:
         m_modelView[0]->setViewDirection( ModelViewport::ViewPerspective );
         break;
      case 2:
         m_modelView[0]->setViewDirection( ModelViewport::ViewOrtho );
         m_modelView[1]->setViewDirection( ModelViewport::ViewPerspective );
         break;
      case 4:
         m_modelView[0]->setViewDirection( ModelViewport::ViewFront );
         m_modelView[1]->setViewDirection( ModelViewport::ViewLeft );
         m_modelView[2]->setViewDirection( ModelViewport::ViewTop );
         m_modelView[3]->setViewDirection( ModelViewport::ViewPerspective );
         break;
      case 6:
         if ( m_tall )
         {
            m_modelView[0]->setViewDirection( ModelViewport::ViewFront );
            m_modelView[1]->setViewDirection( ModelViewport::ViewBack );
            m_modelView[2]->setViewDirection( ModelViewport::ViewLeft );
            m_modelView[3]->setViewDirection( ModelViewport::ViewRight );
            m_modelView[4]->setViewDirection( ModelViewport::ViewTop );
            m_modelView[5]->setViewDirection( ModelViewport::ViewPerspective );
         }
         else
         {
            m_modelView[0]->setViewDirection( ModelViewport::ViewFront );
            m_modelView[1]->setViewDirection( ModelViewport::ViewBack );
            m_modelView[2]->setViewDirection( ModelViewport::ViewTop );
            m_modelView[3]->setViewDirection( ModelViewport::ViewLeft );
            m_modelView[4]->setViewDirection( ModelViewport::ViewRight );
            m_modelView[5]->setViewDirection( ModelViewport::ViewPerspective );
         }
         break;
      default:
         m_modelView[0]->setViewDirection( ModelViewport::ViewFront );
         m_modelView[1]->setViewDirection( ModelViewport::ViewBack );
         m_modelView[4]->setViewDirection( ModelViewport::ViewRight );
         m_modelView[2]->setViewDirection( ModelViewport::ViewTop );
         m_modelView[5]->setViewDirection( ModelViewport::ViewBottom );
         m_modelView[3]->setViewDirection( ModelViewport::ViewLeft );
         m_modelView[6]->setViewDirection( ModelViewport::ViewOrtho );
         m_modelView[7]->setViewDirection( ModelViewport::ViewOrtho );
         m_modelView[8]->setViewDirection( ModelViewport::ViewPerspective );
         break;
   }
}

void ViewPanel::view1()
{
   hide();
   deleteViews();
   m_viewCount = 1;
   makeViews();
   show();
}

void ViewPanel::view1x2()
{
   hide();
   deleteViews();
   m_viewCount = 2;
   m_tall = true;
   makeViews();
   show();
}

void ViewPanel::view2x1()
{
   hide();
   deleteViews();
   m_viewCount = 2;
   m_tall = false;
   makeViews();
   show();
}

void ViewPanel::view2x2()
{
   hide();
   deleteViews();
   m_viewCount = 4;
   makeViews();
   show();
}

void ViewPanel::view2x3()
{
   hide();
   deleteViews();
   m_viewCount = 6;
   m_tall = true;
   makeViews();
   show();
}

void ViewPanel::view3x2()
{
   hide();
   deleteViews();
   m_viewCount = 6;
   m_tall = false;
   makeViews();
   show();
}

void ViewPanel::view3x3()
{
   hide();
   deleteViews();
   m_viewCount = 9;
   makeViews();
   show();
}

void ViewPanel::viewportSaveStateEvent( int slot, const ModelViewport::ViewStateT & viewState )
{
   if ( slot >= 0 && slot < VIEWPORT_STATE_MAX )
   {
      m_viewState[slot] = viewState;
   }
}

void ViewPanel::viewportRecallStateEvent( int slot )
{
   if ( slot >= 0 && slot < VIEWPORT_STATE_MAX && m_viewState[slot].rotation[0] > -900.0 )
   {
      for ( unsigned v = 0; v < m_viewCount; v++ )
      {
         ModelViewport * m = m_modelView[v]->getModelViewport();
         if ( m->hasFocus() )
         {
            m_modelView[v]->setViewDirection( m_viewState[slot].direction );
            m->setViewState( m_viewState[slot] );
         }
      }
   }
}

