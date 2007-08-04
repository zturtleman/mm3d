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

#include "ms3dfilter.h"

#include "weld.h"
#include "misc.h"
#include "log.h"
#include "endianconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

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

Model::ModelErrorE Ms3dFilter::readFile( Model * model, const char * const filename )
{
   LOG_PROFILE();

   if ( model && filename )
   {
      FILE * fp = fopen( filename, "rb" );

      if ( fp == NULL )
      {
         if ( errno == ENOENT )
         {
            log_error( "%s: file does not exist\n", filename );
            return Model::ERROR_NO_FILE;
         }
         if ( errno == EPERM )
         {
            log_error( "%s: access denied\n", filename );
            return Model::ERROR_NO_ACCESS;
         }

         log_error( "%s: could not open file\n", filename );
         return Model::ERROR_FILE_OPEN;
      }

      string modelPath = "";
      string modelBaseName = "";
      string modelFullName = "";

      normalizePath( filename, modelFullName, modelPath, modelBaseName );
      
      model->setFilename( modelFullName.c_str() );

      vector<Model::Vertex *>   & modelVertices  = getVertexList( model );
      vector<Model::Triangle *> & modelTriangles = getTriangleList( model );
      vector<Model::Group *>    & modelGroups    = getGroupList( model );
      vector<Model::Material *> & modelMaterials = getMaterialList( model );
      vector<Model::Joint *>    & modelJoints    = getJointList( model );

      fseek( fp, 0, SEEK_END );
      unsigned fileLength = ftell( fp );
      fseek( fp, 0, SEEK_SET );

      // read whole file into memory
      uint8_t *buffer = new uint8_t[fileLength];

      if ( fread( buffer, fileLength, 1, fp ) != 1 )
      {
         delete[] buffer;
         log_error( "%s: could not read file\n", filename );
         fclose( fp );
         return Model::ERROR_FILE_READ;
      }

      fclose( fp );

      m_model  = model;
      m_buffer = buffer;
      m_bufPos = buffer;
      m_bufEnd = buffer + fileLength;

      if ( fileLength < (sizeof( MS3DHeader ) + sizeof(uint16_t) ) )
      {
         delete[] buffer;
         return Model::ERROR_UNEXPECTED_EOF;
      }

      // Check header
      MS3DHeader header;
      readBytes( header.m_ID, sizeof(header.m_ID) );
      read( header.m_version );

      if ( strncmp( header.m_ID, MAGIC_NUMBER, 10 ) != 0 )
      {
         delete[] buffer;
         log_error( "bad magic number\n" );
         return Model::ERROR_BAD_MAGIC;
      }

      int version = header.m_version;
      if ( version < 3 || version > 4 )
      {
         delete[] buffer;
         log_error( "unsupported version\n" );
         return Model::ERROR_UNSUPPORTED_VERSION;
      }

      uint16_t t; // MS Visual C++ is lame

      uint16_t numVertices = 0;
      read( numVertices );

      // TODO verify file size vs. numVertices

      std::vector< int > vertexJoints;
      for ( t = 0; t < numVertices; t++ )
      {
         MS3DVertex vertex;
         read( vertex.m_flags );
         read( vertex.m_vertex[0] );
         read( vertex.m_vertex[1] );
         read( vertex.m_vertex[2] );
         read( vertex.m_boneId );
         read( vertex.m_refCount );

         Model::Vertex * vert = Model::Vertex::get();
         for ( int v = 0; v < 3; v++ )
         {
            vert->m_coord[v] = vertex.m_vertex[ v ];
         }
         modelVertices.push_back( vert );
         vertexJoints.push_back( vertex.m_boneId );
      }

      uint16_t numTriangles = 0;
      read( numTriangles );

      if ( (fileLength - ( m_bufPos - buffer )) < (FILE_TRIANGLE_SIZE * numTriangles + sizeof( uint16_t )) )
      {
         delete[] buffer;
         return Model::ERROR_UNEXPECTED_EOF;
      }

      for ( t = 0; t < numTriangles; t++ )
      {
         Model::Triangle * curTriangle = Model::Triangle::get();
         MS3DTriangle triangle;
         read( triangle.m_flags );
         read( triangle.m_vertexIndices[0] );
         read( triangle.m_vertexIndices[1] );
         read( triangle.m_vertexIndices[2] );
         read( triangle.m_vertexNormals[0][0] );
         read( triangle.m_vertexNormals[0][1] );
         read( triangle.m_vertexNormals[0][2] );
         read( triangle.m_vertexNormals[1][0] );
         read( triangle.m_vertexNormals[1][1] );
         read( triangle.m_vertexNormals[1][2] );
         read( triangle.m_vertexNormals[2][0] );
         read( triangle.m_vertexNormals[2][1] );
         read( triangle.m_vertexNormals[2][2] );
         read( triangle.m_s[0] );
         read( triangle.m_s[1] );
         read( triangle.m_s[2] );
         read( triangle.m_t[0] );
         read( triangle.m_t[1] );
         read( triangle.m_t[2] );
         read( triangle.m_smoothingGroup );
         read( triangle.m_groupIndex );

         curTriangle->m_vertexIndices[0] = triangle.m_vertexIndices[0];
         curTriangle->m_vertexIndices[1] = triangle.m_vertexIndices[1];
         curTriangle->m_vertexIndices[2] = triangle.m_vertexIndices[2];

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
      read( numGroups );

      log_debug( "model says %d groups\n" );

      for ( t = 0; t < numGroups; t++ )
      {
         Model::Group * group = Model::Group::get();
         modelGroups.push_back( group );

         uint8_t flags = 0;
         read( flags );

         char tempstr[32];
         readString( tempstr, sizeof(tempstr) );
         group->m_name = tempstr;

         log_debug( "group name: %s\n", modelGroups[t]->m_name.c_str() );

         uint16_t numTriangles = 0;
         read( numTriangles );

         uint16_t triIndex = 0;
         for ( uint16_t n = 0; n < numTriangles; n++ )
         {
            read( triIndex );
            group->m_triangleIndices.push_back( triIndex );
         }

         int8_t material = 0;
         read( material );
         group->m_materialIndex = material;

         // Already added group to m_groups
      }

      uint16_t numMaterials = 0;
      read( numMaterials );
      log_debug( "model says %d materials\n", numMaterials );

      for ( t = 0; t < numMaterials; t++ )
      {
         Model::Material * mat = Model::Material::get();
         MS3DMaterial material;

         readString( material.m_name, sizeof(material.m_name) );
         read( material.m_ambient[0] );
         read( material.m_ambient[1] );
         read( material.m_ambient[2] );
         read( material.m_ambient[3] );
         read( material.m_diffuse[0] );
         read( material.m_diffuse[1] );
         read( material.m_diffuse[2] );
         read( material.m_diffuse[3] );
         read( material.m_specular[0] );
         read( material.m_specular[1] );
         read( material.m_specular[2] );
         read( material.m_specular[3] );
         read( material.m_emissive[0] );
         read( material.m_emissive[1] );
         read( material.m_emissive[2] );
         read( material.m_emissive[3] );
         read( material.m_shininess );
         read( material.m_transparency );
         read( material.m_mode );
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
      read( fps );

      // don't need this
      float32_t currentTime = 0;
      read( currentTime );

      int32_t numFrames = 0;
      read( numFrames );

      setModelNumFrames( model, numFrames );

      if ( numFrames > 0 )
      {
         model->addAnimation( Model::ANIMMODE_SKELETAL, "Keyframe" );
         model->setAnimFPS( Model::ANIMMODE_SKELETAL, 0, fps );
         model->setAnimFrameCount( Model::ANIMMODE_SKELETAL, 0, numFrames );
      }

      uint16_t numJoints = 0;
      read( numJoints );

      struct _JointNameListRec_t
      {
         int m_jointIndex;
         std::string m_name;
      };

      typedef struct _JointNameListRec_t JointNameListRecT;

      uint8_t * tempPtr = m_bufPos;
      JointNameListRecT * nameList = new JointNameListRecT[ numJoints ];
      for ( t = 0; t < numJoints; t++ )
      {
         MS3DJoint joint;

         read( joint.m_flags );
         readString( joint.m_name, sizeof( joint.m_name ) );
         readString( joint.m_parentName, sizeof( joint.m_parentName ) );
         read( joint.m_rotation[0] );
         read( joint.m_rotation[1] );
         read( joint.m_rotation[2] );
         read( joint.m_translation[0] );
         read( joint.m_translation[1] );
         read( joint.m_translation[2] );
         read( joint.m_numRotationKeyframes );
         read( joint.m_numTranslationKeyframes );

         skipBytes( FILE_KEYFRAME_SIZE * (joint.m_numRotationKeyframes + joint.m_numTranslationKeyframes) );

         nameList[t].m_jointIndex = t;
         nameList[t].m_name = joint.m_name;
      }

      m_bufPos = tempPtr;

      for ( t = 0; t < numJoints; t++ )
      {
         MS3DJoint joint;

         read( joint.m_flags );
         readString( joint.m_name, sizeof( joint.m_name ) );
         readString( joint.m_parentName, sizeof( joint.m_parentName ) );
         read( joint.m_rotation[0] );
         read( joint.m_rotation[1] );
         read( joint.m_rotation[2] );
         read( joint.m_translation[0] );
         read( joint.m_translation[1] );
         read( joint.m_translation[2] );
         read( joint.m_numRotationKeyframes );
         read( joint.m_numTranslationKeyframes );

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
               delete[] buffer;
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

         uint16_t j; // MS Visual C++ is lame
         for ( j = 0; j < numRotationKeyframes; j++ )
         {
            Model::Keyframe * mkeyframe = Model::Keyframe::get();
            MS3DKeyframe keyframe;

            read( keyframe.m_time );
            read( keyframe.m_parameter[0] );
            read( keyframe.m_parameter[1] );
            read( keyframe.m_parameter[2] );

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

            read( keyframe.m_time );
            read( keyframe.m_parameter[0] );
            read( keyframe.m_parameter[1] );
            read( keyframe.m_parameter[2] );

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
      log_debug( "model loaded\n" );
      log_debug( "  vertices:  %d\n", numVertices );
      log_debug( "  triangles: %d\n", numTriangles );
      log_debug( "  groups:    %d\n", numGroups );
      log_debug( "  materials: %d\n", numMaterials );
      log_debug( "  joints:    %d\n", numJoints );
      log_debug( "\n" );

      delete[] nameList;

      model->setupJoints();

      m_bufPos = NULL;
      delete[] buffer;

      return Model::ERROR_NONE;
   }
   else
   {
      return Model::ERROR_BAD_ARGUMENT;
   }
}

Model::ModelErrorE Ms3dFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o )
{
   LOG_PROFILE();

   if ( model == NULL || filename == NULL || filename[0] == '\0' )
   {
      return Model::ERROR_BAD_ARGUMENT;
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
               model->setFilterSpecificError( "Bone joints must have unique names." );
               return Model::ERROR_FILTER_SPECIFIC;
            }
         }
      }
   }

   m_fp = fopen( filename, "wb" );

   if ( m_fp )
   {
      string modelPath = "";
      string modelBaseName = "";
      string modelFullName = "";

      normalizePath( filename, modelFullName, modelPath, modelBaseName );

      m_model = model;

      // Groups don't share vertices with Milk Shape 3D
      {
         unsigned t = 0;
         unsigned vcount = model->getVertexCount();

         for ( t = 0; t < vcount; t++ )
         {
            model->selectVertex( t );
         }

         unweldSelectedVertices( model );

         unsigned tcount = model->getTriangleCount();
         for ( t = 0; t < tcount; t++ )
         {
            model->selectTriangle( t );
         }

         unsigned gcount = model->getGroupCount();
         for ( t = 0; t < gcount; t++ )
         {
            model->unselectGroup( t );
         }

         list<int> tris;
         model->getSelectedTriangles( tris );
         if ( !tris.empty() )
         {
            weldSelectedVertices( model );
            int ungrouped = model->addGroup( "Ungrouped" );
            model->addSelectedToGroup( ungrouped );
         }

         // gcount doesn't include the group we just created,
         // so this operation does not re-weld vertices we just 
         // operated on.
         for ( t = 0; t < gcount; t++ )
         {
            model->unselectAll();
            model->selectGroup( t );
            weldSelectedVertices( model );
         }

         model->unselectAll();
         model->operationComplete( "Changes for save" );
      }
      
      vector<Model::Vertex *>   & modelVertices  = getVertexList( model );
      vector<Model::Triangle *> & modelTriangles = getTriangleList( model );
      vector<Model::Group *>    & modelGroups    = getGroupList( model );
      vector<Model::Material *> & modelMaterials = getMaterialList( model );
      vector<Model::Joint *>    & modelJoints    = getJointList( model );

      writeBytes( MAGIC_NUMBER, strlen(MAGIC_NUMBER) );
      int32_t version = 4;
      write( version );

      uint16_t t;

      // write vertices
      uint16_t numVertices = (int) modelVertices.size();
      write( numVertices );

      for ( t = 0; t < numVertices; t++ )
      {
         Model::Vertex * mvert = modelVertices[t];
         MS3DVertex vert;
         uint8_t refcount = 0;

         vert.m_flags = 1;
         for ( int n = 0; n < 3; n++ )
         {
            vert.m_vertex[n] = mvert->m_coord[n];
         }
         vert.m_boneId = model->getPrimaryVertexInfluence( t );

         for ( unsigned tri = 0; tri < modelTriangles.size(); tri++ )
         {
            for ( unsigned v = 0; v < 3; v++ )
            {
               if ( modelTriangles[tri]->m_vertexIndices[v] == (unsigned) t )
               {
                  refcount++;
               }
            }
         }

         vert.m_refCount = refcount;

         write( vert.m_flags );
         write( vert.m_vertex[0] );
         write( vert.m_vertex[1] );
         write( vert.m_vertex[2] );
         write( vert.m_boneId );
         write( vert.m_refCount );
      }

      // write triangles
      uint16_t numTriangles = (int) modelTriangles.size();
      write( numTriangles );

      for ( t = 0; t < numTriangles; t++ )
      {
         Model::Triangle * mtri = modelTriangles[t];
         MS3DTriangle tri;

         tri.m_flags = 1;
         for ( int v = 0; v < 3; v++ )
         {
            tri.m_vertexIndices[v] = mtri->m_vertexIndices[v];
            tri.m_s[v] = mtri->m_s[v];
            tri.m_t[v] = 1.0 - mtri->m_t[v];

            for ( int n = 0; n < 3; n++ )
            {
               tri.m_vertexNormals[v][n] = mtri->m_vertexNormals[v][n];
            }
         }

         for ( unsigned g = 0; g < modelGroups.size(); g++ )
         {
            for ( unsigned n = 0; n < modelGroups[g]->m_triangleIndices.size(); n++ )
            {
               if ( modelGroups[g]->m_triangleIndices[n] == t )
               {
                  tri.m_groupIndex = g;
                  break;
               }
            }
         }
         tri.m_smoothingGroup = 0;

         write( tri.m_flags );
         write( tri.m_vertexIndices[0] );
         write( tri.m_vertexIndices[1] );
         write( tri.m_vertexIndices[2] );
         write( tri.m_vertexNormals[0][0] );
         write( tri.m_vertexNormals[0][1] );
         write( tri.m_vertexNormals[0][2] );
         write( tri.m_vertexNormals[1][0] );
         write( tri.m_vertexNormals[1][1] );
         write( tri.m_vertexNormals[1][2] );
         write( tri.m_vertexNormals[2][0] );
         write( tri.m_vertexNormals[2][1] );
         write( tri.m_vertexNormals[2][2] );
         write( tri.m_s[0] );
         write( tri.m_s[1] );
         write( tri.m_s[2] );
         write( tri.m_t[0] );
         write( tri.m_t[1] );
         write( tri.m_t[2] );
         write( tri.m_smoothingGroup );
         write( tri.m_groupIndex );
      }

      uint16_t numGroups = modelGroups.size();
      write( numGroups );

      for ( t = 0; t < numGroups; t++ )
      {
         Model::Group * grp = modelGroups[t];
         uint8_t flags = 0;
         write( flags );

         char groupname[32];
         strncpy( groupname, grp->m_name.c_str(), sizeof(groupname) );
         writeString( groupname, sizeof(groupname) );

         uint16_t numTriangles = grp->m_triangleIndices.size();
         write( numTriangles );

         for ( uint16_t n = 0; n < numTriangles; n++ )
         {
            uint16_t index = grp->m_triangleIndices[n];
            write( index );
         }

         uint8_t material = grp->m_materialIndex;
         write( material );
      }

      uint16_t numMaterials = modelMaterials.size();
      write( numMaterials );

      for ( t = 0; t < numMaterials; t++ )
      {
         Model::Material * mmat = modelMaterials[t];
         MS3DMaterial mat;

         strncpy( mat.m_name, mmat->m_name.c_str(), sizeof( mat.m_name ) );

         string texturePath;
         texturePath = getRelativePath( modelPath.c_str(), mmat->m_filename.c_str() );

         strncpy( mat.m_texture, texturePath.c_str(), sizeof( mat.m_texture ) );
         mat.m_texture[ sizeof(mat.m_texture) - 1 ]  = '\0';

         texturePath = getRelativePath( modelPath.c_str(), mmat->m_alphaFilename.c_str() );

         strncpy( mat.m_alphamap, texturePath.c_str(), sizeof( mat.m_alphamap ) );
         mat.m_alphamap[ sizeof(mat.m_alphamap) - 1 ]  = '\0';

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

         writeString( mat.m_name, sizeof(mat.m_name) );
         write( mat.m_ambient[0] );
         write( mat.m_ambient[1] );
         write( mat.m_ambient[2] );
         write( mat.m_ambient[3] );
         write( mat.m_diffuse[0] );
         write( mat.m_diffuse[1] );
         write( mat.m_diffuse[2] );
         write( mat.m_diffuse[3] );
         write( mat.m_specular[0] );
         write( mat.m_specular[1] );
         write( mat.m_specular[2] );
         write( mat.m_specular[3] );
         write( mat.m_emissive[0] );
         write( mat.m_emissive[1] );
         write( mat.m_emissive[2] );
         write( mat.m_emissive[3] );
         write( mat.m_shininess );
         write( mat.m_transparency );
         write( mat.m_mode );
         writeString( mat.m_texture, sizeof(mat.m_texture) );
         writeString( mat.m_alphamap, sizeof(mat.m_alphamap) );
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

      write( fps );
      write( currentTime );
      write( numFrames );

      uint16_t numJoints = modelJoints.size();
      write( numJoints );

      for ( t = 0; t < numJoints; t++ )
      {
         Model::Joint * mjoint = modelJoints[t];
         MS3DJoint joint;

         strncpy( joint.m_name, mjoint->m_name.c_str(), sizeof(joint.m_name) );

         if ( mjoint->m_parent >= 0 )
         {
            strncpy( joint.m_parentName, modelJoints[ mjoint->m_parent ]->m_name.c_str(), sizeof(joint.m_parentName) );
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
               if ( model->getSkelAnimKeyframe( a, f, t, true, x, y, z ) )
               {
                  rotcount++;
               }
               if ( model->getSkelAnimKeyframe( a, f, t, false, x, y, z ) )
               {
                  transcount++;
               }
            }
         }

         joint.m_numRotationKeyframes = rotcount;
         joint.m_numTranslationKeyframes = transcount;

         log_debug( "rotation: %d\n", rotcount );
         log_debug( "translation: %d\n", transcount );

         write( joint.m_flags );
         writeString( joint.m_name, sizeof(joint.m_name) );
         writeString( joint.m_parentName, sizeof(joint.m_parentName) );
         write( joint.m_rotation[0] );
         write( joint.m_rotation[1] );
         write( joint.m_rotation[2] );
         write( joint.m_translation[0] );
         write( joint.m_translation[1] );
         write( joint.m_translation[2] );
         write( joint.m_numRotationKeyframes );
         write( joint.m_numTranslationKeyframes );

         // Rotation keyframes
         prevcount = 0;
         for ( a = 0; a < animcount; a++ )
         {
            framecount = model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, a );

            for ( f = 0; f < framecount; f++ )
            {
               if ( model->getSkelAnimKeyframe( a, f, t, true, x, y, z ) )
               {
                  MS3DKeyframe keyframe;
                  keyframe.m_time = ((double) (prevcount + f + 1)) * spf;
                  log_debug( "keyframe time: %f\n", keyframe.m_time );

                  keyframe.m_time = keyframe.m_time;
                  keyframe.m_parameter[0] = x;
                  keyframe.m_parameter[1] = y;
                  keyframe.m_parameter[2] = z;

                  write( keyframe.m_time );
                  write( keyframe.m_parameter[0] );
                  write( keyframe.m_parameter[1] );
                  write( keyframe.m_parameter[2] );
               }
            }

            prevcount += framecount;
         }

         // Translation keyframes
         prevcount = 0;
         for ( a = 0; a < animcount; a++ )
         {
            framecount = model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, a );

            for ( f = 0; f < framecount; f++ )
            {
               if ( model->getSkelAnimKeyframe( a, f, t, false, x, y, z ) )
               {
                  MS3DKeyframe keyframe;
                  keyframe.m_time = ((double) (prevcount + f + 1)) * spf;
                  log_debug( "keyframe time: %f\n", keyframe.m_time );

                  keyframe.m_time = keyframe.m_time;
                  keyframe.m_parameter[0] = x;
                  keyframe.m_parameter[1] = y;
                  keyframe.m_parameter[2] = z;

                  write( keyframe.m_time );
                  write( keyframe.m_parameter[0] );
                  write( keyframe.m_parameter[1] );
                  write( keyframe.m_parameter[2] );
               }
            }
         }

         prevcount += framecount;
      }

      fclose( m_fp );
      m_fp = NULL;
   }
   else
   {
      if ( errno == ENOENT )
      {
         log_error( "%s: file does not exist\n", filename );
         return Model::ERROR_NO_FILE;
      }
      if ( errno == EPERM )
      {
         log_error( "%s: access denied\n", filename );
         return Model::ERROR_NO_ACCESS;
      }

      log_error( "%s: could not open file\n", filename );
      return Model::ERROR_FILE_OPEN;
   }

   return Model::Model::ERROR_NONE;
}

void Ms3dFilter::write( uint32_t val )
{
   val = htol_u32( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void Ms3dFilter::write( uint16_t val )
{
   val = htol_u16( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void Ms3dFilter::write( uint8_t val )
{
   fwrite( &val, sizeof(val), 1, m_fp );
}

void Ms3dFilter::write( int32_t val )
{
   val = htol_32( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void Ms3dFilter::write( int16_t val )
{
   val = htol_16( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void Ms3dFilter::write( int8_t val )
{
   fwrite( &val, sizeof(val), 1, m_fp );
}

void Ms3dFilter::write( float32_t val )
{
   val = htol_float( val );
   fwrite( &val, sizeof(val), 1, m_fp );
}

void Ms3dFilter::writeBytes( const void * buf, size_t len )
{
   fwrite( buf, len, 1, m_fp );
}

void Ms3dFilter::writeString( const char * buf, size_t len )
{
   fwrite( buf, len - 1, 1, m_fp );

   // be polite, make sure our strings are null-terminated
   uint8_t ch = 0;
   fwrite( &ch, sizeof(ch), 1, m_fp );
}

void Ms3dFilter::writeCommentSection()
{
   // TODO: get from meta data?
   int32_t subVersion = 1;
   write( subVersion );

   int32_t numComments = 0;
   write( numComments ); // groups
   write( numComments ); // materials
   write( numComments ); // joints
   write( numComments ); // model
}

void Ms3dFilter::writeVertexWeightSection()
{
   // We don't support vertex colors or joint colors, so we might as
   // well just use sub version 1
   int32_t subVersion = 1;
   write( subVersion );

   int vcount = m_model->getVertexCount();
   for ( int v = 0; v < vcount; v++ )
   {
      Model::InfluenceList ilist;
      ilist.clear();
      m_model->getVertexInfluences( v, ilist );
      writeVertexWeight( ilist );
   }
}

void Ms3dFilter::writeVertexWeight( Model::InfluenceList & ilist )
{
   // FIXME implement
   // FIXME need to sort list from least to greatest
   // FIXME support sub version 2?
   int8_t boneId[4]  = { -1, -1, -1, -1 };
   uint8_t weight[4] = { 0, 0, 0, 0 };
   int rawWeight[4]  = { 0, 0, 0, 0 };

   int totalWeight = 0;

   Model::InfluenceList::iterator it;
   int index = 0;

   for ( it = ilist.begin(); index < 4 && it != ilist.end(); it++ )
   {
      totalWeight += static_cast<int>(it->m_weight * 100.0);
      boneId[ index ] = it->m_boneId;
      rawWeight[ index ] = static_cast<int>(it->m_weight * 100.0);

      index++;
   }

   index = 0;
   for ( it = ilist.begin(); index < 4 && it != ilist.end(); it++ )
   {
      if ( totalWeight > 0 )
         weight[ index ] = (uint8_t) (rawWeight[ index ] * 255.0
               / (double) totalWeight);
      else
         weight[ index ] = 255 / ilist.size();
      index++;
   }
}

void Ms3dFilter::read( uint32_t & val )
{
   val = ltoh_u32( * (uint32_t*) m_bufPos );
   m_bufPos += sizeof( uint32_t );
}

void Ms3dFilter::read( uint16_t & val )
{
   val = ltoh_u16( * (uint16_t*) m_bufPos );
   m_bufPos += sizeof( uint16_t );
}

void Ms3dFilter::read( uint8_t & val )
{
   val = * (uint8_t*) m_bufPos;
   m_bufPos += sizeof( uint8_t );
}

void Ms3dFilter::read( int32_t & val )
{
   val = ltoh_32( * (int32_t*) m_bufPos );
   m_bufPos += sizeof( int32_t );
}

void Ms3dFilter::read( int16_t & val )
{
   val = ltoh_16( * (int16_t*) m_bufPos );
   m_bufPos += sizeof( int16_t );
}

void Ms3dFilter::read( int8_t & val )
{
   val = * (int8_t*) m_bufPos;
   m_bufPos += sizeof( int8_t );
}

void Ms3dFilter::read( float32_t & val )
{
   val = ltoh_float( * (float32_t*) m_bufPos );
   m_bufPos += sizeof( float32_t );
}

void Ms3dFilter::readBytes( void * buf, size_t len )
{
   memcpy( buf, m_bufPos, len );
   m_bufPos += len;
}

void Ms3dFilter::skipBytes( size_t len )
{
   m_bufPos += len;
}

void Ms3dFilter::readString( char * buf, size_t len )
{
   if ( len > 0 )
   {
      readBytes( buf, len );
      buf[ len - 1 ] = '\0';
      log_debug( "read string: %s\n", buf );
   }
}

bool Ms3dFilter::readCommentSection() 
{
   // TODO: We don't actually do anything with these... meta data maybe?
   int32_t subVersion = 0;
   read( subVersion );

   int32_t numComments = 0;

   for ( int c = CT_GROUP; c < CT_MAX; c++ )
   {
      read( numComments );
      for ( int n = 0; n < numComments; n++ )
      {
         int32_t index = 0;
         int32_t length = 0;

         read( index );
         read( length );

         if (length > (m_bufEnd - m_bufPos))
         {
            return false;
         }

         std::string comment;
         comment.assign( reinterpret_cast<char*>(m_bufPos), length );
         m_bufPos += length;
      }
   }

   return true;
}

bool Ms3dFilter::readVertexWeightSection() 
{
   int32_t subVersion = 0;
   read( subVersion );

   bool rval = true;
   VertexWeightList weightList;
   VertexWeightList::iterator it;

   int vcount = m_model->getVertexCount();
   for ( int v = 0; rval && v < vcount; v++ )
   {
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

   int8_t boneIds[4]  = { -1, -1, -1, -1 };
   uint8_t weights[4] = {  0,  0,  0,  0 };

   int size = (subVersion == 2) ? 10 : 6;
   if ( size > (m_bufEnd - m_bufPos) )
   {
      return false;
   }

   boneIds[0] = m_model->getVertexBoneJoint( vertex );

   int i = 0;
   for ( i = 0; i < 3; i++ )
   {
      read( boneIds[i+1] );
   }
   for ( i = 0; i < 3; i++ )
   {
      read( weights[i] );
      if ( subVersion == 1 )
      {
         weights[i] = (uint8_t) ((int) weights[i] * 100 / 255);
      }
   }
   if ( subVersion == 2 )
   {
      int32_t extra = 0;
      read( extra ); // don't do anything with this
   }

   VertexWeightT vw;

   int total = 0;
   for ( i = 0; boneIds[i] >= 0 && i < 4; i++ )
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

