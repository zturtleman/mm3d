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


#ifndef __COBFILTER_H
#define __COBFILTER_H

#include "modelfilter.h"
#include "datadest.h"
#include "datasource.h"

#include <vector>
#include <string>
#include <map>

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>


class CobFilter : public ModelFilter
{
   public:
      CobFilter();
      virtual ~CobFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o );

      bool canRead( const char * filename );
      bool canWrite( const char * filename );
      bool canExport( const char * filename );

      bool isSupported( const char * filename );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

   protected:

      enum _ChunkType_e
      {
          CT_Polygon,
          CT_Material,
          CT_Unknown,
          CT_EOF,
          CT_MAX
      };
      typedef enum _ChunkType_e ChunkTypeE;

      enum _FaceFlags_e
      {
          FF_HOLE     = 0x08,
          FF_BACKCULL = 0x10,
      };

      struct _ChunkHeader_t
      {
          ChunkTypeE type;
          char chunkChars[5];
          int majorVersion;
          int minorVersion;
          int chunkId;
          int parentId;
          size_t length;
      };
      typedef struct _ChunkHeader_t ChunkHeaderT;

      struct _LocalAxes_t
      {
          float center[3];
          float xAxis[3];
          float yAxis[3];
          float zAxis[3];
      };
      typedef struct _LocalAxes_t LocalAxesT;

      struct _Vertex_t
      {
          double coord[3];
      };
      typedef struct _Vertex_t VertexT;

      struct _TextureCoord_t
      {
          float u;
          float v;
      };
      typedef struct _TextureCoord_t TextureCoordT;

      struct _PolygonVertex_t
      {
          int vertex;
          int uvCoord;
      };
      typedef struct _PolygonVertex_t PolygonVertexT;

      struct _PolygonFace_t
      {
          int flags;
          int matNumber;
          std::vector< PolygonVertexT > faceVertices;
      };
      typedef struct _PolygonFace_t PolygonFaceT;

      struct _CurrentPosition_t
      {
          // Note: translation is fourth column, not fourth row
          // Fourth matrix row is always 0,0,0,1
          float mat[3][4];

          void toMatrix( Matrix & m )
          {
             m.loadIdentity();
             for ( int r = 0; r < 4; r++ )
             {
                for ( int c = 0; c < 3; c++ )
                {
                   m.set( r, c, mat[c][r] );
                }
             }
          }
      };
      typedef struct _CurrentPosition_t CurrentPositionT;

      struct _PolygonMesh_t
      {
          std::string name;
          LocalAxesT localAxes;
          CurrentPositionT currentPosition;

          std::vector< VertexT >       vertexList;
          std::vector< TextureCoordT > uvList;
          std::vector< PolygonFaceT >  faceList;
      };
      typedef struct _PolygonMesh_t PolygonMeshT;

      struct _Material_t
      {
          int matNumber;
          int shaderType;
          int facetType;
          int facetAngle;

          float red;
          float blue;
          float green;
          float alpha;

          float ambient;
          float specular;
          float hilight;
          float refraction;

          std::string envFile;
          std::string texFile;
          std::string bumpFile;

          // Texture materials only
          float uOffset;
          float vOffset;
          float uRepeat;
          float vRepeat;

          // MM3D model data
          int materialId;
      };
      typedef struct _Material_t MaterialT;

      struct _MeshMaterial_t
      {
         int meshChunkId;
         int matNumber;
         int groupId;
      };
      typedef struct _MeshMaterial_t MeshMaterialT;

      static ChunkTypeE chunkCharsToType( const char * str );
      MeshMaterialT * getMeshMaterial( int chunkId, int matNumber );

      // Common read functions
      Model::ModelErrorE readFileHeader();
      bool readMaterialChunk( const ChunkHeaderT & header );
      bool readPolygonChunk( const ChunkHeaderT & header );

      // Binary read functions
      int16_t readBShort();
      int32_t readBLong();
      float readBFloat();
      char readBChar();

      bool readBString( std::string & );
      bool readBName( std::string & );
      bool readBLocalAxes( LocalAxesT & );
      bool readBCurrentPosition( CurrentPositionT & );

      bool readBChunkHeader( ChunkHeaderT & );
      bool readBPolygonChunk( const ChunkHeaderT & header, PolygonMeshT & mesh );
      bool readBMaterialChunk( const ChunkHeaderT & header, MaterialT & material );
      bool readBUnknownChunk( const ChunkHeaderT & header );

      // ASCII read functions
      void skipASpace();
      bool skipAString( const char * str );

      int16_t readAShort();
      int32_t readALong();
      float readAFloat();
      double readADouble();
      char readAChar();

      bool readAString( std::string & );
      bool readAName( std::string & );
      bool readALocalAxes( LocalAxesT & );
      bool readACurrentPosition( CurrentPositionT & );

      bool readAChunkHeader( ChunkHeaderT & );

      bool readAPolygonChunk( const ChunkHeaderT & header, PolygonMeshT & mesh );
      bool readAMaterialChunk( const ChunkHeaderT & header, MaterialT & material );
      bool readAUnknownChunk( const ChunkHeaderT & header );

      // Binary write functions
      void writeBShort( int16_t );
      void writeBLong( int32_t );
      void writeBFloat( float );
      void writeBChar( char );

      void writeBString( const std::string & );
      void writeBName( const std::string & );
      void writeBStandardAxes();
      void writeBStandardPosition();
      
      // Call this function to write the header
      void writeBChunkHeader( const ChunkHeaderT & header );

      // Call this function after writing the chunk data to
      // correct the chunk size in the header
      void writeBChunkSize();

      void writeBTriangleGroup( const std::list<int> & triList, const std::string & name );

      void writeBUngrouped();
      void writeBGrouped();
      void writeBMaterial( int32_t parentId, size_t materialNumber, int groupNumber );
      void writeBDefaultMaterial( int32_t parentId );

      void writeBEOF();

      int32_t getNextChunkId();

      Model       * m_model;
      DataSource  * m_src;
      DataDest    * m_dst;
      uint8_t     * m_bufPos;
      uint8_t     * m_fileBuf;
      uint8_t     * m_chunkStart;
      uint8_t     * m_chunkEnd;
      size_t        m_fileLength;
      size_t        m_lastChunkSizeOffset;
      int32_t       m_nextChunkId;

      bool          m_isBinary;
      bool          m_isLittleEndian;

      std::string  m_modelPath;
      std::string  m_modelBaseName;
      std::string  m_modelFullName;

      std::vector<MeshMaterialT> m_meshMaterials;
};

#endif // __COBFILTER_H
