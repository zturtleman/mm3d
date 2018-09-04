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


#ifndef MEMDATASOURCE_INC_H__
#define MEMDATASOURCE_INC_H__

#include "datasource.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

// This class is a DataSource that uses memory as the underlying input
// source. See the documentation in datasource.h for the DataSource API.
// The details below are specific to the MemDataSource and probably not
// of direct interest to anyone writing model or texture import filters.

class MemDataSource : public DataSource
{
   public:
      // The MemDataSource does *NOT* take ownership of the memory pointer.
      MemDataSource( const uint8_t * buf, size_t bufSize );
      virtual ~MemDataSource();

   protected:
      virtual bool internalReadAt( off_t offset, const uint8_t ** buf, size_t * bufLen );

   private:
      const uint8_t * m_buf;
      size_t m_bufSize;
      size_t m_bufOffset;
};

#endif // MEMDATASOURCE_INC_H__
