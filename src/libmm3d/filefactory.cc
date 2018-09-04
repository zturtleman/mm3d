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

#include "filefactory.h"

#include "datadest.h"
#include "datasource.h"
#include "filedatadest.h"
#include "filedatasource.h"

FileFactory::FileFactory()
{
}

FileFactory::~FileFactory()
{
   closeAll();
}

void FileFactory::closeAll()
{
   for ( SourceMap::iterator it = m_sources.begin();
         it != m_sources.end(); ++it )
   {
      delete it->second;
   }
   m_sources.clear();
   for ( DestMap::iterator it = m_dests.begin();
         it != m_dests.end(); ++it )
   {
      delete it->second;
   }
   m_dests.clear();
}

DataDest * FileFactory::getDest( const char * filename )
{
   DataDest * dst = createDest( filename );
   m_dests[filename] = dst;
   return dst;
}

DataSource * FileFactory::getSource( const char * filename )
{
   DataSource * dst = createSource( filename );
   m_sources[filename] = dst;
   return dst;
}

DataDest * FileFactory::createDest( const char * filename )
{
   return new FileDataDest( filename );
}

DataSource * FileFactory::createSource( const char * filename )
{
   return new FileDataSource( filename );
}

