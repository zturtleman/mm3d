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


#include "tgatex.h"

#include <string.h>
#include <stdlib.h>

#include "endianconfig.h"
#include "mm3dconfig.h"
#include "log.h"

#include "filedatasource.h"
#include "file_closer.h"

using std::list;
using std::string;

typedef struct _TGAHeader_t
{
	uint8_t Header[12];
} TGAHeaderT;

typedef struct _TGA
{
	uint16_t width;
	uint16_t height;
	uint8_t bpp;
	uint8_t reserved;
} TGA;

TGAHeaderT tgaheader;  // TGA header
TGA tga;

uint8_t uTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
uint8_t cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};	// Compressed TGA Header
Texture::ErrorE LoadUncompressedTGA(Texture *, const char *, DataSource *);  // Load an Uncompressed file
Texture::ErrorE LoadCompressedTGA(Texture *, const char *, DataSource *);    // Load a Compressed file

TgaTextureFilter::TgaTextureFilter()
{
   m_read.push_back( "TGA" );
   m_write.push_back( "TGA" );
}

TgaTextureFilter::~TgaTextureFilter()
{
}

bool TgaTextureFilter::canRead( const char * filename )
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

Texture::ErrorE TgaTextureFilter::readFile(Texture * texture, const char * filename)
{
   FILE * fp = fopen(filename, "rb");

   if(fp == NULL)
      return Texture::ERROR_NO_FILE;

   file_closer fc( fp );

   FileDataSource src( fp );

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

   if ( !src.readBytes( tgaheader.Header, sizeof(tgaheader.Header) ) )
      return Texture::ERROR_BAD_DATA;

   Texture::ErrorE err = Texture::ERROR_NONE;

   if(memcmp(uTGAcompare, &tgaheader.Header, sizeof(tgaheader.Header)) == 0)
   {
      err = LoadUncompressedTGA(texture, filename, &src);
   }
   else if(memcmp(cTGAcompare, &tgaheader.Header, sizeof(tgaheader.Header)) == 0)
   {
      err = LoadCompressedTGA(texture, filename, &src);
   }
   else
   {
      fprintf( stderr, "Unknown file type (not compressed or uncompressed TGA\n" );
      return Texture::ERROR_BAD_DATA;
   }

   return err;
}

list<string> TgaTextureFilter::getReadTypes()
{
   list<string> rval;

   list<string>::iterator it;
   for ( it = m_read.begin(); it != m_read.end(); it++ )
   {
      rval.push_back( string("*.") + *it );
   }

   return rval;
}

list<string> TgaTextureFilter::getWriteTypes()
{
   list<string> rval;

   list<string>::iterator it;
   for ( it = m_write.begin(); it != m_write.end(); it++ )
   {
      rval.push_back( string("*.") + *it );
   }

   return rval;
}

bool SourceHasError( DataSource * src )
{
   if ( src->unexpectedEof() )
      return true;
   if ( src->getErrno() != 0 )
      return true;

   return false;
}

Texture::ErrorE SourceGetError( DataSource * src )
{
   if ( src->unexpectedEof() )
      return Texture::ERROR_UNEXPECTED_EOF;
   if ( src->getErrno() != 0 )
      return Texture::ERROR_FILE_READ;

   return Texture::ERROR_NONE;
}

Texture::ErrorE LoadUncompressedTGA(Texture * texture, const char * filename, DataSource * src)
{
   log_debug( "loading uncompressed TGA\n" );

   src->read( tga.width );
   src->read( tga.height );
   src->read( tga.bpp );
   src->read( tga.reserved );

   if ( SourceHasError( src ) )
      return SourceGetError( src );

   texture->m_width  = tga.width;
   texture->m_height = tga.height;

   uint32_t width  = texture->m_width;
   uint32_t height = texture->m_height;
   uint8_t bpp     = tga.bpp;

   if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))
   {
      fprintf( stderr, "Invalid texture information" );
      return Texture::ERROR_BAD_DATA;
   }

   log_debug( "tga size: %d x %d, %d bbp\n", width, height, bpp );

   bool hasAlpha = (bpp == 32);
   log_debug( "Alpha channel: %s\n", hasAlpha ? "present" : "not present" );

   uint8_t bytespp = (bpp / 8);
   uint32_t imageSize  = (bytespp * width * height);
   texture->m_data = new uint8_t[imageSize];
   texture->m_format = hasAlpha ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB;

   log_debug( "image size = %d\n", imageSize );

   if ( !src->readBytes(texture->m_data, imageSize) )
   {
      return SourceGetError( src );
   }

   for ( uint32_t n = 0; n < imageSize; n += bytespp )
   {
      uint8_t temp = texture->m_data[n + 0];
      texture->m_data[n + 0] = texture->m_data[n + 2];
      texture->m_data[n + 2] = temp;
   }

   return Texture::ERROR_NONE;
}

Texture::ErrorE LoadCompressedTGA(Texture * texture, const char * filename, DataSource * src )
{ 
   log_debug( "loading compressed TGA\n" );

   src->read( tga.width );
   src->read( tga.height );
   src->read( tga.bpp );
   src->read( tga.reserved );

   if ( SourceHasError( src ) )
      return SourceGetError( src );

   texture->m_width  = tga.width;
   texture->m_height = tga.height;

   uint32_t width  = texture->m_width;
   uint32_t height = texture->m_height;
   uint8_t bpp     = tga.bpp;

   if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))
   {
      fprintf( stderr, "Invalid texture information" );
      return Texture::ERROR_BAD_DATA;
   }

   bool hasAlpha = (bpp == 32);
   log_debug( "Alpha channel: %s\n", hasAlpha ? "present" : "not present" );

   log_debug( "tga size: %d x %d, %d bbp\n", width, height, bpp );

   uint8_t bytespp = (bpp / 8);
   uint32_t imageSize  = (bytespp * width * height);
   texture->m_data = new uint8_t[imageSize];

   uint32_t pixelcount = height * width;
   uint32_t currentpixel = 0;
   uint32_t currentbyte = 0;
   uint8_t colorbuffer[4];

   texture->m_format = hasAlpha ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB;

   do
   {
      uint8_t chunkheader = 0;
      if ( !src->read(chunkheader) )
         return SourceGetError( src );

      if ( chunkheader < 128 )
      {
         chunkheader++;
         for ( short counter = 0; counter < chunkheader && currentpixel < pixelcount; counter++)
         {
            if ( !src->readBytes( colorbuffer, bytespp ) )
               return SourceGetError( src );

            texture->m_data[currentbyte + 0] = colorbuffer[2];        
            texture->m_data[currentbyte + 1] = colorbuffer[1];
            texture->m_data[currentbyte + 2] = colorbuffer[0];
            if ( hasAlpha )
            {
               texture->m_data[currentbyte + 3] = colorbuffer[3];
            }

            currentbyte += bytespp;
            currentpixel++;
         }
      }
      else
      {
         chunkheader -= 127;
         if ( !src->readBytes( colorbuffer, bytespp ) )
            return SourceGetError( src );

         for(short counter = 0; counter < chunkheader && currentpixel < pixelcount; counter++)
         {
            texture->m_data[currentbyte + 0] = colorbuffer[2];        
            texture->m_data[currentbyte + 1] = colorbuffer[1];
            texture->m_data[currentbyte + 2] = colorbuffer[0];
            if ( hasAlpha )
            {
               texture->m_data[currentbyte + 3] = colorbuffer[3];
            }

            currentbyte += bytespp;
            currentpixel++;
         }
      }
   } while(currentpixel < pixelcount);

   log_debug( "pixel count = %d, current pixel = %d\n", pixelcount, currentpixel );

   log_debug( "image size = %d\n", imageSize );

   return Texture::ERROR_NONE;
}

Texture::ErrorE OldLoadUncompressedTGA(Texture *, const char *, FILE *);	// Load an Uncompressed file
Texture::ErrorE OldLoadCompressedTGA(Texture *, const char *, FILE *);		// Load a Compressed file

OldTgaTextureFilter::OldTgaTextureFilter()
{
   m_read.push_back( "TGA" );
   m_write.push_back( "TGA" );
}

OldTgaTextureFilter::~OldTgaTextureFilter()
{
}

bool OldTgaTextureFilter::canRead( const char * filename )
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

Texture::ErrorE OldTgaTextureFilter::readFile(Texture * texture, const char * filename)
{
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

   if(fp == NULL)
   {
      return Texture::ERROR_NO_FILE;
   }

   if(fread(&tgaheader.Header, sizeof(tgaheader.Header), 1, fp) < 1)
   {
      if(fp != NULL)
      {
         fclose(fp);
      }
      return Texture::ERROR_BAD_DATA;
   }

   Texture::ErrorE err = Texture::ERROR_NONE;

   if(memcmp(uTGAcompare, &tgaheader.Header, sizeof(tgaheader.Header)) == 0)
   {
      err = OldLoadUncompressedTGA(texture, filename, fp);
   }
   else if(memcmp(cTGAcompare, &tgaheader.Header, sizeof(tgaheader.Header)) == 0)
   {
      err = OldLoadCompressedTGA(texture, filename, fp);
   }
   else
   {
      fprintf( stderr, "Unknown file type (not compressed or uncompressed TGA\n" );
      fclose(fp);
      return Texture::ERROR_BAD_DATA;
   }

   return err;
}

list<string> OldTgaTextureFilter::getReadTypes()
{
   list<string> rval;

   list<string>::iterator it;
   for ( it = m_read.begin(); it != m_read.end(); it++ )
   {
      rval.push_back( string("*.") + *it );
   }

   return rval;
}

list<string> OldTgaTextureFilter::getWriteTypes()
{
   list<string> rval;

   list<string>::iterator it;
   for ( it = m_write.begin(); it != m_write.end(); it++ )
   {
      rval.push_back( string("*.") + *it );
   }

   return rval;
}

void OldTgaTextureFilter::read( uint8_t & val, FILE * fp )
{
   fread( &val, sizeof(val), 1, fp );
}

void OldTgaTextureFilter::read( uint16_t & val, FILE * fp )
{
   fread( &val, sizeof(val), 1, fp );
   val = ltoh_16( val );
}

void OldTgaTextureFilter::readBytes( void * buf, size_t len, FILE * fp )
{
   fread( buf, len, 1, fp );
}

Texture::ErrorE OldLoadUncompressedTGA(Texture * texture, const char * filename, FILE * fp)
{
   log_debug( "loading uncompressed TGA\n" );

   OldTgaTextureFilter::read( tga.width, fp );
   OldTgaTextureFilter::read( tga.height, fp );
   OldTgaTextureFilter::read( tga.bpp, fp );
   OldTgaTextureFilter::read( tga.reserved, fp );

   texture->m_width  = tga.width;
   texture->m_height = tga.height;

   uint32_t width  = texture->m_width;
   uint32_t height = texture->m_height;
   uint8_t bpp     = tga.bpp;

   if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))
   {
      fprintf( stderr, "Invalid texture information" );
      fclose(fp);
      return Texture::ERROR_BAD_DATA;
   }

   log_debug( "tga size: %d x %d, %d bbp\n", width, height, bpp );

   bool hasAlpha = (bpp == 32);
   log_debug( "Alpha channel: %s\n", hasAlpha ? "present" : "not present" );

   uint8_t bytespp = (bpp / 8);
   uint32_t imageSize  = (bytespp * width * height);
   texture->m_data = new uint8_t[imageSize];
   texture->m_format = hasAlpha ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB;

   log_debug( "image size = %d\n", imageSize );

   if( fread(texture->m_data, sizeof(uint8_t), imageSize, fp) != imageSize )
   {
      fprintf( stderr, "Could not read image data" );
      fclose(fp);
      return Texture::ERROR_UNEXPECTED_EOF;
   }

   for ( uint32_t n = 0; n < imageSize; n += bytespp )
   {
      uint8_t temp = texture->m_data[n + 0];
      texture->m_data[n + 0] = texture->m_data[n + 2];
      texture->m_data[n + 2] = temp;
   }

   fclose(fp);
   return Texture::ERROR_NONE;
}

Texture::ErrorE OldLoadCompressedTGA(Texture * texture, const char * filename, FILE * fp)
{ 
   log_debug( "loading compressed TGA\n" );

   OldTgaTextureFilter::read( tga.width, fp );
   OldTgaTextureFilter::read( tga.height, fp );
   OldTgaTextureFilter::read( tga.bpp, fp );
   OldTgaTextureFilter::read( tga.reserved, fp );

   texture->m_width  = tga.width;
   texture->m_height = tga.height;

   uint32_t width  = texture->m_width;
   uint32_t height = texture->m_height;
   uint8_t bpp     = tga.bpp;

   if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))
   {
      fprintf( stderr, "Invalid texture information" );
      fclose(fp);
      return Texture::ERROR_BAD_DATA;
   }

   bool hasAlpha = (bpp == 32);
   log_debug( "Alpha channel: %s\n", hasAlpha ? "present" : "not present" );

   log_debug( "tga size: %d x %d, %d bbp\n", width, height, bpp );

   uint8_t bytespp = (bpp / 8);
   uint32_t imageSize  = (bytespp * width * height);
   texture->m_data = new uint8_t[imageSize];

   uint32_t pixelcount = height * width;
   uint32_t currentpixel = 0;
   uint32_t currentbyte = 0;
   uint8_t colorbuffer[4];

   texture->m_format = hasAlpha ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB;

   do
   {
      uint8_t chunkheader = 0;

      if(fread(&chunkheader, sizeof(uint8_t), 1, fp) == 0)
      {
         fprintf( stderr, "Could not read RLE header" );
         fclose(fp);
         return Texture::ERROR_UNEXPECTED_EOF;
      }

      if(chunkheader < 128)
      {
         chunkheader++;
         for(short counter = 0; counter < chunkheader && currentpixel < pixelcount; counter++)
         {
            if(fread(colorbuffer, 1, bytespp, fp) != bytespp) 
            {
               fclose( fp );
               return Texture::ERROR_UNEXPECTED_EOF;
            }

            texture->m_data[currentbyte + 0] = colorbuffer[2];        
            texture->m_data[currentbyte + 1] = colorbuffer[1];
            texture->m_data[currentbyte + 2] = colorbuffer[0];
            if ( hasAlpha )
            {
               texture->m_data[currentbyte + 3] = colorbuffer[3];
            }

            currentbyte += bytespp;
            currentpixel++;
         }
      }
      else
      {
         chunkheader -= 127;
         if(fread(colorbuffer, 1, bytespp, fp) != bytespp)
         { 
            fclose( fp );
            return Texture::ERROR_UNEXPECTED_EOF;
         }

         for(short counter = 0; counter < chunkheader && currentpixel < pixelcount; counter++)
         {
            texture->m_data[currentbyte + 0] = colorbuffer[2];        
            texture->m_data[currentbyte + 1] = colorbuffer[1];
            texture->m_data[currentbyte + 2] = colorbuffer[0];
            if ( hasAlpha )
            {
               texture->m_data[currentbyte + 3] = colorbuffer[3];
            }

            currentbyte += bytespp;
            currentpixel++;
         }
      }
   } while(currentpixel < pixelcount);

   log_debug( "pixel count = %d, current pixel = %d\n", pixelcount, currentpixel );

   log_debug( "image size = %d\n", imageSize );

   fclose(fp);
   return Texture::ERROR_NONE;
}

