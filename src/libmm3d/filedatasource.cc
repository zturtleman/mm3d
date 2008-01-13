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
     m_mustClose( false )
{
   if ( m_fp == NULL )
   {
      setErrno( EBADF );
      return;
   }

   if ( 0 != fseek( m_fp, 0, SEEK_END ) )
   {
      setErrno( errno );
      return;
   }


   long size = ftell( m_fp );
   if ( size < 0 )
   {
      setErrno( EBADF );
      return;
   }

   if ( 0 != fseek( m_fp, 0, SEEK_SET ) )
   {
      setErrno( errno );
      return;
   }

   setFileSize( size );
}

FileDataSource::FileDataSource( const char * filename )
   : m_mustClose( false )
{
   m_fp = fopen( filename, "r" );
   if ( m_fp == NULL )
   {
      setErrno( errno );
      return;
   }
   
   m_mustClose = true;

   if ( 0 != fseek( m_fp, 0, SEEK_END ) )
   {
      setErrno( errno );
      return;
   }

   long size = ftell( m_fp );
   if ( size < 0 )
   {
      setErrno( errno );
      return;
   }

   if ( 0 != fseek( m_fp, 0, SEEK_SET ) )
   {
      setErrno( errno );
      return;
   }

   setFileSize( size );
}

FileDataSource::~FileDataSource()
{
   if ( m_mustClose )
      close();
}

void FileDataSource::internalClose()
{
   if ( m_fp != NULL )
   {
      fclose( m_fp );
      m_fp = NULL;
   }
}

bool FileDataSource::internalReadAt( off_t offset, const uint8_t ** buf, size_t * bufLen )
{
   // TODO should assert on buf and bufLen

   // If we had an error, just keep returning an error
   if ( errorOccurred() )
      return false;

   if ( offset > (off_t) getFileSize() )
   {
      setUnexpectedEof( true );
      return false;
   }

   if ( fseek( m_fp, offset, SEEK_SET ) != 0 )
   {
      setErrno( errno );
      return false;
   }

   size_t bytes = fread( m_buf, 1, BUF_SIZE, m_fp );

   if ( ferror( m_fp ) != 0 )
   {
      setErrno( errno );
      return false;
   }

   *buf = m_buf;
   *bufLen = bytes;
   return true;
}

