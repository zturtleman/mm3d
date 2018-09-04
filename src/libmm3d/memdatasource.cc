/*  Maverick Model 3D
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


#include "memdatasource.h"

#include <errno.h>

MemDataSource::MemDataSource( const uint8_t * buf, size_t bufSize )
   : m_buf( buf ),
     m_bufSize( bufSize )
{
   if ( m_buf == NULL )
   {
      setErrno( EBADF );
      return;
   }

   setFileSize( bufSize );
}

MemDataSource::~MemDataSource()
{
}

bool MemDataSource::internalReadAt( off_t offset, const uint8_t ** buf, size_t * bufLen )
{
   // TODO should assert on buf and bufLen

   // If we had an error, just keep returning an error
   if ( errorOccurred() )
      return false;

   if ( (size_t) offset > m_bufSize )
   {
      setUnexpectedEof( true );
      return false;
   }

   *buf = &m_buf[offset];
   *bufLen = m_bufSize - offset;
   return true;
}

