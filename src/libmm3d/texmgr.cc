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


#include "texmgr.h"
#include "log.h"
#include "msg.h"
#include "texscale.h"
#include "misc.h"
#include "translate.h"

#include <string.h>
#include <time.h>
#include <errno.h>

#include <ctype.h>
#include <stdlib.h>

#include <string>
using std::string;

static bool _alreadyWarned = false;
static bool _doWarning = false;

void texture_manager_do_warning()
{
   if ( _doWarning )
   {
      msg_warning( transll( QT_TRANSLATE_NOOP( "LowLevel", "The model has a texture that's width or height is not a power of two (2, 4, 8, .., 64, 128, 256, ..)." )).c_str() );
      _alreadyWarned = true;
      _doWarning = false;
   }
}

int TextureFilter::ImageData::s_allocated = 0;

TextureFilter::ImageData::ImageData()
   : m_mustFree( false ),
     m_size( 0 ),
     m_data( NULL )
{
   s_allocated++;
}

TextureFilter::ImageData::~ImageData()
{
   s_allocated--;
   if ( m_mustFree && m_data )
   {
      delete[] m_data;
      m_data = NULL;
      m_mustFree = false;
   }
}

TextureFilter::ImageData * TextureFilter::ImageData::get( size_t dataSize, const uint8_t * dataPtr )
{
   ImageData * ptr = new ImageData();

   ptr->m_size = dataSize;
   
   // Cast to non-const... I promise I won't change it!
   ptr->m_data = (uint8_t *) dataPtr;

   ptr->m_mustFree = false;
   if ( ptr->m_data == NULL && ptr->m_size != 0 )
   {
      ptr->m_mustFree = true;
      ptr->m_data = new uint8_t[ ptr->m_size ];
   }

   return ptr;
}

void TextureFilter::ImageData::release()
{
   delete this;
}

uint8_t * TextureFilter::ImageData::getDataPtr()
{
   if ( m_mustFree )
   {
      return m_data;
   }
   else
   {
      return NULL;  // Memory is read only!  Call getConstDataPtr() instead.
   }
}

const uint8_t * TextureFilter::ImageData::getConstDataPtr() const
{
   return m_data;
}

/* static */
Texture::ErrorE TextureFilter::errnoToTextureError( int err, Texture::ErrorE defaultError )
{
   switch ( err )
   {
      case 0:
         return Texture::ERROR_NONE;
      case EINVAL:
         return Texture::ERROR_BAD_ARGUMENT;
      case EACCES:
      case EPERM:
         return Texture::ERROR_NO_ACCESS;
      case ENOENT:
      case EBADF:
         return Texture::ERROR_NO_FILE;
      case EISDIR:
         return Texture::ERROR_BAD_DATA;
      default:
         break;
   }
   return defaultError;
}

TextureManager * TextureManager::s_instance = NULL;

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
   log_debug( "TextureManager releasing %" PORTuSIZE " textures and %" PORTuSIZE " filters\n", 
         m_textures.size(), m_filters.size() );

   TextureList::iterator texIt = m_textures.begin();
   while ( texIt != m_textures.end() )
   {
      delete (*texIt);
      texIt++;
   }
   m_textures.clear();

   TextureFilterList::iterator filterIt = m_filters.begin();
   while ( filterIt != m_filters.end() )
   {
      (*filterIt)->release();
      filterIt++;
   }
   m_filters.clear();
}

TextureManager * TextureManager::getInstance()
{
   if ( s_instance == NULL )
   {
      s_instance = new TextureManager();
   }

   return s_instance;
}

void TextureManager::release()
{
   if ( s_instance != NULL )
   {
      delete s_instance;
      s_instance = NULL;
   }
}

void TextureManager::registerTextureFilter( TextureFilter * filter )
{
   m_filters.push_front( filter );
}

Texture * TextureManager::getTexture( const char * filename, bool noCache, bool warning )
{
   if ( filename == NULL )
   {
      return NULL;
   }

   if( !noCache )
   {
      TextureList::iterator texIt = m_textures.begin();
   
      while ( texIt != m_textures.end() )
      {
         if ( strcmp( filename, (*texIt)->m_filename ) == 0 )
         {
            log_debug( "cached image %s\n", filename );
            return *texIt;
         }
   
         texIt++;
      }
   }

   if ( filename[0] == '\0' )
   {
      return getBlankTexture( "blank" );
   }

   TextureFilterList::iterator filterIt = m_filters.begin();
   Texture * newTexture = new Texture();
   newTexture->m_isBad = false;

   while ( filterIt != m_filters.end() )
   {
      if ( (*filterIt)->canRead( filename ) )
      {
         Texture::ErrorE error;
         if ( (error = (*filterIt)->readFile( newTexture, filename )) == Texture::ERROR_NONE )
         {
            m_lastError = Texture::ERROR_NONE;
            log_debug( "read from image file %s\n", filename );
            newTexture->removeOpaqueAlphaChannel();
            time_t mtime;
            file_modifiedtime( filename, &mtime );
            newTexture->m_loadTime = mtime;
            newTexture->m_origWidth = newTexture->m_width;
            newTexture->m_origHeight = newTexture->m_height;
            if ( texture_scale_need_scale( newTexture->m_width, newTexture->m_height ) )
            {
               if ( warning )
                  _doWarning = true;

               uint8_t * oldData = newTexture->m_data;
               newTexture->m_data = texture_scale_auto(
                     newTexture->m_data,
                     newTexture->m_format,
                     newTexture->m_width,
                     newTexture->m_height );

               delete[] oldData;
            }

            if( !noCache )
            {
               m_textures.push_front( newTexture );
            }

            return newTexture;
         }
         else
         {
            m_lastError = error;
            log_error( "filter could not read file: %d\n", error );
         }
      }
      filterIt++;
   }

   delete newTexture;

   return NULL;
}

Texture * TextureManager::getTexture( const char * format, const TextureFilter::ImageData * d )
{
   if ( format == NULL || d == NULL )
   {
      return NULL;
   }

   TextureFilterList::iterator filterIt = m_filters.begin();
   Texture * newTexture = new Texture();
   newTexture->m_isBad = false;

   std::string formatStr = std::string(".") + std::string( format );

   log_debug( "finding filter for image in memory\n" );

   while ( filterIt != m_filters.end() )
   {
      if ( (*filterIt)->canRead( formatStr.c_str() ) )
      {
         Texture::ErrorE error;
         log_debug( "found filter for format %s\n", format );

         if ( (error = (*filterIt)->readMemory( format, newTexture, d )) == Texture::ERROR_NONE )
         {
            m_lastError = Texture::ERROR_NONE;
            log_debug( "read from image memory\n" );
            newTexture->removeOpaqueAlphaChannel();
            newTexture->m_origWidth = newTexture->m_width;
            newTexture->m_origHeight = newTexture->m_height;
            if ( texture_scale_need_scale( newTexture->m_width, newTexture->m_height ) )
            {
               _doWarning = true;

               uint8_t * oldData = newTexture->m_data;
               newTexture->m_data = texture_scale_auto(
                     newTexture->m_data,
                     newTexture->m_format,
                     newTexture->m_width,
                     newTexture->m_height );

               delete[] oldData;
            }
            m_textures.push_front( newTexture );

            return newTexture;
         }
         else
         {
            m_lastError = error;
            log_error( "filter could not read memory: %d\n", error );
         }
      }
      filterIt++;
   }

   delete newTexture;

   return NULL;
}



bool TextureManager::reloadTextures()
{
   bool anyTextureChanged = false;
   TextureList::iterator texIter;
   
   for( texIter = m_textures.begin(); texIter != m_textures.end(); texIter++ )
   {
      time_t mtime;
      
      if( file_modifiedtime( (*texIter)->m_filename, &mtime ) && mtime > (*texIter)->m_loadTime )
      {
         Texture *refreshedTexture = getTexture( (*texIter)->m_filename, true );
         if( refreshedTexture )
         {
            log_debug( "reloaded texture %s\n", (*texIter)->m_filename );
            refreshedTexture->removeOpaqueAlphaChannel();
            // Swap m_data pointers for new and old
            uint8_t * tempPtr = (*texIter)->m_data;
            
            (*texIter)->m_width = refreshedTexture->m_width;
            (*texIter)->m_height = refreshedTexture->m_height;
            (*texIter)->m_origWidth = refreshedTexture->m_origWidth;
            (*texIter)->m_origHeight = refreshedTexture->m_origHeight;
            (*texIter)->m_format = refreshedTexture->m_format;
            (*texIter)->m_data = refreshedTexture->m_data;
            (*texIter)->m_loadTime = mtime;

            // new texture will clean up old data ptr.
            refreshedTexture->m_data = tempPtr;
            
            delete refreshedTexture;

            anyTextureChanged = true;
         }
         else
         {
            std::string msg = transll( QT_TRANSLATE_NOOP( "LowLevel", "Could not load" ) );
            msg += std::string(" ") + (*texIter)->m_filename;

            msg_warning( msg.c_str() );
         }
      }
   }
   
   return anyTextureChanged;
}


Texture * TextureManager::getBlankTexture( const char * name )
{
   TextureList::iterator texIt = m_textures.begin();
   while ( texIt != m_textures.end() )
   {
      if ( strcmp( name, "" ) == 0 )
      {
         log_debug( "cached image (blank)\n" );
         return *texIt;
      }

      texIt++;
   }

   Texture * tex = new Texture();
   tex->m_isBad = false;

   tex->m_name = strdup( name );
   tex->m_filename = strdup( "" );
   tex->m_height = 2;
   tex->m_width = 2;
   tex->m_origHeight = tex->m_height;
   tex->m_origWidth  = tex->m_width;
   tex->m_data = (uint8_t *) malloc( sizeof(uint8_t) * 4 * 4);

   for ( unsigned t = 0; t < 4 * 4; t++ )
   {
      tex->m_data[ t ] = 0xFF;
   }

   tex->removeOpaqueAlphaChannel();

   m_textures.push_front( tex );
   return tex;
}

Texture * TextureManager::getDefaultTexture( const char * filename )
{
   Texture * tex = new Texture();
   tex->m_isBad = true;

   string str = string("bad: ") + filename;

   tex->m_name = strdup( str.c_str() );
   tex->m_filename = strdup( str.c_str() );
   tex->m_height = 8;
   tex->m_width = 8;
   tex->m_origWidth = tex->m_width;
   tex->m_origHeight = tex->m_height;
   tex->m_data = (uint8_t *) malloc( sizeof(uint8_t) * 4 * 64);

   char pattern[] = "                 x   x    x x      x      x x    x   x           ";
   for ( int y = 0; y < tex->m_height; y++ )
   {
      for ( int x = 0; x < tex->m_width; x++ )
      {
         switch ( pattern[ y*8 + x ] )
         {
            case ' ':
               tex->m_data[ (y*8 + x) * 4 + 0 ] = 0xFF;
               tex->m_data[ (y*8 + x) * 4 + 1 ] = 0x00;
               tex->m_data[ (y*8 + x) * 4 + 2 ] = 0x00;
               tex->m_data[ (y*8 + x) * 4 + 3 ] = 0xFF;
               break;
            case 'x':
               tex->m_data[ (y*8 + x) * 4 + 0 ] = 0x00;
               tex->m_data[ (y*8 + x) * 4 + 1 ] = 0x00;
               tex->m_data[ (y*8 + x) * 4 + 2 ] = 0x00;
               tex->m_data[ (y*8 + x) * 4 + 3 ] = 0xFF;
               break;
            default:
               break;
         }
      }
   }

   tex->removeOpaqueAlphaChannel();

   m_textures.push_front( tex );
   return tex;
}

Texture::ErrorE TextureManager::writeFile( Texture * tex, const char * filename )
{
   if ( filename == NULL || filename[0] == '\0' || tex == NULL )
   {
      return Texture::ERROR_BAD_ARGUMENT;
   }

   TextureFilterList::iterator filterIt = m_filters.begin();

   while ( filterIt != m_filters.end() )
   {
      if ( (*filterIt)->canWrite( filename ) )
      {
         Texture::ErrorE error;
         if ( (error = (*filterIt)->writeFile( tex, filename )) == Texture::ERROR_NONE )
         {
            m_lastError = Texture::ERROR_NONE;
            return Texture::ERROR_NONE;
         }
         else
         {
            m_lastError = error;
            log_error( "filter could not write file: %d\n", error );
         }
      }
      filterIt++;
   }

   return Texture::ERROR_UNSUPPORTED_OPERATION;
}

bool TextureManager::canWrite( const char * filename )
{
   if ( filename == NULL || filename[0] == '\0' )
   {
      return false;
   }

   TextureFilterList::iterator filterIt = m_filters.begin();

   while ( filterIt != m_filters.end() )
   {
      if ( (*filterIt)->canWrite( filename ) )
      {
         return true;
      }
      filterIt++;
   }

   return false;
}

Texture::ErrorE TextureManager::writeMemory( const char * format, Texture * tex, TextureFilter::ImageData ** d )
{
   if ( format == NULL || format[0] == '\0' || tex == NULL || d == NULL )
   {
      return Texture::ERROR_BAD_ARGUMENT;
   }

   TextureFilterList::iterator filterIt = m_filters.begin();

   log_debug( "finding filter for format %s\n", format );
   std::string filename = std::string( "." ) + std::string( format );
   while ( filterIt != m_filters.end() )
   {
      if ( (*filterIt)->canWrite( filename.c_str() ) )
      {
         log_debug( "found filter for format %s\n", format );
         Texture::ErrorE error;
         if ( (error = (*filterIt)->writeMemory( format, tex, d )) == Texture::ERROR_NONE )
         {
            m_lastError = Texture::ERROR_NONE;
            return Texture::ERROR_NONE;
         }
         else
         {
            m_lastError = error;
            log_error( "filter could not write memory: %d\n", error );
         }
      }
      filterIt++;
   }

   return Texture::ERROR_UNSUPPORTED_OPERATION;
}

list<string> TextureManager::getAllReadTypes()
{
   list<string> rval;
   TextureFilterList::iterator filterIt = m_filters.begin();

   log_debug( "have %" PORTuSIZE " filters\n", m_filters.size() );

   while ( filterIt != m_filters.end() )
   {
      list<string> temp = (*filterIt)->getReadTypes();
      while ( temp.size() )
      {
         bool found = false;

         list<string>::iterator sit;

         for ( sit = rval.begin(); sit != rval.end() && !found; sit++ )
         {
            if ( *sit == temp.front() )
            {
               found = true;
            }
         }

         if ( !found )
         {
            string s = temp.front();
            rval.push_back( s );

#ifndef WIN32
            unsigned len = s.length();
            for ( unsigned n = 0; n < len; n++ )
            {
               if ( isupper( s[n] ) )
               {
                  s[n] = tolower( s[n] );
               }
               else
               {
                  s[n] = toupper( s[n] );
               }
            }
            rval.push_back( s );
#endif // ! WIN32
         }
         temp.pop_front();
      }
      
      filterIt++;
   }

   return rval;
}

