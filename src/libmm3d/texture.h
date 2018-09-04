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


#ifndef __TEXTURE_H
#define __TEXTURE_H

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

class Texture
{
   public:
      enum _Error_e
      {
         ERROR_NONE = 0,
         ERROR_NO_FILE,
         ERROR_NO_ACCESS,
         ERROR_FILE_OPEN,
         ERROR_FILE_READ,
         ERROR_FILE_WRITE,
         ERROR_BAD_MAGIC,
         ERROR_UNSUPPORTED_VERSION,
         ERROR_BAD_DATA,
         ERROR_UNEXPECTED_EOF,
         ERROR_UNSUPPORTED_OPERATION,
         ERROR_BAD_ARGUMENT,
         ERROR_UNKNOWN
      };
      typedef enum _Error_e ErrorE;

      enum _Format_e
      {
         FORMAT_RGB,
         FORMAT_RGBA
      };
      typedef enum _Format_e FormatE;

      struct _CompareResult_t
      {
         bool     comparable;
         unsigned pixelCount;
         unsigned matchCount;
         unsigned fuzzyCount;
      };
      typedef struct _CompareResult_t CompareResultT;

      Texture();
      virtual ~Texture();

      static bool compare( Texture * t1, Texture * t2, CompareResultT * res, unsigned fuzzyValue );

      bool compare( Texture * tex, CompareResultT * res, unsigned fuzzyValue );

      static const char * errorToString( Texture::ErrorE );

      void removeOpaqueAlphaChannel();

      char    * m_name;
      char    * m_filename;
      bool      m_isBad;
      int       m_height;
      int       m_width;
      FormatE   m_format;
      uint8_t * m_data;

      int       m_origWidth;
      int       m_origHeight;

      time_t    m_loadTime;

      static int s_allocated;
};

#endif // __TEXTURE_H
