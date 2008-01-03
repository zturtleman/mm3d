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

#include <list>
#include <vector>
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

      enum CompareBits
      {
         CompareGeometry  =  0x01,  // Vertices and Faces match
         CompareFaces     =  0x02,  // Faces match, vertices may not
         CompareGroups    =  0x04,  // Groups match
         CompareSkeleton  =  0x08,  // Bone joints hierarchy matches
         CompareTextures  =  0x10,  // Textures and texture coordinates match
         CompareAnimSets  =  0x20,  // Number of animations, frame counts, and fps match
         CompareAnimData  =  0x40,  // Animation movements match
         CompareMeta      =  0x80,  // Names and other non-visible data match
         ComparePoints    = 0x100,  // Points match
         CompareAll       = 0x1FF   // All of the above
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
            { return m_weight < rhs.m_weight; }
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

            bool equal( const Vertex & rhs, int compareBits = CompareAll ) const;
            bool operator==( const Vertex & rhs ) const
               { return equal( rhs ); }

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

            bool equal( const Triangle & rhs, int compareBits = CompareAll ) const;
            bool operator==( const Triangle & rhs ) const
               { return equal( rhs ); }

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

            std::string m_name;
            int         m_materialIndex;    // Material index (-1 for none)
            vector<int> m_triangleIndices;  // List of triangles in this group

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

            bool equal( const Group & rhs, int compareBits = CompareAll ) const;
            bool operator==(const Group & rhs ) const
               { return equal( rhs ); }

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

            bool equal( const Material & rhs, int compareBits = CompareAll ) const;
            bool operator==(const Material & rhs ) const
               { return equal( rhs ); }

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
            bool equal( const Keyframe & rhs, int compareBits = CompareAll ) const;

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

            bool equal( const Joint & rhs, int compareBits = CompareAll ) const;
            bool operator==(const Joint & rhs ) const
               { return equal( rhs ); }

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

            bool equal( const Point & rhs, int compareBits = CompareAll ) const;
            bool operator==(const Point & rhs ) const
               { return equal( rhs ); }

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

            std::string m_name;
            int m_type;            // See TextureProjectionTypeE
            double m_pos[3];
            double m_upVec[3];     // Vector that defines "up" for this projection
            double m_seamVec[3];   // Vector that indicates where the texture wraps from 1.0 back to 0.0
            double m_range[2][2];  // min/max, x/y

            bool   m_selected;
            bool   m_marked;

            bool equal( const TextureProjection & rhs, int compareBits = CompareAll ) const;
            bool operator==(const TextureProjection & rhs ) const
               { return equal( rhs ); }

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

            std::string m_name;
            JointKeyframeList m_jointKeyframes;
            double   m_fps;  // Frames per second
            double   m_spf;  // Seconds per frame (for convenience, 1.0 / m_fps)
            unsigned m_frameCount;    // Number of frames in the animation
            bool     m_validNormals;  // Whether or not the normals have been calculated for the current animation frame

            bool equal( const SkelAnim & rhs, int compareBits = CompareAll ) const;
            bool operator==(const SkelAnim & rhs ) const
               { return equal( rhs ); }

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

            double m_coord[3];
            double m_normal[3];

            bool equal( const FrameAnimVertex & rhs, int compareBits = CompareAll ) const;
            bool operator==(const FrameAnimVertex & rhs ) const
               { return equal( rhs ); }

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

            double m_trans[3];
            double m_rot[3];

            bool equal( const FrameAnimPoint & rhs, int compareBits = CompareAll ) const;
            bool operator==(const FrameAnimPoint & rhs ) const
               { return equal( rhs ); }

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

              bool equal( const FrameAnimData & rhs, int compareBits = CompareAll ) const;
              bool operator==(const FrameAnimData & rhs ) const
               { return equal( rhs ); }
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

            std::string m_name;
            // Each element in m_frameData is one frame. The frames hold lists of
            // all vertex positions and point positions.
            FrameAnimDataList m_frameData;

            double m_fps;  // Frames per second
            bool   m_validNormals;  // Whether or not the normals have been calculated

            bool equal( const FrameAnim & rhs, int compareBits = CompareAll ) const;
            bool operator==(const FrameAnim & rhs ) const
               { return equal( rhs ); }

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

            std::string m_filename;
            float m_scale;      // 1.0 means 1 GL unit from the center to the edges of the image
            float m_center[3];  // Point in the viewport where the image is centered

            bool equal( const BackgroundImage & rhs, int compareBits = CompareAll ) const;
            bool operator==(const BackgroundImage & rhs ) const
               { return equal( rhs ); }
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

      // Returns mask of successful compares (see enum CompareBits)
      int equivalent( const Model * model, int compareMask = CompareGeometry, double tolerance = 0.00001 );

      // ------------------------------------------------------------------
      // "Meta" data, model information that is not rendered in a viewport.
      // ------------------------------------------------------------------

      // Indicates if the model has changed since the last time it was saved.
      void setSaved( bool o ) { if ( o ) { m_undoMgr->setSaved(); }; };
      bool getSaved()         { return m_undoMgr->isSaved(); };

      const char * getFilename() { return m_filename.c_str(); };
      void setFilename( const char * filename ) { 
            if ( filename && filename[0] ) { m_filename = filename; } };

      const char * getExportFile() { return m_exportFile.c_str(); };
      void setExportFile( const char * filename ) { 
            if ( filename && filename[0] ) { m_exportFile = filename; } };

      void setFilterSpecificError( const char * str ) { s_lastFilterError = m_filterSpecificError = str; };
      const char * getFilterSpecificError() { return m_filterSpecificError.c_str(); };
      static const char * getLastFilterSpecificError() { return s_lastFilterError.c_str(); };

      // Observers are notified when the model changes. See the Observer class
      // for details.
      void addObserver( Observer * o );
      void removeObserver( Observer * o );

      // See the MetaData class.
      void addMetaData( const char * key, const char * value );
      void updateMetaData( const char * key, const char * value );

      // Look-up by key
      bool getMetaData( const char * key, char * value, size_t valueLen );
      // Look-up by index
      bool getMetaData( unsigned int index, char * key, size_t keyLen, char * value, size_t valueLen );

      unsigned int getMetaDataCount();
      void clearMetaData();
      void removeLastMetaData();  // For undo only!

      // Background image accessors. See the BackgroundImage class.
      void setBackgroundImage( unsigned index, const char * str );
      void setBackgroundScale( unsigned index, float scale );
      void setBackgroundCenter( unsigned index, float x, float y, float z );

      const char * getBackgroundImage( unsigned index );
      float getBackgroundScale( unsigned index );
      void getBackgroundCenter( unsigned index, float & x, float & y, float & z );

      // These are used to store status messages when the model does not have a status
      // bar. When a model is assigned to a viewport window, the messages will be
      // displayed in the status bar.
      bool hasErrors() { return !m_loadErrors.empty(); }
      void pushError( const std::string & err );
      std::string popError();

      // Use these functions to preserve data that Misfit doesn't support natively
      int  addFormatData( FormatData * fd );
      bool deleteFormatData( unsigned index );
      unsigned getFormatDataCount();
      FormatData * getFormatData( unsigned index );
      FormatData * getFormatDataByFormat( const char * format, unsigned index = 0 ); // not case sensitive

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
      int  getCanvasDrawMode() { return m_canvasDrawMode; };

      void setDrawJoints( DrawJointModeE m ) { m_drawJoints = m; };
      DrawJointModeE getDrawJoints() { return m_drawJoints; };

      void setDrawProjections( bool o ) { m_drawProjections = o; };
      bool getDrawProjections() { return m_drawProjections; };

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

      bool setCurrentAnimation( const AnimationModeE & m, const char * name );
      bool setCurrentAnimation( const AnimationModeE & m, unsigned index );
      bool setCurrentAnimationFrame( unsigned frame );
      bool setCurrentAnimationTime( double time );

      unsigned getCurrentAnimation();
      unsigned getCurrentAnimationFrame();
      double   getCurrentAnimationTime();

      void setAnimationLooping( bool o );
      bool isAnimationLooping();

      // Stop animation mode, go back to standard pose editing.
      void setNoAnimation();

      AnimationModeE getAnimationMode() { return m_animationMode; };
      bool inSkeletalMode()  { return (m_animationMode == ANIMMODE_SKELETAL); };

      // Common animation properties
      int addAnimation( const AnimationModeE & mode, const char * name );
      void deleteAnimation( const AnimationModeE & mode, const unsigned & index );

      unsigned getAnimCount( const AnimationModeE & m ) const;

      const char * getAnimName( const AnimationModeE & mode, const unsigned & anim );
      bool setAnimName( const AnimationModeE & mode, const unsigned & anim, const char * name );

      double getAnimFPS( const AnimationModeE & mode, const unsigned & anim );
      bool setAnimFPS( const AnimationModeE & mode, const unsigned & anim, const double & fps );

      unsigned getAnimFrameCount( const AnimationModeE & mode, const unsigned & anim );
      bool setAnimFrameCount( const AnimationModeE & mode, const unsigned & anim, const unsigned & count );

      bool clearAnimFrame( const AnimationModeE & mode, const unsigned & anim, const unsigned & frame );

      // Frame animation geometry
      void setFrameAnimVertexCount( const unsigned & vertexCount );
      void setFrameAnimPointCount( const unsigned & pointCount );

      bool setFrameAnimVertexCoords( const unsigned & anim, const unsigned & frame, const unsigned & vertex, 
            const double & x, const double & y, const double & z );
      bool getFrameAnimVertexCoords( const unsigned & anim, const unsigned & frame, const unsigned & vertex, 
            double & x, double & y, double & z );
      bool getFrameAnimVertexNormal( const unsigned & anim, const unsigned & frame, const unsigned & vertex, 
            double & x, double & y, double & z );

      // Not undo-able
      bool setQuickFrameAnimVertexCoords( const unsigned & anim, const unsigned & frame, const unsigned & vertex, 
            const double & x, const double & y, const double & z );

      bool setFrameAnimPointCoords( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            const double & x, const double & y, const double & z );
      bool getFrameAnimPointCoords( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            double & x, double & y, double & z );

      bool setFrameAnimPointRotation( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            const double & x, const double & y, const double & z );
      bool getFrameAnimPointRotation( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            double & x, double & y, double & z );

      // Skeletal animation keyframs
      int  setSkelAnimKeyframe( const unsigned & anim, const unsigned & frame, const unsigned & joint, const bool & isRotation, 
            const double & x, const double & y, const double & z );
      bool getSkelAnimKeyframe( unsigned anim, unsigned frame,
            unsigned joint, bool isRotation,
            double & x, double & y, double & z );

      bool hasSkelAnimKeyframe( unsigned anim, unsigned frame,
            unsigned joint, bool isRotation );

      bool deleteSkelAnimKeyframe( const unsigned & anim, const unsigned & frame, const unsigned & joint, const bool & isRotation );

      // Interpolate what a keyframe for this joint would be at the specified frame.
      bool interpSkelAnimKeyframe( unsigned anim, unsigned frame,
            bool loop, unsigned joint, bool isRotation,
            double & x, double & y, double & z );
      // Interpolate what a keyframe for this joint would be at the specified time.
      bool interpSkelAnimKeyframeTime( unsigned anim, double frameTime,
            bool loop, unsigned joint,
            Matrix & relativeFinal );

      // Animation set operations
      int  copyAnimation( const AnimationModeE & mode, const unsigned & anim, const char * newName );
      int  splitAnimation( const AnimationModeE & mode, const unsigned & anim, const char * newName, const unsigned & frame );
      bool joinAnimations( const AnimationModeE & mode, const unsigned & anim1, const unsigned & anim2 );
      bool mergeAnimations( const AnimationModeE & mode, const unsigned & anim1, const unsigned & anim2 );
      int  convertAnimToFrame( const AnimationModeE & mode, const unsigned & anim1, const char * newName, const unsigned & frameCount );

      bool moveAnimation( const AnimationModeE & mode, const unsigned & oldIndex, const unsigned & newIndex );

      // For undo, don't call these directly
      bool insertSkelAnimKeyframe( const unsigned & anim, Keyframe * keyframe );
      bool removeSkelAnimKeyframe( const unsigned & anim, const unsigned & frame, const unsigned & joint, const bool & isRotation, bool release = false );

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
      void forceAddOrDelete( bool o ) { m_forceAddOrDelete = o; };

      bool canAddOrDelete() { return (m_frameAnims.size() == 0 || m_forceAddOrDelete); };

      // Show an error because the user tried to add or remove primitives while
      // the model has frame animations.
      void displayFrameAnimPrimitiveError();

      int getNumFrames();  // Deprecated

      // ------------------------------------------------------------------
      // Normal functions
      // ------------------------------------------------------------------

      bool getNormal( unsigned triangleNum, unsigned vertexIndex, float *normal );
      bool getFlatNormal( unsigned triangleNum, float *normal );
      float cosToPoint( unsigned triangleNum, double * point );

      void calculateNormals();
      void calculateSkelNormals();
      void calculateFrameNormals( const unsigned & anim );
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

      bool getPositionCoords( const Position & pos, double * coord );
      bool getPositionCoords( PositionTypeE ptype, unsigned pindex, double * coord );

      int addVertex( double x, double y, double z );
      int addTriangle( unsigned vert1, unsigned vert2, unsigned vert3 );

      void deleteVertex( unsigned vertex );
      void deleteTriangle( unsigned triangle );

      // No undo on this one
      void setVertexFree( unsigned v, bool o );
      bool isVertexFree( unsigned v );

      // When all faces attached to a vertex are deleted, the vertex is considered
      // an "orphan" and deleted (unless it is a "free" vertex, see m_free in the
      // vertex class).
      void deleteOrphanedVertices();

      // A flattened triangle is a triangle with two or more corners that are
      // assigned to the same vertex (this usually happens when vertices are
      // welded together).
      void deleteFlattenedTriangles();

      bool isTriangleMarked( unsigned t );

      void subdivideSelectedTriangles();
      void unsubdivideTriangles( unsigned t1, unsigned t2, unsigned t3, unsigned t4 );

      // If co-planer triangles have edges with points that are co-linear they
      // can be combined into a single triangle. The simplifySelectedMesh function
      // performs this operation to combine all faces that do not add detail to
      // the model.
      void simplifySelectedMesh();

      bool setTriangleVertices( unsigned triangleNum, unsigned vert1, unsigned vert2, unsigned vert3 );
      bool getTriangleVertices( unsigned triangleNum, unsigned & vert1, unsigned & vert2, unsigned & vert3 );
      void setTriangleMarked( unsigned triangleNum, bool marked );
      void clearMarkedTriangles();

      bool getVertexCoordsUnanimated( const unsigned & vertexNumber, double *coord );
      bool getVertexCoords( const unsigned & vertexNumber, double *coord );
      bool getVertexCoords2d( const unsigned & vertexNumber, const ProjectionDirectionE & dir, double *coord );

      int getTriangleVertex( unsigned triangleNumber, unsigned vertexIndex );

      void booleanOperation( BooleanOpE op, 
            std::list<int> & listA, std::list<int> & listB );

      Model * copySelected();

      // A BSP tree is calculated for triangles that have textures with an alpha
      // channel (transparency). It is used to determine in what order triangles
      // must be rendered to get the proper blending results (triangles that are
      // farther away from the camera are rendered first so that closer triangles
      // are drawn on top of them).
      void calculateBspTree();
      void invalidateBspTree();

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
      inline int getTextureCount() { return m_materials.size(); };

      int addGroup( const char * name );

      // Textures and Color materials go into the same material list
      int addTexture( Texture * tex );
      int addColorMaterial( const char * name );

      bool setGroupTextureId( unsigned groupNumber, int textureId );

      void deleteGroup( unsigned group );
      void deleteTexture( unsigned texture );

      const char * getGroupName( unsigned groupNum );
      bool setGroupName( unsigned groupNum, const char * groupName );

      inline int getGroupCount() const { return m_groups.size(); };
      int getGroupByName( const char * groupName, bool ignoreCase = false );
      int getMaterialByName( const char * materialName, bool ignoreCase = false );
      Material::MaterialTypeE getMaterialType( unsigned materialIndex );
      int getMaterialColor( unsigned materialIndex, unsigned c, unsigned v = 0 );

      // These implicitly change the material type.
      void setMaterialTexture( unsigned textureId, Texture * tex );
      void removeMaterialTexture( unsigned textureId );

      uint8_t getGroupSmooth( const unsigned & groupNum );
      bool setGroupSmooth( const unsigned & groupNum, const uint8_t & smooth );
      uint8_t getGroupAngle( const unsigned & groupNum );
      bool setGroupAngle( const unsigned & groupNum, const uint8_t & angle );

      void setTextureName( unsigned textureId, const char * name );
      const char * getTextureName( unsigned textureId );

      const char * getTextureFilename( unsigned textureId );
      Texture * getTextureData( unsigned textureId );

      // Lighting accessors
      bool getTextureAmbient(   unsigned textureId,       float * ambient   );
      bool getTextureDiffuse(   unsigned textureId,       float * diffuse   );
      bool getTextureEmissive(  unsigned textureId,       float * emissive  );
      bool getTextureSpecular(  unsigned textureId,       float * specular  );
      bool getTextureShininess( unsigned textureId,       float & shininess );

      bool setTextureAmbient(   unsigned textureId, const float * ambient   );
      bool setTextureDiffuse(   unsigned textureId, const float * diffuse   );
      bool setTextureEmissive(  unsigned textureId, const float * emissive  );
      bool setTextureSpecular(  unsigned textureId, const float * specular  );
      bool setTextureShininess( unsigned textureId, const float & shininess );

      // See the clamp property in the Material class.
      bool getTextureSClamp( unsigned textureId );
      bool getTextureTClamp( unsigned textureId );
      bool setTextureSClamp( unsigned textureId, bool clamp );
      bool setTextureTClamp( unsigned textureId, bool clamp );

      list<int> getUngroupedTriangles();
      list<int> getGroupTriangles( unsigned groupNumber ) const;
      int       getGroupTextureId( unsigned groupNumber );

      int getTriangleGroup( unsigned triangleNumber );

      void addTriangleToGroup( unsigned groupNum, unsigned triangleNum );
      void removeTriangleFromGroup( unsigned groupNum, unsigned triangleNum );

      void setSelectedAsGroup( unsigned groupNum );
      void addSelectedToGroup( unsigned groupNum );

      bool getTextureCoords( const unsigned & triangleNumber, const unsigned & vertexIndex, float & s, float & t );
      bool setTextureCoords( const unsigned & triangleNumber, const unsigned & vertexIndex, const float & s, const float & t );

      // ------------------------------------------------------------------
      // Skeletal structure and influence functions
      // ------------------------------------------------------------------

      int addBoneJoint( const char * name, const double & x, const double & y, const double & z, 
            const double & xrot, const double & yrot, const double & zrot,
            const int & parent = -1 );

      void deleteBoneJoint( unsigned joint );

      const char * getBoneJointName( const unsigned & joint );
      int getBoneJointParent( const unsigned & joint );
      bool getBoneJointCoords( const unsigned & jointNumber, double * coord );

      bool getBoneJointFinalMatrix( const unsigned & jointNumber, Matrix & m );
      bool getBoneJointAbsoluteMatrix( const unsigned & jointNumber, Matrix & m );
      bool getBoneJointRelativeMatrix( const unsigned & jointNumber, Matrix & m );
      bool getPointFinalMatrix( const unsigned & jointNumber, Matrix & m );

      list<int> getBoneJointVertices( const int & joint );
      int getVertexBoneJoint( const unsigned & vertexNumber );
      int getPointBoneJoint( const unsigned & point );

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

      bool getPositionInfluences( const Position & pos, InfluenceList & l );
      bool getVertexInfluences( unsigned vertex, InfluenceList & l );
      bool getPointInfluences( unsigned point, InfluenceList & l );

      int getPrimaryPositionInfluence( const Position & pos );
      int getPrimaryVertexInfluence( unsigned vertex );
      int getPrimaryPointInfluence( unsigned point );

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

      bool setBoneJointName( const unsigned & joint, const char * name );
      bool setBoneJointParent( const unsigned & joint, const int & parent = -1 );
      bool setBoneJointRotation( const unsigned & j, const double * rot );
      bool setBoneJointTranslation( const unsigned & j, const double * trans );

      double calculatePositionInfluenceWeight( const Position & pos, unsigned joint );
      double calculateVertexInfluenceWeight( unsigned vertex, unsigned joint );
      double calculatePointInfluenceWeight( unsigned point, unsigned joint );
      double calculateCoordInfluenceWeight( double * coord, unsigned joint );

      void calculateRemainderWeight( InfluenceList & list );

      bool getBoneVector( unsigned joint, double * vec, double * coord );

      // No undo on this one
      bool relocateBoneJoint( unsigned j, double x, double y, double z );

      // Call this when a bone joint is moved, rotated, added, or deleted to
      // recalculate the skeletal structure.
      void setupJoints();

      // ------------------------------------------------------------------
      // Point functions
      // ------------------------------------------------------------------

      int addPoint( const char * name, const double & x, const double & y, const double & z, 
            const double & xrot, const double & yrot, const double & zrot,
            const int & boneId = -1 );

      void deletePoint( unsigned point );

      int getPointByName( const char * name );

      const char * getPointName( const unsigned & point );
      bool setPointName( const unsigned & point, const char * name );

      int getPointType( const unsigned & point );
      bool setPointType( const unsigned & point, int type );

      // FIXME is there a difference between these?
      // Looks like coords/orientation should be removed (to keep rotation/translation
      // naming consistent).
      bool getPointCoords( const unsigned & pointNumber, double * coord );
      bool getPointOrientation( const unsigned & pointNumber, double * rot );
      bool getPointRotation( const unsigned & point, double * rot );
      bool getPointTranslation( const unsigned & point, double * trans );

      bool setPointRotation( unsigned point, const double * rot );
      bool setPointTranslation( unsigned point, const double * trans );

      // ------------------------------------------------------------------
      // Texture projection functions
      // ------------------------------------------------------------------

      int addProjection( const char * name, int type, double x, double y, double z );
      void deleteProjection( unsigned proj );

      const char * getProjectionName( const unsigned & proj );

      void   setProjectionScale( unsigned p, double scale );
      double getProjectionScale( unsigned p );

      bool setProjectionName( const unsigned & proj, const char * name );
      bool setProjectionType( const unsigned & proj, int type );
      int  getProjectionType( const unsigned & proj );
      bool setProjectionRotation( const unsigned & proj, int type );
      int  getProjectionRotation( const unsigned & proj );
      bool getProjectionCoords( unsigned projNumber, double *coord );

      bool setProjectionUp( unsigned projNumber, const double *coord );
      bool setProjectionSeam( unsigned projNumber, const double *coord );
      bool setProjectionRange( unsigned projNumber, 
            double xmin, double ymin, double xmax, double ymax );

      bool getProjectionUp( unsigned projNumber, double *coord );
      bool getProjectionSeam( unsigned projNumber, double *coord );
      bool getProjectionRange( unsigned projNumber, 
            double & xmin, double & ymin, double & xmax, double & ymax );

      void setTriangleProjection( unsigned triangleNum, int proj );
      int  getTriangleProjection( unsigned triangleNum );

      void applyProjection( unsigned int proj );

      // ------------------------------------------------------------------
      // Undo/Redo functions
      // ------------------------------------------------------------------

      bool setUndoEnabled( bool o ) { bool old = m_undoEnabled; m_undoEnabled = o; return old; };

      // Indicates that a user-specified operation is complete. A single
      // "operation" may span many function calls and different types of
      // manipulations.
      void operationComplete( const char * opname = NULL );

      bool canUndo();
      bool canRedo();
      void undo();
      void redo();

      // If a manipulations have been performed, but not commited as a single
      // undo list, undo those operations (often used when the user clicks
      // a "Cancel" button to discard "unapplied" changes).
      void undoCurrent();

      const char * getUndoOpName();
      const char * getRedoOpName();

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

      void insertFrameAnim( const unsigned & index, FrameAnim * anim );
      void removeFrameAnim( const unsigned & index );

      void insertSkelAnim( const unsigned & anim, SkelAnim * fa );
      void removeSkelAnim( const unsigned & anim );

      void insertFrameAnimFrame( const unsigned & anim, const unsigned & frame,
            FrameAnimData * data );
      void removeFrameAnimFrame( const unsigned & anim, const unsigned & frame );

      // ------------------------------------------------------------------
      // Selection functions
      // ------------------------------------------------------------------

      void setSelectionMode( SelectionModeE m );
      inline SelectionModeE getSelectionMode() { return m_selectionMode; };

      unsigned getSelectedVertexCount();
      unsigned getSelectedTriangleCount();
      unsigned getSelectedBoneJointCount();
      unsigned getSelectedPointCount();
      unsigned getSelectedProjectionCount();

      void getSelectedPositions( list<Position> & l );
      void getSelectedVertices( list<int> & l );
      void getSelectedTriangles( list<int> & l );
      void getSelectedGroups( list<int> & l );
      void getSelectedBoneJoints( list<int> & l );
      void getSelectedPoints( list<int> & l );
      void getSelectedProjections( list<int> & l );

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

      bool getBoundingRegion( double *x1, double *y1, double *z1, double *x2, double *y2, double *z2 );
      bool getSelectedBoundingRegion( double *x1, double *y1, double *z1, double *x2, double *y2, double *z2 );

      void deleteSelected();

      // When changing the selection state of a lot of primitives, a difference
      // list is used to track undo information. These calls indicate when the
      // undo information should start being tracked and when it should be
      // completed.
      void beginSelectionDifference();
      void endSelectionDifference();

      bool selectVertex( unsigned v );
      bool unselectVertex( unsigned v );
      bool isVertexSelected( unsigned v );

      bool selectTriangle( unsigned t );
      bool unselectTriangle( unsigned t );
      bool isTriangleSelected( unsigned t );

      bool selectGroup( unsigned g );
      bool unselectGroup( unsigned g );
      bool isGroupSelected( unsigned g );

      bool selectBoneJoint( unsigned j );
      bool unselectBoneJoint( unsigned j );
      bool isBoneJointSelected( unsigned j );

      bool selectPoint( unsigned p );
      bool unselectPoint( unsigned p );
      bool isPointSelected( unsigned p );

      bool selectProjection( unsigned p );
      bool unselectProjection( unsigned p );
      bool isProjectionSelected( unsigned p );

      // The behavior of this function changes based on the selection mode.
      bool invertSelection();

      // Select all vertices that have the m_free property set and are not used
      // by any triangles (this can be used to clean up unused free vertices,
      // like a manual analog to the deleteOrphanedVertices() function).
      void selectFreeVertices();

      // ------------------------------------------------------------------
      // Hide functions
      // ------------------------------------------------------------------

      bool hideSelected();
      bool hideUnselected();
      bool unhideAll();

      bool isVertexVisible( unsigned v );
      bool isTriangleVisible( unsigned t );
      bool isGroupVisible( unsigned g );
      bool isBoneJointVisible( unsigned j );
      bool isPointVisible( unsigned p );

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

      bool parentJointSelected( int joint );
      bool directParentJointSelected( int joint );

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
