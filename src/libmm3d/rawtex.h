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


#ifndef __RAWTEX_H
#define __TGATEX_H

#include <stdio.h>
#include <stdint.h>
#include "texmgr.h"

class RawTextureFilter : public TextureFilter
{
   public:
      RawTextureFilter();
      virtual ~RawTextureFilter();

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      Texture::ErrorE readFile( Texture * texture, const char * filename );
      virtual bool canRead( const char * filename );

   protected:
      std::list< std::string > m_read;
      std::list< std::string > m_write;
};

#endif // TGATEX_H
