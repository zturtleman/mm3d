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


#ifndef FILEDATADEST_INC_H__
#define FILEDATADEST_INC_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include "datadest.h"

// This class is a DataDest that uses a FILE* as the underlying output
// source. See the documentation in datadest.h for the DataDest API.
// The details below are specific to the FileDataDest and probably not
// of direct interest to anyone writing model or texture export filters.

class FileDataDest : public DataDest
{
   public:
      // The FileDataDest does *NOT* take ownership of the FILE pointer.
      // However, the internalClose() function has the same effect as calling
      // fclose().
      FileDataDest( FILE * fp, size_t startOffset = 0 );
      FileDataDest( const char * filename );
      virtual ~FileDataDest();

      void internalClose();

      virtual bool internalSeek( off_t offset );
      virtual bool internalWrite( const uint8_t * buf, size_t bufLen );

   protected:

   private:
      void sendErrno( int err );

      FILE * m_fp;
      size_t m_startOffset;
      bool m_mustClose;
};

#endif // FILEDATADEST_INC_H__
