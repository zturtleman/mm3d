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

#include "mesh.h"

#include "model.h"
#include "log.h"

void mesh_create_list( MeshList & meshes, Model * model, int opt )
{
   meshes.clear();

   if ( model )
   {
      int lastGroup = -1;
      int lastMaterial = -1;

      Mesh m;
      m.clear();

      unsigned int gcount = model->getGroupCount();
      for ( unsigned int g = 0; g < gcount; g++ )
      {
         if ( (opt & Mesh::MO_Group)
               || (opt & Mesh::MO_Material && lastMaterial != model->getGroupTextureId( g ) ) 
            )
         {
            // Need to create a new mesh because of a change in group
            // or material (if the current one is not empty)
            if ( !m.faces.empty() )
            {
               meshes.push_back( m );
            }
            m.clear();
         }

         lastGroup = g;

         lastMaterial = model->getGroupTextureId( g );

         m.options = opt;
         m.group = g;

         std::list<int> triList = model->getGroupTriangles( g );
         std::list<int>::iterator it;

         for ( it = triList.begin(); it != triList.end(); it++ )
         {
            m.addTriangle( model, *it );
         }
      }

      // Now add ungrouped triangles (if there are any)
      std::list<int> triList = model->getUngroupedTriangles();
      if ( !triList.empty() )
      {
         if ( (opt & Mesh::MO_Group)
               || (opt & Mesh::MO_Material && lastMaterial >= 0 ) 
            )
         {
            // Need to create a new mesh because of a change in group
            // or material (if the current one is not empty)
            if ( !m.faces.empty() )
            {
               meshes.push_back( m );
            }
            m.clear();
         }

         lastGroup = -1;
         lastMaterial = -1;

         m.options = opt;
         m.group = -1;

         std::list<int>::iterator it;

         for ( it = triList.begin(); it != triList.end(); it++ )
         {
            m.addTriangle( model, *it );
         }

         // ungrouped is not empty, so make sure it gets added to the list
         meshes.push_back( m );
      }
      else
      {
         // Ungrouped is empty, make sure our mesh gets added to the list
         if ( !m.faces.empty() )
         {
            meshes.push_back( m );
         }
      }
   }
}

void Mesh::clear()
{
   options = 0;
   group = -1;
   vertices.clear();
   faces.clear();
}

void Mesh::addTriangle( Model * model, int triangle )
{
   Face f;

   f.modelTri = triangle;
   model->getFlatNormal( triangle, f.norm );

   for ( int i = 0; i < 3; i++ )
   {
      f.v[i] = addVertex( model, triangle, i );
      model->getNormal( triangle, i, f.vnorm[i] );
      model->getTextureCoords( triangle, i, f.uv[i][0], f.uv[i][1] );
   }

   faces.push_back( f );
}

int Mesh::addVertex( Model * model, int triangle, int vertexIndex )
{
   const double CMPTOL = 0.00005;

   Vertex vert;

   vert.v = model->getTriangleVertex( triangle, vertexIndex );
   model->getNormal( triangle, vertexIndex,  vert.norm );
   model->getTextureCoords( triangle, vertexIndex, vert.uv[0], vert.uv[1] );

   unsigned int vcount = vertices.size();
   for ( unsigned int index = 0; index < vcount; index++ )
   {
      Vertex & cmp = vertices[index];
      if ( cmp.v == vert.v )
      {
         int i;

         // Yes, I'm using goto to break out of the compare.
         // Deal with it.

         // Do we have to match UV coords?
         if ( options & Mesh::MO_UV )
         {
            for ( i = 0; i < 2; i++ )
            {
               if ( fabs( vert.uv[i] - cmp.uv[i] ) > CMPTOL )
               {
                  /*
                     log_debug( "failed match on vertex %d uv %f,%f  /  %f,%f\n",
                     vert.v, vert.uv[0], vert.uv[1], cmp.uv[0], cmp.uv[1] );
                     */
                  goto next_vertex;
               }
            }
         }

         // Do we have to match normals?
         if ( options & Mesh::MO_Normal )
         {
            for ( i = 0; i < 3; i++ )
            {
               if ( fabs( vert.norm[i] - cmp.norm[i] ) > CMPTOL )
               {
                  /*
                     log_debug( "failed match on vertex %d normal %f,%f,%f  /  %f,%f,%f\n",
                     vert.v, vert.norm[0], vert.norm[1], vert.norm[2], cmp.norm[0], cmp.norm[1], cmp.norm[2] );
                     */
                  goto next_vertex;
               }
            }
         }

         // This vertex matches our criteria
         return index;
      }
next_vertex:
      ;
   }

   vertices.push_back( vert );
   return vertices.size() - 1;
}

int mesh_list_vertex_count( const MeshList & meshes )
{
   int vcount = 0;
   MeshList::const_iterator it;
   for ( it = meshes.begin(); it != meshes.end(); it++ )
   {
      vcount += it->vertices.size();
   }
   return vcount;
}

int mesh_list_face_count( const MeshList & meshes )
{
   int fcount = 0;
   MeshList::const_iterator it;
   for ( it = meshes.begin(); it != meshes.end(); it++ )
   {
      fcount += it->faces.size();
   }
   return fcount;
}

int mesh_list_model_vertex( const MeshList & meshes, int meshVertex )
{
   MeshList::const_iterator it;
   for ( it = meshes.begin(); it != meshes.end(); it++ )
   {
      if ( meshVertex < (int) it->vertices.size() )
      {
         return it->vertices[meshVertex].v;
      }
      meshVertex -= it->vertices.size();
   }
   // TODO need an assert here
   return 0;
}

int mesh_list_model_triangle( const MeshList & meshes, int meshTriangle )
{
   MeshList::const_iterator it;
   for ( it = meshes.begin(); it != meshes.end(); it++ )
   {
      if ( meshTriangle < (int) it->faces.size() )
      {
         return it->faces[meshTriangle].modelTri;
      }
      meshTriangle -= it->faces.size();
   }
   // TODO need an assert here
   return 0;
}

int mesh_list_mesh_vertex( const MeshList & meshes, int modelVertex )
{
   int vertBase = 0;
   MeshList::const_iterator it;
   for ( it = meshes.begin(); it != meshes.end(); it++ )
   {
      for ( size_t t = 0; t < it->vertices.size(); t++ )
      {
         if ( modelVertex == (int) it->vertices[t].v )
         {
            return vertBase + t;
         }
      }
      vertBase += it->vertices.size();
   }
   // TODO need an assert here
   return 0;
}

int mesh_list_mesh_triangle( const MeshList & meshes, int modelTriangle )
{
   int triBase = 0;
   MeshList::const_iterator it;
   for ( it = meshes.begin(); it != meshes.end(); it++ )
   {
      for ( size_t t = 0; t < it->faces.size(); t++ )
      {
         if ( modelTriangle == (int) it->faces[t].modelTri )
         {
            return triBase + t;
         }
      }
      triBase += it->faces.size();
   }
   // TODO need an assert here
   return 0;
}

