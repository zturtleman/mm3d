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


#include "texwidget.h"

#include "texture.h"
#include "model.h"
#include "log.h"
#include "mm3dport.h"
#include <math.h>

#include <QtCore/QTimer>
#include <QtGui/QCursor>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QKeyEvent>

#include "pixmap/arrow.xpm"
#include "pixmap/crosshairrow.xpm"


#define VP_ZOOMSCALE 0.75

static int const SCROLL_SIZE =  16;

struct _ScrollButton_t
{
   int x;
   int y;
   int texIndex;
   float s1;
   float t1;
   float s2;
   float t2;
   float s3;
   float t3;
   float s4;
   float t4;
};
typedef struct _ScrollButton_t ScrollButtonT;

static ScrollButtonT s_buttons[ TextureWidget::ScrollButtonMAX ] =
{
   { -18, -18, 1,   0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,  0.0f, 1.0f  }, // Pan
   { -52, -18, 0,   0.0f, 1.0f,   0.0f, 0.0f,   1.0f, 0.0f,  1.0f, 1.0f  }, // Left
   { -35, -18, 0,   0.0f, 0.0f,   0.0f, 1.0f,   1.0f, 1.0f,  1.0f, 0.0f  }, // Right
   { -18, -35, 0,   0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,  0.0f, 1.0f  }, // Up
   { -18, -52, 0,   0.0f, 1.0f,   1.0f, 1.0f,   1.0f, 0.0f,  0.0f, 0.0f  }, // Down
};

using std::vector;
using std::list;

TextureWidget::TextureWidget( QWidget * parent )
   : QOpenGLWidget( parent ),
     m_sClamp( false ),
     m_tClamp( false ),
     m_zoom( 1.0 ),
     m_xCenter( 0.5 ),
     m_yCenter( 0.5 ),
     m_model( NULL ),
     m_materialId( -1 ),
     m_texture( NULL ),
     m_glTexture( 0 ),
     m_scrollTimer( new QTimer() ),
     m_overlayButton( ScrollButtonMAX ),
     m_drawMode( DM_Edit ),
     m_drawVertices( true ),
     m_drawBorder( false ),
     m_solidBackground( false ),
     m_operation( MouseSelect ),
     m_scaleKeepAspect( false ),
     m_scaleFromCenter( false ),
     m_selecting( false ),
     m_drawBounding( false ),
     m_drawRange( false ),
     m_interactive( false ),
     m_3d( false ),
     m_button( 0 ),
     m_animTimer( new QTimer() ),
     m_xMin( 0.0 ),
     m_xMax( 1.0 ),
     m_yMin( 0.0 ),
     m_yMax( 1.0 ),
     m_xRotPoint( 0.5 ),
     m_yRotPoint( 0.5 ),
     m_linesColor( 0xffffff ),
     m_selectionColor( 0xff0000 )
{
   connect( m_animTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));
   setFocusPolicy( Qt::WheelFocus );

   connect( m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimeout()));
   setCursor( QCursor( Qt::ArrowCursor ) );
}

TextureWidget::~TextureWidget()
{
   m_animTimer->stop();
   delete m_animTimer;
   m_animTimer = NULL;

   if ( isValid() )
   {
      makeCurrent();
      glDeleteTextures( 1, (GLuint *) &m_glTexture );
      // Do NOT free m_texture.  TextureManager does that.

      glDeleteTextures( 2, m_scrollTextures );
   }

   m_scrollTimer->stop();
   delete m_scrollTimer;
}

void TextureWidget::initializeGL()
{
   if ( !isValid() )
   {
      log_error( "texture widget does not have a valid OpenGL context\n" );
      return;
   }

   // general set-up
   glEnable( GL_TEXTURE_2D );

   glShadeModel( GL_SMOOTH );
   glDepthFunc( GL_LEQUAL );
   glClearColor( 0.80, 0.80, 0.80, 1.0 );
   glClearDepth( 1.0f );

   // set up texture if setTexture() was called before initializeGL()
   updateGLTexture();

   // set up overlay arrows
   QPixmap arrow( arrow_xpm );
   QPixmap cross( crosshairrow_xpm );

   QImage img;

   glGenTextures( 2, m_scrollTextures );

   img = arrow.toImage();
   makeTextureFromImage( img, m_scrollTextures[0] );

   img = cross.toImage();
   makeTextureFromImage( img, m_scrollTextures[1] );

   // set up lighting
   GLfloat ambient[]  = {  0.8f,  0.8f,  0.8f,  1.0f };
   GLfloat diffuse[]  = {  1.0f,  1.0f,  1.0f,  1.0f };
   GLfloat position[] = {  0.0f,  0.0f,  3.0f,  0.0f };

   glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );
   glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
   glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
   glLightfv( GL_LIGHT0, GL_POSITION, position );

   glEnable( GL_LIGHT0 );
   glEnable( GL_LIGHTING );
}

void TextureWidget::resizeGL( int w, int h )
{
   if ( h == 0 )
      h = 1;

   m_viewportWidth  = w;
   m_viewportHeight = h;

   updateViewport();
}

void TextureWidget::updateViewport()
{
   m_xMin = m_xCenter - (m_zoom / 2.0);
   m_xMax = m_xCenter + (m_zoom / 2.0);
   m_yMin = m_yCenter - (m_zoom / 2.0);
   m_yMax = m_yCenter + (m_zoom / 2.0);

   update();
}

void TextureWidget::paintGL()
{
   makeCurrent();
   paintInternal();
}

void TextureWidget::paintOnGlWidget( QOpenGLWidget * w )
{
   w->makeCurrent();
   paintInternal();
}

void TextureWidget::paintInternal()
{
   setViewportDraw();

   //log_debug( "paintInternal()\n" );
   //log_debug( "(%f,%f)  %f\n", m_xCenter, m_yCenter, m_zoom );

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glLoadIdentity( );
   glEnable( GL_LIGHTING );

   if ( m_texture && !m_solidBackground )
   {
      glEnable( GL_TEXTURE_2D );
      glBindTexture( GL_TEXTURE_2D, m_glTexture );
   }
   else
   {
      glDisable( GL_TEXTURE_2D );
   }

   glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

   if ( m_solidBackground )
   {
      glColor3f( 0.0f, 0.0f, 0.0f );
      float fval[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

      glMaterialfv( GL_FRONT, GL_AMBIENT, fval );
      glMaterialfv( GL_FRONT, GL_DIFFUSE, fval );
      glMaterialfv( GL_FRONT, GL_SPECULAR, fval );
      glMaterialfv( GL_FRONT, GL_EMISSION, fval );
      glMaterialf( GL_FRONT, GL_SHININESS, fval[0] );
   }
   else if ( m_materialId >= 0 )
   {
      glColor3f( 1.0f, 1.0f, 1.0f );

      if ( m_model )
      {
         float fval[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

         m_model->getTextureAmbient( m_materialId, fval );
         glMaterialfv( GL_FRONT, GL_AMBIENT,
               fval );
         m_model->getTextureDiffuse( m_materialId, fval );
         glMaterialfv( GL_FRONT, GL_DIFFUSE,
               fval );
         m_model->getTextureSpecular( m_materialId, fval );
         glMaterialfv( GL_FRONT, GL_SPECULAR,
               fval );
         m_model->getTextureEmissive( m_materialId, fval );
         glMaterialfv( GL_FRONT, GL_EMISSION,
               fval );
         m_model->getTextureShininess( m_materialId, fval[0] );
         glMaterialf( GL_FRONT, GL_SHININESS,
               fval[0] );
      }
      else
      {
         float fval[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
         glMaterialfv( GL_FRONT, GL_AMBIENT,
               fval );

         fval[0] = fval[1] = fval[2] = 1.0f;
         glMaterialfv( GL_FRONT, GL_DIFFUSE,
               fval );

         fval[0] = fval[1] = fval[2] = 0.0f;
         glMaterialfv( GL_FRONT, GL_SPECULAR,
               fval );

         fval[0] = fval[1] = fval[2] = 0.0f;
         glMaterialfv( GL_FRONT, GL_EMISSION,
               fval );

         glMaterialf( GL_FRONT, GL_SHININESS,
               0.0f );
      }
   }
   else
   {
      float fval[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
      glMaterialfv( GL_FRONT, GL_AMBIENT,
            fval );

      fval[0] = fval[1] = fval[2] = 1.0f;
      glMaterialfv( GL_FRONT, GL_DIFFUSE,
            fval );

      fval[0] = fval[1] = fval[2] = 0.0f;
      glMaterialfv( GL_FRONT, GL_SPECULAR,
            fval );

      fval[0] = fval[1] = fval[2] = 0.0f;
      glMaterialfv( GL_FRONT, GL_EMISSION,
            fval );

      glMaterialf( GL_FRONT, GL_SHININESS,
            0.0f );

      if ( m_texture )
      {
         glColor3f( 1.0f, 1.0f, 1.0f );
      }
      else
      {
         glDisable( GL_LIGHTING );
         glColor3f( 0.0f, 0.0f, 0.0f );
      }
   }

   if ( m_materialId >= 0 && m_model && m_model->getMaterialType( m_materialId ) == Model::Material::MATTYPE_COLOR )
   {
      GLubyte r = m_model->getMaterialColor( m_materialId, 0 );
      GLubyte g = m_model->getMaterialColor( m_materialId, 1 );
      GLubyte b = m_model->getMaterialColor( m_materialId, 2 );
      glDisable( GL_TEXTURE_2D );
      //glDisable( GL_LIGHTING );
      glColor3ub( r, g, b );
   }

   if ( m_materialId >= 0 && m_3d )
   {
      glEnable( GL_DEPTH_TEST );

      PORT_timeval tv;

      PORT_gettimeofday( &tv );
      int ms = tv.tv_msec + (tv.tv_sec & 7) * 1000;

      float yRot = (float) ms / 4000.0 * 360.0;
      float xRot = (float) ms / 8000.0 * 360.0;
      glTranslatef( 0.0, 0.0, -5.0 );
      glRotatef( yRot, 0.0, 1.0, 0.0 );
      glRotatef( xRot, 1.0, 0.0, 0.0 );

      glBegin( GL_QUADS );

      // Front
      glTexCoord2f( 0.0, 0.0 );
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f( -1.0, -1.0, 1.0 );

      glTexCoord2f( 1.0, 0.0 );
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f(  1.0, -1.0, 1.0 );

      glTexCoord2f( 1.0, 1.0 );
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f(  1.0,  1.0, 1.0 );

      glTexCoord2f( 0.0, 1.0 );
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f( -1.0,  1.0, 1.0 );

      // Back
      glTexCoord2f( 0.0, 0.0 );
      glNormal3f( 0.0, 0.0, -1.0 );
      glVertex3f(  1.0, -1.0, -1.0 );

      glTexCoord2f( 1.0, 0.0 );
      glNormal3f( 0.0, 0.0, -1.0 );
      glVertex3f( -1.0, -1.0, -1.0 );

      glTexCoord2f( 1.0, 1.0 );
      glNormal3f( 0.0, 0.0, -1.0 );
      glVertex3f( -1.0,  1.0, -1.0 );

      glTexCoord2f( 0.0, 1.0 );
      glNormal3f( 0.0, 0.0, -1.0 );
      glVertex3f(  1.0,  1.0, -1.0 );

      // Left
      glTexCoord2f( 0.0, 0.0 );
      glNormal3f( 1.0, 0.0, 0.0 );
      glVertex3f(  1.0, -1.0, 1.0 );

      glTexCoord2f( 1.0, 0.0 );
      glNormal3f( 1.0, 0.0, 0.0 );
      glVertex3f( 1.0, -1.0, -1.0 );

      glTexCoord2f( 1.0, 1.0 );
      glNormal3f( 1.0, 0.0, 0.0 );
      glVertex3f( 1.0,  1.0, -1.0 );

      glTexCoord2f( 0.0, 1.0 );
      glNormal3f( 1.0, 0.0, 0.0 );
      glVertex3f(  1.0,  1.0, 1.0 );

      // Right
      glTexCoord2f( 0.0, 0.0 );
      glNormal3f( -1.0, 0.0, 0.0 );
      glVertex3f( -1.0, -1.0, -1.0 );

      glTexCoord2f( 1.0, 0.0 );
      glNormal3f( -1.0, 0.0, 0.0 );
      glVertex3f( -1.0, -1.0, 1.0 );

      glTexCoord2f( 1.0, 1.0 );
      glNormal3f( -1.0, 0.0, 0.0 );
      glVertex3f( -1.0,  1.0, 1.0 );

      glTexCoord2f( 0.0, 1.0 );
      glNormal3f( -1.0, 0.0, 0.0 );
      glVertex3f( -1.0,  1.0, -1.0 );

      // Top
      glTexCoord2f( 0.0, 0.0 );
      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f( -1.0, 1.0, -1.0 );

      glTexCoord2f( 1.0, 0.0 );
      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f(  1.0, 1.0, -1.0 );

      glTexCoord2f( 1.0, 1.0 );
      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f(  1.0,  1.0, 1.0 );

      glTexCoord2f( 0.0, 1.0 );
      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f( -1.0,  1.0, 1.0 );

      // Bottom
      glTexCoord2f( 0.0, 0.0 );
      glNormal3f( 0.0, -1.0, 0.0 );
      glVertex3f( 1.0, -1.0, -1.0 );

      glTexCoord2f( 1.0, 0.0 );
      glNormal3f( 0.0, -1.0, 0.0 );
      glVertex3f( -1.0, -1.0, -1.0 );

      glTexCoord2f( 1.0, 1.0 );
      glNormal3f( 0.0, -1.0, 0.0 );
      glVertex3f( -1.0, -1.0, 1.0 );

      glTexCoord2f( 0.0, 1.0 );
      glNormal3f( 0.0, -1.0, 0.0 );
      glVertex3f( 1.0, -1.0, 1.0 );

      glEnd();
      glDisable( GL_DEPTH_TEST );
   }
   else
   {
      glBegin( GL_QUADS );

      glTexCoord2f( m_xMin, m_yMin );
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f( m_xMin, m_yMin, 0.0 );

      glTexCoord2f( m_xMax, m_yMin );
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f(  m_xMax, m_yMin, 0.0 );

      glTexCoord2f( m_xMax, m_yMax );
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f(  m_xMax,  m_yMax, 0.0 );

      glTexCoord2f( m_xMin, m_yMax );
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f( m_xMin,  m_yMax, 0.0 );

      glEnd();
   }

   //glLoadIdentity( );
   glDisable( GL_TEXTURE_2D );
   glDisable( GL_LIGHTING );

   glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

   if ( m_drawBorder )
   {
      glColor3f( 0.7, 0.7, 0.7 );
      glBegin( GL_QUADS );
      glVertex3f( 0.0f, 0.0f, -0.25f );
      glVertex3f( 1.0f, 0.0f, -0.25f );
      glVertex3f( 1.0f, 1.0f, -0.25f );
      glVertex3f( 0.0f, 1.0f, -0.25f );
      glEnd();
   }

   useLinesColor();

   if ( m_operation == MouseRange )
   {
      glColor3f( 0.7, 0.7, 0.7 );
   }

   switch ( m_drawMode )
   {
      case DM_Edit:

         glBegin( GL_TRIANGLES );
         drawTriangles();
         glEnd();

         break;

      case DM_Edges:

         glBegin( GL_TRIANGLES );
         drawTriangles();
         glEnd();

         break;

      case DM_Filled:

         glColor3f( 0.0, 0.0, 0.8 );
         glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

         glBegin( GL_TRIANGLES );
         drawTriangles();
         glEnd();

         glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

         break;

      case DM_FilledEdges:

         glColor3f( 0.0, 0.0, 0.8 );
         glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

         glBegin( GL_TRIANGLES );
         drawTriangles();
         glEnd();

         glClear( GL_DEPTH_BUFFER_BIT );

         glColor3f( 1.0, 1.0, 1.0 );
         glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

         glBegin( GL_TRIANGLES );
         drawTriangles();
         glEnd();

         break;

      default:
         log_error( "Unknown draw mode: %d\n", m_drawMode );
         break;
   }

   // TODO may want to make "draw points" a separate property
   if ( m_operation == MouseRange )
   {
      useLinesColor();
   }

   // TODO may want to make "draw points" a separate property
   if ( m_operation != MouseRange || m_drawVertices )
   {
      glPointSize( 3.0 );

      glBegin( GL_POINTS );

      for ( unsigned t = 0; t < m_triangles.size(); t++ )
      {
         for ( unsigned v = 0; v < 3; v++ )
         {
            TextureVertexT * vert = m_vertices[ m_triangles[t]->vertex[v] ];
            if ( m_drawMode == DM_Edit && vert->selected )
            {
               useSelectionColor();
            }
            else
            {
               useLinesColor();
            }

            //glVertex3f( (vert->s - m_xMin) / m_zoom, (vert->t - m_yMin) / m_zoom, -0.5 );
            glVertex3f( vert->s, vert->t, -0.5 );
         }
      }

      glEnd();
   }

   if ( m_drawBounding )
   {
      drawSelectBox();
   }

   if ( m_drawRange )
   {
      drawRangeBox();
   }

   if ( m_operation == MouseRotate )
   {
      drawRotationPoint();
   }

   if ( m_interactive )
   {
      drawOverlay();
   }
}

void TextureWidget::drawTriangles()
{
   bool wrapLeft   = false;
   bool wrapRight  = false;
   bool wrapTop    = false;
   bool wrapBottom = false;

   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      TextureTriangleT * triangle = m_triangles[t];

      wrapLeft   = false;
      wrapRight  = false;
      wrapTop    = false;
      wrapBottom = false;

      for ( unsigned v = 0; v < 3; v++ )
      {
         glVertex3f( m_vertices[ triangle->vertex[v] ]->s, 
               m_vertices[ triangle->vertex[v] ]->t, 
               -0.5 );

         if ( m_drawMode != DM_Edit )
         {
            if ( m_vertices[ triangle->vertex[v] ]->s < 0.0 )
            {
               wrapLeft = true;
            }
            if ( m_vertices[ triangle->vertex[v] ]->s > 1.0 )
            {
               wrapRight = true;
            }
            if ( m_vertices[ triangle->vertex[v] ]->t < 0.0 )
            {
               wrapBottom = true;
            }
            if ( m_vertices[ triangle->vertex[v] ]->t > 1.0 )
            {
               wrapTop = true;
            }
         }
      }

      if ( m_drawMode != DM_Edit )
      {
         if ( wrapLeft )
         {
            for ( unsigned v = 0; v < 3; v++ )
            {
               glVertex3f( m_vertices[ triangle->vertex[v] ]->s + 1.0, 
                     m_vertices[ triangle->vertex[v] ]->t, 
                     -0.5 );
            }
         }
         if ( wrapRight )
         {
            for ( unsigned v = 0; v < 3; v++ )
            {
               glVertex3f( m_vertices[ triangle->vertex[v] ]->s - 1.0, 
                     m_vertices[ triangle->vertex[v] ]->t, 
                     -0.5 );
            }
         }
         if ( wrapBottom )
         {
            for ( unsigned v = 0; v < 3; v++ )
            {
               glVertex3f( m_vertices[ triangle->vertex[v] ]->s, 
                     m_vertices[ triangle->vertex[v] ]->t + 1.0, 
                     -0.5 );
            }
         }
         if ( wrapTop )
         {
            for ( unsigned v = 0; v < 3; v++ )
            {
               glVertex3f( m_vertices[ triangle->vertex[v] ]->s, 
                     m_vertices[ triangle->vertex[v] ]->t - 1.0, 
                     -0.5 );
            }
         }
      }
   }
}

void TextureWidget::animationTimeout()
{
   update();
}

void TextureWidget::setModel( Model * model )
{
   m_model = model;
   m_texture = NULL;
   clearCoordinates();
   glDisable( GL_TEXTURE_2D );
}

void TextureWidget::set3d( bool o )
{
   m_3d = o;

   updateViewport();

   if ( o )
   {
      m_animTimer->start( 30 );
   }
   else
   {
      m_animTimer->stop();
   }
}

void TextureWidget::setInteractive( bool o )
{
   m_interactive = o;
   update();
}

void TextureWidget::setTexture( int materialId, Texture * texture )
{
   m_materialId = materialId;
   m_texture = texture;

   m_sClamp = true;
   m_tClamp = true;
   if ( m_model )
   {
      m_sClamp = m_model->getTextureSClamp( materialId );
      m_tClamp = m_model->getTextureTClamp( materialId );
   }

   m_zoom    = 1.0;
   m_xCenter = 0.5;
   m_yCenter = 0.5;

   updateViewport();

   if ( m_texture )
   {
      /*
      m_xMin = m_xCenter - w;
      m_xMax = m_xCenter + w;
      m_yMin = m_yCenter - w;
      m_yMax = m_yCenter + w;
      */
   }

   if ( !isValid() )
   {
      // initializeGL() hasn't run yet
      return;
   }

   makeCurrent();
   updateGLTexture();

   resizeGL( this->width(), this->height() );
   update();
}

void TextureWidget::vFlipCoordinates()
{
   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->selected ) 
      {
         m_vertices[v]->t = 1.0 - m_vertices[v]->t;
      }
   }
   update();
}

void TextureWidget::hFlipCoordinates()
{
   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->selected ) 
      {
         m_vertices[v]->s = 1.0 - m_vertices[v]->s;
      }
   }
   update();
}

void TextureWidget::rotateCoordinatesCcw()
{
   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->selected ) 
      {

         double temp = m_vertices[v]->t;
         m_vertices[v]->t = m_vertices[v]->s;
         m_vertices[v]->s = 1.0 - temp;
      }
   }
   update();
}

void TextureWidget::rotateCoordinatesCw()
{
   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->selected ) 
      {

         double temp = m_vertices[v]->t;
         m_vertices[v]->t = 1.0 - m_vertices[v]->s;
         m_vertices[v]->s = temp;
      }
   }
   update();
}

int TextureWidget::addVertex( double s, double t )
{
   int index = m_vertices.size();

   TextureVertexT * vert = new TextureVertexT;
   vert->s = s;
   vert->t = t;
   vert->selected = true;
   m_vertices.push_back( vert );

   return index;
}

int TextureWidget::addTriangle( int v1, int v2, int v3 )
{
   int vCount = m_vertices.size();

   if (     v1 >= 0 && v1 < vCount
         && v2 >= 0 && v2 < vCount
         && v3 >= 0 && v3 < vCount )
   {
      int index = m_triangles.size();

      TextureTriangleT * triangle = new TextureTriangleT;
      triangle->vertex[0] = v1;
      triangle->vertex[1] = v2;
      triangle->vertex[2] = v3;
      m_triangles.push_back( triangle );

      return index;
   }

   return -1;
}

double TextureWidget::adjustToNearest( double angle )
{
   double f = angle / PIOVER180; // Change to degrees
   if ( f < 0.0 )
   {
      int n = (int) (f / 15.0 - 0.5);
      f = n * 15.0;
   }
   else
   {
      int n = (int) (f / 15.0 + 0.5);
      f = n * 15.0;
   }
   log_debug( "nearest angle is %f\n", f );
   return f * PIOVER180;
}

void TextureWidget::mousePressEvent( QMouseEvent * e )
{
   if ( m_interactive )
   {
      int w = this->width();
      int h = this->height();

      m_lastXPos = e->pos().x();
      m_lastYPos = e->pos().y();

      m_button = m_button | e->button();

      int bx = e->pos().x();
      int by = h - e->pos().y();

      m_overlayButton = ScrollButtonMAX;

      int sx = 0;
      int sy = 0;
      int size = SCROLL_SIZE;

      for ( int b = 0; m_overlayButton == ScrollButtonMAX && b < ScrollButtonMAX; b++ )
      {
         sx = s_buttons[b].x;
         sy = s_buttons[b].y;

         if (     (bx >= w + sx) && (bx <= w + sx + size) 
               && (by >= h + sy) && (by <= h + sy + size) )
         {
            m_overlayButton = (ScrollButtonE) b;

            switch ( m_overlayButton )
            {
               case ScrollButtonPan:
                  break;
               case ScrollButtonUp:
                  scrollUp();
                  m_scrollTimer->setSingleShot( true );
                  m_scrollTimer->start( 300 );
                  break;
               case ScrollButtonDown:
                  scrollDown();
                  m_scrollTimer->setSingleShot( true );
                  m_scrollTimer->start( 300 );
                  break;
               case ScrollButtonLeft:
                  scrollLeft();
                  m_scrollTimer->setSingleShot( true );
                  m_scrollTimer->start( 300 );
                  break;
               case ScrollButtonRight:
                  scrollRight();
                  m_scrollTimer->setSingleShot( true );
                  m_scrollTimer->start( 300 );
                  break;
               default:
                  break;
            }
         }
      }

      if ( m_overlayButton == ScrollButtonMAX )
      {
         if ( e->button() & Qt::MidButton )
         {
            // We're panning
         }
         else
         {
            switch ( m_operation )
            {
               case MouseSelect:
                  if ( !( (e->modifiers() & Qt::ShiftModifier) || (e->button() &  Qt::RightButton) ) )
                  {
                     clearSelected();
                  }
                  m_xSel1 =  (m_lastXPos / (double) this->width()) * (m_xMax - m_xMin) + m_xMin;
                  m_ySel1 =  (1.0 - (m_lastYPos / (double) this->height())) * (m_yMax - m_yMin) + m_yMin;
                  m_selecting = ( e->button() & Qt::RightButton ) ? false : true;
                  m_drawBounding = true;
                  break;
               case MouseMove:
                  m_allowX = true;
                  m_allowY = true;
                  break;
               case MouseScale:
                  m_allowX = true;
                  m_allowY = true;
                  startScale(
                        (m_lastXPos / (double) this->width()) * (m_xMax - m_xMin) + m_xMin , 
                        (1.0 - (m_lastYPos / (double) this->height())) * (m_yMax - m_yMin) + m_yMin );
                  break;
               case MouseRotate:
                  {
                     double aspect = (double) this->width() / (double) this->height();

                     if ( (e->button() & Qt::RightButton) )
                     {
                        m_xRotPoint =  (m_lastXPos / (double) this->width()) * (m_xMax - m_xMin) + m_xMin;
                        m_yRotPoint =  (1.0 - (m_lastYPos / (double) this->height())) * (m_yMax - m_yMin) + m_yMin;
                     }
                     else
                     {
                        m_xRotStart =  (m_lastXPos / (double) this->width()) * (m_xMax - m_xMin) + m_xMin;
                        m_yRotStart =  (1.0 - (m_lastYPos / (double) this->height())) * (m_yMax - m_yMin) + m_yMin;

                        m_xRotStart -= m_xRotPoint;
                        m_yRotStart -= m_yRotPoint;

                        double opposite = m_yRotStart;
                        double adjacent = m_xRotStart * aspect;

                        if ( adjacent < 0.0001 && adjacent > -0.0001 )
                        {
                           adjacent = (adjacent >= 0 ) ? 0.0001 : -0.0001;
                        }

                        double angle = atan(  opposite / adjacent );

                        float quad = PIOVER180 * 90;

                        if ( adjacent < 0 )
                        {
                           if ( opposite < 0 )
                           {
                              angle = -(quad) - ( (quad) - angle );
                           }
                           else
                           {
                              angle = (quad) + ( (quad) + angle );
                           }
                        }

                        if ( e->modifiers() & Qt::ShiftModifier )
                        {
                           angle = adjustToNearest( angle );
                        }

                        m_startAngle = angle;
                     }

                     freeRotateVertices();
                     for ( unsigned v = 0; v < m_vertices.size(); v++ )
                     {
                        if ( m_vertices[v]->selected ) 
                        {
                           RotateVertexT * rot = new RotateVertexT;
                           rot->x = (m_vertices[v]->s - m_xRotPoint) * aspect;
                           rot->y = m_vertices[v]->t - m_yRotPoint;
                           rot->v = v;
                           m_rotateVertices.push_back( rot );
                        }
                     }

                     update();
                  }
                  break;
               case MouseRange:
                  {
                     m_dragAll    = false;
                     m_dragTop    = false;
                     m_dragBottom = false;
                     m_dragLeft   = false;
                     m_dragRight  = false;

                     double windowX = getWindowXCoord( m_lastXPos );
                     double windowY = getWindowYCoord( m_lastYPos );

                     getDragDirections( windowX, windowY, 
                           m_dragAll, m_dragTop, m_dragBottom, m_dragLeft, m_dragRight );
                     setDragCursor( 
                           m_dragAll, m_dragTop, m_dragBottom, m_dragLeft, m_dragRight );
                  }
                  break;
               default:
                  log_error( "Unknown mouse operation: %d\n", m_operation );
                  break;
            }
         }
      }
   }
}

void TextureWidget::mouseReleaseEvent( QMouseEvent * e )
{
   if ( m_interactive )
   {
      if ( m_overlayButton == ScrollButtonMAX )
      {
         int x = e->pos().x();
         int y = e->pos().y();

         if ( e->button() & Qt::MidButton )
         {
            // We're panning
         }
         else
         {
            switch ( m_operation )
            {
               case MouseSelect:
                  m_drawBounding = false;
                  updateSelectRegion( 
                        (x / (double) this->width()) * (m_xMax - m_xMin) + m_xMin, 
                        (1.0 - (y / (double) this->height())) * (m_yMax - m_yMin) + m_yMin );
                  selectDone();
                  emit updateSelectionDoneSignal();
                  break;
               case MouseMove:
                  if ( e->modifiers() & Qt::ShiftModifier )
                  {
                     if ( m_allowX && m_allowY )
                     {
                        double ax = fabs( x - m_lastXPos );
                        double ay = fabs( y - m_lastYPos );

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
                     x = m_lastXPos;
                  }
                  if ( !m_allowY )
                  {
                     y = m_lastYPos;
                  }

                  moveSelectedVertices( 
                        ((x - m_lastXPos) / (double) this->width()) * (m_xMax - m_xMin), 
                        (-(y - m_lastYPos) / (double) this->height()) * (m_yMax - m_yMin) );
                  emit updateCoordinatesSignal();
                  emit updateCoordinatesDoneSignal();
                  break;
               case MouseRotate:
                  // Nothing to do here
                  emit updateCoordinatesDoneSignal();
                  break;
               case MouseScale:
                  // Nothing to do here
                  emit updateCoordinatesDoneSignal();
                  break;
               case MouseRange:
                  if ( m_button & Qt::LeftButton )
                  {
                     if ( m_dragAll || m_dragTop || m_dragBottom || m_dragLeft || m_dragRight )
                     {
                        emit updateRangeDoneSignal();
                     }
                  }
                  else
                  {
                     if ( m_dragAll )
                     {
                        emit updateSeamDoneSignal();
                     }
                  }
                  break;
               default:
                  log_error( "Unknown mouse operation: %d\n", m_operation );
                  break;
            }
         }
      }
      else
      {
         m_overlayButton = ScrollButtonMAX;
         m_scrollTimer->stop();
      }

      m_button = m_button & ~(e->button());
   }
}

void TextureWidget::mouseMoveEvent( QMouseEvent * e )
{
   if ( m_interactive )
   {
      int x = e->pos().x();
      int y = e->pos().y();

      if ( m_overlayButton == ScrollButtonMAX )
      {
         if ( m_button != 0 )
         {
            if ( m_button & Qt::MidButton )
            {
               double xDiff = (double) -(x - m_lastXPos);
               double yDiff = (double)  (y - m_lastYPos);

               xDiff = xDiff / (double) m_viewportWidth;
               yDiff = yDiff / (double) m_viewportHeight;

               xDiff *= m_zoom;
               yDiff *= m_zoom;

               m_xCenter += xDiff;
               m_yCenter += yDiff;

               m_xMin += xDiff;
               m_yMin += yDiff;
               m_xMax += xDiff;
               m_yMax += yDiff;

               updateViewport();
            }
            else
            {
               switch ( m_operation )
               {
                  case MouseSelect:
                     /*
                        updateSelectRegion( 
                        x / (double) this->width(), 
                        1.0 - (y / (double) this->height()) );
                        */
                     updateSelectRegion( 
                           (x / (double) this->width()) * m_zoom + m_xMin, 
                           (1.0 - (y / (double) this->height())) * m_zoom + m_yMin );
                     break;
                  case MouseMove:
                     if ( e->modifiers() & Qt::ShiftModifier )
                     {
                        if ( m_allowX && m_allowY )
                        {
                           double ax = fabs( x - m_lastXPos );
                           double ay = fabs( y - m_lastYPos );

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
                        x = m_lastXPos;
                     }
                     if ( !m_allowY )
                     {
                        y = m_lastYPos;
                     }

                     moveSelectedVertices( 
                           ((x - m_lastXPos) / (double) this->width()) * m_zoom, 
                           (-(y - m_lastYPos) / (double) this->height()) * m_zoom);
                     emit updateCoordinatesSignal();
                     break;
                  case MouseRotate:
                     {
                        double xNew = (x / (double) this->width())     * (m_xMax - m_xMin) + m_xMin;
                        double yNew = (1.0 - (y / (double) this->height())) * (m_yMax - m_yMin) + m_yMin;

                        xNew -= m_xRotPoint;
                        yNew -= m_yRotPoint;

                        double aspect = (double) this->width() / (double) this->height();

                        double opposite = yNew;
                        double adjacent = xNew * aspect;

                        if ( adjacent < 0.0001 && adjacent > -0.0001 )
                        {
                           adjacent = (adjacent >= 0 ) ? 0.0001 : -0.0001;
                        }

                        double angle = atan(  opposite / adjacent );

                        float quad = PIOVER180 * 90;

                        if ( adjacent < 0 )
                        {
                           if ( opposite < 0 )
                           {
                              angle = -(quad) - ( (quad) - angle );
                           }
                           else
                           {
                              angle = (quad) + ( (quad) + angle );
                           }
                        }

                        if ( e->modifiers() & Qt::ShiftModifier )
                        {
                           angle = adjustToNearest( angle );
                        }

                        rotateSelectedVertices( angle - m_startAngle );

                        emit updateCoordinatesSignal();
                     }
                     break;
                  case MouseScale:
                     if ( e->modifiers() & Qt::ShiftModifier )
                     {
                        if ( m_allowX && m_allowY )
                        {
                           double ax = fabs( x - m_lastXPos );
                           double ay = fabs( y - m_lastYPos );

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
                        x = m_lastXPos;
                     }
                     if ( !m_allowY )
                     {
                        y = m_lastYPos;
                     }

                     scaleSelectedVertices(
                           (x / (double) this->width()) * m_zoom + m_xMin, 
                           (1.0 - (y / (double) this->height())) * m_zoom + m_yMin );
                     emit updateCoordinatesSignal();
                     break;
                  case MouseRange:
                     if ( m_button & Qt::LeftButton )
                     {
                        double xThen = getWindowXCoord( m_lastXPos );
                        double xNow  = getWindowXCoord( x );

                        double yThen = getWindowYCoord( m_lastYPos );
                        double yNow  = getWindowYCoord( y );

                        double xDiff = xNow - xThen;
                        double yDiff = yNow - yThen;

                        if ( m_dragLeft || m_dragAll )
                        {
                           m_xRangeMin += xDiff;

                           if ( m_xRangeMin > m_xRangeMax )
                           {
                              m_xRangeMax = m_xRangeMin;
                           }
                        }
                        if ( m_dragRight || m_dragAll )
                        {
                           m_xRangeMax += xDiff;

                           if ( m_xRangeMax < m_xRangeMin )
                           {
                              m_xRangeMin = m_xRangeMax;
                           }
                        }
                        if ( m_dragBottom || m_dragAll )
                        {
                           m_yRangeMin += yDiff;

                           if ( m_yRangeMin > m_yRangeMax )
                           {
                              m_yRangeMax = m_yRangeMin;
                           }
                        }
                        if ( m_dragTop || m_dragAll )
                        {
                           m_yRangeMax += yDiff;

                           if ( m_yRangeMax < m_yRangeMin )
                           {
                              m_yRangeMin = m_yRangeMax;
                           }
                        }

                        if ( m_dragAll || m_dragTop || m_dragBottom || m_dragLeft || m_dragRight )
                        {
                           emit updateRangeSignal();
                        }
                     }
                     else
                     {
                        if ( m_dragAll )
                        {
                           double xThen = getWindowXCoord( m_lastXPos );
                           double xNow  = getWindowXCoord( x );

                           double yThen = getWindowYCoord( m_lastYPos );
                           double yNow  = getWindowYCoord( y );

                           double xDiff = xNow - xThen;
                           double yDiff = yNow - yThen;

                           xDiff *= -(2 * PI);
                           yDiff *= -(2 * PI);

                           emit updateSeamSignal( xDiff, yDiff );
                        }
                     }
                     break;
                  default:
                     log_error( "Unknown mouse operation: %d\n", m_operation );
                     break;
               }
            }
         }
         else
         {
            updateCursorShape( x, y );
         }
      }
      else
      {
         if ( m_overlayButton == ScrollButtonPan )
         {
            double xDiff = (double) -(x - m_lastXPos);
            double yDiff = (double)  (y - m_lastYPos);

            xDiff = xDiff / (double) m_viewportWidth;
            yDiff = yDiff / (double) m_viewportHeight;

            xDiff *= m_zoom;
            yDiff *= m_zoom;

            m_xCenter += xDiff;
            m_yCenter += yDiff;

            m_xMin += xDiff;
            m_yMin += yDiff;
            m_xMax += xDiff;
            m_yMax += yDiff;

            updateViewport();
         }
      }

      m_lastXPos = x;
      m_lastYPos = y;
   }
}

void TextureWidget::wheelEvent( QWheelEvent * e )
{
   if ( m_interactive )
   {
      if ( e->delta() > 0 )
      {
         zoomIn();
      }
      else
      {
         zoomOut();
      }
   }
}

void TextureWidget::keyPressEvent( QKeyEvent * e )
{
   if ( m_interactive )
   {
      switch ( e->key() )
      {
         case Qt::Key_Home:
            {
               if ( (e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier )
               {
                  if ( m_drawMode == DM_Edit )
                  {
                     if ( m_operation == MouseRange )
                     {
                        m_xCenter = (m_xRangeMax - m_xRangeMin) / 2.0 + m_xRangeMin;
                        m_yCenter = (m_yRangeMax - m_yRangeMin) / 2.0 + m_yRangeMin;

                        double xzoom = m_xRangeMax - m_xRangeMin;
                        double yzoom = m_yRangeMax - m_yRangeMin;

                        m_zoom = (xzoom > yzoom) ? xzoom : yzoom;

                        m_zoom *= 1.10;
                        updateViewport();
                     }
                     else
                     {
                        bool first = true;

                        double xMin = 0.0;
                        double xMax = 0.0;
                        double yMin = 0.0;
                        double yMax = 0.0;

                        size_t vcount = m_vertices.size();

                        for ( size_t v = 1; v < vcount; v++ )
                        {
                           if ( m_vertices[v]->selected )
                           {
                              if ( first )
                              {
                                 xMin = m_vertices[v]->s;
                                 yMin = m_vertices[v]->t;
                                 first = false;
                              }
                              else
                              {
                                 if ( m_vertices[v]->s < xMin )
                                    xMin = m_vertices[v]->s;
                                 if ( m_vertices[v]->s > xMax )
                                    xMax = m_vertices[v]->s;
                                 if ( m_vertices[v]->t < yMin )
                                    yMin = m_vertices[v]->t;
                                 if ( m_vertices[v]->t > yMax )
                                    yMax = m_vertices[v]->t;
                              }
                           }
                        }

                        if ( !first )
                        {
                           m_xCenter = (xMax - xMin) / 2.0 + xMin;
                           m_yCenter = (yMax - yMin) / 2.0 + yMin;

                           double xzoom = xMax - xMin;
                           double yzoom = yMax - yMin;

                           m_zoom = (xzoom > yzoom) ? xzoom : yzoom;

                           m_zoom *= 1.10;
                           updateViewport();
                        }
                     }
                  }
               }
               else
               {
                  m_xCenter = 0.5;
                  m_yCenter = 0.5;
                  m_zoom = 1.0;
                  updateViewport();
               }
            }
            break;
         case Qt::Key_Equal:
         case Qt::Key_Plus:
            {
               zoomIn();
            }
            break;
         case Qt::Key_Minus:
         case Qt::Key_Underscore:
            {
               zoomOut();
            }
            break;
         case Qt::Key_0:
            m_xCenter = 0.5;
            m_yCenter = 0.5;
            updateViewport();
            break;
         case Qt::Key_Up:
            scrollUp();
            break;
         case Qt::Key_Down:
            scrollDown();
            break;
         case Qt::Key_Left:
            scrollLeft();
            break;
         case Qt::Key_Right:
            scrollRight();
            break;
         default:
            QOpenGLWidget::keyPressEvent( e );
            break;
      }
   }
   else
   {
      QOpenGLWidget::keyPressEvent( e );
   }
}

void TextureWidget::zoomIn()
{
   if ( m_interactive )
   {
      if ( (m_zoom / VP_ZOOMSCALE) > 0.0001 )
      {
         m_zoom *= (VP_ZOOMSCALE);
      }

      QString zoomStr;
      zoomStr.sprintf( "%f", (float) m_zoom );
      emit zoomLevelChanged( zoomStr );

      updateViewport();
   }
}

void TextureWidget::zoomOut()
{
   if ( m_interactive )
   {
      if ( (m_zoom / VP_ZOOMSCALE) < 250000 )
      {
         m_zoom /= VP_ZOOMSCALE;
      }

      QString zoomStr;
      zoomStr.sprintf( "%f", (float) m_zoom );
      emit zoomLevelChanged( zoomStr );

      updateViewport();
   }
}

void TextureWidget::setZoomLevel( double zoom )
{
   if ( m_interactive )
   {
      m_zoom = zoom;

      updateViewport();
   }
}

void TextureWidget::scrollTimeout()
{
   switch ( m_overlayButton )
   {
      case ScrollButtonUp:
         scrollUp();
         break;
      case ScrollButtonDown:
         scrollDown();
         break;
      case ScrollButtonLeft:
         scrollLeft();
         break;
      case ScrollButtonRight:
         scrollRight();
         break;
      default:
         m_scrollTimer->stop();
         return;
   }

   m_scrollTimer->setSingleShot( false );
   m_scrollTimer->start( 100 );
}

void TextureWidget::scrollUp()
{
   m_yCenter += m_zoom * 0.10;
   updateViewport();
}

void TextureWidget::scrollDown()
{
   m_yCenter -= m_zoom * 0.10;
   updateViewport();
}

void TextureWidget::scrollLeft()
{
   m_xCenter -= m_zoom * 0.10;
   updateViewport();
}

void TextureWidget::scrollRight()
{
   m_xCenter += m_zoom * 0.10;
   updateViewport();
}

void TextureWidget::moveSelectedVertices( double x, double y )
{
   for ( unsigned t = 0; t < m_vertices.size(); t++ )
   {
      if ( m_vertices[t]->selected )
      {
         m_vertices[t]->s += x;
         m_vertices[t]->t += y;
      }
   }
   update();
}

void TextureWidget::updateSelectRegion( double x, double y )
{
   m_xSel2 = x;
   m_ySel2 = y;
   update();
}

void TextureWidget::setViewportDraw()
{
   glViewport( 0, 0, ( GLint ) m_viewportWidth, ( GLint ) m_viewportHeight );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity( );

   GLfloat ratio = ( GLfloat ) m_viewportWidth / ( GLfloat ) m_viewportHeight;

   glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

   if ( m_3d )
   {
      gluPerspective( 45.0f, ratio, 0.01, 30.0 );
   }
   else
   {
      glOrtho( m_xMin, m_xMax, m_yMin, m_yMax, -1.0, 1.0 );
   }

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );
}

void TextureWidget::setViewportOverlay()
{
   glViewport( 0, 0, ( GLint ) m_viewportWidth, ( GLint ) m_viewportHeight );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity( );

   glOrtho( 0, this->width(), 
         0, this->height(), 
         -1.0, 1.0 );

   glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );
}

void TextureWidget::drawOverlay()
{
   setViewportOverlay();

   glDisable( GL_LIGHTING );
   glColor3f( 1.0f, 1.0f, 1.0f );

   glEnable( GL_TEXTURE_2D );

   int w = this->width();
   int h = this->height();

   int sx = 0;
   int sy = 0;
   int size = SCROLL_SIZE;

   for ( int b = 0; b < ScrollButtonMAX; b++ )
   {
      ScrollButtonT * sbt = &s_buttons[b];
      sx = sbt->x;
      sy = sbt->y;

      glBindTexture( GL_TEXTURE_2D, m_scrollTextures[ sbt->texIndex ] );

      glBegin( GL_QUADS );

      glTexCoord2f( sbt->s1, sbt->t1 );
      glVertex3f( w + sx, h + sy, 0 );
      glTexCoord2f( sbt->s2, sbt->t2 );
      glVertex3f( w + sx + size, h + sy, 0 );
      glTexCoord2f( sbt->s3, sbt->t3 );
      glVertex3f( w + sx + size, h + sy + size, 0 );
      glTexCoord2f( sbt->s4, sbt->t4 );
      glVertex3f( w + sx, h + sy + size, 0 );

      glEnd();
   }

   glDisable( GL_TEXTURE_2D );
}

void TextureWidget::makeTextureFromImage( const QImage & i, GLuint & t )
{
   glBindTexture( GL_TEXTURE_2D, t );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
         GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
         GL_NEAREST );

   int w = i.width();
   int h = i.height();
   unsigned pixelCount = w * i.height();
   uint8_t * data = new uint8_t[ pixelCount * 3 ];
   for ( int y = 0; y < h; y ++ )
   {
      for ( int x = 0; x < w; x++ )
      {
         QRgb p = i.pixel( x, h - y - 1 );
         data[ ((y * w + x)*3) + 0 ] = qRed( p );
         data[ ((y * w + x)*3) + 1 ] = qGreen( p );
         data[ ((y * w + x)*3) + 2 ] = qBlue( p );
      }
   }

   gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB,
         w, h, 
         GL_RGB, GL_UNSIGNED_BYTE,
         data );

   delete[] data;
}

void TextureWidget::updateGLTexture()
{
   if ( !m_texture )
   {
      return;
   }

   glEnable( GL_TEXTURE_2D );

   if ( m_glTexture == 0 )
   {
      glGenTextures( 1, &m_glTexture );
   }

   glBindTexture( GL_TEXTURE_2D, m_glTexture );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
         GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
         GL_NEAREST );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
            (m_sClamp ? GL_CLAMP_TO_EDGE : GL_REPEAT) );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
         (m_tClamp ? GL_CLAMP_TO_EDGE : GL_REPEAT) );

   GLuint format = m_texture->m_format == Texture::FORMAT_RGBA ? GL_RGBA : GL_RGB;
   gluBuild2DMipmaps( GL_TEXTURE_2D, format,
         m_texture->m_width, m_texture->m_height,
         format, GL_UNSIGNED_BYTE,
         m_texture->m_data );
}

void TextureWidget::selectDone()
{
   if ( m_xSel1 > m_xSel2 )
   {
      double temp = m_xSel2;
      m_xSel2 = m_xSel1;
      m_xSel1 = temp;
   }
   if ( m_ySel1 > m_ySel2 )
   {
      double temp = m_ySel2;
      m_ySel2 = m_ySel1;
      m_ySel1 = temp;
   }
   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      if (     m_vertices[v]->s >= m_xSel1 && m_vertices[v]->s <= m_xSel2 
            && m_vertices[v]->t >= m_ySel1 && m_vertices[v]->t <= m_ySel2 )
      {
         m_vertices[v]->selected = m_selecting;
      }
   }
   update();
}

void TextureWidget::setTextureCount( unsigned c )
{
   setTexture( m_materialId, m_texture );
}

void TextureWidget::setMouseOperation( MouseOperationE op )
{
   m_operation = op;

   m_drawRange = false;

   if ( op == MouseRange )
   {
      m_drawRange = true;
   }
}

void TextureWidget::setDrawMode( DrawModeE dm )
{
   m_drawMode = dm;
}

void TextureWidget::drawSelectBox()
{
   glEnable( GL_COLOR_LOGIC_OP );
   glColor3f( 1.0, 1.0, 1.0 );
   glLogicOp( GL_XOR );
   glBegin( GL_LINES );

   glVertex3f( m_xSel1, m_ySel1, -0.75 );
   glVertex3f( m_xSel1, m_ySel2, -0.75 );

   glVertex3f( m_xSel1, m_ySel2, -0.75 );
   glVertex3f( m_xSel2, m_ySel2, -0.75 );

   glVertex3f( m_xSel2, m_ySel2, -0.75 );
   glVertex3f( m_xSel2, m_ySel1, -0.75 );

   glVertex3f( m_xSel2, m_ySel1, -0.75 );
   glVertex3f( m_xSel1, m_ySel1, -0.75 );

   glEnd();
   glLogicOp( GL_COPY );
   glDisable( GL_LOGIC_OP );
}

void TextureWidget::drawRangeBox()
{
   glLogicOp( GL_COPY );
   glDisable( GL_LOGIC_OP );

   glColor3f( 1.0, 1.0, 1.0 );
   glBegin( GL_LINES );

   glVertex3f( m_xRangeMin, m_yRangeMin, -0.95 );
   glVertex3f( m_xRangeMin, m_yRangeMax, -0.95 );

   glVertex3f( m_xRangeMin, m_yRangeMax, -0.95 );
   glVertex3f( m_xRangeMax, m_yRangeMax, -0.95 );

   glVertex3f( m_xRangeMax, m_yRangeMax, -0.95 );
   glVertex3f( m_xRangeMax, m_yRangeMin, -0.95 );

   glVertex3f( m_xRangeMax, m_yRangeMin, -0.95 );
   glVertex3f( m_xRangeMin, m_yRangeMin, -0.95 );

   glEnd();
}

void TextureWidget::drawRotationPoint()
{
   glLogicOp( GL_COPY );
   glDisable( GL_LOGIC_OP );

   glColor3f( 0.0, 1.0, 0.0 );
   glBegin( GL_LINES );

   double offset = m_zoom * 0.04;
   double aspect = (double) this->width() / (double) this->height();
   double xoff = offset / aspect;
   double yoff = offset;

   glVertex3f( m_xRotPoint - xoff, m_yRotPoint + 0.0,  -0.95 );
   glVertex3f( m_xRotPoint + 0.0,  m_yRotPoint - yoff, -0.95 );

   glVertex3f( m_xRotPoint + 0.0,  m_yRotPoint - yoff, -0.95 );
   glVertex3f( m_xRotPoint + xoff, m_yRotPoint + 0.0,  -0.95 );

   glVertex3f( m_xRotPoint + xoff, m_yRotPoint + 0.0,  -0.95 );
   glVertex3f( m_xRotPoint + 0.0,  m_yRotPoint + yoff, -0.95 );

   glVertex3f( m_xRotPoint + 0.0,  m_yRotPoint + yoff, -0.95 );
   glVertex3f( m_xRotPoint - xoff, m_yRotPoint + 0.0,  -0.95 );

   glEnd();
}

void TextureWidget::clearSelected()
{
   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      m_vertices[v]->selected = false;
   }
}

double TextureWidget::getWindowXCoord( int x )
{
   return (x / (double) m_viewportWidth) * m_zoom + m_xMin;
}

double TextureWidget::getWindowYCoord( int y )
{
   return ((m_viewportHeight - y) / (double) m_viewportHeight) * m_zoom + m_yMin;
}

void TextureWidget::updateCursorShape( int x, int y )
{
   if ( m_interactive )
   {
      int w = this->width();
      int h = this->height();

      int sx = 0;
      int sy = 0;
      int size = SCROLL_SIZE;

      int bx = x;
      int by = h - y;

      ScrollButtonE button = ScrollButtonMAX;

      for ( int b = 0; button == ScrollButtonMAX && b < ScrollButtonMAX; b++ )
      {
         sx = s_buttons[b].x;
         sy = s_buttons[b].y;

         if (     (bx >= w + sx) && (bx <= w + sx + size) 
               && (by >= h + sy) && (by <= h + sy + size) )
         {
            button = (ScrollButtonE) b;
         }
      }

      switch ( button )
      {
         case ScrollButtonPan:
         case ScrollButtonUp:
         case ScrollButtonDown:
         case ScrollButtonLeft:
         case ScrollButtonRight:
            break;
         default:
            break;
      }

      if ( button == ScrollButtonMAX )
      {
         if ( m_operation == MouseRange )
         {
            bool dragAll    = false;
            bool dragTop    = false;
            bool dragBottom = false;
            bool dragLeft   = false;
            bool dragRight  = false;

            double windowX = getWindowXCoord( x );
            double windowY = getWindowYCoord( y );

            getDragDirections( windowX, windowY,
                  dragAll, dragTop, dragBottom, dragLeft, dragRight );
            setDragCursor( dragAll, dragTop, dragBottom, dragLeft, dragRight );

            return;
         }
      }
   }

   setDragCursor( false, false, false, false, false );
}

void TextureWidget::getDragDirections( double windowX, double windowY,
      bool & dragAll, bool & dragTop, bool & dragBottom, bool & dragLeft, bool & dragRight )
{
   dragAll    = false;
   dragTop    = false;
   dragBottom = false;
   dragLeft   = false;
   dragRight  = false;

   double prox = (6.0 / m_viewportWidth) * m_zoom;

   if (  (windowX >= (m_xRangeMin - prox)) 
      && (windowX <= (m_xRangeMax + prox)) 
      && (windowY >= (m_yRangeMin - prox)) 
      && (windowY <= (m_yRangeMax + prox)) )
   {
      if ( fabs(m_xRangeMin - windowX) <= prox )
      {
         dragLeft = true;
      }
      if ( fabs(m_xRangeMax - windowX) <= prox )
      {
         dragRight = true;
      }
      if ( fabs(m_yRangeMin - windowY) <= prox )
      {
         dragBottom = true;
      }
      if ( fabs(m_yRangeMax - windowY) <= prox )
      {
         dragTop = true;
      }

      if ( dragLeft && dragRight )
      {
         // The min and max are very close together, don't drag
         // both at one time.
         if ( windowX < m_xRangeMin )
         {
            dragRight = false;
         }
         else if ( windowX > m_xRangeMax )
         {
            dragLeft = false;
         }
         else
         {
            // We're in-between, don't drag either (top/bottom still okay)
            dragLeft = false;
            dragRight = false;
         }
      }
      if ( dragTop && dragBottom )
      {
         // The min and max are very close together, don't drag
         // both at one time.
         if ( windowY < m_yRangeMin )
         {
            dragTop = false;
         }
         else if ( windowY > m_yRangeMax )
         {
            dragBottom = false;
         }
         else
         {
            // We're in-between, don't drag either (left/right still okay)
            dragTop = false;
            dragBottom = false;
         }
      }

      if ( !dragTop && !dragBottom && !dragLeft && !dragRight )
      {
         if ( (windowX > m_xRangeMin && windowX < m_xRangeMax)
               && (windowY > m_yRangeMin && windowY < m_yRangeMax ) )
         {
            dragAll = true;
         }
      }
   }
}

void TextureWidget::setDragCursor( bool dragAll, 
      bool dragTop, bool dragBottom, bool dragLeft, bool dragRight )
{
   if ( !m_interactive )
   {
      setCursor( QCursor( Qt::ArrowCursor ) );
   }
   else if ( (dragLeft && dragTop) || (dragRight && dragBottom) )
   {
      setCursor( QCursor( Qt::SizeFDiagCursor ) );
   }
   else if ( (dragLeft && dragBottom) || (dragRight && dragTop) )
   {
      setCursor( QCursor( Qt::SizeBDiagCursor ) );
   }
   else if ( dragLeft || dragRight )
   {
      setCursor( QCursor( Qt::SizeHorCursor ) );
   }
   else if ( dragTop || dragBottom )
   {
      setCursor( QCursor( Qt::SizeVerCursor ) );
   }
   else if ( dragAll )
   {
      setCursor( QCursor( Qt::SizeAllCursor ) );
   }
   else
   {
      setCursor( QCursor( Qt::ArrowCursor ) );
   }
}

void TextureWidget::clearCoordinates()
{
   while ( m_triangles.size() )
   {
      delete m_triangles.back();
      m_triangles.pop_back();
   }

   while ( m_vertices.size() )
   {
      delete m_vertices.back();
      m_vertices.pop_back();
   }

   freeRotateVertices();
}

void TextureWidget::getCoordinates( int tri, float * s, float * t )
{
   if ( !t || !s || tri >= (signed) m_triangles.size() )
   {
      return;
   }

   for ( int v = 0; v < 3; v++ )
   {
      s[v] = m_vertices[ m_triangles[ tri ]->vertex[v] ]->s;
      t[v] = m_vertices[ m_triangles[ tri ]->vertex[v] ]->t;
   }
}

void TextureWidget::saveSelectedUv()
{
   std::vector<int> selectedUv;

   for ( size_t vert = 0; vert < m_vertices.size(); ++vert )
   {
      if ( m_vertices[ vert ]->selected )
         selectedUv.push_back( vert );
   }

   m_model->setSelectedUv( selectedUv );
}

void TextureWidget::restoreSelectedUv()
{
   std::vector<int> selectedUv;
   m_model->getSelectedUv( selectedUv );

   for ( size_t vert = 0; vert < m_vertices.size(); ++vert )
   {
      m_vertices[ vert ]->selected = false;
   }

   for ( size_t vert = 0; vert < selectedUv.size(); ++vert )
   {
      size_t v = selectedUv[vert];
      if ( v < m_vertices.size() )
         m_vertices[ v ]->selected = true;
   }
}

void TextureWidget::setRange( double xMin, double yMin, double xMax, double yMax )
{
   m_xRangeMin = xMin;
   m_yRangeMin = yMin;
   m_xRangeMax = xMax;
   m_yRangeMax = yMax;
}

void TextureWidget::getRange( double & xMin, double & yMin, double & xMax, double & yMax )
{
   xMin = m_xRangeMin;
   yMin = m_yRangeMin;
   xMax = m_xRangeMax;
   yMax = m_yRangeMax;
}

void TextureWidget::startScale( double x, double y )
{
   m_scaleList.clear();
   bool first = true;

   double minX = 0;
   double minY = 0;
   double maxX = 0;
   double maxY = 0;

   for ( unsigned t = 0; t < m_vertices.size(); t++ )
   {
      if ( m_vertices[t]->selected )
      {
         // update range
         if ( first )
         {
            minX = m_vertices[t]->s;
            minY = m_vertices[t]->t;
            maxX = m_vertices[t]->s;
            maxY = m_vertices[t]->t;

            first = false;
         }
         else
         {
            if ( m_vertices[t]->s < minX ) { minX = m_vertices[t]->s; };
            if ( m_vertices[t]->t < minY ) { minY = m_vertices[t]->t; };
            if ( m_vertices[t]->s > maxX ) { maxX = m_vertices[t]->s; };
            if ( m_vertices[t]->t > maxY ) { maxY = m_vertices[t]->t; };
         }

         ScaleVerticesT sv;
         sv.index = t;
         sv.x = m_vertices[t]->s;
         sv.y = m_vertices[t]->t;

         m_scaleList.push_back( sv );
      }
   }

   if ( m_scaleFromCenter )
   {
      m_centerX = (maxX - minX) / 2.0 + minX;
      m_centerY = (maxY - minY) / 2.0 + minY;

      m_startLengthX = fabs( m_centerX - x );
      m_startLengthY = fabs( m_centerY - y );
   }
   else
   {
      double minmin = distance( x, y, minX, minY );
      double minmax = distance( x, y, minX, maxY );
      double maxmin = distance( x, y, maxX, minY );
      double maxmax = distance( x, y, maxX, maxY );

      if ( minmin > minmax )
      {
         if ( minmin > maxmin )
         {
            if ( minmin > maxmax )
            {
               m_farX = minX;
               m_farY = minY;
            }
            else
            {
               m_farX = maxX;
               m_farY = maxY;
            }
         }
         else // maxmin > minmin
         {
            if ( maxmin > maxmax )
            {
               m_farX = maxX;
               m_farY = minY;
            }
            else
            {
               m_farX = maxX;
               m_farY = maxY;
            }
         }
      }
      else // minmax > minmin
      {
         if ( minmax > maxmin )
         {
            if ( minmax > maxmax )
            {
               m_farX = minX;
               m_farY = maxY;
            }
            else
            {
               m_farX = maxX;
               m_farY = maxY;
            }
         }
         else // maxmin > minmax
         {
            if ( maxmin > maxmax )
            {
               m_farX = maxX;
               m_farY = minY;
            }
            else
            {
               m_farX = maxX;
               m_farY = maxY;
            }
         }
      }

      m_startLengthX = fabs( x - m_farX );
      m_startLengthY = fabs( y - m_farY );
   }
}

void TextureWidget::rotateSelectedVertices( double angle )
{
   Matrix m;
   Vector rot;
   rot[0] = 0.0;
   rot[1] = 0.0;
   rot[2] = angle;
   m.setRotation( rot );
   
   double aspect = (double) this->width() / (double) this->height();

   unsigned vcount = m_rotateVertices.size();
   for ( unsigned v = 0; v < vcount; v++ )
   {
      Vector vec;
      vec[0] = m_rotateVertices[v]->x;
      vec[1] = m_rotateVertices[v]->y;
      m.apply3( vec );

      unsigned index = m_rotateVertices[v]->v;
      m_vertices[index]->s = (vec[0] / aspect)+ m_xRotPoint;
      m_vertices[index]->t = vec[1] + m_yRotPoint;
   }

   update();
}

void TextureWidget::scaleSelectedVertices( double x, double y )
{
   double spX = m_scaleFromCenter ? m_centerX : m_farX;
   double spY = m_scaleFromCenter ? m_centerY : m_farY;

   double lengthX = fabs( x - spX );
   double lengthY = fabs( y - spY );

   ScaleVerticesList::iterator it;
   for( it = m_scaleList.begin(); it != m_scaleList.end(); it++ )
   {
      double x = (*it).x;
      double y = (*it).y;

      x -= spX;
      y -= spY;

      double xper = (lengthX / m_startLengthX);
      if ( m_startLengthX <= 0.00006 )
      {
         xper = 1.0;
      }
      double yper = (lengthY / m_startLengthY);
      if ( m_startLengthY <= 0.00006 )
      {
         yper = 1.0;
      }

      if ( m_scaleKeepAspect )
      {
         if ( xper > yper )
         {
            yper = xper;
         }
         else
         {
            xper = yper;
         }
      }

      x *= xper;
      y *= yper;

      x += spX;
      y += spY;

      m_vertices[(*it).index]->s = x;
      m_vertices[(*it).index]->t = y;
   }

   update();
}

void TextureWidget::freeRotateVertices()
{
   while ( m_rotateVertices.size() )
   {
      delete m_rotateVertices.back();
      m_rotateVertices.pop_back();
   }
}

double TextureWidget::distance( const double & x1, const double & y1, const double & x2, const double & y2 )
{
   double xDiff = x2 - x1;
   double yDiff = y2 - y1;
   return sqrt( xDiff*xDiff + yDiff*yDiff );
}

double TextureWidget::max( const double & a, const double & b )
{
   return ( a > b ) ? a : b;
}

void TextureWidget::useLinesColor()
{
   float b = ((m_linesColor >> 0) & 255) / 255.0;
   float g = ((m_linesColor >> 8) & 255) / 255.0;
   float r = ((m_linesColor >> 16) & 255) / 255.0;
   glColor3f( r, g, b );
}

void TextureWidget::useSelectionColor()
{
   float b = ((m_selectionColor >> 0) & 255) / 255.0;
   float g = ((m_selectionColor >> 8) & 255) / 255.0;
   float r = ((m_selectionColor >> 16) & 255) / 255.0;
   glColor3f( r, g, b );
}

