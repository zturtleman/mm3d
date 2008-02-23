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


#include "cobfilter.h"

#include "model.h"
#include "texture.h"
#include "log.h"
#include "binutil.h"
#include "misc.h"
#include "filtermgr.h"
#include "endianconfig.h"
#include "texmgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif // WIN32

#include <vector>

using std::list;
using std::string;

#ifdef PLUGIN
static CobFilter * s_filter = NULL;
#endif // PLUGIN

static void _invertModelNormals( Model * model )
{
   size_t tcount = model->getTriangleCount();
   for ( size_t t = 0; t < tcount; t++ )
   {
      model->invertNormals( t );
   }
}

static float _valToCoeff( float * fval )
{
   float rval = 0.0;
   if ( fval != NULL )
   {
      for ( int i = 0; i < 3; i++ )
      {
         rval += fval[i];
      }
      rval /= 3.0f;
   }
   return rval;
}

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

CobFilter::CobFilter()
{
}

CobFilter::~CobFilter()
{
}

Model::ModelErrorE CobFilter::readFile( Model * model, const char * const filename )
{
   // TODO: At some point it would be nice to handle joints and animation, maybe...

   Model::ModelErrorE err = Model::ERROR_NONE;

   m_fp = fopen( filename, "rb" );

   if ( m_fp )
   {
      m_meshMaterials.clear();

      m_modelPath = "";
      m_modelBaseName = "";
      m_modelFullName = "";

      normalizePath( filename, m_modelFullName, m_modelPath, m_modelBaseName );

      model->setFilename( m_modelFullName.c_str() );

      fseek( m_fp, 0, SEEK_END );
      m_fileLength = ftell( m_fp );
      fseek( m_fp, 0, SEEK_SET );

      m_fileBuf = new uint8_t[m_fileLength];
      m_bufPos = m_fileBuf;

      fread( m_fileBuf, m_fileLength, 1, m_fp );
      fclose( m_fp );

      m_model = model;

      err = readFileHeader();
      if ( err == Model::ERROR_NONE )
      {
          ChunkHeaderT header;
          if ( m_isBinary )
          {
              while ( readBChunkHeader( header ) && header.type != CT_EOF )
              {
                  m_chunkStart = m_bufPos;
                  m_chunkEnd   = m_chunkStart + header.length;

                  switch ( header.type )
                  {
                      case CT_Polygon:
                          readPolygonChunk( header );
                          break;
                      case CT_Material:
                          readMaterialChunk( header );
                          break;
                      default:
                          readBUnknownChunk( header );
                          break;
                  }
              }
          }
          else
          {
              while ( readAChunkHeader( header ) && header.type != CT_EOF )
              {
                  m_chunkStart = m_bufPos;
                  m_chunkEnd   = m_chunkStart + header.length;

                  switch ( header.type )
                  {
                      case CT_Polygon:
                          readPolygonChunk( header );
                          break;
                      case CT_Material:
                          readMaterialChunk( header );
                          break;
                      default:
                          readAUnknownChunk( header );
                          break;
                  }
              }
          }
      }

      delete[] m_fileBuf;
      m_fileBuf = NULL;
      m_bufPos  = NULL;
      m_fp = NULL;

      _invertModelNormals( model );
   }
   else
   {
      switch ( errno )
      {
         case EACCES:
         case EPERM:
            return Model::ERROR_NO_ACCESS;
         case ENOENT:
            return Model::ERROR_NO_FILE;
         case EISDIR:
            return Model::ERROR_BAD_DATA;
         default:
            return Model::ERROR_FILE_OPEN;
      }
   }

   return err;
}

Model::ModelErrorE CobFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o  )
{
   if ( filename == NULL || filename[0] == '\0' )
   {
      return Model::ERROR_BAD_ARGUMENT;
   }

   m_fp = fopen( filename, "wb" );

   if ( m_fp == NULL )
   {
      switch ( errno )
      {
         case EACCES:
         case EPERM:
            return Model::ERROR_NO_ACCESS;
         case ENOENT:
            return Model::ERROR_NO_FILE;
         case EISDIR:
            return Model::ERROR_BAD_DATA;
         default:
            return Model::ERROR_FILE_OPEN;
      }
   }

   m_model = model;

   _invertModelNormals( model );

   m_modelPath = "";
   m_modelBaseName = "";
   m_modelFullName = "";

   normalizePath( filename, m_modelFullName, m_modelPath, m_modelBaseName );
      
   m_lastChunkSizeOffset = 0;
   m_nextChunkId         = 10001;

   fwrite( "Caligari V00.01BLH             \n", 32, 1, m_fp );
   writeBUngrouped();
   writeBGrouped();
   writeBEOF();

   fclose( m_fp );

   _invertModelNormals( model );
   model->operationComplete( "Invert normals for save" );

   return Model::ERROR_NONE;
}

bool CobFilter::canRead( const char * filename )
{
   log_debug( "canRead( %s )\n", filename );
   log_debug( "  true\n" );
   return true;
}

bool CobFilter::canWrite( const char * filename )
{
   log_debug( "canWrite( %s )\n", filename );
   log_debug( "  false\n" );
   return false;
}

bool CobFilter::canExport( const char * filename )
{
   log_debug( "canExport( %s )\n", filename );
   log_debug( "  true\n" );
   return true;
}

bool CobFilter::isSupported( const char * filename )
{
   log_debug( "isSupported( %s )\n", filename );
   unsigned len = strlen( filename );

   if ( len >= 4 && strcasecmp( &filename[len-4], ".cob" ) == 0 )
   {
      log_debug( "  true\n" );
      return true;
   }
   else
   {
      log_debug( "  false\n" );
      return false;
   }
}

list< string > CobFilter::getReadTypes()
{
   list<string> rval;
   rval.push_back( "*.cob" );
   return rval;
}

list< string > CobFilter::getWriteTypes()
{
   list<string> rval;
   rval.push_back( "*.cob" );
   return rval;
}

//------------------------------------------------------------------
// Protected Methods
//------------------------------------------------------------------

CobFilter::ChunkTypeE CobFilter::chunkCharsToType( const char * str )
{
    if ( strcmp( str, "END " ) == 0 )
    {
        return CT_EOF;
    }
    else if ( strcmp( str, "PolH" ) == 0 )
    {
        return CT_Polygon;
    }
    else if ( strcmp( str, "Mat1" ) == 0 )
    {
        return CT_Material;
    }
    return CT_Unknown;
}

CobFilter::MeshMaterialT * CobFilter::getMeshMaterial( int chunkId, int matNumber )
{
   for ( size_t m = 0; m < m_meshMaterials.size(); m++ )
   {
      if ( m_meshMaterials[m].meshChunkId == chunkId
            &&  m_meshMaterials[m].matNumber == matNumber )
      {
         return &m_meshMaterials[m];
      }
   }

   log_debug( "creating material for poly mesh %d, material %d\n", 
         chunkId, matNumber );

   MeshMaterialT mMat;
   mMat.meshChunkId = chunkId;
   mMat.matNumber   = matNumber;
   mMat.groupId     = -1;

   m_meshMaterials.push_back( mMat );

   return &m_meshMaterials.back();
}

int32_t CobFilter::getNextChunkId()
{
    return m_nextChunkId++;
}

//------------------------------------------------------------------
// Common read functions

Model::ModelErrorE CobFilter::readFileHeader()
{
    if ( strncmp( (char*) m_bufPos, "Caligari ", 9 ) != 0 )
    {
        return Model::ERROR_BAD_MAGIC;
    }
    m_bufPos += 9;

    if ( strncmp( (char*) m_bufPos, "V00.01", 6 ) != 0 )
    {
        return Model::ERROR_UNSUPPORTED_VERSION;
    }
    m_bufPos += 6;

    if ( m_bufPos[0] == 'B' )
    {
        m_isBinary = true;
    }
    else
    {
        m_isBinary = false;
    }
    m_bufPos++;

    if ( strncmp( (char*) m_bufPos, "LH", 2 ) == 0 )
    {
        m_isLittleEndian = true;
    }
    else
    {
        m_isLittleEndian = false;
    }
    m_bufPos += 2;

    // Last 14 bytes are pad and newline
    m_bufPos += 14;

    return Model::ERROR_NONE;
}

bool CobFilter::readPolygonChunk( const CobFilter::ChunkHeaderT & header )
{
    log_debug( "reading polygon chunk %d of %d bytes\n", 
          header.chunkId, header.length );

    PolygonMeshT mesh;
    if ( m_isBinary )
    {
        readBPolygonChunk( header, mesh );
    }
    else
    {
        readAPolygonChunk( header, mesh );
    }

    log_debug( "verts %d uvs %d faces %d\n", 
          mesh.vertexList.size(),
          mesh.uvList.size(),
          mesh.faceList.size() );

    int vertBase = m_model->getVertexCount();

    size_t v;
    for ( v = 0; v < mesh.vertexList.size(); v++ )
    {
        m_model->addVertex( mesh.vertexList[v].coord[0], 
                mesh.vertexList[v].coord[1], 
                mesh.vertexList[v].coord[2] );
    }

    MeshMaterialT * mMat = NULL;

    // Faces
    size_t fcount = mesh.faceList.size();
    for ( size_t f = 0; f < fcount; f++ )
    {
        PolygonFaceT * face = &mesh.faceList[f];
        if ( mMat == NULL 
              || mMat->matNumber != face->matNumber )
        {
           mMat = getMeshMaterial( header.chunkId, face->matNumber );
           if ( mMat->groupId < 0 )
           {
              mMat->groupId = m_model->addGroup( mesh.name.c_str() );
           }
        }
        if ( (face->flags & FF_HOLE) == 0 )
        {
            size_t vcount = face->faceVertices.size();
            if ( vcount > 2 )
            {
                size_t v = 0;
                for ( v = 0; v < vcount - 2; v++ )
                {
                    int tri = m_model->addTriangle( 
                            face->faceVertices[0].vertex + vertBase,
                            face->faceVertices[v+1].vertex + vertBase,
                            face->faceVertices[v+2].vertex + vertBase );

                    if ( !mesh.uvList.empty() )
                    {
                       m_model->setTextureCoords( tri, 0,
                             mesh.uvList[ face->faceVertices[0].uvCoord ].u,
                             mesh.uvList[ face->faceVertices[0].uvCoord ].v );
                       m_model->setTextureCoords( tri, 1,
                             mesh.uvList[ face->faceVertices[v+1].uvCoord ].u,
                             mesh.uvList[ face->faceVertices[v+1].uvCoord ].v );
                       m_model->setTextureCoords( tri, 2,
                             mesh.uvList[ face->faceVertices[v+2].uvCoord ].u,
                             mesh.uvList[ face->faceVertices[v+2].uvCoord ].v );
                    }

                    m_model->addTriangleToGroup( mMat->groupId, tri );
                }
            }
        }
    }

    m_bufPos = m_chunkEnd;
    return true;
}

bool CobFilter::readMaterialChunk( const CobFilter::ChunkHeaderT & header )
{
    log_debug( "reading material chunk %d of %d bytes\n", 
          header.chunkId, header.length );

    MaterialT material;
    if ( m_isBinary )
    {
        readBMaterialChunk( header, material );
    }
    else
    {
        readAMaterialChunk( header, material );
    }

    log_debug( "read material %d\n", material.matNumber );

    log_debug( " mat color: %f %f %f %f\n", 
          material.red   ,
          material.green ,
          material.blue  ,
          material.alpha );

    log_debug( " mat coeff: %f %f %f %f\n", 
          material.ambient    ,
          material.specular   ,
          material.hilight    ,
          material.refraction );

    // Texture map
    if ( !material.texFile.empty() )
    {
       Texture * tex = TextureManager::getInstance()->getTexture( material.texFile.c_str() );
       material.materialId = m_model->addTexture( tex );
    }
    else
    {
       char name[64];
       sprintf( name, "Material%d", m_model->getTextureCount() );
       log_debug( " using name '%s'\n", name );
       material.materialId = m_model->addColorMaterial( name );
    }

    float fval[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    fval[0] = material.red;
    fval[1] = material.green;
    fval[2] = material.blue;
    fval[3] = material.alpha;
    m_model->setTextureDiffuse( material.materialId, fval );

    fval[0] = material.red    * material.ambient;
    fval[1] = material.green  * material.ambient;
    fval[2] = material.blue   * material.ambient;
    fval[3] = material.alpha  * material.ambient;
    m_model->setTextureAmbient( material.materialId, fval );

    fval[0] = material.red    * material.specular;
    fval[1] = material.green  * material.specular;
    fval[2] = material.blue   * material.specular;
    fval[3] = material.alpha  * material.specular;
    m_model->setTextureSpecular( material.materialId, fval );

    fval[0] = material.red    * material.hilight;
    fval[1] = material.green  * material.hilight;
    fval[2] = material.blue   * material.hilight;
    fval[3] = material.alpha  * material.hilight;
    m_model->setTextureEmissive( material.materialId, fval );

    MeshMaterialT * mMat = getMeshMaterial( header.parentId, material.matNumber );

    if ( mMat->groupId >= 0 )
    {
       m_model->setGroupTextureId( mMat->groupId, material.materialId );
       m_model->setGroupAngle( mMat->groupId, material.facetAngle );

       if ( !material.texFile.empty() )
       {
          if( !_float_equiv( material.uOffset, 0.0f )
                || !_float_equiv( material.vOffset, 0.0f )
                || !_float_equiv( material.uRepeat, 0.1f )
                || !_float_equiv( material.vRepeat, 0.1f ) )
          {
             log_debug( "adjusting coordinates for group %d (chunk %d)\n", 
                   mMat->groupId, mMat->meshChunkId );
             std::list<int> triangles = m_model->getGroupTriangles( mMat->groupId );

             std::list<int>::iterator it;
             for ( it = triangles.begin(); it != triangles.end(); it++ )
             {
                float u = 0.0f;
                float v = 0.0f;
                for ( int i = 0; i < 3; i++ )
                {
                   m_model->getTextureCoords( (*it), i, u, v );
                   u = (u * material.uRepeat) + material.uOffset;
                   v = (v * material.vRepeat) + material.vOffset;
                   m_model->setTextureCoords( (*it), i, u, v );
                }
             }
          }
       }
    }

    m_bufPos = m_chunkEnd;
    return true;
}


//------------------------------------------------------------------
// ASCII read functions

void CobFilter::skipASpace()
{
    /*
    log_debug( "skipping space at '%c%c%c%c'\n", 
            m_bufPos[0],
            m_bufPos[1],
            m_bufPos[2],
            m_bufPos[3] );
            */
    while( isspace( m_bufPos[0] ) )
    {
        m_bufPos++;
    }
}

bool CobFilter::skipAString( const char * str )
{
    skipASpace();
    /*
    log_debug( "skipping string %s at '%c%c%c%c'\n", str,
            m_bufPos[0],
            m_bufPos[1],
            m_bufPos[2],
            m_bufPos[3] );
            */
    size_t len = strlen(str);
    if ( strncmp( (char *) m_bufPos, str, len) == 0 )
    {
        m_bufPos += len;
        return true;
    }
    return false;
}

int32_t CobFilter::readALong()
{
    skipASpace();
    /*
    log_debug( "reading long at '%c%c%c%c'\n", 
            m_bufPos[0],
            m_bufPos[1],
            m_bufPos[2],
            m_bufPos[3] );
            */
    int32_t rval = atoi( (char *) m_bufPos );
    if ( m_bufPos[0] == '-' )
    {
        m_bufPos++;
    }
    while ( isdigit( m_bufPos[0] ) )
    {
        m_bufPos++;
    }

    return rval;
}

int16_t CobFilter::readAShort()
{
    return readALong();
}

char CobFilter::readAChar()
{
    char rval = (char) m_bufPos[0];
    m_bufPos++;
    return rval;
}

float CobFilter::readAFloat()
{
    skipASpace();
    float rval = 0.0f;
    size_t len = 0;
    if ( sscanf( (char*) m_bufPos, "%f%n", &rval, &len ) > 0 )
    {
        m_bufPos += len;
    }
    return rval;
}

double CobFilter::readADouble()
{
    skipASpace();
    float rval = 0.0;
    size_t len = 0;
    if ( sscanf( (char*) m_bufPos, "%f%n", &rval, &len ) > 0 )
    {
        m_bufPos += len;
    }
    return rval;
}

bool CobFilter::readAString( std::string & str )
{
    skipASpace();

    size_t len = 0;
    while ( !isspace( m_bufPos[len] ) )
    {
        len++;
    }
    str.assign( (char*) m_bufPos, len );
    m_bufPos += len;

    return true;
}

bool CobFilter::readAName( std::string & str )
{
    skipAString( "Name" );

    readAString( str );
    const char * ptr = str.c_str();
    const char * off = strchr( ptr, ',' );

    // Command and after is dupecount, and ignore and remove it
    if ( off != NULL )
    {
        str.resize( off - ptr );
    }

    return true;
}

bool CobFilter::readALocalAxes( CobFilter::LocalAxesT & axes )
{
    int i = 0;

    skipAString( "center" );
    for ( i = 0; i < 3; i++ )
    {
        axes.center[i] = readADouble();
    }

    skipAString( "x axis" );
    for ( i = 0; i < 3; i++ )
    {
        axes.xAxis[i]  = readADouble();
    }

    skipAString( "y axis" );
    for ( i = 0; i < 3; i++ )
    {
        axes.yAxis[i]  = readADouble();
    }

    skipAString( "z axis" );
    for ( i = 0; i < 3; i++ )
    {
        axes.zAxis[i]  = readADouble();
    }
    return true;
}

bool CobFilter::readACurrentPosition( CobFilter::CurrentPositionT & pos )
{
    int r = 0;
    int c = 0;

    skipAString( "Transform" );
    for ( r = 0; r < 3; r++ )
    {
        for ( c = 0; c < 4; c++ )
        {
            pos.mat[r][c] = readADouble();
        }
    }

    // ignore last row
    for ( c = 0; c < 4; c++ )
    {
        readADouble();
    }

    return true;
}

bool CobFilter::readAChunkHeader( CobFilter::ChunkHeaderT & header )
{
    strncpy( header.chunkChars, (char*) m_bufPos, 4 );
    header.chunkChars[4] = '\0';
    m_bufPos += 4;

    header.type = chunkCharsToType( header.chunkChars );
    if ( header.type == CT_EOF )
    {
        return true;  // We're done
    }

    skipASpace();
    if ( m_bufPos[0] != 'V' )
    {
        log_error( "version indicator not found\n" );
        return false;
    }
    m_bufPos++;

    header.majorVersion = readAShort();
    m_bufPos++;
    header.minorVersion = readAShort();
    skipASpace();

    skipAString( "Id" );
    header.chunkId      = readALong();
    skipAString( "Parent" );
    header.parentId     = readALong();
    skipAString( "Size" );
    header.length       = readALong();

    return true;
}

bool CobFilter::readAPolygonChunk( const CobFilter::ChunkHeaderT & header,
        CobFilter::PolygonMeshT & mesh )
{
    // Versions 0.2 through 0.5 are identical
    // Version 0.6+ adds additional flags
    // 
    // Name
    // Local Axes
    // Current Position
    // Local Vertex List
    // UV Vertex List
    // Face List
    // Additional Flags (0.6+)

    readAName( mesh.name );

    readALocalAxes( mesh.localAxes );
    readACurrentPosition( mesh.currentPosition );

    Matrix m;
    mesh.currentPosition.toMatrix( m );

    // Vertices
    skipAString( "World Vertices" );
    size_t vcount = readALong();
    for ( size_t v = 0; v < vcount; v++ )
    {
        double coord[4];
        coord[0] = readAFloat();
        coord[1] = readAFloat();
        coord[2] = readAFloat();
        coord[3] = 1.0;

        m.apply( coord );

        VertexT vert;
        vert.coord[0] = coord[0];
        vert.coord[1] = coord[1];
        vert.coord[2] = coord[2];

        mesh.vertexList.push_back( vert );
    }


    // UV Coordinates
    skipAString( "Texture Vertices" );
    size_t uvcount = readALong();
    TextureCoordT tc;
    for ( size_t uv = 0; uv < uvcount; uv++ )
    {
        tc.u = readAFloat();
        tc.v = readAFloat();

        mesh.uvList.push_back( tc );
    }

    // Faces
    skipAString( "Faces" );
    size_t fcount = readALong();
    for ( size_t f = 0; f < fcount; f++ )
    {
        PolygonFaceT face;

        skipASpace();

        if ( skipAString( "Face verts" ) )
        {
            // Read face
            size_t vcount = readALong();

            skipAString( "flags" );
            face.flags = readALong();

            skipAString( "mat" );
            face.matNumber = readALong();

            while( vcount )
            {
                PolygonVertexT vert;
                skipAString("<");
                vert.vertex = readALong();
                skipAString(",");
                vert.uvCoord = readALong();
                skipAString(">");

                vcount--;
                face.faceVertices.push_back( vert );
            }
        }
        else
        {
            // Read hole
            // HOLE faces are not supported, ignore them
            skipAString( "Hole verts" );
            size_t vcount = readALong();

            log_warning( "skipping %d vertices in hole\n", vcount );

            char * pos = (char *) m_bufPos;
            while ( pos && vcount > 0 )
            {
                pos = strchr( pos, '>' ); // end of vertex indicator
                if ( pos )
                {
                    pos++;
                }
                vcount--;
            }

            m_bufPos = (uint8_t *) pos;
        }

        mesh.faceList.push_back( face );
    }

    return true;
}

bool CobFilter::readAMaterialChunk( const CobFilter::ChunkHeaderT & header, CobFilter::MaterialT & material )
{
    skipAString( "mat#" );
    material.matNumber = readALong();
    skipAString( "shader:" );
    std::string ignoreStr;
    readAString( ignoreStr ); // shader type
    skipAString( "facet:" );

    std::string facetType;
    readAString( facetType );

    material.facetAngle = 180;
    if ( strcmp( facetType.c_str(), "faceted" ) == 0 )
    {
       material.facetAngle = 0;
    }
    else if ( strncmp( facetType.c_str(), "auto", 4 ) == 0 )
    {
       material.facetAngle = atoi( &(facetType.c_str()[4]) );
    }

    skipAString( "rgb" );

    material.red   = readAFloat();
    skipAString( "," );
    material.green = readAFloat();
    skipAString( "," );
    material.blue  = readAFloat();

    skipAString( "alpha" );
    material.alpha = readAFloat();

    skipAString( "ka" );
    material.ambient    = readAFloat();
    skipAString( "ks" );
    material.specular   = readAFloat();
    skipAString( "exp" );
    material.hilight    = readAFloat();
    skipAString( "ior" );
    material.refraction = readAFloat();

    // Environment map
    if ( skipAString("environment:" ) )
    {
       size_t len = readALong(); // file name length
       m_bufPos += len; // skip environment file name
       skipAString( "flags" );
       readALong(); // skip flags
    }

    // Texture map
    if ( skipAString("texture:" ) )
    {
       size_t len = readALong(); // file name length
       std::string path;
       path.assign( (char *) m_bufPos, len );
       m_bufPos += len; // scan past filename

       replaceBackslash( path );

       std::string file = getFileNameFromPath( path.c_str() );

       std::string newPath = m_modelPath + "/";
       log_debug( "texture filename is '%s'\n", file.c_str() );

       material.texFile = newPath + fixFileCase( newPath.c_str(), file.c_str() );

       skipAString( "offset" );
       material.uOffset = readAFloat();
       skipAString( "," );
       material.vOffset = readAFloat();

       skipAString( "repeats" );
       material.uRepeat = readAFloat();
       skipAString( "," );
       material.vRepeat = readAFloat();

       skipAString( "flags" );
       readAFloat(); // flags, ignored
    }

    // Bump map ignored

    return true;
}

bool CobFilter::readAUnknownChunk( const CobFilter::ChunkHeaderT & header )
{
    //log_debug( "skipping unknown chunk %s of %d bytes\n", header.chunkChars, header.length );
    m_bufPos = m_chunkEnd;
    return true;
}

//------------------------------------------------------------------
// Binary read functions

int32_t CobFilter::readBLong()
{
    uint32_t rval = *((uint32_t *) m_bufPos);
    m_bufPos += sizeof( rval );
    rval = ltoh_u32( rval );
    return rval;
}

int16_t CobFilter::readBShort()
{
    uint16_t rval = *((uint16_t *) m_bufPos);
    m_bufPos += sizeof( rval );
    rval = ltoh_u16( rval );
    return rval;
}

char CobFilter::readBChar()
{
    char rval = *((char *) m_bufPos);
    m_bufPos += sizeof( rval );
    return rval;
}

float CobFilter::readBFloat()
{
    float rval = *((float *) m_bufPos);
    m_bufPos += sizeof( rval );
    rval = ltoh_float( rval );
    return rval;
}

bool CobFilter::readBString( std::string & str )
{
    log_debug( "reading string at %p\n", m_bufPos );
    log_debug( "value is %02x %02x\n", m_bufPos[0], m_bufPos[1] );
    size_t len = readBShort();
    log_debug( "strlen is %d at %p\n", len, m_bufPos );

    str.assign( (char *) m_bufPos, len );
    m_bufPos += len;

    log_debug( "done reading string '%s' at %p\n", str.c_str(), m_bufPos );

    return true;
}

bool CobFilter::readBName( std::string & str )
{
    readBShort(); // dup count, ignored
    readBString( str );
    return true;
}

bool CobFilter::readBLocalAxes( CobFilter::LocalAxesT & axes )
{
    int i = 0;
    for ( i = 0; i < 3; i++ )
    {
        axes.center[i] = readBFloat();
    }
    for ( i = 0; i < 3; i++ )
    {
        axes.xAxis[i] = readBFloat();
    }
    for ( i = 0; i < 3; i++ )
    {
        axes.yAxis[i] = readBFloat();
    }
    for ( i = 0; i < 3; i++ )
    {
        axes.zAxis[i] = readBFloat();
    }
    return true;
}

bool CobFilter::readBCurrentPosition( CobFilter::CurrentPositionT & pos )
{
    int r = 0;
    int c = 0;
    for ( r = 0; r < 3; r++ )
    {
        for ( c = 0; c < 4; c++ )
        {
            pos.mat[r][c] = readBFloat();
        }
    }
    return true;
}

bool CobFilter::readBChunkHeader( CobFilter::ChunkHeaderT & header )
{
    strncpy( header.chunkChars, (char*) m_bufPos, 4 );
    header.chunkChars[4] = '\0';
    m_bufPos += 4;

    header.type = chunkCharsToType( header.chunkChars );
    if ( header.type == CT_EOF )
    {
        return true;  // We're done
    }

    header.majorVersion = readBShort();
    header.minorVersion = readBShort();
    header.chunkId      = readBLong();
    header.parentId     = readBLong();
    header.length       = readBLong();

    return true;
}

bool CobFilter::readBPolygonChunk( const CobFilter::ChunkHeaderT & header,
        CobFilter::PolygonMeshT & mesh )
{
    // Versions 0.2 through 0.5 are identical
    // Version 0.6+ adds additional flags
    // 
    // Name
    // Local Axes
    // Current Position
    // Local Vertex List
    // UV Vertex List
    // Face List
    // Additional Flags (0.6+)

    readBName( mesh.name );
    log_debug( "poly chunk name is %s\n", mesh.name.c_str() );

    readBLocalAxes( mesh.localAxes );
    readBCurrentPosition( mesh.currentPosition );

    Matrix m;
    mesh.currentPosition.toMatrix( m );
    fflush( stdout );

    // Vertices
    size_t vcount = readBLong();
    log_debug( "reading %d vertices\n", vcount );
    for ( size_t v = 0; v < vcount; v++ )
    {
        double coord[4];
        coord[0] = readBFloat();
        coord[1] = readBFloat();
        coord[2] = readBFloat();
        coord[3] = 1.0;

        m.apply( coord );

        VertexT vert;
        vert.coord[0] = coord[0];
        vert.coord[1] = coord[1];
        vert.coord[2] = coord[2];
        mesh.vertexList.push_back( vert );
    }


    // UV Coordinates
    size_t uvcount = readBLong();
    log_debug( "reading %d texture coordiantes\n", uvcount );
    TextureCoordT tc;
    for ( size_t uv = 0; uv < uvcount; uv++ )
    {
        tc.u = readBFloat();
        tc.v = readBFloat();

        mesh.uvList.push_back( tc );
    }

    // Faces

    size_t fcount = readBLong();
    log_debug( "reading %d faces\n", fcount );
    for ( size_t f = 0; f < fcount; f++ )
    {
        PolygonFaceT face;

        face.flags = readBChar();
        size_t vcount = readBShort();

        if ( (face.flags & FF_HOLE) == 0 )
        {
            face.matNumber = readBShort();

            PolygonVertexT pv;

            size_t v = 0;

            for ( v = 0; v < vcount; v++ )
            {
                pv.vertex  = readBLong();
                pv.uvCoord = readBLong();
                face.faceVertices.push_back( pv );
            }
        }
        else
        {
            log_warning( "skipping %d vertices in hole\n", vcount );
            // HOLE faces are not supported, ignore them
            m_bufPos += vcount * 8; // Each hole vertex is 8 bytes
        }

        mesh.faceList.push_back( face );
    }

    return true;
}

bool CobFilter::readBMaterialChunk( const CobFilter::ChunkHeaderT & header, CobFilter::MaterialT & material )
{
    material.matNumber  = readBShort();
    material.shaderType = readBChar();
    material.facetType  = readBChar();
    material.facetAngle = readBChar();

    if ( material.facetType == 's' )
    {
       material.facetAngle = 180;
    }
    else if ( material.facetType == 'f' )
    {
       material.facetAngle = 0;
    }

    material.red   = readBFloat();
    material.green = readBFloat();
    material.blue  = readBFloat();
    material.alpha = readBFloat();

    material.ambient    = readBFloat();
    material.specular   = readBFloat();
    material.hilight    = readBFloat();
    material.refraction = readBFloat();

    // Environment map (skip)
    if ( strncmp( (char *) m_bufPos, "e:", 2 ) == 0 )
    {
       m_bufPos += 3; // skip environment identifier and flags
       std::string path = "";
       readBString( path );
    }

    // Texture map
    if ( strncmp( (char *) m_bufPos, "t:", 2 ) == 0 )
    {
       m_bufPos += 2; // skip texture identifier

       readBChar(); // ignore flags

       std::string path = "";
       readBString( path );
       replaceBackslash( path );

       std::string file = getFileNameFromPath( path.c_str() );

       log_debug( "texture filename is '%s'\n", file.c_str() );

       std::string newPath = m_modelPath + std::string("/");
       material.texFile = newPath + fixFileCase( newPath.c_str(), file.c_str() );

       material.uOffset = readBFloat();
       material.vOffset = readBFloat();
       material.uRepeat = readBFloat();
       material.vRepeat = readBFloat();
    }

    // Bump map (skip)
    if ( strncmp( (char *) m_bufPos, "b:", 2 ) == 0 )
    {
       m_bufPos += 2; // skip bump map identifier and flags

       std::string path = "";
       readBString( path );

       readBFloat(); // U Offset
       readBFloat(); // V Offset
       readBFloat(); // U Repeat
       readBFloat(); // V Repeat
       readBFloat(); // Amplitude
    }

    return true;
}

bool CobFilter::readBUnknownChunk( const CobFilter::ChunkHeaderT & header )
{
    //log_debug( "skipping unknown chunk %s of %d bytes\n", header.chunkChars, header.length );
    m_bufPos = m_chunkEnd;
    return true;
}

//------------------------------------------------------------------
// Binary write functions

void CobFilter::writeBLong( int32_t val )
{
    int32_t writeVal = htol_u32( val );
    fwrite( &writeVal, sizeof(writeVal), 1, m_fp );
}

void CobFilter::writeBShort( int16_t val )
{
    int16_t writeVal = htol_u16( val );
    fwrite( &writeVal, sizeof(writeVal), 1, m_fp );
}

void CobFilter::writeBChar( char val )
{
    int8_t writeVal = (int8_t) val;
    fwrite( &writeVal, sizeof(writeVal), 1, m_fp );
}

void CobFilter::writeBFloat( float val )
{
    float writeVal = htol_float( val );
    fwrite( &writeVal, sizeof(writeVal), 1, m_fp );
}

void CobFilter::writeBString( const std::string & str )
{
    size_t len = str.size();
    writeBShort( (int16_t) len );
    fwrite( str.c_str(), len, 1, m_fp );
}

void CobFilter::writeBName( const std::string & str )
{
    writeBShort( 0 ); // dup count, ignored
    writeBString( str );
}

void CobFilter::writeBStandardAxes()
{
    int i = 0;

    // Center
    for ( i = 0; i < 3; i++ )
    {
        writeBFloat( 0.0f );
    }

    // X Axis
    writeBFloat( 1.0f );
    writeBFloat( 0.0f );
    writeBFloat( 0.0f );

    // Y Axis
    writeBFloat( 0.0f );
    writeBFloat( 1.0f );
    writeBFloat( 0.0f );

    // Z Axis
    writeBFloat( 0.0f );
    writeBFloat( 0.0f );
    writeBFloat( 1.0f );
}

void CobFilter::writeBStandardPosition()
{
    int r = 0;
    int c = 0;
    
    // Last row is omitted (assumed 0,0,0,1)
    for ( r = 0; r < 3; r++ )
    {
        for ( c = 0; c < 4; c++ )
        {
            writeBFloat( (c == r) ? 1.0f : 0.0f );
        }
    }
}

void CobFilter::writeBChunkHeader( const CobFilter::ChunkHeaderT & header )
{
    switch ( header.type )
    {
        case CT_EOF:
            fwrite( "END ", 4, 1, m_fp );
            break;
        case CT_Polygon:
            fwrite( "PolH", 4, 1, m_fp );
            break;
        case CT_Material:
            fwrite( "Mat1", 4, 1, m_fp );
            break;
        default:
            log_error( "Uknown type %d, file will be corrupt\n", (int) header.type );
            fwrite( "XXXX", 4, 1, m_fp );
            break;
    }

    writeBShort( header.majorVersion );
    writeBShort( header.minorVersion );
    writeBLong( header.chunkId );
    writeBLong( header.parentId );

    // save offset of chunk size writeBChunkSize() can update it later
    m_lastChunkSizeOffset = ftell( m_fp );
    writeBLong( header.length );
}

void CobFilter::writeBChunkSize()
{
    if ( m_lastChunkSizeOffset != 0 )
    {
        size_t currentOffset = ftell( m_fp );

        // Seek back to chunk size offset
        fseek( m_fp, m_lastChunkSizeOffset, SEEK_SET );

        // Get chunk size and write it into chunk header
        int32_t chunkSize = currentOffset - m_lastChunkSizeOffset - sizeof(int32_t);
        writeBLong( chunkSize );

        // Seek back to where we were in the file
        fseek( m_fp, currentOffset, SEEK_SET );
    }
}

void CobFilter::writeBTriangleGroup( const std::list<int> & triList, const std::string & constName )
{
    std::string name = constName;

    std::map<int,int> vertMap;
    std::vector<VertexT> vertList;
    std::vector<TextureCoordT> uvList;

    for ( size_t idx = 0; idx < name.size(); idx++ )
    {
        if ( !isalnum( name[idx] ) )
        {
            name[idx] = '_';
        }
    }

    writeBName( name );
    writeBStandardAxes();
    writeBStandardPosition();

    size_t tcount = triList.size();
    std::list<int>::const_iterator it;

    for ( it = triList.begin(); it != triList.end(); it++ )
    {
        for ( size_t v = 0; v < 3; v++ )
        {
            int vert = m_model->getTriangleVertex( *it, v );

            if ( vertMap.find( vert ) == vertMap.end() )
            {
                vertMap[vert] = vertList.size();

                VertexT vertex;
                m_model->getVertexCoords( vert, vertex.coord );
                vertList.push_back( vertex );
            }

            // TODO could optimize by combining matching coords
            TextureCoordT uv;
            m_model->getTextureCoords( *it, v, uv.u, uv.v );
            uvList.push_back( uv );
        }
    }

    // Write local vertex count
    size_t vcount = vertList.size();
    log_debug( "writing %d vertices for group\n", vcount );
    writeBLong( vcount );

    // Write local vertex positions
    for ( size_t v = 0; v < vcount; v++ )
    {
       writeBFloat( vertList[v].coord[0] );
       writeBFloat( vertList[v].coord[1] );
       writeBFloat( vertList[v].coord[2] );
    }

    // Write uv count
    size_t uvcount = uvList.size();
    writeBLong( uvcount );

    // Write uv data
    // TODO deal with out of range UVs (related to UV Offset/Repeat on Material?)
    for ( size_t uv = 0; uv < uvcount; uv++ )
    {
        writeBFloat( uvList[uv].u );
        writeBFloat( uvList[uv].v );
    }

    // Write face count
    writeBLong( tcount );

    // Write face data
    size_t t = 0;
    for ( it = triList.begin(); it != triList.end(); it++ )
    {
        writeBChar( 0 );
        writeBShort( 3 );
        writeBShort( 0 );

        for ( int i = 0; i < 3; i++ )
        {
            int vert = m_model->getTriangleVertex( *it, i );
            writeBLong( vertMap[ vert ] );
            writeBLong( t * 3 + i );  // UV is deterministic
        }
        t++;
    }

}

void CobFilter::writeBUngrouped()
{
    std::list<int> triList = m_model->getUngroupedTriangles();

    if ( !triList.empty() )
    {
        ChunkHeaderT header;

        header.type          = CT_Polygon;
        header.majorVersion  = 0;
        header.minorVersion  = 4;
        header.chunkId       = getNextChunkId();
        header.parentId      = 0;
        header.length        = 0;

        // call writeBChunkSize() when done with chunk to
        // correct chunk size field
        writeBChunkHeader( header );

        writeBTriangleGroup( triList, "Ungrouped" );

        // correct chunk size in header
        writeBChunkSize();

        writeBDefaultMaterial( header.chunkId );
    }
}

void CobFilter::writeBGrouped()
{
    size_t gcount = m_model->getGroupCount();

    for ( size_t g = 0; g < gcount; g++ )
    {
        std::list<int> triList = m_model->getGroupTriangles( g );

        if ( !triList.empty() )
        {
            ChunkHeaderT header;

            header.type          = CT_Polygon;
            header.majorVersion  = 0;
            header.minorVersion  = 4;
            header.chunkId       = getNextChunkId();
            header.parentId      = 0;
            header.length        = 0;

            // call writeBChunkSize() when done with chunk to
            // correct chunk size field
            writeBChunkHeader( header );

            writeBTriangleGroup( triList, m_model->getGroupName( g ) );

            // correct chunk size in header
            writeBChunkSize();

            int matId = m_model->getGroupTextureId( g );
            if ( matId >= 0 )
            {
                writeBMaterial( header.chunkId, matId, g );
            }
            else
            {
                writeBDefaultMaterial( header.chunkId );
            }
        }
    }
}

void CobFilter::writeBMaterial( int32_t parentId, size_t materialNumber, int groupNumber )
{
    if ( materialNumber < (size_t) m_model->getTextureCount() )
    {
       ChunkHeaderT header;

       header.type          = CT_Material;
       header.majorVersion  = 0;
       header.minorVersion  = 6;
       header.chunkId       = getNextChunkId();
       header.parentId      = parentId;
       header.length        = 0;

       // call writeBChunkSize() when done with chunk to
       // correct chunk size field
       writeBChunkHeader( header );

       writeBShort( 0 );  // material index, always zero (only one material per group)
       writeBChar( 'p' ); // shader type

       int angle = m_model->getGroupAngle( groupNumber );
       if ( angle > 0 )
       {
          if ( angle > 179 )
          {
             angle = 179;
          }
          writeBChar( 'a' );
          writeBChar( (char) angle );
       }
       else
       {
          writeBChar( 'f' );
          writeBChar( 0 );
       }

       // Color
       float fcolor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
       m_model->getTextureDiffuse( materialNumber, fcolor );

       writeBFloat( fcolor[0] ); // red
       writeBFloat( fcolor[1] ); // green
       writeBFloat( fcolor[2] ); // blue
       writeBFloat( fcolor[3] ); // alpha

       // Co-efficients

       float fval[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
       float coeff = 0.0f;

       // Ambient co-efficient
       m_model->getTextureAmbient( materialNumber, fval );
       coeff = _valToCoeff( fval );
       writeBFloat( coeff );

       // Specular co-efficient
       m_model->getTextureSpecular( materialNumber, fval );
       coeff = _valToCoeff( fval );
       writeBFloat( coeff );

       // Hilight co-efficient
       writeBFloat( 0.0f );

       // Index of refraction
       writeBFloat( 1.0f );

       // write texture filename if exists
       if ( m_model->getMaterialType( materialNumber ) == Model::Material::MATTYPE_TEXTURE )
       {
          fwrite( "t:", 2, 1, m_fp );
          writeBChar( 0x02 );
          std::string texFile = m_model->getTextureFilename( materialNumber );
          texFile = getFileNameFromPath( texFile.c_str() );
          writeBString( texFile );

          // TODO make sure these are handled correctly
          writeBFloat( 0.0 ); // U Offset
          writeBFloat( 0.0 ); // V Offset
          writeBFloat( 1.0 ); // U Repeat
          writeBFloat( 1.0 ); // V Repeat
       }

       // correct chunk size in header
       writeBChunkSize();
    }
    else
    {
       writeBDefaultMaterial( parentId );
    }
}

void CobFilter::writeBDefaultMaterial( int32_t parentId )
{
    ChunkHeaderT header;

    header.type          = CT_Material;
    header.majorVersion  = 0;
    header.minorVersion  = 6;
    header.chunkId       = getNextChunkId();
    header.parentId      = parentId;
    header.length        = 0;

    // call writeBChunkSize() when done with chunk to
    // correct chunk size field
    writeBChunkHeader( header );

    writeBShort( 0 );  // material index, always zero (only one material per group)
    writeBChar( 'p' ); // shader type
    writeBChar( 's' ); // facet type
    writeBChar( 32 );  // autofacet angle

    // Color
    writeBFloat( 1.0f ); // red
    writeBFloat( 1.0f ); // green
    writeBFloat( 1.0f ); // blue
    writeBFloat( 1.0f ); // alpha

    // Ambient co-efficient
    writeBFloat( 0.0f );

    // Specular co-efficient
    writeBFloat( 0.0f );

    // Hilight co-efficient
    writeBFloat( 0.0f );

    // Index of refraction
    writeBFloat( 1.0f );

    // correct chunk size in header
    writeBChunkSize();
}

void CobFilter::writeBEOF()
{
    ChunkHeaderT header;

    header.type          = CT_EOF;
    header.majorVersion  = 1;
    header.minorVersion  = 0;
    header.chunkId       = getNextChunkId();
    header.parentId      = 0;
    header.length        = 0;

    // call writeBChunkSize() when done with chunk to
    // correct chunk size field
    writeBChunkHeader( header );

    // correct chunk size in header
    writeBChunkSize();
}

#ifdef PLUGIN

//------------------------------------------------------------------
// Plugin functions
//------------------------------------------------------------------

extern "C" bool plugin_init()
{
   if ( s_filter == NULL )
   {
      s_filter = new CobFilter();
      FilterManager * texmgr = FilterManager::getInstance();
      texmgr->registerFilter( s_filter );
   }
   log_debug( "COB model filter plugin initialized\n" );
   return true;
}

// The filter manager will delete our registered filter.
// We have no other cleanup to do
extern "C" bool plugin_uninit()
{
   s_filter = NULL; // FilterManager deletes filters
   log_debug( "COB model filter plugin uninitialized\n" );
   return true;
}

extern "C" const char * plugin_version()
{
   return "0.1.0";
}

extern "C" const char * plugin_desc()
{
   return "COB model filter";
}

#endif // PLUGIN
