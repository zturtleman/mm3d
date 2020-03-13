/*  Maverick Model 3D
 * 
 *  Copyright (c) 2004-2007 Kevin Worcester
 *  Copyright (c) 2019 Zack Middleton
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


#ifndef __D3DFILTER_H
#define __D3DFILTER_H

#include "modelfilter.h"

#include <string>

class D3dFilter : public ModelFilter
{
   public:
      D3dFilter();
      virtual ~D3dFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o = NULL );

      bool canRead( const char * filename = NULL )   { return true; };
      bool canWrite( const char * filename = NULL )  { return false; };
      bool canExport( const char * filename = NULL ) { return true; };

      bool isSupported( const char * file );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

   protected:

      DataDest * m_dst;

      bool writeLine( const char * line, ... ) __attribute__ ((format (printf, 2, 3)));
};

#endif // __D#DFILTER_H
