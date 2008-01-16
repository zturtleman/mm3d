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


#include "menuconf.h"
#include "projtool.h"

#include "projtoolwidget.h"

#include "model.h"
#include "msg.h"
#include "log.h"
#include "modelstatus.h"

#include "pixmap/projtool.xpm"

#include <qobject.h>
#include <qapplication.h>

ProjectionTool::ProjectionTool()
   : m_type( Model::TPT_Sphere )
{
   m_proj.pos.type = Model::PT_Vertex;
}

ProjectionTool::~ProjectionTool()
{
}


void ProjectionTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   m_allowX = true;
   m_allowY = true;
   m_x = x;
   m_y = y;

   Model * model = parent->getModel();

   model->setDrawProjections( true );

   // use parent view matrix to translate 2D coords into 3D coords
   double coord[4] = {0,0,0,1};

   parent->getParentXYValue( x, y, coord[0], coord[1], true );

   Matrix m = parent->getParentViewInverseMatrix();
   m.apply( coord );

   // Find a unique name for the projection
   char name[32] = "Projection 1";
   unsigned c = model->getProjectionCount();
   bool uniqueName = (c == 0) ? true : false;

   for ( unsigned i = 1; !uniqueName && i < 1000; i++ )
   {
      uniqueName = true;
      sprintf( name, "Projection %d", i );
      for ( unsigned j = 0; j < c; j++ )
      {
         if ( strcmp( name, model->getProjectionName( j ) ) == 0 )
         {
            uniqueName = false;
            break;
         }
      }
   }

   // I give up, just call it "Projection"
   if ( ! uniqueName )
   {
      strcpy( name, "Projection" );
   }

   m_orig[0] = coord[0];
   m_orig[1] = coord[1];
   m_orig[2] = coord[2];

   // Create projection
   m_proj.pos.type = Model::PT_Projection;
   m_proj.pos.index = model->addProjection( name,
         static_cast<Model::TextureProjectionTypeE>( m_type ),
         coord[0], coord[1], coord[2] );

   double upVec[3]   = { 0, 1, 0 };
   double seamVec[3] = { 0, 0, -1.0 / 3.0 };

   m.apply3( upVec );
   m.apply3( seamVec );

   model->setProjectionUp( m_proj.pos.index, upVec );
   model->setProjectionSeam( m_proj.pos.index, seamVec );

   model->setProjectionScale( m_proj.pos.index, 1.0 );

   // Assign selected faces to projection
   unsigned tcount = model->getTriangleCount();
   for ( unsigned t = 0; t < tcount; t++ )
   {
      if ( model->isTriangleSelected( t ) )
      {
         model->setTriangleProjection( t, m_proj.pos.index );
      }
   }
   model->applyProjection( m_proj.pos.index );

   // Make new projection the only thing selected
   model->unselectAll();
   model->selectProjection( m_proj.pos.index );

   parent->updateAllViews();

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Projection created" ).utf8() );
}

void ProjectionTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   if ( m_proj.pos.type == Model::PT_Projection )
   {
      if ( buttonState & BS_Shift )
      {
         if ( m_allowX && m_allowY )
         {
            double ax = fabs( x - m_x );
            double ay = fabs( y - m_y );

            if ( ax > ay )
            {
               m_allowY = false;
            }
            if ( ay > ax )
            {
               m_allowX = false;
            }
         }
      }

      if ( !m_allowX )
      {
         x = m_x;
      }
      if ( !m_allowY )
      {
         y = m_y;
      }

      double coord[3] = {0,0,0};

      // Convert 2D coords to 3D in parent's view space
      parent->getParentXYValue( x, y, coord[0], coord[1] );

      Matrix m = parent->getParentViewInverseMatrix();

      double tranVec[4] = { coord[0], coord[1], 0.0, 1.0 };

      m.apply( tranVec );

      tranVec[0] -= m_orig[0];
      tranVec[1] -= m_orig[1];
      tranVec[2] -= m_orig[2];

      // Set the up vector to whereever the mouse is
      Model * model = parent->getModel();
      model->setProjectionUp( m_proj.pos.index, tranVec );
      //model->applyProjection( m_proj.pos.index );

      parent->updateAllViews();
   }
}

void ProjectionTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
}

const char ** ProjectionTool::getPixmap()
{
   return (const char **) projtool_xpm;
}

const char * ProjectionTool::getPath()
{
   return TOOLS_CREATE_MENU;
}

const char * ProjectionTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Projection" );
}

void ProjectionTool::setTypeValue( int newValue )
{
   m_type = newValue;
}

void ProjectionTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   m_widget = new ProjToolWidget( this, mainwin );
#ifdef HAVE_QT4
   //mainwin->addDockWindow( m_widget, DockBottom );
#endif
   m_widget->show();
}

void ProjectionTool::deactivated()
{
   m_widget->close();
}

