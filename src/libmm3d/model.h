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

#include <stdint.h>

#include <list>
#include <vector>
#include <string>

using std::list;
using std::vector;

class Texture;

class Model
{
   public:
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
        TPT_Custom      = -1,
        TPT_Cylinder    = 0,
        TPT_Sphere      = 1,
        TPT_Plane       = 2,
        TPT_Icosahedron = 3,
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
      };
      typedef struct _Influence_t InfluenceT;
      typedef std::list< InfluenceT > InfluenceList;

      class Position
      {
         public:
            PositionTypeE type;
            unsigned      index;
      };

      class Vertex
      {
         public:
            static int flush();
            static void stats();
            static Vertex * get();
            void release();

            double m_coord[3];
            double m_kfCoord[3];
            bool   m_selected;
            bool   m_visible;
            bool   m_marked;
            bool   m_marked2;
            bool   m_free;

            double * m_drawSource;

            InfluenceList m_influences;

         protected:
            Vertex();
            virtual ~Vertex();
            void init();

            static list<Vertex *> s_recycle;
            static int s_allocated;
      };

      class Triangle
      {
         public:
            static int flush();
            static void stats();
            static Triangle * get();
            void release();

            unsigned m_vertexIndices[3];
            float m_s[3];
            float m_t[3];
            double m_finalNormals[3][3];
            double m_vertexNormals[3][3];
            double m_flatNormals[3];
            double m_kfFlatNormals[3];
            double m_kfNormals[3][3];
            double * m_flatSource;
            double * m_normalSource[3];
            bool  m_selected;
            bool  m_visible;
            bool  m_marked;
            bool  m_marked2;
            bool  m_userMarked;
            int   m_projection;

         protected:
            Triangle();
            virtual ~Triangle();
            void init();

            static list<Triangle *> s_recycle;
            static int s_allocated;
      };

      class Group
      {
         public:
            static int flush();
            static void stats();
            static Group * get();
            void release();

            std::string m_name;
            int         m_materialIndex;
            vector<int> m_triangleIndices;
            uint8_t     m_smooth;
            uint8_t     m_angle;
            bool        m_selected;
            bool        m_visible;
            bool        m_marked;
         protected:
            Group();
            virtual ~Group();
            void init();

            static list<Group *> s_recycle;
            static int s_allocated;
      };

      class Material
      {
         public:
            enum _MaterialType_e
            {
               MATTYPE_TEXTURE  =  0,
               MATTYPE_BLANK    =  1,
               MATTYPE_COLOR    =  2,
               MATTYPE_GRADIENT =  3,
               MATTYPE_MAX      =  4
            };
            typedef enum _MaterialType_e MaterialTypeE;

            static int flush();
            static void stats();
            static Material * get();
            void release();

            std::string   m_name;
            MaterialTypeE  m_type;
            float         m_ambient[4];
            float         m_diffuse[4];
            float         m_specular[4];
            float         m_emissive[4];
            float         m_shininess;
            uint8_t       m_color[4][4];
            bool          m_sClamp;
            bool          m_tClamp;
            GLuint        m_texture;
            std::string   m_filename;
            std::string   m_alphaFilename;
            Texture     * m_textureData;
         protected:
            Material();
            virtual ~Material();
            void init();

            static list<Material *> s_recycle;
            static int s_allocated;
      };

      class Keyframe
      {
         public:
            static int flush();
            static void stats();
            static Keyframe * get();

            void release();

            int m_jointIndex;
            unsigned m_frame;
            double m_time;
            double m_parameter[3];
            bool   m_isRotation;

            bool operator<( const Keyframe & rhs )
            {
               return ( this->m_frame < rhs.m_frame || (this->m_frame == rhs.m_frame && !this->m_isRotation && rhs.m_isRotation) );
            };
            bool operator==( const Keyframe & rhs )
            {
               return ( this->m_frame == rhs.m_frame && this->m_isRotation == rhs.m_isRotation );
            };

         protected:
            Keyframe();
            virtual ~Keyframe();
            void init();

            static list<Keyframe *> s_recycle;
            static int s_allocated;
      };


      class Joint
      {
         public:
            static int flush();
            static void stats();
            static Joint * get();
            void release();

            std::string m_name;
            double m_localRotation[3];
            double m_localTranslation[3];
            Matrix m_absolute;
            Matrix m_relative;

            int m_currentRotationKeyframe;
            int m_currentTranslationKeyframe;
            Matrix m_final;

            int m_parent;

            bool m_selected;
            bool m_visible;
            bool m_marked;

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
            static void stats();
            static Point * get();
            void release();

            std::string m_name;
            int m_type;
            double m_trans[3];
            double m_rot[3];
            double m_kfTrans[3];
            double m_kfRot[3];
            double * m_drawSource;
            double * m_rotSource;

            bool   m_selected;
            bool   m_visible;
            bool   m_marked;

            InfluenceList m_influences;

         protected:
            Point();
            virtual ~Point();
            void init();

            static list<Point *> s_recycle;
            static int s_allocated;
      };

      class TextureProjection
      {
         public:
            static int flush();
            static void stats();
            static TextureProjection * get();
            void release();

            std::string m_name;
            int m_type;
            double m_pos[3];
            double m_upVec[3];
            double m_seamVec[3];
            double m_range[2][2];  // min/max, x/y

            bool   m_selected;
            bool   m_marked;

         protected:
            TextureProjection();
            virtual ~TextureProjection();
            void init();

            static int s_allocated;
      };

      typedef sorted_ptr_list<Keyframe *>        KeyframeList;
      typedef vector<KeyframeList>      JointKeyframeList;

      class SkelAnim
      {
         public:
            static int flush();
            static void stats();
            static SkelAnim * get();
            void release();

            std::string m_name;
            JointKeyframeList m_jointKeyframes;
            double   m_fps;
            double   m_spf;
            unsigned m_frameCount;
            bool     m_validNormals;

         protected:
            SkelAnim();
            virtual ~SkelAnim();
            void init();

            static list<SkelAnim *> s_recycle;
            static int s_allocated;
      };

      class FrameAnimVertex
      {
         public:
            static int flush();
            static void stats();
            static FrameAnimVertex * get();
            void release();

            double m_coord[3];
            double m_normal[3];

         protected:
            FrameAnimVertex();
            virtual ~FrameAnimVertex();
            void init();

            static list<FrameAnimVertex *> s_recycle;
            static int s_allocated;
      };

      typedef vector<FrameAnimVertex *> FrameAnimVertexList;

      class FrameAnimPoint
      {
         public:
            static int flush();
            static void stats();
            static FrameAnimPoint * get();
            void release();

            double m_trans[3];
            double m_rot[3];

         protected:
            FrameAnimPoint();
            virtual ~FrameAnimPoint();
            void init();

            static list<FrameAnimPoint *> s_recycle;
            static int s_allocated;
      };

      typedef vector<FrameAnimPoint *> FrameAnimPointList;

      // TODO recycle?
      class FrameAnimData
      {
          public:
              FrameAnimVertexList * m_frameVertices;
              FrameAnimPointList  * m_framePoints;
      };

      typedef vector< FrameAnimData *> FrameAnimDataList;

      class FrameAnim
      {
         public:
            static int flush();
            static void stats();
            static FrameAnim * get();
            void release();

            std::string m_name;
            FrameAnimDataList m_frameData;
            double m_fps;
            bool   m_validNormals;

         protected:
            FrameAnim();
            virtual ~FrameAnim();
            void init();

            static list<FrameAnim *> s_recycle;
            static int s_allocated;
      };

      class KeyframeVertex
      {
         public:
            double m_coord[3];
            double m_normal[3];
            double m_rotatedNormal[3];
      };

      class KeyframeTriangle
      {
         public:
            double m_normals[3][3];
      };

      class BackgroundImage
      {
         public:
            BackgroundImage();
            virtual ~BackgroundImage();

            std::string m_filename;
            float m_scale;
            float m_center[3];
      };

      typedef Model::Vertex * VertexPtr;
      typedef Model::Triangle * TrianglePtr;
      typedef Model::Group * GroupPtr;
      typedef Model::Material * MaterialPtr;
      //typedef Model::Joint * JointPtr;

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

      class MetaData
      {
         public:
            std::string key;
            std::string value;
      };
      typedef std::vector< MetaData > MetaDataList;

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

      enum
      {
         DO_NONE           = 0x00,
         DO_TEXTURE        = 0x01,  // powers of 2
         DO_SMOOTHING      = 0x02,
         DO_WIREFRAME      = 0x04,
         DO_BADTEX         = 0x08,
         DO_ALPHA          = 0x10,
         DO_BACKFACECULL   = 0x20,
      };

      enum _AnimationMode_e
      {
         ANIMMODE_NONE = 0,
         ANIMMODE_SKELETAL,
         ANIMMODE_FRAME,
         ANIMMODE_FRAMERELATIVE,
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
      class Observer
      {
         public:
            virtual ~Observer();
            virtual void modelChanged( int changeBits ) = 0;
      };
      typedef std::list< Observer * > ObserverList;
#endif // MM3D_EDIT

      // Public methods
      Model();
      virtual ~Model();

      static const char * errorToString( Model::ModelErrorE, Model * model = NULL );
      static bool operationFailed( Model::ModelErrorE );

      // Returns mask of successful compares (see enum CompareBits)
      int equivalent( const Model * model, int compareMask = CompareGeometry, double tolerance = 0.00001 );

      const char * getFilename() { return m_filename.c_str(); };
      void setFilename( const char * filename ) { 
            if ( filename && filename[0] ) { m_filename = filename; } };

      const char * getExportFile() { return m_exportFile.c_str(); };
      void setExportFile( const char * filename ) { 
            if ( filename && filename[0] ) { m_exportFile = filename; } };

      void setFilterSpecificError( const char * str ) { s_lastFilterError = m_filterSpecificError = str; };
      const char * getFilterSpecificError() { return m_filterSpecificError.c_str(); };
      static const char * getLastFilterSpecificError() { return s_lastFilterError.c_str(); };

      void draw( unsigned drawOptions = DO_TEXTURE, ContextT context = NULL, float *viewPoint = NULL );
      void drawLines();
      void drawVertices();
      void drawPoints();
      void drawProjections();
      bool loadTextures( ContextT context = NULL );
      void removeContext( ContextT context );

      // Use these functions to preserve data that Misfit doesn't support natively
      int  addFormatData( FormatData * fd );
      bool deleteFormatData( unsigned index );
      unsigned getFormatDataCount();
      FormatData * getFormatData( unsigned index );
      FormatData * getFormatDataByFormat( const char * format, unsigned index = 0 ); // not case sensitive

      bool setCurrentAnimation( const AnimationModeE & m, const char * name );
      bool setCurrentAnimation( const AnimationModeE & m, unsigned index );
      bool setCurrentAnimationFrame( unsigned frame );
      bool setCurrentAnimationTime( double time );

      unsigned getCurrentAnimation();
      unsigned getCurrentAnimationFrame();
      double   getCurrentAnimationTime();

      void setAnimationLooping( bool o );
      bool isAnimationLooping();

      void setNoAnimation();

      double   getAnimFPS( const AnimationModeE & mode, const unsigned & anim );

      const char * getAnimName( const AnimationModeE & mode, const unsigned & anim );

      unsigned getAnimFrameCount( const AnimationModeE & mode, const unsigned & anim );

      AnimationModeE getAnimationMode() { return m_animationMode; };
      bool inSkeletalMode()  { return (m_animationMode == ANIMMODE_SKELETAL); };

      bool getFrameAnimVertexCoords( const unsigned & anim, const unsigned & frame, const unsigned & vertex, 
            double & x, double & y, double & z );
      bool getFrameAnimVertexNormal( const unsigned & anim, const unsigned & frame, const unsigned & vertex, 
            double & x, double & y, double & z );

      bool hasSkelAnimKeyframe( unsigned anim, unsigned frame,
            unsigned joint, bool isRotation );
      bool getSkelAnimKeyframe( unsigned anim, unsigned frame,
            unsigned joint, bool isRotation,
            double & x, double & y, double & z );
      bool interpSkelAnimKeyframe( unsigned anim, unsigned frame,
            bool loop, unsigned joint, bool isRotation,
            double & x, double & y, double & z );
      bool interpSkelAnimKeyframeTime( unsigned anim, double frameTime,
            bool loop, unsigned joint,
            Matrix & relativeFinal );

      bool getNormal( unsigned triangleNum, unsigned vertexIndex, float *normal );
      bool getFlatNormal( unsigned triangleNum, float *normal );
      float cosToPoint( unsigned triangleNum, double * point );

      void calculateNormals();
      void calculateSkelNormals();
      void calculateFrameNormals( const unsigned & anim );
      void invalidateNormals();

      void calculateBspTree();
      void invalidateBspTree();

      const char * getGroupName( unsigned groupNum );
      inline int getGroupCount() const { return m_groups.size(); };
      int getGroupByName( const char * groupName, bool ignoreCase = false );
      int getMaterialByName( const char * materialName, bool ignoreCase = false );
      Material::MaterialTypeE getMaterialType( unsigned materialIndex );
      int getMaterialColor( unsigned materialIndex, unsigned c, unsigned v = 0 );

      uint8_t getGroupSmooth( const unsigned & groupNum );
      uint8_t getGroupAngle( const unsigned & groupNum );

      const char * getTextureName( unsigned textureId );
      const char * getTextureFilename( unsigned textureId );
      Texture * getTextureData( unsigned textureId );
      inline int getTextureCount() { return m_materials.size(); };

      bool getTextureAmbient(   unsigned textureId,       float * ambient   );
      bool getTextureDiffuse(   unsigned textureId,       float * diffuse   );
      bool getTextureEmissive(  unsigned textureId,       float * emissive  );
      bool getTextureSpecular(  unsigned textureId,       float * specular  );
      bool getTextureShininess( unsigned textureId,       float & shininess );

      bool getTextureSClamp( unsigned textureId );
      bool getTextureTClamp( unsigned textureId );

      list<int> getUngroupedTriangles();
      list<int> getGroupTriangles( unsigned groupNumber ) const;
      int       getGroupTextureId( unsigned groupNumber );

      list<int> getBoneJointVertices( const int & joint );
      const char * getBoneJointName( const unsigned & joint );

      const char * getPointName( const unsigned & point );
      int getPointByName( const char * name );
      int getPointType( const unsigned & point );
      int getPointBoneJoint( const unsigned & point );

      int getBoneJointParent( const unsigned & joint );

      const char * getProjectionName( const unsigned & proj );

      inline int getVertexCount()     const { return m_vertices.size(); }
      inline int getTriangleCount()   const { return m_triangles.size(); }
      inline int getBoneJointCount()  const { return m_joints.size(); }
      inline int getPointCount()      const { return m_points.size(); }
      inline int getProjectionCount() const { return m_projections.size(); }
      unsigned getAnimCount( const AnimationModeE & m ) const;

      bool getPositionCoords( const Position & pos, double * coord );
      bool getPositionCoords( PositionTypeE ptype, unsigned pindex, double * coord );

      bool getVertexCoordsUnanimated( const unsigned & vertexNumber, double *coord );
      bool getVertexCoords( const unsigned & vertexNumber, double *coord );
      bool getVertexCoords2d( const unsigned & vertexNumber, const ProjectionDirectionE & dir, double *coord );
      int getVertexBoneJoint( const unsigned & vertexNumber );
      bool getTextureCoords( const unsigned & triangleNumber, const unsigned & vertexIndex, float & s, float & t );
      bool setTextureCoords( const unsigned & triangleNumber, const unsigned & vertexIndex, const float & s, const float & t );
      bool getBoneJointCoords( const unsigned & jointNumber, double * coord );
      bool getPointCoords( const unsigned & pointNumber, double * coord );
      bool getPointOrientation( const unsigned & pointNumber, double * rot );
      bool getProjectionCoords( unsigned projNumber, double *coord );
      bool getProjectionUp( unsigned projNumber, double *coord );
      bool getProjectionSeam( unsigned projNumber, double *coord );
      bool getProjectionRange( unsigned projNumber, 
            double & xmin, double & ymin, double & xmax, double & ymax );

      void applyProjection( unsigned int proj );

      bool getBoneJointFinalMatrix( const unsigned & jointNumber, Matrix & m );
      bool getBoneJointAbsoluteMatrix( const unsigned & jointNumber, Matrix & m );
      bool getBoneJointRelativeMatrix( const unsigned & jointNumber, Matrix & m );
      bool getPointFinalMatrix( const unsigned & jointNumber, Matrix & m );

      /*
      bool getRelativeBoneJointPosition( const unsigned & jointNumber,
            const double & newX, const double & newY, const double & newZ,
            double & diffX, double & diffY, double & diffZ
            );
      */

      int getTriangleVertex( unsigned triangleNumber, unsigned vertexIndex );
      int getTriangleGroup( unsigned triangleNumber );

      void setupJoints();

      void forceAddOrDelete( bool o ) { m_forceAddOrDelete = o; };
      int getNumFrames();

      void invalidateTextures();

      static int    s_glTextures;

#ifdef MM3D_EDIT
      void addObserver( Observer * o );
      void removeObserver( Observer * o );

      void addMetaData( const char * key, const char * value );
      bool getMetaData( const char * key, char * value, size_t valueLen );
      bool getMetaData( unsigned int index, char * key, size_t keyLen, char * value, size_t valueLen );
      unsigned int getMetaDataCount();
      void clearMetaData();
      void removeLastMetaData(); // For undo only!

      void displayFrameAnimPrimitiveError();

      bool mergeAnimations( Model * model ); // For skeletal, skeletons must match
      bool mergeModels( Model * model, bool textures, AnimationMergeE mergeMode, bool emptyGroups,
            double * trans = NULL, double * rot = NULL );

      void booleanOperation( BooleanOpE op, 
            std::list<int> & listA, std::list<int> & listB );

      void drawJoints();

      void setSaved( bool o ) { if ( o ) { m_undoMgr->setSaved(); }; };
      bool getSaved()         { return m_undoMgr->isSaved(); };

      void setCanvasDrawMode( int m ) { m_canvasDrawMode = m; };
      int  getCanvasDrawMode() { return m_canvasDrawMode; };

      void setDrawJoints( DrawJointModeE m ) { m_drawJoints = m; };
      DrawJointModeE getDrawJoints() { return m_drawJoints; };

      void setDrawProjections( bool o ) { m_drawProjections = o; };
      bool getDrawProjections() { return m_drawProjections; };

      void setUndoSizeLimit( unsigned sizeLimit );
      void setUndoCountLimit( unsigned countLimit );

      int addVertex( double x, double y, double z );
      int addTriangle( unsigned vert1, unsigned vert2, unsigned vert3 );
      int addGroup( const char * name );
      int addBoneJoint( const char * name, const double & x, const double & y, const double & z, 
            const double & xrot, const double & yrot, const double & zrot,
            const int & parent = -1 );
      int addPoint( const char * name, const double & x, const double & y, const double & z, 
            const double & xrot, const double & yrot, const double & zrot,
            const int & boneId = -1 );
      int addProjection( const char * name, int type, double x, double y, double z );

      int addTexture( Texture * tex );
      int addColorMaterial( const char * name );

      int addAnimation( const AnimationModeE & mode, const char * name );

      void deleteAnimation( const AnimationModeE & mode, const unsigned & index );

      int  copyAnimation( const AnimationModeE & mode, const unsigned & anim, const char * newName );
      int  splitAnimation( const AnimationModeE & mode, const unsigned & anim, const char * newName, const unsigned & frame );
      bool joinAnimations( const AnimationModeE & mode, const unsigned & anim1, const unsigned & anim2 );
      bool mergeAnimations( const AnimationModeE & mode, const unsigned & anim1, const unsigned & anim2 );
      int  convertAnimToFrame( const AnimationModeE & mode, const unsigned & anim1, const char * newName, const unsigned & frameCount );

      bool moveAnimation( const AnimationModeE & mode, const unsigned & oldIndex, const unsigned & newIndex );

      bool clearAnimFrame( const AnimationModeE & mode, const unsigned & anim, const unsigned & frame );

      bool     setAnimFPS( const AnimationModeE & mode, const unsigned & anim, const double & fps );
      bool setAnimName( const AnimationModeE & mode, const unsigned & anim, const char * name );
      bool     setAnimFrameCount( const AnimationModeE & mode, const unsigned & anim, const unsigned & count );

      void setFrameAnimPointCount( const unsigned & pointCount );

      bool getFrameAnimPointCoords( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            const double & x, const double & y, const double & z );
      bool setFrameAnimPointCoords( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            const double & x, const double & y, const double & z );
      bool getFrameAnimPointCoords( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            double & x, double & y, double & z );

      bool setFrameAnimPointRotation( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            const double & x, const double & y, const double & z );
      bool getFrameAnimPointRotation( const unsigned & anim, const unsigned & frame, const unsigned & point, 
            double & x, double & y, double & z );

      void setFrameAnimVertexCount( const unsigned & vertexCount );

      bool setFrameAnimVertexCoords( const unsigned & anim, const unsigned & frame, const unsigned & vertex, 
            const double & x, const double & y, const double & z );

      int  setSkelAnimKeyframe( const unsigned & anim, const unsigned & frame, const unsigned & joint, const bool & isRotation, 
            const double & x, const double & y, const double & z );
      bool deleteSkelAnimKeyframe( const unsigned & anim, const unsigned & frame, const unsigned & joint, const bool & isRotation );

      bool insertSkelAnimKeyframe( const unsigned & anim, Keyframe * keyframe );
      bool removeSkelAnimKeyframe( const unsigned & anim, const unsigned & frame, const unsigned & joint, const bool & isRotation, bool release = false );

      bool setTriangleVertices( unsigned triangleNum, unsigned vert1, unsigned vert2, unsigned vert3 );
      bool getTriangleVertices( unsigned triangleNum, unsigned & vert1, unsigned & vert2, unsigned & vert3 );
      void setTriangleMarked( unsigned triangleNum, bool marked );
      void clearMarkedTriangles();

      void setTriangleProjection( unsigned triangleNum, int proj );
      int  getTriangleProjection( unsigned triangleNum );

      void deleteVertex( unsigned vertex );
      void deleteTriangle( unsigned triangle );
      void deleteGroup( unsigned group );
      void deleteBoneJoint( unsigned joint );
      void deletePoint( unsigned point );
      void deleteProjection( unsigned proj );
      void deleteTexture( unsigned texture );

      void deleteOrphanedVertices();
      void deleteFlattenedTriangles();
      void deleteSelected();

      void invertNormals( unsigned triangleNum );
      bool triangleFacesIn( unsigned triangleNum );

      bool movePosition( const Position & pos, double x, double y, double z );
      bool moveVertex( unsigned v, double x, double y, double z );
      bool moveBoneJoint( unsigned j, double x, double y, double z );
      bool movePoint( unsigned p, double x, double y, double z );
      bool moveProjection( unsigned p, double x, double y, double z );

      bool setProjectionUp( unsigned projNumber, const double *coord );
      bool setProjectionSeam( unsigned projNumber, const double *coord );

      bool   setProjectionRange( unsigned projNumber, 
            double xmin, double ymin, double xmax, double ymax );

      void   setProjectionScale( unsigned p, double scale );
      double getProjectionScale( unsigned p );

      bool setProjectionName( const unsigned & proj, const char * name );
      bool setProjectionType( const unsigned & proj, int type );
      int  getProjectionType( const unsigned & proj );
      bool setProjectionRotation( const unsigned & proj, int type );
      int  getProjectionRotation( const unsigned & proj );

      // No undo on this one
      bool relocateBoneJoint( unsigned j, double x, double y, double z );

      void beginSelectionDifference();
      void endSelectionDifference();

      bool selectVertex( unsigned v );
      bool unselectVertex( unsigned v );
      bool isVertexSelected( unsigned v );
      bool isVertexVisible( unsigned v );

      // No undo on this one
      void setVertexFree( unsigned v, bool o );
      bool isVertexFree( unsigned v );

      bool selectTriangle( unsigned t );
      bool unselectTriangle( unsigned t );
      bool isTriangleSelected( unsigned t );
      bool isTriangleVisible( unsigned t );
      bool isTriangleMarked( unsigned t );

      bool selectGroup( unsigned g );
      bool unselectGroup( unsigned g );
      bool isGroupSelected( unsigned g );
      bool isGroupVisible( unsigned g );

      bool selectBoneJoint( unsigned j );
      bool unselectBoneJoint( unsigned j );
      bool isBoneJointSelected( unsigned j );
      bool isBoneJointVisible( unsigned j );

      bool selectPoint( unsigned p );
      bool unselectPoint( unsigned p );
      bool isPointSelected( unsigned p );
      bool isPointVisible( unsigned p );

      bool selectProjection( unsigned p );
      bool unselectProjection( unsigned p );
      bool isProjectionSelected( unsigned p );

      class SelectionTest
      {
         public:
            virtual ~SelectionTest() {};
            virtual bool shouldSelect( void * element ) = 0;
      };

      bool selectInVolumeMatrix( const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test = NULL );
      bool unselectInVolumeMatrix( const Matrix & viewMat, double x1, double y1, double x2, double y2, SelectionTest * test = NULL );

      void selectFreeVertices();

      bool unselectAll();

      bool invertSelection();

      // Don't call these directly... use selection/hide selection
      bool hideVertex( unsigned );
      bool unhideVertex( unsigned );
      bool hideTriangle( unsigned );
      bool unhideTriangle( unsigned );
      bool hideJoint( unsigned );
      bool unhideJoint( unsigned );
      bool hidePoint( unsigned );
      bool unhidePoint( unsigned );

      bool hideSelected();
      bool hideUnselected();
      bool unhideAll();

      bool getBoundingRegion( double *x1, double *y1, double *z1, double *x2, double *y2, double *z2 );
      bool getSelectedBoundingRegion( double *x1, double *y1, double *z1, double *x2, double *y2, double *z2 );

      unsigned getSelectedVertexCount();
      unsigned getSelectedTriangleCount();
      unsigned getSelectedBoneJointCount();
      unsigned getSelectedPointCount();
      unsigned getSelectedProjectionCount();

      void translateSelected( const Matrix & m );
      void rotateSelected( const Matrix & m, double * point );
      void applyMatrix( const Matrix & m, OperationScopeE scope, bool animations, bool undoable );

      void subdivideSelectedTriangles();
      void unsubdivideTriangles( unsigned t1, unsigned t2, unsigned t3, unsigned t4 );
      void simplifySelectedMesh();

      void setSelectionMode( SelectionModeE m );
      inline SelectionModeE getSelectionMode() { return m_selectionMode; };

      bool setGroupName( unsigned groupNum, const char * groupName );

      bool setGroupSmooth( const unsigned & groupNum, const uint8_t & smooth );
      bool setGroupAngle( const unsigned & groupNum, const uint8_t & angle );

      void setSelectedAsGroup( unsigned groupNum );
      void addSelectedToGroup( unsigned groupNum );

      void setTextureName( unsigned textureId, const char * name );

      void setMaterialTexture( unsigned textureId, Texture * tex );
      void removeMaterialTexture( unsigned textureId );

      bool setTextureAmbient(   unsigned textureId, const float * ambient   );
      bool setTextureDiffuse(   unsigned textureId, const float * diffuse   );
      bool setTextureEmissive(  unsigned textureId, const float * emissive  );
      bool setTextureSpecular(  unsigned textureId, const float * specular  );
      bool setTextureShininess( unsigned textureId, const float & shininess );

      bool setTextureSClamp( unsigned textureId, bool clamp );
      bool setTextureTClamp( unsigned textureId, bool clamp );

      void getSelectedPositions( list<Position> & l );
      void getSelectedVertices( list<int> & l );
      void getSelectedTriangles( list<int> & l );
      void getSelectedGroups( list<int> & l );
      void getSelectedBoneJoints( list<int> & l );
      void getSelectedPoints( list<int> & l );
      void getSelectedProjections( list<int> & l );

      bool setGroupTextureId( unsigned groupNumber, int textureId );

      bool setPointName( const unsigned & point, const char * name );
      bool setPointType( const unsigned & point, int type );

      bool getPointRotation( const unsigned & point, double * rot );
      bool getPointTranslation( const unsigned & point, double * trans );

      bool setPointRotation( unsigned point, const double * rot );
      bool setPointTranslation( unsigned point, const double * trans );

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

      bool setUndoEnabled( bool o ) { bool old = m_undoEnabled; m_undoEnabled = o; return old; };

      void operationComplete( const char * opname = NULL );

      bool canUndo();
      bool canRedo();
      const char * getUndoOpName();
      const char * getRedoOpName();
      void undo();
      void redo();
      void undoCurrent();

      bool canAddOrDelete() { return (m_frameAnims.size() == 0 || m_forceAddOrDelete); };

      // These are helper functions for the boolean operations
      void removeInternalTriangles( 
            std::list<int> & listA, std::list<int> & listB );
      void removeExternalTriangles( 
            std::list<int> & listA, std::list<int> & listB );
      void removeBTriangles( 
            std::list<int> & listA, std::list<int> & listB );

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

      void addTriangleToGroup( unsigned groupNum, unsigned triangleNum );
      void removeTriangleFromGroup( unsigned groupNum, unsigned triangleNum );

      bool unselectAllVertices();
      bool unselectAllTriangles();
      bool unselectAllGroups();
      bool unselectAllBoneJoints();
      bool unselectAllPoints();
      bool unselectAllProjections();

      void setBackgroundImage( unsigned index, const char * str );
      void setBackgroundScale( unsigned index, float scale );
      void setBackgroundCenter( unsigned index, float x, float y, float z );

      const char * getBackgroundImage( unsigned index );
      float getBackgroundScale( unsigned index );
      void getBackgroundCenter( unsigned index, float & x, float & y, float & z );

#endif // MM3D_EDIT

      void setLocalMatrix( const Matrix & m ) { m_localMatrix = m; };

   protected:

      DrawingContext * getDrawingContext( ContextT context );

      void deleteGlTextures( ContextT context );

#ifdef MM3D_EDIT

      void adjustVertexIndices( unsigned index, int count );
      void adjustTriangleIndices( unsigned index, int count );
      void adjustProjectionIndices( unsigned index, int count );

      void applyCylinderProjection( unsigned int proj );
      void applySphereProjection( unsigned int proj );
      void applyPlaneProjection( unsigned int proj );

      void beginHideDifference();
      void endHideDifference();

      void hideVerticesFromTriangles();
      void unhideVerticesFromTriangles();

      void hideTrianglesFromVertices();
      void unhideTrianglesFromVertices();

      bool selectVerticesInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );
      bool selectTrianglesInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, bool connected, SelectionTest * test = NULL );
      bool selectGroupsInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );
      bool selectBoneJointsInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );
      bool selectPointsInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );
      bool selectProjectionsInVolumeMatrix( bool select, const Matrix & viewMat, double a1, double b1, double a2, double b2, SelectionTest * test = NULL );

      void selectTrianglesFromGroups();
      void selectVerticesFromTriangles();

      void selectTrianglesFromVertices( bool all = true );
      void selectGroupsFromTriangles( bool all = true );

      bool parentJointSelected( int joint );
      bool directParentJointSelected( int joint );

      void sendUndo( Undo * undo, bool listCombine = false );
      void noTexture( unsigned id );

      void updateObservers();

#endif // MM3D_EDIT

      friend class ModelFilter;

      bool          m_initialized;
      std::string   m_filename;
      std::string   m_exportFile;
      std::string   m_filterSpecificError;
      static std::string   s_lastFilterError;

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

      int    m_numFrames;

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
      int            m_changeBits;

      UndoManager * m_undoMgr;
      UndoManager * m_animUndoMgr;
      bool m_undoEnabled;
#endif // MM3D_EDIT

      Matrix   m_localMatrix;
};

extern void model_show_alloc_stats();
extern int model_free_primitives();

#endif //__MODEL_H
