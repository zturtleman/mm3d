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


#ifndef __LWOFILTER_H
#define __LWOFILTER_H

#include "modelfilter.h"

#include <vector>
#include <string>

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>


class LwoFilter : public ModelFilter
{
   public:
      LwoFilter();
      virtual ~LwoFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o );

      bool canRead( const char * filename );
      bool canWrite( const char * filename );
      bool canExport( const char * filename );

      bool isSupported( const char * filename );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      struct _PolyMap_t
      {
         unsigned polyIndex;
         unsigned polyCount;
      };
      typedef struct _PolyMap_t PolyMapT;

      typedef struct _ClipData_t
      {
         std::string filename;
      } ClipDataT;

      typedef struct _SurfaceBlockData_t
      {
         int textureIndex;
      } SurfaceBlockDataT;

      typedef struct _VertexMapData_t
      {
         int vertex;
         float s;
         float t;
      } VertexMapDataT;

      typedef struct _DiscVertexMapData_t
      {
         int polyIndex;
         int vertexNumber;
         float s;
         float t;
      } DiscVertexMapDataT;

      typedef std::vector< unsigned >    PolyList;
      typedef std::vector< unsigned >    SurfaceTags;
      typedef std::vector< PolyList >    SurfacePolys;
      typedef std::vector< std::string > TagList;
      typedef std::vector< PolyMapT >     PolyMapList;
      typedef std::vector< ClipDataT >    ClipList;
      typedef std::vector< float >       FloatList;
      typedef std::vector< VertexMapDataT >     VertexMapList;
      typedef std::vector< DiscVertexMapDataT > DiscVertexMapList;

   protected:

      bool readChunk( size_t len );

      bool readVertexChunk( size_t len );
      bool readPolygonChunk( size_t len );
      bool readSurfaceListChunk( size_t len);
      bool readSurfaceDefinitionChunk( size_t len );
      bool readSurfaceBlockChunk( size_t len, SurfaceBlockDataT & sbd  );
      bool readVertexMapChunk( size_t len );
      bool readDiscVertexMapChunk( size_t len );
      bool readTagChunk( size_t len );
      bool readPolyTagChunk( size_t len );
      bool readClipChunk( size_t len );

      uint32_t readU4();
      uint16_t readU2();
      uint8_t  readU1();
      int32_t  readI4();
      int16_t  readI2();
      int8_t   readI1();
      float    readF4();
      unsigned readColor( float & r, float & g, float & b );
      unsigned readVX( unsigned & );
      void     readID( char * );
      unsigned readString( char * dest, size_t len );
      unsigned readNothing( size_t len );

      Model      * m_model;
      FILE       * m_fp;
      uint8_t    * m_bufPos;
      uint8_t    * m_fileBuf;
      int          m_curGroup;
      int          m_vertices;
      int          m_faces;
      int          m_groups;
      uint32_t     m_lastVertexBase;
      uint32_t     m_lastPolyBase;
      bool         m_isLWO2;
      SurfacePolys m_surfacePolys;
      SurfaceTags  m_surfaceTags;
      TagList      m_tags;
      PolyMapList  m_polyMaps;
      ClipList     m_clips;
      FloatList    m_smoothAngles;

      VertexMapList     m_vertexMaps;
      DiscVertexMapList m_discVertexMaps;

      std::string  m_modelPath;
      std::string  m_modelBaseName;
      std::string  m_modelFullName;
};

#endif // __LWOFILTER_H
