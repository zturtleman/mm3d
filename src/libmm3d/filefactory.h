/*  Maverick Model 3D
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


#ifndef FILEFACTORY_H_INC__
#define FILEFACTORY_H_INC__

// The FileFactory creates DataSource and DataDest objects. It is used by
// the ModelFilter class so that file I/O can be replaced with memory I/O.
//
// To implement a different FileFactory type, override createDest and
// createSource to create the DataDest and DataSource types of your choice.

#include <map>
#include <string>

class DataDest;
class DataSource;

class FileFactory
{
   public:
      typedef std::map< std::string, DataDest * > DestMap;
      typedef std::map< std::string, DataSource * > SourceMap;

      FileFactory();
      virtual ~FileFactory();

      void closeAll();

      DataDest * getDest( const char * filename );
      DataSource * getSource( const char * filename );

      SourceMap & getSourceMap() { return m_sources; }
      DestMap & getDestMap() { return m_dests; }

   protected:
      virtual DataDest * createDest( const char * filename );
      virtual DataSource * createSource( const char * filename );

   private:
      DestMap m_dests;
      SourceMap m_sources;
};

#endif  // FILEFACTORY_H_INC__
