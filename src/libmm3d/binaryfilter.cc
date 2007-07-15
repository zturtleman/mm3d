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

#include "binaryfilter.h"

#include "model.h"
#include "texture.h"
#include "texmgr.h"
#include "log.h"
#include "binutil.h"
#include "misc.h"
#include "filtermgr.h"

#include "mm3dport.h"
#include "endianconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <vector>

BinaryFilter::BinaryFilter()
{
}

BinaryFilter::~BinaryFilter()
{
}

uint8_t BinaryFilter::readU1()
{
   uint8_t val = 0;
   memcpy( &val, m_bufPos, sizeof(val) );
   m_bufPos += sizeof( val );

   return val;
}

uint16_t BinaryFilter::readU2()
{
   uint16_t val = 0;
   memcpy( &val, m_bufPos, sizeof(val) );
   m_bufPos += sizeof( val );

   if ( m_endianness == FE_Little )
   {
      val = ltoh_16( val );
   }
   else
   {
      val = ntohs( val );
   }

   return val;
}

uint32_t BinaryFilter::readU4()
{
   uint32_t val = 0;
   memcpy( &val, m_bufPos, sizeof(val) );
   m_bufPos += sizeof( val );

   if ( m_endianness == FE_Little )
   {
      val = ltoh_32( val );
   }
   else
   {
      val = ntohl( val );
   }

   return val;
}

int8_t BinaryFilter::readI1()
{
   int8_t val = 0;
   memcpy( &val, m_bufPos, sizeof( val ) );
   m_bufPos += sizeof( val );

   return val;
}

int16_t BinaryFilter::readI2()
{
   int16_t val = 0;
   memcpy( &val, m_bufPos, sizeof( val ) );
   m_bufPos += sizeof( val );

   if ( m_endianness == FE_Little )
   {
      val = ltoh_16( val );
   }
   else
   {
      val = ntohs( val );
   }

   return val;
}

int32_t BinaryFilter::readI4()
{
   int32_t val = 0;
   memcpy( &val, m_bufPos, sizeof( val ) );
   m_bufPos += sizeof( val );

   if ( m_endianness == FE_Little )
   {
      val = ltoh_32( val );
   }
   else
   {
      val = ntohl( val );
   }

   return val;
}

float BinaryFilter::readF4()
{
   float val = 0;
   memcpy( &val, m_bufPos, sizeof( val ) );
   m_bufPos += sizeof( val );

   if ( m_endianness == FE_Little )
   {
      val = ltoh_float( val );
   }
   else
   {
      uint32_t uval = 0;
      memcpy( &uval, &val, sizeof(val) );
      uval = ntohl( uval );
      memcpy( &val, &uval, sizeof(val) );
   }

   return val;
}

size_t BinaryFilter::readFixedString( char * dest, size_t len )
{
   strncpy( dest, (char *) m_bufPos, len );
   dest[ len - 1 ]= '\0';

   m_bufPos += len;
   return len;
}

size_t BinaryFilter::readAsciizString( char * dest, size_t len )
{
   strncpy( dest, (char *) m_bufPos, len );
   dest[ len - 1 ]= '\0';

   size_t bytes = 1; // including null terminator
   while ( *dest != '\0' )
   {
      dest++;
      bytes++;
   }

   m_bufPos += bytes;
   return bytes;
}

size_t BinaryFilter::writeI1( int8_t val )
{
   int8_t wval = val;
   return fwrite( &wval, sizeof( wval ), 1, m_fp );
}

size_t BinaryFilter::writeI2( int16_t val )
{
   int16_t wval = (m_endianness == FE_Little) ? htol_16( val ) : ntohs( val );
   return fwrite( &wval, sizeof( wval ), 1, m_fp );
}

size_t BinaryFilter::writeI4( int32_t val )
{
   int32_t wval = (m_endianness == FE_Little) ? htol_32( val ) : ntohl( val );
   return fwrite( &wval, sizeof( wval ), 1, m_fp );
}

size_t BinaryFilter::writeU1( uint8_t val )
{
   uint8_t wval = val;
   return fwrite( &wval, sizeof( wval ), 1, m_fp );
}

size_t BinaryFilter::writeU2( uint16_t val )
{
   uint16_t wval = (m_endianness == FE_Little) ? htol_16( val ) : ntohs( val );
   return fwrite( &wval, sizeof( wval ), 1, m_fp );
}

size_t BinaryFilter::writeU4( uint32_t val )
{
   uint32_t wval = (m_endianness == FE_Little) ? htol_32( val ) : ntohl( val );
   return fwrite( &wval, sizeof( wval ), 1, m_fp );
}

size_t BinaryFilter::writeF4( float val )
{
   float wval = 0;

   if ( m_endianness == FE_Little )
   {
      wval = htol_float( val );
   }
   else
   {
      uint32_t uval = 0;
      memcpy( &uval, &val, sizeof(uval) );
      uval = ntohl( uval );
      memcpy( &wval, &uval, sizeof(uval) );
   }
   return fwrite( &wval, sizeof( wval ), 1, m_fp );
}

size_t BinaryFilter::writeFixedString( char * src, size_t len )
{
   return fwrite( src, len, 1, m_fp );
}

size_t BinaryFilter::writeAsciizString( char * src )
{
   return fprintf( m_fp, "%s", src );
}


