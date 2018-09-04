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


#ifndef __QTTEX_H
#define __QTTEX_H

#include "texmgr.h"

#include <QtCore/QStringList>

class QImage;

class QtTextureFilter : public TextureFilter
{
   public:
      QtTextureFilter();
      virtual ~QtTextureFilter();

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      Texture::ErrorE readFile( Texture * texture, const char * filename );
      Texture::ErrorE writeFile( Texture * texture, const char * filename );
      bool canRead( const char * filename );
      bool canWrite( const char * filename );

      Texture::ErrorE readMemory( const char * format, Texture * texture, const ImageData * d );
      Texture::ErrorE writeMemory( const char * format, Texture * texture, ImageData ** d );

   protected:

      void initializeSupported();

      void imageToTexture( Texture * texture, QImage * image );
      void textureToImage( Texture * texture, uint8_t * data );

      void getFormatString( char * format, const char * filename );

      bool m_initialized;

      QStringList m_read;
      QStringList m_write;
};

#endif // __QTTEX_H
