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


#include "txtfilter.h"

#include "model.h"
#include "texture.h"
#include "log.h"
#include "misc.h"
#include "mm3dconfig.h"
#include "datadest.h"
#include "datasource.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <vector>

using std::list;
using std::string;

struct _TextPoint_t
{
    float x;
    float y;
    float z;
};
typedef struct _TextPoint_t TextPointT;

static size_t _skipToFloat( const char * str )
{
    size_t off = 0;
    while ( (str[off] != '\0' && !isdigit( str[off] ) && str[off] != '.' && str[off] != '-' ) 
            || (str[off] == '-' && !isdigit( str[off+1] ) ) )
    {
        off++;
    }
    return off;
}

static size_t _skipToNonFloat( const char * str )
{
    size_t off = 0;
    if ( str[off] == '-' )
    {
        off++;
    }
    while ( isdigit( str[off] ) || str[off] == '.' )
    {
        off++;
    }
    return off;
}

static size_t _readFloat( const char * str, float * f )
{
    *f = atof( str );
    return _skipToNonFloat( str );
}

static size_t _readPoint( const char * str, TextPointT * p, bool * gotPoint )
{
    size_t off = 0;
    size_t len = strlen( str );

    int index = 0;
    float f[3];

    off += _skipToFloat( &str[off] );
    while ( off < len && index < 3 )
    {
        off += _readFloat( &str[off], &f[index] );
        off += _skipToFloat( &str[off] );
        index++;
    }

    p->x = f[0];
    p->y = f[1];
    p->z = f[2];

    *gotPoint = (index == 3);

    log_debug( "got point %f %f %f\n", p->x, p->y, p->z );

    return off;
}

TextFilter::TextFilter()
{
}

TextFilter::~TextFilter()
{
}

Model::ModelErrorE TextFilter::readFile( Model * model, const char * const filename )
{
   Model::ModelErrorE err = Model::ERROR_NONE;
   DataSource *src = openInput( filename, err );
   SourceCloser fc( src );

   if ( err != Model::ERROR_NONE )
      return err;

   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   model->setFilename( modelFullName.c_str() );

   std::vector< TextPointT > pointList;

   char line[ 512 ];
   while ( src->readLine( line, sizeof(line) ) )
   {
       size_t off = 0;
       size_t len = strlen( line );
       bool gotPoint = false;

       TextPointT p;
       pointList.clear();

       while ( (off += _readPoint( &line[off], &p, &gotPoint ) ) < len )
       {
           if ( gotPoint )
           {
               pointList.push_back( p );
           }
       }
       if ( gotPoint )
       {
           pointList.push_back( p );
       }

       log_debug( "got %d points\n", pointList.size() );

       if ( pointList.size() >= 3 )
       {
           // Make triangles
           size_t k =0;
           for ( k = 0; k + 2 < pointList.size(); k++ )
           {
               int v = model->addVertex( 
                       pointList[0].x, pointList[0].y, pointList[0].z );
               model->addVertex( 
                       pointList[k+1].x, pointList[k+1].y, pointList[k+1].z );
               model->addVertex( 
                       pointList[k+2].x, pointList[k+2].y, pointList[k+2].z );

               model->addTriangle( v, v+1, v+2 );
           }
       }
   }

   model->setupJoints();
   return Model::ERROR_NONE;
}

Model::ModelErrorE TextFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o  )
{
   Model::ModelErrorE err = Model::ERROR_NONE;
   DataDest *dst = openOutput( filename, err );
   DestCloser fc( dst );

   if ( err != Model::ERROR_NONE )
      return err;

   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );
      
   // Write model here
   size_t tcount = model->getTriangleCount();
   for ( size_t t = 0; t < tcount; t++ )
   {
       for ( int v = 0; v < 3; v++ )
       {
           int vert = model->getTriangleVertex( t, v );
           double coord[3];
           model->getVertexCoords( vert, coord );
           dst->writePrintf( "%f,%f,%f ", coord[0], coord[1], coord[2] );
       }
       dst->writeString( FILE_NEWLINE );
   }

   return Model::ERROR_NONE;
}

bool TextFilter::canRead( const char * filename )
{
   return true;
}

bool TextFilter::canWrite( const char * filename )
{
   return true;
}

bool TextFilter::canExport( const char * filename )
{
   return true;
}

bool TextFilter::isSupported( const char * filename )
{
   unsigned len = strlen( filename );

   if ( len >= 4 && strcasecmp( &filename[len-4], ".txt" ) == 0 )
   {
      return true;
   }
   else
   {
      return false;
   }
}

list< string > TextFilter::getReadTypes()
{
   list<string> rval;
   rval.push_back( "*.txt" );
   return rval;
}

list< string > TextFilter::getWriteTypes()
{
   list<string> rval;
   rval.push_back( "*.txt" );
   return rval;
}

