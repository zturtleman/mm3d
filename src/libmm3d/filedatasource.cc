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


#include "filedatasource.h"
#include "misc.h"

#include <errno.h>

#ifdef WIN32
FileDataSource::FileDataSource( const char * filename )
   : m_mustClose( false )
{
   if ( filename == NULL || filename[0] == '\0' )
   {
      setErrno( EINVAL );
      return;
   }

   std::wstring wideString = utf8PathToWide( filename );
   if ( wideString.empty() )
   {
      setErrno( EINVAL );
      return;
   }

   m_handle = CreateFileW( &wideString[0], GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
   if ( m_handle == INVALID_HANDLE_VALUE || m_handle == NULL )
   {
      m_handle = NULL;

      if ( GetLastError() == ERROR_ACCESS_DENIED )
      {
         setErrno( EACCES );
      }
      else
      {
         setErrno( ENOENT );
      }
      return;
   }

   m_mustClose = true;

   LARGE_INTEGER length;
   if ( !GetFileSizeEx( m_handle, &length ) )
   {
      // GetLastError()
      setErrno( EPERM );
      return;
   }

   // FIXME: what if file size is too large for size_t.
   setFileSize( length.QuadPart );
}

FileDataSource::~FileDataSource()
{
   if ( m_mustClose )
      close();
}

void FileDataSource::internalClose()
{
   if ( m_handle != NULL )
   {
      CloseHandle( m_handle );
      m_handle = NULL;
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

   LARGE_INTEGER li;
   li.QuadPart = offset;

   li.LowPart = SetFilePointer( m_handle, li.LowPart, &li.HighPart, FILE_BEGIN );
   if ( li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
   {
      setErrno( EPERM );
      return false;
   }

   DWORD read;
   if ( ReadFile( m_handle, m_buf, BUF_SIZE, &read, NULL ) == FALSE )
   {
      // GetLastError()
      setErrno( EPERM );
      return false;
   }

   *buf = m_buf;
   *bufLen = read;
   return true;
}
#else
FileDataSource::FileDataSource( const char * filename )
   : m_mustClose( false )
{
   if ( filename == NULL || filename[0] == '\0' )
   {
      setErrno( EINVAL );
      return;
   }

   m_fp = fopen( filename, "rb" );
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
#endif // WIN32

