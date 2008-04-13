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


#ifndef TESTFILEFACTORY_H_INC__
#define TESTFILEFACTORY_H_INC__

// The FileFactory creates DataSource and DataDest objects. It is used by
// the ModelFilter class so that file I/O can be replaced with memory I/O.
//
// To implement a different FileFactory type, override createDest and
// createSource to create the DataDest and DataSource types of your choice.

#include <map>
#include <string>

#include "filefactory.h"

class DataDest;
class DataSource;
class MemDataDest;

class TestFileFactory : public FileFactory
{
   public:
      typedef std::map< std::string, DataDest * > DestMap;
      typedef std::map< std::string, DataSource * > SourceMap;

      struct FileBuffer
      {
         uint8_t * buf;
         size_t bufSize;  // Max size of buffer
         size_t bufLen;   // Length of actual data written to buffer
         MemDataDest * dest;  // FileFactory owns this pointer
      };
      typedef std::map< std::string, FileBuffer > BufferMap;

      TestFileFactory();
      virtual ~TestFileFactory();

      void setFilePath( const std::string & filePath );

   public:
      virtual DataDest * createDest( const char * filename );
      virtual DataSource * createSource( const char * filename );

   private:
      BufferMap m_fileBuffers;
      std::string m_filePath;
};

#endif  // TESTFILEFACTORY_H_INC__
