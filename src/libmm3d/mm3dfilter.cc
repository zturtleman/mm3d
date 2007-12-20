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


#include "mm3dfilter.h"

#include "model.h"
#include "texture.h"
#include "log.h"
#include "binutil.h"
#include "misc.h"
#include "endianconfig.h"
#include "mm3dport.h"
#include "msg.h"
#include "translate.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>

using std::list;
using std::string;

// Misfit MM3D format:
//
// File Header
//
// MAGIC_NUMBER    8 bytes  "MISFIT3D"
// MAJOR_VERSION   uint8    0x01
// MINOR_VERSION   uint8    0x05
// MODEL_FLAGS     uint8    0x00 (reserved)
// OFFSET_COUNT    uint8    [offc]
//
//    [OFFSET_COUNT] instances of:
//    Offset header list
//    OFFSET_HEADER   6 bytes * [offc]
//
//    Offset Header
//    OFFSET_TYPE     uint16 (highest bit 0 = Data type A, 1 = data type B)
//    OFFSET_VALUE    uint32 
//
// Data header A (Variable data size)
// DATA_FLAGS      uint16 
// DATA_COUNT      uint32 
//
// Data block A
//
// [DATA_COUNT] instances of:
// DATA_SIZE       uint32 bytes
// DATA_BLOCK_A    [DATA_SIZE] bytes
//
// Data header B (Uniform data size)
// DATA_FLAGS      uint16 
// DATA_COUNT      uint32 
// DATA_SIZE       uint32 
//
// Data block B
//
// [DATA_COUNT] instances of:
// DATA_BLOCK_B   [DATA_SIZE] bytes
//
//
// Offset A types:
//   0x1001          Meta information
//   0x1002          Unknown type information
//   0x0101          Groups
//   0x0141          Embedded textures
//   0x0142          External textures
//   0x0161          Materials
//   0x016c          Texture Projection Triangles
//   0x0191          Canvas background images
//
//   0x0301          Skeletal Animations
//   0x0321          Frame Animations
//   0x0326          Frame Animation Points
//   0x0341          Frame Relative Animations
//
// Offset B types:
//   0x0001          Vertices
//   0x0021          Triangles
//   0x0026          Triangle Normals
//   0x0041          Joints
//   0x0046          Joint Vertices
//   0x0061          Points
//   0x0106          Smooth Angles
//   0x0168          Texture Projections
//   0x0121          Texture Coordinates
//
//
// 
// Meta information data
//   KEY             ASCIIZ  <= 1024 (may not be empty)
//   VALUE           ASCIIZ  <= 1024 (may be empty)
//
// Unknown type information
//   OFFSET_TYPE     uint16
//   INFO_STRING     ASCIIZ <= 256 (inform user how to support this data)
//
//
// Vertex data
//   FLAGS           uint16
//   X_COORD         float32
//   Y_COORD         float32
//   Z_COORD         float32
//
//
// Triangle data
//   FLAGS           uint16
//   VERTEX1_INDEX   uint32
//   VERTEX2_INDEX   uint32
//   VERTEX3_INDEX   uint32
//
// Group data
//   FLAGS           uint16
//   NAME            ASCIIZ <= inf
//   TRI_COUNT       uint32
//   TRI_INDICES     uint32 * [TRI_COUNT]
//   SMOOTHNESS      uint8
//   MATERIAL_INDEX  uint32
//   
// Smooth Angles (maximum angle between smoothed normals for a group)
//   GROUP_INDEX     uint32
//   SMOOTHNESS      uint8
//   
// Weighted Influences
//   POS_TYPE        uint8
//   POS_INDEX       uint32
//   INF_INDEX       uint32
//   INF_TYPE        uint8
//   INF_WEIGHT      uint8
//   
// External texture data
//   FLAGS           uint16
//   FILENAME        ASCIIZ
//
// Embedded texture data
//   FLAGS           uint16
//   FORMAT          char8 * 4  ('JPEG', 'PNG ', 'TGA ', etc...)
//   TEXTURE_SIZE    uint32
//   DATA            uint8 * [TEXTURE_SIZE]
//
// Material
//   FLAGS           uint16
//   TEXTURE_INDEX   uint32
//   NAME            ASCIIZ
//   AMBIENT         float32 * 4
//   DIFFUSE         float32 * 4
//   SPECULAR        float32 * 4
//   EMISSIVE        float32 * 4
//   SHININESS       float32
//   
// Projection Triangles
//   PROJECTION_INDEX  uint32
//   TRI_COUNT         uint32
//   TRI_INDICES       uint32 * [TRI_COUNT]
//   
// Canvas background image
//   FLAGS           uint16
//   VIEW_INDEX      uint8
//   SCALE           float32
//   CENTER_X        float32
//   CENTER_X        float32
//   CENTER_X        float32
//   FILENAME        ASCIIZ
//
// Texture coordinates
//   FLAGS           uint16
//   TRI_INDEX       uint32
//   COORD_S         float32 * 3
//   COORD_T         float32 * 3
//
// Joint data
//   FLAGS           uint16
//   NAME            char8 * 40
//   PARENT_INDEX    int32
//   LOCAL_ROT       float32 * 3
//   LOCAL_TRANS     float32 * 3
//
// Joint Vertices
//   VERTEX_INDEX    uint32
//   JOINT_INDEX     int32
//
// Point data
//   FLAGS           uint16
//   NAME            char8 * 40
//   TYPE            int32
//   BONE_INDEX      int32
//   ROTATION        float32 * 3
//   TRANSLATION     float32 * 3
//
// Texture Projection 
//   FLAGS           uint16
//   NAME            char8 * 40
//   TYPE            int32
//   POSITION        float32 * 3
//   UP VECTOR       float32 * 3
//   SEAM VECTOR     float32 * 3
//   U MIN           float32
//   V MIN           float32
//   U MAX           float32
//   V MAX           float32
//
// Skeletal animation
//   FLAGS           uint16
//   NAME            ASCIIZ
//   FPS             float32
//   FRAME_COUNT     uint32
//
//      [FRAME_COUNT] instances of:
//      KEYFRAME_COUNT  uint32
//
//        [KEYFRAME_COUNT] instances of:
//        JOINT_INDEX     uint32
//        KEYFRAME_TYPE   uint8  (0 = rotation, 1 = translation)
//        PARAM           float32 * 3
//
// Frame animation
//   FLAGS           uint16
//   NAME            ASCIIZ <= 64
//   FPS             float32
//   FRAME_COUNT     uint32
//
//      [FRAME_COUNT] instances of:
//      
//         [VERTEX_COUNT] instances of:
//         COORD_X         float32
//         COORD_Y         float32
//         COORD_Z         float32
// 
// Frame animation points
//   FLAGS             uint16
//   FRAME_ANIM_INDEX  uint32
//   FRAME_COUNT       uint32
//
//      [FRAME_COUNT] instances of:
//      
//         [POINT_COUNT] instances of:
//         ROT_X         float32
//         ROT_Y         float32
//         ROT_Z         float32
//         TRANS_X       float32
//         TRANS_Y       float32
//         TRANS_Z       float32
// 
// Frame relative animation
//   FLAGS           uint16
//   NAME            ASCIIZ <= 64
//   FPS             float32
//   FRAME_COUNT     uint32
//
//      [FRAME_COUNT] instances of:
//      FVERT_COUNT     uint32
//      
//         [FVERT_COUNT] instances of:
//         VERTEX_INDEX
//         COORD_X_OFFSET  float32
//         COORD_Y_OFFSET  float32
//         COORD_Z_OFFSET  float32
// 

const char     MisfitFilter::MAGIC[] = "MISFIT3D";

const uint8_t  MisfitFilter::WRITE_VERSION_MAJOR = 0x01;
const uint8_t  MisfitFilter::WRITE_VERSION_MINOR = 0x05;  // FIXME bump version

const uint16_t MisfitFilter::OFFSET_TYPE_MASK  = 0x3fff;
const uint16_t MisfitFilter::OFFSET_UNI_MASK   = 0x8000;
const uint16_t MisfitFilter::OFFSET_DIRTY_MASK = 0x4000;

typedef struct _MisfitOffset_t
{
   uint16_t offsetType;
   uint32_t offsetValue;
} MisfitOffsetT;

typedef std::vector<MisfitOffsetT>  MisfitOffsetList;

typedef enum _MisfitDataTypes_e {

   // A Types
   MDT_Meta,
   MDT_TypeInfo,
   MDT_Groups,
   MDT_EmbTextures,
   MDT_ExtTextures,
   MDT_Materials,
   MDT_ProjectionTriangles,
   MDT_CanvasBackgrounds,
   MDT_SkelAnims,
   MDT_FrameAnims,
   MDT_FrameAnimPoints,
   MDT_RelativeAnims,

   // B Types
   MDT_Vertices,
   MDT_Triangles,
   MDT_TriangleNormals,
   MDT_Joints,
   MDT_JointVertices,
   MDT_Points,
   MDT_SmoothAngles,
   MDT_WeightedInfluences,
   MDT_TexProjections,
   MDT_TexCoords,

   // End of list
   MDT_EndOfFile,
   MDT_MAX
} MisfitDataTypesE;

typedef enum _MisfitFlags_e
{
   MF_HIDDEN    = 1,  // powers of 2
   MF_SELECTED  = 2,
   MF_VERTFREE  = 4,  // vertex does not have to be conntected to a face

   // Type-specific flags
   MF_MAT_CLAMP_S = 16,
   MF_MAT_CLAMP_T = 32
} MisfitFlagsE;

static const uint16_t _misfitOffsetTypes[MDT_MAX]  = {

   // Offset A types
   0x1001,         // Meta information
   0x1002,         // Unknown type information
   0x0101,         // Groups
   0x0141,         // Embedded textures
   0x0142,         // External textures
   0x0161,         // Materials
   0x016c,         // Texture Projection Triangles
   0x0191,         // Canvas background images
   0x0301,         // Skeletal Animations
   0x0321,         // Frame Animations
   0x0326,         // Frame Animation Points
   0x0341,         // Frame Relative Animations

   // Offset B types:
   0x0001,         // Vertices
   0x0021,         // Triangles
   0x0026,         // Triangle Normals
   0x0041,         // Joints
   0x0046,         // Joint Vertices
   0x0061,         // Points
   0x0106,         // Smooth Angles
   0x0146,         // Weighted Influences
   0x0168,         // Texture Projections
   0x0121,         // Texture Coordinates

   0x3fff,         // End of file
};

static const char _misfitOffsetNames[MDT_MAX][30] = {
   // Offset A types
   "Meta information",
   "Type identity",
   "Groups",
   "Embedded textures",
   "External textures",
   "Materials",
   "Texture projection triangles",
   "Canvas background images",
   "Skeletal animations",
   "Frame animations",
   "Frame animation points",
   "Frame relative animations",
   // Offset B types
   "Vertices",
   "Triangles",
   "Triangle normals",
   "Joints",
   "Joint vertices",
   "Points",
   "Max smoothing angles",
   "Weighted Influences",
   "Texture projections",
   "Texture coordinates",
   // End of file
   "End of file"
};


// File header
struct _MM3DFILE_Header_t
{
   char magic[8];
   uint8_t versionMajor;
   uint8_t versionMinor;
   uint8_t modelFlags;
   uint8_t offsetCount;
};
typedef struct _MM3DFILE_Header_t MM3DFILE_HeaderT;

// Data header A (Variable data size)
struct _MM3DFILE_DataHeaderA_t
{
   uint16_t flags;
   uint32_t count;
};
typedef struct _MM3DFILE_DataHeaderA_t MM3DFILE_DataHeaderAT;

// Data header B (Uniform data size)
struct _MM3DFILE_DataHeaderB_t
{
   uint16_t flags;
   uint32_t count;
   uint32_t size;
};
typedef struct _MM3DFILE_DataHeaderB_t MM3DFILE_DataHeaderBT;

struct _MM3DFILE_Vertex_t
{
   uint16_t  flags;
   float32_t coord[3];
};
typedef struct _MM3DFILE_Vertex_t MM3DFILE_VertexT;

const size_t FILE_VERTEX_SIZE = 14;

struct _MM3DFILE_Triangle_t
{
   uint16_t  flags;
   uint32_t  vertex[3];
};
typedef struct _MM3DFILE_Triangle_t MM3DFILE_TriangleT;

const size_t FILE_TRIANGLE_SIZE = 14;

struct _MM3DFILE_TriangleNormals_t
{
   uint16_t   flags;
   uint32_t   index;
   float32_t  normal[3][3];
};
typedef struct _MM3DFILE_TriangleNormals_t MM3DFILE_TriangleNormalsT;

const size_t FILE_TRIANGLE_NORMAL_SIZE = 42;

struct _MM3DFILE_Joint_t
{
   uint16_t  flags;
   char      name[40];
   int32_t   parentIndex;
   float32_t localRot[3];
   float32_t localTrans[3];
};
typedef struct _MM3DFILE_Joint_t MM3DFILE_JointT;

const size_t FILE_JOINT_SIZE = 70;

struct _MM3DFILE_JointVertex_t
{
   uint32_t  vertexIndex;
   int32_t   jointIndex;
};
typedef struct _MM3DFILE_JointVertex_t MM3DFILE_JointVertexT;

const size_t FILE_JOINT_VERTEX_SIZE = 8;

struct _MM3DFILE_WeightedInfluence_t
{
   uint8_t   posType;
   uint32_t  posIndex;
   uint32_t  infIndex;
   uint8_t   infType;
   int8_t    infWeight;
};
typedef struct _MM3DFILE_WeightedInfluence_t MM3DFILE_WeightedInfluenceT;

const size_t FILE_WEIGHTED_INFLUENCE_SIZE = 11;

struct _MM3DFILE_Point_t
{
   uint16_t  flags;
   char      name[40];
   int32_t   type;
   int32_t   boneIndex;
   float32_t rot[3];
   float32_t trans[3];
};
typedef struct _MM3DFILE_Point_t MM3DFILE_PointT;

const size_t FILE_POINT_SIZE = 74;

struct _MM3DFILE_SmoothAngle_t
{
   uint32_t  groupIndex;
   uint8_t   angle;
};
typedef struct _MM3DFILE_SmoothAngle_t MM3DFILE_SmoothAngleT;

const size_t FILE_SMOOTH_ANGLE_SIZE = 5;

struct _MM3DFILE_CanvasBackground_t
{
   uint16_t  flags;
   uint8_t   viewIndex;
   float32_t scale;
   float32_t center[3];
};
typedef struct _MM3DFILE_CanvasBackground_t MM3DFILE_CanvasBackgroundT;

const size_t FILE_CANVAS_BACKGROUND_SIZE = 19;

struct _MM3DFILE_SkelKeyframe_t
{
   uint32_t   jointIndex;
   uint8_t    keyframeType;
   float32_t  param[3];
};
typedef struct _MM3DFILE_SkelKeyframe_t MM3DFILE_SkelKeyframeT;

const size_t FILE_SKEL_KEYFRAME_SIZE = 17;

struct _MM3DFILE_TexCoord_t
{
   uint16_t  flags;
   uint32_t  triangleIndex;
   float32_t sCoord[3];
   float32_t tCoord[3];
};
typedef struct _MM3DFILE_TexCoord_t MM3DFILE_TexCoordT;

const size_t FILE_TEXCOORD_SIZE = 30;

const size_t FILE_TEXTURE_PROJECTION_SIZE = 98;

struct _UnknownData_t
{
   uint16_t offsetType;
   uint32_t offsetValue;
   uint32_t dataLen;
};

typedef struct _UnknownData_t UnknownDataT;

typedef std::list<UnknownDataT> UnknownDataList;

static void _addOffset( MisfitDataTypesE type, bool include, MisfitOffsetList & list )
{
   if ( include )
   {
      MisfitOffsetT mo;
      mo.offsetType = _misfitOffsetTypes[ type ];
      mo.offsetValue = 0;
      log_debug( "adding offset type %04X\n", mo.offsetType );
      list.push_back( mo );
   }
}

static void _setOffset( MisfitDataTypesE type, uint32_t offset, MisfitOffsetList & list )
{
   unsigned count = list.size();
   for ( unsigned n = 0; n < count; n++ )
   {
      if ( (list[n].offsetType & MisfitFilter::OFFSET_TYPE_MASK) == _misfitOffsetTypes[type] )
      {
         list[n].offsetValue = offset;
         log_debug( "updated offset for %04X to %08X\n", list[n].offsetType, list[n].offsetValue );
         break;
      }
   }
}

static void _setUniformOffset( MisfitDataTypesE type, bool uniform, MisfitOffsetList & list )
{
   unsigned count = list.size();
   for ( unsigned n = 0; n < count; n++ )
   {
      if ( (list[n].offsetType & MisfitFilter::OFFSET_TYPE_MASK) == _misfitOffsetTypes[type] )
      {
         if ( uniform )
         {
            log_debug( "before uniform: %04X\n", list[n].offsetType );
            list[n].offsetType |= MisfitFilter::OFFSET_UNI_MASK;
            log_debug( "after uniform: %04X\n", list[n].offsetType );
         }
         else
         {
            log_debug( "before variable: %04X\n", list[n].offsetType );
            list[n].offsetType &= MisfitFilter::OFFSET_TYPE_MASK;
            log_debug( "after variable: %04X\n", list[n].offsetType );
         }
         break;
      }
   }
}

bool _offsetIncluded( MisfitDataTypesE type, MisfitOffsetList & list )
{
   unsigned count = list.size();
   for ( unsigned n = 0; n < count; n++ )
   {
      if ( (list[n].offsetType & MisfitFilter::OFFSET_TYPE_MASK) == _misfitOffsetTypes[type] )
      {
         return true;
      }
   }
   return false;
}

unsigned _offsetGet( MisfitDataTypesE type, MisfitOffsetList & list )
{
   unsigned count = list.size();
   for ( unsigned n = 0; n < count; n++ )
   {
      if ( (list[n].offsetType & MisfitFilter::OFFSET_TYPE_MASK) == _misfitOffsetTypes[type] )
      {
         return list[n].offsetValue;
      }
   }
   return 0;
}

bool _offsetIsVariable( MisfitDataTypesE type, MisfitOffsetList & list )
{
   unsigned count = list.size();
   for ( unsigned n = 0; n < count; n++ )
   {
      if ( (list[n].offsetType & MisfitFilter::OFFSET_TYPE_MASK) == _misfitOffsetTypes[type] )
      {
         return ((list[n].offsetType & MisfitFilter::OFFSET_UNI_MASK) == 0);
      }
   }
   return false;
}

MisfitFilter::MisfitFilter()
{
}

MisfitFilter::~MisfitFilter()
{
}

Model::ModelErrorE MisfitFilter::readFile( Model * model, const char * const filename )
{
   if ( filename == NULL || filename[0] == '\0' )
   {
      return Model::ERROR_BAD_ARGUMENT;
   }

   if ( sizeof( float32_t ) != 4 )
   {
      msg_error( transll( QT_TRANSLATE_NOOP( "LowLevel", "MM3D encountered an unexpected data size problem\nSee Help->About to contact the developers" )).c_str() );
      return Model::ERROR_FILE_OPEN;
   }

   m_fp = fopen( filename, "rb" );

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

   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   model->setFilename( modelFullName.c_str() );

   fseek( m_fp, 0, SEEK_END );
   unsigned fileLength = ftell( m_fp );
   m_readLength = fileLength;
   fseek( m_fp, 0, SEEK_SET );

   uint8_t * fileBuf = new uint8_t[fileLength];

   m_bufPos = fileBuf;
   m_readLength = fileLength;

   fread( fileBuf, fileLength, 1, m_fp );
   fclose( m_fp );
   m_fp = NULL;

   MM3DFILE_HeaderT fileHeader;

   readBytes( fileHeader.magic, sizeof(fileHeader.magic) );

   read( fileHeader.versionMajor );
   read( fileHeader.versionMinor );
   read( fileHeader.modelFlags );
   read( fileHeader.offsetCount );

   if ( strncmp( fileHeader.magic, MAGIC, strlen(MAGIC) ) != 0 )
   {
      delete[] fileBuf;
      log_warning( "bad magic number file\n" );
      return Model::ERROR_BAD_MAGIC;
   }

   if ( fileHeader.versionMajor != WRITE_VERSION_MAJOR )
   {
      delete[] fileBuf;
      return Model::ERROR_UNSUPPORTED_VERSION;
   }

   unsigned offsetCount = fileHeader.offsetCount;

   log_debug( "Misfit file:\n" );
   log_debug( "  major:   %d\n", fileHeader.versionMajor );
   log_debug( "  minor:   %d\n", fileHeader.versionMinor );
   log_debug( "  offsets: %d\n", fileHeader.offsetCount );

   log_debug( "Offset information:\n" );

   unsigned t;

   MisfitOffsetList offsetList;

   unsigned lastOffset = 0;
   bool     lastUnknown = false;
   UnknownDataList unknownList;

   for ( t = 0; t < offsetCount; t++ )
   {
      MisfitOffsetT mo;
      read( mo.offsetType );
      read( mo.offsetValue );

      if ( mo.offsetValue < lastOffset )
      {
         log_error( "Offset are out of order\n" );
         delete[] fileBuf;
         return Model::ERROR_BAD_DATA;
      }

      if ( lastUnknown )
      {
         unknownList.back().dataLen = mo.offsetValue - lastOffset;
         log_warning( "unknown data size = %d\n", unknownList.back().dataLen );
         lastUnknown = false;
      }
      lastOffset = mo.offsetValue;

      offsetList.push_back( mo );

      bool found = false;
      for ( unsigned e = 0; !found && e < MDT_MAX; e++ )
      {
         if ( _misfitOffsetTypes[e] == (mo.offsetType & OFFSET_TYPE_MASK) )
         {
            log_debug( "  %08X %s\n", mo.offsetValue, _misfitOffsetNames[e] );
            found = true;

            if ( e == MDT_EndOfFile )
            {
               if ( mo.offsetValue != fileLength )
               {
                  if ( mo.offsetValue > fileLength )
                  {
                     return Model::ERROR_UNEXPECTED_EOF;
                  }
                  else
                  {
                     log_warning( "EOF offset and file size do not match (%d and %d)\n", mo.offsetValue, fileLength );
                     return Model::ERROR_BAD_DATA;
                  }
               }
            }
         }
      }

      if ( !found )
      {
         log_debug( "  %08X Unknown type %04X\n", mo.offsetValue, mo.offsetType );

         lastUnknown = true;
         UnknownDataT ud;
         ud.offsetType = mo.offsetType;
         ud.offsetValue = mo.offsetValue;
         ud.dataLen = 0;

         unknownList.push_back( ud );
      }
   }

   vector<Model::Vertex *>   & modelVertices  = getVertexList( model );
   vector<Model::Triangle *> & modelTriangles = getTriangleList( model );
   vector<Model::Group *>    & modelGroups    = getGroupList( model );
   vector<Model::Material *> & modelMaterials = getMaterialList( model );
   vector<Model::Joint *>    & modelJoints    = getJointList( model );

   // Used to track whether indices are valid
   bool missingElements = false;

   // Meta data
   if ( _offsetIncluded( MDT_Meta, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_Meta, offsetList );
      uint32_t offset   = _offsetGet( MDT_Meta, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned m = 0; m < count; m++ )
      {
         if ( variable )
         {
            read( size );
         }

         unsigned len;
         char key[1024];
         char value[1024];

         strncpy( key, (char *) m_bufPos, sizeof(key) );
         utf8chrtrunc( key, sizeof(key)-1 );

         len = strlen(key) + 1;
         m_bufPos     += len;
         m_readLength -= len;

         strncpy( value, (char *) m_bufPos, sizeof(value) );
         utf8chrtrunc( value, sizeof(value)-1 );

         len = strlen(value) + 1;
         m_bufPos     += len;
         m_readLength -= len;

         model->addMetaData( key, value );
      }
   }

   // Vertices
   if ( _offsetIncluded( MDT_Vertices, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_Vertices, offsetList );
      uint32_t offset   = _offsetGet( MDT_Vertices, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned v = 0; v < count; v++ )
      {
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_VertexT fileVert;

         read( fileVert.flags );
         read( fileVert.coord[0] );
         read( fileVert.coord[1] );
         read( fileVert.coord[2] );

         Model::Vertex    * vert = Model::Vertex::get();

         //vert->m_boneId = -1;
         vert->m_coord[0] = fileVert.coord[0];
         vert->m_coord[1] = fileVert.coord[1];
         vert->m_coord[2] = fileVert.coord[2];

         uint16_t vertFlags = fileVert.flags;
         vert->m_selected = ((vertFlags & MF_SELECTED) == MF_SELECTED);
         vert->m_visible  = ((vertFlags & MF_HIDDEN)   != MF_HIDDEN);
         vert->m_free     = ((vertFlags & MF_VERTFREE) == MF_VERTFREE);

         modelVertices.push_back( vert );
      }
   }

   // Triangles
   if ( _offsetIncluded( MDT_Triangles, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_Triangles, offsetList );
      uint32_t offset   = _offsetGet( MDT_Triangles, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned t = 0; t < count; t++ )
      {
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_TriangleT fileTri;
         read( fileTri.flags );
         read( fileTri.vertex[0] );
         read( fileTri.vertex[1] );
         read( fileTri.vertex[2] );

         Model::Triangle    * tri = Model::Triangle::get();

         tri->m_vertexIndices[0] = fileTri.vertex[0];
         tri->m_vertexIndices[1] = fileTri.vertex[1];
         tri->m_vertexIndices[2] = fileTri.vertex[2];
         uint16_t triFlags = fileTri.flags;
         tri->m_selected = ((triFlags & MF_SELECTED) == MF_SELECTED);
         tri->m_visible  = ((triFlags & MF_HIDDEN) != MF_HIDDEN);
         tri->m_projection = -1;

         modelTriangles.push_back( tri );
      }
   }

#if 0
   // Triangle Normals
   if ( _offsetIncluded( MDT_TriangleNormals, offsetList ) )
   {
      // Just for debugging... we don't actually use any of this

      bool     variable = _offsetIsVariable( MDT_TriangleNormals, offsetList );
      uint32_t offset   = _offsetGet( MDT_TriangleNormals, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned t = 0; t < count; t++ )
      {
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_TriangleNormalsT fileTri;
         read( fileTri.flags );
         read( fileTri.index );
         read( fileTri.normal[0][0] );
         read( fileTri.normal[0][1] );
         read( fileTri.normal[0][2] );
         read( fileTri.normal[1][0] );
         read( fileTri.normal[1][1] );
         read( fileTri.normal[1][2] );
         read( fileTri.normal[2][0] );
         read( fileTri.normal[2][1] );
         read( fileTri.normal[2][2] );

         log_debug( "triangle %d normals:\n", fileTri.index );

         for ( unsigned v = 0; v < 3; v++ )
         {
            log_debug( "  v %d:  %f %f %f\n", v, 
                  fileTri.normal[v][0],
                  fileTri.normal[v][1],
                  fileTri.normal[v][2] );
         }
      }
   }
#endif // 0

   // Groups
   if ( _offsetIncluded( MDT_Groups, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_Groups, offsetList );
      uint32_t offset   = _offsetGet( MDT_Groups, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned g = 0; g < count; g++ )
      {
         if ( variable )
         {
            read( size );
         }

         Model::Group    * grp = Model::Group::get();

         uint16_t flags;
         uint32_t triCount;
         uint8_t  smoothness;
         uint32_t materialIndex;
         char name[1024];

         read( flags );

         strncpy( name, (char *) m_bufPos, sizeof(name));
         utf8chrtrunc( name, sizeof(name)-1 );

         unsigned nameSize = strlen(name) + 1;
         m_bufPos     += nameSize;
         m_readLength -= nameSize;

         read( triCount );

         for ( unsigned t = 0; t < triCount; t++ )
         {
            uint32_t triIndex = 0;
            read( triIndex );
            grp->m_triangleIndices.push_back( triIndex );
         }

         read( smoothness );
         read( materialIndex );

         grp->m_name = name;
         grp->m_smooth = smoothness;
         grp->m_selected = ((flags & MF_SELECTED) == MF_SELECTED);
         grp->m_visible  = ((flags & MF_HIDDEN) != MF_HIDDEN);
         grp->m_materialIndex = materialIndex;

         modelGroups.push_back( grp );
      }
   }

   // External Textures
   std::vector< std::string > texNames;
   if ( _offsetIncluded( MDT_ExtTextures, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_ExtTextures, offsetList );
      uint32_t offset   = _offsetGet( MDT_ExtTextures, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned t = 0; t < count; t++ )
      {
         log_debug( "reading external texture %d/%d\n", t, count );
         if ( variable )
         {
            read( size );
         }

         uint16_t flags;
         char filename[PATH_MAX];

         read( flags );

         strncpy( filename, (char *) m_bufPos, sizeof(filename));
         utf8chrtrunc( filename, sizeof(filename)-1 );

         unsigned nameSize = strlen(filename) + 1;
         m_bufPos     += nameSize;
         m_readLength -= nameSize;
         replaceBackslash( filename );
         log_debug( "  filename is %s\n", filename );

         std::string fullpath = getAbsolutePath( modelPath.c_str(), filename );

         texNames.push_back( fullpath );
      }
   }

   // Materials
   if ( _offsetIncluded( MDT_Materials, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_Materials, offsetList );
      uint32_t offset   = _offsetGet( MDT_Materials, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned m = 0; m < count; m++ )
      {
         log_debug( "reading material %d/%d\n", m, count );
         if ( variable )
         {
            read( size );
         }

         uint16_t flags = 0;
         uint32_t texIndex = 0;
         char name[1024];

         Model::Material * mat = Model::Material::get();

         read( flags );
         read( texIndex );

         strncpy( name, (char *) m_bufPos, sizeof(name));
         utf8chrtrunc( name, sizeof(name)-1 );

         unsigned nameSize = strlen(name) + 1;
         m_bufPos     += nameSize;
         m_readLength -= nameSize;

         log_debug( "  material name: %s\n", name );

         mat->m_name = name;
         switch ( flags & 0x0f )
         {
            case 0:
               log_debug( "  got external texture %d\n", texIndex );
               mat->m_type = Model::Material::MATTYPE_TEXTURE;
               if ( texIndex < texNames.size() )
               {
                  mat->m_filename = texNames[texIndex];
               }
               else
               {
                  mat->m_filename = "";
               }
               break;
            case 13:
               mat->m_type = Model::Material::MATTYPE_COLOR;
               mat->m_filename = "";
               memset( mat->m_color, 255, sizeof( mat->m_color ) );
               break;
            case 14:
               mat->m_type = Model::Material::MATTYPE_GRADIENT;
               mat->m_filename = "";
               memset( mat->m_color, 255, sizeof( mat->m_color ) );
               break;
            case 15:
               mat->m_type = Model::Material::MATTYPE_BLANK;
               mat->m_filename = "";
               memset( mat->m_color, 255, sizeof( mat->m_color ) );
               break;
            default:
               log_debug( "  got unknown material type\n", texIndex );
               mat->m_type = Model::Material::MATTYPE_BLANK;
               mat->m_filename = "";
               memset( mat->m_color, 255, sizeof( mat->m_color ) );
               break;
         }

         mat->m_sClamp = ( (flags & MF_MAT_CLAMP_S) != 0 );
         mat->m_tClamp = ( (flags & MF_MAT_CLAMP_T) != 0 );

         unsigned i = 0;
         float32_t lightProp = 0;
         for ( i = 0; i < 4; i++ )
         {
            read( lightProp );
            mat->m_ambient[i] = lightProp;
         }
         for ( i = 0; i < 4; i++ )
         {
            read( lightProp );
            mat->m_diffuse[i] = lightProp;
         }
         for ( i = 0; i < 4; i++ )
         {
            read( lightProp );
            mat->m_specular[i] = lightProp;
         }
         for ( i = 0; i < 4; i++ )
         {
            read( lightProp );
            mat->m_emissive[i] = lightProp;
         }
         read( lightProp );
         mat->m_shininess = lightProp;

         modelMaterials.push_back( mat );
      }
   }

   // Texture coordinates
   if ( _offsetIncluded( MDT_TexCoords, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_TexCoords, offsetList );
      uint32_t offset   = _offsetGet( MDT_TexCoords, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned c = 0; c < count; c++ )
      {
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_TexCoordT tc;
         read( tc.flags );
         read( tc.triangleIndex );
         read( tc.sCoord[0] );
         read( tc.sCoord[1] );
         read( tc.sCoord[2] );
         read( tc.tCoord[0] );
         read( tc.tCoord[1] );
         read( tc.tCoord[2] );

         uint32_t triIndex = tc.triangleIndex;

         if ( triIndex < modelTriangles.size() )
         {
            for ( unsigned v = 0; v < 3; v++ )
            {
               modelTriangles[triIndex]->m_s[v] = tc.sCoord[v];
               modelTriangles[triIndex]->m_t[v] = tc.tCoord[v];
            }
         }
      }
   }

   // Canvas Background Images
   if ( _offsetIncluded( MDT_CanvasBackgrounds, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_CanvasBackgrounds, offsetList );
      uint32_t offset   = _offsetGet( MDT_CanvasBackgrounds, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned g = 0; g < count; g++ )
      {
         log_debug( "reading canvas background %d/%d\n", g, count );
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_CanvasBackgroundT cb;
         read( cb.flags );
         read( cb.viewIndex );
         read( cb.scale );
         read( cb.center[0] );
         read( cb.center[1] );
         read( cb.center[2] );

         char name[PATH_MAX];

         strncpy( name, (char *) m_bufPos, sizeof(name));
         utf8chrtrunc( name, sizeof(name)-1 );

         unsigned nameSize = strlen(name) + 1;
         m_bufPos     += nameSize;
         m_readLength -= nameSize;
         replaceBackslash( name );
         std::string fileStr = getAbsolutePath( modelPath.c_str(), name );

         model->setBackgroundImage( cb.viewIndex, fileStr.c_str() );
         model->setBackgroundScale( cb.viewIndex, cb.scale );
         model->setBackgroundCenter( cb.viewIndex, 
               cb.center[0], cb.center[1], cb.center[2] );
      }
   }

   // Joints
   if ( _offsetIncluded( MDT_Joints, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_Joints, offsetList );
      uint32_t offset   = _offsetGet( MDT_Joints, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned j = 0; j < count; j++ )
      {
         log_debug( "reading joint %d/%d\n", j, count );
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_JointT fileJoint;
         read( fileJoint.flags );
         readBytes( fileJoint.name, sizeof(fileJoint.name) );
         read( fileJoint.parentIndex );
         read( fileJoint.localRot[0] );
         read( fileJoint.localRot[1] );
         read( fileJoint.localRot[2] );
         read( fileJoint.localTrans[0] );
         read( fileJoint.localTrans[1] );
         read( fileJoint.localTrans[2] );

         Model::Joint    * joint = Model::Joint::get();

         fileJoint.name[ sizeof(fileJoint.name) - 1 ] = '\0';

         joint->m_name = fileJoint.name;
         joint->m_parent = fileJoint.parentIndex;
         for ( unsigned i = 0; i < 3; i++ )
         {
            joint->m_localRotation[i]    = fileJoint.localRot[i];
            joint->m_localTranslation[i] = fileJoint.localTrans[i];
         }
         uint16_t jointFlags = fileJoint.flags;
         joint->m_selected = ((jointFlags & MF_SELECTED) == MF_SELECTED);
         joint->m_visible  = ((jointFlags & MF_HIDDEN) != MF_HIDDEN);

         modelJoints.push_back( joint );
      }

      log_debug( "read %d joints\n", count );
   }

   // Joint Vertices

   // Newer versions of the file format use MDT_WeightedInfluences instead.
   // We still want to read this data to use as a default in case the
   // weighted influences are not present.
   if ( _offsetIncluded( MDT_JointVertices, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_JointVertices, offsetList );
      uint32_t offset   = _offsetGet( MDT_JointVertices, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned t = 0; t < count; t++ )
      {
         if ( variable )
         {
            read( size );
         }

         uint32_t vertexIndex = 0;
         int32_t jointIndex   = 0;
         read( vertexIndex );
         read( jointIndex );

         if ( vertexIndex < modelVertices.size() && (unsigned) jointIndex < modelJoints.size() )
         {
            //modelVertices[ vertexIndex ]->m_boneId = jointIndex;
            model->addVertexInfluence( vertexIndex, jointIndex, Model::IT_Custom, 1.0 );
         }
         else
         {
            missingElements = true;
            log_error( "vertex %d or joint %d out of range\n", vertexIndex, jointIndex );
         }
      }

      log_debug( "read %d joints vertices\n", count );
   }

   // Points
   if ( _offsetIncluded( MDT_Points, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_Points, offsetList );
      uint32_t offset   = _offsetGet( MDT_Points, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned j = 0; j < count; j++ )
      {
         log_debug( "reading Point %d/%d\n", j, count );
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_PointT filePoint;
         read( filePoint.flags );
         readBytes( filePoint.name, sizeof(filePoint.name) );
         read( filePoint.type );
         read( filePoint.boneIndex );
         read( filePoint.rot[0] );
         read( filePoint.rot[1] );
         read( filePoint.rot[2] );
         read( filePoint.trans[0] );
         read( filePoint.trans[1] );
         read( filePoint.trans[2] );

         filePoint.name[ sizeof(filePoint.name) - 1 ] = '\0';

         float rot[3];
         float trans[3];
         for ( unsigned i = 0; i < 3; i++ )
         {
            rot[i]   = filePoint.rot[i];
            trans[i] = filePoint.trans[i];
         }

         int boneIndex = filePoint.boneIndex;

         int p = model->addPoint( filePoint.name,
               trans[0], trans[1], trans[2], 
               rot[0], rot[1], rot[2], 
               boneIndex );

         int type = filePoint.type;
         model->setPointType( p, type );

         uint16_t pointFlags = filePoint.flags;
         if ( (pointFlags & MF_SELECTED) == MF_SELECTED )
         {
            model->selectPoint( p );
         }
         if ( (pointFlags & MF_HIDDEN) == MF_HIDDEN)
         {
            model->hidePoint( p );
         }
      }

      log_debug( "read %d points\n" );
   }

   // Smooth Angles
   if ( _offsetIncluded( MDT_SmoothAngles, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_SmoothAngles, offsetList );
      uint32_t offset   = _offsetGet( MDT_SmoothAngles, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned t = 0; t < count; t++ )
      {
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_SmoothAngleT fileSa;
         read( fileSa.groupIndex );
         read( fileSa.angle );

         if ( fileSa.angle > 180 )
         {
            fileSa.angle = 180;
         }
         if ( fileSa.groupIndex < modelGroups.size() )
         {
            model->setGroupAngle( fileSa.groupIndex, fileSa.angle );
         }
      }

      log_debug( "read %d group smoothness angles\n", count );
   }

   // Weighted influences
   if ( _offsetIncluded( MDT_WeightedInfluences, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_WeightedInfluences, offsetList );
      uint32_t offset   = _offsetGet( MDT_WeightedInfluences, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      unsigned vcount = model->getVertexCount();
      for ( unsigned v = 0; v < vcount; v++ )
      {
         model->removeAllVertexInfluences( v );
      }

      unsigned pcount = model->getVertexCount();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         model->removeAllPointInfluences( p );
      }

      for ( unsigned t = 0; t < count; t++ )
      {
         if ( variable )
         {
            read( size );
         }

         MM3DFILE_WeightedInfluenceT fileWi;
         read( fileWi.posType );
         read( fileWi.posIndex );
         read( fileWi.infIndex );
         read( fileWi.infType );
         read( fileWi.infWeight );

         if ( fileWi.posType == Model::PT_Vertex
               || fileWi.posType == Model::PT_Point )
         {
            Model::Position pos;
            pos.type = static_cast<Model::PositionTypeE>( fileWi.posType );
            pos.index = fileWi.posIndex;

            Model::InfluenceTypeE type = Model::IT_Custom;

            switch ( fileWi.infType )
            {
               case Model::IT_Custom:
               case Model::IT_Auto:
               case Model::IT_Remainder:
                  type = static_cast<Model::InfluenceTypeE>( fileWi.infType );
                  break;
               default:
                  log_error( "unknown influence type %d\n", fileWi.infType );
                  break;
            }

            double weight = ((double) fileWi.infWeight) / 100.0;

            log_debug( "adding position influence %d,%d,%f\n",
                  pos.index, (int) type, (float) weight );
            model->addPositionInfluence( pos, 
                  fileWi.infIndex, type, weight );
         }
      }
   }

   // Texture Projections
   if ( _offsetIncluded( MDT_TexProjections, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_TexProjections, offsetList );
      uint32_t offset   = _offsetGet( MDT_TexProjections, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned j = 0; j < count; j++ )
      {
         log_debug( "reading Projection %d/%d\n", j, count );
         if ( variable )
         {
            read( size );
         }

         uint16_t flags = 0;
         read( flags );

         std::string name;
         name.assign( (char *) m_bufPos, 40 );
         m_bufPos += 40;
         m_readLength -= 40;

         int32_t type = 0;
         read( type );

         double vec[3];
         float  fvec[3];

         unsigned i = 0;

         // position
         for ( unsigned i = 0; i < 3; i++ )
         {
            read( fvec[i] );
            vec[i] = fvec[i];
         }

         int proj = model->addProjection( name.c_str(), type, 
               vec[0], vec[1], vec[2] );

         // up vector
         for ( unsigned i = 0; i < 3; i++ )
         {
            read( fvec[i] );
            vec[i] = fvec[i];
         }

         model->setProjectionUp( proj, vec );

         // seam vector
         for ( unsigned i = 0; i < 3; i++ )
         {
            read( fvec[i] );
            vec[i] = fvec[i];
         }

         model->setProjectionSeam( proj, vec );

         double uv[2][2];
         float  fuv[2][2];

         // texture coordinate range
         for ( i = 0; i < 2; i++ )
         {
            for ( int j = 0; j < 2; j++ )
            {
               read( fuv[i][j] );
               uv[i][j] = fuv[i][j];
            }
         }

         model->setProjectionRange( proj,
               uv[0][0], uv[0][1], uv[1][0], uv[1][1] );
      }

      log_debug( "read %d projections\n" );
   }

   // Texture Projection Triangles (have to read this after projections)
   if ( _offsetIncluded( MDT_ProjectionTriangles, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_ProjectionTriangles, offsetList );
      uint32_t offset   = _offsetGet( MDT_ProjectionTriangles, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;

      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned c = 0; c < count; c++ )
      {
         if ( variable )
         {
            read( size );
         }

         uint32_t proj = 0;
         uint32_t triCount = 0;
         uint32_t tri = 0;

         read( proj );
         read( triCount );

         uint32_t t;
         for ( t = 0; t < triCount; t++ )
         {
            read( tri );
            model->setTriangleProjection( tri, proj );
         }
      }
   }

   // Skeletal Animations
   if ( _offsetIncluded( MDT_SkelAnims, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_SkelAnims, offsetList );
      uint32_t offset   = _offsetGet( MDT_SkelAnims, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned s = 0; s < count; s++ )
      {
         log_debug( "reading skel anim %d/%d\n", s, count );
         if ( variable )
         {
            read( size );
         }

         uint16_t  flags;
         char      name[1024];
         float32_t fps;
         uint32_t  frameCount;

         read( flags );

         strncpy( name, (char *) m_bufPos, sizeof(name));
         utf8chrtrunc( name, sizeof(name)-1 );

         unsigned nameSize = strlen(name) + 1;
         m_bufPos     += nameSize;
         m_readLength -= nameSize;
         log_debug( "  name is %s\n", name );

         read( fps );
         read( frameCount );

         unsigned anim = model->addAnimation( Model::ANIMMODE_SKELETAL, name );
         model->setAnimFPS( Model::ANIMMODE_SKELETAL, anim, fps );
         model->setAnimFrameCount( Model::ANIMMODE_SKELETAL, anim, frameCount );

         for ( unsigned f = 0; f < frameCount; f++ )
         {
            uint32_t keyframeCount;
            read( keyframeCount );

            for ( unsigned k = 0; k < keyframeCount; k++ )
            {
               MM3DFILE_SkelKeyframeT fileKf;
               read( fileKf.jointIndex );
               read( fileKf.keyframeType );
               read( fileKf.param[0] );
               read( fileKf.param[1] );
               read( fileKf.param[2] );

               model->setSkelAnimKeyframe( anim, f, 
                     fileKf.jointIndex, (fileKf.keyframeType ? false : true), 
                     fileKf.param[0], fileKf.param[1], fileKf.param[2] );
            }
         }
      }
   }

   // Frame Animations
   if ( _offsetIncluded( MDT_FrameAnims, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_FrameAnims, offsetList );
      uint32_t offset   = _offsetGet( MDT_FrameAnims, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned a = 0; a < count; a++ )
      {
         log_debug( "reading frame animation %d/%d\n", a, count );

         if ( variable )
         {
            read( size );
         }

         if ( size > m_readLength )
         {
            log_error( "Size of frame animation is too large for file data\n" );
            delete[] fileBuf;
            return Model::ERROR_BAD_DATA;
         }

         uint16_t  flags;
         char      name[1024];
         float32_t fps;
         uint32_t  frameCount;

         read( flags );

         strncpy( name, (char *) m_bufPos, sizeof(name));
         utf8chrtrunc( name, sizeof(name)-1 );
         log_debug( "anim name '%s' size %d\n", name, size );

         unsigned nameSize = strlen(name) + 1;
         m_bufPos     += nameSize;
         m_readLength -= nameSize;

         read( fps );
         log_debug( "fps %f\n", fps );
         read( frameCount );
         log_debug( "frame count %u\n", frameCount );

         if ( (frameCount > size) ||
                ((frameCount * modelVertices.size() * sizeof(float32_t)) * 3 >
                    (size - 10)))
         {
            log_error( "Frame count for animation is too large for file data\n" );
            delete[] fileBuf;
            return Model::ERROR_BAD_DATA;
         }

         unsigned anim = model->addAnimation( Model::ANIMMODE_FRAME, name );
         model->setAnimFPS( Model::ANIMMODE_FRAME, anim, fps );
         model->setAnimFrameCount( Model::ANIMMODE_FRAME, anim, frameCount );

         for ( unsigned f = 0; f < frameCount; f++ )
         {
            unsigned maxVertex = 0;
            float32_t coord[3];
            for ( unsigned v = 0; v < modelVertices.size(); v++ )
            {
               for ( unsigned i = 0; i < 3; i++ )
               {
                  read( coord[i] );
               }
               model->setFrameAnimVertexCoords( anim, f, v,
                     coord[0], coord[1], coord[2] );
               maxVertex = v;
            }

            maxVertex = maxVertex + 1;
            if ( (maxVertex) != modelVertices.size() )
            {
               missingElements = true;
               log_error( "Vertex count for frame animation %d, frame %d has %d vertices, should be %d\n", anim, f, maxVertex, modelVertices.size() );
            }
         }
      }
   }

   // Frame Animation Points
   if ( _offsetIncluded( MDT_FrameAnimPoints, offsetList ) )
   {
      bool     variable = _offsetIsVariable( MDT_FrameAnimPoints, offsetList );
      uint32_t offset   = _offsetGet( MDT_FrameAnimPoints, offsetList );

      m_bufPos     = &fileBuf[offset];
      m_readLength = fileLength - offset;

      uint16_t flags = 0;
      uint32_t count = 0;
      read( flags );
      read( count );

      uint32_t size = 0;
      if ( !variable )
      {
         read( size );
      }

      for ( unsigned a = 0; a < count; a++ )
      {
         if ( variable )
         {
            read( size );
         }

         uint16_t  flags;
         uint32_t  anim;
         uint32_t  frameCount;

         read( flags );

         read( anim );
         read( frameCount );

         size_t pcount = model->getPointCount();

         for ( unsigned f = 0; f < frameCount; f++ )
         {
            float32_t rot[3];
            float32_t trans[3];
            for ( unsigned p = 0; p < pcount; p++ )
            {
               for ( unsigned i = 0; i < 3; i++ )
               {
                  read( rot[i] );
               }
               for ( unsigned i = 0; i < 3; i++ )
               {
                  read( trans[i] );
               }
               model->setFrameAnimPointCoords( anim, f, p,
                     trans[0], trans[1], trans[2] );
               model->setFrameAnimPointRotation( anim, f, p,
                     rot[0], rot[1], rot[2] );
            }
         }
      }
   }

   // Read unknown data
   UnknownDataList::iterator it;
   for ( it = unknownList.begin(); it != unknownList.end(); it++ )
   {
      Model::FormatData * fd = new Model::FormatData;
      fd->offsetType = (*it).offsetType;
      m_bufPos = &fileBuf[ (*it).offsetValue ];

      log_debug( "unknown data is type %x...\n", fd->offsetType );

      fd->data = new uint8_t[ (*it).dataLen ];
      fd->len  = (*it).dataLen;
      memcpy( fd->data, m_bufPos, (*it).dataLen );

      if ( model->addFormatData( fd ) < 0 )
      {
         delete fd;
      }
   }

   // Account for missing elements (vertices, triangles, textures, joints)
   {
      unsigned vcount = modelVertices.size();
      unsigned tcount = modelTriangles.size();
      unsigned gcount = modelGroups.size();
      unsigned jcount = modelJoints.size();
      unsigned mcount = modelMaterials.size();

//      for ( unsigned v = 0; v < vcount; v++ )
//      {
//         if ( modelVertices[v]->m_boneId >= (signed) jcount )
//         {
//            missingElements = true;
//            log_error( "Vertex %d uses missing bone joint %d\n", v, modelVertices[v]->m_boneId );
//         }
//      }

      for ( unsigned t = 0; t < tcount; t++ )
      {
         for ( unsigned i = 0; i < 3; i++ )
         {
            if ( modelTriangles[t]->m_vertexIndices[i] >= vcount )
            {
               missingElements = true;
               log_error( "Triangle %d uses missing vertex %d\n", t, modelTriangles[t]->m_vertexIndices[i] );
            }
         }
      }

      for ( unsigned g = 0; g < gcount; g++ )
      {
         unsigned count = modelGroups[g]->m_triangleIndices.size();
         for ( unsigned i = 0; i < count; i++ )
         {
            if ( modelGroups[g]->m_triangleIndices[i] >= (signed) tcount )
            {
               missingElements = true;
               log_error( "Group %d uses missing triangle %d\n", g, modelGroups[g]->m_triangleIndices[i] );
            }
         }

         if ( modelGroups[g]->m_materialIndex >= (signed) mcount )
         {
            missingElements = true;
            log_error( "Group %d uses missing texture %d\n", g, modelGroups[g]->m_materialIndex );
         }
      }

      for ( unsigned j = 0; j < jcount; j++ )
      {
         if ( modelJoints[j]->m_parent >= (signed) jcount )
         {
            log_warning( "Joint %d has bad parent joint, checking endianness\n", j );
            if ( htonl( modelJoints[j]->m_parent ) < jcount )
            {
               modelJoints[j]->m_parent = htonl( modelJoints[j]->m_parent );
            }
            else if ( htol_u32( modelJoints[j]->m_parent ) < jcount )
            {
               modelJoints[j]->m_parent = htol_u32( modelJoints[j]->m_parent );
            }
            else
            {
               missingElements = true;
               log_error( "Joint %d has missing parent joint %d\n", j, modelJoints[j]->m_parent );
            }
         }
      }
   }

   delete[] fileBuf;

   if ( missingElements )
   {
      log_warning( "missing elements in file\n" );
      return Model::ERROR_BAD_DATA;
   }
   else
   {
      model->setupJoints();
      return Model::ERROR_NONE;
   }
}

Model::ModelErrorE MisfitFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o  )
{
   if ( filename == NULL || filename[0] == '\0' )
   {
      return Model::ERROR_BAD_ARGUMENT;
   }

   if ( sizeof( float32_t ) != 4 )
   {
      msg_error( transll( QT_TRANSLATE_NOOP( "LowLevel", "MM3D encountered an unexpected data size problem\nSee Help->About to contact the developers" )).c_str() );
      return Model::ERROR_FILE_OPEN;
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

   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   bool doWrite[ MDT_MAX ];
   unsigned t = 0;

   for ( t = 0; t < MDT_MAX; t++ )
   {
      doWrite[t] = false;
   }

   // Get model data

   vector<Model::Vertex *>      & modelVertices     = getVertexList( model );
   vector<Model::Triangle *>    & modelTriangles    = getTriangleList( model );
   vector<Model::Group *>       & modelGroups       = getGroupList( model );
   vector<Model::Material *>    & modelMaterials    = getMaterialList( model );
   vector<Model::Joint *>       & modelJoints       = getJointList( model );
   vector<Model::Point *>       & modelPoints       = getPointList( model );
   //vector<Model::TextureProjection *> & modelProjections = getProjectionList( model );
   vector<Model::SkelAnim *>    & modelSkels        = getSkelList( model );
   vector<Model::FrameAnim *>   & modelFrames       = getFrameList( model );

   int backgrounds = 0;
   for ( t = 0; t < 6; t++ )
   {
      const char * file =  model->getBackgroundImage( t );
      if ( file[0] != '\0' )
      {
         backgrounds++;
      }
   }

   bool haveProjectionTriangles = false;
   if ( model->getProjectionCount() > 0 )
   {
      size_t tcount = model->getTriangleCount();
      for ( size_t t = 0; !haveProjectionTriangles && t < tcount; t++ )
      {
         if ( model->getTriangleProjection( t ) >= 0 )
         {
            haveProjectionTriangles = true;
         }
      }
   }

   // Find out what sections we need to write
   doWrite[ MDT_Meta ]                = (model->getMetaDataCount() > 0);
   doWrite[ MDT_Vertices ]            = (modelVertices.size() > 0);
   doWrite[ MDT_Triangles ]           = (modelTriangles.size() > 0);
   doWrite[ MDT_TriangleNormals ]     = (modelTriangles.size() > 0);
   doWrite[ MDT_Groups ]              = (modelGroups.size() > 0);
   doWrite[ MDT_Materials ]           = (modelMaterials.size() > 0);
   doWrite[ MDT_ExtTextures ]         =  doWrite[ MDT_Materials ];
   doWrite[ MDT_TexCoords ]           = true; // Some users map texture coordinates before assigning a texture (think: paint texture)
   doWrite[ MDT_ProjectionTriangles ] = haveProjectionTriangles;
   doWrite[ MDT_CanvasBackgrounds ]   = (backgrounds > 0);
   doWrite[ MDT_Joints ]              = (modelJoints.size() > 0);
   doWrite[ MDT_JointVertices ]       =  doWrite[ MDT_Joints ];
   doWrite[ MDT_Points ]              = (model->getPointCount() > 0);
   doWrite[ MDT_SmoothAngles ]        = (modelGroups.size() > 0);
   doWrite[ MDT_WeightedInfluences ]  = (modelJoints.size() > 0);
   doWrite[ MDT_TexProjections ]      = (model->getProjectionCount() > 0);
   doWrite[ MDT_SkelAnims ]           = (model->getAnimCount( Model::ANIMMODE_SKELETAL ) > 0);
   doWrite[ MDT_FrameAnims ]          = (model->getAnimCount( Model::ANIMMODE_FRAME ) > 0);
   doWrite[ MDT_FrameAnimPoints ]     = (model->getAnimCount( Model::ANIMMODE_FRAME ) > 0 && model->getPointCount() > 0 );
   doWrite[ MDT_RelativeAnims ]       = (model->getAnimCount( Model::ANIMMODE_FRAMERELATIVE ) > 0);

   uint8_t modelFlags = 0x00;

   // Write header
   MisfitOffsetList offsetList;

   _addOffset( MDT_Meta,                doWrite[ MDT_Meta ],                offsetList );
   _addOffset( MDT_Vertices,            doWrite[ MDT_Vertices ],            offsetList );
   _addOffset( MDT_Triangles,           doWrite[ MDT_Triangles ],           offsetList );
   _addOffset( MDT_TriangleNormals,     doWrite[ MDT_TriangleNormals ],     offsetList );
   _addOffset( MDT_Groups,              doWrite[ MDT_Groups ],              offsetList );
   _addOffset( MDT_Materials,           doWrite[ MDT_Materials ],           offsetList );
   _addOffset( MDT_ExtTextures,         doWrite[ MDT_ExtTextures ],         offsetList );
   _addOffset( MDT_TexCoords,           doWrite[ MDT_TexCoords ],           offsetList );
   _addOffset( MDT_ProjectionTriangles, doWrite[ MDT_ProjectionTriangles ], offsetList );
   _addOffset( MDT_CanvasBackgrounds,   doWrite[ MDT_CanvasBackgrounds ],   offsetList );
   _addOffset( MDT_Joints,              doWrite[ MDT_Joints ],              offsetList );
   _addOffset( MDT_JointVertices,       doWrite[ MDT_JointVertices ],       offsetList );
   _addOffset( MDT_Points,              doWrite[ MDT_Points ],              offsetList );
   _addOffset( MDT_SmoothAngles,        doWrite[ MDT_SmoothAngles ],        offsetList );
   _addOffset( MDT_WeightedInfluences,  doWrite[ MDT_WeightedInfluences ],  offsetList );
   _addOffset( MDT_TexProjections,      doWrite[ MDT_TexProjections ],      offsetList );
   _addOffset( MDT_SkelAnims,           doWrite[ MDT_SkelAnims ],           offsetList );
   _addOffset( MDT_FrameAnims,          doWrite[ MDT_FrameAnims ],          offsetList );
   _addOffset( MDT_FrameAnimPoints,     doWrite[ MDT_FrameAnimPoints ],     offsetList );

   unsigned formatDataCount = model->getFormatDataCount();
   unsigned f = 0;

   for ( f = 0; f < formatDataCount; f++ )
   {
      Model::FormatData * fd = model->getFormatData( f );
      if ( fd->offsetType != 0 )
      {
         MisfitOffsetT mo;
         mo.offsetType = (fd->offsetType | OFFSET_DIRTY_MASK);
         mo.offsetValue = 0;
         offsetList.push_back( mo );
         log_warning( "adding uknown data type %04x\n", mo.offsetType );
      }
   }

   _addOffset( MDT_EndOfFile, true, offsetList );

   uint8_t offsetCount = (uint8_t) offsetList.size();

   writeBytes( MAGIC, strlen(MAGIC) );
   write( WRITE_VERSION_MAJOR );
   write( WRITE_VERSION_MINOR );
   write( modelFlags );
   write( offsetCount );

   for ( t = 0; t < offsetCount; t++ )
   {
      MisfitOffsetT & mo = offsetList[t];
      write( mo.offsetType );
      write( mo.offsetValue );
   }
   log_debug( "wrote %d offsets\n", offsetCount );

   // Write data

   // Meta data
   if ( doWrite[ MDT_Meta ] )
   {
      _setOffset( MDT_Meta, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_Meta, false, offsetList );

      unsigned count = model->getMetaDataCount();

      writeHeaderA( 0x0000, count );

      for ( unsigned m = 0; m < count; m++ )
      {
         char key[1024];
         char value[1024];

         model->getMetaData( m, key, sizeof(key), value, sizeof(value) );

         unsigned keyLen = strlen( key ) + 1;
         unsigned valueLen = strlen( value ) + 1;

         write(keyLen + valueLen);

         writeBytes( key, keyLen );
         writeBytes( value, valueLen );
      }
      log_debug( "wrote %d meta data pairs\n", count );
   }

   // Vertices
   if ( doWrite[ MDT_Vertices ] )
   {
      _setOffset( MDT_Vertices, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_Vertices, true, offsetList );

      unsigned count = modelVertices.size();

      writeHeaderB( 0x0000, count, FILE_VERTEX_SIZE );

      for ( unsigned v = 0; v < count; v++ )
      {
         MM3DFILE_VertexT fileVertex;

         fileVertex.flags  = 0x0000;
         fileVertex.flags |= (modelVertices[v]->m_visible)  ? 0 : MF_HIDDEN;
         fileVertex.flags |= (modelVertices[v]->m_selected) ? MF_SELECTED : 0;
         fileVertex.flags |= (modelVertices[v]->m_free)     ? MF_VERTFREE : 0;

         fileVertex.coord[0] = modelVertices[v]->m_coord[0];
         fileVertex.coord[1] = modelVertices[v]->m_coord[1];
         fileVertex.coord[2] = modelVertices[v]->m_coord[2];

         write( fileVertex.flags );
         write( fileVertex.coord[0] );
         write( fileVertex.coord[1] );
         write( fileVertex.coord[2] );
      }
      log_debug( "wrote %d vertices\n", count );
   }

   // Triangles
   if ( doWrite[ MDT_Triangles ] )
   {
      _setOffset( MDT_Triangles, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_Triangles, true, offsetList );

      unsigned count = modelTriangles.size();

      writeHeaderB( 0x0000, count, FILE_TRIANGLE_SIZE );

      for ( unsigned t = 0; t < count; t++ )
      {
         MM3DFILE_TriangleT fileTriangle;

         fileTriangle.flags = 0x0000;
         fileTriangle.flags |= (modelTriangles[t]->m_visible)  ? 0 : MF_HIDDEN;
         fileTriangle.flags |= (modelTriangles[t]->m_selected) ? MF_SELECTED : 0;
         fileTriangle.flags = fileTriangle.flags;

         fileTriangle.vertex[0] = modelTriangles[t]->m_vertexIndices[0];
         fileTriangle.vertex[1] = modelTriangles[t]->m_vertexIndices[1];
         fileTriangle.vertex[2] = modelTriangles[t]->m_vertexIndices[2];

         write( fileTriangle.flags );
         write( fileTriangle.vertex[0] );
         write( fileTriangle.vertex[1] );
         write( fileTriangle.vertex[2] );
      }
      log_debug( "wrote %d triangles\n", count );
   }

   // Triangle Normals
   if ( doWrite[ MDT_TriangleNormals ] )
   {
      _setOffset( MDT_TriangleNormals, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_TriangleNormals, true, offsetList );

      unsigned count = modelTriangles.size();

      writeHeaderB( 0x0000, count, FILE_TRIANGLE_NORMAL_SIZE );

      for ( unsigned t = 0; t < count; t++ )
      {
         MM3DFILE_TriangleNormalsT fileNormals;

         fileNormals.flags = 0x0000;
         fileNormals.index = t;

         for ( unsigned v = 0; v < 3; v++ )
         {
            fileNormals.normal[v][0] = modelTriangles[t]->m_vertexNormals[v][0];
            fileNormals.normal[v][1] = modelTriangles[t]->m_vertexNormals[v][1];
            fileNormals.normal[v][2] = modelTriangles[t]->m_vertexNormals[v][2];
         }

         write( fileNormals.flags );
         write( fileNormals.index );
         write( fileNormals.normal[0][0] );
         write( fileNormals.normal[0][1] );
         write( fileNormals.normal[0][2] );
         write( fileNormals.normal[1][0] );
         write( fileNormals.normal[1][1] );
         write( fileNormals.normal[1][2] );
         write( fileNormals.normal[2][0] );
         write( fileNormals.normal[2][1] );
         write( fileNormals.normal[2][2] );
      }
      log_debug( "wrote %d triangle normals\n", count );
   }

   // Groups
   if ( doWrite[ MDT_Groups ] )
   {
      _setOffset( MDT_Groups, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_Groups, false, offsetList );

      unsigned count = modelGroups.size();

      writeHeaderA( 0x0000, count );

      unsigned baseSize = sizeof(uint16_t) + sizeof(uint32_t)
         + sizeof(uint8_t) + sizeof(uint32_t);

      for ( unsigned g = 0; g < count; g++ )
      {
         Model::Group * grp = modelGroups[g];
         unsigned groupSize = baseSize + grp->m_name.length() + 1 
            + (grp->m_triangleIndices.size() * sizeof(uint32_t));

         uint16_t flags = 0x0000;
         flags |= (modelGroups[g]->m_visible)  ? 0 : MF_HIDDEN;
         flags |= (modelGroups[g]->m_selected) ? MF_SELECTED : 0;
         uint32_t triCount = grp->m_triangleIndices.size();

         write( groupSize );
         write( flags );
         writeBytes( grp->m_name.c_str(), grp->m_name.length() + 1 );
         write( triCount );

         for ( unsigned t = 0; t < triCount; t++ )
         {
            uint32_t triIndex = grp->m_triangleIndices[t];
            write( triIndex );
         }
         uint8_t  smoothness = grp->m_smooth;
         uint32_t materialIndex = grp->m_materialIndex;
         write( smoothness );
         write( materialIndex );

      }
      log_debug( "wrote %d groups\n", count );
   }

   int texNum = 0;

   // Materials
   if ( doWrite[ MDT_Materials ] )
   {
      _setOffset( MDT_Materials, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_Materials, false, offsetList );

      unsigned count = modelMaterials.size();

      writeHeaderA( 0x0000, count );

      unsigned baseSize = sizeof(uint16_t) + sizeof(uint32_t)
         + (sizeof(float32_t) * 17);

      for ( unsigned m = 0; m < count; m++ )
      {
         Model::Material * mat = modelMaterials[m];
         unsigned matSize = baseSize + mat->m_name.length() + 1;

         uint16_t flags = 0x0000;
         uint32_t texIndex = texNum;  // TODO deal with embedded textures

         switch ( mat->m_type )
         {
            case Model::Material::MATTYPE_COLOR:
               flags = 0x000d;
               break;
            case Model::Material::MATTYPE_GRADIENT:
               flags = 0x000e;
               break;
            case Model::Material::MATTYPE_BLANK:
               flags = 0x000f;
               break;
            case Model::Material::MATTYPE_TEXTURE:
               flags = 0x0000;
               texNum++;
               break;
            default:
               break;
         }

         if ( mat->m_sClamp )
         {
            flags |= MF_MAT_CLAMP_S;
         }
         if ( mat->m_tClamp )
         {
            flags |= MF_MAT_CLAMP_T;
         }

         write( matSize );
         write( flags );
         write( texIndex );
         writeBytes( mat->m_name.c_str(), mat->m_name.length() + 1 );

         unsigned i = 0;
         float32_t lightProp = 0;
         for ( i = 0; i < 4; i++ )
         {
            lightProp = mat->m_ambient[i];
            write( lightProp );
         }
         for ( i = 0; i < 4; i++ )
         {
            lightProp = mat->m_diffuse[i];
            write( lightProp );
         }
         for ( i = 0; i < 4; i++ )
         {
            lightProp = mat->m_specular[i];
            write( lightProp );
         }
         for ( i = 0; i < 4; i++ )
         {
            lightProp = mat->m_emissive[i];
            write( lightProp );
         }
         write( lightProp );

      }
      log_debug( "wrote %d materials with %d internal textures\n", count, texNum );
   }

   // External Textures
   if ( doWrite[ MDT_ExtTextures ] )
   {
      _setOffset( MDT_ExtTextures, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_ExtTextures, false, offsetList );

      unsigned count = modelMaterials.size();

      writeHeaderA( 0x0000, count );

      unsigned baseSize = sizeof(uint16_t);

      for ( unsigned m = 0; m < count; m++ )
      {
         Model::Material * mat = modelMaterials[m];
         if ( mat->m_type == Model::Material::MATTYPE_TEXTURE )
         {
            std::string fileStr = getRelativePath( modelPath.c_str(), mat->m_filename.c_str() );

            char filename[PATH_MAX];
            strncpy( filename, fileStr.c_str(), PATH_MAX );
            utf8chrtrunc( filename, sizeof(filename)-1 );

            replaceSlash( filename );

            unsigned texSize = baseSize + strlen(filename) + 1;

            uint16_t flags = 0x0000;

            write( texSize );
            write( flags );
            writeBytes( filename, strlen(filename) + 1 );

            log_debug( "material file is %s\n", filename );
         }
      }
      log_debug( "wrote %d external textures\n", texNum );
   }

   // Texture Coordinates
   if ( doWrite[ MDT_TexCoords ] )
   {
      _setOffset( MDT_TexCoords, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_TexCoords, true, offsetList );

      unsigned count = modelTriangles.size();

      writeHeaderB( 0x0000, count, FILE_TEXCOORD_SIZE );

      for ( unsigned t = 0; t < count; t++ )
      {
         Model::Triangle * tri = modelTriangles[t];
         MM3DFILE_TexCoordT tc;

         tc.flags = 0x0000;
         tc.triangleIndex = t;
         for ( unsigned v = 0; v < 3; v++ )
         {
            tc.sCoord[v] = tri->m_s[v];
            tc.tCoord[v] = tri->m_t[v];
         }

         write( tc.flags );
         write( tc.triangleIndex );
         write( tc.sCoord[0] );
         write( tc.sCoord[1] );
         write( tc.sCoord[2] );
         write( tc.tCoord[0] );
         write( tc.tCoord[1] );
         write( tc.tCoord[2] );
      }
      log_debug( "wrote %d texture coordinates\n", count );
   }

   // Texture Projection Triangles
   if ( doWrite[ MDT_ProjectionTriangles ] )
   {
      _setOffset( MDT_ProjectionTriangles, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_ProjectionTriangles, false, offsetList );

      unsigned pcount = model->getProjectionCount();

      writeHeaderA( 0x0000, pcount );

      for ( unsigned p = 0; p < pcount; p++ )
      {
         unsigned wcount = 0; // triangles to write

         unsigned tcount = model->getTriangleCount();
         unsigned t;
         for ( t = 0; t < tcount; t++ )
         {
            if ( model->getTriangleProjection(t) == (int) p )
            {
               wcount++;
            }
         }

         uint32_t triSize = sizeof(uint32_t) * 2 
               + sizeof(uint32_t) * wcount;
         uint32_t writeProj  = (uint32_t) p;
         uint32_t writeCount = (uint32_t) wcount;

         write( triSize );
         write( writeProj );
         write( writeCount );

         for ( t = 0; t < tcount; t++ )
         {
            if ( model->getTriangleProjection(t) == (int) p )
            {
               uint32_t tri = t;
               write( tri );
            }
         }
      }
      log_debug( "wrote %d external textures\n", texNum );
   }

   // Canvas Backgrounds
   if ( doWrite[ MDT_CanvasBackgrounds ] )
   {
      _setOffset( MDT_CanvasBackgrounds, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_CanvasBackgrounds, false, offsetList );

      unsigned count = backgrounds;

      writeHeaderA( 0x0000, count );

      unsigned baseSize = FILE_CANVAS_BACKGROUND_SIZE;

      for ( unsigned b = 0; b < 6; b++ )
      {
         const char * file = model->getBackgroundImage( b );

         if ( file[0] != '\0' )
         {
            MM3DFILE_CanvasBackgroundT cb;
            cb.flags = 0x0000;
            cb.viewIndex = b;

            cb.scale = model->getBackgroundScale( b );
            model->getBackgroundCenter( b, cb.center[0], cb.center[1], cb.center[2] );
            cb.center[0] = cb.center[0];
            cb.center[1] = cb.center[1];
            cb.center[2] = cb.center[2];

            std::string fileStr = getRelativePath( modelPath.c_str(), file );
            unsigned backSize = baseSize + fileStr.size() + 1;

            char * filedup = strdup( fileStr.c_str() );
            replaceSlash( filedup );
            utf8chrtrunc( filedup, PATH_MAX-1 );

            write( backSize );
            write( cb.flags );
            write( cb.viewIndex );
            write( cb.scale );
            write( cb.center[0] );
            write( cb.center[1] );
            write( cb.center[2] );
            writeBytes( filedup, fileStr.size() + 1 );

            free( filedup );
         }
      }
      log_debug( "wrote %d canvas backgrounds\n", count );
   }

   // Joints
   if ( doWrite[ MDT_Joints ] )
   {
      _setOffset( MDT_Joints, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_Joints, true, offsetList );

      unsigned count = modelJoints.size();

      writeHeaderB( 0x0000, count, FILE_JOINT_SIZE );

      for ( unsigned j = 0; j < count; j++ )
      {
         MM3DFILE_JointT fileJoint;
         Model::Joint * joint = modelJoints[j];

         fileJoint.flags = 0x0000;
         fileJoint.flags |= (modelJoints[j]->m_visible)  ? 0 : MF_HIDDEN;
         fileJoint.flags |= (modelJoints[j]->m_selected) ? MF_SELECTED : 0;

         strncpy( fileJoint.name, joint->m_name.c_str(), sizeof(fileJoint.name) );
         utf8chrtrunc( fileJoint.name, sizeof(fileJoint.name)-1 );

         fileJoint.parentIndex = joint->m_parent;
         for ( unsigned i = 0; i < 3; i++ )
         {
            fileJoint.localRot[i]   = joint->m_localRotation[i];
            fileJoint.localTrans[i] = joint->m_localTranslation[i];
         }

         write( fileJoint.flags );
         writeBytes( fileJoint.name, sizeof(fileJoint.name) );
         write( fileJoint.parentIndex );
         write( fileJoint.localRot[0] );
         write( fileJoint.localRot[1] );
         write( fileJoint.localRot[2] );
         write( fileJoint.localTrans[0] );
         write( fileJoint.localTrans[1] );
         write( fileJoint.localTrans[2] );
      }
      log_debug( "wrote %d joints\n", count );
   }

   // Joint Vertices
   // Newer versions of the file format do not use this section, but old
   // versions of mm3d need it.  Write it for the sake of backwards 
   // compatibility
   if ( doWrite[ MDT_JointVertices ] )
   {
      _setOffset( MDT_JointVertices, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_JointVertices, true, offsetList );

      unsigned count = 0;
      unsigned vcount = modelVertices.size();

      unsigned v = 0;
      for ( v = 0; v < vcount; v++ )
      {
         if ( model->getPrimaryVertexInfluence( v ) >= 0 )
         {
            count++;
         }
      }

      writeHeaderB( 0x0000, count, FILE_JOINT_VERTEX_SIZE );

      for ( v = 0; v < vcount; v++ )
      {
         int boneId = model->getPrimaryVertexInfluence( v );
         if ( boneId >= 0 )
         {
            MM3DFILE_JointVertexT fileJv;
            fileJv.vertexIndex = v;
            fileJv.jointIndex = boneId;

            write( fileJv.vertexIndex );
            write( fileJv.jointIndex );
         }
      }
      log_debug( "wrote %d joint vertex assignments\n", count );
   }

   // Points
   if ( doWrite[ MDT_Points ] )
   {
      _setOffset( MDT_Points, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_Points, true, offsetList );

      unsigned count = model->getPointCount();

      writeHeaderB( 0x0000, count, FILE_POINT_SIZE );

      for ( unsigned p = 0; p < count; p++ )
      {
         MM3DFILE_PointT filePoint;

         filePoint.flags = 0x0000;
         filePoint.flags |= (model->isPointVisible(p)) ? 0 : MF_HIDDEN;
         filePoint.flags |= (model->isPointSelected(p)) ? MF_SELECTED : 0;

         strncpy( filePoint.name, model->getPointName(p), sizeof(filePoint.name) );
         utf8chrtrunc( filePoint.name, sizeof(filePoint.name)-1 );

         filePoint.boneIndex = model->getPrimaryPointInfluence( p );

         Model::Point * point = modelPoints[p];
         for ( unsigned i = 0; i < 3; i++ )
         {
            filePoint.rot[i]   = point->m_rot[i];
            filePoint.trans[i] = point->m_trans[i];
         }

         write( filePoint.flags );
         writeBytes( filePoint.name, sizeof(filePoint.name) );
         write( filePoint.type );
         write( filePoint.boneIndex );
         write( filePoint.rot[0] );
         write( filePoint.rot[1] );
         write( filePoint.rot[2] );
         write( filePoint.trans[0] );
         write( filePoint.trans[1] );
         write( filePoint.trans[2] );
      }
      log_debug( "wrote %d points\n", count );
   }

   // Smooth Angles
   if ( doWrite[ MDT_SmoothAngles ] )
   {
      _setOffset( MDT_SmoothAngles, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_SmoothAngles, true, offsetList );

      unsigned count = modelGroups.size();

      writeHeaderB( 0x0000, count, FILE_SMOOTH_ANGLE_SIZE );

      for ( unsigned t = 0; t < count; t++ )
      {
         MM3DFILE_SmoothAngleT fileSa;
         fileSa.groupIndex = t;
         fileSa.angle = model->getGroupAngle( t );

         write( fileSa.groupIndex );
         write( fileSa.angle );
      }
      log_debug( "wrote %d group smoothness angles\n", count );
   }

   // Weighted influences
   if ( doWrite[ MDT_WeightedInfluences ] )
   {
      _setOffset( MDT_WeightedInfluences, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_WeightedInfluences, true, offsetList );

      Model::InfluenceList ilist;
      Model::InfluenceList::iterator it;

      unsigned count = 0;
      unsigned vcount = modelVertices.size();
      unsigned pcount = modelPoints.size();

      unsigned v = 0;
      unsigned p = 0;

      for ( v = 0; v < vcount; v++ )
      {
         model->getVertexInfluences( v, ilist );
         count += ilist.size();
      }

      for ( p = 0; p < pcount; p++ )
      {
         model->getPointInfluences( p, ilist );
         count += ilist.size();
      }

      writeHeaderB( 0x0000, count, FILE_WEIGHTED_INFLUENCE_SIZE );

      for ( v = 0; v < vcount; v++ )
      {
         model->getVertexInfluences( v, ilist );
         for ( it = ilist.begin(); it != ilist.end(); it++ )
         {
            MM3DFILE_WeightedInfluenceT fileWi;
            fileWi.posType   = Model::PT_Vertex;
            fileWi.posIndex  = v;
            fileWi.infType   = (*it).m_type;
            fileWi.infIndex  = (*it).m_boneId;
            fileWi.infWeight = (int) ((*it).m_weight * 100.1); // round up slightly to prevent loss

            write( fileWi.posType );
            write( fileWi.posIndex );
            write( fileWi.infIndex );
            write( fileWi.infType );
            write( fileWi.infWeight );
         }
      }

      for ( p = 0; p < pcount; p++ )
      {
         model->getPointInfluences( p, ilist );
         for ( it = ilist.begin(); it != ilist.end(); it++ )
         {
            MM3DFILE_WeightedInfluenceT fileWi;
            fileWi.posType   = Model::PT_Point;
            fileWi.posIndex  = p;
            fileWi.infType   = (*it).m_type;
            fileWi.infIndex  = (*it).m_boneId;
            fileWi.infWeight = (int) ((*it).m_weight * 100.1); // round up slightly to prevent loss

            write( fileWi.posType );
            write( fileWi.posIndex );
            write( fileWi.infIndex );
            write( fileWi.infType );
            write( fileWi.infWeight );
         }
      }

      log_debug( "wrote %d weighted influences\n", count );
   }

   // Texture Projections
   if ( doWrite[ MDT_TexProjections ] )
   {
      _setOffset( MDT_TexProjections, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_TexProjections, true, offsetList );

      unsigned count = model->getProjectionCount();

      writeHeaderB( 0x0000, count, FILE_TEXTURE_PROJECTION_SIZE );

      for ( unsigned p = 0; p < count; p++ )
      {
         uint16_t flags = 0x0000;
         write( flags );

         char name[40];
         PORT_snprintf( name, sizeof(name), "%s", model->getProjectionName( p ) );
         utf8chrtrunc( name, sizeof(name)-1 );
         writeBytes( name, sizeof(name) );

         int32_t type = model->getProjectionType( p );
         write( type );

         double coord[3]  = { 0, 0, 0 };
         float  fcoord[3] = { 0.0f, 0.0f, 0.0f };

         model->getProjectionCoords( p, coord );
         fcoord[0] = coord[0];
         fcoord[1] = coord[1];
         fcoord[2] = coord[2];
         write( fcoord[0] );
         write( fcoord[1] );
         write( fcoord[2] );

         model->getProjectionUp( p, coord );
         fcoord[0] = coord[0];
         fcoord[1] = coord[1];
         fcoord[2] = coord[2];
         write( fcoord[0] );
         write( fcoord[1] );
         write( fcoord[2] );

         model->getProjectionSeam( p, coord );
         fcoord[0] = coord[0];
         fcoord[1] = coord[1];
         fcoord[2] = coord[2];
         write( fcoord[0] );
         write( fcoord[1] );
         write( fcoord[2] );

         double uv[2][2];
         float  fuv[2][2];

         model->getProjectionRange( p, uv[0][0], uv[0][1], uv[1][0], uv[1][1] );
         fuv[0][0] = uv[0][0];
         fuv[0][1] = uv[0][1];
         fuv[1][0] = uv[1][0];
         fuv[1][1] = uv[1][1];
         write( fuv[0][0] );
         write( fuv[0][1] );
         write( fuv[1][0] );
         write( fuv[1][1] );
      }
      log_debug( "wrote %d texture projections\n", count );
   }

   // Skel Anims
   if ( doWrite[ MDT_SkelAnims ] )
   {
      _setOffset( MDT_SkelAnims, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_SkelAnims, false, offsetList );

      unsigned count = modelSkels.size();

      writeHeaderA( 0x0000, count );

      unsigned baseSize = sizeof(uint16_t) + sizeof(float32_t) + sizeof(uint32_t);

      for ( unsigned s = 0; s < count; s++ )
      {
         Model::SkelAnim * sa = modelSkels[s];
         unsigned animSize = baseSize + sa->m_name.length() + 1;

         uint32_t frameCount = sa->m_frameCount;
         uint32_t keyframeCount = 0;
         unsigned f = 0;
         for ( f = 0; f < frameCount; f++ )
         {
            for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
            {
               for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
               {
                  if ( sa->m_jointKeyframes[j][k]->m_frame == f )
                  {
                     keyframeCount++;
                  }
               }
            }
         }

         animSize += frameCount    * sizeof(uint32_t);
         animSize += keyframeCount * FILE_SKEL_KEYFRAME_SIZE;

         uint16_t  flags = 0x0000;
         float32_t fps = sa->m_fps;

         animSize = animSize;
         write( animSize );
         write( flags );
         writeBytes( sa->m_name.c_str(), sa->m_name.length() + 1 );
         write( fps );
         uint32_t temp32 = frameCount;
         write( temp32 );

         for ( f = 0; f < frameCount; f++ )
         {
            keyframeCount = 0;
            unsigned j = 0;
            for ( j = 0; j < sa->m_jointKeyframes.size(); j++ )
            {
               for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
               {
                  if ( sa->m_jointKeyframes[j][k]->m_frame == f )
                  {
                     keyframeCount++;
                  }
               }
            }

            temp32 = keyframeCount;
            write( temp32 );

            unsigned written = 0;
            for ( j = 0; j < sa->m_jointKeyframes.size(); j++ )
            {
               for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
               {
                  Model::Keyframe * kf = sa->m_jointKeyframes[j][k];
                  if ( kf->m_frame == f )
                  {
                     MM3DFILE_SkelKeyframeT fileKf;
                     fileKf.jointIndex = j;
                     fileKf.keyframeType = kf->m_isRotation ? 0 : 1;
                     fileKf.param[0] = kf->m_parameter[0];
                     fileKf.param[1] = kf->m_parameter[1];
                     fileKf.param[2] = kf->m_parameter[2];

                     write( fileKf.jointIndex );
                     write( fileKf.keyframeType );
                     write( fileKf.param[0] );
                     write( fileKf.param[1] );
                     write( fileKf.param[2] );

                     written++;
                  }
               }
            }
         }
      }
      log_debug( "wrote %d skel anims\n", count );
   }

   // Frame Anims
   if ( doWrite[ MDT_FrameAnims ] )
   {
      _setOffset( MDT_FrameAnims, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_FrameAnims, false, offsetList );

      unsigned count = modelFrames.size();

      writeHeaderA( 0x0000, count );

      unsigned baseSize = sizeof(uint16_t) + sizeof(float32_t) + sizeof(uint32_t);

      for ( unsigned a = 0; a < count; a++ )
      {
         Model::FrameAnim * fa = modelFrames[a];
         unsigned animSize = baseSize + fa->m_name.length() + 1;

         uint32_t frameCount = fa->m_frameData.size();

         unsigned vcount = modelVertices.size();

         animSize += frameCount * sizeof(uint32_t);
         animSize += frameCount * vcount * sizeof(float32_t) * 3;

         uint16_t  flags = 0x0000;
         float32_t fps = fa->m_fps;

         write( animSize );
         write( flags );
         writeBytes( fa->m_name.c_str(), fa->m_name.length() + 1 );
         write( fps );
         uint32_t temp32 = frameCount;
         write( temp32 );

         for ( unsigned f = 0; f < frameCount; f++ )
         {
            Model::FrameAnimVertexList * list = fa->m_frameData[f]->m_frameVertices;
            unsigned avcount = list->size();
            for ( unsigned v = 0; v < vcount; v++ )
            {
               if ( v < avcount )
               {
                  for ( unsigned i = 0; i < 3; i++ )
                  {
                     float32_t coord = (*list)[v]->m_coord[i];
                     write( coord );
                  }
               }
               else
               {
                  write( 0.0f );
                  write( 0.0f );
                  write( 0.0f );
               }
            }
         }
      }
      log_debug( "wrote %d frame anims\n", count );
   }

   // Frame Anim Points
   if ( doWrite[ MDT_FrameAnimPoints ] )
   {
      _setOffset( MDT_FrameAnimPoints, ftell(m_fp), offsetList );
      _setUniformOffset( MDT_FrameAnimPoints, false, offsetList );

      unsigned count = modelFrames.size();

      writeHeaderA( 0x0000, count );

      unsigned baseSize = sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t);

      unsigned pcount = model->getPointCount();

      for ( unsigned a = 0; a < count; a++ )
      {
         Model::FrameAnim * fa = modelFrames[a];

         uint32_t frameCount = fa->m_frameData.size();
         uint32_t animSize = baseSize + frameCount * (pcount * 6 * sizeof(float32_t));

         uint16_t  flags = 0x0000;

         uint32_t anim = a;

         write( animSize );
         write( flags );
         write( anim );
         write( frameCount );

         for ( unsigned f = 0; f < frameCount; f++ )
         {
            Model::FrameAnimPointList * list = fa->m_frameData[f]->m_framePoints;
            unsigned apcount = list->size();
            for ( unsigned p = 0; p < pcount; p++ )
            {
               unsigned i;
               if ( p < apcount )
               {
                  for ( i = 0; i < 3; i++ )
                  {
                     float32_t rot = (*list)[p]->m_rot[i];
                     write( rot );
                  }
                  for ( i = 0; i < 3; i++ )
                  {
                     float32_t trans = (*list)[p]->m_trans[i];
                     write( trans );
                  }
               }
               else
               {
                  // rot
                  write( 0.0f );
                  write( 0.0f );
                  write( 0.0f );

                  // trans
                  write( 0.0f );
                  write( 0.0f );
                  write( 0.0f );
               }
            }
         }
      }
      log_debug( "wrote %d frame anim points\n", count );
   }

   // Write unknown data (add dirty flag to offset type)
   for ( f = 0; f < formatDataCount; f++ )
   {
      Model::FormatData * fd = model->getFormatData( f );
      uint16_t thisType = (fd->offsetType | OFFSET_DIRTY_MASK);
      if ( fd->offsetType != 0 )
      {
         MisfitOffsetList::iterator it;
         for ( it = offsetList.begin(); it != offsetList.end(); it++ )
         {
            if ( (*it).offsetType == thisType )
            {
               (*it).offsetValue = ftell( m_fp );
               log_warning( "setting uknown data type %04x offset at %08x\n", (*it).offsetType, (*it).offsetValue );
               writeBytes( fd->data, fd->len );
               break;
            }
         }
      }
   }

   // Re-write header with offsets

   _setOffset( MDT_EndOfFile, ftell(m_fp), offsetList );

   fseek( m_fp, 12, SEEK_SET );
   for ( t = 0; t < offsetCount; t++ )
   {
      MisfitOffsetT & mo = offsetList[t];
      write( mo.offsetType );
      write( mo.offsetValue );
   }
   log_debug( "wrote %d updated offsets\n", offsetCount );

   fclose( m_fp );
   m_fp = NULL;

   return Model::ERROR_NONE;
}

bool MisfitFilter::canRead( const char * filename )
{
   return true;
}

bool MisfitFilter::canWrite( const char * filename )
{
   return true;
}

bool MisfitFilter::canExport( const char * filename )
{
   return true;
}

bool MisfitFilter::isSupported( const char * filename )
{
   unsigned len = strlen( filename );

   if ( len >= 5 && strcasecmp( &filename[len-5], ".mm3d" ) == 0 )
   {
      return true;
   }
   else
   {
      return false;
   }
}

list< string > MisfitFilter::getReadTypes()
{
   list<string> rval;
   rval.push_back( "*.mm3d" );
   return rval;
}

list< string > MisfitFilter::getWriteTypes()
{
   list<string> rval;
   rval.push_back( "*.mm3d" );
   return rval;
}

void MisfitFilter::write( uint32_t val )
{
   val = htol_u32( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void MisfitFilter::write( uint16_t val )
{
   val = htol_u16( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void MisfitFilter::write( uint8_t val )
{
   fwrite( &val, sizeof(val), 1, m_fp );
}

void MisfitFilter::write( int32_t val )
{
   val = htol_32( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void MisfitFilter::write( int16_t val )
{
   val = htol_16( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void MisfitFilter::write( int8_t val )
{
   fwrite( &val, sizeof(val), 1, m_fp );
}

void MisfitFilter::write( float32_t val )
{
   val = htol_float( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void MisfitFilter::writeBytes( const void * buf, size_t len )
{
   fwrite( buf, len, 1, m_fp );
}

void MisfitFilter::writeHeaderA( uint16_t flags, uint32_t count )
{
   write( flags );
   write( count );
}

void MisfitFilter::writeHeaderB( uint16_t flags, uint32_t count, uint32_t size )
{
   write( flags );
   write( count );
   write( size );
}

void MisfitFilter::read( uint32_t & val )
{
   if ( m_readLength >= sizeof( val ) )
   {
      val = ltoh_u32( * (uint32_t*) m_bufPos );
      m_bufPos += sizeof( uint32_t );
      m_readLength -= sizeof( uint32_t );
   }
}

void MisfitFilter::read( uint16_t & val )
{
   if ( m_readLength >= sizeof( val ) )
   {
      val = ltoh_u16( * (uint16_t*) m_bufPos );
      m_bufPos += sizeof( uint16_t );
      m_readLength -= sizeof( uint16_t );
   }
}

void MisfitFilter::read( uint8_t & val )
{
   if ( m_readLength >= sizeof( val ) )
   {
      val = * (uint8_t*) m_bufPos;
      m_bufPos += sizeof( uint8_t );
      m_readLength -= sizeof( uint8_t );
   }
}

void MisfitFilter::read( int32_t & val )
{
   if ( m_readLength >= sizeof( val ) )
   {
      val = ltoh_32( * (int32_t*) m_bufPos );
      m_bufPos += sizeof( int32_t );
      m_readLength -= sizeof( int32_t );
   }
}

void MisfitFilter::read( int16_t & val )
{
   if ( m_readLength >= sizeof( val ) )
   {
      val = ltoh_16( * (int16_t*) m_bufPos );
      m_bufPos += sizeof( int16_t );
      m_readLength -= sizeof( int16_t );
   }
}

void MisfitFilter::read( int8_t & val )
{
   if ( m_readLength >= sizeof( val ) )
   {
      val = * (int8_t*) m_bufPos;
      m_bufPos += sizeof( int8_t );
      m_readLength -= sizeof( int8_t );
   }
}

void MisfitFilter::read( float32_t & val )
{
   if ( m_readLength >= sizeof( val ) )
   {
      val = ltoh_float( * (float32_t*) m_bufPos );
      m_bufPos += sizeof( float32_t );
      m_readLength -= sizeof( float32_t );
   }
}

void MisfitFilter::readBytes( void * buf, size_t len )
{
   if ( m_readLength >= len )
   {
      memcpy( buf, m_bufPos, len );
      m_bufPos += len;
      m_readLength -= len;
   }
}

void MisfitFilter::readHeaderA( uint16_t & flags, uint32_t & count )
{
   read( flags );
   read( count );
}

void MisfitFilter::readHeaderB( uint16_t & flags, uint32_t & count, uint32_t & size )
{
   read( flags );
   read( count );
   read( size );
}

