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


#ifndef __MESH_H
#define __MESH_H

#include <vector>

class Model;

// The Mesh class contains a subset of model data. The purpose is to
// force model data to conform to some restrictions which do not
// apply to MM3D models. Examples of this include some model formats
// which don't allow vertices to belong to more one texture group
// or have different normals for different faces that use them.
//
// Use mesh_create_list() to build meshes from a model

class Mesh
{
    public:
        // Rules for creating a mesh. If the option value is 0, all criteria
        // are ignored; a single mesh containing all vertices and faces will
        // be constructed. Including any option below will cause multiple
        // meshes to be created and vertices to be duplicated as necessary.
        enum _Option_e
        {
            MO_UV       = 0x0001,  // Mesh vertices must have only one UV coord
            MO_Normal   = 0x0002,  // Mesh vertices must have only one normal
            MO_Group    = 0x0100,  // Mesh faces must belong to the same group
            MO_Material = 0x0200,  // Mesh faces must have the same material
            MO_All      = 0xffff,  // All of the above
            MO_MAX
        };
        typedef enum _Option_e OptionE;

        // There is an entry in a VertexList for each vertex that is
        // used by the mesh. The index is the vertex number in the mesh,
        // the value is the vertex number in the model plus other 
        // vertex-specific data (uvs, normals, etc)
        class Vertex
        {
            public:
                int v;         // model vertex number
                float norm[3]; // normal
                float uv[2];   // texture coords
        };
        typedef std::vector< Vertex > VertexList;

        class Face
        {
            public:
                int   modelTri;    // triangle that this face was built from
                int   v[3];        // vertex index (in Mesh's vertices list)
                float norm[3];     // face normal
                float vnorm[3][3]; // vertex normals
                float uv[3][2];    // texture coords (per triangle vertex)
        };
        typedef std::vector< Face > FaceList;

        int options;           // options this mesh was created with
        int group;             // group that this mesh was built from (or -1 if none)
        VertexList vertices;   // list of model vertices used by the mesh
        FaceList faces;        // list of mesh faces

        void clear();
        void addTriangle( Model * m, int triangle );
        int  addVertex( Model * model, int triangle, int vertexIndex );
};
typedef std::vector< Mesh > MeshList;

void mesh_create_list( MeshList & meshes, Model * model, int options = Mesh::MO_All );

#endif // __MESH_H
