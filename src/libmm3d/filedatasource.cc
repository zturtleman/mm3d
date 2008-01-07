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


#include "filedatasource.h"

#include <errno.h>

FileDataSource::FileDataSource( FILE * fp )
   : m_fp( fp ),
     m_unexpectedEof( false ),
     m_errno( 0 )
{
   if ( fp == NULL )
   {
      sendErrno( EBADF );
      return;
   }

   int rval = 0;
   long size = -1;

   rval = fseek( fp, 0, SEEK_END );

   size = ftell( fp );

   if ( rval != 0 || size < 0 )
   {
      sendErrno( EBADF );
      return;
   }

   fseek( fp, 0, SEEK_SET );

   setFileSize( size );
}

FileDataSource::~FileDataSource()
{
}

bool FileDataSource::internalReadAt( off_t offset, uint8_t ** buf, size_t * bufLen )
{
   // TODO should assert on buf and bufLen

   // If we had an error, just keep returning an error
   if ( m_unexpectedEof || m_errno != 0 )
      return false;

   if ( offset > (off_t) getFileSize() )
   {
      m_unexpectedEof = true;
      setUnexpectedEof( true );
      return false;
   }

   if ( fseek( m_fp, offset, SEEK_SET ) != 0 )
   {
      sendErrno( errno );
      return false;
   }

   size_t bytes = fread( m_buf, 1, BUF_SIZE, m_fp );

   if ( ferror( m_fp ) != 0 )
   {
      sendErrno( errno );
      return false;
   }

   *buf = m_buf;
   *bufLen = bytes;
   return true;
}

void FileDataSource::sendErrno( int err )
{
   m_errno = err;
   setErrno( m_errno );
}

