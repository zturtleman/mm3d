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


#ifndef MEMDATADEST_INC_H__
#define MEMDATADEST_INC_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include "datadest.h"

// This class is a DataDest that uses memory as the underlying output
// source. See the documentation in datadest.h for the DataDest API.
// The details below are specific to the MemDataDest and probably not
// of direct interest to anyone writing model or texture export filters.

class MemDataDest : public DataDest
{
   public:
      // The MemDataDest does *NOT* take ownership of the memory pointer.
      MemDataDest( uint8_t * buf, size_t bufSize );
      virtual ~MemDataDest();

      virtual bool internalSeek( off_t offset );
      virtual bool internalWrite( const uint8_t * buf, size_t bufLen );

   protected:

   private:
      uint8_t * m_buf;
      size_t m_bufSize;
      size_t m_bufOffset;
};

#endif // MEMDATADEST_INC_H__
