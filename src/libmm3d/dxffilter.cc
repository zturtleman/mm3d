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


#include "dxffilter.h"

#include "model.h"
#include "texture.h"
#include "log.h"
#include "binutil.h"
#include "misc.h"
#include "filtermgr.h"
//#include "version.h"
#include "mm3dport.h"
#include "datadest.h"
#include "datasource.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <vector>

using std::list;
using std::string;

#ifdef PLUGIN
static DxfFilter * s_filter = NULL;
#endif // PLUGIN

static bool _float_equiv( float rhs, float lhs )
{
   if ( fabs( rhs - lhs ) < 0.0001f )
   {
      return true;
   }
   else
   {
      return false;
   }
}

#define DXF_COLOR_COUNT 256

static float _colorTable[ DXF_COLOR_COUNT ][ 3 ] = {
    { 0.0f,    0.0f,    0.0f },
    { 1.0f,    0.0f,    0.0f },
    { 1.0f,    1.0f,    0.0f },
    { 0.0f,    1.0f,    0.0f },
    { 0.0f,    1.0f,    1.0f },
    { 0.0f,    0.0f,    1.0f },
    { 1.0f,    0.0f,    1.0f },
    { 1.0f,    1.0f,    1.0f },
    { 0.5f,    0.5f,    0.5f },
    { 0.75f,   0.75f,   0.75f },
    { 1.0f,    0.0f,    0.0f },
    { 1.0f,    0.5f,    0.5f },
    { 0.65f,   0.0f,    0.0f },
    { 0.65f,   0.325f,  0.325f },
    { 0.5f,    0.0f,    0.0f },
    { 0.5f,    0.25f,   0.25f },
    { 0.3f,    0.0f,    0.0f },
    { 0.3f,    0.15f,   0.15f },
    { 0.15f,   0.0f,    0.0f },
    { 0.15f,   0.075f,  0.075f },
    { 1.0f,    0.25f,   0.0f },
    { 1.0f,    0.625f,  0.5f },
    { 0.65f,   0.1625f, 0.0f },
    { 0.65f,   0.4063f, 0.325f },
    { 0.5f,    0.125f,  0.0f },
    { 0.5f,    0.3125f, 0.25f },
    { 0.3f,    0.075f,  0.0f },
    { 0.3f,    0.1875f, 0.15f },
    { 0.15f,   0.0375f, 0.0f },
    { 0.15f,   0.0938f, 0.075f },
    { 1.0f,    0.5f,    0.0f },
    { 1.0f,    0.75f,   0.5f },
    { 0.65f,   0.325f,  0.0f },
    { 0.65f,   0.4875f, 0.325f },
    { 0.5f,    0.25f,   0.0f },
    { 0.5f,    0.375f,  0.25f },
    { 0.3f,    0.15f,   0.0f },
    { 0.3f,    0.225f,  0.15f },
    { 0.15f,   0.075f,  0.0f },
    { 0.15f,   0.1125f, 0.075f },
    { 1.0f,    0.75f,   0.0f },
    { 1.0f,    0.875f,  0.5f },
    { 0.65f,   0.4875f, 0.0f },
    { 0.65f,   0.5688f, 0.325f },
    { 0.5f,    0.375f,  0.0f },
    { 0.5f,    0.4375f, 0.25f },
    { 0.3f,    0.225f,  0.0f },
    { 0.3f,    0.2625f, 0.15f },
    { 0.15f,   0.1125f, 0.0f },
    { 0.15f,   0.1313f, 0.075f },
    { 1.0f,    1.0f,    0.0f },
    { 1.0f,    1.0f,    0.5f },
    { 0.65f,   0.65f,   0.0f },
    { 0.65f,   0.65f,   0.325f },
    { 0.5f,    0.5f,    0.0f },
    { 0.5f,    0.5f,    0.25f },
    { 0.3f,    0.3f,    0.0f },
    { 0.3f,    0.3f,    0.15f },
    { 0.15f,   0.15f,   0.0f },
    { 0.15f,   0.15f,   0.075f },
    { 0.75f,   1.0f,    0.0f },
    { 0.875f,  1.0f,    0.5f },
    { 0.4875f, 0.65f,   0.0f },
    { 0.5688f, 0.65f,   0.325f },
    { 0.375f,  0.5f,    0.0f },
    { 0.4375f, 0.5f,    0.25f },
    { 0.225f,  0.3f,    0.0f },
    { 0.2625f, 0.3f,    0.15f },
    { 0.1125f, 0.15f,   0.0f },
    { 0.1313f, 0.15f,   0.075f },
    { 0.50f,   1.0f,    0.0f },
    { 0.75f,   1.0f,    0.5f },
    { 0.325f,  0.65f,   0.0f },
    { 0.4875f, 0.65f,   0.325f  },
    { 0.25f,   0.5f,    0.0f },
    { 0.375f,  0.5f,    0.25f },
    { 0.15f,   0.3f,    0.0f },
    { 0.225f,  0.3f,    0.15f },
    { 0.075f,  0.15f,   0.0f },
    { 0.1125f, 0.15f,   0.075f },
    { 0.25f,   1.0f,    0.0f },
    { 0.625f,  1.0f,    0.5f },
    { 0.1625f, 0.65f,   0.0f },
    { 0.4063f, 0.65f,   0.325f },
    { 0.125f,  0.5f,    0.0f },
    { 0.3125f, 0.5f,    0.25f },
    { 0.075f,  0.3f,    0.0f },
    { 0.1875f, 0.3f,    0.15f },
    { 0.0375f, 0.15f,   0.0f },
    { 0.0938f, 0.15f,   0.075f },
    { 0.0f,    1.0f,    0.0f },
    { 0.50f,   1.0f,    0.5f },
    { 0.0f,    0.65f,   0.0f },
    { 0.325f,  0.65f,   0.325f },
    { 0.0f,    0.50f,   0.0f },
    { 0.25f,   0.50f,   0.25f },
    { 0.0f,    0.30f,   0.0f },
    { 0.15f,   0.30f,   0.15f },
    { 0.0f,    0.15f,   0.0f },
    { 0.075f,  0.15f,   0.075f },
    { 0.0f,    1.0f,    0.25f },
    { 0.50f,   1.0f,    0.625f },
    { 0.0f,    0.65f,   0.1625f },
    { 0.325f,  0.65f,   0.4063f },
    { 0.0f,    0.50f,   0.125f },
    { 0.25f,   0.50f,   0.3125f },
    { 0.0f,    0.30f,   0.075f },
    { 0.15f,   0.30f,   0.1875f },
    { 0.0f,    0.15f,   0.0375f },
    { 0.075f,  0.15f,   0.0938f },
    { 0.0f,    1.0f,    0.5f },
    { 0.50f,   1.0f,    0.75f },
    { 0.0f,    0.65f,   0.325f },
    { 0.325f,  0.65f,   0.4875f },
    { 0.0f,    0.50f,   0.25f },
    { 0.25f,   0.50f,   0.375f },
    { 0.0f,    0.30f,   0.15f },
    { 0.15f,   0.30f,   0.225f },
    { 0.0f,    0.15f,   0.075f },
    { 0.075f,  0.15f,   0.1125f },
    { 0.0f,    1.0f,    0.75f },
    { 0.50f,   1.0f,    0.875f },
    { 0.0f,    0.65f,   0.4875f },
    { 0.325f,  0.65f,   0.5688f },
    { 0.0f,    0.5f,    0.375f },
    { 0.25f,   0.5f,    0.4375f },
    { 0.0f,    0.3f,    0.225f },
    { 0.15f,   0.3f,    0.2625f },
    { 0.0f,    0.15f,   0.1125f },
    { 0.075f,  0.15f,   0.1313f },
    { 0.0f,    1.0f,    1.0f },
    { 0.50f,   1.0f,    1.0f },
    { 0.0f,    0.65f,   0.65f },
    { 0.325f,  0.65f,   0.65f },
    { 0.0f,    0.5f,    0.5f },
    { 0.25f,   0.5f,    0.5f },
    { 0.0f,    0.3f,    0.3f },
    { 0.15f,   0.3f,    0.3f },
    { 0.0f,    0.15f,   0.15f },
    { 0.075f,  0.15f,   0.15f },
    { 0.0f,    0.75f,   1.0f },
    { 0.50f,   0.875f,  1.0f },
    { 0.0f,    0.4875f, 0.65f },
    { 0.325f,  0.5688f, 0.65f },
    { 0.0f,    0.375f,  0.5f },
    { 0.25f,   0.4375f, 0.5f },
    { 0.0f,    0.225f,  0.3f },
    { 0.15f,   0.2625f, 0.3f },
    { 0.0f,    0.1125f, 0.15f },
    { 0.075f,  0.1313f, 0.15f },
    { 0.0f,    0.50f,   1.0f },
    { 0.50f,   0.75f,   1.0f },
    { 0.0f,    0.325f,  0.65f },
    { 0.325f,  0.4875f, 0.65f },
    { 0.0f,    0.25f,   0.5f },
    { 0.25f,   0.375f,  0.5f },
    { 0.0f,    0.15f,   0.3f },
    { 0.15f,   0.225f,  0.3f },
    { 0.0f,    0.075f,  0.15f },
    { 0.075f,  0.1125f, 0.15f },
    { 0.0f,    0.25f,   1.0f },
    { 0.50f,   0.625f,  1.0f },
    { 0.0f,    0.1625f, 0.65f },
    { 0.325f,  0.4063f, 0.65f },
    { 0.0f,    0.125f,  0.5f },
    { 0.25f,   0.3125f, 0.5f },
    { 0.0f,    0.075f,  0.3f },
    { 0.15f,   0.1875f, 0.3f },
    { 0.0f,    0.0375f, 0.15f },
    { 0.075f,  0.0938f, 0.15f },
    { 0.0f,    0.0f,    1.0f },
    { 0.5f,    0.5f,    1.0f },
    { 0.0f,    0.0f,    0.65f },
    { 0.325f,  0.325f,  0.65f },
    { 0.0f,    0.0f,    0.5f },
    { 0.25f,   0.25f,   0.5f },
    { 0.0f,    0.0f,    0.3f },
    { 0.15f,   0.15f,   0.3f },
    { 0.0f,    0.0f,    0.15f },
    { 0.075f,  0.075f,  0.15f },
    { 0.25f,   0.0f,    1.0f },
    { 0.625f,  0.5f,    1.0f },
    { 0.1625f, 0.0f,    0.65f },
    { 0.4063f, 0.325f,  0.65f },
    { 0.125f,  0.0f,    0.5f },
    { 0.3125f, 0.25f,   0.5f },
    { 0.075f,  0.0f,    0.3f },
    { 0.1875f, 0.15f,   0.3f },
    { 0.0375f, 0.0f,    0.15f },
    { 0.0938f, 0.075f,  0.15f },
    { 0.5f,    0.0f,    1.0f },
    { 0.75f,   0.5f,    1.0f },
    { 0.325f,  0.0f,    0.65f },
    { 0.4875f, 0.325f,  0.65f },
    { 0.25f,   0.0f,    0.5f },
    { 0.375f,  0.25f,   0.5f },
    { 0.15f,   0.0f,    0.3f },
    { 0.225f,  0.15f,   0.3f },
    { 0.075f,  0.0f,    0.15f },
    { 0.1125f, 0.075f,  0.15f },
    { 0.75f,   0.0f,    1.0f },
    { 0.875f,  0.5f,    1.0f },
    { 0.4875f, 0.0f,    0.65f },
    { 0.5688f, 0.325f,  0.65f },
    { 0.375f,  0.0f,    0.5f },
    { 0.4375f, 0.25f,   0.5f },
    { 0.225f,  0.0f,    0.3f },
    { 0.2625f, 0.15f,   0.3f },
    { 0.1125f, 0.0f,    0.15f },
    { 0.1313f, 0.075f,  0.15f },
    { 1.0f,    0.0f,    1.0f },
    { 1.0f,    0.5f,    1.0f },
    { 0.65f,   0.0f,    0.65f },
    { 0.65f,   0.325f,  0.65f },
    { 0.5f,    0.0f,    0.5f },
    { 0.5f,    0.25f,   0.5f },
    { 0.3f,    0.0f,    0.3f },
    { 0.3f,    0.15f,   0.3f },
    { 0.15f,   0.0f,    0.15f },
    { 0.15f,   0.075f,  0.15f },
    { 1.0f,    0.0f,    0.75f },
    { 1.0f,    0.5f,    0.875f },
    { 0.65f,   0.0f,    0.4875f },
    { 0.65f,   0.325f,  0.5688f },
    { 0.5f,    0.0f,    0.375f },
    { 0.5f,    0.25f,   0.4375f },
    { 0.3f,    0.0f,    0.225f },
    { 0.3f,    0.15f,   0.2625f },
    { 0.15f,   0.0f,    0.1125f },
    { 0.15f,   0.075f,  0.1313f },
    { 1.0f,    0.0f,    0.5f },
    { 1.0f,    0.5f,    0.75f },
    { 0.65f,   0.0f,    0.325f },
    { 0.65f,   0.325f,  0.4875f },
    { 0.5f,    0.0f,    0.25f },
    { 0.5f,    0.25f,   0.375f },
    { 0.3f,    0.0f,    0.15f },
    { 0.3f,    0.15f,   0.225f },
    { 0.15f,   0.0f,    0.075f },
    { 0.15f,   0.075f,  0.1125f },
    { 1.0f,    0.0f,    0.25f },
    { 1.0f,    0.5f,    0.625f },
    { 0.65f,   0.0f,    0.1625f },
    { 0.65f,   0.325f,  0.4063f },
    { 0.5f,    0.0f,    0.125f },
    { 0.5f,    0.25f,   0.3125f },
    { 0.3f,    0.0f,    0.075f },
    { 0.3f,    0.15f,   0.1875f },
    { 0.15f,   0.0f,    0.0375f },
    { 0.15f,   0.075f,  0.0938f },
    { 0.33f,   0.33f,   0.33f },
    { 0.464f,  0.464f,  0.464f },
    { 0.598f,  0.598f,  0.598f },
    { 0.732f,  0.732f,  0.732f },
    { 0.866f,  0.866f,  0.866f },
    { 1.0f,    1.0f,    1.0f }
};

static int _colorToIndex( float r, float g, float b )
{
    int index = 1;
    float diff = fabs( r - _colorTable[1][0] )
               + fabs( g - _colorTable[1][1] )
               + fabs( b - _colorTable[1][2] );

    for ( int t = 2; t < DXF_COLOR_COUNT; t++ )
    {
        float d = fabs( r - _colorTable[t][0] )
                + fabs( g - _colorTable[t][1] )
                + fabs( b - _colorTable[t][2] );

        if ( d < diff )
        {
            diff = d;
            index = t;
        }
    }

    return index;
}


DxfFilter::DxfFilter()
{
}

DxfFilter::~DxfFilter()
{
}

Model::ModelErrorE DxfFilter::readFile( Model * model, const char * const filename )
{
   Model::ModelErrorE err = Model::ERROR_NONE;
   m_src = openInput( filename, err );
   SourceCloser fc( m_src );

   if ( err != Model::ERROR_NONE )
      return err;

   log_debug( "dxf filter reading file %s\n", filename );
   m_modelPath = "";
   m_modelBaseName = "";
   m_modelFullName = "";

   normalizePath( filename, m_modelFullName, m_modelPath, m_modelBaseName );

   model->setFilename( m_modelFullName.c_str() );

   m_model = model;

   m_materialColor.clear();

   m_currentGroup = -1;
   m_currentColor =  7; // white

   setReadState( RS_MAIN );

   char line[ 1024 ];
   while ( m_src->readLine( line, sizeof(line) ) )
   {
      line[ sizeof( line ) - 1 ] = '\0';
      int len = strlen( line ) - 1;
      while ( len >= 0 && isspace( line[len] ) )
      {
         line[len] = '\0';
         len--;
      }
      int pos = 0;
      while ( isspace( line[pos] ) )
      {
         pos++;
      }
      readLine( &line[pos] );
   }

   return Model::ERROR_NONE;
}

Model::ModelErrorE DxfFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o )
{
   m_model = model;

   m_modelPath = "";
   m_modelBaseName = "";
   m_modelFullName = "";

   normalizePath( filename, m_modelFullName, m_modelPath, m_modelBaseName );

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   DestCloser fc( m_dst );

   if ( err != Model::ERROR_NONE )
      return err;

   writeHeader();
   writeGroups();
   writeFaces();
   writeLine( "0" );
   writeLine( "EOF" );

   return Model::ERROR_NONE;
}


bool DxfFilter::canRead( const char * filename )
{
   return true;
}

bool DxfFilter::canWrite( const char * filename )
{
   return false;
}

bool DxfFilter::canExport( const char * filename )
{
   return true;
}

bool DxfFilter::isSupported( const char * filename )
{
   unsigned len = strlen( filename );

   if ( len >= 4 && strcasecmp( &filename[len-4], ".dxf" ) == 0 )
   {
      return true;
   }
   else
   {
      return false;
   }
}

list< string > DxfFilter::getReadTypes()
{
   list<string> rval;
   rval.push_back( "*.dxf" );
   return rval;
}

list< string > DxfFilter::getWriteTypes()
{
   list<string> rval;
   rval.push_back( "*.dxf" );
   return rval;
}

//------------------------------------------------------------------
// Protected Methods
//------------------------------------------------------------------

bool DxfFilter::writeLine( const char * line, ... )
{
   va_list ap;
   va_start( ap, line );
   m_dst->writeVPrintf( line, ap );
   va_end( ap );
#ifdef WIN32
   m_dst->writeString( "\n" );
#else // WIN32
   m_dst->writeString( "\r\n" );
#endif // WIN32
   return true;
}

bool DxfFilter::readLine( const char * line )
{
    switch ( m_state )
    {
        case RS_MAIN:
            readMain( line );
            break;
        case RS_SECTION:
            readSection( line );
            break;
        case RS_HEADER:
            readHeader( line );
            break;
        case RS_TABLES:
            readTables( line );
            break;
        case RS_LAYER:
            readLayer( line );
            break;
        case RS_ENTITIES:
            readEntities( line );
            break;
        case RS_3DFACE:
            read3dface( line );
            break;
        case RS_POLYLINE:
            readPolyline( line );
            break;
        case RS_VERTEX:
            readVertex( line );
            break;
        case RS_UNKNOWN:
            readUnknown( line );
            break;
        default:
            log_error( "unhandled dxf read state %d\n", (int) m_state );
            break;
    }

    return true;
}

bool DxfFilter::readMain( const char * line )
{
    if ( strcmp( line, "EOF" ) == 0 )
    {
        setReadState( RS_DONE );
    }
    else if ( strcmp( line, "SECTION" ) == 0 )
    {
        setReadState( RS_SECTION );
    }
    return true;
}

bool DxfFilter::readSection( const char * line )
{
    if ( strcmp( line, "HEADER" ) == 0 )
    {
        setReadState( RS_HEADER );
    }
    else if ( strcmp( line, "TABLES" ) == 0 )
    {
        setReadState( RS_TABLES );
    }
    else if ( strcmp( line, "TABLE" ) == 0 ) // TODO may need to treat TABLES and TABLE separately
    {
        setReadState( RS_TABLES );
    }
    else if ( strcmp( line, "ENTITIES" ) == 0 )
    {
        setReadState( RS_ENTITIES );
    }
    return true;
}

bool DxfFilter::readHeader( const char * line )
{
    if ( strcmp( line, "ENDSEC" ) == 0 )
    {
        setReadState( RS_MAIN );
    }
    return true;
}

bool DxfFilter::readTables( const char * line )
{
    if ( strcmp( line, "ENDTAB" ) == 0 )
    {
        setReadState( RS_SECTION );
    }
    else if ( strcmp( line, "LAYER" ) == 0 )
    {
        log_debug( "got layer in table, last code was %d\n", m_lastCode );
        if ( m_lastCode <= 0 )
        {
            log_debug( "in layer\n" );
            setReadState( RS_LAYER );
        }
        m_lastCode = 0;
    }
    else
    {
        if ( m_lastCode <= 0 )
        {
            m_lastCode = atoi( line );
        }
        else
        {
            m_lastCode = 0;
        }
    }
    return true;
}

bool DxfFilter::readLayer( const char * line )
{
    if ( m_lastCode <= 0 )
    {
        m_lastCode = atoi( line );
        if ( m_lastCode == 0 )
        {
            // Done, set color
            log_debug( "read layer, color is %d\n", m_currentColor );
            log_debug( "  group is %d\n", m_currentGroup );
            if ( m_currentGroup >= 0 )
            {
                const char * name = m_model->getGroupName( m_currentGroup );
                m_currentGroup = getGroupByNameColor( name, m_currentColor );
                log_debug( "  material is %d\n", m_model->getGroupTextureId( m_currentGroup ) );
            }
            setReadState( RS_TABLES );
        }
    }
    else
    {
        switch( m_lastCode )
        {
            case 2: // layer name
                log_debug( "got layer name %s\n", line );
                m_currentGroup = getGroupByName( line );
                break;
            case 70: // layer flags, ignored
                break;
            case 62:
                m_currentColor = atoi( line );
                log_debug( "color is %d\n", m_currentColor );
                break;
            case 6: // linetype, should be CONTINUOUS
                break;
            default:
                break;
        }

        m_lastCode = 0;
    }
    return true;
}

bool DxfFilter::readEntities( const char * line )
{
    if ( strcmp( line, "3DFACE" ) == 0 )
    {
        setReadState( RS_3DFACE );
    }
    if ( strcmp( line, "POLYLINE" ) == 0 )
    {
        setReadState( RS_POLYLINE );
        m_isPolyfaceMesh = false;
    }
    else if ( strcmp( line, "ENDSEC" ) == 0 )
    {
        setReadState( RS_MAIN );
    }
    return true;
}

bool DxfFilter::read3dface( const char * line )
{
    if ( m_lastCode <= 0 )
    {
        m_lastCode = atoi( line );
        if ( m_lastCode == 0 )
        {
            setReadState( RS_ENTITIES );
        }
    }
    else
    {
        switch ( m_lastCode )
        {
            case 62:
                m_currentColor = atoi( line );
                log_debug( "color is %d\n", m_currentColor );
                if ( m_currentGroup >= 0 )
                {
                   const char * name = m_model->getGroupName( m_currentGroup );
                   m_currentGroup = getGroupByNameColor( name, m_currentColor );
                   log_debug( "  material is %d\n", m_model->getGroupTextureId( m_currentGroup ) );
                }
                break;
            case 8: // layer name, group this poly belongs to
                m_currentGroup = getGroupByName( line );
                break;

            // Vertices
            case 10: // x1
            case 20: // y1
            case 30: // z1
            case 11: // x2
            case 21: // y2
            case 31: // z2
            case 12: // x3
            case 22: // y3
            case 32: // z3
            case 13: // x4
            case 23: // y4
            case 33: // z4
                {
                    int vertex = (m_lastCode % 10);
                    int axis   = (m_lastCode / 10) - 1;

                    m_coord[ vertex ][ axis ] = atof( line );

                    if ( m_lastCode == 33 )
                    {
                        // add polygon
                        int vert[4];
                        int tri;

                        // Exchange Y and Z coordinates
                        for ( int v = 0; v < 3; v++ )
                        {
                            vert[v] = m_model->addVertex( 
                                    m_coord[v][0], m_coord[v][2], m_coord[v][1] );
                        }
                        
                        materialPrep();
                        tri = m_model->addTriangle( vert[0], vert[1], vert[2] );
                        m_model->addTriangleToGroup( m_currentGroup, tri );

                        log_debug( "adding face %d to group %d\n", tri, m_currentGroup );

                        if ( !_float_equiv( m_coord[2][0], m_coord[3][0] )
                                || !_float_equiv( m_coord[2][1], m_coord[3][1] )
                                || !_float_equiv( m_coord[2][2], m_coord[3][2] ) )
                        {
                            vert[3] = m_model->addVertex( 
                                    m_coord[3][0], m_coord[3][2], m_coord[3][1] );

                            tri = m_model->addTriangle( vert[0], vert[2], vert[3] );
                            m_model->addTriangleToGroup( m_currentGroup, tri );
                            log_debug( "adding face %d to group %d\n", tri, m_currentGroup );
                        }
                    }
                }
                break;

            default:
                break;
        }

        m_lastCode = 0;
    }
    return true;
}

bool DxfFilter::readPolyline( const char * line )
{
    if ( m_lastCode < 0 )
    {
        m_lastCode = atoi( line );
    }
    else
    {
        switch ( m_lastCode )
        {
            case 0:
               if ( strcmp( line, "ENDSEC" ) == 0 )
               {
                  setReadState( RS_SECTION );
               }
               else if ( strcmp( line, "VERTEX" ) == 0 )
               {
                  log_debug( "found vertex section\n" );
                  setReadState( RS_VERTEX );
                  m_baseVertex = m_model->getVertexCount();
                  m_vertexIsFace = false;
                  m_vertexIsValid = false;
               }
               break;
            case 62:
                m_currentColor = atoi( line );
                log_debug( "color is %d\n", m_currentColor );
                if ( m_currentGroup >= 0 )
                {
                   const char * name = m_model->getGroupName( m_currentGroup );
                   m_currentGroup = getGroupByNameColor( name, m_currentColor );
                   log_debug( "  material is %d\n", m_model->getGroupTextureId( m_currentGroup ) );
                }
                break;
            case 70:
                if ( (atoi(line) & 64) != 0 )
                {
                   log_debug( "poly line is polyface mesh\n" );
                   m_isPolyfaceMesh = true;
                }
                else
                {
                   log_warning( "poly line is not polyface mesh\n" );
                   m_isPolyfaceMesh = false;
                }
                break;
            case 8: // layer name, group this poly belongs to
                m_currentGroup = getGroupByName( line );
                break;

            default:
                break;
        }

        m_lastCode = -1;
    }
    return true;
}

bool DxfFilter::readVertex( const char * line )
{
    if ( m_lastCode < 0 )
    {
        m_lastCode = atoi( line );
        if ( m_lastCode == 0 )
        {
           if ( m_vertexIsValid )
           {
              if ( m_vertexIsFace )
              {
                 if ( m_vertices[0] >= 0 
                       && m_vertices[1] >= 0 
                       && m_vertices[2] >= 0 )
                 {
                    int tri = m_model->addTriangle( m_vertices[0] + m_baseVertex,
                       m_vertices[1] + m_baseVertex,
                       m_vertices[2] + m_baseVertex );

                    materialPrep();
                    m_model->addTriangleToGroup( m_currentGroup, tri );
                    log_debug( "adding face %d to group %d\n", tri, m_currentGroup );
                 }
                 if ( m_vertices[0] >= 0 
                       && m_vertices[2] >= 0 
                       && m_vertices[3] >= 0 )
                 {
                    int tri = m_model->addTriangle( m_vertices[0] + m_baseVertex,
                       m_vertices[2] + m_baseVertex,
                       m_vertices[3] + m_baseVertex );

                    materialPrep();
                    m_model->addTriangleToGroup( m_currentGroup, tri );
                    log_debug( "adding face %d to group %d\n", tri, m_currentGroup );
                 }
              }
              else
              {
                 // Exchange Y and Z coordinates
                 m_model->addVertex( m_coord[0][0], m_coord[0][2], m_coord[0][1] );
              }
           }
           m_vertexIsFace = false;
           m_vertexIsValid = false;
           for ( int i = 0; i < 4; i++ )
           {
              m_vertices[i] = -1;
           }
        }
    }
    else
    {
        switch ( m_lastCode )
        {
            case 0:
                if ( strcmp( line, "SEQEND" ) == 0 )
                {
                   setReadState( RS_POLYLINE );
                }
                break;
            case 62:
                m_currentColor = atoi( line );
                log_debug( "color is %d\n", m_currentColor );
                break;
            case 70:
                {
                   int flags = atoi(line);
                   if ( (flags & 128) != 0 )
                   {
                      m_vertexIsValid = true;
                   }
                   if ( (flags & 64) == 0 )
                   {
                      m_vertexIsFace = true;
                   }
                }
                break;
            case 8: // layer name, group this poly belongs to
                m_currentGroup = getGroupByName( line );
                break;

            // Vertices
            case 10: // x1
            case 20: // y1
            case 30: // z1
            case 11: // x2
            case 21: // y2
            case 31: // z2
            case 12: // x3
            case 22: // y3
            case 32: // z3
            case 13: // x4
            case 23: // y4
            case 33: // z4
                {
                    int vertex = (m_lastCode % 10);
                    int axis   = (m_lastCode / 10) - 1;

                    m_coord[ vertex ][ axis ] = atof( line );
                }
                break;

            case 71: // v1
            case 72: // v2
            case 73: // v3
            case 74: // v4
                {
                   int v = m_lastCode - 71;
                   m_vertices[v] = atoi( line );
                   if ( m_vertices[v] < 0 )
                   {
                      m_vertices[v] = -m_vertices[v];
                   }
                   m_vertices[v]--;
                }
                break;

            default:
                break;
        }

        m_lastCode = -1;
    }
    return true;
}

bool DxfFilter::readUnknown( const char * line )
{
    // There shouldn't be anything to do here
    return true;
}

void DxfFilter::setReadState( ReadStateE state )
{
    m_state = state;
    m_lastCode = -1;
    //log_debug( "read state is now %d\n", (int) state );
}

void DxfFilter::writeHeader()
{
    writeLine( "0" );
    writeLine( "SECTION" );
    writeLine( "2" );
    writeLine( "HEADER" );
    writeLine( "9" );
    writeLine( "$ACADVER" );
    writeLine( "1" );
    writeLine( "AC1009" );
    writeLine( "0" );
    writeLine( "ENDSEC" );
}

void DxfFilter::writeGroups()
{
    writeLine( "0" );
    writeLine( "SECTION" );
    writeLine( "2" );
    writeLine( "TABLES" );
    writeLine( "0" );
    writeLine( "TABLE" );
    writeLine( "2" );
    writeLine( "LAYER" );
    writeLine( "70" );
    writeLine( "3" );
    size_t gcount = m_model->getGroupCount();
    for ( size_t g = 0; g < gcount; g++ )
    {
        float fval[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
        int mat = m_model->getGroupTextureId( g );

        if ( mat >= 0 )
        {
            m_model->getTextureDiffuse( mat, fval );
        }

        int colorIndex = _colorToIndex( fval[0], fval[1], fval[2] );

        writeLine( "0" );
        writeLine( "LAYER" );
        writeLine( "2" );
        writeLine( m_model->getGroupName(g) );
        writeLine( "70" );
        writeLine( "0" );
        writeLine( "62" );
        writeLine( "%d", colorIndex );
        writeLine( "6" );
        writeLine( "CONTINUOUS" );
    }
    writeLine( "0" );
    writeLine( "ENDTAB" );
    writeLine( "0" );
    writeLine( "ENDSEC" );
}

void DxfFilter::writeFaces()
{
    writeLine( "0" );
    writeLine( "SECTION" );
    writeLine( "2" );
    writeLine( "ENTITIES" );
    size_t fcount = m_model->getTriangleCount();
    for ( size_t f = 0; f < fcount; f++ )
    {
        writeLine( "0" );
        writeLine( "3DFACE" );
        writeLine( "8" );

        int colorIndex = 7; // white

        int g = m_model->getTriangleGroup(f);

        if ( g >= 0 )
        {
            writeLine( m_model->getGroupName(g) );

            int mat = m_model->getGroupTextureId( g );
            if ( mat >= 0 )
            {
                float fval[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
                m_model->getTextureDiffuse( mat, fval );
                colorIndex = _colorToIndex( fval[0], fval[1], fval[2] );
            }
        }
        else
        {
            writeLine( "UNGROUPED" );
        }
        writeLine( "62" );
        writeLine( "%d", colorIndex );
        int vert[3];
        int v;
        for ( v = 0; v < 3; v++ )
        {
            vert[v] = m_model->getTriangleVertex( f, v );
        }

        double coord[3];
        for ( v = 0; v < 3; v++ )
        {
            // Exchange Y and Z coordinates
            m_model->getVertexCoords( vert[v], coord );
            writeLine( "1%d", v );
            writeLine( "%f", (float) coord[0] );
            writeLine( "2%d", v );
            writeLine( "%f", (float) coord[2] );
            writeLine( "3%d", v );
            writeLine( "%f", (float) coord[1] );
        }

        // Every face is a triangle, write third vertex again
        writeLine( "1%d", v );
        writeLine( "%f", (float) coord[0] );
        writeLine( "2%d", v );
        writeLine( "%f", (float) coord[2] );
        writeLine( "3%d", v );
        writeLine( "%f", (float) coord[1] );
    }
    writeLine( "0" );
    writeLine( "ENDSEC" );
}

void DxfFilter::materialPrep()
{
   if ( m_currentGroup >= 0 && m_currentColor >= 0 )
   {
      const char * name = m_model->getGroupName( m_currentGroup );
      m_currentGroup = getGroupByNameColor( name, m_currentColor );
      log_debug( "  material is %d\n", m_model->getGroupTextureId( m_currentGroup ) );
   }
}

int DxfFilter::getGroupByName( const char * name )
{
    int group = m_model->getGroupByName( name );
    if ( group < 0 )
    {
        group = m_model->addGroup( name );

        m_model->setGroupTextureId( group, 
                m_model->addColorMaterial( name ) );
    }

    return group;
}

int DxfFilter::getGroupByNameColor( const char * name, int colorIndex )
{
    size_t g;
    size_t gcount = m_model->getGroupCount();

    for ( g = 0; g < gcount; g++ )
    {
       const char * n = m_model->getGroupName( g );
       if ( strcmp( name, n ) == 0 )
       {
          int material = m_model->getGroupTextureId( g );
          if ( materialHasColor( material ) )
          {
             if ( getMaterialColor( material ) == colorIndex )
             {
                return g;
             }
          }
          else
          {
             setMaterialColor( material, colorIndex );
             return g;
          }
       }
    }

    g = m_model->addGroup( name );
    
    int material = m_model->addColorMaterial( name );
    m_model->setGroupTextureId( g, material );
    setMaterialColor( material, colorIndex );

    return g;
}

void DxfFilter::setMaterialColor( unsigned int material, int colorIndex )
{
   float fval[4];

   if ( colorIndex < 0 )
   {
      colorIndex = 0;
   }
   else if ( colorIndex > 255 )
   {
      colorIndex = 255;
   }

   fval[0] = _colorTable[ colorIndex ][0];
   fval[1] = _colorTable[ colorIndex ][1];
   fval[2] = _colorTable[ colorIndex ][2];
   fval[3] = 1.0f;

   m_model->setTextureDiffuse( material, fval );

   fval[0] = _colorTable[ colorIndex ][0] * 0.20f;
   fval[1] = _colorTable[ colorIndex ][1] * 0.20f;
   fval[2] = _colorTable[ colorIndex ][2] * 0.20f;
   fval[3] = 1.0f;

   m_model->setTextureAmbient( material, fval );

   fval[0] = 0.0f;
   fval[1] = 0.0f;
   fval[2] = 0.0f;
   fval[3] = 1.0f;

   m_model->setTextureEmissive( material, fval );

   fval[0] = 0.0f;
   fval[1] = 0.0f;
   fval[2] = 0.0f;
   fval[3] = 1.0f;

   m_model->setTextureSpecular( material, fval );

   m_materialColor[ material ] = colorIndex;
}

bool DxfFilter::materialHasColor( unsigned int material )
{
   return ( m_materialColor.find( material ) != m_materialColor.end() );
}

int DxfFilter::getMaterialColor( unsigned int material )
{
   if ( materialHasColor( material ) )
   {
      return m_materialColor[ material ];
   }
   return 7;
}


#ifdef PLUGIN

//------------------------------------------------------------------
// Plugin functions
//------------------------------------------------------------------

extern "C" bool plugin_init()
{
   if ( s_filter == NULL )
   {
      s_filter = new DxfFilter();
      FilterManager * texmgr = FilterManager::getInstance();
      texmgr->registerFilter( s_filter );
   }
   log_debug( "DXF model filter plugin initialized\n" );
   return true;
}

// The filter manager will delete our registered filter.
// We have no other cleanup to do
extern "C" bool plugin_uninit()
{
   s_filter = NULL; // FilterManager deletes filters
   log_debug( "DXF model filter plugin uninitialized\n" );
   return true;
}

extern "C" const char * plugin_version()
{
   return "0.1.0";
}

extern "C" const char * plugin_desc()
{
   return "DXF model filter";
}

#endif // PLUGIN
