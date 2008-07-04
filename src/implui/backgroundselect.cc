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


#include "backgroundselect.h"

#include "model.h"
#include "log.h"
#include "msg.h"
#include "decalmgr.h"
#include "textureframe.h"
#include "3dmprefs.h"
#include "texmgr.h"
#include "texture.h"

#include "model.h"
#include "errorobj.h"

#include <QtGui/QPushButton>
#include <QtGui/QPixmap>
#include <QtGui/QFileDialog>

#include <stdlib.h>

#include <string>

using std::list;
using std::map;

BackgroundSelect::BackgroundSelect( Model * model, unsigned index, QWidget * parent )
   : QWidget( parent ),
     m_model( model ),
     m_index( index )
{
   setupUi( this );

   std::string filename = m_model->getBackgroundImage( index );
   setFilename( filename.c_str() );
   m_textureFrame->updateSize();
}

BackgroundSelect::~BackgroundSelect()
{
}

void BackgroundSelect::noneEvent()
{
   m_model->setBackgroundImage( m_index, "" );
   m_textureFrame->setTexture( -1, NULL );
   m_textureFrame->updateSize();
}

void BackgroundSelect::selectFileEvent()
{
   list<std::string> formats = TextureManager::getInstance()->getAllReadTypes();

   QString formatsStr = tr( "All Supported Formats (" );

   list<std::string>::iterator it = formats.begin();
   while(  it != formats.end() )
   {
      formatsStr += QString( (*it).c_str() );

      it++;

      if ( it != formats.end() )
      {
         formatsStr += " ";
      }
   }

   formatsStr += ")";

   QString dir = QString::fromUtf8( g_prefs( "ui_background_dir" ).stringValue().c_str() );
   if ( dir.isEmpty() )
   {
      dir = ".";
   }

   QFileDialog d(NULL, QString(""), dir, formatsStr + QString(";; All Files (*)" ) );

   d.setWindowTitle( tr("Open background image") );
   d.selectFilter( formatsStr );

   int execval = d.exec();

   QStringList files = d.selectedFiles();
   if ( QDialog::Accepted == execval && !files.empty() )
   {
      std::string file = (const char *) files[0].toUtf8();
      QString path = d.directory().absolutePath();

      g_prefs( "ui_background_dir" ) = (const char *) path.toUtf8();

      Texture * tex = TextureManager::getInstance()->getTexture( file.c_str() );
      if ( tex )
      {
         m_model->setBackgroundImage( m_index, file.c_str() );
         m_textureFrame->setTexture( -1, tex );
         m_textureFrame->updateSize();

         // Do NOT delete tex, TextureManager does that
      }
      else
      {
         QString err = tr(file.c_str()) + "\n";
         Texture::ErrorE e = TextureManager::getInstance()->getLastError();
         if ( e != Texture::ERROR_NONE )
         {
            err += textureErrStr( e );
         }
         else
         {
            err += tr("Could not open file");
         }
         msg_error( (const char *) err.toUtf8() );
      }
   }
}

void BackgroundSelect::setFilename( const char * filename )
{
   Texture * tex = TextureManager::getInstance()->getTexture( filename );
   if ( tex )
   {
      m_model->setBackgroundImage( m_index, filename );
      m_textureFrame->setTexture( -1, tex );
      m_textureFrame->updateSize();

      // Do NOT delete tex, TextureManager does that
   }
}
