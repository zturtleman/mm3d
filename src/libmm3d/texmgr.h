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


#ifndef __TEXMGR_H
#define __TEXMGR_H

#include "texture.h"

#include <list>
#include <string>

using std::list;

typedef list< Texture * > TextureList;

//------------------------------------------------------------------
// About the TextureFilter class
//------------------------------------------------------------------
//
// The TextureFilter class is a base class for implementing filters to
// import and export texture images to various formats.  If you 
// implement a TextureFilter, you need to register the filter with the 
// TextureManager.  You only need one instance of your filter.
//
class TextureFilter
{
   public:
      virtual ~TextureFilter() {};

      class ImageData
      {
         public:

            // dataPtr should be null for write, non-null for read.
            // For write, ImageData will free the memory at dataPtr.
            // For read, you must free the memory at dataPtr.
            static ImageData * get( size_t dataSize, const uint8_t * dataPtr = NULL );

            void release(); // call this instead of delete

            uint8_t * getDataPtr();
            const uint8_t * getConstDataPtr() const;

            size_t getDataSize() const { return m_size; };

         protected:
            ImageData();
            virtual ~ImageData();

            static int s_allocated;

            bool      m_mustFree;
            size_t    m_size;
            uint8_t * m_data;
      };

      virtual std::list< std::string > getReadTypes()   = 0;
      virtual std::list< std::string > getWriteTypes() = 0;

      // It is a good idea to override this if you implement
      // a filter as a plugin.
      virtual void release() { delete this; };

      // readFile reads the contents of 'filename' and modifies 'texture' to
      // match the description in 'filename'.  This is the import function.
      //
      // The texture argument will be an empty texture instance.  If the file
      // cannot be loaded, return the appropriate TextureError error code.
      // If the load succeeds, return TextureError::ERROR_NONE.
      virtual Texture::ErrorE readFile( Texture * texture, const char * filename ) = 0;

      // writeFile writes the contents of 'texture' to 'filename'.
      // This is the export function.
      //
      // This function is currently not called and doesn't really need to be
      // implemented at this time.  It may be used in the future for more
      // advanced texturing modes.
      virtual Texture::ErrorE writeFile( Texture * texture, const char * filename ) { return Texture::ERROR_UNSUPPORTED_OPERATION; };

      // readMemory reads the contents of the ImageData memory segment and 
      // modifies 'texture' to match it.
      //
      // The texture argument will be an empty texture instance.  If the image
      // cannot be decoded, return the appropriate TextureError error code.
      // If the load succeeds, return TextureError::ERROR_NONE.
      virtual Texture::ErrorE readMemory( const char * format, Texture * texture, const ImageData * d ) { return Texture::ERROR_UNSUPPORTED_OPERATION; };

      // writeMemory writes the contents of 'texture' into the ImageData
      // Memory segment.
      //
      // This function is currently not called and doesn't really need to be
      // implemented at this time.  It may be used in the future for more
      // advanced texturing modes.
      //
      // 'd' should be a pointer to a NULL ImageData pointer.  Filter will
      // allocate a new ImageData object, you must free it.
      //
      // ie:
      //   TextureFilter::ImageData * d = NULL;
      //   filter->writeMemory( tex, &d );
      //   // Use data in 'd'
      //   d->release();
      virtual Texture::ErrorE writeMemory( const char * format, Texture * texture, ImageData ** d ) { return Texture::ERROR_UNSUPPORTED_OPERATION; };

      // This function should return true if the filename's extension matches 
      // a type supported by your filter.
      virtual bool canRead( const char * filename ) = 0;

      // This function should return true if the filename's extension matches 
      // a type supported by your filter and your filter has write support.
      //
      // This function is currently not called and doesn't need to be
      // implemented at this time.  It may be used in the future for more
      // advanced texturing modes.
      virtual bool canWrite( const char * filename ) { return false; };
};

typedef list< TextureFilter * > TextureFilterList;

//------------------------------------------------------------------
// About the TextureManager class
//------------------------------------------------------------------
//
// The TextureManager class is a singleton that keeps a list of filters
// for importing and exporting texture images to various file formats.
// At this time only read support is used.
//
// To create a new filter you must derive from the TextureFilter class
// and register your new filter with the TextureManager instance.  If
// your filter is called MyFilter, you would register your filter with
// the following function:
//
//    MyFilter * mf = new MyFilter();
//    TextureManager::getInstance()->registerFilter( mf );
//
// This would usually be done in the plugin_init function of a plugin.
// You only need one instance of your filter.
//
class TextureManager
{
   public:
      static TextureManager * getInstance();
      static void release();

      std::list< std::string > getAllReadTypes();
      std::list< std::string > getAllWritesTypes();

      void registerTextureFilter( TextureFilter * filter );
      
      Texture * getTexture( const char * filename, bool noCache = false, bool warning = true );
      Texture * getBlankTexture( const char * filename );
      Texture * getDefaultTexture( const char * filename );
      Texture::ErrorE getLastError() { return m_lastError; };

      Texture::ErrorE writeFile( Texture * tex, const char * filename );

      bool canWrite( const char * filename );

      Texture * getTexture( const char * format, const TextureFilter::ImageData * d );

      // 'd' should be a pointer to a NULL ImageData pointer.  Filter will
      // allocate a new ImageData object, you must free it.
      //
      // ie:
      //   TextureFilter::ImageData * d = NULL;
      //   texMgr->writeMemory( "PNG", tex, &d );
      //   // Use data in 'd'
      //   d->release();
      Texture::ErrorE writeMemory( const char * format, Texture * tex, TextureFilter::ImageData ** d );
      
      // reloads all textures that have changed on disk since last load time.
      // returns true if any textures have been reloaded.
      bool reloadTextures();

   protected:
      TextureManager();
      ~TextureManager();

      static TextureManager * s_instance;

      TextureFilterList m_filters;
      TextureList       m_textures;
      Texture::ErrorE   m_lastError;

      Texture         * m_defaultTexture;
};

extern void texture_manager_do_warning();

#endif // __TEXMGR_H
