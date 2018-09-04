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


// This file was heavily modified from the gimp PCX plugin.
//
//------------------------------------------------------------------
// Original header comments follow:
//
/* pcx.c GIMP plug-in for loading & saving PCX files */

/* This code is based in parts on code by Francisco Bustamante, but the
   largest portion of the code has been rewritten and is now maintained
   occasionally by Nick Lamb njl195@zepler.org.uk */

/* New for 1998 -- Load 1, 4, 8 & 24 bit PCX files */
/*              -- Save 8 & 24 bit PCX files */
/* 1998-01-19 - fixed some endianness problems (Raphael Quinet) */
/* 1998-02-05 - merged patch with "official" tree, some tidying up (njl) */
/* 1998-05-17 - changed email address, more tidying up (njl) */
/* 1998-05-31 - //log_error (njl) */

/* Please contact me if you can't use your PCXs with this tool, I want
   The GIMP to have the best file filters on the planet */

//
// End original comments for gimp PCX plugin
//------------------------------------------------------------------
//

#include "pcxtex.h"
#include "mm3dconfig.h"
#include "endianconfig.h"
#include "log.h"
#include "filedatasource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

using std::list;
using std::string;

PcxTextureFilter::PcxTextureFilter()
   : m_texture( NULL )
{
}

PcxTextureFilter::~PcxTextureFilter()
{
}

Texture::ErrorE PcxTextureFilter::readFile( Texture * texture, const char * filename )
{
   if ( texture && filename )
   {
      m_texture = texture;
      Texture::ErrorE rval = load_image( filename );
      m_texture = NULL;

      texture->m_format = Texture::FORMAT_RGB;
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

      return rval;
   }
   else
   {
      return Texture::ERROR_BAD_ARGUMENT;
   }
}

static uint8_t _mono[6]= { 0, 0, 0, 255, 255, 255 };

static struct
{
   uint8_t manufacturer;
   uint8_t version;
   uint8_t compression;
   uint8_t bpp;
   int16_t x1, y1;
   int16_t x2, y2;
   int16_t hdpi;
   int16_t vdpi;
   uint8_t colormap[48];
   uint8_t reserved;
   uint8_t planes;
   int16_t bytesperline;
   int16_t color;
   uint8_t filler[58];
} pcx_header;

Texture::ErrorE PcxTextureFilter::load_image ( const char * filename ) 
{
   FileDataSource src( filename );
   int offset_x, offset_y;

   if ( src.errorOccurred() )
   {
      return errnoToTextureError( src.getErrno(), Texture::ERROR_FILE_OPEN );
   }

   src.read( pcx_header.manufacturer );
   src.read( pcx_header.version );
   src.read( pcx_header.compression );
   src.read( pcx_header.bpp );
   src.read( pcx_header.x1 );
   src.read( pcx_header.y1 );
   src.read( pcx_header.x2 );
   src.read( pcx_header.y2 );
   src.read( pcx_header.hdpi );
   src.read( pcx_header.vdpi );
   src.readBytes( pcx_header.colormap, sizeof(pcx_header.colormap) );
   src.read( pcx_header.reserved );
   src.read( pcx_header.planes );
   src.read( pcx_header.bytesperline );
   src.read( pcx_header.color );
   src.readBytes( pcx_header.filler, sizeof(pcx_header.filler) );

   if ( src.offset() != 128 )
   {
      return Texture::ERROR_BAD_MAGIC;
   }

   if (pcx_header.manufacturer != 10)
   {
      return Texture::ERROR_BAD_MAGIC;
   }

   offset_x =  (pcx_header.x1);
   offset_y =  (pcx_header.y1);
   m_texture->m_width =  (pcx_header.x2) - offset_x + 1;
   m_texture->m_height =  (pcx_header.y2) - offset_y + 1;

   m_texture->m_data = new uint8_t[ m_texture->m_width * m_texture->m_height * 3 ];

   if (pcx_header.planes == 1 && pcx_header.bpp == 1)
   {
      memcpy( m_palette, _mono, (2*3) );
      load_1 (src, m_texture->m_width, m_texture->m_height, m_texture->m_data,  (pcx_header.bytesperline));
   }
#if 0
   else if (pcx_header.planes == 4 && pcx_header.bpp == 1)
   {
      memcpy( m_palette, pcx_header.colormap, (16*3) );
      load_4(src, m_texture->m_width, m_texture->m_height, m_texture->m_data,  (pcx_header.bytesperline));
   }
#endif
   else if (pcx_header.planes == 1 && pcx_header.bpp == 8)
   {
      off_t pos = src.offset();
      src.seek( src.getFileSize() - (256*3) );
      src.readBytes( &m_palette[0][0], (256*3) );
      src.seek( pos );
      if ( src.errorOccurred() )
      {
         return errnoToTextureError( src.getErrno(), Texture::ERROR_FILE_READ );
      }
      load_8(src, m_texture->m_width, m_texture->m_height, m_texture->m_data,  (pcx_header.bytesperline));
   }
   else if (pcx_header.planes == 3 && pcx_header.bpp == 8)
   {
      load_24(src, m_texture->m_width, m_texture->m_height, m_texture->m_data,  (pcx_header.bytesperline));
   }
   else
   {
      return Texture::ERROR_UNSUPPORTED_VERSION;
   }

   return Texture::ERROR_NONE;
}

bool PcxTextureFilter::canRead( const char * filename )
{
   if ( filename == NULL )
   {
      return false;
   }

   unsigned len = strlen( filename );

   if ( strcasecmp( &filename[len-4], ".pcx" ) == 0 )
   {
      return true;
   }

   return false;
}

list<string> PcxTextureFilter::getReadTypes()
{
   list<string> rval;

   rval.push_back( "*.PCX" );

   return rval;
}

list<string> PcxTextureFilter::getWriteTypes()
{
   list<string> rval;

   //rval.push_back( "*.PCX" );

   return rval;
}

void PcxTextureFilter::load_8 (DataSource & src, int   m_width, int   m_height, uint8_t *buffer, int   bytes) 
{
   int x, y;
   uint8_t *line;
   line = (uint8_t *) malloc (bytes);
   uint8_t * row;

   for (y = m_height - 1; y >= 0; --y) 
   {
      row = &buffer[ y * (m_width*3) ];
      readline (src, line, bytes);
      for (x = 0; x < m_width; ++x) 
      {
         memcpy( &row[x*3], m_palette[ line[x] ], 3 );
      }
   }

   free (line);
}

void PcxTextureFilter::load_24 (DataSource & src, int   m_width, int   m_height, uint8_t *buffer, int   bytes) 
{
   int x, y, c;
   uint8_t * line;
   line = (uint8_t *) malloc (bytes * 3);
   uint8_t * row;

   for (y = m_height - 1; y >= 0; --y) 
   {
      row = &buffer[ y * (m_width*3) ];
      for (c = 0; c < 3; ++c) 
      {
         readline (src, line, bytes);
         for (x = 0; x < m_width; ++x) 
         {
            row[x * 3 + c] = line[x];
         }
      }
   }

   free (line);
}

void PcxTextureFilter::load_1 (DataSource & src, int   m_width, int   m_height, uint8_t *buffer, int   bytes) 
{
   int x, y;
   uint8_t *line;
   line = (uint8_t *) malloc (bytes);
   uint8_t * row;

   for (y = m_height - 1; y >= 0; --y) 
   {
      row = &buffer[ y * (m_width*3) ];
      readline (src, line, bytes);
      for (x = 0; x < m_width; ++x) 
      {
         if (line[x / 8] & (128 >> (x % 8)))
         {
            memcpy( &row[x*3], m_palette[ 1 ], 3 );
         }
         else
         {
            memcpy( &row[x*3], m_palette[ 0 ], 3 );
         }
      }
   }

   free (line);
}

void PcxTextureFilter::load_4 (DataSource & src, int   m_width, int   m_height, uint8_t *buffer, int   bytes) 
{
   // TODO implement this if I ever want it to work
   /*
   int x, y, c;
   uint8_t *line;
   line= (uint8_t *) malloc (bytes);
   uint8_t * row;

   for (y = m_height - 1; y >= 0; --y) 
   {
      row = &buffer[ y * (m_width*3) ];
      for (x = 0; x < m_width; ++x) row[x] = 0;
      for (c = 0; c < 4; ++c) 
      {
         readline(fp, line, bytes);
         for (x = 0; x < m_width; ++x) 
         {
            if (line[x / 8] & (128 >> (x % 8)))
               buffer[x] += (1 << c);
         }
      }
   }

   free (line);
   */
}

void PcxTextureFilter::readline (DataSource & src, uint8_t *buffer, int    bytes) 
{
   static uint8_t count = 0, value = 0;

   if (pcx_header.compression) 
   {
      while (bytes--) 
      {
         if (count == 0) 
         {
            src.read(value);
            if (value < 0xc0) 
            {
               count = 1;
            } 
            else 
            {
               count = value - 0xc0;
               src.read(value);
            }
         }
         count--;
         *(buffer++) = value;
      }
   }
   else
   {
      src.readBytes(buffer, bytes);
   }
}

/*
bool save_image (char   *filename, 
      int32_t  image, 
      int32_t  layer) 
{
   FILE *fp;
   uint8_t *cmap= 0, *pixels;
   int offset_x, offset_y, m_width, m_height;
   int colors, i;

   //m_width = drawable->m_width;
   //m_height = drawable->m_height;

   pcx_header.manufacturer = 0x0a;
   pcx_header.version = 5;
   pcx_header.compression = 1;

   uint32_t drawable_type = 0;
   switch (drawable_type) 
   {
      case 0: // GIMP_INDEXED_IMAGE:
         pcx_header.bpp = 8;
         pcx_header.bytesperline =  (m_width);
         pcx_header.planes = 1;
         pcx_header.color =  (1);
         break;

      case 1: // GIMP_RGB_IMAGE:
         pcx_header.bpp = 8;
         pcx_header.planes = 3;
         pcx_header.color =  (1);
         pcx_header.bytesperline =  (m_width);
         break;

      case 2: // GIMP_GRAY_IMAGE:
         pcx_header.bpp = 8;
         pcx_header.planes = 1;
         pcx_header.color =  (2);
         pcx_header.bytesperline =  (m_width);
         break;

      default:
         //log_error ("PCX Can't save this image type\nFlatten your image\n");
         return false;
         break;
   }

   if ((fp = fopen(filename, "wb")) == NULL) 
   {
      //log_error ("PCX Can't open \n%s", filename);
      return false;
   }

   pixels = (uint8_t *) malloc (m_width * m_height * pcx_header.planes);

   pcx_header.x1 =  (offset_x);
   pcx_header.y1 =  (offset_y);
   pcx_header.x2 =  (offset_x + m_width - 1);
   pcx_header.y2 =  (offset_y + m_height - 1);

   pcx_header.hdpi =  (300);
   pcx_header.vdpi =  (300);
   pcx_header.reserved = 0;

   fwrite (&pcx_header, 128, 1, fp);

   switch (drawable_type) 
   {
      case 0: // GIMP_INDEXED_IMAGE:
         save_8 (fp, m_width, m_height, pixels);
         fputc (0x0c, fp);
         fwrite (cmap, colors, 3, fp);
         for (i = colors; i < 256; i++) 
         {
            fputc (0, fp); fputc (0, fp); fputc (0, fp);
         }
         break;
      case 1: // GIMP_RGB_IMAGE:
         save_24 (fp, m_width, m_height, pixels);
         break;
      case 2: // GIMP_GRAY_IMAGE:
         save_8 (fp, m_width, m_height, pixels);
         fputc (0x0c, fp);
         for (i = 0; i < 256; i++) 
         {
            fputc ((uint8_t) i, fp); fputc ((uint8_t) i, fp); fputc ((uint8_t) i, fp);
         }
         break;
      default:
         //log_error ("Can't save this image as PCX\nFlatten your image\n");
         return false;
         break;
   }

   free (pixels);

   fclose (fp);
   return true;
}

static void
save_8 (FILE   *fp, 
      int    m_width, 
      int    m_height, 
      uint8_t *buffer) 
{
   int row;

   for (row = 0; row < m_height; ++row) 
   {
      writeline (fp, buffer, m_width);
      buffer += m_width;
   }
}

static void
save_24 (FILE   *fp, 
      int    m_width, 
      int    m_height, 
      uint8_t *buffer) 
{
   int x, y, c;
   uint8_t *line;
   line = (uint8_t *) malloc (m_width);

   for (y = 0; y < m_height; ++y) 
   {
      for (c = 0; c < 3; ++c) 
      {
         for (x = 0; x < m_width; ++x) 
         {
            line[x] = buffer[(3*x) + c];
         }
         writeline (fp, line, m_width);
      }
      buffer += m_width * 3;
   }
   free (line);
}

static void
writeline (FILE   *fp, 
      uint8_t *buffer, 
      int    bytes) 
{
   uint8_t value, count;
   uint8_t *finish = buffer+ bytes;

   while (buffer < finish) 
   {
      value = *(buffer++);
      count = 1;

      while (buffer < finish && count < 63 && *buffer == value) 
      {
         count++; buffer++;
      }

      if (value < 0xc0 && count == 1) 
      {
         fputc (value, fp);
      } 
      else 
      {
         fputc (0xc0 + count, fp);
         fputc (value, fp);
      }
   }
}
*/

#ifdef PLUGIN

static PcxTextureFilter * s_filter = NULL;

//------------------------------------------------------------------
// Plugin functions
//------------------------------------------------------------------

PLUGIN_API bool plugin_init()
{
   if ( s_filter == NULL )
   {
      s_filter = new PcxTextureFilter();
      TextureManager * texmgr = TextureManager::getInstance();
      texmgr->registerTextureFilter( s_filter );
   }
   log_debug( "PCX texture filture plugin initialized\n" );
   return true;
}

PLUGIN_API bool plugin_uninit()
{
   s_filter = NULL; // TextureManager deletes filters
   log_debug( "PCX texture filture plugin uninitialized\n" );
   return true;
}

PLUGIN_API const char * plugin_mm3d_version()
{
   return VERSION_STRING;
}

PLUGIN_API const char * plugin_version()
{
   return "1.0.0";
}

PLUGIN_API const char * plugin_desc()
{
   return "PCX texture filture";
}

#endif // PLUGIN
