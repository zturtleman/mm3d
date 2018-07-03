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


#include "texture.h"
#include "translate.h"

int Texture::s_allocated = 0;

#include <stdio.h>
#include <stdlib.h>

Texture::Texture()
   : m_name( NULL ),
     m_filename( NULL ),
     m_isBad( false ),
     m_height( 0 ),
     m_width( 0 ),
     m_format( FORMAT_RGBA ),
     m_data( NULL )
{
   s_allocated++;
}

Texture::~Texture()
{
   if ( m_name )
   {
      free( m_name );
   }
   if ( m_filename )
   {
      free( m_filename );
   }
   if ( m_data )
   {
      delete[] m_data;
   }
   s_allocated--;
}

bool Texture::compare( Texture * t1, Texture * t2, CompareResultT * res, unsigned fuzzyValue )
{
   return t1->compare( t2, res, fuzzyValue );
}

bool Texture::compare( Texture * tex, CompareResultT * res, unsigned fuzzyValue )
{
   Texture * t1 = this;
   Texture * t2 = tex;

   if ( t1->m_width != t2->m_width
         || t1->m_height != t2->m_height
         || t1->m_format != t2->m_format 
         || t1->m_isBad
         || t2->m_isBad )
   {
      res->comparable = false;
      return false;
   }

   res->comparable = true;
   bool hasAlpha = (t1->m_format == Texture::FORMAT_RGBA) ? true : false;
   unsigned count = t1->m_width * t1->m_height;

   res->pixelCount = count;
   res->matchCount = 0;
   res->fuzzyCount = 0;

   unsigned bytespp = hasAlpha ? 4 : 3;
   unsigned off = 0;
   unsigned fuzzy = 0;

   for ( unsigned p = 0; p < count; p++ )
   {
      off = p * bytespp;
      fuzzy = 0;

      fuzzy += abs(t1->m_data[ off + 0 ] - t2->m_data[ off + 0 ]);
      fuzzy += abs(t1->m_data[ off + 1 ] - t2->m_data[ off + 1 ]);
      fuzzy += abs(t1->m_data[ off + 2 ] - t2->m_data[ off + 2 ]);
      if ( hasAlpha )
      {
         fuzzy += abs(t1->m_data[ off + 3 ] - t2->m_data[ off + 3 ]);
      }

      if ( hasAlpha 
            && t1->m_data[ off + 3 ] == 0
            && t2->m_data[ off + 3 ] == 0 )
      {
         fuzzy = 0;
      }

      if ( fuzzy == 0 )
         res->matchCount++;
      if ( fuzzy <= fuzzyValue )
         res->fuzzyCount++;
   }

   return( res->pixelCount == res->matchCount );
}


const char * Texture::errorToString( Texture::ErrorE e )
{
   switch ( e )
   {
      case ERROR_NONE:
         return QT_TRANSLATE_NOOP( "LowLevel", "Success" );
      case ERROR_NO_FILE:
         return QT_TRANSLATE_NOOP( "LowLevel", "File does not exist" );
      case ERROR_NO_ACCESS:
         return QT_TRANSLATE_NOOP( "LowLevel", "Permission denied" );
      case ERROR_FILE_OPEN:
         return QT_TRANSLATE_NOOP( "LowLevel", "Could not open file" );
      case ERROR_FILE_READ:
         return QT_TRANSLATE_NOOP( "LowLevel", "Could not read from file" );
      case ERROR_FILE_WRITE:
         return QT_TRANSLATE_NOOP( "LowLevel", "Could not write file" );
      case ERROR_BAD_MAGIC:
         return QT_TRANSLATE_NOOP( "LowLevel", "File is the wrong type or corrupted" );
      case ERROR_UNSUPPORTED_VERSION:
         return QT_TRANSLATE_NOOP( "LowLevel", "Unsupported version" );
      case ERROR_BAD_DATA:
         return QT_TRANSLATE_NOOP( "LowLevel", "File contains invalid data" );
      case ERROR_UNEXPECTED_EOF:
         return QT_TRANSLATE_NOOP( "LowLevel", "Unexpected end of file" );
      case ERROR_UNSUPPORTED_OPERATION:
         return QT_TRANSLATE_NOOP( "LowLevel", "This operation is not supported" );
      case ERROR_BAD_ARGUMENT:
         return QT_TRANSLATE_NOOP( "LowLevel", "Invalid argument (internal error, probably null pointer argument)" );
      case ERROR_UNKNOWN:
         return QT_TRANSLATE_NOOP( "LowLevel", "Unknown error"  );
      default:
         break;
   }
   return QT_TRANSLATE_NOOP( "LowLevel", "Invalid error code" );
}

void Texture::removeOpaqueAlphaChannel()
{
   if ( m_data == NULL || m_format != FORMAT_RGBA || m_width < 1 || m_height < 1 )
   {
      return;
   }

   uint32_t srcImageSize = 4 * m_width * m_height;
   for ( uint32_t src = 0; src < srcImageSize; src += 4 )
   {
      if ( m_data[src + 3] < 255 )
      {
         // Alpha channel is used.
         return;
      }
   }

   m_format = FORMAT_RGB;
   for ( uint32_t src = 0, dst = 0; src < srcImageSize; src += 4, dst += 3 )
   {
      m_data[dst + 0] = m_data[src + 0];
      m_data[dst + 1] = m_data[src + 1];
      m_data[dst + 2] = m_data[src + 2];
   }
}
