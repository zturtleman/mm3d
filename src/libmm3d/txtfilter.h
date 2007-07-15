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


#ifndef __TXTFILTER_H
#define __TXTFILTER_H

// Very simple text file for triangles.
//
// Format is three or more points on a single line of text.  A point 
// is three floating point numbers.  In other words, polygons are
// sets of 9, 12, 15, 18, etc. floating point numbers on a single line.
// A txt model file can contain multiple lines.
//
// There is no support for normals, materials, or texture coordinates.

#include "modelfilter.h"
#include <stdint.h>

class TextFilter : public ModelFilter
{
   public:
      TextFilter();
      virtual ~TextFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o = NULL );

      bool canRead( const char * filename );
      bool canWrite( const char * filename );
      bool canExport( const char * filename );

      bool isSupported( const char * filename );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

   protected:


};

#endif // __TXTFILTER_H
