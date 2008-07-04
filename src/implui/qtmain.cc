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


#include "qtmain.h"
#include "viewwin.h"
#include "model.h"
#include "sysconf.h"
#include "config.h"

#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtGui/QApplication>
#include <QtOpenGL/QGLFormat>

#include <list>
#include <string>
#include <unistd.h>
#include <limits.h>

#include "log.h"
#include "mm3dport.h"
#include "misc.h"
#include "msg.h"
#include "msgqt.h"
#include "3dmprefs.h"
#include "cmdline.h"

#include "mlocale.h"

static QApplication * s_app = NULL;
static QTranslator  * s_qtXlat = NULL;
static QTranslator  * s_mm3dXlat = NULL;

static bool _has_gl_support()
{
   if ( ! QGLFormat::hasOpenGL() )
   {
      log_error( "No openGL support, exiting...\n" );
      return false;
   }

   QGLFormat format = QGLFormat::defaultFormat();

   format.setDoubleBuffer( true );
   format.setRgba( true );
   format.setAlpha( true );

   QGLFormat::setDefaultFormat( format );

   format = QGLFormat::defaultFormat();

   log_debug( "qt says alpha is%s enabled\n", (format.alpha() ? "" : " not" ) );

   return true;
}

static void _cleanup()
{
   // TODO add any necessary cleanup
}

static bool loadTranslationFile( QTranslator * xlat, const QString & localeFile )
{
   std::list<std::string> path_list;

   path_list.push_back( "." );
   path_list.push_back( "../i18n" );
   path_list.push_back( getI18nDirectory() );

   // try current directory first (for override), then mm3d system directory
   for ( std::list<std::string>::iterator it = path_list.begin(); it != path_list.end(); ++it )
   {
      log_debug( "attempting to load translation %s from %s\n", (const char *) localeFile.toUtf8(), (const char *) it->c_str() );
      if ( s_qtXlat->load( localeFile, it->c_str() ) )
      {
         log_debug( "  loaded.\n" );
         return true;
      }
   }

   log_warning( "unable to load translation for %s\n", (const char *) localeFile.toUtf8() );
   return false;
}

QApplication * ui_getapp()
{
   return s_app;
}

int ui_prep( int & argc, char * argv[] )
{
   s_app = new QApplication( argc, argv );

   QString loc = mlocale_get().c_str();

   if ( loc == "" )
   {
      loc = QLocale::system().name();
   }

   // General Qt translations
   s_qtXlat = new QTranslator( 0 );
   QString qtLocale = QString( "qt_" ) + loc;
   loadTranslationFile( s_qtXlat, qtLocale );
   s_app->installTranslator( s_qtXlat );

   // MM3D translations
   s_mm3dXlat = new QTranslator( 0 );
   QString mm3dLocale = QString( "mm3d_" ) + loc;
   loadTranslationFile( s_mm3dXlat, mm3dLocale );
   s_app->installTranslator( s_mm3dXlat );

   return 0;
}

int ui_init( int & argc, char * argv[] )
{
   int rval = 0;

   if ( !s_app )
   {
      ui_prep( argc, argv );
   }

   if ( cmdline_runcommand )
   {
      rval = cmdline_command();
   }
   if ( cmdline_runui )
   {
      if ( ! _has_gl_support() )
      {
         return -1;
      }

      bool opened = false;
      unsigned openCount = 0;

      init_msgqt();

      openCount = cmdline_getOpenModelCount();

      if ( openCount == 0 )
      {
         for ( int t = 1; t < argc; t++ )
         {
            char pwd[ PATH_MAX ];
            getcwd( pwd, PATH_MAX );
            std::string file = normalizePath( argv[t], pwd );
            if ( ViewWindow::openModel( file.c_str() ) )
            {
               openCount++;
               opened = true;
            }
         }
      }
      else
      {
         for ( unsigned t = 0; t < openCount; t++ )
         {
            Model * m = cmdline_getOpenModel( t );
            ViewWindow * win = new ViewWindow( m );
            win->getSaved(); // Just so I don't have a warning
            opened = true;
         }

         cmdline_clearOpenModelList();
      }

      if ( opened )
      {
         rval = s_app->exec();
      }
      else
      {
         ViewWindow * win = new ViewWindow( new Model );
         win->getSaved(); // Just so I don't have a warning
         rval = s_app->exec();

         /*
            StartPrompt p;
            p.exec();

            if ( !p.shouldExit() )
            {
            rval = s_app->exec();
            }
            */
      }

      _cleanup();
   }

   delete s_mm3dXlat;
   delete s_qtXlat;
   delete s_app;

   s_app = NULL;
   s_qtXlat = NULL;
   s_mm3dXlat = NULL;

   return rval;
}

void ui_exit()
{
   _cleanup();
   if ( qApp )
   {
      qApp->quit();
   }
   else
   {
      exit( 0 );
   }
}

