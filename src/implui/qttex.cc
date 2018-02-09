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


#include "qttex.h"
//Added by qt3to4:

#include "mm3dconfig.h"
#include "log.h"

#include <ctype.h>
#include <stdlib.h>
#include <cstdio>

#include <QtCore/QBuffer>
#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <QtGui/QImageWriter>

using std::list;
using std::string;

QtTextureFilter::QtTextureFilter()
   : m_initialized( false )
{
}

QtTextureFilter::~QtTextureFilter()
{
}

bool QtTextureFilter::canRead( const char * filename )
{
   initializeSupported();

   if ( filename == NULL )
   {
      return false;
   }

   QString cmpstr;
   unsigned len = strlen( filename );

   QStringList::Iterator it;
   
   for ( it = m_read.begin(); it != m_read.end(); it++ )
   {
      cmpstr = QString(".") + *it;

      if ( (int) len >= (int) cmpstr.length() )
      {
         if ( strcasecmp( &filename[len-cmpstr.length()], (const char *) cmpstr.toUtf8()) == 0 )
         {
            return true;
         }
      }
   }

   return false;
}

bool QtTextureFilter::canWrite( const char * filename )
{
   initializeSupported();

   if ( filename == NULL )
   {
      return false;
   }

   QString cmpstr;
   unsigned len = strlen( filename );

   QStringList::Iterator it;
   
   for ( it = m_write.begin(); it != m_write.end(); it++ )
   {
      cmpstr = QString(".") + *it;

      if ( (int) len >= (int) cmpstr.length() )
      {
         if ( strcasecmp( &filename[len-cmpstr.length()], (const char *) cmpstr.toUtf8()) == 0 )
         {
            return true;
         }
      }
   }

   return false;
}

Texture::ErrorE QtTextureFilter::readFile( Texture * texture, const char * filename )
{
   initializeSupported();

   QImage image;

   if ( filename == NULL )
   {
      log_error( "filename NULL\n" );
      return Texture::ERROR_NO_FILE;
   }

   if ( texture == NULL )
   {
      log_error( "texture NULL\n" );
      return Texture::ERROR_UNKNOWN;
   }

   if ( image.load( QString::fromUtf8( filename ) ) )
   {
      imageToTexture( texture, &image );

      texture->m_filename = strdup( filename );

      const char * name = strrchr( filename, DIR_SLASH );
      if ( name )
      {
         texture->m_name = strdup( &name[1] );
      }
      else
      {
         texture->m_name = strdup( filename );
      }
      char * ext = strrchr( texture->m_name, '.' );
      if ( ext )
      {
         ext[0] = '\0';
      }

      return Texture::ERROR_NONE;
   }
   else
   {
      return Texture::ERROR_FILE_OPEN;
   }

   return Texture::ERROR_UNKNOWN;
}

Texture::ErrorE QtTextureFilter::readMemory( const char * format, Texture * texture, const ImageData * d )
{
   initializeSupported();

   if ( format == NULL || texture == NULL || d == NULL )
   {
      log_error( "bad argument\n" );
      return Texture::ERROR_UNKNOWN;
   }

   char fmt[5] = "PNG";
   getFormatString( fmt, format );

   log_debug( "loading image from data for %s, size %d\n", fmt, d->getDataSize() );
   const uint8_t * ptr = d->getConstDataPtr();

   if ( ptr == NULL )
   {
      log_error( "ImageData data pointer is null\n" );
      return Texture::ERROR_UNKNOWN;
   }

   QImage image;
   if ( image.loadFromData( ptr, d->getDataSize() ) )
   {
      imageToTexture( texture, &image );

      texture->m_filename = (char *) malloc( 20 );
      sprintf( texture->m_filename, "%p", texture );
      texture->m_name = (char *) malloc( 20 );
      strcpy( texture->m_name, texture->m_filename );

      return Texture::ERROR_NONE;
   }
   else
   {
      log_debug( "load failed\n" );
      return Texture::ERROR_FILE_READ;
   }

   return Texture::ERROR_UNKNOWN;
}

Texture::ErrorE QtTextureFilter::writeFile( Texture * texture, const char * filename )
{
   initializeSupported();

   if ( filename == NULL )
   {
      log_error( "filename NULL\n" );
      return Texture::ERROR_NO_FILE;
   }

   if ( texture == NULL )
   {
      log_error( "texture NULL\n" );
      return Texture::ERROR_UNKNOWN;
   }

   uint8_t * data = new uint8_t[ texture->m_width * texture->m_height * 4 ];
   textureToImage( texture, data );

   char fmt[5] = "PNG";
   getFormatString( fmt, filename );
   
   Texture::ErrorE err = Texture::ERROR_NONE;
   {
      QImage image ( data, texture->m_width, texture->m_height, QImage::Format_ARGB32 );

      if ( ! image.save( QString::fromUtf8( filename ), fmt, 100 ) )
      {
         return Texture::ERROR_FILE_WRITE;
      }
   }

   delete[] data; // Must do this after image goes out of scope

   return err;
}

Texture::ErrorE QtTextureFilter::writeMemory( const char * format, Texture * texture, TextureFilter::ImageData ** d )
{
   initializeSupported();

   if ( format == NULL )
   {
      log_error( "filename NULL\n" );
      return Texture::ERROR_NO_FILE;
   }

   if ( texture == NULL || d == NULL )
   {
      log_error( "texture NULL\n" );
      return Texture::ERROR_UNKNOWN;
   }

   uint8_t * data = new uint8_t[ texture->m_width * texture->m_height * 4 ];
   textureToImage( texture, data );

   char fmt[5] = "PNG";
   getFormatString( fmt, format );

   Texture::ErrorE err = Texture::ERROR_NONE;
   {
      QImage image ( data, texture->m_width, texture->m_height, QImage::Format_ARGB32 );

      QByteArray ba;
      QBuffer buffer( &ba );
      buffer.open( QIODevice::WriteOnly );
      if ( ! image.save( &buffer, fmt, 100 ) )
      {
         return Texture::ERROR_FILE_WRITE;
      }

      *d = TextureFilter::ImageData::get( ba.size() );

      memcpy( (*d)->getDataPtr(), ba.data(), ba.size() );
   }

   delete[] data; // Must do this after image goes out of scope

   return err;
}

list<string> QtTextureFilter::getReadTypes()
{
   initializeSupported();

   list<string> rval;

   QStringList::Iterator it;
   for ( it = m_read.begin(); it != m_read.end(); it++ )
   {
      rval.push_back( (const char *) (QString("*.") + *it).toUtf8() );
   }

   return rval;
}

list<string> QtTextureFilter::getWriteTypes()
{
   initializeSupported();

   list<string> rval;

   QStringList::Iterator it;
   for ( it = m_write.begin(); it != m_write.end(); it++ )
   {
      rval.push_back( (const char *) (QString("*.") + *it).toUtf8() );
   }

   return rval;
}

//------------------------------------------------------------------
// Protected members
//------------------------------------------------------------------

void QtTextureFilter::getFormatString( char * format, const char * filename )
{
   const char * ext = strrchr( filename, '.' );
   if ( ext )
   {
      ext++; // Skip '.'
   }
   else
   {
      ext = (char *) filename;
   }

   strncpy( format, ext, 5 );

   for ( unsigned t = 0; format[t]; t++ )
   {
      format[t] = toupper( format[t] );
   }

   if ( strcmp( format, "JPG" ) == 0 )
   {
      strcpy( format, "JPEG" );
   }
   if ( strcmp( format, "TIF" ) == 0 )
   {
      strcpy( format, "TIFF" );
   }
}

void QtTextureFilter::textureToImage( Texture * texture, uint8_t * data )
{
   unsigned  sbpp = ( texture->m_format == Texture::FORMAT_RGB ) ? 3 : 4;
   unsigned  dbpp = 4;
   uint8_t * src;
   uint8_t * dest;

   for ( int y = 0; y < texture->m_height; y++ )
   {
      for ( int x = 0; x < texture->m_width; x++ )
      {
         src  = &texture->m_data[ ((texture->m_height - y - 1) * texture->m_width + x) * sbpp ];
         dest = &data[ (y * texture->m_width + x) * dbpp ];

         dest[ 0 ] = src[ 2 ];
         dest[ 1 ] = src[ 1 ];
         dest[ 2 ] = src[ 0 ];

         if ( texture->m_format == Texture::FORMAT_RGBA )
         {
            dest[ 3 ] = src[3];
         }
         else
         {
            dest[ 3 ] = 0xff;
         }
      }
   }
}

void QtTextureFilter::imageToTexture( Texture * texture, QImage * image )
{
   texture->m_width  = image->width();
   texture->m_height = image->height();

   bool hasAlpha = image->hasAlphaChannel();
   log_debug( "Alpha channel: %s\n", hasAlpha ? "present" : "not present" );

   unsigned pixelBytes = hasAlpha ? 4 : 3;
   unsigned pixelCount = texture->m_width * texture->m_height;
   unsigned imageSize = pixelCount * (pixelBytes * sizeof(uint8_t));
   texture->m_data = new uint8_t[ imageSize ];

   // Make bottom row the first row, as required by OpenGL
   if ( hasAlpha )
   {
      texture->m_format = Texture::FORMAT_RGBA;
      for ( int y = 0; y < texture->m_height; y ++ )
      {
         for ( int x = 0; x < texture->m_width; x++ )
         {
            QRgb p = image->pixel( x, texture->m_height - y - 1 );
            texture->m_data[ ((y * texture->m_width + x)*4) + 0 ] = qRed( p );
            texture->m_data[ ((y * texture->m_width + x)*4) + 1 ] = qGreen( p );
            texture->m_data[ ((y * texture->m_width + x)*4) + 2 ] = qBlue( p );
            texture->m_data[ ((y * texture->m_width + x)*4) + 3 ] = qAlpha( p );
         }
      }
   }
   else
   {
      texture->m_format = Texture::FORMAT_RGB;
      for ( int y = 0; y < texture->m_height; y ++ )
      {
         for ( int x = 0; x < texture->m_width; x++ )
         {
            QRgb p = image->pixel( x, texture->m_height - y - 1 );
            texture->m_data[ ((y * texture->m_width + x)*3) + 0 ] = qRed( p );
            texture->m_data[ ((y * texture->m_width + x)*3) + 1 ] = qGreen( p );
            texture->m_data[ ((y * texture->m_width + x)*3) + 2 ] = qBlue( p );
         }
      }
   }
}

void QtTextureFilter::initializeSupported()
{
   if ( !m_initialized )
   {
      m_initialized = true;
      {
         QList< QByteArray > list = QImageReader::supportedImageFormats();
         for ( QList< QByteArray >::iterator it = list.begin(); it != list.end(); it++ )
         {
            const char * str = (*it).constData();
            m_read.push_back( QString(str) );
            if ( strcasecmp( str, "JPEG" ) == 0 )
            {
               m_read.push_back( QString("JPG") );
            }
            if ( strcasecmp( str, "TIFF" ) == 0 )
            {
               m_read.push_back( QString("TIF") );
            }
         }
      }

      {
         QList< QByteArray > list = QImageWriter::supportedImageFormats();
         for ( QList< QByteArray >::iterator it = list.begin(); it != list.end(); it++ )
         {
            const char * str = (*it).constData();
            m_write.push_back( QString(str) );
            if ( strcasecmp( str, "JPEG" ) == 0 )
            {
               m_write.push_back( QString("JPG") );
            }
            if ( strcasecmp( str, "TIFF" ) == 0 )
            {
               m_write.push_back( QString("TIF") );
            }
         }
      }
   }
}
