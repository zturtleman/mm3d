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


#include "paintwidget.h"
#include "texwidget.h"

#include "log.h"

PaintWidget::PaintWidget( TextureWidget * drawBuddy, QWidget * parent )
   : QOpenGLWidget( parent ),
     m_drawBuddy( drawBuddy )
{
}

PaintWidget::~PaintWidget()
{
}

void PaintWidget::initializeGL()
{
   if ( !isValid() )
   {
      log_error( "paint widget does not have a valid OpenGL context\n" );
      return;
   }

   glEnable( GL_TEXTURE_2D );

   glShadeModel( GL_SMOOTH );
   glDepthFunc( GL_LEQUAL );
   glClearColor( 0.0, 0.0, 0.0, 1.0 );
   glClearDepth( 1.0f );

   GLfloat ambient[]  = {  0.8f,  0.8f,  0.8f,  1.0f };
   GLfloat diffuse[]  = {  1.0f,  1.0f,  1.0f,  1.0f };
   GLfloat position[] = {  0.0f,  0.0f,  3.0f,  0.0f };

   glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );
   glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
   glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
   glLightfv( GL_LIGHT0, GL_POSITION, position );

   glDisable( GL_LIGHT0 );
   glDisable( GL_LIGHTING );
}

void PaintWidget::resizeGL( int w, int h )
{
   glViewport( 0, 0, ( GLint ) w, ( GLint ) h );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity( );

   glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

   glOrtho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );
}

void PaintWidget::paintGL()
{
   // I don't draw on myself
   log_debug( "PaintWidget::paintGL\n" );
   m_drawBuddy->paintOnGlWidget( this );
}

