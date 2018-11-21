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

#include "ms3dfilter.h"

#include "weld.h"
#include "misc.h"
#include "log.h"
#include "release_ptr.h"
#include "local_array.h"
#include "file_closer.h"
#include "mm3dport.h"
#include "util.h"
#include "datadest.h"
#include "datasource.h"

#include "translate.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include <string>
#include <vector>

using std::list;
using std::string;

char const Ms3dFilter::MAGIC_NUMBER[] = "MS3D000000";

/* 
	MS3D STRUCTURES 
*/

// File header
struct MS3DHeader
{
	char m_ID[10];
	int32_t m_version;
};

// Vertex information
struct MS3DVertex
{
	uint8_t m_flags;
	float32_t m_vertex[3];
	uint8_t m_boneId;
	uint8_t m_refCount;
};

const size_t FILE_VERTEX_SIZE = 15;

// Triangle information
struct MS3DTriangle
{
	uint16_t m_flags;
	uint16_t m_vertexIndices[3];
	float32_t m_vertexNormals[3][3];
	float32_t m_s[3];
   float32_t m_t[3];
	uint8_t m_smoothingGroup;
	uint8_t m_groupIndex;
};

const size_t FILE_TRIANGLE_SIZE = 72;

// Material information
struct MS3DMaterial
{
    char m_name[32];
    float32_t m_ambient[4];
    float32_t m_diffuse[4];
    float32_t m_specular[4];
    float32_t m_emissive[4];
    float32_t m_shininess;	// 0.0f - 128.0f
    float32_t m_transparency;	// 0.0f - 1.0f
    uint8_t m_mode;	// 0, 1, 2 is unused now
    char m_texture[128];
    char m_alphamap[128];
};

const size_t FILE_MATERIAL_SIZE = 361;

//	Joint information
struct MS3DJoint
{
	uint8_t m_flags;
	char m_name[32];
	char m_parentName[32];
	float32_t m_rotation[3];
	float32_t m_translation[3];
	uint16_t m_numRotationKeyframes;
	uint16_t m_numTranslationKeyframes;
};

const size_t FILE_JOINT_SIZE = 93;

// Keyframe data
struct MS3DKeyframe
{
	float32_t m_time;
	float32_t m_parameter[3];
};

const size_t FILE_KEYFRAME_SIZE = 16;

// flags 
//    1 = selected
//    2 = hidden
//

struct _JointNameListRec_t
{
   int m_jointIndex;
   std::string m_name;
};
typedef struct _JointNameListRec_t JointNameListRecT;

Ms3dFilter::Ms3dOptions::Ms3dOptions()
   : m_subVersion( 0 ),
     m_vertexExtra( 0xffffffff ),
     m_vertexExtra2( 0xffffffff ),
     m_jointColor( 0xffffffff )
{
}

Ms3dFilter::Ms3dOptions::~Ms3dOptions()
{
}

void Ms3dFilter::Ms3dOptions::setOptionsFromModel( Model * m )
{
   char value[128];
   if ( m->getMetaData( "ms3d_sub_version", value, sizeof(value) ) )
   {
      m_subVersion = atoi( value );
      m_subVersion = util_clamp( m_subVersion, 0, 3 );
   }
   else
   {
      // No sub-version defined. Use zero unless we have multiple bone joint
      // influences for any vertex.
      m_subVersion = 0;
      unsigned vcount = m->getVertexCount();
      Model::InfluenceList il;
      for ( unsigned v = 0; m_subVersion == 0 && v < vcount; v++ )
      {
         m->getVertexInfluences( v, il );
         if ( il.size() > 1 )
         {
            m_subVersion = 1;
         }
      }
   }

   if ( m->getMetaData( "ms3d_vertex_extra", value, sizeof(value) ) )
   {
      sscanf( value, "%x", &m_vertexExtra );
   }

   if ( m->getMetaData( "ms3d_vertex_extra2", value, sizeof(value) ) )
   {
      sscanf( value, "%x", &m_vertexExtra2 );
   }

   // TODO joint color
   //if ( m->getMetaData( "ms3d_joint_color", value, sizeof(value) ) )
   //{
   //   sscanf( value, "%x", &m_jointColor );
   //}
}

Model::ModelErrorE Ms3dFilter::readFile( Model * model, const char * const filename )
{
   if ( !model || !filename )
      return Model::ERROR_BAD_ARGUMENT;

   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_src = openInput( filename, err );
   SourceCloser fc( m_src );

   if ( err != Model::ERROR_NONE )
      return err;

   model->setFilename( modelFullName.c_str() );

   vector<Model::Vertex *>   & modelVertices  = getVertexList( model );
   vector<Model::Triangle *> & modelTriangles = getTriangleList( model );
   vector<Model::Group *>    & modelGroups    = getGroupList( model );
   vector<Model::Material *> & modelMaterials = getMaterialList( model );
   vector<Model::Joint *>    & modelJoints    = getJointList( model );

   unsigned fileLength = m_src->getFileSize();

   m_model  = model;

   if ( fileLength < (sizeof( MS3DHeader ) + sizeof(uint16_t) ) )
   {
      return Model::ERROR_UNEXPECTED_EOF;
   }

   // Check header
   MS3DHeader header;
   m_src->readBytes( (uint8_t *) header.m_ID, sizeof(header.m_ID) );
   m_src->read( header.m_version );

   if ( strncmp( header.m_ID, MAGIC_NUMBER, 10 ) != 0 )
   {
      log_error( "bad magic number\n" );
      return Model::ERROR_BAD_MAGIC;
   }

   int version = header.m_version;
   if ( version < 3 || version > 4 )
   {
      log_error( "unsupported version\n" );
      return Model::ERROR_UNSUPPORTED_VERSION;
   }

   uint16_t t; // MS Visual C++ is lame

   uint16_t numVertices = 0;
   m_src->read( numVertices );

   // TODO verify file size vs. numVertices

   std::vector< int > vertexJoints;
   for ( t = 0; t < numVertices; t++ )
   {
      MS3DVertex vertex;
      m_src->read( vertex.m_flags );
      m_src->read( vertex.m_vertex[0] );
      m_src->read( vertex.m_vertex[1] );
      m_src->read( vertex.m_vertex[2] );
      m_src->read( vertex.m_boneId );
      m_src->read( vertex.m_refCount );

      Model::Vertex * vert = Model::Vertex::get();
      for ( int v = 0; v < 3; v++ )
      {
         vert->m_coord[v] = vertex.m_vertex[ v ];
      }
      modelVertices.push_back( vert );
      if ( vertex.m_boneId == 0xFF )
      {
         vertexJoints.push_back( -1 );
      }
      else
      {
         vertexJoints.push_back( vertex.m_boneId );
      }
   }

   uint16_t numTriangles = 0;
   m_src->read( numTriangles );

   if ( m_src->getRemaining() < (FILE_TRIANGLE_SIZE * numTriangles + sizeof( uint16_t )) )
   {
      return Model::ERROR_UNEXPECTED_EOF;
   }

   for ( t = 0; t < numTriangles; t++ )
   {
      Model::Triangle * curTriangle = Model::Triangle::get();
      MS3DTriangle triangle;
      m_src->read( triangle.m_flags );
      m_src->read( triangle.m_vertexIndices[0] );
      m_src->read( triangle.m_vertexIndices[1] );
      m_src->read( triangle.m_vertexIndices[2] );
      m_src->read( triangle.m_vertexNormals[0][0] );
      m_src->read( triangle.m_vertexNormals[0][1] );
      m_src->read( triangle.m_vertexNormals[0][2] );
      m_src->read( triangle.m_vertexNormals[1][0] );
      m_src->read( triangle.m_vertexNormals[1][1] );
      m_src->read( triangle.m_vertexNormals[1][2] );
      m_src->read( triangle.m_vertexNormals[2][0] );
      m_src->read( triangle.m_vertexNormals[2][1] );
      m_src->read( triangle.m_vertexNormals[2][2] );
      m_src->read( triangle.m_s[0] );
      m_src->read( triangle.m_s[1] );
      m_src->read( triangle.m_s[2] );
      m_src->read( triangle.m_t[0] );
      m_src->read( triangle.m_t[1] );
      m_src->read( triangle.m_t[2] );
      m_src->read( triangle.m_smoothingGroup );
      m_src->read( triangle.m_groupIndex );

      curTriangle->m_vertexIndices[0] = triangle.m_vertexIndices[0];
      curTriangle->m_vertexIndices[1] = triangle.m_vertexIndices[1];
      curTriangle->m_vertexIndices[2] = triangle.m_vertexIndices[2];

      for ( int i = 0; i < 3; i++ )
      {
         if ( curTriangle->m_vertexIndices[i] >= numVertices )
         {
            log_error( "vertex out of range: %d/%d\n", curTriangle->m_vertexIndices[i],
                  modelVertices.size() );
            return Model::ERROR_BAD_DATA;
         }
      }

      for ( int i = 0; i < 3; i++ )
      {
         // Need to invert the T coord, since milkshape seems to store it
         // upside-down.
         curTriangle->m_s[i] = triangle.m_s[i];
         curTriangle->m_t[i] = 1.0 - triangle.m_t[i];
      }

      for ( int y = 0; y < 3; y++ )
      {
         for( int x = 0; x < 3; x++ )
         {
            curTriangle->m_vertexNormals[y][x] = triangle.m_vertexNormals[y][x];
         }
      }

      modelTriangles.push_back( curTriangle );
   }

   uint16_t numGroups = 0;
   m_src->read( numGroups );

   log_debug( "model says %d groups\n" );

   for ( t = 0; t < numGroups; t++ )
   {
      Model::Group * group = Model::Group::get();
      modelGroups.push_back( group );

      uint8_t flags = 0;
      m_src->read( flags );

      char tempstr[32];
      readString( tempstr, sizeof(tempstr) );
      group->m_name = tempstr;

      log_debug( "group name: %s\n", modelGroups[t]->m_name.c_str() );

      uint16_t numTriangles = 0;
      m_src->read( numTriangles );

      uint16_t triIndex = 0;
      for ( uint16_t n = 0; n < numTriangles; n++ )
      {
         m_src->read( triIndex );
         if ( triIndex >= modelTriangles.size() )
         {
            log_error( "triangle out of range: %d/%d\n",
                  triIndex, modelTriangles.size() );
            return Model::ERROR_BAD_DATA;
         }
         group->m_triangleIndices.insert( triIndex );
      }

      uint8_t material = 0;
      m_src->read( material );
      if ( material == 0xFF )
      {
         group->m_materialIndex = -1;
      }
      else
      {
         group->m_materialIndex = material;
      }

      // Already added group to m_groups
   }

   uint16_t numMaterials = 0;
   m_src->read( numMaterials );
   log_debug( "model says %d materials\n", numMaterials );

   for ( t = 0; t < numGroups; t++ )
   {
      if ( modelGroups[t]->m_materialIndex >= numMaterials )
      {
         log_error( "material out of range: %d/%d\n",
               modelGroups[t]->m_materialIndex, numMaterials );
         return Model::ERROR_BAD_DATA;
      }
   }

   for ( t = 0; t < numMaterials; t++ )
   {
      Model::Material * mat = Model::Material::get();
      MS3DMaterial material;

      readString( material.m_name, sizeof(material.m_name) );
      m_src->read( material.m_ambient[0] );
      m_src->read( material.m_ambient[1] );
      m_src->read( material.m_ambient[2] );
      m_src->read( material.m_ambient[3] );
      m_src->read( material.m_diffuse[0] );
      m_src->read( material.m_diffuse[1] );
      m_src->read( material.m_diffuse[2] );
      m_src->read( material.m_diffuse[3] );
      m_src->read( material.m_specular[0] );
      m_src->read( material.m_specular[1] );
      m_src->read( material.m_specular[2] );
      m_src->read( material.m_specular[3] );
      m_src->read( material.m_emissive[0] );
      m_src->read( material.m_emissive[1] );
      m_src->read( material.m_emissive[2] );
      m_src->read( material.m_emissive[3] );
      m_src->read( material.m_shininess );
      m_src->read( material.m_transparency );
      m_src->read( material.m_mode );
      readString( material.m_texture, sizeof( material.m_texture ) );
      readString( material.m_alphamap, sizeof( material.m_alphamap ) );

      log_debug( "material name is %s\n", material.m_name );
      mat->m_name = material.m_name;

      mat->m_ambient[0]  = material.m_ambient[0];
      mat->m_ambient[1]  = material.m_ambient[1];
      mat->m_ambient[2]  = material.m_ambient[2];
      mat->m_ambient[3]  = material.m_ambient[3];
      mat->m_diffuse[0]  = material.m_diffuse[0];
      mat->m_diffuse[1]  = material.m_diffuse[1];
      mat->m_diffuse[2]  = material.m_diffuse[2];
      mat->m_diffuse[3]  = material.m_diffuse[3];
      mat->m_specular[0] = material.m_specular[0];
      mat->m_specular[1] = material.m_specular[1];
      mat->m_specular[2] = material.m_specular[2];
      mat->m_specular[3] = material.m_specular[3];
      mat->m_emissive[0] = material.m_emissive[0];
      mat->m_emissive[1] = material.m_emissive[1];
      mat->m_emissive[2] = material.m_emissive[2];
      mat->m_emissive[3] = material.m_emissive[3];
      mat->m_shininess   = material.m_shininess;

      mat->m_type = Model::Material::MATTYPE_TEXTURE;
      if ( material.m_texture[0] == '\0' )
      {
         mat->m_type = Model::Material::MATTYPE_BLANK;
      }

      replaceBackslash( material.m_texture );
      replaceBackslash( material.m_alphamap );

      // Get absolute path for texture
      string texturePath = material.m_texture;

      texturePath = fixAbsolutePath( modelPath.c_str(), texturePath.c_str() );
      texturePath = getAbsolutePath( modelPath.c_str(), texturePath.c_str() );

      mat->m_filename  = texturePath;

      // Get absolute path for alpha map
      texturePath = material.m_alphamap;

      if ( texturePath.length() > 0 )
      {
         texturePath = fixAbsolutePath( modelPath.c_str(), texturePath.c_str() );
         texturePath = getAbsolutePath( modelPath.c_str(), texturePath.c_str() );
      }

      mat->m_alphaFilename = texturePath;

      modelMaterials.push_back( mat );
   }

   setModelInitialized( model, true );

   float32_t fps = 0;
   m_src->read( fps );

   // don't need this
   float32_t currentTime = 0;
   m_src->read( currentTime );

   int32_t numFrames = 0;
   m_src->read( numFrames );

   setModelNumFrames( model, numFrames );

   if ( numFrames > 0 )
   {
      model->addAnimation( Model::ANIMMODE_SKELETAL, "Keyframe" );
      model->setAnimFPS( Model::ANIMMODE_SKELETAL, 0, fps );
      model->setAnimFrameCount( Model::ANIMMODE_SKELETAL, 0, numFrames );
   }

   uint16_t numJoints = 0;
   m_src->read( numJoints );

   off_t tmpOffset = m_src->offset();
   JointNameListRecT * nameList = new JointNameListRecT[ numJoints ];
   local_array<JointNameListRecT> freeList( nameList );

   for ( t = 0; t < numJoints; t++ )
   {
      MS3DJoint joint;

      m_src->read( joint.m_flags );
      readString( joint.m_name, sizeof( joint.m_name ) );
      readString( joint.m_parentName, sizeof( joint.m_parentName ) );
      m_src->read( joint.m_rotation[0] );
      m_src->read( joint.m_rotation[1] );
      m_src->read( joint.m_rotation[2] );
      m_src->read( joint.m_translation[0] );
      m_src->read( joint.m_translation[1] );
      m_src->read( joint.m_translation[2] );
      m_src->read( joint.m_numRotationKeyframes );
      m_src->read( joint.m_numTranslationKeyframes );

      m_src->seek( m_src->offset() + ( FILE_KEYFRAME_SIZE * (joint.m_numRotationKeyframes + joint.m_numTranslationKeyframes) ) );

      nameList[t].m_jointIndex = t;
      nameList[t].m_name = joint.m_name;
   }

   m_src->seek( tmpOffset );

   for ( t = 0; t < numJoints; t++ )
   {
      log_debug( "Reading joint %d\n", t );
      MS3DJoint joint;

      m_src->read( joint.m_flags );
      readString( joint.m_name, sizeof( joint.m_name ) );
      readString( joint.m_parentName, sizeof( joint.m_parentName ) );
      m_src->read( joint.m_rotation[0] );
      m_src->read( joint.m_rotation[1] );
      m_src->read( joint.m_rotation[2] );
      m_src->read( joint.m_translation[0] );
      m_src->read( joint.m_translation[1] );
      m_src->read( joint.m_translation[2] );
      m_src->read( joint.m_numRotationKeyframes );
      m_src->read( joint.m_numTranslationKeyframes );

      int parentIndex = -1;
      if ( strlen(joint.m_parentName) > 0 )
      {
         for ( uint16_t j = 0; j < numJoints; j++ )
         {
            if ( strcasecmp( nameList[j].m_name.c_str(), joint.m_parentName ) == 0 )
            {
               parentIndex = nameList[j].m_jointIndex;
               break;
            }
         }

         if ( parentIndex == -1 )
         {
            log_error( "No parent\n" );
            return Model::ERROR_BAD_DATA; // no parent!
         }
      }

      modelJoints.push_back( Model::Joint::get() );

      for ( int i = 0; i < 3; i++ )
      {
         modelJoints[t]->m_localRotation[i]    = joint.m_rotation[i];
         modelJoints[t]->m_localTranslation[i] = joint.m_translation[i];
      }
      modelJoints[t]->m_parent = parentIndex;
      modelJoints[t]->m_name   = joint.m_name;

      uint16_t numRotationKeyframes    = joint.m_numRotationKeyframes;
      uint16_t numTranslationKeyframes = joint.m_numTranslationKeyframes;

      log_debug( "Joint %d keyframes: %d rot %d trans\n",
            t, numRotationKeyframes, numTranslationKeyframes );

      uint16_t j; // MS Visual C++ is lame
      for ( j = 0; j < numRotationKeyframes; j++ )
      {
         Model::Keyframe * mkeyframe = Model::Keyframe::get();
         MS3DKeyframe keyframe;

         m_src->read( keyframe.m_time );
         m_src->read( keyframe.m_parameter[0] );
         m_src->read( keyframe.m_parameter[1] );
         m_src->read( keyframe.m_parameter[2] );

         mkeyframe->m_jointIndex = t;
         mkeyframe->m_time = keyframe.m_time;

         for ( int i = 0; i < 3; i++ )
         {
            mkeyframe->m_parameter[i] = keyframe.m_parameter[i];
         }

         unsigned frame = (unsigned) (keyframe.m_time / (1.0 / fps)) - 1;

         model->setSkelAnimKeyframe( 0, frame, t, true,
               mkeyframe->m_parameter[0], mkeyframe->m_parameter[1], mkeyframe->m_parameter[2] );

         mkeyframe->release();
      }
      for ( j = 0; j < numTranslationKeyframes; j++ )
      {
         Model::Keyframe * mkeyframe = Model::Keyframe::get();
         MS3DKeyframe keyframe;

         m_src->read( keyframe.m_time );
         m_src->read( keyframe.m_parameter[0] );
         m_src->read( keyframe.m_parameter[1] );
         m_src->read( keyframe.m_parameter[2] );

         mkeyframe->m_jointIndex = t;
         mkeyframe->m_time = keyframe.m_time;

         for ( int i = 0; i < 3; i++ )
         {
            mkeyframe->m_parameter[i] = keyframe.m_parameter[i];
         }

         unsigned frame = (unsigned) (keyframe.m_time / (1.0 / fps)) - 1;

         model->setSkelAnimKeyframe( 0, frame, t, false,
               mkeyframe->m_parameter[0], mkeyframe->m_parameter[1], mkeyframe->m_parameter[2] );

         mkeyframe->release();
      }
   }

   for ( int i = 0; i < numVertices; i++ )
   {
      model->setVertexBoneJoint( i, vertexJoints[i] );
   }

   bool keepReading = true;
   if ( !m_src->eof() )
   {
      keepReading = readCommentSection();
   }
   if ( keepReading && !m_src->eof() )
   {
      keepReading = readVertexWeightSection();
   }

   // TODO: May want to read joint extra data eventually

   model->setupJoints();

   log_debug( "model loaded\n" );
   log_debug( "  vertices:  %d\n", numVertices );
   log_debug( "  triangles: %d\n", numTriangles );
   log_debug( "  groups:    %d\n", numGroups );
   log_debug( "  materials: %d\n", numMaterials );
   log_debug( "  joints:    %d\n", numJoints );
   log_debug( "\n" );

   return Model::ERROR_NONE;
}

Model::ModelErrorE Ms3dFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o )
{
   LOG_PROFILE();

   if ( model == NULL || filename == NULL || filename[0] == '\0' )
   {
      return Model::ERROR_BAD_ARGUMENT;
   }

   if ( model->getVertexCount() > USHRT_MAX ) {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Too many vertexes for MS3D export (max 65,536)." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   if ( model->getTriangleCount() > USHRT_MAX ) {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Too many faces for MS3D export (max 65,536)." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   if ( model->getGroupCount() > 255 ) {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Too many groups for MS3D export (max 255)." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   if ( model->getTextureCount() > 255 ) {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Too many materials for MS3D export (max 255)." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   if ( model->getBoneJointCount() > 255 ) {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Too many bone joints for MS3D export (max 255)." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   // Check for identical bone joint names
   {
      unsigned c = model->getBoneJointCount();
      for ( unsigned i = 0; i < c; i++ )
      {
         for ( unsigned j = i+1; j < c; j++ )
         {
            if ( strcmp( model->getBoneJointName( i ), model->getBoneJointName( j ) ) == 0 )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Bone joints must have unique names for MS3D export." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }
         }
      }
   }

   // Groups don't share vertices with Milk Shape 3D. Create mesh list
   // that has unique vertices for each group. UVs/normals are per-face
   // vertex rather than per-vertex, so vertices do not have to have
   // unique UVs or normals.

   MeshList ml;
   mesh_create_list( ml, model, Mesh::MO_Group | Mesh::MO_Material );

   if ( mesh_list_vertex_count( ml ) > USHRT_MAX ) {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Too many vertexes for MS3D export (max 65,536) after duplicating vertexes used by multiple groups or materials." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   if ( mesh_list_face_count( ml ) > USHRT_MAX ) {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Too many faces for MS3D export (max 65,536) after duplicating vertexes used by multiple groups or materials." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   Model::ModelErrorE rval = Model::ERROR_NONE;

   // Use dynamic cast to determine if the object is of the proper type
   // If not, create new one that we will be deleted later.
   //
   // We need to create one to make sure that the default options we
   // use in the filter match the default options presented to the
   // user in the dialog box.
   release_ptr<Ms3dOptions> freeOptions = NULL;
   m_options = dynamic_cast< Ms3dOptions *>( o );
   if ( !m_options )
   {
      freeOptions = static_cast<Ms3dOptions *>( getDefaultOptions() );
      m_options = freeOptions.get();
   }
   
   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   m_model = model;

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   DestCloser fc( m_dst );

   if ( err != Model::ERROR_NONE )
      return err;

   vector<Model::Vertex *>   & modelVertices  = getVertexList( model );
   vector<Model::Triangle *> & modelTriangles = getTriangleList( model );
   vector<Model::Group *>    & modelGroups    = getGroupList( model );
   vector<Model::Material *> & modelMaterials = getMaterialList( model );
   vector<Model::Joint *>    & modelJoints    = getJointList( model );

   m_dst->writeBytes( (const uint8_t*) MAGIC_NUMBER, strlen(MAGIC_NUMBER) );
   int32_t version = 4;
   m_dst->write( version );

   uint16_t t;

   // write vertices
   uint16_t numVertices = mesh_list_vertex_count( ml );
   m_dst->write( numVertices );
   log_debug( "writing %d vertices\n", numVertices );

   MeshList::const_iterator it;

   for ( it = ml.begin(); it != ml.end(); it++ )
   {
      for ( t = 0; t < it->vertices.size(); t++ )
      {
         int modelVert = it->vertices[t].v;
         Model::Vertex * mvert = modelVertices[modelVert];
         MS3DVertex vert;
         uint8_t refcount = 0;

         vert.m_flags = 1;
         for ( int n = 0; n < 3; n++ )
         {
            vert.m_vertex[n] = mvert->m_coord[n];
         }
         vert.m_boneId = model->getPrimaryVertexInfluence( modelVert );

         unsigned modelTriangleCount = it->faces.size();
         for ( unsigned tri = 0; tri < modelTriangleCount; tri++ )
         {
            for ( unsigned v = 0; v < 3; v++ )
            {
               if ( it->faces[tri].v[v] == t )
               {
                  refcount++;
               }
            }
         }

         vert.m_refCount = refcount;

         m_dst->write( vert.m_flags );
         m_dst->write( vert.m_vertex[0] );
         m_dst->write( vert.m_vertex[1] );
         m_dst->write( vert.m_vertex[2] );
         m_dst->write( vert.m_boneId );
         m_dst->write( vert.m_refCount );
      }
   }

   // write triangles
   uint16_t numTriangles = (uint16_t) mesh_list_face_count( ml );
   m_dst->write( numTriangles );
   log_debug( "writing %d triangles\n", numTriangles );

   int vertBase = 0;
   int meshNum = 0;
   for ( it = ml.begin(); it != ml.end(); it++ )
   {
      for ( t = 0; t < it->faces.size(); t++ )
      {
         int modelTri = it->faces[t].modelTri;
         Model::Triangle * mtri = modelTriangles[modelTri];
         MS3DTriangle tri;

         tri.m_flags = 1;
         for ( int v = 0; v < 3; v++ )
         {
            tri.m_vertexIndices[v] = it->faces[t].v[v] + vertBase;
            tri.m_s[v] = mtri->m_s[v];
            tri.m_t[v] = 1.0 - mtri->m_t[v];

            for ( int n = 0; n < 3; n++ )
            {
               tri.m_vertexNormals[v][n] = mtri->m_vertexNormals[v][n];
            }
         }

         tri.m_groupIndex = meshNum;
         tri.m_smoothingGroup = 0;

         m_dst->write( tri.m_flags );
         m_dst->write( tri.m_vertexIndices[0] );
         m_dst->write( tri.m_vertexIndices[1] );
         m_dst->write( tri.m_vertexIndices[2] );
         m_dst->write( tri.m_vertexNormals[0][0] );
         m_dst->write( tri.m_vertexNormals[0][1] );
         m_dst->write( tri.m_vertexNormals[0][2] );
         m_dst->write( tri.m_vertexNormals[1][0] );
         m_dst->write( tri.m_vertexNormals[1][1] );
         m_dst->write( tri.m_vertexNormals[1][2] );
         m_dst->write( tri.m_vertexNormals[2][0] );
         m_dst->write( tri.m_vertexNormals[2][1] );
         m_dst->write( tri.m_vertexNormals[2][2] );
         m_dst->write( tri.m_s[0] );
         m_dst->write( tri.m_s[1] );
         m_dst->write( tri.m_s[2] );
         m_dst->write( tri.m_t[0] );
         m_dst->write( tri.m_t[1] );
         m_dst->write( tri.m_t[2] );
         m_dst->write( tri.m_smoothingGroup );
         m_dst->write( tri.m_groupIndex );
      }
      vertBase += it->vertices.size();
      meshNum++;
   }

   // write groups
   uint16_t numGroups = ml.size();
   m_dst->write( numGroups );
   log_debug( "writing %d groups\n", numGroups );

   int triBase = 0;
   for ( it = ml.begin(); it != ml.end(); it++ )
   {
      Model::Group * grp = NULL;
      if ( it->group >= 0 )
      {
         grp = modelGroups[it->group];
      }
      uint8_t flags = 0;
      m_dst->write( flags );

      char groupname[32];
      if ( grp )
         PORT_snprintf( groupname, sizeof(groupname),
               "%s", grp->m_name.c_str() );
      else
         strcpy( groupname, "Ungrouped" );

      m_dst->writeBytes( (const uint8_t*) groupname, sizeof(groupname) );

      uint16_t groupTriangles = it->faces.size();
      m_dst->write( groupTriangles );

      for ( uint16_t n = 0; n < groupTriangles; n++ )
      {
         uint16_t index = n + triBase;
         m_dst->write( index );
      }

      uint8_t material = 0xFF;
      if ( grp && grp->m_materialIndex >= 0 )
         material = grp->m_materialIndex;
      m_dst->write( material );

      triBase += it->faces.size();
   }

   uint16_t numMaterials = modelMaterials.size();
   m_dst->write( numMaterials );
   log_debug( "writing %d materials\n", numMaterials );

   for ( t = 0; t < numMaterials; t++ )
   {
      Model::Material * mmat = modelMaterials[t];
      MS3DMaterial mat;

      PORT_snprintf( mat.m_name, sizeof( mat.m_name ),
            "%s", mmat->m_name.c_str() );

      string texturePath;
      texturePath = getRelativePath( modelPath.c_str(), mmat->m_filename.c_str() );

      PORT_snprintf( mat.m_texture, sizeof( mat.m_texture ),
            "%s", texturePath.c_str() );

      texturePath = getRelativePath( modelPath.c_str(), mmat->m_alphaFilename.c_str() );

      PORT_snprintf( mat.m_alphamap, sizeof( mat.m_alphamap ),
            "%s", texturePath.c_str() );

      replaceSlash( mat.m_texture );
      replaceSlash( mat.m_alphamap );

      for ( int n = 0; n < 4; n++ )
      {
         mat.m_ambient[n]  = mmat->m_ambient[n];
         mat.m_diffuse[n]  = mmat->m_diffuse[n];
         mat.m_specular[n] = mmat->m_specular[n];
         mat.m_emissive[n] = mmat->m_emissive[n];
      }

      mat.m_shininess    = mmat->m_shininess;
      mat.m_transparency = 1.0;
      mat.m_mode = 0;

      m_dst->writeBytes( (const uint8_t*) mat.m_name, sizeof(mat.m_name) );
      m_dst->write( mat.m_ambient[0] );
      m_dst->write( mat.m_ambient[1] );
      m_dst->write( mat.m_ambient[2] );
      m_dst->write( mat.m_ambient[3] );
      m_dst->write( mat.m_diffuse[0] );
      m_dst->write( mat.m_diffuse[1] );
      m_dst->write( mat.m_diffuse[2] );
      m_dst->write( mat.m_diffuse[3] );
      m_dst->write( mat.m_specular[0] );
      m_dst->write( mat.m_specular[1] );
      m_dst->write( mat.m_specular[2] );
      m_dst->write( mat.m_specular[3] );
      m_dst->write( mat.m_emissive[0] );
      m_dst->write( mat.m_emissive[1] );
      m_dst->write( mat.m_emissive[2] );
      m_dst->write( mat.m_emissive[3] );
      m_dst->write( mat.m_shininess );
      m_dst->write( mat.m_transparency );
      m_dst->write( mat.m_mode );
      m_dst->writeBytes( (const uint8_t*) mat.m_texture, sizeof(mat.m_texture) );
      m_dst->writeBytes( (const uint8_t*) mat.m_alphamap, sizeof(mat.m_alphamap) );
   }

   float32_t fps = 30.0f;
   if ( model->getAnimCount( Model::ANIMMODE_SKELETAL ) > 0 )
   {
      fps = model->getAnimFPS( Model::ANIMMODE_SKELETAL, 0 );
   }
   if ( fps <= 0.0 )
   {
      fps = 30.0f;
   }

   float32_t currentTime = 1.0;
   int32_t numFrames = model->getNumFrames();

   double spf = 1.0 / fps;

   m_dst->write( fps );
   m_dst->write( currentTime );
   m_dst->write( numFrames );

   uint16_t numJoints = modelJoints.size();
   m_dst->write( numJoints );

   for ( t = 0; t < numJoints; t++ )
   {
      Model::Joint * mjoint = modelJoints[t];
      MS3DJoint joint;

      PORT_snprintf( joint.m_name, sizeof(joint.m_name),
            "%s", mjoint->m_name.c_str() );

      if ( mjoint->m_parent >= 0 )
      {
         PORT_snprintf( joint.m_parentName, sizeof(joint.m_parentName),
               "%s", modelJoints[ mjoint->m_parent ]->m_name.c_str() );
      }
      else
      {
         joint.m_parentName[0] = '\0';
      }

      joint.m_flags = 8;

      for ( int i = 0; i < 3; i++ )
      {
         joint.m_rotation[i] = mjoint->m_localRotation[i];
         joint.m_translation[i] = mjoint->m_localTranslation[i];
      }

      unsigned animcount = model->getAnimCount( Model::ANIMMODE_SKELETAL );
      unsigned framecount = 0;
      unsigned prevcount = 0;
//#define FIX_MS3D_ANIM_KEYFRAMES // Adds keyframes to beginning and end of animations so they don't affect each other.
                                  // Something is still wrong because animations translation is wrong with this enabled.
#ifdef FIX_MS3D_ANIM_KEYFRAMES
      bool loop;
#endif // FIX_MS3D_ANIM_KEYFRAMES

      unsigned a = 0;
      unsigned f = 0;

      unsigned rotcount   = 0;
      unsigned transcount = 0;

      double x = 0;
      double y = 0;
      double z = 0;

      for ( a = 0; a < animcount; a++ )
      {
         framecount = model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, a );

         for ( f = 0; f < framecount; f++ )
         {
            if ( model->getSkelAnimKeyframe( a, f, t, true, x, y, z )
#ifdef FIX_MS3D_ANIM_KEYFRAMES
				|| ( f == 0 || f == framecount-1 )
#endif
				)
            {
               rotcount++;
            }
            if ( model->getSkelAnimKeyframe( a, f, t, false, x, y, z )
#ifdef FIX_MS3D_ANIM_KEYFRAMES
				|| ( f == 0 || f == framecount-1 )
#endif
				)
            {
               transcount++;
            }
         }
      }

      joint.m_numRotationKeyframes = rotcount;
      joint.m_numTranslationKeyframes = transcount;

      log_debug( "rotation: %d\n", rotcount );
      log_debug( "translation: %d\n", transcount );

      m_dst->write( joint.m_flags );
      m_dst->writeBytes( (const uint8_t*) joint.m_name, sizeof(joint.m_name) );
      m_dst->writeBytes( (const uint8_t*) joint.m_parentName, sizeof(joint.m_parentName) );
      m_dst->write( joint.m_rotation[0] );
      m_dst->write( joint.m_rotation[1] );
      m_dst->write( joint.m_rotation[2] );
      m_dst->write( joint.m_translation[0] );
      m_dst->write( joint.m_translation[1] );
      m_dst->write( joint.m_translation[2] );
      m_dst->write( joint.m_numRotationKeyframes );
      m_dst->write( joint.m_numTranslationKeyframes );

      // Rotation keyframes
      prevcount = 0;
      for ( a = 0; a < animcount; a++ )
      {
         framecount = model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, a );
#ifdef FIX_MS3D_ANIM_KEYFRAMES
         loop = model->getAnimLooping( Model::ANIMMODE_SKELETAL, a );
#endif // FIX_MS3D_ANIM_KEYFRAMES

         for ( f = 0; f < framecount; f++ )
         {
            // force keyframes at beginning and end of animation
            if ( model->getSkelAnimKeyframe( a, f, t, true, x, y, z )
#ifdef FIX_MS3D_ANIM_KEYFRAMES
				|| ( ( f == 0 || f == framecount-1 ) && model->interpSkelAnimKeyframe( a, f, loop, t, true, x, y, z ) )
#endif // FIX_MS3D_ANIM_KEYFRAMES
				)
            {
               MS3DKeyframe keyframe;
               keyframe.m_time = ((double) (prevcount + f + 1)) * spf;
               log_debug( "keyframe time: %f\n", keyframe.m_time );

               keyframe.m_time = keyframe.m_time;
               keyframe.m_parameter[0] = x;
               keyframe.m_parameter[1] = y;
               keyframe.m_parameter[2] = z;

               m_dst->write( keyframe.m_time );
               m_dst->write( keyframe.m_parameter[0] );
               m_dst->write( keyframe.m_parameter[1] );
               m_dst->write( keyframe.m_parameter[2] );
            }
         }

         prevcount += framecount;
      }

      // Translation keyframes
      prevcount = 0;
      for ( a = 0; a < animcount; a++ )
      {
         framecount = model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, a );
#ifdef FIX_MS3D_ANIM_KEYFRAMES
         loop = model->getAnimLooping( Model::ANIMMODE_SKELETAL, a );
#endif // FIX_MS3D_ANIM_KEYFRAMES

         for ( f = 0; f < framecount; f++ )
         {
            // force keyframes at beginning and end of animation
            if ( model->getSkelAnimKeyframe( a, f, t, false, x, y, z )
#ifdef FIX_MS3D_ANIM_KEYFRAMES
				|| ( ( f == 0 || f == framecount-1 ) && model->interpSkelAnimKeyframe( a, f, loop, t, false, x, y, z ) )
#endif // FIX_MS3D_ANIM_KEYFRAMES
				)
            {
               MS3DKeyframe keyframe;
               keyframe.m_time = ((double) (prevcount + f + 1)) * spf;
               log_debug( "keyframe time: %f\n", keyframe.m_time );

               keyframe.m_time = keyframe.m_time;
               keyframe.m_parameter[0] = x;
               keyframe.m_parameter[1] = y;
               keyframe.m_parameter[2] = z;

               m_dst->write( keyframe.m_time );
               m_dst->write( keyframe.m_parameter[0] );
               m_dst->write( keyframe.m_parameter[1] );
               m_dst->write( keyframe.m_parameter[2] );
            }
         }
      }

      prevcount += framecount;
   }

   int32_t subVersion = m_options->m_subVersion;
   if ( subVersion >= 1 )
   {
      // Remember some values in meta data
      char value[128];
      sprintf( value, "%d", subVersion );
      m_model->updateMetaData( "ms3d_sub_version", value );

      if ( subVersion >= 2 )
      {
         sprintf( value, "%x", m_options->m_vertexExtra );
         m_model->updateMetaData( "ms3d_vertex_extra", value );
      }

      if ( subVersion >= 3 )
      {
         sprintf( value, "%x", m_options->m_vertexExtra2 );
         m_model->updateMetaData( "ms3d_vertex_extra2", value );
      }

      writeCommentSection();
      writeVertexWeightSection( ml );

      // TODO joint color
      //writeJointColorSection();
   }

   rval = Model::Model::ERROR_NONE;

   m_options = NULL;

   return rval;
}

void Ms3dFilter::writeCommentSection()
{
   // This is always 1
   int32_t subVersion = 1;
   m_dst->write( subVersion );

   log_debug( "writing comments subversion %d\n", subVersion );

   int32_t numComments = 0;
   m_dst->write( numComments ); // groups
   m_dst->write( numComments ); // materials
   m_dst->write( numComments ); // joints
   m_dst->write( numComments ); // model
}

void Ms3dFilter::writeVertexWeightSection( const MeshList & ml )
{
   int32_t subVersion = m_options->m_subVersion;
   if ( subVersion < 1 || subVersion > 3 )
   {
      subVersion = 1;
   }
   m_dst->write( subVersion );

   log_debug( "writing vertex weights subversion %d\n", subVersion );

   Model::InfluenceList ilist;

   int vwritten = 0;
   MeshList::const_iterator it;
   for ( it = ml.begin(); it != ml.end(); it++ )
   {
      int vcount = it->vertices.size();
      for ( int v = 0; v < vcount; v++ )
      {
         ilist.clear();
         if (!m_model->getVertexInfluences( it->vertices[v].v, ilist ))
            log_error( "get influences failed for vertex %d\n",
                  it->vertices[v].v );

         ilist.sort();
         writeVertexWeight( subVersion, ilist );
         ++vwritten;
      }
   }
}

void Ms3dFilter::writeJointColorSection()
{
   int32_t subVersion = 1;
   m_dst->write( subVersion );

   log_debug( "writing joint color subversion %d\n", subVersion );

   int bcount = m_model->getBoneJointCount();
   for ( int b = 0; b < bcount; b++ )
   {
      for ( int t = 0; t < 3; t++ )
      {
         int ic = 0xff & (m_options->m_jointColor >> (t*8));
         float fc = static_cast<float>(ic) / 255.0;
         m_dst->write( fc );
      }
   }
}

void Ms3dFilter::writeVertexWeight( int subVersion,
      const Model::InfluenceList & ilist )
{
   uint8_t boneId[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
   uint8_t weight[4] = { 0, 0, 0, 0 };
   int rawWeight[4]  = { 0, 0, 0, 0 };

   int totalWeight = 0;

   Model::InfluenceList::const_reverse_iterator it;
   int index = 0;

   for ( it = ilist.rbegin(); index < 4 && it != ilist.rend(); it++ )
   {
      boneId[ index ] = it->m_boneId;
      rawWeight[ index ] = (int) lround( it->m_weight * 100.0 );
      totalWeight += rawWeight[ index ];

      index++;
   }

   int maxWeight = (subVersion == 1) ? 255 : 100;

   index = 0;
   for ( it = ilist.rbegin(); index < 4 && it != ilist.rend(); it++ )
   {
      if ( totalWeight > 0 )
         weight[ index ] = (uint8_t) lround( rawWeight[ index ] * (double) maxWeight
               / (double) totalWeight );
      else
         weight[ index ] = (uint8_t) lround( maxWeight / (double) ilist.size() );

      index++;
   }

   if ( weight[0] + weight[1] + weight[2] + weight[3] != maxWeight ) {
      log_debug( "write weights: %d, %d, %d, %d (total: %d != %d)\n", weight[0], weight[1], weight[2], weight[3], weight[0] + weight[1] + weight[2] + weight[3], maxWeight );
   }

   // Yes, this needs to start at 1 (one), boneId[0] is stored
   // earlier in the file
   m_dst->write( boneId[1] );
   m_dst->write( boneId[2] );
   m_dst->write( boneId[3] );

   // Yes, this needs to start at 0 (zero), weight[3] is implicit
   //log_debug( "write weights: %d, %d, %d\n", weight[0], weight[1], weight[2] );
   m_dst->write( weight[0] );
   m_dst->write( weight[1] );
   m_dst->write( weight[2] );

   if ( subVersion >= 2 )
   {
      uint32_t extra = m_options->m_vertexExtra;
      m_dst->write( extra );
   }
   if ( subVersion >= 3 )
   {
      uint32_t extra2 = m_options->m_vertexExtra2;
      m_dst->write( extra2 );
   }
}

void Ms3dFilter::readString( char * buf, size_t len )
{
   if ( len > 0 )
   {
      m_src->readBytes( (uint8_t *) buf, len );
      buf[ len - 1 ] = '\0';
      log_debug( "read string: %s\n", buf );
   }
}

bool Ms3dFilter::readCommentSection() 
{
   // TODO: We don't actually do anything with these... meta data maybe?
   log_debug( "reading comments section\n");

   int32_t subVersion = 0;
   m_src->read( subVersion );

   log_debug( "  sub version: %d\n", subVersion );

   int32_t numComments = 0;

   for ( int c = CT_GROUP; c < CT_MAX; c++ )
   {
      m_src->read( numComments );
      log_debug( "  comment type %d: %d comments\n", c, numComments );

      for ( int n = 0; n < numComments; n++ )
      {
         int32_t index = 0;
         int32_t length = 0;

         m_src->read( index );
         m_src->read( length );

         log_debug( "    index %d, %d bytes\n", index, length );

         if ( m_src->eof() )
         {
            return false;
         }

         uint8_t * tmp = new uint8_t[length+1];
         m_src->readBytes( tmp, length );
         tmp[length] = '\0';
         std::string comment;
         comment.assign( reinterpret_cast<char*>(tmp), length );
         log_debug( "      comment = %s\n", comment.c_str() );
         delete[] tmp;
      }
   }

   return true;
}

bool Ms3dFilter::readVertexWeightSection() 
{
   log_debug( "reading vertex weight section\n");

   int32_t subVersion = 0;
   m_src->read( subVersion );

   if ( subVersion < 1 || subVersion > 3 )
   {
      log_debug( "  sub version: %d (unknown)\n", subVersion );
      return false;
   }

   char value[128];
   sprintf( value, "%d", subVersion );
   m_model->updateMetaData( "ms3d_sub_version", value );

   log_debug( "  sub version: %d\n", subVersion );

   bool rval = true;
   VertexWeightList weightList;
   VertexWeightList::iterator it;

   int vcount = m_model->getVertexCount();
   for ( int v = 0; rval && v < vcount; v++ )
   {
      //log_debug( "     vertex: %d\n", v );
      rval = readVertexWeight( subVersion, v, weightList );

      if ( rval )
      {
         m_model->removeAllVertexInfluences( v );
         for ( it = weightList.begin(); it != weightList.end(); it++ )
         {
            m_model->addVertexInfluence( v, it->boneId,
                  Model::IT_Custom, ((double) it->weight) / 100.0 );
         }
      }
   }
   return rval;
}

bool Ms3dFilter::readVertexWeight( int subVersion,
      int vertex, VertexWeightList & weightList )
{
   weightList.clear();

   uint8_t boneIds[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
   uint8_t weights[4] = { 0, 0, 0, 0 };

   size_t size[4] = { 0, 6, 10, 14 };
   if ( size[subVersion] > m_src->getRemaining() )
   {
      return false;
   }

   boneIds[0] = m_model->getVertexBoneJoint( vertex );

   int i = 0;
   for ( i = 0; i < 3; i++ )
   {
      m_src->read( boneIds[i+1] );
      //log_debug( "      read boneId %d\n", boneIds[i+1] );
   }
   for ( i = 0; i < 3; i++ )
   {
      m_src->read( weights[i] );
      if ( subVersion == 1 )
      {
         weights[i] = (uint8_t) ((int) weights[i] * 100 / 255);
      }
      //log_debug( "      read weights %d\n", weights[i] );
   }
   if ( subVersion >= 2 )
   {
      int32_t extra = 0;
      m_src->read( extra ); // don't do anything with this
   }
   if ( subVersion >= 3 )
   {
      int32_t extra2 = 0;
      m_src->read( extra2 ); // don't do anything with this
   }

   //log_debug( "      vtx %d: b %d, %d, %d, %d; w %d, %d, %d, %d\n", vertex,
   //   (int)boneIds[0], (int)boneIds[1], (int)boneIds[2], (int)boneIds[3],
   //   (int)weights[0], (int)weights[1], (int)weights[2], (int)weights[3] );

   VertexWeightT vw;

   int total = 0;
   for ( i = 0; i < 4 && boneIds[i] != 0xFF; i++ )
   {
      vw.boneId = boneIds[i];
      if ( i < 3 )
      {
         vw.weight = weights[i];
         total += weights[i];
      }
      else
      {
         int diff = 100 - total;
         vw.weight = (diff > 0) ? diff : 0;
      }
      weightList.push_back( vw );
   }

   return true;
}

bool Ms3dFilter::isSupported( const char * filename )
{
   if ( filename )
   {
      unsigned len = strlen( filename );

      if ( len >= 5 && strcasecmp( &filename[len-5], ".ms3d" ) == 0 )
      {
         return true;
      }
   }

   return false;
}

list<string> Ms3dFilter::getReadTypes()
{
   list<string> rval;

   rval.push_back( "*.ms3d" );

   return rval;
}

list<string> Ms3dFilter::getWriteTypes()
{
   list<string> rval;

   rval.push_back( "*.ms3d" );

   return rval;
}

