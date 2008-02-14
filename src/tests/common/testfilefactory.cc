/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2007 Kevin Worcester
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

#include "testfilefactory.h"

#include "memdatadest.h"
#include "memdatasource.h"

TestFileFactory::TestFileFactory()
{
}

TestFileFactory::~TestFileFactory()
{
   for ( BufferMap::const_iterator it = m_fileBuffers.begin();
         it != m_fileBuffers.end(); ++it )
   {
      delete it->second.buf;
   }
   m_fileBuffers.clear();
}

DataDest * TestFileFactory::createDest( const char * filename )
{
   // Create a new memory buffer fo this file if needed
   BufferMap::iterator it = m_fileBuffers.find( filename );
   if ( it == m_fileBuffers.end() )
   {
      const int MAX_SIZE = 1256000;

      FileBuffer fb;
      fb.buf = new uint8_t[ MAX_SIZE ];
      fb.bufSize = MAX_SIZE;
      fb.bufLen = 0;
      m_fileBuffers[filename] = fb;

      it = m_fileBuffers.find( filename );
   }

   // Create dest to wrap the memory buffer. The FileFactory will take
   // onwership of this pointer.
   MemDataDest * dest = new MemDataDest( it->second.buf, it->second.bufSize );
   it->second.dest = dest;

   return dest;
}

DataSource * TestFileFactory::createSource( const char * filename )
{
   // If we have an in-memory buffer for this file, use it
   BufferMap::iterator it = m_fileBuffers.find( filename );
   if ( it != m_fileBuffers.end() )
   {
      it->second.bufLen = it->second.dest->getDataLength();
      return new MemDataSource( it->second.buf, it->second.bufLen );
   }

   // No in-memory buffer, use a real file as the source
   return FileFactory::createSource( filename );
}

