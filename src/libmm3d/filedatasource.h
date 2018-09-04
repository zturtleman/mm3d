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


#ifndef FILEDATASOURCE_INC_H__
#define FILEDATASOURCE_INC_H__

#include "datasource.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32

// This class is a DataSource that uses a FILE* as the underlying input
// source. See the documentation in datasource.h for the DataSource API.
// The details below are specific to the FileDataSource and probably not
// of direct interest to anyone writing model or texture import filters.

class FileDataSource : public DataSource
{
   public:
      FileDataSource( const char * filename );
      virtual ~FileDataSource();

      void internalClose();

   protected:
      virtual bool internalReadAt( off_t offset, const uint8_t ** buf, size_t * bufLen );

   private:
      void sendErrno( int err );

      enum
      {
         BUF_SIZE = 128 * 1024  // Arbitrary
      };

#ifdef WIN32
      HANDLE m_handle;
#else
      FILE * m_fp;
#endif // WIN32
      bool m_mustClose;
      uint8_t m_buf[ BUF_SIZE ];
};

#endif // FILEDATASOURCE_INC_H__
