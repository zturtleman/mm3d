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


#ifndef __SCRIPTIF_H
#define __SCRIPTIF_H

#include <vector>

#include "model.h"

class MeshRectangle
{
   public:
      MeshRectangle() {};
      virtual ~MeshRectangle() {};

      std::vector<int> m_vertices;
      std::vector<int> m_faces;
};


//------------------------------------------------------------------
// doc_section General
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> modelGetName </h3>
//
// <p>
// <b> Descritption: </b> Get the filename of the model
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> <b> string </b> - filename (or empty string)
// </ul>
//
// <p>
// <b> Notes: </b> If the model does not have a filename this function
// will return an empty string.
// </p>
//
// doc_f_end
//
extern const char * scriptif_modelGetName( Model * model );

// doc_f_begin
//
// <h3> modelSaveAs </h3>
//
// <p>
// <b> Descritption: </b> Save the model as a specified format.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string format </b> - Format to save as (file extension)
// </ul>
//
// Returns:
// <ul>
//    <li> <b> string </b> - filename
// </ul>
//
// <p>
// <b> Notes: </b> Existing files will not be over-written.  If writing
// the file in the specified format would cause an over-write, a number is
// added to the filename and incremented until the filename is unique.
// </p>
//
// doc_f_end
//
bool scriptif_modelSaveAs( Model * model, const char * f );

// doc_f_begin
//
// <h3> modelGetVertexCount </h3>
//
// <p>
// <b> Descritption: </b> Get the number of vertices in the model.
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - number of vertices
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int  scriptif_modelGetVertexCount( Model * model );

// doc_f_begin
//
// <h3> modelGetFaceCount </h3>
//
// <p>
// <b> Descritption: </b> Get the number of faces (polygons (triangles)) in the model.
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - number of faces
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int  scriptif_modelGetFaceCount( Model * model );

// doc_f_begin
//
// <h3> modelGetGroupCount </h3>
//
// <p>
// <b> Descritption: </b> Get the number of polygon groups in the model.
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - number of groups
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int  scriptif_modelGetGroupCount( Model * model );

// doc_f_begin
//
// <h3> modelGetJointCount </h3>
//
// <p>
// <b> Descritption: </b> Get the number of bone joints in the model.
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - number of bone joints
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int  scriptif_modelGetJointCount( Model * model );

// doc_f_begin
//
// <h3> modelGetTextureCount </h3>
//
// <p>
// <b> Descritption: </b> Get the number of textures in the model.
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - number of textures
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int  scriptif_modelGetTextureCount( Model * model );

// doc_f_begin
//
// <h3> modelGetGroupByName </h3>
//
// <p>
// <b> Descritption: </b> Get the ID of a group from its name.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string name </b> - Name of group
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - Group ID (-1 if not found)
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int  scriptif_modelGetGroupByName( Model * model, const char * name );

// doc_f_begin
//
// <h3> groupGetName </h3>
//
// <p>
// <b> Descritption: </b> Get the name of the specified group.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int groupID </b> - ID number of group
// </ul>
//
// Returns:
// <ul>
//    <li> <b> string </b> - Name of group
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern const char * scriptif_groupGetName( Model * model, unsigned groupId );

// doc_f_begin
//
// <h3> modelGetTextureByName </h3>
//
// <p>
// <b> Descritption: </b> Get the ID of a texture from its name.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string name </b> - Name of texture
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - Texture ID (-1 if not found)
// </ul>
//
// <p>
// <b> Notes: </b> In most circumstances the name of the texture is the 
// filename without its
// full path or extension.  The exeption to this is if the model did not 
// have textures explicitly associated with it and the model filter had
// to guess at texture file names.  In this case the texture name will
// be the texture filename including the extension, but without the full path.
// </p>
//
// doc_f_end
//
extern int  scriptif_modelGetTextureByName( Model * model, const char * name );

// doc_f_begin
//
// <h3> textureGetName </h3>
//
// <p>
// <b> Descritption: </b> Get the name of the specified texture.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int textureID </b> - ID number of texture
// </ul>
//
// Returns:
// <ul>
//    <li> <b> string </b> - Name of texture
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern const char * scriptif_textureGetName( Model * model, unsigned textureId );

// doc_f_begin
//
// <h3> textureGetFilename </h3>
//
// <p>
// <b> Descritption: </b> Get the filename of the specified texture.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int textureID </b> - ID number of texture
// </ul>
//
// Returns:
// <ul>
//    <li> <b> string </b> - Filename of texture (full path)
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern const char * scriptif_textureGetFilename( Model * model, unsigned textureId );

//------------------------------------------------------------------
// doc_section Primitive Creation
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> modelCrateMeshRectangle </h3>
//
// <p>
// <b> Descritption: </b> Create a rectangular mesh.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> float x1 </b> - X coordinate of vertex 1
//    <li> <b> float y1 </b> - Y coordinate of vertex 1
//    <li> <b> float z1 </b> - Z coordinate of vertex 1
//    <li> <b> float x2 </b> - X coordinate of vertex 2
//    <li> <b> float y2 </b> - Y coordinate of vertex 2
//    <li> <b> float z2 </b> - Z coordinate of vertex 2
//    <li> <b> float x3 </b> - X coordinate of vertex 3
//    <li> <b> float y3 </b> - Y coordinate of vertex 3
//    <li> <b> float z3 </b> - Z coordinate of vertex 3
//    <li> <b> float x4 </b> - X coordinate of vertex 4
//    <li> <b> float y4 </b> - Y coordinate of vertex 4
//    <li> <b> float z4 </b> - Z coordinate of vertex 4
//    <li> <b> segments </b> (opt) - Number of rows/columns in rectangle (defaults to 1)
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - Mesh ID
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern MeshRectangle * scriptif_modelCreateMeshRectangle( Model * model, 
      double x0, double y0, double z0,
      double x1, double y1, double z1,
      double x2, double y2, double z2,
      double x3, double y3, double z3,
      unsigned segments  = 1 );

// doc_f_begin
//
// <h3> modelCrateBoneJoint </h3>
//
// <p>
// <b> Descritption: </b> Get the number of vertices in the model.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string name </b> - Name of bone joint
//    <li> <b> float x </b> - X coordinate of bone joint
//    <li> <b> float y </b> - Y coordinate of bone joint
//    <li> <b> float z </b> - Z coordinate of bone joint
//    <li> <b> float xrot </b> - X rotation of bone joint (in degrees)
//    <li> <b> float yrot </b> - Y rotation of bone joint (in degrees)
//    <li> <b> float zrot </b> - Z rotation of bone joint (in degrees)
//    <li> <b> int parentID </b> - ID of parent joint (-1 is no parent, root joint)
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - Joint ID
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int scriptif_modelCreateBoneJoint( Model * model, const char * name, 
      double x, double y, double z, double xrot, double yrot, double zrot, int parent );

//------------------------------------------------------------------
// Primitive Manipulation
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> vertexSetCoords </h3>
//
// <p>
// <b> Descritption: </b> Move a vertex to a new position
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int vertexID </b> ID of vertex to move
//    <li> <b> float x </b> New X coordinate
//    <li> <b> float y </b> New Y coordinate
//    <li> <b> float z </b> New Z coordinate
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern void scriptif_vertexSetCoords( Model * model, unsigned vertexIndex,
      double x, double y, double z );

// doc_f_begin
//
// <h3> modelAddTexture </h3>
//
// <p>
// <b> Descritption: </b> Add an image file to a model as a texture.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string filename </b> - Filename of texture to add (can be relative)
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - Texture ID
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int scriptif_modelAddTexture( Model * model, const char * filename );

// doc_f_begin
//
// <h3> vertexSetCoords </h3>
//
// <p>
// <b> Descritption: </b> Assign a texture to a group.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int groupID </b> ID of group
//    <li> <b> int textureID </b> ID of texture for group
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern void scriptif_groupSetTexture( Model * model, unsigned groupId, 
      unsigned textureId );

// doc_f_begin
//
// <h3> faceSetTextureCoords </h3>
//
// <p>
// <b> Descritption: </b> Set the UV texture coordinates for a vertex of a polygon
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int faceID </b> Polygon ID
//    <li> <b> int vertexIndex </b> Index of polygon vertex to assign (0-2 for triangles)
//    <li> <b> float u </b> U (horizontal) coordinate for vertex on the group texture (0.0 - 1.0)
//    <li> <b> float v </b> V (vertical) coordinate for vertex on the group texture (0.0 - 1.0)
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
// <p>
// Texture coordinates are from 0.0 to 1.0.  The origin (0.0, 0.0) is the 
// <b>lower-left</b> corner of the texture, not the <b>upper-right</b> corner.
// </p>
//
// doc_f_end
//
extern void scriptif_faceSetTextureCoords( Model * model, unsigned faceId, 
      unsigned vertexIndex, double s, double t );

//------------------------------------------------------------------
// doc_section Selection manipulation
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> selectedRotate </h3>
//
// <p>
// <b> Descritption: </b> Rotate the selected vertices and bone joints around the
// origin.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> float x </b> - Rotation around X axis in degrees
//    <li> <b> float y </b> - Rotation around Y axis in degrees
//    <li> <b> float z </b> - Rotation around Z axis in degrees
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> Rotation is around the origin.  If you want to rotate around an
// arbitrary point (x0,y0,z0) you can use the following steps:
// </p>
//
// <ol>
//    <li> selectedTranslate ( -x0, -y0, -z0 )
//    <li> selectedRotate( x, y, z )
//    <li> selectedTranslate (  x0,  y0,  z0 )
// </ol>
//
// doc_f_end
//
extern void scriptif_selectedRotate( Model * model,
      double x, double y, double z ); // degrees

// doc_f_begin
//
// <h3> selectedTranslate </h3>
//
// <p>
// <b> Descritption: </b> Translate the selected vertices and bone joints.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> float x </b> - Units to translate on the X axis
//    <li> <b> float y </b> - Units to translate on the Y axis
//    <li> <b> float z </b> - Units to translate on the Z axis
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_selectedTranslate( Model * model,
      double x, double y, double z );

// doc_f_begin
//
// <h3> selectedScale </h3>
//
// <p>
// <b> Descritption: </b> Scale the distance between selected vertices and bone joints.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> float x </b> - Units to scale on the X axis
//    <li> <b> float y </b> - Units to scale on the Y axis
//    <li> <b> float z </b> - Units to scale on the Z axis
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_selectedScale( Model *,
      double x, double y, double z );

// doc_f_begin
//
// <h3> selectedApplyMatrix </h3>
//
// <p>
// <b> Descritption: </b> Apply an arbitrary 4x4 matrix to the selected vertices and joints.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> float m1 </b> - First matrix element
//    <li> <b> m2 ... m14 </b>
//    <li> <b> float m15 </b> - Last matrix element
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> The m13, m14, m15 elements are the translation elements.
// </p>
//
// doc_f_end
//
extern void scriptif_selectedApplyMatrix( Model * model,
      double m0,  double m1,  double m2,  double m3,
      double m4,  double m5,  double m6,  double m7,
      double m8,  double m9,  double m10, double m11,
      double m12, double m13, double m14, double m15 ); // 12, 13, 14 are translation

// For testing only, no documentation
extern void scriptif_selectedDelete( Model * model );

// doc_f_begin
//
// <h3> selectedWeldVertices </h3>
//
// <p>
// <b> Descritption: </b> Any selected vertcies that share coordinates
// into one vertex.
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> This does not weld all verticies into one.  It only welds
// vertices that are very, very close to each other.
// </p>
//
// doc_f_end
//
extern void scriptif_selectedWeldVertices( Model * model );

// doc_f_begin
//
// <h3> selectedInvertNormals </h3>
//
// <p>
// <b> Descritption: </b> Invertes the normals on any selected faces (polygons (triangles)).
// Effectively this turns objects inside-out so that light reflects off the opposite sides of the faces.
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_selectedInvertNormals( Model * model );

// doc_f_begin
//
// <h3> selectedGroupFaces </h3>
//
// <p>
// <b> Descritption: </b> Assigns all selected faces to a group specified by name.
// The group is created if it does not exist.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string name </b> - Name of group for faces
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int groupID </b> - Group ID
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int  scriptif_selectedGroupFaces( Model * model, const char * name );

// doc_f_begin
//
// <h3> selectedAddToGroup </h3>
//
// <p>
// <b> Descritption: </b> Adds selected faces to an existing group.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int groupID </b> ID of group
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern void scriptif_selectedAddToGroup( Model * model, int groupId );

//------------------------------------------------------------------
// doc_section Animation
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> modelCreateAnimation </h3>
//
// <p>
// <b> Descritption: </b> Creates a new animation
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> string name </b> - Name of new animation
//    <li> <b> int frameIndex </b> - Frame count
//    <li> <b> float fps </b> - Frames per second
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - AnimationID
// </ul>
//
// <p>
// <b> Notes: </b> Animation IDs are unique to their type, but can be the same for
// two different types.  In other words, the first skeletal animation and the first
// frame animation are always zero (0).  This is why you must use an
// animation type/mode string when you specify an animation ID.
// </p>
//
// doc_f_end
//
extern int scriptif_modelCreateAnimation( Model * model, Model::AnimationModeE mode,
      const char * name, unsigned frameCount, double fps );

// doc_f_begin
//
// <h3> setAnimByIndex </h3>
//
// <p>
// <b> Descritption: </b> Sets the current animation from a mode string
// and an ID value.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animID </b> - Animation ID
// </ul>
//
// Returns:
// <ul>
//    <li> <b> bool </b> - Returns true if animation exists, false otherwise.
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// <p>
// You can set frame animation vertex positions or skeletal animation keyframes
// directly using functions below.  You only need to use this function if you want
// to use matrix transformations to manipulate several animated primitives at once;
// for example, translating or rotating many vertex positions at one time.
// </p>
//
// doc_f_end
//
extern bool scriptif_setAnimByIndex( Model * model, Model::AnimationModeE mode,
      unsigned anim );

// doc_f_begin
//
// <h3> setAnimByName </h3>
//
// <p>
// <b> Descritption: </b> Sets the current animation from a mode string
// and a name string.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> string name </b> - Name of animation
// </ul>
//
// Returns:
// <ul>
//    <li> <b> bool </b> - Returns true if animation exists, false otherwise.
// </ul>
//
// <p>
// <b> Notes: </b> You can set frame animation vertex positions or skeletal animation keyframes
// directly using functions below.  You only need to use this function if you want
// to use matrix transformations to manipulate several animated primitives at once;
// for example, translating or rotating many vertex positions at one time.
// </p>
//
// doc_f_end
//
extern bool scriptif_setAnimByName( Model * model, Model::AnimationModeE mode,
      const char * name );

// doc_f_begin
//
// <h3> animSetFrame </h3>
//
// <p>
// <b> Descritption: </b> Sets the current frame of the current animation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int frameIndex </b> - Frame index of current animation (0 to frame_count-1)
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> You can set frame animation vertex positions or skeletal animation keyframes
// directly using functions below.  You only need to use this function if you want
// to use matrix transformations to manipulate several animated primitives at once;
// for example, translating or rotating many vertex positions at one time.
// </p>
//
// doc_f_end
//
extern void scriptif_animSetFrame( Model * model, unsigned frame );

extern void scriptif_setAnimTime( Model * model, double seconds );

// doc_f_begin
//
// <h3> animGetCount </h3>
//
// <p>
// <b> Descritption: </b> Get the number of animations of a specified type.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - The number of animations of this type
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int scriptif_animGetCount( Model * model, Model::AnimationModeE mode );

// doc_f_begin
//
// <h3> animGetName </h3>
//
// <p>
// <b> Descritption: </b> Get the name of an animation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
// </ul>
//
// Returns:
// <ul>
//    <li> <b> string </b> - The animation name
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern const char * scriptif_animGetName( Model * model, Model::AnimationModeE mode, unsigned animIndex );

// doc_f_begin
//
// <h3> animGetFrameCount </h3>
//
// <p>
// <b> Descritption: </b> Get the frame count of an animation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - The frame count of the animation
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int scriptif_animGetFrameCount( Model * model, Model::AnimationModeE mode, unsigned animIndex );

// doc_f_begin
//
// <h3> animSetName </h3>
//
// <p>
// <b> Descritption: </b> Change the name of the specified animation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> string name </b> - New animation name
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_animSetName( Model * model, Model::AnimationModeE mode, 
      unsigned animIndex, const char * name );

// doc_f_begin
//
// <h3> animSetFrameCount </h3>
//
// <p>
// <b> Descritption: </b> Change the frame count of the specified animation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> int frameCount </b> - New animation frame count
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_animSetFrameCount( Model * model, Model::AnimationModeE mode,
      unsigned animIndex, unsigned frameCount );

// doc_f_begin
//
// <h3> animSetFPS </h3>
//
// <p>
// <b> Descritption: </b> Change the animation speed.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> float fps </b> - New animation speed in frames per second
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_animSetFPS( Model * model, Model::AnimationModeE mode,
      unsigned animIndex, double fps );

// doc_f_begin
//
// <h3> animClearFrame </h3>
//
// <p>
// <b> Descritption: </b> Clear an animation frame so that all primitives are in
// their default position and orientation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> int frame </b> - Frame number (0 to frame_count-1, inclusive)
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_animClearFrame( Model * model, Model::AnimationModeE mode,
      unsigned animIndex, unsigned frame );

// doc_f_begin
//
// <h3> animCopyFrame </h3>
//
// <p>
// <b> Descritption: </b> Copy an animation frame to another frame so that each
// frame is identical.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> int src </b> - Source frame number (0 to frame_count-1, inclusive), the 
//       frame you want to make a copy of
//    <li> <b> int dest </b> - Destination frame number (0 to frame_count-1, inclusive), 
//       the frame you want to copy to
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_animCopyFrame( Model * model, Model::AnimationModeE mode,
      unsigned animIndex, unsigned src, unsigned dest );


//------------------------------------------------------------------
// doc_section Animation set manipulation
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> animMove </h3>
//
// <p>
// <b> Descritption: </b> Move an animation from one index to another.  This changes
// the order in which animations are listed.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animIndex1 </b> - The current animation index (ID number)
//    <li> <b> int animIndex2 </b> - The new animation index
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> Animation indices and IDs are the same thing.  If you move an
// animation its ID changes to its new index position.  Other animation IDs may
// be changed as well if they appear after the old or new animation index.
// </p>
//
// doc_f_end
//
extern void scriptif_animMove( Model * model, Model::AnimationModeE mode,
      unsigned animIndex1, unsigned animIndex2 );

// doc_f_begin
//
// <h3> animCopy </h3>
//
// <p>
// <b> Descritption: </b> Create a new animation that is a copy of an existing
// animation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> string name </b> - Name of new animation
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - ID number of the new animation
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int scriptif_animCopy( Model * model, Model::AnimationModeE mode,
      unsigned animIndex, const char * name );

// doc_f_begin
//
// <h3> animSplit </h3>
//
// <p>
// <b> Descritption: </b> Split an animation into two separate animations at a
// specified frame.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> string name </b> - Name of new animation
//    <li> <b> int frame </b> - The frame at which to split the animation (0 to frame_count-1, inclusive)
// </ul>
//
// Returns:
// <ul>
//    <li> <b> int </b> - ID number of the new animation
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern int scriptif_animSplit( Model * model, Model::AnimationModeE mode,
      unsigned animIndex, const char * name, unsigned frame );

// doc_f_begin
//
// <h3> animJoin </h3>
//
// <p>
// <b> Descritption: </b> Join two animations together into one.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID1 </b> - The ID number of the animation to join to
//    <li> <b> int animationID2 </b> - The ID number of the animation to join from
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> In case the above argument descriptions are unclear, 
// animation 2 will be appended to animation 1.
// </p>
//
// doc_f_end
//
extern void scriptif_animJoin( Model * model, Model::AnimationModeE mode,
      unsigned anim1, unsigned anim2 );

// doc_f_begin
//
// <h3> animConvertToFrame </h3>
//
// <p>
// <b> Descritption: </b> Convert a skeletal animation to a frame animation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation to convert
//    <li> <b> string name </b> - The name of the new animation
//    <li> <b> int frame_count </b> - The number of frames in the new animation
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> The two animations will complete in the same amount of time.
// The existing animation may be resampled if the frame count of
// the new animation is different from the existing animation.  The new animation's
// frames per second will be changed to make the two animations compelte in the same
// amount of time.
// </p>
//
// <p> In other words:  <code>frame_count1 * fps1 = frame_count2 * fps2</code>
// </p>
//
// doc_f_end
//
extern int scriptif_animConvertToFrame( Model * model, Model::AnimationModeE mode,
      unsigned animIndex, const char * newName, unsigned frameCount );


//------------------------------------------------------------------
// doc_section Mode-specific animation manipulations
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> skelAnimSetKeyframe </h3>
//
// <p>
// <b> Descritption: </b> Set a keyframe for a specified animation, frame, and
// joint.  Keyframe may be rotation or translation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> int frame </b> - The index of the frame (0 to frame_count-1, inclusive)
//    <li> <b> int jointID </b> - The ID number of the joint
//    <li> <b> bool isRotation </b> - Is true for rotation keyframe, false for translation keyframe
//    <li> <b> float x </b> - Coordinate or rotation on X axis
//    <li> <b> float y </b> - Coordinate or rotation on Y axis
//    <li> <b> float z </b> - Coordinate or rotation on Z axis
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_skelAnimSetKeyframe( Model * model,
      unsigned animIndex, unsigned frame, unsigned joint, bool isRotation,
      double x, double y, double z );

// doc_f_begin
//
// <h3> skelAnimDeleteKeyframe </h3>
//
// <p>
// <b> Descritption: </b> Deletes a keyframe for a specified animation, frame, and
// joint.  Keyframe may be rotation or translation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> int frame </b> - The index of the frame (0 to frame_count-1, inclusive)
//    <li> <b> int jointID </b> - The ID number of the joint
//    <li> <b> bool isRotation </b> - Is true for rotation keyframe, false for translation keyframe
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> The joint will have its default position or orientation after
// calling this function.  If there are rotation and translation keyframes, you must
// call this function twice to delete both; once with isRotation true, and once
// with isRotation false.
// </p>
//
// doc_f_end
//
extern void scriptif_skelAnimDeleteKeyframe( Model * model,
      unsigned animIndex, unsigned frame, unsigned joint, bool isRotation );

// doc_f_begin
//
// <h3> frameAnimSetVertex </h3>
//
// <p>
// <b> Descritption: </b> Deletes a keyframe for a specified animation, frame, and
// joint.  Keyframe may be rotation or translation.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string mode </b> - Animation mode: &quot;skeletal&quot; or &quot;frame&quot;
//    <li> <b> int animationID </b> - The ID number of the animation
//    <li> <b> int frame </b> - The index of the frame (0 to frame_count-1, inclusive)
//    <li> <b> int vertexID </b> - The ID number of the joint
//    <li> <b> float x </b> - New X coordinate
//    <li> <b> float y </b> - New Y coordinate
//    <li> <b> float z </b> - New Z coordinate
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_frameAnimSetVertex( Model * model, unsigned animIndex, 
      unsigned frame, unsigned v, double x, double y, double z );

//------------------------------------------------------------------
// doc_section Selection
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> modelSelectAll </h3>
//
// <p>
// <b> Descritption: </b> Selects all primitives (vertices, faces, and bone joints)
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectAll( Model * model );

// doc_f_begin
//
// <h3> modelSelectAllVertices </h3>
//
// <p>
// <b> Descritption: </b> Selects all vertices
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectAllVertices( Model * model );

// doc_f_begin
//
// <h3> modelSelectAllFaces </h3>
//
// <p>
// <b> Descritption: </b> Selects all faces
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectAllFaces( Model * model );

// doc_f_begin
//
// <h3> modelSelectAllGroups </h3>
//
// <p>
// <b> Descritption: </b> Selects all faces of all groups
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectAllGroups( Model * model );

// doc_f_begin
//
// <h3> modelSelectVertices </h3>
//
// <p>
// <b> Descritption: </b> Selects all vertices
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectAllJoints( Model * model );

// doc_f_begin
//
// <h3> modelSelectVertex </h3>
//
// <p>
// <b> Descritption: </b> Selects specified vertex
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int vertexID </b> - ID of vertex to select
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectVertex( Model * model, int vertex );

// doc_f_begin
//
// <h3> modelSelectFace </h3>
//
// <p>
// <b> Descritption: </b> Selects specified face
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int faceID </b> - ID of face to select
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectFace( Model * model, int face );

// doc_f_begin
//
// <h3> modelSelectGroup </h3>
//
// <p>
// <b> Descritption: </b> Selects faces of specified group
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int groupID </b> - ID of group to select
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectGroup( Model * model, int group );

// doc_f_begin
//
// <h3> modelSelectJoint </h3>
//
// <p>
// <b> Descritption: </b> Selects specified joint
// </p>
//
// Arguments:
// <ul>
//    <li> <b> int jointID </b> - ID of joint to select
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> IDs of any type are integers from zero to count-1 inclusive.
// Negative numbers are invalid.
// </p>
//
// doc_f_end
//
extern void scriptif_modelSelectJoint( Model * model, int joint );

// doc_f_begin
//
// <h3> modelUnselectAll </h3>
//
// <p>
// <b> Descritption: </b> Unselects all primitives (vertices, faces, bone joints)
// </p>
//
// Arguments:
// <ul>
//    <li> None
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_modelUnselectAll( Model * );

//------------------------------------------------------------------
// doc_section Logging
//------------------------------------------------------------------

// doc_f_begin
//
// <h3> logDebug </h3>
//
// <p>
// <b> Descritption: </b> Send a message string to the debug log.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string message </b> - message to log
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_logDebug( const char * str );

// doc_f_begin
//
// <h3> logWarning </h3>
//
// <p>
// <b> Descritption: </b> Send a message string to the warning log.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string message </b> - message to log
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_logWarning( const char * str );

// doc_f_begin
//
// <h3> logError </h3>
//
// <p>
// <b> Descritption: </b> Send a message string to the error log.
// </p>
//
// Arguments:
// <ul>
//    <li> <b> string message </b> - message to log
// </ul>
//
// Returns:
// <ul>
//    <li> None
// </ul>
//
// <p>
// <b> Notes: </b> None
// </p>
//
// doc_f_end
//
extern void scriptif_logError( const char * str );

#endif // __SCRIPTIF_H
