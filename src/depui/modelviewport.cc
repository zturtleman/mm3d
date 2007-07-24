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


#include "model.h"
#include "toolbox.h"
#include "tool.h"
#include "glmath.h"
#include "decalmgr.h"
#include "decal.h"
#include "log.h"
#include "modelstatus.h"
#include "texmgr.h"
#include "modelviewport.h"
#include "3dmprefs.h"
#include "mm3dport.h"

#include "mq3compat.h"

#include "pixmap/arrow.xpm"
#include "pixmap/crosshairrow.xpm"

#include <qfont.h>
#include <qtimer.h>
#include <math.h>
#include <stdarg.h>

#ifdef HAVE_QT4
#include <QMouseEvent>
#include <QWheelEvent>
#endif // HAVE_QT4

#define NEWVIEWPORT

#define VP_ZOOMSCALE 0.75

#define MM3D_ENABLEALPHA

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

static ScrollButtonT s_buttons[ ModelViewport::ScrollButtonMAX ] =
{
   { -18, -18, 1,   0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,  0.0f, 1.0f  }, // Pan
   { -52, -18, 0,   0.0f, 1.0f,   0.0f, 0.0f,   1.0f, 0.0f,  1.0f, 1.0f  }, // Left
   { -35, -18, 0,   0.0f, 0.0f,   0.0f, 1.0f,   1.0f, 1.0f,  1.0f, 0.0f  }, // Right
   { -18, -35, 0,   0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,  0.0f, 1.0f  }, // Up
   { -18, -52, 0,   0.0f, 1.0f,   1.0f, 1.0f,   1.0f, 0.0f,  0.0f, 0.0f  }, // Down
};

static Matrix s_mat;

ModelViewport::ModelViewport( QWidget * parent, const char * name )
   : QGLWidget( parent, name ),
     m_model( NULL ),
     m_operation( MO_None ),
     m_activeButton( Qt::NoButton ),
     m_viewDirection( ViewFront ),
     m_centerX( 0.0 ),
     m_centerY( 0.0 ),
     m_centerZ( 0.0 ),
     m_rotX( 0.0 ),
     m_rotY( 0.0 ),
     m_rotZ( 0.0 ),
     m_zoomLevel( 32.0 ),
     m_far( 10000.0 ),
     m_near( 1.0 ),
     m_farOrtho( 1000000.0 ),
     m_nearOrtho( 0.001 ),
     m_viewportWidth( 0 ),
     m_viewportHeight( 0 ),
     m_scrollTimer( new QTimer() ),
     m_overlayButton( ScrollButtonMAX ),
     m_capture( false ),
     m_texturesLoaded( false ),
     m_viewOptions( ViewTexture ),
     m_toolbox( NULL )
{
   // Default preferences
   m_arcballPoint[0] = 0.0;
   m_arcballPoint[1] = 0.0;
   m_arcballPoint[2] = 0.0;

   g_prefs.setDefault( "ui_grid_inc", 4.0 );

   g_prefs.setDefault( "ui_3dgrid_inc", 4.0 );
   g_prefs.setDefault( "ui_3dgrid_count", 6.0 );

   g_prefs.setDefault( "ui_3dgrid_xy", 0 );
   g_prefs.setDefault( "ui_3dgrid_xz", 1 );
   g_prefs.setDefault( "ui_3dgrid_yz", 0 );

   setAutoBufferSwap( false );

   setFocusPolicy( WheelFocus );
   setMinimumSize( 220, 180 );

   double rot[3] = { 45 * PIOVER180, 0, 0 };
   s_mat.setRotation( rot );

   m_backColor.setRgb( 130, 200, 200 );

   setAcceptDrops( true );
   setMouseTracking( true );

   QPixmap arrow( arrow_xpm );
   QPixmap cross( crosshairrow_xpm );

   QImage img;

   makeCurrent();

   glGenTextures( 2, m_scrollTextures );

   img = arrow.convertToImage();
   makeTextureFromImage( img, m_scrollTextures[0] );

   img = cross.convertToImage();
   makeTextureFromImage( img, m_scrollTextures[1] );

   connect( m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimeout()));
}

ModelViewport::~ModelViewport()
{
   log_debug( "deleting model viewport\n" );

   makeCurrent();

   glDeleteTextures( 1, &m_backgroundTexture );
   glDeleteTextures( 2, m_scrollTextures );

   freeTextures();

   delete m_scrollTimer;
}

void ModelViewport::freeTextures()
{
   log_debug( "freeing texture for viewport\n" );
   makeCurrent();
   if ( m_model )
   {
      m_model->removeContext( static_cast<ContextT>( this ) );
   }
}

void ModelViewport::initializeGL()
{
   glShadeModel( GL_SMOOTH );
   glClearColor( m_backColor.red() / 256.0, 
         m_backColor.green() / 256.0, 
         m_backColor.blue() / 256.0, 1.0f );
   glClearDepth( 1.0f );
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LEQUAL );
   glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

   {
      GLfloat ambient[]  = {  0.8f,  0.8f,  0.8f,  1.0f };
      GLfloat diffuse[]  = {  0.9f,  0.9f,  0.9f,  1.0f };
      GLfloat position[] = {  0.0f,  0.0f,  1.0f,  0.0f };

      glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );
      glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
      glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
      glLightfv( GL_LIGHT0, GL_POSITION, position );
      //glEnable( GL_LIGHT0 );
   }

   {
      GLfloat ambient[]  = {  0.8f,  0.4f,  0.4f,  1.0f };
      GLfloat diffuse[]  = {  0.9f,  0.5f,  0.5f,  1.0f };
      GLfloat position[] = {  0.0f,  0.0f,  1.0f,  0.0f };

      glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );
      glLightfv( GL_LIGHT1, GL_AMBIENT, ambient );
      glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse );
      glLightfv( GL_LIGHT1, GL_POSITION, position );
      //glEnable( GL_LIGHT1 );
   }

   {
      GLint texSize = 0;
      glGetIntegerv( GL_MAX_TEXTURE_SIZE, &texSize );
      log_debug( "max texture size is %dx%d\n", texSize, texSize );
   }

   glGenTextures( 1, &m_backgroundTexture );

   checkGlErrors();

#ifdef MM3D_ENABLEALPHA
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   if ( ! glIsEnabled( GL_BLEND) )
   {
      log_warning( "alpha not supported\n" );
      glDisable( GL_BLEND );
      glGetError(); // clear errors
   }
#endif // MM3D_ENABLEALPHA
}

void ModelViewport::resizeGL( int w, int h )
{
   if ( h == 0 )
   {
      h = 1;
   }

   m_viewportWidth  = w;
   m_viewportHeight = h;

   adjustViewport();
}

void ModelViewport::paintGL()
{
   //LOG_PROFILE();

   if ( m_inOverlay )
   {
      setViewportDraw();
   }

   if ( m_capture )
   {
      glClearColor( 130.0 / 256.0, 200.0 / 256.0, 200.0 / 256.0, 1.0f );
   }

   float viewPoint[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

   if ( m_viewDirection == ViewPerspective || m_viewDirection == ViewOrtho )
   {
      glEnable( GL_LIGHT0 );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      //if ( m_viewDirection == ViewPerspective )
      {
         glLoadIdentity( );
      }

#ifdef NEWVIEWPORT
      viewPoint[0] = m_arcballPoint[0]; // m_centerX;
      viewPoint[1] = m_arcballPoint[1]; // m_centerY;
      viewPoint[2] = m_arcballPoint[2] + (m_zoomLevel * 2.0);

      if ( m_viewDirection == ViewPerspective )
      {
         glTranslatef( -viewPoint[0], -viewPoint[1], -viewPoint[2] );
      }
      else
      {
         glTranslatef( 0.0, 0.0, -m_farOrtho / 2.0 );
      }
      glRotatef( m_rotZ, 0.0, 0.0, 1.0);
      glRotatef( m_rotY, 0.0, 1.0, 0.0);
      glRotatef( m_rotX, 1.0, 0.0, 0.0);
#else
      viewPoint[0] = m_centerX;
      viewPoint[1] = m_centerY;
      viewPoint[2] = (m_zoomLevel * 2.0);

      glTranslatef( -viewPoint[0], -viewPoint[1], -viewPoint[2] );
      glRotatef( m_rotZ, 0.0, 0.0, 1.0);
      glRotatef( m_rotY, 0.0, 1.0, 0.0);
      glRotatef( m_rotX, 1.0, 0.0, 0.0);
#endif // NEWVIEWPORT

      Matrix m;
      m.setRotationInDegrees( 0.0f, 0.0f, -m_rotZ );
      m.apply( viewPoint );
      m.setRotationInDegrees( 0.0f, -m_rotY, 0.0f );
      m.apply( viewPoint );
      m.setRotationInDegrees( -m_rotX, 0.0f, 0.0f );
      m.apply( viewPoint );
   }
   else
   {
      glDisable( GL_LIGHT0 );
      glDisable( GL_LIGHT1 );

      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glLoadIdentity( );

      viewPoint[0] = 0.0f;
      viewPoint[1] = 0.0f;
      viewPoint[2] = 500000.0f;

      glTranslatef( -viewPoint[0], -viewPoint[1], -viewPoint[2] );

      Matrix m;
      switch ( m_viewDirection )
      {
         case ViewFront:
            // do nothing
            break;
         case ViewBack:
            glRotatef( 180.0, 0.0, 1.0, 0.0);
            m.setRotationInDegrees( 0.0f, -180.0f, 0.0f );
            break;
         case ViewLeft:
            glRotatef( -90.0, 0.0, 1.0, 0.0);
            m.setRotationInDegrees( 0.0f, 90.0f, 0.0f );
            break;
         case ViewRight:
            glRotatef(  90.0, 0.0, 1.0, 0.0);
            m.setRotationInDegrees( 0.0f, -90.0f, 0.0f );
            break;
         case ViewTop:
            glRotatef(  90.0, 1.0, 0.0, 0.0);
            m.setRotationInDegrees( -90.0f, 0.0f, 0.0f );
            break;
         case ViewBottom:
            glRotatef( -90.0, 1.0, 0.0, 0.0);
            m.setRotationInDegrees( 90.0f, 0.0f, 0.0f );
            break;
         default:
            log_error( "Unknown ViewDirection: %d\n", m_viewDirection );
            swapBuffers();
            return;
            break;
      }

      m.apply( viewPoint );
   }

   if ( !m_capture )
   {
      drawGridLines();
   }

   if ( m_model )
   {
      glColor3f( 0.7, 0.7, 0.7 );
      if ( m_viewDirection == ViewPerspective )
      {
         glEnable( GL_LIGHTING );
         int opt = Model::DO_TEXTURE | Model::DO_SMOOTHING 
            | ( (g_prefs( "ui_render_bad_textures" ).intValue() == 0) ? 0 : Model::DO_BADTEX);

         bool drawSelections 
            = (g_prefs( "ui_render_3d_selections" ).intValue() == 0) 
            ? false : true;

         switch ( m_viewOptions )
         {
            case ViewWireframe:
               opt = Model::DO_WIREFRAME;
               drawSelections = true;
               break;
            case ViewFlat:
               opt = Model::DO_NONE;
               break;
            case ViewSmooth:
               opt = Model::DO_SMOOTHING;
               break;
            case ViewAlpha:
               opt = opt | Model::DO_ALPHA;
               break;
            default:
               break;
         }

         opt |= ( (g_prefs( "ui_render_backface_cull" ).intValue() == 0) ? 0 : Model::DO_BACKFACECULL);

         if ( drawSelections )
         {
            glDisable( GL_LIGHTING );
            m_model->drawLines();
            m_model->drawVertices();
         }

         if ( opt != Model::DO_WIREFRAME )
         {
            glEnable( GL_LIGHTING );
            m_model->draw( opt, static_cast<ContextT>( this) , viewPoint );

         }

         if ( drawSelections )
         {
            glDisable( GL_LIGHTING );
            glDisable( GL_DEPTH_TEST );
            m_model->drawJoints();
         }

         glDisable( GL_LIGHTING );
         glDisable( GL_DEPTH_TEST );
         m_model->drawPoints();
         m_model->drawProjections();
      }
      else
      {
         // Draw background
         drawBackground();

         glClear( GL_DEPTH_BUFFER_BIT );

         m_model->drawLines();
         m_model->drawVertices();

         int drawMode = m_model->getCanvasDrawMode();
         if ( drawMode != ViewWireframe )
         {
            glEnable( GL_LIGHTING );
            glEnable( GL_LIGHT0 );

            int opt = Model::DO_TEXTURE | Model::DO_SMOOTHING 
               | ( (g_prefs( "ui_render_bad_textures" ).intValue() == 0) ? 0 : Model::DO_BADTEX);
            switch ( drawMode )
            {
               case ViewFlat:
                  opt = Model::DO_NONE;
                  break;
               case ViewSmooth:
                  opt = Model::DO_SMOOTHING;
                  break;
               case ViewAlpha:
                  opt = opt | Model::DO_ALPHA;
                  break;
               default:
                  break;
            }
            opt |= ( (g_prefs( "ui_render_backface_cull" ).intValue() == 0) ? 0 : Model::DO_BACKFACECULL);
            m_model->draw( opt, static_cast<ContextT>( this ), viewPoint );

            glDisable( GL_LIGHTING );
         }

         glDisable( GL_DEPTH_TEST );
         m_model->drawJoints();
         m_model->drawPoints();
         m_model->drawProjections();
         for ( DecalList::iterator it = m_decals.begin(); it != m_decals.end(); it++ )
         {
            (*it)->draw();
         }
         glEnable( GL_DEPTH_TEST );
      }
   }

   glDisable( GL_LIGHTING );
   glDisable( GL_TEXTURE_2D );

   if ( !m_capture )
   {
      drawOrigin();
   }
   else
   {
      glEnable( GL_DEPTH_TEST );
   }

   if ( this->hasFocus() )
   {
      drawOverlay();
   }

   checkGlErrors();

   swapBuffers();
}

void ModelViewport::drawGridLines()
{
   if ( m_viewDirection == ViewPerspective || m_viewDirection == ViewOrtho )
   {
      double inc = g_prefs( "ui_3dgrid_inc" ).doubleValue();
      double max = g_prefs( "ui_3dgrid_count" ).doubleValue() * inc;
      double x;
      double y;
      double z;

      glColor3f( 0.55f, 0.55f, 0.55f );

      glBegin( GL_LINES );

      if ( g_prefs( "ui_3dgrid_xy" ).intValue() != 0 )
      {
         for ( x = -max; x <= max; x += inc )
         {
            glVertex3f( x, -max, 0 );
            glVertex3f( x, +max, 0 );
         }

         for ( y = -max; y <= max; y += inc )
         {
            glVertex3f( -max, y, 0 );
            glVertex3f( +max, y, 0 );
         }
      }

      if ( g_prefs( "ui_3dgrid_xz" ).intValue() != 0 )
      {
         for ( x = -max; x <= max; x += inc )
         {
            glVertex3f( x, 0, -max );
            glVertex3f( x, 0, +max );
         }

         for ( z = -max; z <= max; z += inc )
         {
            glVertex3f( -max, 0, z );
            glVertex3f( +max, 0, z );
         }
      }

      if ( g_prefs( "ui_3dgrid_yz" ).intValue() != 0 )
      {
         for ( y = -max; y <= max; y += inc )
         {
            glVertex3f( 0, y, -max );
            glVertex3f( 0, y, +max );
         }

         for ( z = -max; z <= max; z += inc )
         {
            glVertex3f( 0, -max, z );
            glVertex3f( 0, +max, z );
         }
      }

      glEnd();
   }
   else
   {
      double maxDimension = (m_width > m_height) ? m_width : m_height;

      double unitWidth = g_prefs( "ui_grid_inc" ).doubleValue();

      while ( (maxDimension / unitWidth) > 16 )
      {
         unitWidth *= 2.0;
      }
      while ( (maxDimension / unitWidth) < 4 )
      {
         unitWidth /= 2.0;
      }

      double xRangeMin = 0;
      double xRangeMax = 0;
      double yRangeMin = 0;
      double yRangeMax = 0;

      double xStart = 0;
      double yStart = 0;

      double x = 0;
      double y = 0;

      glColor3f( 0.55f, 0.55f, 0.55f );

      glBegin( GL_LINES );
      
      switch ( m_viewDirection )
      {
         case ViewFront:
            xRangeMin = m_arcballPoint[0] - (m_width / 2.0);
            xRangeMax = m_arcballPoint[0] + (m_width / 2.0);
            yRangeMin = m_arcballPoint[1] - (m_height / 2.0);
            yRangeMax = m_arcballPoint[1] + (m_height / 2.0);

            xStart = unitWidth * ((int) (xRangeMin / unitWidth));
            yStart = unitWidth * ((int) (yRangeMin / unitWidth));

            for ( x = xStart; x < xRangeMax; x += unitWidth )
            {
               glVertex3f( x, yRangeMin, 0 );
               glVertex3f( x, yRangeMax, 0 );
            }

            for ( y = yStart; y < yRangeMax; y += unitWidth )
            {
               glVertex3f( xRangeMin, y, 0 );
               glVertex3f( xRangeMax, y, 0 );
            }
            break;

         case ViewBack:
            xRangeMin = -m_arcballPoint[0] - (m_width / 2.0);
            xRangeMax = -m_arcballPoint[0] + (m_width / 2.0);
            yRangeMin = m_arcballPoint[1] - (m_height / 2.0);
            yRangeMax = m_arcballPoint[1] + (m_height / 2.0);

            xStart = unitWidth * ((int) (xRangeMin / unitWidth));
            yStart = unitWidth * ((int) (yRangeMin / unitWidth));

            for ( x = xStart; x < xRangeMax; x += unitWidth )
            {
               glVertex3f( x, yRangeMin, 0 );
               glVertex3f( x, yRangeMax, 0 );
            }

            for ( y = yStart; y < yRangeMax; y += unitWidth )
            {
               glVertex3f( xRangeMin, y, 0 );
               glVertex3f( xRangeMax, y, 0 );
            }
            break;

         case ViewLeft:
            xRangeMin = -m_arcballPoint[0] - (m_width / 2.0);
            xRangeMax = -m_arcballPoint[0] + (m_width / 2.0);
            yRangeMin = m_arcballPoint[1] - (m_height / 2.0);
            yRangeMax = m_arcballPoint[1] + (m_height / 2.0);

            xStart = unitWidth * ((int) (xRangeMin / unitWidth));
            yStart = unitWidth * ((int) (yRangeMin / unitWidth));

            for ( x = xStart; x < xRangeMax; x += unitWidth )
            {
               glVertex3f( 0, yRangeMin, x );
               glVertex3f( 0, yRangeMax, x );
            }

            for ( y = yStart; y < yRangeMax; y += unitWidth )
            {
               glVertex3f( 0, y, xRangeMin );
               glVertex3f( 0, y, xRangeMax );
            }
            break;

         case ViewRight:
            xRangeMin = m_arcballPoint[0] - (m_width / 2.0);
            xRangeMax = m_arcballPoint[0] + (m_width / 2.0);
            yRangeMin = m_arcballPoint[1] - (m_height / 2.0);
            yRangeMax = m_arcballPoint[1] + (m_height / 2.0);

            xStart = unitWidth * ((int) (xRangeMin / unitWidth));
            yStart = unitWidth * ((int) (yRangeMin / unitWidth));

            for ( x = xStart; x < xRangeMax; x += unitWidth )
            {
               glVertex3f( 0, yRangeMin, x );
               glVertex3f( 0, yRangeMax, x );
            }

            for ( y = yStart; y < yRangeMax; y += unitWidth )
            {
               glVertex3f( 0, y, xRangeMin );
               glVertex3f( 0, y, xRangeMax );
            }
            break;

         case ViewTop:
            xRangeMin = m_arcballPoint[0] - (m_width / 2.0);
            xRangeMax = m_arcballPoint[0] + (m_width / 2.0);
            yRangeMin = -m_arcballPoint[1] - (m_height / 2.0);
            yRangeMax = -m_arcballPoint[1] + (m_height / 2.0);

            xStart = unitWidth * ((int) (xRangeMin / unitWidth));
            yStart = unitWidth * ((int) (yRangeMin / unitWidth));

            for ( x = xStart; x < xRangeMax; x += unitWidth )
            {
               glVertex3f( x, 0, yRangeMin );
               glVertex3f( x, 0, yRangeMax );
            }

            for ( y = yStart; y < yRangeMax; y += unitWidth )
            {
               glVertex3f( xRangeMin, 0, y );
               glVertex3f( xRangeMax, 0, y );
            }
            break;

         case ViewBottom:
            xRangeMin = m_arcballPoint[0] - (m_width / 2.0);
            xRangeMax = m_arcballPoint[0] + (m_width / 2.0);
            yRangeMin = m_arcballPoint[1] - (m_height / 2.0);
            yRangeMax = m_arcballPoint[1] + (m_height / 2.0);

            xStart = unitWidth * ((int) (xRangeMin / unitWidth));
            yStart = unitWidth * ((int) (yRangeMin / unitWidth));

            for ( x = xStart; x < xRangeMax; x += unitWidth )
            {
               glVertex3f( x, 0, yRangeMin );
               glVertex3f( x, 0, yRangeMax );
            }

            for ( y = yStart; y < yRangeMax; y += unitWidth )
            {
               glVertex3f( xRangeMin, 0, y );
               glVertex3f( xRangeMax, 0, y );
            }
            break;

         default:
            log_error( "Unhandled view direction: %d\n", m_viewDirection );
            break;
      }

      glEnd();

      glColor3f( 0.35f, 0.35f, 0.35f );
      glRasterPos3f( xRangeMin, yRangeMin, 0.0f );
      switch ( m_viewDirection )
      {
         case ViewFront:
            break;

         case ViewBack:
            glRasterPos3f( xRangeMax, yRangeMin, 0.0f );
            break;

         case ViewLeft:
            glRasterPos3f( 0.0f, yRangeMin, xRangeMax );
            break;

         case ViewRight:
            glRasterPos3f( 0.0f, yRangeMin, xRangeMin );
            break;

         case ViewTop:
            glRasterPos3f( xRangeMin, 0.0f, yRangeMax );
            break;

         case ViewBottom:
            glRasterPos3f( xRangeMin, 0.0f, yRangeMin );
            break;

         default:
            log_error( "Unhandled view direction: %d\n", m_viewDirection );
            break;
      }
      QString text;
      text.sprintf( "%g", unitWidth );
      renderText( 2, this->height() - 12, text, QFont( "Fixed", 10 ) );
   }
}

void ModelViewport::drawOrigin()
{
   glDisable( GL_DEPTH_TEST );

   glBegin( GL_LINES );

   double scale = m_zoomLevel / 10.0;

#ifdef NEWVIEWPORT
   if ( m_viewDirection == ViewPerspective )
   {
      double x = m_arcballPoint[0];
      double y = m_arcballPoint[1];
      double z = m_arcballPoint[2] + m_zoomLevel;
      scale = sqrt( x*x + y*y + z*z ) / 10.0;
   }
#endif // NEWVIEWPORT

   glColor3f( 1, 0, 0 );
   glVertex3f( 0, 0, 0 );
   glVertex3f( scale, 0, 0 );
   glColor3f( 0, 1, 0 );
   glVertex3f( 0, 0, 0 );
   glVertex3f( 0, scale, 0 );
   glColor3f( 0, 0, 1 );
   glVertex3f( 0, 0, 0 );
   glVertex3f( 0, 0, scale );

   glEnd();

   glEnable( GL_DEPTH_TEST );
}

void ModelViewport::drawBackground()
{
   glDisable( GL_LIGHTING );
   glColor3f( 1.0f, 1.0f, 1.0f );

   updateBackground();

   if ( m_backgroundFile[0] != '\0' )
   {
      if ( m_viewDirection != ViewPerspective && m_viewDirection != ViewOrtho )
      {
         int index = (int) m_viewDirection - 1;

         float cenX  = 0.0f;
         float cenY  = 0.0f;
         float cenZ  = 0.0f;

         float minX  = 0.0f;
         float minY  = 0.0f;
         float minZ  = 0.0f;
         float maxX  = 0.0f;
         float maxY  = 0.0f;
         float maxZ  = 0.0f;

         float normX = 0.0f;
         float normY = 0.0f;
         float normZ = 0.0f;

         float w = m_texture->m_origWidth;
         float h = m_texture->m_origHeight;
         float dimMax = w > h ? w : h;

         float scale  = m_model->getBackgroundScale( index );
         m_model->getBackgroundCenter( index, cenX, cenY, cenZ );

         glBindTexture( GL_TEXTURE_2D, m_backgroundTexture );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP  );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
         glEnable( GL_TEXTURE_2D );

         glBegin( GL_QUADS );

         switch ( m_viewDirection )
         {
            case ViewFront:
               minZ  =  maxZ = -((m_farOrtho / 2.0f) - 0.1);
               minX  = -scale * (w / dimMax) + cenX;
               maxX  =  scale * (w / dimMax) + cenX;
               minY  = -scale * (h / dimMax) + cenY;
               maxY  =  scale * (h / dimMax) + cenY;
               normZ =  1.0f;
               break;

            case ViewBack:
               minZ  =  maxZ = ((m_farOrtho / 2.0f) - 0.1);
               minX  =  scale * (w / dimMax) + cenX;
               maxX  = -scale * (w / dimMax) + cenX;
               minY  = -scale * (h / dimMax) + cenY;
               maxY  =  scale * (h / dimMax) + cenY;
               normZ = -1.0f;
               break;

            case ViewRight:
               minX  =  maxX = ((m_farOrtho / 2.0f) - 0.1);
               minZ  = -scale * (w / dimMax) + cenZ;
               maxZ  =  scale * (w / dimMax) + cenZ;
               minY  = -scale * (h / dimMax) + cenY;
               maxY  =  scale * (h / dimMax) + cenY;
               normX =  1.0f;
               break;

            case ViewLeft:
               minX  =  maxX = -((m_farOrtho / 2.0f) - 0.1);
               minZ  =  scale * (w / dimMax) + cenZ;
               maxZ  = -scale * (w / dimMax) + cenZ;
               minY  = -scale * (h / dimMax) + cenY;
               maxY  =  scale * (h / dimMax) + cenY;
               normX = -1.0f;
               break;

            case ViewTop:
               minY  =  maxY = -((m_farOrtho / 2.0f) - 0.1);
               minX  = -scale * (w / dimMax) + cenX;
               maxX  =  scale * (w / dimMax) + cenX;
               minZ  =  scale * (h / dimMax) + cenZ;
               maxZ  = -scale * (h / dimMax) + cenZ;
               normY =  1.0f;
               break;

            case ViewBottom:
               minY  =  maxY = ((m_farOrtho / 2.0f) - 0.1);
               minX  = -scale * (w / dimMax) + cenX;
               maxX  =  scale * (w / dimMax) + cenX;
               minZ  = -scale * (h / dimMax) + cenZ;
               maxZ  =  scale * (h / dimMax) + cenZ;
               normY = -1.0f;
               break;

            default:
               break;
         }

         if ( m_viewDirection == ViewLeft || m_viewDirection == ViewRight )
         {
            glNormal3f( normX, normY, normZ );
            glTexCoord2f( 0.0f, 0.0f );
            glVertex3f( minX,  minY, minZ );
            glNormal3f( normX, normY, normZ );
            glTexCoord2f( 0.0f, 1.0f );
            glVertex3f( maxX,  maxY, minZ );
            glNormal3f( normX, normY, normZ );
            glTexCoord2f( 1.0f, 1.0f );
            glVertex3f( maxX,  maxY, maxZ );
            glNormal3f( normX, normY, normZ );
            glTexCoord2f( 1.0f, 0.0f );
            glVertex3f( minX,  minY, maxZ );
         }
         else
         {
            glNormal3f( normX, normY, normZ );
            glTexCoord2f( 0.0f, 0.0f );
            glVertex3f( minX,  minY, minZ );
            glNormal3f( normX, normY, normZ );
            glTexCoord2f( 1.0f, 0.0f );
            glVertex3f( maxX,  minY, minZ );
            glNormal3f( normX, normY, normZ );
            glTexCoord2f( 1.0f, 1.0f );
            glVertex3f( maxX,  maxY, maxZ );
            glNormal3f( normX, normY, normZ );
            glTexCoord2f( 0.0f, 1.0f );
            glVertex3f( minX,  maxY, maxZ );
         }

         glEnd();
      }
   }
}

void ModelViewport::drawOverlay()
{
   setViewportOverlay();

   glDisable( GL_LIGHTING );
   glColor3f( 1.0f, 1.0f, 1.0f );

   glEnable( GL_TEXTURE_2D );

   int w = this->width();
   int h = this->height();

   /*
   glVertex3f( w - , h, 0 );
   glVertex3f( w - SCROLL_SIZE, h, 0 );
   glVertex3f( w - SCROLL_SIZE, h - SCROLL_SIZE, 0 );
   glVertex3f( w, h - SCROLL_SIZE, 0 );

   glVertex3f( w - SCROLL_ALL_X, h - SCROLL_ALL_Y, 0 );
   glVertex3f( w - SCROLL_ALL_X + SCROLL_SIZE, h - SCROLL_ALL_Y, 0 );
   glVertex3f( w - SCROLL_ALL_X + SCROLL_SIZE, h - SCROLL_ALL_Y + SCROLL_SIZE, 0 );
   glVertex3f( w - SCROLL_ALL_X, h - SCROLL_ALL_Y + SCROLL_SIZE, 0 );
   */

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

void ModelViewport::makeTextureFromImage( const QImage & i, GLuint & t )
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

   delete data;
}

void ModelViewport::updateBackground()
{
   int index = (int) m_viewDirection - 1;
   std::string filename = m_model->getBackgroundImage( index );
   if ( strcmp( filename.c_str(), m_backgroundFile.c_str() ) != 0 )
   {
      m_backgroundFile = filename;

      if ( m_backgroundFile[0] != '\0' )
      {
         m_texture = TextureManager::getInstance()->getTexture( m_backgroundFile.c_str() );
         if ( !m_texture )
         {
            QString str = tr("Could not load background %1").arg( m_backgroundFile.c_str() );
            model_status( m_model, StatusError, STATUSTIME_LONG, "%s", (const char *) str.utf8() );
            m_texture = TextureManager::getInstance()->getDefaultTexture( m_backgroundFile.c_str() );
         }
         glBindTexture( GL_TEXTURE_2D, m_backgroundTexture );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

         GLuint format = m_texture->m_format == Texture::FORMAT_RGBA ? GL_RGBA : GL_RGB;
         gluBuild2DMipmaps( GL_TEXTURE_2D, format,
               m_texture->m_width, m_texture->m_height,
               format, GL_UNSIGNED_BYTE,
               m_texture->m_data );
      }
   }
}

void ModelViewport::adjustViewport()
{
   setViewportDraw();

   updateGL();
}

void ModelViewport::setViewportDraw()
{
   makeCurrent();

   m_inOverlay = false;

   glViewport( 0, 0, ( GLint ) m_viewportWidth, ( GLint ) m_viewportHeight );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity( );

   GLfloat ratio = ( GLfloat ) m_viewportWidth / ( GLfloat ) m_viewportHeight;
   if ( m_viewDirection == ViewPerspective )
   {
      m_far  = m_zoomLevel * 2000.0;
      m_near = m_zoomLevel * 0.002;
      gluPerspective( 45.0f, ratio, m_near, m_far );

      float x = 0.0;
      float y = 0.0;
      if ( m_viewportHeight > m_viewportWidth )
      {
         x = m_zoomLevel;
         y = x / ratio;
      }
      else
      {
         y = m_zoomLevel;
         x = y * ratio;
      }

      m_width = x * 2.0;
      m_height = y * 2.0;
   }
   else
   {
      float x = 0.0;
      float y = 0.0;
      if ( m_viewportHeight > m_viewportWidth )
      {
         x = m_zoomLevel;
         y = x / ratio;
      }
      else
      {
         y = m_zoomLevel;
         x = y * ratio;
      }
#ifdef NEWVIEWPORT
      glOrtho( m_arcballPoint[0] - x, m_arcballPoint[0] + x, 
            m_arcballPoint[1] - y, m_arcballPoint[1] + y, 
            m_nearOrtho, m_farOrtho );
#else // NEWVIEWPORT
      glOrtho( m_centerX - x, m_centerX + x, 
            m_centerY - y, m_centerY + y, 
            m_nearOrtho, m_farOrtho );
#endif // NEWVIEWPORT

      m_width = x * 2.0;
      m_height = y * 2.0;

      switch ( m_viewDirection )
      {
         case ViewFront:
            m_rotX = 0;
            m_rotY = 0;
            m_rotZ = 0;
            break;
         case ViewBack:
            m_rotX = 0;
            m_rotY = 180;
            m_rotZ = 0;
            break;
         case ViewLeft:
            m_rotX = 0;
            m_rotY = 90;
            m_rotZ = 0;
            break;
         case ViewRight:
            m_rotX = 0;
            m_rotY = -90;
            m_rotZ = 0;
            break;
         case ViewTop:
            m_rotX = -90;
            m_rotY = 0;
            m_rotZ = 0;
            break;
         case ViewBottom:
            m_rotX = 90;
            m_rotY = 0;
            m_rotZ = 0;
            break;
         default:
            break;
      }
   }

   m_viewMatrix.loadIdentity();
   if ( m_viewDirection == ViewOrtho )
   {
      m_viewMatrix.setRotationInDegrees( m_rotX, m_rotY, m_rotZ );
   }
   else
   {
      m_viewMatrix.setRotationInDegrees( -m_rotX, -m_rotY, -m_rotZ );
   }
#ifdef NEWVIEWPORT
   m_viewMatrix.setTranslation( -m_arcballPoint[0], -m_arcballPoint[1], -m_arcballPoint[2] );
#else // NEWVIEWPORT
   m_viewMatrix.setTranslation( -m_centerX, -m_centerY, 0.0 );
#endif // NEWVIEWPORT

   if ( m_viewDirection == ViewOrtho )
   {
      //m_viewMatrix.setRotationInDegrees( m_arcballPoint[0], m_arcballPoint[1], m_arcballPoint[2] );
   }

   m_invMatrix = m_viewMatrix.getInverse();

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );
}

void ModelViewport::setViewportOverlay()
{
   makeCurrent();

   m_inOverlay = true;

   glViewport( 0, 0, ( GLint ) m_viewportWidth, ( GLint ) m_viewportHeight );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity( );

   glOrtho( 0, this->width(), 
         0, this->height(), 
         m_nearOrtho, m_farOrtho );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );
}

void ModelViewport::wheelEvent( QWheelEvent * e )
{
   if ( e->delta() > 0 )
   {
      if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
      {
         rotateClockwise();
      }
      else
      {
         zoomIn();
      }
   }
   else
   {
      if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
      {
         rotateCounterClockwise();
      }
      else
      {
         zoomOut();
      }
   }
}

void ModelViewport::zoomIn()
{
   if ( m_activeButton == Qt::NoButton )
   {
      if ( (m_zoomLevel * VP_ZOOMSCALE) > 0.0001 )
      {
         m_zoomLevel *= (VP_ZOOMSCALE);
      }

      QString zoomStr;
      zoomStr.sprintf( "%f", m_zoomLevel );
      emit zoomLevelChanged( zoomStr );

      makeCurrent();
      adjustViewport();
   }
}

void ModelViewport::zoomOut()
{
   if ( m_activeButton == Qt::NoButton )
   {
      if ( (m_zoomLevel / VP_ZOOMSCALE) < 250000 )
      {
         m_zoomLevel /= VP_ZOOMSCALE;
      }

      QString zoomStr;
      zoomStr.sprintf( "%f", m_zoomLevel );
      emit zoomLevelChanged( zoomStr );

      makeCurrent();
      adjustViewport();
   }
}

void ModelViewport::scrollUp()
{
   m_centerY += m_zoomLevel * 0.10f;
   m_arcballPoint[1] += m_zoomLevel * 0.10f;
   makeCurrent();
   adjustViewport();
}

void ModelViewport::scrollDown()
{
   m_centerY -= m_zoomLevel * 0.10f;
   m_arcballPoint[1] -= m_zoomLevel * 0.10f;
   makeCurrent();
   adjustViewport();
}

void ModelViewport::scrollLeft()
{
   m_centerX -= m_zoomLevel * 0.10f;
   m_arcballPoint[0] -= m_zoomLevel * 0.10f;
   makeCurrent();
   adjustViewport();
}

void ModelViewport::scrollRight()
{
   m_centerX += m_zoomLevel * 0.10f;
   m_arcballPoint[0] += m_zoomLevel * 0.10f;
   makeCurrent();
   adjustViewport();
}

void ModelViewport::rotateUp()
{
   rotateViewport( -15.0 * PIOVER180, 0.0 );
   makeCurrent();
   adjustViewport();
}

void ModelViewport::rotateDown()
{
   rotateViewport( 15.0 * PIOVER180, 0.0 );
   makeCurrent();
   adjustViewport();
}

void ModelViewport::rotateLeft()
{
   rotateViewport( 0.0, -15.0 * PIOVER180 );
   makeCurrent();
   adjustViewport();
}

void ModelViewport::rotateRight()
{
   rotateViewport( 0.0, 15.0 * PIOVER180 );
   makeCurrent();
   adjustViewport();
}

void ModelViewport::rotateClockwise()
{
   rotateViewport( 0.0, 0.0, -15.0 * PIOVER180 );
   makeCurrent();
   adjustViewport();
}

void ModelViewport::rotateCounterClockwise()
{
   rotateViewport( 0.0, 0.0, 15.0 * PIOVER180 );
   makeCurrent();
   adjustViewport();
}

void ModelViewport::mousePressEvent( QMouseEvent * e )
{
   //printf( "press = %d\n", e->button() );
   if ( m_activeButton != Qt::NoButton )
   {
      e->ignore();
      return;
   }

   e->accept();
   m_activeButton = e->button();

   m_operation = MO_None;

   int w = this->width();
   int h = this->height();

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

         if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
         {
            m_operation = ( m_overlayButton == ScrollButtonPan )
               ? MO_Rotate : MO_RotateButton;
         }
         else
         {
            m_operation = ( m_overlayButton == ScrollButtonPan )
               ? MO_Pan : MO_PanButton;
         }

         switch ( m_overlayButton )
         {
            case ScrollButtonPan:
               m_scrollStartPosition = e->pos();
               break;
            case ScrollButtonUp:
               if ( m_operation == MO_RotateButton )
                  rotateUp();
               else 
                  scrollUp();
               m_scrollTimer->start( 300, true );
               break;
            case ScrollButtonDown:
               if ( m_operation == MO_RotateButton )
                  rotateDown();
               else 
                  scrollDown();
               m_scrollTimer->start( 300, true );
               break;
            case ScrollButtonLeft:
               if ( m_operation == MO_RotateButton )
                  rotateLeft();
               else 
                  scrollLeft();
               m_scrollTimer->start( 300, true );
               break;
            case ScrollButtonRight:
               if ( m_operation == MO_RotateButton )
                  rotateRight();
               else 
                  scrollRight();
               m_scrollTimer->start( 300, true );
               break;
            default:
               break;
         }
      }
   }

   if ( m_operation == MO_None )
   {
      if ( e->button() == MidButton ) 
      {
         m_operation = MO_Pan;
         m_scrollStartPosition = e->pos();
      }
      else if ( (m_viewDirection == ViewPerspective && e->button() == LeftButton) 
            || (e->state() & Qt::ControlButton) )
      {
         m_operation = MO_Rotate;
         m_scrollStartPosition = e->pos();
      }
      else if ( m_viewDirection != ViewPerspective )
      {
         m_operation = MO_Tool;

         int button = constructButtonState( e );

         ::Tool * tool = m_toolbox->getCurrentTool();
         tool->mouseButtonDown( this, button, e->pos().x(), e->pos().y() );
      }
   }
   /*
   if ( m_overlayButton == ScrollButtonMAX )
   {
      if ( e->button() == MidButton || (m_viewDirection == ViewPerspective && e->button() == LeftButton) )
      {
         m_scrollStartPosition = e->pos();
      }
      else
      {
         int button = constructButtonState( e );

         ::Tool * tool = m_toolbox->getCurrentTool();
         tool->mouseButtonDown( this, button, e->pos().x(), e->pos().y() );
      }
   }
   */
}

void ModelViewport::mouseReleaseEvent( QMouseEvent * e )
{
   //printf( "release = %d\n", e->button() );
   if ( e->button() == m_activeButton )
   {
      if ( m_overlayButton == ScrollButtonMAX )
      {
         e->accept();
         if ( m_operation == MO_Tool )
         {
            int button = constructButtonState( e );

            ::Tool * tool = m_toolbox->getCurrentTool();
            tool->mouseButtonUp( this, button, e->pos().x(), e->pos().y() );
            m_model->operationComplete( tool->getName( 0 ) );

            if ( m_model->getSelectedProjectionCount() > 0 )
            {
               m_model->setDrawProjections( true );
               updateView();
            }

            if ( m_model->getSelectedBoneJointCount() > 0 )
            {
               m_model->setDrawJoints( 
                     static_cast<Model::DrawJointModeE>( g_prefs( "ui_draw_joints" ).intValue() ) );
               updateView();
            }
         }
      }
      else
      {
         m_overlayButton = ScrollButtonMAX;
         m_scrollTimer->stop();

         model_status( m_model, StatusNormal, STATUSTIME_SHORT, tr("Use the middle mouse button to drag/pan the viewport").utf8() );
      }
      m_activeButton = NoButton;
      m_operation    = MO_None;
   }
}

void ModelViewport::mouseMoveEvent( QMouseEvent * e )
{
   e->accept();

   if ( ! this->hasFocus() )
   {
      this->setFocus();
   }

   if ( m_viewDirection != ViewPerspective )
   {
      int x = e->pos().x();
      int y = e->pos().y();

      /*
      double xcoord = 0.0;
      double ycoord = 0.0;
      double zcoord = 0.0;

      getXValue( x, y, &xcoord );
      getYValue( x, y, &ycoord );
      getZValue( x, y, &zcoord );
      */

      Vector pos;

      getParentXYValue( x, y, pos[0], pos[1], true );

      m_invMatrix.apply( pos );

      for ( int i = 0; i < 3; i++ )
      {
         if ( fabs(pos[i]) < 0.000001 ) 
            pos[i] = 0.0;
      }

      char str[80];
      PORT_snprintf( str, sizeof(str), "%g, %g, %g", pos[0], pos[1], pos[2] );
      model_status( m_model, StatusNormal, STATUSTIME_NONE, str );
   }
   else
   {
      model_status( m_model, StatusNormal, STATUSTIME_NONE, "" );
   }

   if ( m_operation == MO_Rotate )
   {
      QPoint curPos = e->pos();

      double xDiff = -(m_scrollStartPosition.x() - curPos.x());
      double yDiff = -(m_scrollStartPosition.y() - curPos.y());

      double rotY = (double) xDiff / (double) this->width()  * 3.14159 * 2.0;
      double rotX = (double) yDiff / (double) this->height() * 3.14159 * 2.0;

      rotateViewport( rotX, rotY );

      m_scrollStartPosition = curPos;
   }
   else if ( m_operation == MO_Pan )
   {
#ifdef NEWVIEWPORT
      //printf( "adjusting translation\n" );
      QPoint curPos = e->pos();

      double xDiff = m_scrollStartPosition.x() - curPos.x();
      xDiff = xDiff * (m_width / m_viewportWidth);
      m_centerX += xDiff;

      double yDiff = m_scrollStartPosition.y() - curPos.y();
      yDiff = -yDiff;  // Adjust for difference between pixel and GL coordinates
      yDiff = yDiff * (m_height / m_viewportHeight);
      m_centerY += yDiff;

      //m_viewMatrix.inverseRotateVector( vec );
      m_arcballPoint[0] += xDiff;
      m_arcballPoint[1] += yDiff;

      /*
         double vec[3];
         vec[0] = xDiff;
         vec[1] = yDiff;
         vec[2] = 0.0;

         vec[0] = m_arcballPoint[0];
         vec[1] = m_arcballPoint[1];
         vec[2] = m_arcballPoint[2];

         m_viewMatrix.inverseRotateVector( vec );

         log_debug( "World coords = %f,%f,%f\n",
         vec[0], vec[1], vec[2] );
         */

      m_scrollStartPosition = curPos;
#else
      //printf( "adjusting translation\n" );
      QPoint curPos = e->pos();

      double xDiff = m_scrollStartPosition.x() - curPos.x();
      m_centerX += xDiff * (m_width / m_viewportWidth);

      double yDiff = m_scrollStartPosition.y() - curPos.y();
      yDiff = -yDiff;  // Adjust for difference between pixel and GL coordinates
      m_centerY += yDiff * (m_height / m_viewportHeight);

      m_scrollStartPosition = curPos;
#endif // NEWVIEWPORT

      makeCurrent();
      adjustViewport();
   }
   else if ( m_operation == MO_Tool )
   {
      //printf( "tool mouse event\n" );
      int button = constructButtonState( e );

      ::Tool * tool = m_toolbox->getCurrentTool();
      tool->mouseButtonMove( this, button, e->pos().x(), e->pos().y() );
   }
#if 0
   if ( m_overlayButton == ScrollButtonMAX )
   {
      if ( m_activeButton )
      {
         if ( m_activeButton == MidButton || (m_viewDirection == ViewPerspective && m_activeButton == LeftButton) )
         {
            if ( m_viewDirection == ViewPerspective && (m_activeButton == LeftButton) )
            {
               QPoint curPos = e->pos();

               double xDiff = -(m_scrollStartPosition.x() - curPos.x());
               double yDiff = -(m_scrollStartPosition.y() - curPos.y());

               Matrix mcur;
               Matrix mcurinv;
               double rot[3];
               rot[0] = m_rotX * PIOVER180;
               rot[1] = m_rotY * PIOVER180;
               rot[2] = m_rotZ * PIOVER180;
               mcur.setRotation( rot );
               mcurinv = mcur.getInverse();

#ifdef NEWVIEWPORT
               mcur.inverseRotateVector( m_arcballPoint );
#endif // NEWVIEWPORT

               Vector yvec;
               yvec.set( 0, 0.0 );
               yvec.set( 1, 1.0 );
               yvec.set( 2, 0.0 );
               yvec.set( 3, 0.0 );

               Vector xvec;
               xvec.set( 0, 1.0 );
               xvec.set( 1, 0.0 );
               xvec.set( 2, 0.0 );
               xvec.set( 3, 0.0 );

               Matrix mx;
               Matrix my;
               double rotY = (double) xDiff / (double) this->width()  * 3.14159 * 2.0;
               double rotX = (double) yDiff / (double) this->height() * 3.14159 * 2.0;

               yvec = yvec * mcurinv;
               my.setRotationOnAxis( yvec.getVector(), rotY );

               xvec = xvec * mcurinv;
               mx.setRotationOnAxis( xvec.getVector(), rotX );

               mcur = mx * mcur;
               mcur = my * mcur;
               mcur.getRotation( rot );
               m_rotX = rot[0] / PIOVER180;
               m_rotY = rot[1] / PIOVER180;
               m_rotZ = rot[2] / PIOVER180;

               //m_rotY += xDiff * PIOVER180 * 14.0;
               //m_rotX += yDiff * PIOVER180 * 14.0;

#ifdef NEWVIEWPORT
               mcur.apply3( m_arcballPoint );
#endif // NEWVIEWPORT

               m_scrollStartPosition = curPos;

               // And finally, update the view
               makeCurrent();
               adjustViewport();
            }
            else
            {
#ifdef NEWVIEWPORT
               //printf( "adjusting translation\n" );
               QPoint curPos = e->pos();

               double xDiff = m_scrollStartPosition.x() - curPos.x();
               xDiff = xDiff * (m_width / m_viewportWidth);
               m_centerX += xDiff;

               double yDiff = m_scrollStartPosition.y() - curPos.y();
               yDiff = -yDiff;  // Adjust for difference between pixel and GL coordinates
               yDiff = yDiff * (m_height / m_viewportHeight);
               m_centerY += yDiff;

               //m_viewMatrix.inverseRotateVector( vec );
               m_arcballPoint[0] += xDiff;
               m_arcballPoint[1] += yDiff;

               /*
               double vec[3];
               vec[0] = xDiff;
               vec[1] = yDiff;
               vec[2] = 0.0;

               vec[0] = m_arcballPoint[0];
               vec[1] = m_arcballPoint[1];
               vec[2] = m_arcballPoint[2];

               m_viewMatrix.inverseRotateVector( vec );

               log_debug( "World coords = %f,%f,%f\n",
                     vec[0], vec[1], vec[2] );
               */

               m_scrollStartPosition = curPos;
#else
               //printf( "adjusting translation\n" );
               QPoint curPos = e->pos();

               double xDiff = m_scrollStartPosition.x() - curPos.x();
               m_centerX += xDiff * (m_width / m_viewportWidth);

               double yDiff = m_scrollStartPosition.y() - curPos.y();
               yDiff = -yDiff;  // Adjust for difference between pixel and GL coordinates
               m_centerY += yDiff * (m_height / m_viewportHeight);

               m_scrollStartPosition = curPos;
#endif // NEWVIEWPORT

               makeCurrent();
               adjustViewport();
            }
         }
         else
         {
            //printf( "tool mouse event\n" );
            int button = constructButtonState( e );

            ::Tool * tool = m_toolbox->getCurrentTool();
            tool->mouseButtonMove( this, button, e->pos().x(), e->pos().y() );
         }
      }
   }
   else
   {
      if ( m_overlayButton == ScrollButtonPan )
      {
         //printf( "adjusting translation\n" );
         QPoint curPos = e->pos();

         double xDiff = m_scrollStartPosition.x() - curPos.x();
         xDiff = xDiff * (m_width / m_viewportWidth);
         m_centerX += xDiff;

         double yDiff = m_scrollStartPosition.y() - curPos.y();
         yDiff = -yDiff;  // Adjust for difference between pixel and GL coordinates
         yDiff = yDiff * (m_height / m_viewportHeight);
         m_centerY += yDiff;

         m_arcballPoint[0] += xDiff;
         m_arcballPoint[1] += yDiff;

         m_scrollStartPosition = curPos;

         makeCurrent();
         adjustViewport();
      }
   }
#endif // 0
}

void ModelViewport::keyPressEvent( QKeyEvent * e )
{
   if ( m_activeButton == Qt::NoButton )
   {
      switch ( e->key() )
      {
         case Key_Equal:
         case Key_Plus:
            {
               if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
               {
                  rotateClockwise();
               }
               else
               {
                  zoomIn();
               }
            }
            break;
         case Key_Minus:
         case Key_Underscore:
            {
               if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
               {
                  rotateCounterClockwise();
               }
               else
               {
                  zoomOut();
               }
            }
            break;
         case Key_QuoteLeft:
            {
               int newDir = 0;
               switch ( m_viewDirection )
               {
                  case ViewPerspective:
                     newDir = ViewOrtho;
                     break;
                  case ViewFront:
                  case ViewBack:
                  case ViewRight:
                  case ViewLeft:
                  case ViewTop:
                  case ViewBottom:
                  case ViewOrtho:
                     newDir = ViewPerspective;
                     break;
                  default:
                     break;
               }

               emit viewDirectionChanged( newDir );
            }
            break;
         case Key_Backslash:
            {
               int newDir = 0;
               switch ( m_viewDirection )
               {
                  case ViewFront:
                     newDir = ViewBack;
                     break;
                  case ViewBack:
                     newDir = ViewFront;
                     break;
                  case ViewRight:
                     newDir = ViewLeft;
                     break;
                  case ViewLeft:
                     newDir = ViewRight;
                     break;
                  case ViewTop:
                     newDir = ViewBottom;
                     break;
                  case ViewBottom:
                     newDir = ViewTop;
                     break;
                  case ViewPerspective:
                     newDir = ViewOrtho;
                     break;
                  case ViewOrtho:
                     newDir = ViewPerspective;
                     break;
                  default:
                     break;
               }

               emit viewDirectionChanged( newDir );
            }
            break;
         case Key_0:
            m_centerX = 0.0;
            m_centerY = 0.0;
            m_arcballPoint[0] = 0.0;
            m_arcballPoint[1] = 0.0;
            m_arcballPoint[2] = 0.0;
            makeCurrent();
            adjustViewport();
            break;
         case Key_Up:
            if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
               rotateUp();
            else
               scrollUp();
            break;
         case Key_Down:
            if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
               rotateDown();
            else
               scrollDown();
            break;
         case Key_Left:
            if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
               rotateLeft();
            else
               scrollLeft();
            break;
         case Key_Right:
            if ( (e->state() & Qt::ControlButton) == Qt::ControlButton )
               rotateRight();
            else
               scrollRight();
            break;
         case Key_1:
         case Key_2:
         case Key_3:
         case Key_4:
         case Key_5:
         case Key_6:
         case Key_7:
         case Key_8:
         case Key_9:
#ifdef HAVE_QT4
            if ( e->modifiers() & ControlModifier )
#else
            if ( e->state() & ControlButton )
#endif // HAVE_QT4
            {
               log_debug( "set viewport %d\n", e->key() - Key_1 );

               int slot = ( (int) e->key() - (int) Key_1 );
               ViewStateT viewState;

               viewState.direction = m_viewDirection;
               viewState.zoom = m_zoomLevel;
               viewState.rotation[0] = m_rotX;
               viewState.rotation[1] = m_rotY;
               viewState.rotation[2] = m_rotZ;
               viewState.translation[0] = m_arcballPoint[0];
               viewState.translation[1] = m_arcballPoint[1];
               viewState.translation[2] = m_arcballPoint[2];

               emit viewportSaveState( slot, viewState );
            }
            else
            {
               log_debug( "viewport recall %d\n", e->key() - Key_1 );
               int slot = ( (int) e->key() - (int) Key_1 );
               emit viewportRecallState( slot );
               return;
            }
            break;

         default:
            QGLWidget::keyPressEvent( e );
            return;
            break;
      }
   }

   e->accept();
}

void ModelViewport::rotateViewport( double rotX, double rotY, double rotZ )
{
   if ( fabs( rotX ) >0.00001 || fabs( rotY ) > 0.00001 || fabs( rotZ ) > 0.00001 )
   {
      if ( m_viewDirection != ViewPerspective && m_viewDirection != ViewOrtho )
      {
         emit viewDirectionChanged( ViewOrtho );
      }

      Matrix mcur;
      Matrix mcurinv;
      double rot[3];
      rot[0] = m_rotX * PIOVER180;
      rot[1] = m_rotY * PIOVER180;
      rot[2] = m_rotZ * PIOVER180;
      mcur.setRotation( rot );
      mcurinv = mcur.getInverse();

#ifdef NEWVIEWPORT
      mcur.inverseRotateVector( m_arcballPoint );
#endif // NEWVIEWPORT

      Vector yvec;
      yvec.set( 0, 0.0 );
      yvec.set( 1, 1.0 );
      yvec.set( 2, 0.0 );
      yvec.set( 3, 0.0 );

      Vector xvec;
      xvec.set( 0, 1.0 );
      xvec.set( 1, 0.0 );
      xvec.set( 2, 0.0 );
      xvec.set( 3, 0.0 );

      Vector zvec;
      zvec.set( 0, 0.0 );
      zvec.set( 1, 0.0 );
      zvec.set( 2, 1.0 );
      zvec.set( 3, 0.0 );

      Matrix mx;
      Matrix my;
      Matrix mz;

      zvec = zvec * mcurinv;
      mz.setRotationOnAxis( zvec.getVector(), rotZ );

      yvec = yvec * mcurinv;
      my.setRotationOnAxis( yvec.getVector(), rotY );

      xvec = xvec * mcurinv;
      mx.setRotationOnAxis( xvec.getVector(), rotX );

      mcur = mx * mcur;
      mcur = my * mcur;
      mcur = mz * mcur;
      mcur.getRotation( rot );
      m_rotX = rot[0] / PIOVER180;
      m_rotY = rot[1] / PIOVER180;
      m_rotZ = rot[2] / PIOVER180;

      //m_rotY += xDiff * PIOVER180 * 14.0;
      //m_rotX += yDiff * PIOVER180 * 14.0;

#ifdef NEWVIEWPORT
      mcur.apply3( m_arcballPoint );
#endif // NEWVIEWPORT

      // And finally, update the view
      makeCurrent();
      adjustViewport();
   }
}

void ModelViewport::setViewState( const ViewStateT & viewState )
{
   m_viewDirection = viewState.direction;
   m_zoomLevel = viewState.zoom;
   m_rotX = viewState.rotation[0];
   m_rotY = viewState.rotation[1];
   m_rotZ = viewState.rotation[2];
   m_arcballPoint[0] = viewState.translation[0];
   m_arcballPoint[1] = viewState.translation[1];
   m_arcballPoint[2] = viewState.translation[2];

   makeCurrent();
   adjustViewport();
}

void ModelViewport::viewChangeEvent( int dir )
{
   log_debug( "viewChangeEvent( %d )\n", dir );

   if ( dir == m_viewDirection )
   {
      return;
   }

   if ( m_viewDirection == ViewPerspective ) //|| m_viewDirection == ViewOrtho )
   {
      Matrix m;
      m.setRotationInDegrees( 0.0f, 0.0f, -m_rotZ );
      m.apply3( m_arcballPoint );
      m.setRotationInDegrees( 0.0f, -m_rotY, 0.0f );
      m.apply3( m_arcballPoint );
      m.setRotationInDegrees( -m_rotX, 0.0f, 0.0f );
      m.apply3( m_arcballPoint );
   }
   else
   {
      m_invMatrix.apply3( m_arcballPoint );
   }
   log_debug( "center point = %f,%f,%f\n",
         m_arcballPoint[0],
         m_arcballPoint[1],
         m_arcballPoint[2] );

   bool toFree = false;

   switch ( m_viewDirection )
   {
      case ViewFront:
      case ViewBack:
      case ViewRight:
      case ViewLeft:
      case ViewTop:
      case ViewBottom:
         if ( dir == ViewPerspective || dir == ViewOrtho )
         {
            toFree = true;
         }
         break;
      default:
         break;
   }

   if ( toFree )
   {
      log_debug( "inverting rotation\n" );
      m_rotX = -m_rotX;
      m_rotY = -m_rotY;
      m_rotZ = -m_rotZ;
   }
   else
   {
      switch ( dir )
      {
         case ViewFront:
            m_rotX = 0;
            m_rotY = 0;
            m_rotZ = 0;
            break;
         case ViewBack:
            m_rotX = 0;
            m_rotY = 180;
            m_rotZ = 0;
            break;
         case ViewLeft:
            m_rotX = 0;
            m_rotY = 90;
            m_rotZ = 0;
            break;
         case ViewRight:
            m_rotX = 0;
            m_rotY = -90;
            m_rotZ = 0;
            break;
         case ViewTop:
            m_rotX = -90;
            m_rotY = 0;
            m_rotZ = 0;
            break;
         case ViewBottom:
            m_rotX = 90;
            m_rotY = 0;
            m_rotZ = 0;
            break;
         default:
            break;
      }
   }

   Matrix m;
   m.loadIdentity();
   if ( false
         || (m_viewDirection == ViewPerspective && dir == ViewOrtho) 
         || (m_viewDirection == ViewOrtho && dir == ViewPerspective) 
         )
   {
      m.setRotationInDegrees( m_rotX, m_rotY, m_rotZ );
   }
   else
   {
      m.setRotationInDegrees( -m_rotX, -m_rotY, -m_rotZ );
   }

   if ( toFree )
   {
      m = m.getInverse();
   }

   m.apply3( m_arcballPoint );
   log_debug( "after point = %f,%f,%f\n",
         m_arcballPoint[0],
         m_arcballPoint[1],
         m_arcballPoint[2] );

   m_viewDirection = static_cast<ViewDirectionE>( dir );
   makeCurrent();
   adjustViewport();
}

void ModelViewport::setZoomLevel( double zoom )
{
   if ( m_activeButton == Qt::NoButton )
   {
      m_zoomLevel = zoom;
      makeCurrent();
      adjustViewport();

      QString zoomStr;
      zoomStr.sprintf( "%f", m_zoomLevel );
      emit zoomLevelChanged( zoomStr );
   }
}

void ModelViewport::wireframeEvent()
{
   m_viewOptions = ViewWireframe;
   updateGL();
}

void ModelViewport::flatEvent()
{
   m_viewOptions = ViewFlat;
   updateGL();
}

void ModelViewport::smoothEvent()
{
   m_viewOptions = ViewSmooth;
   updateGL();
}

void ModelViewport::textureEvent()
{
   m_viewOptions = ViewTexture;
   updateGL();
}

void ModelViewport::alphaEvent()
{
   m_viewOptions = ViewAlpha;
   updateGL();
}

void ModelViewport::scrollTimeout()
{
   if ( m_operation == MO_RotateButton )
   {
      switch ( m_overlayButton )
      {
         case ScrollButtonUp:
            rotateUp();
            break;
         case ScrollButtonDown:
            rotateDown();
            break;
         case ScrollButtonLeft:
            rotateLeft();
            break;
         case ScrollButtonRight:
            rotateRight();
            break;
         default:
            m_scrollTimer->stop();
            return;
      }
   }
   else
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
   }

   m_scrollTimer->start( 100 );
}

void ModelViewport::focusInEvent( QFocusEvent * e )
{
   m_backColor.setRgb( 150, 210, 210 );
   makeCurrent();
   glClearColor( m_backColor.red() / 256.0, 
         m_backColor.green() / 256.0, 
         m_backColor.blue() / 256.0, 1.0f );
   updateGL();
}

void ModelViewport::focusOutEvent( QFocusEvent * e )
{
   m_backColor.setRgb( 130, 200, 200 );
   makeCurrent();
   glClearColor( m_backColor.red() / 256.0, 
         m_backColor.green() / 256.0, 
         m_backColor.blue() / 256.0, 1.0f );
   updateGL();
}

/*
void ModelViewport::dragEnterEvent( QDragMoveEvent * e )
{
   log_debug( "got drag enter event\n" );
   if ( QUriDrag::canDecode( e ) )
   {
      log_debug( "is URI\n" );
   }
   if ( QTextDrag::canDecode( e ) )
   {
      log_debug( "is Text\n" );
   }
   if ( QImageDrag::canDecode( e ) )
   {
      log_debug( "is Image\n" );
   }
   //e->accept( QRect( 0, 0, this->width(), this->height() ) );
}
*/

void ModelViewport::getParentXYValue( int x, int y, double & xval, double & yval, bool selected )
{
   xval = (((double) x / (double) m_viewportWidth)  * m_width)
      - (m_width / 2.0);
   yval = (((double) -y / (double) m_viewportHeight) * m_height)
      + (m_height / 2.0);

   double maxDist = (4.1 / (double) m_viewportWidth) * m_width;

   if ( g_prefs.exists( "ui_snap_vertex" ) 
         &&  g_prefs( "ui_snap_vertex" ).intValue() != 0 )
   {
      // snap to vertex

      double curDist = maxDist;
      int curIndex = -1;
      Model::PositionTypeE curType = Model::PT_Vertex;
      const Matrix & mat = getParentViewMatrix();
      double coord[3] = { 0, 0, 0 };
      double saveCoord[3] = { 0, 0, 0 };

      size_t vcount = m_model->getVertexCount();
      for ( size_t v = 0; v < vcount; v++ )
      {
         if ( selected || !m_model->isVertexSelected( v ) )
         {
            m_model->getVertexCoords( v, coord );

            mat.apply3( coord );
            coord[0] += mat.get(3,0);
            coord[1] += mat.get(3,1);
            coord[2] += mat.get(3,2);

            double xdiff = coord[0] - xval;
            double ydiff = coord[1] - yval;
            double diff = sqrt( xdiff*xdiff + ydiff*ydiff );

            if ( diff < curDist )
            {
               curDist = diff;
               curIndex = v;
               curType  = Model::PT_Vertex;
               saveCoord[0] = coord[0];
               saveCoord[1] = coord[1];
               saveCoord[2] = coord[2];
            }
         }
      }

      size_t bcount = m_model->getBoneJointCount();
      for ( size_t b = 0; b < bcount; b++ )
      {
         if ( selected || !m_model->isBoneJointSelected( b ) )
         {
            m_model->getBoneJointCoords( b, coord );

            mat.apply3( coord );
            coord[0] += mat.get(3,0);
            coord[1] += mat.get(3,1);
            coord[2] += mat.get(3,2);

            double xdiff = coord[0] - xval;
            double ydiff = coord[1] - yval;
            double diff = sqrt( xdiff*xdiff + ydiff*ydiff );

            if ( diff < curDist )
            {
               curDist = diff;
               curIndex = b;
               curType  = Model::PT_Joint;
               saveCoord[0] = coord[0];
               saveCoord[1] = coord[1];
               saveCoord[2] = coord[2];
            }
         }
      }

      size_t p;
      size_t pcount = m_model->getPointCount();

      for ( p = 0; p < pcount; p++ )
      {
         if ( selected || !m_model->isPointSelected( p ) )
         {
            m_model->getPointCoords( p, coord );

            mat.apply3( coord );
            coord[0] += mat.get(3,0);
            coord[1] += mat.get(3,1);
            coord[2] += mat.get(3,2);

            double xdiff = coord[0] - xval;
            double ydiff = coord[1] - yval;
            double diff = sqrt( xdiff*xdiff + ydiff*ydiff );

            if ( diff < curDist )
            {
               curDist = diff;
               curIndex = p;
               curType  = Model::PT_Point;
               saveCoord[0] = coord[0];
               saveCoord[1] = coord[1];
               saveCoord[2] = coord[2];
            }
         }
      }

      pcount = m_model->getProjectionCount();

      for ( p = 0; p < pcount; p++ )
      {
         if ( selected || !m_model->isProjectionSelected( p ) )
         {
            m_model->getProjectionCoords( p, coord );

            mat.apply3( coord );
            coord[0] += mat.get(3,0);
            coord[1] += mat.get(3,1);
            coord[2] += mat.get(3,2);

            double xdiff = coord[0] - xval;
            double ydiff = coord[1] - yval;
            double diff = sqrt( xdiff*xdiff + ydiff*ydiff );

            if ( diff < curDist )
            {
               curDist = diff;
               curIndex = p;
               curType  = Model::PT_Projection;
               saveCoord[0] = coord[0];
               saveCoord[1] = coord[1];
               saveCoord[2] = coord[2];
            }
         }
      }

      if ( curIndex >= 0 )
      {
         xval = saveCoord[0];
         yval = saveCoord[1];
         maxDist = 0.0;
      }
   }

   if ( m_viewDirection != ViewOrtho
         && g_prefs.exists( "ui_snap_grid" ) 
         && g_prefs( "ui_snap_grid" ).intValue() != 0 )
   {
      // snap to grid

      double unitWidth = 1.0;
      double maxDimension = (m_width > m_height) ? m_width : m_height;

      unitWidth = g_prefs( "ui_grid_inc" ).doubleValue();

      while ( (maxDimension / unitWidth) > 16 )
      {
         unitWidth *= 2.0;
      }
      while ( (maxDimension / unitWidth) < 4 )
      {
         unitWidth /= 2.0;
      }

      double val;
      int mult;
      double fudge;

      fudge = 0.5;
      
#ifdef NEWVIEWPORT
      double x = xval + m_arcballPoint[0];
#else // NEWVIEWPORT
      double x = xval + m_centerX;
#endif // NEWVIEWPORT

      if ( x < 0.0 )
      {
         fudge = -0.5;
      }

      mult = (int) (x / unitWidth + fudge);
      val = (double) mult * unitWidth;

      if ( fabs( x - val ) < maxDist )
      {
#ifdef NEWVIEWPORT
         xval = val - m_arcballPoint[0];
#else // NEWVIEWPORT
         xval = val - m_centerX;
#endif // NEWVIEWPORT
      }

      fudge = 0.5;
      
#ifdef NEWVIEWPORT
      double y = yval + m_arcballPoint[1];
#else // NEWVIEWPORT
      double y = yval + m_centerY;
#endif // NEWVIEWPORT

      if ( y < 0.0 )
      {
         fudge = -0.5;
      }

      mult = (int) (y / unitWidth + fudge);
      val = (double) mult * unitWidth;

      if ( fabs( y - val ) < maxDist )
      {
#ifdef NEWVIEWPORT
         yval = val - m_arcballPoint[1];
#else // NEWVIEWPORT
         yval = val - m_centerY;
#endif // NEWVIEWPORT
      }
   }
}

void ModelViewport::getRawParentXYValue( int x, int y, double & xval, double & yval )
{
   xval = (((double) x / (double) m_viewportWidth)  * m_width)
      - (m_width / 2.0);
   yval = (((double) -y / (double) m_viewportHeight) * m_height)
      + (m_height / 2.0);
}

#ifdef NEWVIEWPORT
bool ModelViewport::getXValue( int x, int y, double * val )
{
   Vector vec;
   vec[0] =  (((double) x / (double) m_viewportWidth)  * m_width)  - (m_width  / 2.0);
   vec[1] = -((((double) y / (double) m_viewportHeight) * m_height) - (m_height / 2.0));
   vec[2] = 0.0;

   m_invMatrix.apply( vec );

   *val = vec[0];

   switch ( m_viewDirection )
   {
      case ViewFront:
      case ViewTop:
      case ViewBottom:
      case ViewBack:
      case ViewOrtho:
         return true;
      default:
         break;
   }

   return false;
}

bool ModelViewport::getYValue( int x, int y, double * val )
{
   Vector vec;
   vec[0] =  (((double) x / (double) m_viewportWidth)  * m_width)  - (m_width  / 2.0);
   vec[1] = -((((double) y / (double) m_viewportHeight) * m_height) - (m_height / 2.0));
   vec[2] = 0.0;

   m_invMatrix.apply( vec );

   *val = vec[1];

   switch ( m_viewDirection )
   {
      case ViewFront:
      case ViewBack:
      case ViewLeft:
      case ViewRight:
      case ViewOrtho:
         return true;
      default:
         break;
   }

   return false;
}

bool ModelViewport::getZValue( int x, int y, double * val )
{
   Vector vec;
   vec[0] =  (((double) x / (double) m_viewportWidth)  * m_width)  - (m_width  / 2.0);
   vec[1] = -((((double) y / (double) m_viewportHeight) * m_height) - (m_height / 2.0));
   vec[2] = 0.0;

   m_invMatrix.apply( vec );

   *val = vec[2];

   switch ( m_viewDirection )
   {
      case ViewLeft:
      case ViewRight:
      case ViewTop:
      case ViewBottom:
      case ViewOrtho:
         return true;
      default:
         break;
   }

   return false;
}

#else // NEWVIEWPORT
bool ModelViewport::getXValue( int x, int y, double * val )
{
   switch ( m_viewDirection )
   {
      case ViewFront:
      case ViewTop:
      case ViewBottom:
         *val = (m_centerX - (m_width / 2.0)) + (((double) x / (double) m_viewportWidth) * m_width);
         break;
      case ViewBack:
         *val = ((-m_centerX) + (m_width / 2.0)) + (((double) (-x) / (double) m_viewportWidth) * m_width);
         break;
      case ViewRight:
      case ViewLeft:
      default:
         return false;
   }

   return true;
}

bool ModelViewport::getYValue( int x, int y, double * val )
{
   switch ( m_viewDirection )
   {
      case ViewFront:
      case ViewBack:
      case ViewRight:
      case ViewLeft:
         *val = (m_centerY + (m_height / 2.0)) - (((double) y / (double) m_viewportHeight) * m_height);
         break;
      case ViewTop:
      case ViewBottom:
      default:
         return false;
         break;
   }

   return true;
}

bool ModelViewport::getZValue( int x, int y, double * val )
{
   switch ( m_viewDirection )
   {
      case ViewTop:
         *val = ((-m_centerY) - (m_height / 2.0)) - (((double) (-y) / (double) m_viewportHeight) * m_height);
         break;
      case ViewBottom:
         *val = (m_centerY + (m_height / 2.0)) - (((double) y / (double) m_viewportHeight) * m_height);
         //*val = (m_centerY + (m_height / 2.0)) - (((double) y / (double) m_viewportHeight) * m_height);
         break;
      case ViewRight:
         *val = (m_centerX - (m_width / 2.0)) + (((double) x / (double) m_viewportWidth) * m_width);
         break;
      case ViewLeft:
         *val = ((-m_centerX) + (m_width / 2.0)) + (((double) (-x) / (double) m_viewportWidth) * m_width);
         break;
      case ViewFront:
      case ViewBack:
      default:
         return false;
   }

   return true;
}

#endif // NEWVIEWPORT

int ModelViewport::constructButtonState( QMouseEvent * e )
{
   int button = 0;

   //switch ( e->button() )
   switch ( m_activeButton )
   {
      case LeftButton:
         button = ::Tool::BS_Left;
         break;
      case MidButton:
         button = ::Tool::BS_Middle;
         break;
      case RightButton:
         button = ::Tool::BS_Right;
         break;
      default:
         break;
   }

#ifdef HAVE_QT4
   if ( e->modifiers() & ShiftButton )
#else
   if ( e->state() & ShiftButton )
#endif // HAVE_QT4
   {
      button |= ::Tool::BS_Shift;
   }

#ifdef HAVE_QT4
   if ( e->modifiers() & AltButton )
#else
   if ( e->state() & AltButton )
#endif // HAVE_QT4
   {
      button |= ::Tool::BS_Alt;
   }

#ifdef HAVE_QT4
   if ( e->modifiers() & ControlButton )
#else
   if ( e->state() & ControlButton )
#endif // HAVE_QT4
   {
      button |= ::Tool::BS_Ctrl;
   }

   return button;
}

void ModelViewport::updateView()
{
   updateGL();
   StatusObject * bar = model_status_get_object( m_model );
   bar->setVertices(   m_model->getVertexCount(),    m_model->getSelectedVertexCount() );
   bar->setFaces(      m_model->getTriangleCount(),  m_model->getSelectedTriangleCount() );
   bar->setGroups(     m_model->getGroupCount() );
   bar->setBoneJoints( m_model->getBoneJointCount(), m_model->getSelectedBoneJointCount() );
   bar->setPoints(     m_model->getPointCount(),     m_model->getSelectedPointCount() );
   bar->setTextures(   m_model->getTextureCount() );
}

void ModelViewport::update3dView()
{
   if ( m_viewDirection == ViewPerspective )
   {
      updateView();
   }
}

void ModelViewport::addDecal( Decal * decal )
{
   m_decals.push_back( decal );
   updateGL();
}

void ModelViewport::removeDecal( Decal * decal )
{
   m_decals.remove( decal );
   updateGL();
}

void ModelViewport::frameArea( double x1, double y1, double z1, double x2, double y2, double z2 )
{
   double centerX = (x1 + x2) / 2.0;
   double centerY = (y1 + y2) / 2.0;
   double centerZ = (z1 + z2) / 2.0;

   double width  = fabs( x1 - x2 );
   double height = fabs( y1 - y2 );
   double depth  = fabs( z1 - z2 );

   if ( width < 0.0001667 )
   {
      width = 0.0001667;
   }
   if ( height < 0.0001667 )
   {
      height = 0.0001667;
   }
   if ( depth < 0.0001667 )
   {
      depth = 0.0001667;
   }

   width  *= 1.20;
   height *= 1.20;
   depth  *= 1.20;

   Vector bounds;
   bounds[0] = width;
   bounds[1] = height;
   bounds[2] = depth;

   double viewWidth   = 0.0;
   double viewHeight  = 0.0;

   switch ( m_viewDirection )
   {
#ifdef NEWVIEWPORT
      case ViewFront:
      case ViewBack:
      case ViewRight:
      case ViewLeft:
      case ViewTop:
      case ViewBottom:
      case ViewOrtho:
      case ViewPerspective:
         {
            m_arcballPoint[0] = centerX;
            m_arcballPoint[1] = centerY;
            m_arcballPoint[2] = centerZ;

            //log_debug( "view = %d (%f,%f,%f)\n",
            //      (int) m_viewDirection, 
            //      m_rotX, m_rotY, m_rotZ );

            double rx = m_rotX;
            double ry = m_rotY;
            double rz = m_rotZ;
            
            if ( m_viewDirection != ViewPerspective || m_viewDirection == ViewOrtho )
            {
               rx = -rx;
               ry = -ry;
               rz = -rz;
            }
            
            //m_viewMatrix.apply3( m_arcballPoint );
            if ( m_viewDirection == ViewOrtho )
            {
               Matrix m;
               m_viewMatrix.apply3( m_arcballPoint );
               m_viewMatrix.apply3( bounds );
               /*
               m.setRotationInDegrees(  rx, 0.0f, 0.0f );
               m.apply3( m_arcballPoint );
               m.apply3( bounds );
               m.setRotationInDegrees( 0.0f,  ry, 0.0f );
               m.apply3( m_arcballPoint );
               m.apply3( bounds );
               m.setRotationInDegrees( 0.0f, 0.0f,  rz );
               m.apply3( m_arcballPoint );
               m.apply3( bounds );
               */
            }
            else
            {
               Matrix m;
               m.setRotationInDegrees(  rx, 0.0f, 0.0f );
               m.apply3( m_arcballPoint );
               m.apply3( bounds );
               m.setRotationInDegrees( 0.0f,  ry, 0.0f );
               m.apply3( m_arcballPoint );
               m.apply3( bounds );
               m.setRotationInDegrees( 0.0f, 0.0f,  rz );
               m.apply3( m_arcballPoint );
               m.apply3( bounds );
            }

            if ( m_viewDirection == ViewPerspective || m_viewDirection == ViewOrtho )
            {
               viewWidth = viewHeight = sqrt( height*height + width*width + depth*depth ) * 1.2;
            }
            else
            {
               viewWidth  = fabs( bounds[0] );
               viewHeight = fabs( bounds[1] );
            }

            //log_debug( "point = %d (%f,%f,%f)\n",
            //      (int) m_viewDirection, 
            //      m_arcballPoint[0], m_arcballPoint[1], m_arcballPoint[2] );
         }
         break;
      default:
         return;
         break;
#else  // NEWVIEWPORT
      case ViewFront:
         m_centerX   = centerX;
         m_centerY   = centerY;
         viewWidth   = width;
         viewHeight  = height;
         break;
      case ViewBack:
         m_centerX   = -centerX;
         m_centerY   = centerY;
         viewWidth   = width;
         viewHeight  = height;
         break;
      case ViewRight:
         m_centerX   = centerZ;
         m_centerY   = centerY;
         viewWidth   = depth;
         viewHeight  = height;
         break;
      case ViewLeft:
         m_centerX   = -centerZ;
         m_centerY   = centerY;
         viewWidth   = depth;
         viewHeight  = height;
         break;
      case ViewTop:
         m_centerX   = centerX;
         m_centerY   = -centerZ;
         viewWidth   = width;
         viewHeight  = depth;
         break;
      case ViewBottom:
         m_centerX   = centerX;
         m_centerY   = centerZ;
         viewWidth   = width;
         viewHeight  = depth;
         break;
      case ViewPerspective:
         {
            double rotation[3];
            rotation[0] = m_rotX * PIOVER180;
            rotation[1] = m_rotY * PIOVER180;
            rotation[2] = m_rotZ * PIOVER180;
            Matrix m;
            m.setRotation( rotation );

            double v[3];
            v[0] = centerX;
            v[1] = centerY;
            v[2] = centerZ;

            m.inverseRotateVector( v );

            m_centerX   = v[0];
            m_centerY   = v[1];
            viewWidth   = height;
            viewHeight  = width;
         }
         break;
      default:
         return;
         break;
#endif // NEWVIEWPORT
   }

   if ( viewWidth > viewHeight )
   {
      if ( m_viewDirection == ViewPerspective )
      {
         m_zoomLevel = viewWidth / 2.0;
      }
      else
      {
         m_zoomLevel = viewWidth / 2.0;
      }
   }
   else
   {
      if ( m_viewDirection == ViewPerspective )
      {
         m_zoomLevel = viewHeight / 2.0;
      }
      else
      {
         m_zoomLevel = viewHeight / 2.0;
      }
   }

   QString zoomStr;
   zoomStr.sprintf( "%f", m_zoomLevel );
   emit zoomLevelChanged( zoomStr );

   makeCurrent();
   adjustViewport();
}

void ModelViewport::checkGlErrors()
{
   int error = glGetError();
   if ( error )
   {
      switch ( error )
      {
         case GL_INVALID_VALUE:
            model_status( m_model, StatusNormal, STATUSTIME_NONE, tr("OpenGL error = Invalid Value").utf8() );
            break;
         case GL_INVALID_ENUM:
            model_status( m_model, StatusNormal, STATUSTIME_NONE, tr("OpenGL error = Invalid Enum").utf8() );
            break;
         case GL_INVALID_OPERATION:
            model_status( m_model, StatusNormal, STATUSTIME_NONE, tr("OpenGL error = Invalid Operation").utf8() );
            break;
         case GL_STACK_OVERFLOW:
            model_status( m_model, StatusNormal, STATUSTIME_NONE, tr("OpenGL error = Stack Overflow").utf8() );
            break;
         case GL_STACK_UNDERFLOW:
            model_status( m_model, StatusNormal, STATUSTIME_NONE, tr("OpenGL error = Stack Underflow").utf8() );
            break;
         case GL_OUT_OF_MEMORY:
            model_status( m_model, StatusNormal, STATUSTIME_NONE, tr("OpenGL error = Out Of Memory").utf8() );
            break;
         default:
            model_status( m_model, StatusNormal, STATUSTIME_NONE, tr("OpenGL error = Unknown").utf8() );
            break;
      }
   }
}

void ModelViewport::copyContentsToTexture( Texture * tex )
{
   makeCurrent();

   m_capture = true;
   if ( tex )
   {
      unsigned w = this->width();
      unsigned h = this->height();

      // make sure texture can hold data

      if ( tex->m_data )
      {
         unsigned bpp = (tex->m_format == Texture::FORMAT_RGB) ? 3 : 4;
         if ( (tex->m_width * tex->m_height * bpp) < (w * h * 4) )
         {
            delete[] tex->m_data;
            tex->m_data = NULL;
         }
      }

      if ( tex->m_data == NULL )
      {
         tex->m_data = new uint8_t[ w * h * 4 ];
      }

      tex->m_width  = w;
      tex->m_height = h;
      tex->m_format = Texture::FORMAT_RGBA;

      makeCurrent();
      updateGL();

      glReadPixels( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tex->m_data );
   }

   makeCurrent();
   glClearColor( m_backColor.red() / 256.0, 
         m_backColor.green() / 256.0, 
         m_backColor.blue() / 256.0, 1.0f );

   m_capture = false;
}

void ModelViewport::updateCaptureGL()
{
   m_capture = true;
   makeCurrent();
   updateGL();
   m_capture = false;
}

