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


#ifndef __MODEL_H
#define __MODEL_H

#include "glheaders.h"
#include "glmath.h"
#include "bsptree.h"
#include "drawcontext.h"
#include "sorted_list.h"

#ifdef MM3D_EDIT
#include "undomgr.h"
#endif // MM3D_EDIT

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <list>
#include <vector>
#include <set>
#include <string>

using std::list;
using std::vector;

class Texture;

// Warning: Here be dragons.
//
// The Model class is the core of MM3D. It contains both the oldest code and
// the newest code. Some very early design decisions were bad decisions. Some of
// those bad decisions are still here causing headaches. I've tried to add some
// documentation to this header file. Despite that, the best way to understand
// how to use the model class is to find reference code elsewhere in MM3D that
// does something similar to what you want to do.
//
// The model represents everything about how the model is rendered as well as some
// properties that are not visible to the user. A model primitive is any sub-object
// within the model (vertices, triangles, bone joints, points, texture projections,
// etc).
//
// All faces in the Model are triangles.
//
// A group can be thought of as a mesh made up of triangles. Materials can be
// assigned to groups (so all triangles in the group share the same material).
// Unlike meshes in some implementations, vertices in MM3D models may be shared
// by triangles in different groups.
//
// A material defines how lighting is reflected off of a group of faces. A material
// may have an associated texture for texture mapping. Occasionally you will see
// the term "texture" used where "material" is meant.
//
// The Model implementation is split among several C++ files, listed below:
//
//    model.cc: A catch-all for core Model functionality.
//    model_anim.cc: Animation
//    model_bool.cc: Boolean operations (union, subtract, intersect)
//    model_draw.cc: Visual rendering of the model
//    model_group.cc: Group/Mesh-related functionality
//    model_influence.cc: Bone joint influences on vertices and points
//    model_inner.cc: Core functionality for Model's inner classes
//    model_insert.cc: Undo/redo book-keeping for adjusting indices in primitive lists
//    model_meta.cc: MetaData methods
//    model_ops.cc: Model comparison, merging, simplification
//    model_proj.cc: Texture projections
//    model_select.cc: Selection logic
//    model_test.cc: Old tests for file format loading/saving, being replaced
//    model_texture.cc: Material and texture code
//
// TODO rename Texture -> Material where appropriate
// TODO Make texture creation more consistent

class Model
{
   public:
      // ChangeBits is used to tell Model::Observer objects what has changed
      // about the model.
      enum ChangeBits
      {
         SelectionChange    =  0x00000001,  // General selection change
         SelectionVertices  =  0x00000002,  // Vertices selection changed
         SelectionFaces     =  0x00000004,  // Faces selection changed
         SelectionGroups    =  0x00000008,  // Groups selection changed
         SelectionJoints    =  0x00000010,  // Joints selection changed
         SelectionPoints    =  0x00000020,  // Points selection changed
         AddGeometry        =  0x00000100,  // Added or removed objects
         AddAnimation       =  0x00000200,  // Added or removed animations
         AddOther           =  0x00008000,  // Added or removed something else
         MoveGeometry       =  0x00010000,  // Model shape changed
         MoveOther          =  0x00080000,  // Something non-geometric was moved
         AnimationMode      =  0x00010000,  // Changed animation mode
         AnimationSet       =  0x00020000,  // Changes to animation sets
         AnimationFrame     =  0x00080000,  // Changed current animation frame
         ChangeAll          =  0xFFFFFFFF   // All of the above
      };

      // FIXME remove this when new equal routines are ready
      enum CompareBits
      {
         CompareGeometry   =   0x01,  // Vertices and Faces match
         CompareFaces      =   0x02,  // Faces match, vertices may not
         CompareGroups     =   0x04,  // Groups match
         CompareSkeleton   =   0x08,  // Bone joints hierarchy matches
         CompareMaterials  =   0x10,  // Material properties and group material assignments match
         CompareAnimSets   =   0x20,  // Number of animations, frame counts, and fps match
         CompareAnimData   =   0x40,  // Animation movements match
         CompareMeta       =   0x80,  // Names and other non-visible data match
         ComparePoints     =  0x100,  // Points match
         CompareInfluences =  0x200,  // Vertex and point influences match
         CompareTextures   =  0x400,  // Texture coordinates and texture data match
         CompareAll        = 0xFFFF   // All of the above
      };

      enum ComparePartsE
      {
         PartVertices    = 0x0001,  // 
         PartFaces       = 0x0002,  // 
         PartGroups      = 0x0004,  // 
         PartMaterials   = 0x0008,  // 
         PartTextures    = 0x0010,  // 
         PartJoints      = 0x0020,  // 
         PartPoints      = 0x0040,  // 
         PartProjections = 0x0080,  // 
         PartBackgrounds = 0x0100,  // 
         PartMeta        = 0x0200,  // 
         PartSkelAnims   = 0x0400,  // 
         PartFrameAnims  = 0x0800,  // 
         PartFormatData  = 0x1000,  // 
         PartFilePaths   = 0x2000,  // 
         PartAll         = 0xFFFF,  // 

         // These are combinations of parts above, for convenience
         PartGeometry    = PartVertices | PartFaces | PartGroups,  // 
         PartTextureMap  = PartFaces | PartGroups | PartMaterials | PartTextures | PartProjections,  // 
         PartAnimations  = PartSkelAnims | PartFrameAnims,  // 
      };

      enum PartPropertiesE
      {
         PropName        = 0x000001,  // 
         PropType        = 0x000002,  // 
         PropSelection   = 0x000004,  // 
         PropVisibility  = 0x000008,  // 
         PropFree        = 0x000010,  // 
         PropCoords      = 0x000020,  // 
         PropRotation    = 0x000040,  // 
         PropScale       = 0x000080,  // 
         PropInfluences  = 0x000100,  // 
         PropWeights     = 0x000200,  // 
         PropNormals     = 0x000400,  // 
         PropTexCoords   = 0x000800,  // 
         PropMaterials   = 0x001000,  // 
         PropProjections = 0x002000,  // 
         PropVertices    = 0x004000,  // 
         PropPoints      = 0x008000,  // 
         PropTriangles   = 0x010000,  // 
         PropLighting    = 0x020000,  // 
         PropClamp       = 0x040000,  // 
         PropPaths       = 0x080000,  // 
         PropDimensions  = 0x100000,  // 
         PropPixels      = 0x200000,  // 
         PropTime        = 0x400000,  // 
         PropAll         = 0xFFFFFF,  // 

         // These are combinations of parts above, for convenience
         PropFlags       = PropSelection | PropVisibility | PropFree,
      };

      enum 
      {
         MAX_INFLUENCES = 4
      };

      enum _PositionType_e
      {
        PT_Vertex,
        PT_Joint,
        PT_Point,
        PT_Projection
      };
      typedef enum _PositionType_e PositionTypeE;

      enum _OperationScope_e
      {
         OS_Selected,
         OS_Global
      };
      typedef enum _OperationScope_e OperationScopeE;

      enum _TextureProjectionType_e
      {
        TPT_Custom      = -1,  // No projection
        TPT_Cylinder    = 0,
        TPT_Sphere      = 1,
        TPT_Plane       = 2,
        TPT_Icosahedron = 3,   // Not yet implemented
        TPT_MAX
      };
      typedef enum _TextureProjectionType_e TextureProjectionTypeE;

      enum _InfluenceType_e
      {
         IT_Custom = 0,
         IT_Auto,
         IT_Remainder,
         IT_MAX
      };
      typedef enum _InfluenceType_e InfluenceTypeE;

      struct _Influence_t
      {
         int m_boneId;
         InfluenceTypeE m_type;
         double m_weight;

         bool operator==( const struct _Influence_t & rhs ) const
         {
            return m_boneId == rhs.m_boneId
               && m_type == rhs.m_type
               && fabs( m_weight - rhs.m_weight ) < 0.00001;
         }

         bool operator!=( const struct _Influence_t & rhs ) const
         {
            return ! (*this == rhs );
         }

         bool operator<( const struct _Influence_t & rhs ) const
         {
            if ( fabs( m_weight - rhs.m_weight ) < 0.00001 )
               return m_boneId < rhs.m_boneId;
            return m_weight < rhs.m_weight;
         }
      };
      typedef struct _Influence_t InfluenceT;
      typedef std::list< InfluenceT > InfluenceList;

      // A position is a common way to manipulate any object that has a
      // single position value. See the PositionTypeE values.
      // The index property is the index in the the list that corresponds
      // to the type.
      class Position
      {
         public:
            PositionTypeE type;
            unsigned      index;
      };

      // A vertex defines a polygon corner. The position is in m_coord.
      // All triangles in all groups (meshes) references triangles from this
      // one list.
      class Vertex
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static Vertex * get();
            void release();
            void sprint( std::string & dest );

            double m_coord[3];     // Absolute vertex location
            double m_kfCoord[3];   // Animated position
            bool   m_selected;
            bool   m_visible;
            bool   m_marked;
            bool   m_marked2;

            // If m_free is false, the vertex will be implicitly deleted when
            // all faces using it are deleted
            bool   m_free;

            double * m_drawSource;  // Points to m_coord or m_kfCoord for drawing

            // List of bone joints that move the vertex in skeletal animations.
            InfluenceList m_influences;

            bool propEqual( const Vertex & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==( const Vertex & rhs ) const
               { return propEqual( rhs ); }

         protected:
            Vertex();
            virtual ~Vertex();
            void init();

            static list<Vertex *> s_recycle;
            static int s_allocated;
      };

      // A triangle represents faces in the model. All faces are triangles.
      // The vertices the triangle is attached to are in m_vertexIndices.
      class Triangle
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static Triangle * get();
            void release();
            void sprint( std::string & dest );

            unsigned m_vertexIndices[3];

            // Texture coordinate 0,0 is in the lower left corner of
            // the texture.
            float m_s[3];  // Horizontal, one for each vertex.
            float m_t[3];  // Vertical, one for each vertex.

            double m_finalNormals[3][3];    // Final normals to draw
            double m_vertexNormals[3][3];   // Normals blended for each face attached to the vertex
            double m_flatNormals[3];        // Normal for this triangle
            double m_kfFlatNormals[3];      // Flat normal, rotated relative to the animating bone joints
            double m_kfNormals[3][3];       // Final normals, rotated relative to the animating bone joints
            double * m_flatSource;          // Either m_flatNormals or m_kfFlatNormals
            double * m_normalSource[3];     // Either m_finalNormals or m_kfNormals
            bool  m_selected;
            bool  m_visible;
            bool  m_marked;
            bool  m_marked2;
            bool  m_userMarked;
            int   m_projection;  // Index of texture projection (-1 for none)

            bool propEqual( const Triangle & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==( const Triangle & rhs ) const
               { return propEqual( rhs ); }

         protected:
            Triangle();
            virtual ~Triangle();
            void init();

            static list<Triangle *> s_recycle;
            static int s_allocated;
      };

      // Group of triangles. All triangles in a group share a material (if one
      // is assigned to the Group). Vertices assigned to the triangles may
      // be shared between triangles in different groups. You can change how
      // normals are blended by modifying m_angle and m_smooth.
      class Group
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static Group * get();
            void release();
            void sprint( std::string & dest );

            std::string m_name;
            int         m_materialIndex;    // Material index (-1 for none)
            std::set<int>  m_triangleIndices;  // List of triangles in this group

            // Percentage of blending between flat normals and smooth normals
            // 0 = 0%, 255 = 100%
            uint8_t     m_smooth;

            // Maximum angle around which triangle normals will be blended
            // (ie, if m_angle = 90, triangles with an edge that forms an
            // angle greater than 90 will not be blended).
            uint8_t     m_angle;

            bool        m_selected;
            bool        m_visible;
            bool        m_marked;

            bool propEqual( const Group & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const Group & rhs ) const
               { return propEqual( rhs ); }

         protected:
            Group();
            virtual ~Group();
            void init();

            static list<Group *> s_recycle;
            static int s_allocated;
      };

      // The Material defines how lighting is reflected off of triangles and
      // may include a texture map.
      class Material
      {
         public:
            enum _MaterialType_e
            {
               MATTYPE_TEXTURE  =  0,   // Texture mapped material
               MATTYPE_BLANK    =  1,   // Blank texture, lighting only
               MATTYPE_COLOR    =  2,   // Unused
               MATTYPE_GRADIENT =  3,   // Unused
               MATTYPE_MAX      =  4    // For convenience
            };
            typedef enum _MaterialType_e MaterialTypeE;

            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static Material * get();
            void release();
            void sprint( std::string & dest );

            std::string   m_name;
            MaterialTypeE m_type;         // See MaterialTypeE above

            // Lighting values (RGBA, 0.0 to 1.0)
            float         m_ambient[4];
            float         m_diffuse[4];
            float         m_specular[4];
            float         m_emissive[4];

            // Lighting value 0 to 100.0
            float         m_shininess;

            uint8_t       m_color[4][4];  // Unused


            // The clamp properties determine if the texture map wraps when crossing
            // the 0.0 or 1.0 boundary (false) or if the pixels along the edge are
            // used for coordinates outside the 0.0 - 1.0 limits (true).
            bool          m_sClamp;  // horizontal wrap/clamp
            bool          m_tClamp;  // vertical wrap/clamp

            // Open GL texture index (not actually used in model editing
            // viewports since each viewport needs its own texture index)
            GLuint        m_texture;

            std::string   m_filename;       // Absolute path to texture file (for MATTYPE_TEXTURE)
            std::string   m_alphaFilename;  // Unused
            Texture     * m_textureData;    // Texture data (for MATTYPE_TEXTURE)

            bool propEqual( const Material & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const Material & rhs ) const
               { return propEqual( rhs ); }

         protected:
            Material();
            virtual ~Material();
            void init();

            static list<Material *> s_recycle;
            static int s_allocated;
      };

      // A keyframe for a single joint in a single frame. Keyframes may be rotation or 
      // translation (you can set one without setting the other).
      class Keyframe
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static Keyframe * get();

            void release();
            void sprint( std::string & dest );

            int m_jointIndex;       // Joint that this keyframe affects
            unsigned m_frame;       // Frame number for this keyframe
            double m_time;          // Time for this keyframe in seconds
            double m_parameter[3];  // Translation or rotation (radians), see m_isRotation
            bool   m_isRotation;    // Indicates if m_parameter describes rotation (true) or translation (false)

            bool operator<( const Keyframe & rhs ) const
            {
               return ( this->m_frame < rhs.m_frame || (this->m_frame == rhs.m_frame && !this->m_isRotation && rhs.m_isRotation) );
            };
            bool operator==( const Keyframe & rhs ) const
            {
               return ( this->m_frame == rhs.m_frame && this->m_isRotation == rhs.m_isRotation );
            };
            bool propEqual( const Keyframe & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;

         protected:
            Keyframe();
            virtual ~Keyframe();
            void init();

            static list<Keyframe *> s_recycle;
            static int s_allocated;
      };


      // A bone joint in the skeletal structure, used for skeletal animations.
      // When a vertex or point is assigned to one or more bone joints with a specified
      // weight, the joint is referred to as an Influence.
      class Joint
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static Joint * get();
            void release();
            void sprint( std::string & dest );

            std::string m_name;
            double m_localRotation[3];     // Rotation relative to parent joint (or origin if no parent)
            double m_localTranslation[3];  // Translation relative to parent joint (or origin if no parent)

            Matrix m_absolute;  // Absolute matrix for the joint's original position and orientation
            Matrix m_relative;  // Matrix relative to parent joint
            Matrix m_final;     // Final animated absolute position for bone joint

            int m_currentRotationKeyframe;     // Unused
            int m_currentTranslationKeyframe;  // Unused

            int m_parent;  // Parent joint index (-1 for no parent)

            bool m_selected;
            bool m_visible;
            bool m_marked;

            bool propEqual( const Joint & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const Joint & rhs ) const
               { return propEqual( rhs ); }

         protected:
            Joint();
            virtual ~Joint();
            void init();

            static list<Joint *> s_recycle;
            static int s_allocated;
      };

      class Point
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static Point * get();
            void release();
            void sprint( std::string & dest );

            std::string m_name;
            int m_type;
            double m_trans[3];    // Position
            double m_rot[3];      // Rotation
            double m_kfTrans[3];  // Animated position
            double m_kfRot[3];    // Animated rotation

            // These pointers point to the unanimated or animated properties
            // depending on whether or not the model is animated when it is drawn.
            double * m_drawSource;  // m_trans or m_kfTrans
            double * m_rotSource;   // m_rot or m_kfRot

            bool   m_selected;
            bool   m_visible;
            bool   m_marked;

            // List of bone joints that move the point in skeletal animations.
            InfluenceList m_influences;

            bool propEqual( const Point & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const Point & rhs ) const
               { return propEqual( rhs ); }

         protected:
            Point();
            virtual ~Point();
            void init();

            static list<Point *> s_recycle;
            static int s_allocated;
      };

      // A TextureProjection is used automatically map texture coordinates to a group
      // of vertices. Common projection types are Sphere, Cylinder, and Plane.
      class TextureProjection
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static void stats();
            static TextureProjection * get();
            void release();
            void sprint( std::string & dest );

            std::string m_name;
            int m_type;            // See TextureProjectionTypeE
            double m_pos[3];
            double m_upVec[3];     // Vector that defines "up" for this projection
            double m_seamVec[3];   // Vector that indicates where the texture wraps from 1.0 back to 0.0
            double m_range[2][2];  // min/max, x/y

            bool   m_selected;
            bool   m_marked;

            bool propEqual( const TextureProjection & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const TextureProjection & rhs ) const
               { return propEqual( rhs ); }

         protected:
            TextureProjection();
            virtual ~TextureProjection();
            void init();

            static int s_allocated;
      };

      // TODO: Probably should use a map for the KeyframeList
      typedef sorted_ptr_list<Keyframe *>  KeyframeList;
      typedef vector<KeyframeList>         JointKeyframeList;

      // Describes a skeletal animation.
      class SkelAnim
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static SkelAnim * get();
            void release();
            void releaseData();
            void sprint( std::string & dest );

            std::string m_name;
            JointKeyframeList m_jointKeyframes;
            double   m_fps;  // Frames per second
            double   m_spf;  // Seconds per frame (for convenience, 1.0 / m_fps)
            unsigned m_frameCount;    // Number of frames in the animation
            bool     m_validNormals;  // Whether or not the normals have been calculated for the current animation frame

            bool propEqual( const SkelAnim & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const SkelAnim & rhs ) const
               { return propEqual( rhs ); }

         protected:
            SkelAnim();
            virtual ~SkelAnim();
            void init();

            static list<SkelAnim *> s_recycle;
            static int s_allocated;
      };

      // Describes the position and normal for a vertex in a frame animation.
      class FrameAnimVertex
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static FrameAnimVertex * get();
            void release();
            void sprint( std::string & dest );

            double m_coord[3];
            double m_normal[3];

            bool propEqual( const FrameAnimVertex & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const FrameAnimVertex & rhs ) const
               { return propEqual( rhs ); }

         protected:
            FrameAnimVertex();
            virtual ~FrameAnimVertex();
            void init();

            static list<FrameAnimVertex *> s_recycle;
            static int s_allocated;
      };

      typedef vector<FrameAnimVertex *> FrameAnimVertexList;

      // Describes the position and orientation for a point in a frame animation.
      class FrameAnimPoint
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static FrameAnimPoint * get();
            void release();
            void sprint( std::string & dest );

            double m_trans[3];
            double m_rot[3];

            bool propEqual( const FrameAnimPoint & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const FrameAnimPoint & rhs ) const
               { return propEqual( rhs ); }

         protected:
            FrameAnimPoint();
            virtual ~FrameAnimPoint();
            void init();

            static list<FrameAnimPoint *> s_recycle;
            static int s_allocated;
      };

      typedef vector<FrameAnimPoint *> FrameAnimPointList;

      // Contains the list of vertex positions and point positions for each vertex
      // and point for one animation frame.
      class FrameAnimData
      {
          public:
              FrameAnimData() : m_frameVertices(NULL), m_framePoints(NULL) {}
              FrameAnimVertexList * m_frameVertices;
              FrameAnimPointList  * m_framePoints;

              void releaseData();

              bool propEqual( const FrameAnimData & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
              bool operator==(const FrameAnimData & rhs ) const
               { return propEqual( rhs ); }
      };

      typedef vector< FrameAnimData *> FrameAnimDataList;

      // Describes a frame animation (also known as "Mesh Deformation Animation").
      // This object contains a list of vertex positions for each vertex for every
      // frame (and also every point for every frame).
      class FrameAnim
      {
         public:
            static int flush();
            static int allocated() { return s_allocated; }
            static int recycled() { return s_recycle.size(); }
            static void stats();
            static FrameAnim * get();
            void release();
            void releaseData();
            void sprint( std::string & dest );

            std::string m_name;
            // Each element in m_frameData is one frame. The frames hold lists of
            // all vertex positions and point positions.
            FrameAnimDataList m_frameData;

            double m_fps;  // Frames per second
            bool   m_validNormals;  // Whether or not the normals have been calculated

            bool propEqual( const FrameAnim & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const FrameAnim & rhs ) const
               { return propEqual( rhs ); }

         protected:
            FrameAnim();
            virtual ~FrameAnim();
            void init();

            static list<FrameAnim *> s_recycle;
            static int s_allocated;
      };

      // Working storage for an animated vertex.
      class KeyframeVertex
      {
         public:
            double m_coord[3];
            double m_normal[3];
            double m_rotatedNormal[3];
      };

      // Working storage for an animated triangle.
      class KeyframeTriangle
      {
         public:
            double m_normals[3][3];
      };

      // Reference background images for canvas viewports.
      class BackgroundImage
      {
         public:
            BackgroundImage();
            virtual ~BackgroundImage();

            void sprint( std::string & dest );

            std::string m_filename;
            float m_scale;      // 1.0 means 1 GL unit from the center to the edges of the image
            float m_center[3];  // Point in the viewport where the image is centered

            bool propEqual( const BackgroundImage & rhs, int propBits = PropAll, double tolerance = 0.00001 ) const;
            bool operator==(const BackgroundImage & rhs ) const
               { return propEqual( rhs ); }
      };

      typedef Model::Vertex * VertexPtr;
      typedef Model::Triangle * TrianglePtr;
      typedef Model::Group * GroupPtr;
      typedef Model::Material * MaterialPtr;
      //typedef Model::Joint * JointPtr;

      // FormatData is used to store data that is used by specific file formats
      // that is not understood by MM3D.
      class FormatData
      {
         public:
            FormatData()
               : offsetType( 0 ),
                 format( "" ),
                 index( 0 ),
                 len( 0 ),
                 data( NULL ) { };
            virtual ~FormatData();

            uint16_t      offsetType;  // 0 = none, is valid
            std::string   format;      // Should not be empty
            uint32_t      index;       // for formats with multiple data sets
            uint32_t      len;         // length of data in 'data'
            uint8_t     * data;        // pointer to data

            virtual void serialize();
      };

      // Arbitrary key/value string pairs. This is used to provide a simple interface
      // for model attributes that are not manipulated by commands/tools. Often
      // user-editable format-specific data is stored as MetaData key/value pairs
      // (for example texture paths in MD2 and MD3 models).
      class MetaData
      {
         public:
            std::string key;
            std::string value;
      };
      typedef std::vector< MetaData > MetaDataList;

      // See errorToString()
      enum _ModelError_e
      {
         ERROR_NONE = 0,
         ERROR_CANCEL,
         ERROR_UNKNOWN_TYPE,
         ERROR_UNSUPPORTED_OPERATION,
         ERROR_BAD_ARGUMENT,
         ERROR_NO_FILE,
         ERROR_NO_ACCESS,
         ERROR_FILE_OPEN,
         ERROR_FILE_READ,
         ERROR_BAD_MAGIC,
         ERROR_UNSUPPORTED_VERSION,
         ERROR_BAD_DATA,
         ERROR_UNEXPECTED_EOF,
         ERROR_FILTER_SPECIFIC,
         ERROR_EXPORT_ONLY,
         ERROR_UNKNOWN
      };
      typedef enum _ModelError_e ModelErrorE;

      enum _SelectionMode_e
      {
         SelectNone,
         SelectVertices,
         SelectTriangles,
         SelectConnected,
         SelectGroups,
         SelectJoints,
         SelectPoints,
         SelectProjections,
      };
      typedef enum _SelectionMode_e SelectionModeE;

      // Canvas drawing projections (not related to TextureProjections)
      enum _ProjectionDirection_e
      {
         View3d = 0,
         ViewFront,
         ViewBack,
         ViewLeft,
         ViewRight,
         ViewTop,
         ViewBottom
      };
      typedef enum _ProjectionDirection_e ProjectionDirectionE;

      enum
      {
         MAX_GROUP_NAME_LEN = 32,
         MAX_BACKGROUND_IMAGES = 6
      };

      // Drawing options. These are defined as powers of 2 so that they
      // can be combined with a bitwise OR.
      enum
      {
         DO_NONE           = 0x00,
         DO_TEXTURE        = 0x01,
         DO_SMOOTHING      = 0x02,  // Normal smoothing/blending between faces
         DO_WIREFRAME      = 0x04,
         DO_BADTEX         = 0x08,  // Render bad textures, or render as no texture
         DO_ALPHA          = 0x10,  // Do alpha blending
         DO_BACKFACECULL   = 0x20,  // Do not render triangles that face away from the camera
      };

      enum _AnimationMode_e
      {
         ANIMMODE_NONE = 0,
         ANIMMODE_SKELETAL,
         ANIMMODE_FRAME,
         ANIMMODE_FRAMERELATIVE,  // Not implemented
         ANIMMODE_MAX
      };
      typedef enum _AnimationMode_e AnimationModeE;

      enum _DrawJointMode_e
      {
         JOINTMODE_NONE = 0,
         JOINTMODE_LINES,
         JOINTMODE_BONES,
         JOINTMODE_MAX
      };
      typedef enum _DrawJointMode_e DrawJointModeE;

      enum _AnimationMerge_e
      {
         AM_NONE = 0,
         AM_ADD,
         AM_MERGE
      };
      typedef enum _AnimationMerge_e AnimationMergeE;

      enum _BooleanOp_e
      {
         BO_Union,
         BO_UnionRemove,
         BO_Subtraction,
         BO_Intersection,
         BO_MAX
      };
      typedef enum _BooleanOp_e BooleanOpE;

#ifdef MM3D_EDIT
      // Register an observer if you have an object that must be notified when the
      // model changes. The modelChanged function will be called with changeBits
      // set to describe (in general terms) what changed. See ChangeBits.
      class Observer
      {
         public:
            virtual ~Observer();
            virtual void modelChanged( int changeBits ) = 0;
      };
      typedef std::list< Observer * > ObserverList;
#endif // MM3D_EDIT

      // ==================================================================
      // Public methods
      // ==================================================================

      Model();
      virtual ~Model();

      static const char * errorToString( Model::ModelErrorE, Model * model = NULL );
      static bool operationFailed( Model::ModelErrorE );

      // Returns if models are visually equivalent
      bool equivalent( const Model * model, double tolerance = 0.00001 ) const;

      // Compares if two models are equal. Returns true of all specified
      // properties of all specified parts match. See ComparePartsE and
      // PartPropertiesE.
      bool propEqual( const Model * model, int partBits = PartAll, int propBits = PropAll,
            double tolerance = 0.00001 ) const;

      void sprint( std::string & dest );

      // ------------------------------------------------------------------
      // "Meta" data, model information that is not rendered in a viewport.
      // ------------------------------------------------------------------

      // Indicates if the model has changed since the last time it was saved.
      void setSaved( bool o ) { if ( o ) { m_undoMgr->setSaved(); }; };
      bool getSaved() const   { return m_undoMgr->isSaved(); };

      const char * getFilename() const { return m_filename.c_str(); };
      void setFilename( const char * filename ) { 
            if ( filename && filename[0] ) { m_filename = filename; } };

      const char * getExportFile() const { return m_exportFile.c_str(); };
      void setExportFile( const char * filename ) { 
            if ( filename && filename[0] ) { m_exportFile = filename; } };

      void setFilterSpecificError( const char * str ) { s_lastFilterError = m_filterSpecificError = str; };
      const char * getFilterSpecificError() const { return m_filterSpecificError.c_str(); };
      static const char * getLastFilterSpecificError() { return s_lastFilterError.c_str(); };

      // Observers are notified when the model changes. See the Observer class
      // for details.
      void addObserver( Observer * o );
      void removeObserver( Observer * o );

      // See the MetaData class.
      void addMetaData( const char * key, const char * value );
      void updateMetaData( const char * key, const char * value );

      // Look-up by key
      bool getMetaData( const char * key, char * value, size_t valueLen ) const;
      // Look-up by index
      bool getMetaData( unsigned int index, char * key, size_t keyLen, char * value, size_t valueLen ) const;

      unsigned int getMetaDataCount() const;
      void clearMetaData();
      void removeLastMetaData();  // For undo only!

      // Background image accessors. See the BackgroundImage class.
      void setBackgroundImage( unsigned index, const char * str );
      void setBackgroundScale( unsigned index, float scale );
      void setBackgroundCenter( unsigned index, float x, float y, float z );

      const char * getBackgroundImage( unsigned index ) const;
      float getBackgroundScale( unsigned index ) const;
      void getBackgroundCenter( unsigned index, float & x, float & y, float & z ) const;

      // These are used to store status messages when the model does not have a status
      // bar. When a model is assigned to a viewport window, the messages will be
      // displayed in the status bar.
      bool hasErrors() const { return !m_loadErrors.empty(); }
      void pushError( const std::string & err );
      std::string popError();

      // Use these functions to preserve data that Misfit doesn't support natively
      int  addFormatData( FormatData * fd );
      bool deleteFormatData( unsigned index );
      unsigned getFormatDataCount() const;
      FormatData * getFormatData( unsigned index ) const;
      FormatData * getFormatDataByFormat( const char * format, unsigned index = 0 ) const; // not case sensitive

      // ------------------------------------------------------------------
      // Rendering functions
      // ------------------------------------------------------------------

      // Functions for rendering the model in a viewport
      void draw( unsigned drawOptions = DO_TEXTURE, ContextT context = NULL, float *viewPoint = NULL );
      void drawLines();
      void drawVertices();
      void drawPoints();
      void drawProjections();
      void drawJoints();

      void setCanvasDrawMode( int m ) { m_canvasDrawMode = m; };
      int  getCanvasDrawMode() const { return m_canvasDrawMode; };

      void setDrawJoints( DrawJointModeE m ) { m_drawJoints = m; };
      DrawJointModeE getDrawJoints() const { return m_drawJoints; };

      void setDrawProjections( bool o ) { m_drawProjections = o; };
      bool getDrawProjections() const { return m_drawProjections; };

      // Open GL needs textures allocated for each viewport that renders the textures.
      // A ContextT associates a set of OpenGL textures with a viewport.
      bool loadTextures( ContextT context = NULL );
      void removeContext( ContextT context );

      // Forces a reload and re-initialization of all textures in all
      // viewports (contexts)
      void invalidateTextures();

      // Counter that indicates how many OpenGL textures are currently allocated.
      static int    s_glTextures;

      // The local matrix is used by non-mm3d applications that use this
      // library to render MM3D models. In MM3D it should always be set to the
      // identity matrix.  Other apps use it to move an animated model to a new
      // location in space, it only affects rendering of skeletal animations.
      void setLocalMatrix( const Matrix & m ) { m_localMatrix = m; };

      // ------------------------------------------------------------------
      // Animation functions
      // ------------------------------------------------------------------

      bool setCurrentAnimation( AnimationModeE m, const char * name );
      bool setCurrentAnimation( AnimationModeE m, unsigned index );
      bool setCurrentAnimationFrame( unsigned frame );
      bool setCurrentAnimationTime( double time );

      unsigned getCurrentAnimation() const;
      unsigned getCurrentAnimationFrame() const;
      double   getCurrentAnimationTime() const;

      void setAnimationLooping( bool o );
      bool isAnimationLooping() const;

      // Stop animation mode, go back to standard pose editing.
      void setNoAnimation();

      AnimationModeE getAnimationMode() const { return m_animationMode; };
      bool inSkeletalMode() const { return (m_animationMode == ANIMMODE_SKELETAL); };

      // Common animation properties
      int addAnimation( AnimationModeE mode, const char * name );
      void deleteAnimation( AnimationModeE mode, unsigned index );

      unsigned getAnimCount( AnimationModeE m ) const;

      const char * getAnimName( AnimationModeE mode, unsigned anim ) const;
      bool setAnimName( AnimationModeE mode, unsigned anim, const char * name );

      double getAnimFPS( AnimationModeE mode, unsigned anim ) const;
      bool setAnimFPS( AnimationModeE mode, unsigned anim, double fps );

      unsigned getAnimFrameCount( AnimationModeE mode, unsigned anim ) const;
      bool setAnimFrameCount( AnimationModeE mode, unsigned anim, unsigned count );

      bool clearAnimFrame( AnimationModeE mode, unsigned anim, unsigned frame );

      // Frame animation geometry
      void setFrameAnimVertexCount( unsigned vertexCount );
      void setFrameAnimPointCount( unsigned pointCount );

      bool setFrameAnimVertexCoords( unsigned anim, unsigned frame, unsigned vertex, 
            double x, double y, double z );
      bool getFrameAnimVertexCoords( unsigned anim, unsigned frame, unsigned vertex, 
            double & x, double & y, double & z ) const;
      bool getFrameAnimVertexNormal( unsigned anim, unsigned frame, unsigned vertex, 
            double & x, double & y, double & z ) const;

      // Not undo-able
      bool setQuickFrameAnimVertexCoords( unsigned anim, unsigned frame, unsigned vertex, 
            double x, double y, double z );

      bool setFrameAnimPointCoords( unsigned anim, unsigned frame, unsigned point, 
            double x, double y, double z );
      bool getFrameAnimPointCoords( unsigned anim, unsigned frame, unsigned point, 
            double & x, double & y, double & z ) const;

      bool setFrameAnimPointRotation( unsigned anim, unsigned frame, unsigned point, 
            double x, double y, double z );
      bool getFrameAnimPointRotation( unsigned anim, unsigned frame, unsigned point, 
            double & x, double & y, double & z ) const;

      // Skeletal animation keyframs
      int  setSkelAnimKeyframe( unsigned anim, unsigned frame, unsigned joint, bool isRotation, 
            double x, double y, double z );
      bool getSkelAnimKeyframe( unsigned anim, unsigned frame,
            unsigned joint, bool isRotation,
            double & x, double & y, double & z ) const;

      bool hasSkelAnimKeyframe( unsigned anim, unsigned frame,
            unsigned joint, bool isRotation ) const;

      bool deleteSkelAnimKeyframe( unsigned anim, unsigned frame, unsigned joint, bool isRotation );

      // Interpolate what a keyframe for this joint would be at the specified frame.
      bool interpSkelAnimKeyframe( unsigned anim, unsigned frame,
            bool loop, unsigned joint, bool isRotation,
            double & x, double & y, double & z ) const;
      // Interpolate what a keyframe for this joint would be at the specified time.
      bool interpSkelAnimKeyframeTime( unsigned anim, double frameTime,
            bool loop, unsigned joint,
            Matrix & relativeFinal ) const;

      // Animation set operations
      int  copyAnimation( AnimationModeE mode, unsigned anim, const char * newName );
      int  splitAnimation( AnimationModeE mode, unsigned anim, const char * newName, unsigned frame );
      bool joinAnimations( AnimationModeE mode, unsigned anim1, unsigned anim2 );
      bool mergeAnimations( AnimationModeE mode, unsigned anim1, unsigned anim2 );
      int  convertAnimToFrame( AnimationModeE mode, unsigned anim1, const char * newName, unsigned frameCount );

      bool moveAnimation( AnimationModeE mode, unsigned oldIndex, unsigned newIndex );

      // For undo, don't call these directly
      bool insertSkelAnimKeyframe( unsigned anim, Keyframe * keyframe );
      bool removeSkelAnimKeyframe( unsigned anim, unsigned frame, unsigned joint, bool isRotation, bool release = false );

      // Merge all animations from model into this model.
      // For skeletal, skeletons must match
      bool mergeAnimations( Model * model );

      // When a model has frame animations, adding or removing primitives is disabled
      // (because of the large amount of undo information that would have to be stored).
      // Use this function to force an add or remove (you must make sure that you adjust
      // the frame animations appropriately as well as preventing an undo on the
      // operation you are performing).
      //
      // In other words, setting this to true is probably a really bad idea unless
      // you know what you're doing.
      void forceAddOrDelete( bool o );

      bool canAddOrDelete() const { return (m_frameAnims.size() == 0 || m_forceAddOrDelete); };

      // Show an error because the user tried to add or remove primitives while
      // the model has frame animations.
      void displayFrameAnimPrimitiveError();

      int getNumFrames() const;  // Deprecated

      // ------------------------------------------------------------------
      // Normal functions
      // ------------------------------------------------------------------

      bool getNormal( unsigned triangleNum, unsigned vertexIndex, float *normal ) const;
      bool getFlatNormal( unsigned triangleNum, float *normal ) const;
      float cosToPoint( unsigned triangleNum, double * point ) const;

      void calculateNormals();
      void calculateSkelNormals();
      void calculateFrameNormals( unsigned anim );
      void invalidateNormals();

      void invertNormals( unsigned triangleNum );
      bool triangleFacesIn( unsigned triangleNum );

      // ------------------------------------------------------------------
      // Geometry functions
      // ------------------------------------------------------------------

      inline int getVertexCount()     const { return m_vertices.size(); }
      inline int getTriangleCount()   const { return m_triangles.size(); }
      inline int getBoneJointCount()  const { return m_joints.size(); }
      inline int getPointCount()      const { return m_points.size(); }
      inline int getProjectionCount() const { return m_projections.size(); }

      bool getPositionCoords( const Position & pos, double * coord ) const;

      int addVertex( double x, double y, double z );
      int addTriangle( unsigned vert1, unsigned vert2, unsigned vert3 );

      void deleteVertex( unsigned vertex );
      void deleteTriangle( unsigned triangle );

      // No undo on this one
      void setVertexFree( unsigned v, bool o );
      bool isVertexFree( unsigned v ) const;

      // When all faces attached to a vertex are deleted, the vertex is considered
      // an "orphan" and deleted (unless it is a "free" vertex, see m_free in the
      // vertex class).
      void deleteOrphanedVertices();

      // A flattened triangle is a triangle with two or more corners that are
      // assigned to the same vertex (this usually happens when vertices are
      // welded together).
      void deleteFlattenedTriangles();

      bool isTriangleMarked( unsigned t ) const;

      void subdivideSelectedTriangles();
      void unsubdivideTriangles( unsigned t1, unsigned t2, unsigned t3, unsigned t4 );

      // If co-planer triangles have edges with points that are co-linear they
      // can be combined into a single triangle. The simplifySelectedMesh function
      // performs this operation to combine all faces that do not add detail to
      // the model.
      void simplifySelectedMesh();

      bool setTriangleVertices( unsigned triangleNum, unsigned vert1, unsigned vert2, unsigned vert3 );
      bool getTriangleVertices( unsigned triangleNum, unsigned & vert1, unsigned & vert2, unsigned & vert3 ) const;
      void setTriangleMarked( unsigned triangleNum, bool marked );
      void clearMarkedTriangles();

      bool getVertexCoordsUnanimated( unsigned vertexNumber, double *coord ) const;
      bool getVertexCoords( unsigned vertexNumber, double *coord ) const;
      bool getVertexCoords2d( unsigned vertexNumber, ProjectionDirectionE dir, double *coord ) const;

      int getTriangleVertex( unsigned triangleNumber, unsigned vertexIndex ) const;

      void booleanOperation( BooleanOpE op, 
            std::list<int> & listA, std::list<int> & listB );

      Model * copySelected() const;

      // A BSP tree is calculated for triangles that have textures with an alpha
      // channel (transparency). It is used to determine in what order triangles
      // must be rendered to get the proper blending results (triangles that are
      // farther away from the camera are rendered first so that closer triangles
      // are drawn on top of them).
      void calculateBspTree();
      void invalidateBspTree();

      // The model argument should be const, but it calls setupJoints.
      // TODO: Calling setupJoints in here should not be necessary.
      bool mergeModels( Model * model, bool textures, AnimationMergeE mergeMode, bool emptyGroups,
            double * trans = NULL, double * rot = NULL );

      // These are helper functions for the boolean operations
      void removeInternalTriangles( 
            std::list<int> & listA, std::list<int> & listB );
      void removeExternalTriangles( 
            std::list<int> & listA, std::list<int> & listB );
      void removeBTriangles( 
            std::list<int> & listA, std::list<int> & listB );

      // ------------------------------------------------------------------
      // Group and material functions
      // ------------------------------------------------------------------

      // TODO: Misnamed, should be getMaterialCount()
      inline int getTextureCount() const { return m_materials.size(); };

      int addGroup( const char * name );

      // Textures and Color materials go into the same material list
      int addTexture( Texture * tex );
      int addColorMaterial( const char * name );

      bool setGroupTextureId( unsigned groupNumber, int textureId );

      void deleteGroup( unsigned group );
      void deleteTexture( unsigned texture );

      const char * getGroupName( unsigned groupNum ) const;
      bool setGroupName( unsigned groupNum, const char * groupName );

      inline int getGroupCount() const { return m_groups.size(); };
      int getGroupByName( const char * groupName, bool ignoreCase = false ) const;
      int getMaterialByName( const char * materialName, bool ignoreCase = false ) const;
      Material::MaterialTypeE getMaterialType( unsigned materialIndex ) const;
      int getMaterialColor( unsigned materialIndex, unsigned c, unsigned v = 0 ) const;

      // These implicitly change the material type.
      void setMaterialTexture( unsigned textureId, Texture * tex );
      void removeMaterialTexture( unsigned textureId );

      uint8_t getGroupSmooth( unsigned groupNum ) const;
      bool setGroupSmooth( unsigned groupNum, uint8_t smooth );
      uint8_t getGroupAngle( unsigned groupNum ) const;
      bool setGroupAngle( unsigned groupNum, uint8_t angle );

      void setTextureName( unsigned textureId, const char * name );
      const char * getTextureName( unsigned textureId ) const;

      const char * getTextureFilename( unsigned textureId ) const;
      Texture * getTextureData( unsigned textureId );
      const Texture * getTextureData( unsigned textureId ) const { return getTextureData( textureId ); }

      // Lighting accessors
      bool getTextureAmbient(   unsigned textureId,       float * ambient   ) const;
      bool getTextureDiffuse(   unsigned textureId,       float * diffuse   ) const;
      bool getTextureEmissive(  unsigned textureId,       float * emissive  ) const;
      bool getTextureSpecular(  unsigned textureId,       float * specular  ) const;
      bool getTextureShininess( unsigned textureId,       float & shininess ) const;

      bool setTextureAmbient(   unsigned textureId, const float * ambient   );
      bool setTextureDiffuse(   unsigned textureId, const float * diffuse   );
      bool setTextureEmissive(  unsigned textureId, const float * emissive  );
      bool setTextureSpecular(  unsigned textureId, const float * specular  );
      bool setTextureShininess( unsigned textureId, float shininess );

      // See the clamp property in the Material class.
      bool getTextureSClamp( unsigned textureId ) const;
      bool getTextureTClamp( unsigned textureId ) const;
      bool setTextureSClamp( unsigned textureId, bool clamp );
      bool setTextureTClamp( unsigned textureId, bool clamp );

      list<int> getUngroupedTriangles() const;
      list<int> getGroupTriangles( unsigned groupNumber ) const;
      int       getGroupTextureId( unsigned groupNumber ) const;

      int getTriangleGroup( unsigned triangleNumber ) const;

      void addTriangleToGroup( unsigned groupNum, unsigned triangleNum );
      void removeTriangleFromGroup( unsigned groupNum, unsigned triangleNum );

      void setSelectedAsGroup( unsigned groupNum );
      void addSelectedToGroup( unsigned groupNum );

      bool getTextureCoords( unsigned triangleNumber, unsigned vertexIndex, float & s, float & t ) const;
      bool setTextureCoords( unsigned triangleNumber, unsigned vertexIndex, float s, float t );

      // ------------------------------------------------------------------
      // Skeletal structure and influence functions
      // ------------------------------------------------------------------

      int addBoneJoint( const char * name, double x, double y, double z, 
            double xrot, double yrot, double zrot,
            int parent = -1 );

      void deleteBoneJoint( unsigned joint );

      const char * getBoneJointName( unsigned joint ) const;
      int getBoneJointParent( unsigned joint ) const;
      bool getBoneJointCoords( unsigned jointNumber, double * coord ) const;

      bool getBoneJointFinalMatrix( unsigned jointNumber, Matrix & m ) const;
      bool getBoneJointAbsoluteMatrix( unsigned jointNumber, Matrix & m ) const;
      bool getBoneJointRelativeMatrix( unsigned jointNumber, Matrix & m ) const;
      bool getPointFinalMatrix( unsigned jointNumber, Matrix & m ) const;

      list<int> getBoneJointVertices( int joint ) const;
      int getVertexBoneJoint( unsigned vertexNumber ) const;
      int getPointBoneJoint( unsigned point ) const;

      bool setPositionBoneJoint( const Position & pos, int joint );
      bool setVertexBoneJoint( unsigned vertex, int joint );
      bool setPointBoneJoint( unsigned point, int joint );

      bool addPositionInfluence( const Position & pos, unsigned joint, InfluenceTypeE type, double weight );
      bool addVertexInfluence( unsigned vertex, unsigned joint, InfluenceTypeE type, double weight );
      bool addPointInfluence( unsigned point, unsigned joint, InfluenceTypeE type, double weight );

      bool removePositionInfluence( const Position & pos, unsigned joint );
      bool removeVertexInfluence( unsigned vertex, unsigned joint );
      bool removePointInfluence( unsigned point, unsigned joint );

      bool removeAllPositionInfluences( const Position & pos );
      bool removeAllVertexInfluences( unsigned vertex );
      bool removeAllPointInfluences( unsigned point );

      bool getPositionInfluences( const Position & pos, InfluenceList & l ) const;
      bool getVertexInfluences( unsigned vertex, InfluenceList & l ) const;
      bool getPointInfluences( unsigned point, InfluenceList & l ) const;

      int getPrimaryPositionInfluence( const Position & pos ) const;
      int getPrimaryVertexInfluence( unsigned vertex ) const;
      int getPrimaryPointInfluence( unsigned point ) const;

      bool setPositionInfluenceType( const Position & pos, unsigned joint, InfluenceTypeE type );
      bool setVertexInfluenceType( unsigned vertex, unsigned joint, InfluenceTypeE type );
      bool setPointInfluenceType( unsigned point, unsigned joint, InfluenceTypeE type );

      bool setPositionInfluenceWeight( const Position & pos, unsigned joint, double weight );
      bool setVertexInfluenceWeight( unsigned vertex, unsigned joint, double weight );
      bool setPointInfluenceWeight( unsigned point, unsigned joint, double weight );

      bool autoSetPositionInfluences( const Position & pos, double sensitivity, bool selected );
      bool autoSetVertexInfluences( unsigned vertex, double sensitivity, bool selected );
      bool autoSetPointInfluences( unsigned point, double sensitivity, bool selected );
      bool autoSetCoordInfluences( double * coord, double sensitivity, bool selected, std::list<int> & infList );

      bool setBoneJointName( unsigned joint, const char * name );
      bool setBoneJointParent( unsigned joint, int parent = -1 );
      bool setBoneJointRotation( unsigned j, const double * rot );
      bool setBoneJointTranslation( unsigned j, const double * trans );

      double calculatePositionInfluenceWeight( const Position & pos, unsigned joint ) const;
      double calculateVertexInfluenceWeight( unsigned vertex, unsigned joint ) const;
      double calculatePointInfluenceWeight( unsigned point, unsigned joint ) const;
      double calculateCoordInfluenceWeight( const double * coord, unsigned joint ) const;

      void calculateRemainderWeight( InfluenceList & list ) const;

      bool getBoneVector( unsigned joint, double * vec, const double * coord ) const;

      // No undo on this one
      bool relocateBoneJoint( unsigned j, double x, double y, double z );

      // Call this when a bone joint is moved, rotated, added, or deleted to
      // recalculate the skeletal structure.
      void setupJoints();

      // ------------------------------------------------------------------
      // Point functions
      // ------------------------------------------------------------------

      int addPoint( const char * name, double x, double y, double z, 
            double xrot, double yrot, double zrot,
            int boneId = -1 );

      void deletePoint( unsigned point );

      int getPointByName( const char * name ) const;

      const char * getPointName( unsigned point ) const;
      bool setPointName( unsigned point, const char * name );

      int getPointType( unsigned point ) const;
      bool setPointType( unsigned point, int type );

      // TODO: Orientation and Rotation are used for different purposes.
      // If it's safe to remove one in favor of the other, that should be done.
      bool getPointCoords( unsigned pointNumber, double * coord ) const;
      bool getPointOrientation( unsigned pointNumber, double * rot ) const;
      bool getPointRotation( unsigned point, double * rot ) const;
      bool getPointTranslation( unsigned point, double * trans ) const;

      bool setPointRotation( unsigned point, const double * rot );
      bool setPointTranslation( unsigned point, const double * trans );

      // ------------------------------------------------------------------
      // Texture projection functions
      // ------------------------------------------------------------------

      int addProjection( const char * name, int type, double x, double y, double z );
      void deleteProjection( unsigned proj );

      const char * getProjectionName( unsigned proj ) const;

      void   setProjectionScale( unsigned p, double scale );
      double getProjectionScale( unsigned p ) const;

      bool setProjectionName( unsigned proj, const char * name );
      bool setProjectionType( unsigned proj, int type );
      int  getProjectionType( unsigned proj ) const;
      bool setProjectionRotation( unsigned proj, int type );
      int  getProjectionRotation( unsigned proj ) const;
      bool getProjectionCoords( unsigned projNumber, double *coord ) const;

      bool setProjectionUp( unsigned projNumber, const double *coord );
      bool setProjectionSeam( unsigned projNumber, const double *coord );
      bool setProjectionRange( unsigned projNumber, 
            double xmin, double ymin, double xmax, double ymax );

      bool getProjectionUp( unsigned projNumber, double *coord ) const;
      bool getProjectionSeam( unsigned projNumber, double *coord ) const;
      bool getProjectionRange( unsigned projNumber, 
            double & xmin, double & ymin, double & xmax, double & ymax ) const;

      void setTriangleProjection( unsigned triangleNum, int proj );
      int  getTriangleProjection( unsigned triangleNum ) const;

      void applyProjection( unsigned int proj );

      // ------------------------------------------------------------------
      // Undo/Redo functions
      // ------------------------------------------------------------------

      bool setUndoEnabled( bool o );

      // Indicates that a user-specified operation is complete. A single
      // "operation" may span many function calls and different types of
      // manipulations.
      void operationComplete( const char * opname = NULL );

      // Clear undo list
      void clearUndo();

      bool canUndo() const;
      bool canRedo() const;
      void undo();
      void redo();

      // If a manipulations have been performed, but not commited as a single
      // undo list, undo those operations (often used when the user clicks
      // a "Cancel" button to discard "unapplied" changes).
      void undoCurrent();

      const char * getUndoOpName() const;
      const char * getRedoOpName() const;

      // The limits at which undo operations are removed from memory.
      void setUndoSizeLimit( unsigned sizeLimit );
      void setUndoCountLimit( unsigned countLimit );

      // These functions are for undo and features such as this.
      // Don't use them unless you are me.
      void insertVertex( unsigned index, Vertex * vertex );
      void removeVertex( unsigned index );

      void insertTriangle( unsigned index, Triangle * triangle );
      void removeTriangle( unsigned index );

      void insertGroup( unsigned index, Group * group );
      void removeGroup( unsigned index );

      void insertBoneJoint( unsigned index, Joint * joint );
      void removeBoneJoint( unsigned index );

      void insertInfluence( const Position & pos, unsigned index, const InfluenceT & influence );
      void removeInfluence( const Position & pos, unsigned index );

      void insertPoint( unsigned index, Point * point );
      void removePoint( unsigned index );

      void insertProjection( unsigned index, TextureProjection * proj );
      void removeProjection( unsigned index );

      void insertTexture( unsigned index, Material * material );
      void removeTexture( unsigned index );

      void insertFrameAnim( unsigned index, FrameAnim * anim );
      void removeFrameAnim( unsigned index );

      void insertSkelAnim( unsigned anim, SkelAnim * fa );
      void removeSkelAnim( unsigned anim );

      void insertFrameAnimFrame( unsigned anim, unsigned frame,
            FrameAnimData * data );
      void removeFrameAnimFrame( unsigned anim, unsigned frame );

      // ------------------------------------------------------------------
      // Selection functions
      // ------------------------------------------------------------------

      void setSelectionMode( SelectionModeE m );
      inline SelectionModeE getSelectionMode() { return m_selectionMode; };

      unsigned getSelectedVertexCount() const;
      unsigned getSelectedTriangleCount() const;
      unsigned getSelectedBoneJointCount() const;
      unsigned getSelectedPointCount() const;
      unsigned getSelectedProjectionCount() const;

      void getSelectedPositions( list<Position> & l ) const;
      void getSelectedVertices( list<int> & l ) const;
      void getSelectedTriangles( list<int> & l ) const;
      void getSelectedGroups( list<int> & l ) const;
      void getSelectedBoneJoints( list<int> & l ) const;
      void getSelectedPoints( list<int> & l ) const;
      void getSelectedProjections( list<int> & l ) const;

      bool unselectAll();

      bool unselectAllVertices();
      bool unselectAllTriangles();
      bool unselectAllGroups();
      bool unselectAllBoneJoints();
      bool unselectAllPoints();
      bool unselectAllProjections();

      // A selection test is an additional condition you can attach to whether
      // or not an object in the selection volume should be selected. For example,
      // this is used to add a test for which way a triangle is facing so that
      // triangles not facing the camera can be excluded from the selection.
      // You can implement a selection test for any property that you can check
      // on the primitive.
      class SelectionTest
      {
         public:
            virtual ~SelectionTest() {};
            virtual bool shouldSelect( void * element ) = 0;
      };

      bool selectInVolumeMatrix( const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test = NULL );
      bool unselectInVolumeMatrix( const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test = NULL );

      bool getBoundingRegion( double *x1, double *y1, double *z1, double *x2, double *y2, double *z2 ) const;
      bool getSelectedBoundingRegion( double *x1, double *y1, double *z1, double *x2, double *y2, double *z2 ) const;

      void deleteSelected();

      // When changing the selection state of a lot of primitives, a difference
      // list is used to track undo information. These calls indicate when the
      // undo information should start being tracked and when it should be
      // completed.
      void beginSelectionDifference();
      void endSelectionDifference();

      bool selectVertex( unsigned v );
      bool unselectVertex( unsigned v );
      bool isVertexSelected( unsigned v ) const;

      bool selectTriangle( unsigned t );
      bool unselectTriangle( unsigned t );
      bool isTriangleSelected( unsigned t ) const;

      bool selectGroup( unsigned g );
      bool unselectGroup( unsigned g );
      bool isGroupSelected( unsigned g ) const;

      bool selectBoneJoint( unsigned j );
      bool unselectBoneJoint( unsigned j );
      bool isBoneJointSelected( unsigned j ) const;

      bool selectPoint( unsigned p );
      bool unselectPoint( unsigned p );
      bool isPointSelected( unsigned p ) const;

      bool selectProjection( unsigned p );
      bool unselectProjection( unsigned p );
      bool isProjectionSelected( unsigned p ) const;

      // The behavior of this function changes based on the selection mode.
      bool invertSelection();

      // Select all vertices that have the m_free property set and are not used
      // by any triangles (this can be used to clean up unused free vertices,
      // like a manual analog to the deleteOrphanedVertices() function).
      void selectFreeVertices();

      void setSelectedUv( const vector<int> & uvList );
      void getSelectedUv( vector<int> & uvList ) const;
      void clearSelectedUv();

      // ------------------------------------------------------------------
      // Hide functions
      // ------------------------------------------------------------------

      bool hideSelected();
      bool hideUnselected();
      bool unhideAll();

      bool isVertexVisible( unsigned v ) const;
      bool isTriangleVisible( unsigned t ) const;
      bool isGroupVisible( unsigned g ) const;
      bool isBoneJointVisible( unsigned j ) const;
      bool isPointVisible( unsigned p ) const;

      // Don't call these directly... use selection/hide selection
      bool hideVertex( unsigned );
      bool unhideVertex( unsigned );
      bool hideTriangle( unsigned );
      bool unhideTriangle( unsigned );
      bool hideJoint( unsigned );
      bool unhideJoint( unsigned );
      bool hidePoint( unsigned );
      bool unhidePoint( unsigned );

      // ------------------------------------------------------------------
      // Transform functions
      // ------------------------------------------------------------------

      bool movePosition( const Position & pos, double x, double y, double z );
      bool moveVertex( unsigned v, double x, double y, double z );
      bool moveBoneJoint( unsigned j, double x, double y, double z );
      bool movePoint( unsigned p, double x, double y, double z );
      bool moveProjection( unsigned p, double x, double y, double z );

      void translateSelected( const Matrix & m );
      void rotateSelected( const Matrix & m, double * point );

      // Applies arbitrary matrix to primitives (selected or all based on scope).
      // Some matrices cannot be undone (consider a matrix that scales a dimension to 0).
      // If the matrix cannot be undone, set undoable to false (a matrix is undoable if
      // the determinant is not equal to zero).
      void applyMatrix( const Matrix & m, OperationScopeE scope, bool animations, bool undoable );

   protected:

      // ==================================================================
      // Protected methods
      // ==================================================================

      // ------------------------------------------------------------------
      // Texture context methods
      // ------------------------------------------------------------------

      DrawingContext * getDrawingContext( ContextT context );
      void deleteGlTextures( ContextT context );

      // If any group is using material "id", set the group to having
      // no texture (used when materials are deleted).
      void noTexture( unsigned id );

      // ------------------------------------------------------------------
      // Book-keeping for add/delete undo
      // ------------------------------------------------------------------

      void adjustVertexIndices( unsigned index, int count );
      void adjustTriangleIndices( unsigned index, int count );
      void adjustProjectionIndices( unsigned index, int count );

      // ------------------------------------------------------------------
      // Texture projections
      // ------------------------------------------------------------------

      void applyCylinderProjection( unsigned int proj );
      void applySphereProjection( unsigned int proj );
      void applyPlaneProjection( unsigned int proj );

      // ------------------------------------------------------------------
      // Hiding/visibility
      // ------------------------------------------------------------------

      void beginHideDifference();
      void endHideDifference();

      // When primitives of one type are hidden, other primitives may need to
      // be hidden at the same time.
      void hideVerticesFromTriangles();
      void unhideVerticesFromTriangles();

      void hideTrianglesFromVertices();
      void unhideTrianglesFromVertices();

      // ------------------------------------------------------------------
      // Selection
      // ------------------------------------------------------------------

      bool selectVerticesInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );
      bool selectTrianglesInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, bool connected, SelectionTest * test = NULL );
      bool selectGroupsInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );
      bool selectBoneJointsInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );
      bool selectPointsInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );
      bool selectProjectionsInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );

      // When primitives of one type are selected, other primitives may need to
      // be selected at the same time.
      void selectTrianglesFromGroups();
      void selectVerticesFromTriangles();
      void selectTrianglesFromVertices( bool all = true );
      void selectGroupsFromTriangles( bool all = true );

      bool parentJointSelected( int joint ) const;
      bool directParentJointSelected( int joint ) const;

      // ------------------------------------------------------------------
      // Undo
      // ------------------------------------------------------------------

      void sendUndo( Undo * undo, bool listCombine = false );

      // ------------------------------------------------------------------
      // Meta
      // ------------------------------------------------------------------

      void updateObservers();

      // ------------------------------------------------------------------
      // Protected data
      // ------------------------------------------------------------------

      // See the various accessors for this data for details on what these
      // properties mean.

      bool          m_initialized;
      std::string   m_filename;
      std::string   m_exportFile;
      std::string   m_filterSpecificError;
      static std::string   s_lastFilterError;

      std::list<std::string> m_loadErrors;

      MetaDataList          m_metaData;
      vector<Vertex *>      m_vertices;
      vector<Triangle *>    m_triangles;
      vector<Group *>       m_groups;
      vector<Material *>    m_materials;
      vector<Joint *>       m_joints;
      vector<Point *>       m_points;
      vector<TextureProjection *>    m_projections;
      vector<FrameAnim *>   m_frameAnims;
      vector<SkelAnim *>    m_skelAnims;

      DrawingContextList m_drawingContexts;

      BspTree     m_bspTree;
      bool        m_validBspTree;

      vector<FormatData *>  m_formatData;

      int  m_canvasDrawMode;
      DrawJointModeE m_drawJoints;
      bool m_drawProjections;

      bool m_validNormals;
      bool m_validJoints;

      bool m_forceAddOrDelete;

      int    m_numFrames;  // Deprecated

      AnimationModeE m_animationMode;
      bool     m_animationLoop;
      unsigned m_currentFrame;
      unsigned m_currentAnim;
      double   m_currentTime;

#ifdef MM3D_EDIT
      vector<int> m_selectedUv;

      ObserverList m_observers;

      BackgroundImage * m_background[6];

      bool           m_saved;
      SelectionModeE m_selectionMode;
      bool           m_selecting;

      // What has changed since the last time the observers were notified?
      // See ChangeBits
      int            m_changeBits;

      UndoManager * m_undoMgr;
      UndoManager * m_animUndoMgr;
      bool m_undoEnabled;
#endif // MM3D_EDIT

      // Base position for skeletal animations (safe to ignore in MM3D).
      Matrix   m_localMatrix;

      // ModelFilter is defined as a friend so that classes derived from ModelFilter
      // can get direct access to the model primitive lists (yes, it's a messy hack).
      friend class ModelFilter;
};

extern void model_show_alloc_stats();
extern int model_free_primitives();

#endif //__MODEL_H
