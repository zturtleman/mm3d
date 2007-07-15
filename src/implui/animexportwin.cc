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


#include "animexportwin.h"
#include "helpwin.h"
#include "viewpanel.h"
#include "model.h"
#include "mview.h"
#include "3dmprefs.h"
#include "misc.h"
#include "texture.h"
#include "texmgr.h"
#include "msg.h"

#include "mm3dport.h"
#include "mq3compat.h"

#include <unistd.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qimage.h>

AnimExportWindow::AnimExportWindow( Model * model, ViewPanel * viewPanel, QWidget * parent )
   : AnimExportWinBase( parent, "", true ),
     m_accel( new QAccel(this) ),
     m_model( model ),
     m_viewPanel( viewPanel )
{
   QString path = QString::fromUtf8( g_prefs( "ui_animexport_dir" ).stringValue().c_str() );

   if ( g_prefs.exists( "ui_animexport_format" ) )
   {
      int f = g_prefs( "ui_animexport_format" ).intValue();

      if ( f >= 0 && f < m_formatValue->count() )
      {
         m_formatValue->setCurrentItem( f );
      }
   }

   if ( g_prefs.exists( "ui_animexport_framerate" ) )
   {
      double fps = g_prefs( "ui_animexport_framerate" ).doubleValue();

      if ( fps > 0.0001 )
      {
         char fpsStr[20] = "";
         PORT_snprintf( fpsStr, sizeof(fpsStr), "%f", fps );
         m_frameRateValue->setText( QString(fpsStr) );
      }
   }

   if ( g_prefs.exists( "ui_animexport_seconds" ) )
   {
      int sec = g_prefs( "ui_animexport_seconds" ).intValue();

      if ( sec > 0 )
      {
         char secStr[20] = "";
         PORT_snprintf( secStr, sizeof(secStr), "%d", sec );
         m_secondsValue->setText( QString(secStr) );
      }
   }

   if ( g_prefs.exists( "ui_animexport_iterations" ) )
   {
      int iterations = g_prefs( "ui_animexport_iterations" ).intValue();

      if ( iterations > 0 )
      {
         char itStr[20] = "";
         PORT_snprintf( itStr, sizeof(itStr), "%d", iterations );
         m_iterationsValue->setText( QString(itStr) );
      }
   }

   if ( m_model )
   {
      bool labelAnims = false;
      unsigned scount = m_model->getAnimCount( Model::ANIMMODE_SKELETAL );
      unsigned fcount = m_model->getAnimCount( Model::ANIMMODE_FRAME );
      if ( scount > 0 && fcount > 0 )
      {
         labelAnims = true;
      }

      unsigned a = 0;

      for ( a = 0; a < scount; a++ )
      {
         QString name = labelAnims ? tr( "Skeletal - ", "Skeletal Animation prefix" ) : QString("");

         name += m_model->getAnimName( Model::ANIMMODE_SKELETAL, a );
         m_animValue->insertItem( name );
      }

      for ( a = 0; a < fcount; a++ )
      {
         QString name = labelAnims ? tr( "Frame - ", "Frame Animation prefix" )    : QString("");

         name += m_model->getAnimName( Model::ANIMMODE_FRAME, a );
         m_animValue->insertItem( name );
      }

      const char * filename = m_model->getFilename();

      if ( path.isNull() || path.isEmpty() )
      {
         std::string fullName = "";
         std::string fullPath = "";
         std::string baseName = "";

         normalizePath( filename, fullName, fullPath, baseName );

         if ( is_directory( fullPath.c_str() ) )
         {
            path = fullPath.c_str();
         }
      }
   }

   if ( path.isNull() || path.isEmpty() )
   {
      char cwd[ PATH_MAX ] = "/";
//#ifdef WIN32
//      _getcwd( cwd, sizeof( cwd ) );
//#else
      getcwd( cwd, sizeof( cwd ) );
//#endif
      path = cwd;
   }

   m_directoryLabel->setText( QString::fromUtf8( path ) );

   if ( m_viewPanel )
   {
      int index = -1;
      unsigned count = m_viewPanel->getModelViewCount();
      for ( unsigned m = 0; m < count; m++ )
      {
         ModelView * v = m_viewPanel->getModelView( m );

         QString name = tr( "[None]", "No viewport for animation image export" );
         if ( v )
         {
            name = tr("Viewport %1 - ").arg(m+1);
            name += v->getViewDirectionLabel();

            if ( index == -1 && v->getViewDirection() == 0 )
            {
               index = m;
            }
         }

         m_viewportValue->insertItem( name );
      }

      if ( index >= 0 )
      {
         m_viewportValue->setCurrentItem( index );
      }
   }

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );
}

AnimExportWindow::~AnimExportWindow()
{
}

void AnimExportWindow::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_animexportwin.html", true );
   win->show();
}

void AnimExportWindow::accept()
{
   if ( m_viewPanel == NULL || m_model == NULL )
   {
      return;
   }

   ModelView * v = m_viewPanel->getModelView( m_viewportValue->currentItem() );

   if ( v == NULL )
   {
      return;
   }

   Model::AnimationModeE mode = Model::ANIMMODE_SKELETAL;
   unsigned a = m_animValue->currentItem();
   if ( a >= m_model->getAnimCount( mode ) )
   {
      a -= m_model->getAnimCount( mode );
      mode = Model::ANIMMODE_FRAME;
   }

   double fps = m_model->getAnimFPS( mode, a );
   double spf = ( 1.0 / fps );
   double duration = 0.0;
   double tm = 0.0;

   double outfps = atof( m_frameRateValue->text().latin1() );
   if ( outfps < 0.0001 )
   {
      msg_warning( (const char *) tr("Must have more than 0 frames per second").utf8() );
      return;
   }
   double interval = (1.0 / outfps);

   if ( m_timeButton->isChecked() )
   {
      duration = atof( m_secondsValue->text().latin1() );
   }
   else
   {
      duration = spf 
         * atoi( m_iterationsValue->text().latin1() ) 
         * m_model->getAnimFrameCount( mode, a );
   }

   if ( duration <= 0.0 )
   {
      msg_warning( (const char *) tr("Must have more than 0 seconds of animation").utf8() );
      return;
   }

   QString path = m_directoryLabel->text();

   if ( is_directory( path.latin1() ) )
   {
      g_prefs( "ui_animexport_dir" ) = path.latin1();
      g_prefs( "ui_animexport_format" ) = m_formatValue->currentItem();
      g_prefs( "ui_animexport_framerate" ) = atof( m_frameRateValue->text().latin1() );
      g_prefs( "ui_animexport_seconds" ) = atoi( m_secondsValue->text().latin1() );
      g_prefs( "ui_animexport_iterations" ) = atoi( m_iterationsValue->text().latin1() );

      bool enable = m_model->setUndoEnabled( false );

      m_model->setAnimationLooping( true );
      m_model->setCurrentAnimation( mode, a );

      int frameNum = 0;

      char formatStr[20] = "";
      QString saveFormat = QString( "JPEG" );
      switch ( m_formatValue->currentItem() )
      {
         case 0:
            strcpy( formatStr, "%s/anim_%04d.jpg" );
            break;
         case 1:
            strcpy( formatStr, "%s/anim_%d.jpg" );
            break;
         case 2:
            strcpy( formatStr, "%s/anim_%04d.png" );
            saveFormat = QString( "PNG" );
            break;
         case 3:
         default:
            strcpy( formatStr, "%s/anim_%d.png" );
            saveFormat = QString( "PNG" );
            break;
      }

      this->hide();
      
      bool prompt    = true;
      bool keepGoing = true;

      while ( keepGoing && tm <= duration )
      {
         m_model->setCurrentAnimationTime( tm );
         v->updateCaptureGL();

         frameNum++;

         QString file;
         file.sprintf( formatStr, path.latin1(), frameNum );

         QImage img = v->grabFrameBuffer( false );

         if ( !img.save( file, saveFormat, 100 ) && prompt )
         {
            QString msg = tr( "Could not write file: " ) + QString("\n");
            msg += file;

            msg_error( (const char *) msg.utf8() );

            keepGoing = false;
         }

         tm += interval;
      }

      m_model->setNoAnimation();
      m_model->setUndoEnabled( enable );
      v->updateView();

      AnimExportWinBase::accept();
   }
   else
   {
      msg_warning( (const char *) tr("Output directory does not exist.").utf8() );
   }
}

void AnimExportWindow::reject()
{
   AnimExportWinBase::reject();
}

void AnimExportWindow::directoryButtonClicked()
{
   QString dir = QFileDialog::getExistingDirectory( m_directoryLabel->text() );
   if ( ! dir.isNull() && ! dir.isEmpty() )
   {
      m_directoryLabel->setText( QString::fromUtf8( dir ) );
   }
}
