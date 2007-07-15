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

#ifndef __BINARYFILTER_H
#define __BINARYFILTER_H

#include "modelfilter.h"

#include <string>

#include <stdint.h>

class BinaryFilter : public ModelFilter
{
   public:
      BinaryFilter();
      virtual ~BinaryFilter();

      enum _FilterEndianness_e
      {
         FE_Little,
         FE_Big
      };
      typedef enum _FilterEndianness_e FilterEndiannessE;
      void setEndianness( FilterEndiannessE e );

   protected:

      uint8_t  readU1();
      uint16_t readU2();
      uint32_t readU4();

      int8_t   readI1();
      int16_t  readI2();
      int32_t  readI4();

      float    readF4();
      size_t   readFixedString( char * dest, size_t len );
      size_t   readAsciizString( char * dest, size_t len );

      size_t   writeU1( uint8_t );
      size_t   writeU2( uint16_t );
      size_t   writeU4( uint32_t );

      size_t   writeI1( int8_t );
      size_t   writeI2( int16_t );
      size_t   writeI4( int32_t );

      size_t   writeF4( float );
      size_t   writeFixedString(char * val, size_t len);
      size_t   writeAsciizString(char * val);

      Model      * m_model;

      size_t       m_fileLen;

      uint8_t    * m_fileBuf;
      uint8_t    * m_bufPos;

      FILE * m_fp;

      FilterEndiannessE m_endianness;
};
#endif // __BINARYFILTER_H
