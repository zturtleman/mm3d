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


#include "filedatadest.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

FileDataDest::FileDataDest( FILE * fp, size_t startOffset )
   : m_fp( fp ),
     m_startOffset( startOffset ),
     m_mustClose( false )
{
   if ( m_fp == NULL )
   {
      sendErrno( EBADF );
      return;
   }

   // Don't count on the caller to put as the starting offset
   if ( fseek( m_fp, m_startOffset, SEEK_SET ) != 0 )
   {
      sendErrno( errno );
      return;
   }
}

FileDataDest::FileDataDest( const char * filename )
   : m_startOffset( 0 ),
     m_mustClose( false )
{
   if ( filename == NULL || filename[0] == '\0' )
   {
      setErrno( EINVAL );
      return;
   }

   m_fp = fopen( filename, "wb" );
   if ( m_fp == NULL )
   {
      sendErrno( errno );
      return;
   }

   m_mustClose = true;
}

FileDataDest::~FileDataDest()
{
   if ( m_mustClose )
      close();
}

void FileDataDest::internalClose()
{
   if ( m_fp != NULL )
   {
      fclose( m_fp );
      m_fp = NULL;
   }
}

void FileDataDest::sendErrno( int err )
{
   setErrno( err );
}

bool FileDataDest::internalSeek( off_t off )
{
   if ( errorOccurred() )
      return false;

   if ( m_fp == NULL )
   {
      sendErrno( EBADF );
      return false;
   }

   if ( fseek( m_fp, off + m_startOffset, SEEK_SET ) != 0 )
      return false;

   return true;
}

bool FileDataDest::internalWrite( const uint8_t * buf, size_t bufLen )
{
   if ( errorOccurred() )
      return false;

   if ( m_fp == NULL )
   {
      sendErrno( EBADF );
      return false;
   }

   if ( fwrite( buf, 1, bufLen, m_fp ) == 0 && bufLen != 0 )
   {
      sendErrno( errno );
      return false;
   }

   return true;
}

