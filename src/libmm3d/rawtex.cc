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


#include "rawtex.h"

#include <string.h>
#include <stdlib.h>

#include "mm3dconfig.h"
#include "log.h"

using std::list;
using std::string;

RawTextureFilter::RawTextureFilter()
{
   m_read.push_back( "RAW" );
   m_write.push_back( "RAW" );
}

RawTextureFilter::~RawTextureFilter()
{
}

bool RawTextureFilter::canRead( const char * filename )
{
   if ( filename == NULL )
   {
      return false;
   }

   string cmpstr;
   unsigned len = strlen( filename );

   list<string>::iterator it;
   
   for ( it = m_read.begin(); it != m_read.end(); it++ )
   {
      cmpstr = string(".") + *it;

      if ( len >= cmpstr.length() )
      {
         if ( strcasecmp( &filename[len-cmpstr.length()], cmpstr.c_str()) == 0 )
         {
            return true;
         }
      }
   }

   return false;
}

Texture::ErrorE RawTextureFilter::readFile(Texture * texture, const char * filename)
{
   log_debug( "filename is %s\n", filename );

   FILE * fp;
   fp = fopen(filename, "rb");

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

   texture->m_filename = strdup( filename );

   if ( fp == NULL )
   {
      return Texture::ERROR_NO_FILE;
   }

   int32_t width   = 0;
   int32_t height  = 0;
   int32_t bytespp = 0;

   log_debug( "loading uncompressed RAW image file\n" );
   int itemsRead = 0;

   itemsRead += fread(&width,   sizeof(width),   1, fp);
   itemsRead += fread(&height,  sizeof(height),  1, fp);
   itemsRead += fread(&bytespp, sizeof(bytespp), 1, fp);

   log_debug( "read %d items\n", itemsRead );

   if ( itemsRead == 3 )
   {
      if((width <= 0) || (height <= 0) || ((bytespp != 3) && (bytespp != 4)))
      {
         fprintf( stderr, "Invalid texture information (%d x %d x %d)\n", 
               width, height, bytespp );
         fclose(fp);
         return Texture::ERROR_BAD_DATA;
      }

      log_debug( "raw size: %d x %d, %d bytes per pixel\n", width, height, bytespp );

      bool hasAlpha = (bytespp == 8);
      log_debug( "Alpha channel: %s\n", hasAlpha ? "present" : "not present" );

      uint32_t imageSize  = (bytespp * width * height);
      texture->m_data = new uint8_t[imageSize];
      texture->m_format = hasAlpha ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB;
      texture->m_width = width;
      texture->m_height = height;

      log_debug( "image size = %d\n", imageSize );

      for ( int h = height; h > 0; h-- )
      {
         if( fread( &texture->m_data[ (h-1) * width * bytespp ], 
                  width * bytespp, 1, fp) != 1 )
         {
            fprintf( stderr, "Could not read image data" );
            fclose(fp);
            return Texture::ERROR_UNEXPECTED_EOF;
         }
      }
   }
   else
   {
      if(fp != NULL)
      {
         fclose(fp);
      }
      return Texture::ERROR_BAD_DATA;
   }

   fclose(fp);
   return Texture::ERROR_NONE;
}

list<string> RawTextureFilter::getReadTypes()
{
   list<string> rval;

   list<string>::iterator it;
   for ( it = m_read.begin(); it != m_read.end(); it++ )
   {
      rval.push_back( string("*.") + *it );
   }

   return rval;
}

list<string> RawTextureFilter::getWriteTypes()
{
   list<string> rval;

   list<string>::iterator it;
   for ( it = m_write.begin(); it != m_write.end(); it++ )
   {
      rval.push_back( string("*.") + *it );
   }

   return rval;
}

