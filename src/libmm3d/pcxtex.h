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


#ifndef __PCXTEX_H
#define __PCXTEX_H

#include "texmgr.h"
#include "datasource.h"

#include <stdio.h>

class PcxTextureFilter : public TextureFilter
{
   public:
      PcxTextureFilter();
      virtual ~PcxTextureFilter();

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      Texture::ErrorE readFile( Texture * texture, const char * filename );

      bool canRead( const char * filename );

   protected:
      Texture::ErrorE load_image ( const char *filename );
      void   load_1     (DataSource & src, int    width, int    height, uint8_t  *buffer, int    bytes);
      void   load_4     (DataSource & src, int    width, int    height, uint8_t  *buffer, int    bytes);
      void   load_8     (DataSource & src, int    width, int    height, uint8_t  *buffer, int    bytes);
      void   load_24    (DataSource & src, int    width, int    height, uint8_t  *buffer, int    bytes);
      void   readline   (DataSource & src, uint8_t *buffer, int    bytes);

      /*
      bool   save_image (char  *filename, int32_t  image, int32_t  layer);
      void   save_8     (FILE   *fp, int    width, int    height, unsigned char *buffer);
      void   save_24    (FILE   *fp, int    width, int    height, unsigned char *buffer);
      void   writeline  (FILE   *fp, unsigned char *buffer, int    bytes);
      */

      uint8_t m_palette[256][3];
      Texture * m_texture;

};

#endif //  __PCXTEX_H
