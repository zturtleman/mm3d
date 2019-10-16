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

//
// GameMaker Studio D3D model format
//
// TODO: Support importing triangle strip and triangle fan primitives.
//

#include "d3dfilter.h"

#include "texture.h"
#include "texmgr.h"
#include "misc.h"
#include "log.h"
#include "translate.h"
#include "modelstatus.h"
#include "mesh.h"

#include "mm3dport.h"
#include "datasource.h"
#include "datadest.h"
#include "release_ptr.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include <string>

using std::string;

D3dFilter::D3dFilter()
{
}

D3dFilter::~D3dFilter()
{
}

Model::ModelErrorE D3dFilter::readFile( Model * model, const char * const filename )
{
   Model::ModelErrorE err = Model::ERROR_NONE;
   DataSource *src = openInput( filename, err );
   SourceCloser fc( src );

   if ( err != Model::ERROR_NONE )
      return err;

   Matrix loadMatrix;
   loadMatrix.setRotationInDegrees( -90, -90, 0 );

   std::string modelPath = "";
   std::string modelBaseName = "";
   std::string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   model->setFilename( modelFullName.c_str() );

   int version, directivesCount;
   char line[1024];

   if ( src->readLine( line, sizeof(line) ) )
   {
      line[ sizeof( line ) - 1 ] = '\0';
      version = atoi( line );

      if ( version != 100 )
      {
         model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Unsupported D3D version." ) ).c_str() ); // TODO: Display version in error.
         return Model::ERROR_FILTER_SPECIFIC;
      }
   }
   else
   {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Missing version in D3D model." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   if ( src->readLine( line, sizeof(line) ) )
   {
      line[ sizeof( line ) - 1 ] = '\0';
      directivesCount = atoi( line );

      if ( directivesCount <= 0 )
      {
         model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Invalid D3D directives count." ) ).c_str() ); // TODO: Display directives count in error.
         return Model::ERROR_FILTER_SPECIFIC;
      }
   }
   else
   {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Missing line count in D3D model." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   int numLines = 0;
   int primitiveType = 0;
   int group = -1;

   vector<Model::Vertex *> & modelVertices = getVertexList( model );
   vector<std::array<double,3>> normalList;

   #define MAX_PRIMITIVE_VERTS 3
   int unprocessedVerts = 0;
   int vertexIndices[MAX_PRIMITIVE_VERTS];
   double texCoords[MAX_PRIMITIVE_VERTS][2];

   while ( src->readLine( line, sizeof(line) ) )
   {
      line[ sizeof( line ) - 1 ] = '\0';
      numLines++;

      int directive;
      float arg[10];
      if ( sscanf( line, "%d %f %f %f %f %f %f %f %f %f %f", &directive, &arg[0], &arg[1], &arg[2], &arg[3], &arg[4], &arg[5], &arg[6], &arg[7], &arg[8], &arg[9] ) != 11 )
      {
         model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Too few tokens on line in D3D model." ) ).c_str() );
         return Model::ERROR_FILTER_SPECIFIC;
      }

      bool createVertex = false;
      double origin[3] = { 0, 0, 0 };
      double normal[3] = { 0, 0, 0 };
      double uv[2] = { 0, 0 };

      switch ( directive )
      {
         case 0: // d3d_model_primitive_begin
            if ( primitiveType != 0 )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Found primitive start without primitive end in D3D model." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }

            primitiveType = (int)arg[0];

            switch ( primitiveType )
            {
               case 1: // pr_pointlist
                  break;

               case 2: // pr_linelist
               case 3: // pr_linestrip
                  model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Line primitive type is not supported for D3D model." ) ).c_str() );
                  return Model::ERROR_FILTER_SPECIFIC;

               case 4: // pr_trianglelist
                  break;

               case 5: // pr_trianglestrip
                  model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Triangle strip primitive type is not supported for D3D model." ) ).c_str() );
                  return Model::ERROR_FILTER_SPECIFIC;

               case 6: // pr_trianglefan
                  model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Triangle fan primitive type is not supported for D3D model." ) ).c_str() );
                  return Model::ERROR_FILTER_SPECIFIC;

               default:
                  model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Unsupported primitive type in D3D model." ) ).c_str() ); // TODO: Display primitive type.
                  return Model::ERROR_FILTER_SPECIFIC;
            }
            break;

         case 1: // d3d_model_primitive_end
            if ( primitiveType == 0 )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Primitive end without start in D3D model." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }

            // TODO: End triangle fan and strip.

            if ( primitiveType == 4 && unprocessedVerts != 0 )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Incomplete triangle list before primitive end in D3D model." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }

            primitiveType = 0;
            break;

         case 2: // d3d_model_vertex
            createVertex = true;
            origin[0] = arg[0];
            origin[1] = arg[1];
            origin[2] = arg[2];
            break;

         case 3: // d3d_model_vertex_colour
            createVertex = true;
            origin[0] = arg[0];
            origin[1] = arg[1];
            origin[2] = arg[2];
            // color = arg[3]
            // alpha = arg[4]
            break;

         case 4: // d3d_model_vertex_texture
            createVertex = true;
            origin[0] = arg[0];
            origin[1] = arg[1];
            origin[2] = arg[2];
            uv[0] = arg[3];
            uv[1] = arg[4];
            break;

         case 5: // d3d_model_vertex_texture_colour
            createVertex = true;
            origin[0] = arg[0];
            origin[1] = arg[1];
            origin[2] = arg[2];
            uv[0] = arg[3];
            uv[1] = arg[4];
            // color = arg[5]
            // alpha = arg[6]
            break;

         case 6: // d3d_model_vertex_normal
            createVertex = true;
            origin[0] = arg[0];
            origin[1] = arg[1];
            origin[2] = arg[2];
            normal[0] = arg[3];
            normal[1] = arg[4];
            normal[2] = arg[5];
            break;

         case 7: // d3d_model_vertex_normal_colour
            createVertex = true;
            origin[0] = arg[0];
            origin[1] = arg[1];
            origin[2] = arg[2];
            normal[0] = arg[3];
            normal[1] = arg[4];
            normal[2] = arg[5];
            // color = arg[6]
            // alpha = arg[7]
            break;

         case 8: // d3d_model_vertex_normal_texture
            createVertex = true;
            origin[0] = arg[0];
            origin[1] = arg[1];
            origin[2] = arg[2];
            normal[0] = arg[3];
            normal[1] = arg[4];
            normal[2] = arg[5];
            uv[0] = arg[6];
            uv[1] = arg[7];
            break;

         case 9: // d3d_model_vertex_normal_texture_colour
            createVertex = true;
            origin[0] = arg[0];
            origin[1] = arg[1];
            origin[2] = arg[2];
            normal[0] = arg[3];
            normal[1] = arg[4];
            normal[2] = arg[5];
            uv[0] = arg[6];
            uv[1] = arg[7];
            // color = arg[8]
            // alpha = arg[9]
            break;

         default:
            log_warning( "Unknown D3D directive %d\n", directive );
            break;
      }

      if ( createVertex )
      {
         if ( primitiveType == 0 )
         {
            model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Vertex outside of primitive begin/end in D3D model." ) ).c_str() );
            return Model::ERROR_FILTER_SPECIFIC;
         }

         // Invert Y axis.
         origin[1] = -origin[1];
         normal[1] = -normal[1];

         loadMatrix.apply3x( origin );
         loadMatrix.apply3( normal );

         int newVert = -1;

         // Reuse vertex if it already exists.
         unsigned vcount = modelVertices.size();
         for ( unsigned v = 0; v < vcount; v++ )
         {
            if ( floatCompareVector( origin, modelVertices[v]->m_coord, 3 )
              && floatCompareVector( normal, &normalList[v][0], 3 ) )
            {
               newVert = v;
               break;
            }
         }

         if ( newVert == -1 )
         {
            newVert = model->addVertex( origin[0], origin[1], origin[2] );
            normalList.resize( newVert + 1 );
            normalList[newVert][0] = normal[0];
            normalList[newVert][1] = normal[1];
            normalList[newVert][2] = normal[2];
         }

         if ( primitiveType == 4 )
         {
            vertexIndices[unprocessedVerts] = newVert;
            texCoords[unprocessedVerts][0] = uv[0];
            texCoords[unprocessedVerts][1] = uv[1];
            unprocessedVerts++;

            if ( unprocessedVerts == 3 )
            {
               unprocessedVerts = 0;
               int newTri = model->addTriangle( vertexIndices[2], vertexIndices[1], vertexIndices[0] );

               model->setTextureCoords( newTri, 0, texCoords[2][0], 1.0 - texCoords[2][1] );
               model->setTextureCoords( newTri, 1, texCoords[1][0], 1.0 - texCoords[1][1] );
               model->setTextureCoords( newTri, 2, texCoords[0][0], 1.0 - texCoords[0][1] );

               if ( group == -1 )
               {
                  group = model->addGroup( "Group" );
               }
               model->addTriangleToGroup( group, newTri );
            }
         }
      }
   }

   if ( primitiveType != 0 )
   {
      model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Primitive start without end in D3D model." ) ).c_str() );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   if ( numLines > directivesCount )
   {
      log_warning( "Too many directives in D3D model, found %d but expected %d\n", numLines, directivesCount );
   }
   else if ( numLines < directivesCount )
   {
      log_warning( "Too few directives in D3D model, found %d but expected %d\n", numLines, directivesCount );
   }

   return Model::ERROR_NONE;
}

Model::ModelErrorE D3dFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o )
{
   if ( !model || !filename || !filename[0] )
   {
      log_error( "no filename supplied for model filter" );
      return Model::ERROR_BAD_ARGUMENT;
   }

   {
      int groupTexture = -1;
      int gcount = model->getGroupCount();
      for ( int g = 0; g < gcount; g++ )
      {
         int tex = model->getGroupTextureId( g );
         if ( tex >= 0 )
         {
            if ( groupTexture >= 0 && groupTexture != tex )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "D3D requires all groups to have the same material." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }
            groupTexture = tex;
         }
      }
   }

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   DestCloser fc( m_dst );

   if ( err != Model::ERROR_NONE )
      return err;

   // Use the load matrix and then invert it
   Matrix saveMatrix;
   saveMatrix.setRotationInDegrees( -90, -90, 0 );
   saveMatrix = saveMatrix.getInverse();

   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   vector<Model::Vertex *>   & modelVertices  = getVertexList( model );

   MeshList meshes;

   mesh_create_list( meshes, model );

   int numDirectives = 2; // triangle list begin and end
   MeshList::iterator mlit;
   for ( mlit = meshes.begin(); mlit != meshes.end(); mlit++ )
   {
      numDirectives += (*mlit).faces.size() * 3;
   }

   writeLine( "100" ); // version
   writeLine( "%d", numDirectives );
   // d3d_model_primitive_begin( pr_trianglelist )
   writeLine( "0     4.0000     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000" );

   for ( mlit = meshes.begin(); mlit != meshes.end(); mlit++ )
   {
      Mesh::FaceList::iterator fit;

      for ( fit = (*mlit).faces.begin(); fit != (*mlit).faces.end(); fit++ )
      {
         for ( int index = 2; index >= 0; index-- )
         {
            int modelVert = (*mlit).vertices[(*fit).v[index]].v;
            double meshVec[3] = { 0,0,0 };
            float meshNor[3] = { 0,0,0 };

            meshVec[0] = modelVertices[modelVert]->m_coord[0];
            meshVec[1] = modelVertices[modelVert]->m_coord[1];
            meshVec[2] = modelVertices[modelVert]->m_coord[2];

            meshNor[0] = (*fit).vnorm[index][0];
            meshNor[1] = (*fit).vnorm[index][1];
            meshNor[2] = (*fit).vnorm[index][2];

            saveMatrix.apply3x( meshVec );
            saveMatrix.apply3( meshNor );

            // d3d_model_vertex_normal_texture_colour
            // Invert Y axis.
            writeLine( "9     %.6f     %.6f     %.6f     %.6f     %.6f     %.6f     %.6f     %.6f     16777215     1",
               (float)meshVec[0], -(float)meshVec[1], (float)meshVec[2],
               (float)meshNor[0], -(float)meshNor[1], (float)meshNor[2],
               (float)(*fit).uv[index][0], 1.0f - (float)(*fit).uv[index][1] );
         }
      }
   }

   writeLine( "1     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000     0.0000" ); // d3d_model_primitive_end

   return Model::ERROR_NONE;
}

bool D3dFilter::isSupported( const char * filename )
{
   if ( filename )
   {
      unsigned len = strlen( filename );

      if ( len >= 4 && strcasecmp( &filename[len-4], ".d3d" ) == 0 )
      {
         return true;
      }
   }

   return false;
}

list<string> D3dFilter::getReadTypes()
{
   list<string> rval;

   rval.push_back( "*.d3d" );

   return rval;
}

list<string> D3dFilter::getWriteTypes()
{
   list<string> rval;

   rval.push_back( "*.d3d" );

   return rval;
}

bool D3dFilter::writeLine( const char * line, ... )
{
   va_list ap;
   va_start( ap, line );
   m_dst->writeVPrintf( line, ap );
   va_end( ap );
   m_dst->writePrintf( "\r\n" );
   return true;
}
