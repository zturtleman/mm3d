/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2008 Kevin Worcester
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


#include "datasource.h"

#include "endianconfig.h"

#include <arpa/inet.h>

DataSource::DataSource()
   : m_endian( LittleEndian ),
     m_buf( NULL ),
     m_fileSize( 0 ),
     m_bufLen( 0 ),
     m_bufOffset( 0 ),
     m_errorOccurred( false ),
     m_unexpectedEof( false ),
     m_errno( 0 ),
     m_endfunc16( ltoh_u16 ),
     m_endfunc32( ltoh_u32 ),
     m_endfuncfl( ltoh_float )
{
}

DataSource::~DataSource()
{
}

void DataSource::setEndianness( EndiannessE e )
{
   m_endian = e;

   if ( m_endian == LittleEndian )
   {
      m_endfunc16 = ltoh_u16;
      m_endfunc32 = ltoh_u32;
      m_endfuncfl = ltoh_float;
   }
   else
   {
#ifdef WIN32
      m_endfunc16 = btoh_u16;
      m_endfunc32 = btoh_u32;
#else  // !WIN32
      m_endfunc16 = ntohs;
      m_endfunc32 = ntohl;
#endif  // WIN32
      m_endfuncfl = btoh_float;
   }
}

void DataSource::setUnexpectedEof( bool o )
{
   if ( o )
      m_errorOccurred = true;
   m_unexpectedEof = o;
}

void DataSource::setErrno( int err )
{
   m_errorOccurred = true;
   m_errno = err;
}

bool DataSource::requireBytes( size_t bytes )
{
   if ( m_bufLen < bytes )
   {
      if ( !internalReadAt( m_bufOffset, &m_buf, &m_bufLen ) )
      {
         // TODO should probably have an assert here to make sure that
         // the actual source sets an error condition.
         return false;
      }

      if ( m_bufLen < bytes )
      {
         seek( m_fileSize );
         setUnexpectedEof( true );
         return false;
      }
   }

   return true;
}

void DataSource::advanceBytes( size_t bytes )
{
   m_buf += bytes;
   m_bufLen -= bytes;
   m_bufOffset += bytes;

   if ( m_bufLen == 0 )
   {
      m_buf = NULL;
   }
}

bool DataSource::seek( off_t offset )
{
   if ( (size_t) offset > m_fileSize )
   {
      setUnexpectedEof( true );
      return false;
   }

   m_bufOffset = offset;

   if ( (size_t) m_bufOffset == m_fileSize )
   {
      m_buf = NULL;
      return true;
   }

   if ( !internalReadAt( m_bufOffset, &m_buf, &m_bufLen ) )
   {
      // TODO should probably have an assert here to make sure that
      // the actual source set an error condition.
      return false;
   }

   return true;
}

bool DataSource::read( int8_t & val )
{
   if ( !requireBytes( sizeof(val) ) )
      return false;

   val = * (int8_t*) m_buf;

   advanceBytes( sizeof(val) );
   return true;
}

bool DataSource::read( uint8_t & val )
{
   if ( !requireBytes( sizeof(val) ) )
      return false;

   val = * (int8_t*) m_buf;

   advanceBytes( sizeof(val) );
   return true;
}

bool DataSource::read( int16_t & val )
{
   if ( !requireBytes( sizeof(val) ) )
      return false;

   val = m_endfunc16( * (uint16_t*) m_buf );
   advanceBytes( sizeof(val) );

   return true;
}

bool DataSource::read( uint16_t & val )
{
   if ( !requireBytes( sizeof(val) ) )
      return false;

   val = m_endfunc16( * (uint16_t*) m_buf );
   advanceBytes( sizeof(val) );

   return true;
}

bool DataSource::read( int32_t & val )
{
   if ( !requireBytes( sizeof(val) ) )
      return false;

   val = m_endfunc32( * (uint32_t*) m_buf );
   advanceBytes( sizeof(val) );

   return true;
}

bool DataSource::read( uint32_t & val )
{
   if ( !requireBytes( sizeof(val) ) )
      return false;

   val = m_endfunc32( * (uint32_t*) m_buf );
   advanceBytes( sizeof(val) );

   return true;
}

bool DataSource::read( float & val )
{
   if ( !requireBytes( sizeof(val) ) )
      return false;

   val = m_endfuncfl( * (float*) m_buf );
   advanceBytes( sizeof(val) );

   return true;
}

bool DataSource::fillBuffer()
{
   if ( m_bufLen == 0 )
   {
      if ( !internalReadAt( m_bufOffset, &m_buf, &m_bufLen ) )
         return false;

      if ( m_bufOffset == m_fileSize )
      {
         setUnexpectedEof( true );
         return false;
      }

      // TODO should probably assert on m_bufLen == 0
   }

   // There is now data in the buffer
   return true;
}

bool DataSource::readAsciiz( char * buf, size_t bufLen, bool * foundNull )
{
   if ( bufLen < 1 )
      return false;

   bool rval = readTo( '\0', buf, bufLen, foundNull );

   // No matter what is in the buffer, the last char must be null
   buf[ bufLen - 1] = '\0';

   return rval;
}

bool DataSource::readTo( char stopChar, char * buf, size_t bufLen, bool * foundChar )
{
   if ( foundChar != NULL )
      *foundChar = false;

   size_t bufOff = 0;

   // The only success condition is if we find stopChar. That happens
   // inside this loop.
   while ( !eof() && bufOff < bufLen )
   {
      if ( !fillBuffer() )
         return false;

      buf[bufOff] = m_buf[0];
      
      ++m_buf;
      ++m_bufOffset;
      --m_bufLen;

      if ( buf[ bufOff ] == stopChar )
      {
         if ( foundChar != NULL )
            *foundChar = true;

         return true;
      }

      bufOff++;
   }

   if ( eof() )
      setUnexpectedEof( true );

   return false;
}

bool DataSource::readBytes( uint8_t * buf, size_t bufLen )
{
   if ( bufLen > (size_t) (m_fileSize - m_bufOffset) )
   {
      seek( m_fileSize );
      setUnexpectedEof( true );
      return false;
   }

   size_t bufOff = 0;

   // The only success condition is if we find stopChar. That happens
   // inside this loop.
   while ( bufLen > 0 )
   {
      if ( !fillBuffer() )
         return false;

      size_t copySize = ( bufLen < m_bufLen ) ? bufLen : m_bufLen;

      memcpy( &buf[bufOff], m_buf, copySize );
      buf[bufOff] = m_buf[0];
      
      m_buf += copySize;
      m_bufOffset += copySize;
      m_bufLen -= copySize;

      bufOff += copySize;
      bufLen -= copySize;
   }

   return true;
}

