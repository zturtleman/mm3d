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


#include "painttexturewin.h"

#include "textureframe.h"
#include "texwidget.h"
#include "model.h"
#include "texture.h"
#include "log.h"
#include "misc.h"
#include "3dmprefs.h"
#include "msg.h"
#include "decalmgr.h"
#include "helpwin.h"
#include "config.h"

#include "mq3compat.h"

#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qslider.h>
#include <qlineedit.h>
#include <math.h>

PaintTextureWin::PaintTextureWin( Model * model, QWidget * parent, const char * name )
   : PaintTextureWinBase( parent, name, true, WDestructiveClose ),
     m_accel( new QAccel(this) ),
     m_model( model ),
     m_saved( false )
{
   m_textureFrame->setModel( model );

   m_textureWidget = m_textureFrame->getTextureWidget();
   m_textureWidget->setInteractive( false );
   m_textureWidget->setMouseOperation( TextureWidget::MouseRange );

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );

   bool foundTexture = false;

   list<int> triangles;
   m_model->getSelectedTriangles( triangles );

   int material = -1;

   list<int>::iterator it;
   for ( it = triangles.begin(); !foundTexture && it != triangles.end(); it++ )
   {
      int g = m_model->getTriangleGroup( triangles.front() );
      int m = m_model->getGroupTextureId( g );
      if ( m >= 0 )
      {
         m_textureFrame->textureChangedEvent( m + 1 );
         foundTexture = true;
         material = m;
      }
   }

   if ( !foundTexture )
   {
      log_error( "no group selected\n" );
   }

   addTriangles();

   m_polygonsButton->setCurrentItem( 2 );
   m_verticesButton->setCurrentItem( 0 );

   m_hSize->setCurrentItem( 3 );
   m_vSize->setCurrentItem( 3 );

   if ( material >= 0 )
   {
      Texture * tex = m_model->getTextureData( material );
      if ( tex )
      {
         int x = tex->m_width;
         int y = tex->m_height;

         if ( x == y )
         {
            m_hSize->setCurrentItem( 3 );
            m_vSize->setCurrentItem( 3 );
         }
         else if ( y > x )
         {
            m_hSize->setCurrentItem( 2 );
            m_vSize->setCurrentItem( 5 );

            int index = 2;
            while ( y > x )
            {
               y = y / 2;
               index++;
            }
            m_vSize->setCurrentItem( index );
         }
         else
         {
            m_vSize->setCurrentItem( 2 );
            m_hSize->setCurrentItem( 5 );

            int index = 2;
            while ( x > y )
            {
               x = x / 2;
               index++;
            }
            m_hSize->setCurrentItem( index );
         }
      }
   }
   else
   {
      m_textureFrame->textureChangedEvent( 0 );
   }

   // FIXME allow background, or remove clear button
   m_clearButton->hide();
   m_textureWidget->setSolidBackground( true );

   updateDisplay();
}

PaintTextureWin::~PaintTextureWin()
{
}

void PaintTextureWin::helpNowEvent( int )
{
   HelpWin * win = new HelpWin( "olh_painttexturewin.html", true );
   win->show();
}

void PaintTextureWin::accept()
{
   PaintTextureWinBase::accept();
}

void PaintTextureWin::textureSizeChangeEvent()
{
   updateDisplay();
}

void PaintTextureWin::displayChangedEvent()
{
   updateDisplay();
}

void PaintTextureWin::clearEvent()
{
   m_textureWidget->setSolidBackground( true );
   m_textureWidget->updateGL();
}

void PaintTextureWin::saveEvent()
{
   const char * modelFile = m_model->getFilename();
   QString dir = QString::fromUtf8( g_prefs( "ui_model_dir" ).stringValue().c_str() );
   if ( modelFile && modelFile[0] != '\0' )
   {
      std::string fullname;
      std::string fullpath;
      std::string basename;

      normalizePath( modelFile, fullname, fullpath, basename );
      dir = tr( fullpath.c_str() );
   }

   bool again = true;
   while ( again )
   {
      again = false;

#ifdef HAVE_QT4
      QString filename = QFileDialog::getSaveFileName( 
            this, tr("File name for saved texture?"), dir, QString("PNG Images (*.png *.PNG)") );
#else
      QString filename = QFileDialog::getSaveFileName( 
            dir, QString("PNG Images (*.png *.PNG)"), this, QString(""), tr("File name for saved texture?") );
#endif

      if ( filename.length() > 0 )
      {
         bool save = true;

         if ( file_exists( filename.latin1() ) )
         {
            char val = msg_warning_prompt( (const char *) tr( "File exists.  Overwrite?" ).utf8(), "yNc" );
            switch ( val )
            {
               case 'N':
                  again = true;

                  // We want to fall through here

               case 'C':
                  save = false;
                  break;

               default:
                  break;
            }
         }

         if ( save )
         {
            int h = atoi( m_vSize->currentText().latin1() );
            int w = atoi( m_hSize->currentText().latin1() );

            // This is a total hack. For some reason Qt refuses to repaint
            // the widget here. To force an update I have to resize the
            // OpenGL widget, repaint, and then resize it back to the
            // original size. It's an ugly hack, but it makes this function
            // work.
            {
               int hack_w = m_textureWidget->width();
               int hack_h = m_textureWidget->height();
               m_textureWidget->resize( 4, 4 );
               m_textureWidget->updateGL();
               m_textureWidget->resize( hack_w, hack_h );
            }

            QImage img = m_textureWidget->grabFrameBuffer( false );
            img = img.smoothScale( w, h, QImage::ScaleMax );
            if ( !img.save( filename, "PNG", 100 ) )
            {
               QString msg = tr( "Could not write file: " ) + QString( "\n" );
               msg += filename;

               msg_error( (const char *) msg.utf8() );
            }

            updateDisplay();
         }
      }
      else
      {
         log_debug( "save frame buffer cancelled\n" );
      }
   }
}

void PaintTextureWin::addTriangles()
{
   m_textureWidget->clearCoordinates();

   list<int> triangles;
   m_model->getSelectedTriangles( triangles );

   list<int>::iterator it;

   float u = 0.0f;
   float v = 0.0f;

   for( it = triangles.begin(); it != triangles.end(); it++ )
   {
      int t;
      int vert[3];
      for ( t = 0; t < 3; t++ )
      {
         m_model->getTextureCoords( (*it), t, u, v );

         vert[t] = m_textureWidget->addVertex( u, v );
      }

      m_textureWidget->addTriangle( vert[0], vert[1], vert[2] );
   }
}

void PaintTextureWin::updateDisplay()
{
   int dm = m_polygonsButton->currentItem();

   dm++;

   m_textureWidget->setDrawMode( static_cast<TextureWidget::DrawModeE>(dm) );
   m_textureWidget->setDrawVertices( (m_verticesButton->currentItem() != 0) ? true : false );

   m_textureFrame->sizeOverride( atoi( m_hSize->currentText().latin1() ),
         atoi( m_vSize->currentText().latin1() ) );

   m_textureWidget->updateGL();
}

