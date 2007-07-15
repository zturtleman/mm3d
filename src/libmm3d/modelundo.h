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


#ifndef __MODELUNDO_H
#define __MODELUNDO_H

#include "undo.h"
#include "model.h"
#include "glmath.h"
#include "sorted_list.h"

#include <list>
#include <string>

using std::list;
using std::string;

class Model;

class ModelUndo : public Undo
{
   public:
      ModelUndo() { s_allocated++; };
      virtual ~ModelUndo() { s_allocated--; };
      
      virtual void undo( Model * ) = 0;
      virtual void redo( Model * ) = 0;

      static int s_allocated;
};

class MU_NoOp : public Undo
{
   public:
      MU_NoOp() {};
      virtual ~MU_NoOp() {};

      void undo( Model * ) {};
      void redo( Model * ) {};
      bool combine( Undo * );  // Actually has an implementation

      unsigned size() { return sizeof(MU_NoOp); };
};

class MU_TranslateSelected : public ModelUndo
{
   public:
      MU_TranslateSelected();
      virtual ~MU_TranslateSelected();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size() { return sizeof(MU_TranslateSelected); };

      void setMatrix( const Matrix & rhs );

      Matrix getMatrix() const { return m_matrix; };

   private:
      Matrix m_matrix;
};

class MU_RotateSelected : public ModelUndo
{
   public:
      MU_RotateSelected();
      virtual ~MU_RotateSelected();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size() { return sizeof(MU_RotateSelected); };

      void setMatrixPoint( const Matrix & rhs, double * point );
      Matrix getMatrix() const { return m_matrix; };

   private:

      Matrix m_matrix;
      double m_point[3];
};

class MU_ApplyMatrix : public ModelUndo
{
   public:
      MU_ApplyMatrix();
      virtual ~MU_ApplyMatrix();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size() { return sizeof(MU_ApplyMatrix); };

      void setMatrix( const Matrix & mat, Model::OperationScopeE scope, bool animations );
      Matrix getMatrix() const { return m_matrix; };

   private:
      Matrix m_matrix;
      Model::OperationScopeE m_scope;
      bool m_animations;
};

class MU_SelectionMode : public ModelUndo
{
   public:
      MU_SelectionMode();
      virtual ~MU_SelectionMode();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setSelectionMode( Model::SelectionModeE mode, Model::SelectionModeE oldMode ) { m_mode = mode; m_oldMode = oldMode; };

   private:
      Model::SelectionModeE m_mode;
      Model::SelectionModeE m_oldMode;
};

class MU_Select : public ModelUndo
{
   public:
      MU_Select( Model::SelectionModeE mode );
      virtual ~MU_Select();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      Model::SelectionModeE getSelectionMode() const { return m_mode; };

      void setSelectionDifference( int number, bool selected, bool oldSelected );
      void toggle( int number );

      unsigned diffCount() { return m_diff.size(); };

   private:
      typedef struct _SelectionDifference_t
      {
         int  number;
         bool selected;
         bool oldSelected;
         bool operator< ( const struct _SelectionDifference_t & rhs ) const
         {
            return ( this->number < rhs.number );
         }
         bool operator== ( const struct _SelectionDifference_t & rhs ) const
         {
            return ( this->number == rhs.number );
         }
      } SelectionDifferenceT;

      typedef sorted_list< SelectionDifferenceT > SelectionDifferenceList;

      Model::SelectionModeE m_mode;
      SelectionDifferenceList m_diff;
      
};

class MU_Hide : public ModelUndo
{
   public:
      MU_Hide( Model::SelectionModeE mode );
      virtual ~MU_Hide();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      Model::SelectionModeE getSelectionMode() const { return m_mode; };

      void setHideDifference( int number, bool visible );

   private:
      typedef struct _HideDifference_t
      {
         int  number;
         bool visible;
      } HideDifferenceT;

      typedef list< HideDifferenceT > HideDifferenceList;

      Model::SelectionModeE m_mode;
      HideDifferenceList m_diff;
};

class MU_InvertNormal : public ModelUndo
{
   public:
      MU_InvertNormal();
      virtual ~MU_InvertNormal();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void addTriangle( int triangle );

   private:
      list<int> m_triangles;
};

class MU_MovePrimitive : public ModelUndo
{
   public:
      MU_MovePrimitive();
      virtual ~MU_MovePrimitive();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      typedef enum _MoveType_e
      {
          MT_Vertex,
          MT_Joint,
          MT_Point,
          MT_Projection
      };
      typedef enum _MoveType_e MoveTypeE;

      void addMovePrimitive( MoveTypeE, int i, double x, double y, double z,
            double oldx, double oldy, double oldz );

   private:

      typedef struct _MovePrimitive_t
      {
         int number;
         double x;
         double y;
         double z;
         double oldx;
         double oldy;
         double oldz;
         MoveTypeE type;

         bool operator< ( const struct _MovePrimitive_t & rhs ) const
         {
            return ( this->number < rhs.number || 
                  ( this->number == rhs.number && this->type < rhs.type ) 
                  );
         };
         bool operator== ( const struct _MovePrimitive_t & rhs ) const
         {
            return ( this->number == rhs.number && this->type == rhs.type );
         };
      } MovePrimitiveT;
      typedef sorted_list<MovePrimitiveT> MovePrimitiveList;

      MovePrimitiveList m_objects;
};

class MU_SetPointRotation : public ModelUndo
{
   public:
      MU_SetPointRotation();
      virtual ~MU_SetPointRotation();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setPointRotation( int p, double x, double y, double z,
            double oldx, double oldy, double oldz );

   private:

      int number;
      double x;
      double y;
      double z;
      double oldx;
      double oldy;
      double oldz;
};

class MU_SetPointTranslation : public ModelUndo
{
   public:
      MU_SetPointTranslation();
      virtual ~MU_SetPointTranslation();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setPointTranslation( int p, double x, double y, double z,
            double oldx, double oldy, double oldz );

   private:

      int number;
      double x;
      double y;
      double z;
      double oldx;
      double oldy;
      double oldz;
};

class MU_SetTexture : public ModelUndo
{
   public:
      MU_SetTexture();
      virtual ~MU_SetTexture();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setTexture( unsigned groupNumber, int newTexture, int oldTexture );

   private:
      typedef struct _SetTexture_t
      {
         unsigned groupNumber;
         int newTexture;
         int oldTexture;
      } SetTextureT;
      typedef list<SetTextureT> SetTextureList;

      SetTextureList m_list;
};

class MU_AddVertex : public ModelUndo
{
   public:
      MU_AddVertex();
      virtual ~MU_AddVertex();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void addVertex( unsigned index, Model::Vertex * vertex );

   private:
      typedef struct _AddVertex_t
      {
         unsigned index;
         Model::Vertex * vertex;
      } AddVertexT;
      typedef list<AddVertexT> AddVertexList;

      AddVertexList m_list;
};

class MU_AddTriangle : public ModelUndo
{
   public:
      MU_AddTriangle();
      virtual ~MU_AddTriangle();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void addTriangle( unsigned index, Model::Triangle * );

   private:
      typedef struct _AddTriangle_t
      {
         unsigned index;
         Model::Triangle * triangle;
      } AddTriangleT;
      typedef list<AddTriangleT> AddTriangleList;

      AddTriangleList m_list;
};

class MU_AddGroup : public ModelUndo
{
   public:
      MU_AddGroup();
      virtual ~MU_AddGroup();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void addGroup( unsigned index, Model::Group * );

   private:
      typedef struct _AddGroup_t
      {
         unsigned index;
         Model::Group * group;
      } AddGroupT;
      typedef list<AddGroupT> AddGroupList;

      AddGroupList m_list;
};

class MU_AddTexture : public ModelUndo
{
   public:
      MU_AddTexture();
      virtual ~MU_AddTexture();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void addTexture( unsigned index, Model::Material * );

   private:
      typedef struct _AddTexture_t
      {
         unsigned index;
         Model::Material * texture;
      } AddTextureT;
      typedef list<AddTextureT> AddTextureList;

      AddTextureList m_list;
};

class MU_SetTextureCoords : public ModelUndo
{
   public:
      MU_SetTextureCoords();
      virtual ~MU_SetTextureCoords();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void addTextureCoords( unsigned triangle, unsigned vertexIndex, float s, float t, float oldS, float oldT );

   private:
      typedef struct _SetTextureCoords_t
      {
         unsigned triangle;
         unsigned vertexIndex;
         float t;
         float s;
         float oldT;
         float oldS;
         bool operator< ( const struct _SetTextureCoords_t & rhs ) const
         {
            return ( this->triangle < rhs.triangle
                  || (this->triangle == rhs.triangle && this->vertexIndex < rhs.vertexIndex ) );
         };
         bool operator == ( const struct _SetTextureCoords_t & rhs ) const
         {
            return ( this->triangle == rhs.triangle && this->vertexIndex == rhs.vertexIndex );
         };
      } SetTextureCoordsT;

      typedef sorted_list<SetTextureCoordsT> STCList;

      STCList m_list;
};

class MU_AddToGroup : public ModelUndo
{
   public:
      MU_AddToGroup();
      virtual ~MU_AddToGroup();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void addToGroup( unsigned groupNum, unsigned triangleNum );

   private:
      typedef struct _AddToGroup_t
      {
         unsigned groupNum;
         unsigned triangleNum;
      } AddToGroupT;
      typedef list<AddToGroupT> AddToGroupList;

      AddToGroupList m_list;
};

class MU_RemoveFromGroup : public ModelUndo
{
   public:
      MU_RemoveFromGroup();
      virtual ~MU_RemoveFromGroup();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void removeFromGroup( unsigned groupNum, unsigned triangleNum );

   private:
      typedef struct _RemoveFromGroup_t
      {
         unsigned groupNum;
         unsigned triangleNum;
      } RemoveFromGroupT;
      typedef list<RemoveFromGroupT> RemoveFromGroupList;

      RemoveFromGroupList m_list;
};

class MU_DeleteTriangle : public ModelUndo
{
   public:
      MU_DeleteTriangle();
      virtual ~MU_DeleteTriangle();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void deleteTriangle( unsigned triangleNum, Model::Triangle * triangle );

   private:
      typedef struct _DeleteTriangle_t
      {
         unsigned triangleNum;
         Model::Triangle * triangle;
      } DeleteTriangleT;
      typedef list<DeleteTriangleT> DeleteTriangleList;

      DeleteTriangleList m_list;
};

class MU_DeleteVertex : public ModelUndo
{
   public:
      MU_DeleteVertex();
      virtual ~MU_DeleteVertex();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void deleteVertex( unsigned vertexNum, Model::Vertex * vertex );

   private:
      typedef struct _DeleteVertex_t
      {
         unsigned vertexNum;
         Model::Vertex * vertex;
      } DeleteVertexT;
      typedef list<DeleteVertexT> DeleteVertexList;

      DeleteVertexList m_list;
};

class MU_DeleteBoneJoint : public ModelUndo
{
   public:
      MU_DeleteBoneJoint();
      virtual ~MU_DeleteBoneJoint();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void deleteBoneJoint( unsigned jointNum, Model::Joint * joint );

   private:
      unsigned       m_jointNum;
      Model::Joint * m_joint;
};

class MU_DeletePoint : public ModelUndo
{
   public:
      MU_DeletePoint();
      virtual ~MU_DeletePoint();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void deletePoint( unsigned pointNum, Model::Point * point );

   private:
      unsigned       m_pointNum;
      Model::Point * m_point;
};

class MU_DeleteProjection : public ModelUndo
{
   public:
      MU_DeleteProjection();
      virtual ~MU_DeleteProjection();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void deleteProjection( unsigned projNum, Model::TextureProjection * proj );

   private:
      unsigned       m_projNum;
      Model::TextureProjection * m_proj;
};

class MU_AddBoneJoint : public ModelUndo
{
   public:
      MU_AddBoneJoint();
      virtual ~MU_AddBoneJoint();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void addBoneJoint( unsigned jointNum, Model::Joint * joint );

   private:
      unsigned       m_jointNum;
      Model::Joint * m_joint;
};

class MU_AddPoint : public ModelUndo
{
   public:
      MU_AddPoint();
      virtual ~MU_AddPoint();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void addPoint( unsigned pointNum, Model::Point * point );

   private:
      unsigned       m_pointNum;
      Model::Point * m_point;
};

class MU_AddProjection : public ModelUndo
{
   public:
      MU_AddProjection();
      virtual ~MU_AddProjection();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void addProjection( unsigned pointNum, Model::TextureProjection * point );

   private:
      unsigned       m_projNum;
      Model::TextureProjection * m_proj;
};

class MU_DeleteGroup : public ModelUndo
{
   public:
      MU_DeleteGroup();
      virtual ~MU_DeleteGroup();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void deleteGroup( unsigned groupNum, Model::Group * group );

   private:
      typedef struct _DeleteGroup_t
      {
         unsigned groupNum;
         Model::Group * group;
      } DeleteGroupT;
      typedef list<DeleteGroupT> DeleteGroupList;

      DeleteGroupList m_list;
};

class MU_DeleteTexture : public ModelUndo
{
   public:
      MU_DeleteTexture();
      virtual ~MU_DeleteTexture();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void deleteTexture( unsigned textureNum, Model::Material * texture );

   private:
      typedef struct _DeleteTexture_t
      {
         unsigned textureNum;
         Model::Material * texture;
      } DeleteTextureT;
      typedef list<DeleteTextureT> DeleteTextureList;

      DeleteTextureList m_list;
};

class MU_SetLightProperties : public ModelUndo
{
   public:
      enum _LightType_e
      {
         LightAmbient  = 0,
         LightDiffuse,
         LightSpecular,
         LightEmissive,
         LightTypeMax  
      };
      typedef enum _LightType_e LightTypeE;

      MU_SetLightProperties();
      virtual ~MU_SetLightProperties();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setLightProperties( unsigned textureNum, LightTypeE type, const float * newLight, const float * oldLight );

   private:
      typedef struct _LightProperties_t
      {
         unsigned textureNum;
         float newLight[ LightTypeMax ][4];
         float oldLight[ LightTypeMax ][4];
         bool  isSet[ LightTypeMax ];
      } LightPropertiesT;

      typedef list<LightPropertiesT> LightPropertiesList;

      list<LightPropertiesT> m_list;
};

class MU_SetShininess : public ModelUndo
{
   public:

      MU_SetShininess();
      virtual ~MU_SetShininess();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setShininess( unsigned textureNum, const float & newValue, const float & oldValue );

   private:
      typedef struct _Shininess_t
      {
         unsigned textureNum;
         float oldValue;
         float newValue;
      } ShininessT;

      typedef list<ShininessT> ShininessList;

      list<ShininessT> m_list;
};

class MU_SetTextureName : public ModelUndo
{
   public:

      MU_SetTextureName();
      virtual ~MU_SetTextureName();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setTextureName( unsigned textureNum, const char * newName, const char * oldName );

   private:

      unsigned m_textureNum;
      string   m_newName;
      string   m_oldName;
};

class MU_SetTriangleVertices : public ModelUndo
{
   public:
      MU_SetTriangleVertices();
      virtual ~MU_SetTriangleVertices();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setTriangleVertices( unsigned triangleNum, 
            unsigned newV1, unsigned newV2, unsigned newV3,
            unsigned oldV1, unsigned oldV2, unsigned oldV3 );

   private:
      typedef struct _TriangleVertices_t
      {
         unsigned triangleNum;
         unsigned newVertices[3];
         unsigned oldVertices[3];
      } TriangleVerticesT;

      typedef list<TriangleVerticesT> TriangleVerticesList;

      TriangleVerticesList m_list;
};

class MU_SubdivideSelected : public ModelUndo
{
   public:
      MU_SubdivideSelected();
      virtual ~MU_SubdivideSelected();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size() { return sizeof( MU_SubdivideSelected ); };

   private:
};

class MU_SubdivideTriangle : public ModelUndo
{
   public:
      MU_SubdivideTriangle();
      virtual ~MU_SubdivideTriangle();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void subdivide( unsigned a, unsigned b, unsigned c, unsigned d );
      void addVertex( unsigned v );

   private:

      typedef struct _SubdivideTriangle_t
      {
         unsigned a;
         unsigned b;
         unsigned c;
         unsigned d;
      } SubdivideTriangleT;

      typedef list<SubdivideTriangleT> SubdivideTriangleList;

      SubdivideTriangleList m_list;
      list<unsigned> m_vlist;
};

class MU_ChangeAnimState : public ModelUndo
{
   public:
      MU_ChangeAnimState();
      virtual ~MU_ChangeAnimState();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setState( Model::AnimationModeE newMode, Model::AnimationModeE oldMode, unsigned anim, unsigned frame );

   protected:

      Model::AnimationModeE m_newMode;
      Model::AnimationModeE m_oldMode;
      unsigned m_anim;
      unsigned m_frame;
};

class MU_SetAnimName : public ModelUndo
{
   public:

      MU_SetAnimName();
      virtual ~MU_SetAnimName();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setName( Model::AnimationModeE mode, unsigned animNum, const char * newName, const char * oldName );

   private:

      Model::AnimationModeE m_mode;
      unsigned m_animNum;
      string   m_newName;
      string   m_oldName;
};

class MU_SetAnimFrameCount : public ModelUndo
{
   public:

      MU_SetAnimFrameCount();
      virtual ~MU_SetAnimFrameCount();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setAnimFrameCount( Model::AnimationModeE m, unsigned animNum, unsigned newCount, unsigned oldCount );

   private:

      Model::AnimationModeE m_mode;
      unsigned m_animNum;
      unsigned m_newCount;
      unsigned m_oldCount;
};

class MU_SetAnimFPS : public ModelUndo
{
   public:

      MU_SetAnimFPS();
      virtual ~MU_SetAnimFPS();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setFPS( Model::AnimationModeE mode, unsigned animNum, double newFps, double oldFps );

   private:

      Model::AnimationModeE m_mode;
      unsigned m_animNum;
      double   m_newFPS;
      double   m_oldFPS;
};

class MU_SetAnimKeyframe : public ModelUndo
{
   public:
      MU_SetAnimKeyframe();
      virtual ~MU_SetAnimKeyframe();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setAnimationData( const unsigned & anim, const unsigned & frame, const bool & isRotation );

      void addBoneJoint( int j, bool isNew, double x, double y, double z,
            double oldx, double oldy, double oldz );

   private:

      typedef struct _SetKeyframe_t
      {
         int number;
         bool isNew;
         double x;
         double y;
         double z;
         double oldx;
         double oldy;
         double oldz;

         bool operator< ( const struct _SetKeyframe_t & rhs ) const
         {
            return ( this->number < rhs.number );
         };
         bool operator== ( const struct _SetKeyframe_t & rhs ) const
         {
            return ( this->number == rhs.number );
         };
      } SetKeyFrameT;
      typedef sorted_list<SetKeyFrameT> SetKeyframeList;

      SetKeyframeList m_keyframes;
      unsigned        m_anim;
      unsigned        m_frame;
      bool            m_isRotation;
};

class MU_DeleteKeyframe : public ModelUndo
{
   public:
      MU_DeleteKeyframe();
      virtual ~MU_DeleteKeyframe();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void setAnimationData( const unsigned & anim );
      void deleteKeyframe( Model::Keyframe * keyframe );

   private:
      typedef list<Model::Keyframe *> DeleteKeyframeList;

      DeleteKeyframeList m_list;
      unsigned           m_anim;
};

class MU_SetJointName : public ModelUndo
{
   public:

      MU_SetJointName();
      virtual ~MU_SetJointName();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setName( unsigned joint, const char * newName, const char * oldName );

   private:

      unsigned m_joint;
      string   m_newName;
      string   m_oldName;
};

class MU_SetPointName : public ModelUndo
{
   public:

      MU_SetPointName();
      virtual ~MU_SetPointName();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setName( unsigned point, const char * newName, const char * oldName );

   private:

      unsigned m_point;
      string   m_newName;
      string   m_oldName;
};

class MU_SetProjectionName : public ModelUndo
{
   public:

      MU_SetProjectionName();
      virtual ~MU_SetProjectionName();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setName( unsigned projection, const char * newName, const char * oldName );

   private:

      unsigned m_projection;
      string   m_newName;
      string   m_oldName;
};

class MU_SetProjectionType : public ModelUndo
{
   public:

      MU_SetProjectionType();
      virtual ~MU_SetProjectionType();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setType( unsigned projection, int newType, int oldType );

   private:

      unsigned m_projection;
      int   m_newType;
      int   m_oldType;
};

class MU_MoveFrameVertex : public ModelUndo
{
   public:
      MU_MoveFrameVertex();
      virtual ~MU_MoveFrameVertex();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setAnimationData( const unsigned & anim, const unsigned & frame  );

      void addVertex( int v, double x, double y, double z,
            double oldx, double oldy, double oldz );

   private:

      typedef struct _MoveFrameVertex_t
      {
         int number;
         double x;
         double y;
         double z;
         double oldx;
         double oldy;
         double oldz;

         bool operator< ( const struct _MoveFrameVertex_t & rhs ) const
         {
            return ( this->number < rhs.number );
         };
         bool operator== ( const struct _MoveFrameVertex_t & rhs ) const
         {
            return ( this->number == rhs.number );
         };
      } MoveFrameVertexT;
      typedef sorted_list<MoveFrameVertexT> MoveFrameVertexList;

      MoveFrameVertexList m_vertices;
      unsigned            m_anim;
      unsigned            m_frame;
};

class MU_MoveFramePoint : public ModelUndo
{
   public:
      MU_MoveFramePoint();
      virtual ~MU_MoveFramePoint();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setAnimationData( const unsigned & anim, const unsigned & frame  );

      void addPoint( int p, double x, double y, double z,
            double oldx, double oldy, double oldz );

   private:

      typedef struct _MoveFramePoint_t
      {
         int number;
         double x;
         double y;
         double z;
         double oldx;
         double oldy;
         double oldz;

         bool operator< ( const struct _MoveFramePoint_t & rhs ) const
         {
            return ( this->number < rhs.number );
         };
         bool operator== ( const struct _MoveFramePoint_t & rhs ) const
         {
            return ( this->number == rhs.number );
         };
      } MoveFramePointT;
      typedef sorted_list<MoveFramePointT> MoveFramePointList;

      MoveFramePointList  m_points;
      unsigned            m_anim;
      unsigned            m_frame;
};

class MU_RotateFramePoint : public ModelUndo
{
   public:
      MU_RotateFramePoint();
      virtual ~MU_RotateFramePoint();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setAnimationData( const unsigned & anim, const unsigned & frame  );

      void addPointRotation( int p, double x, double y, double z,
            double oldx, double oldy, double oldz );

   private:

      typedef struct _RotateFramePoint_t
      {
         int number;
         double x;
         double y;
         double z;
         double oldx;
         double oldy;
         double oldz;

         bool operator< ( const struct _RotateFramePoint_t & rhs ) const
         {
            return ( this->number < rhs.number );
         };
         bool operator== ( const struct _RotateFramePoint_t & rhs ) const
         {
            return ( this->number == rhs.number );
         };
      } RotateFramePointT;
      typedef sorted_list<RotateFramePointT> RotateFramePointList;

      RotateFramePointList  m_points;
      unsigned            m_anim;
      unsigned            m_frame;
};

class MU_AddFrameAnimFrame : public ModelUndo
{
   public:
      MU_AddFrameAnimFrame();
      virtual ~MU_AddFrameAnimFrame();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void setAnimationData( const unsigned & anim );
      void addFrame( const unsigned & frame, Model::FrameAnimData * data );

   private:
      typedef struct _AddFrame_t
      {
         unsigned frame;
         Model::FrameAnimData * data;
      } AddFrameT;
      typedef list<AddFrameT> FrameDataList;

      FrameDataList m_list;
      unsigned      m_anim;
};

class MU_DeleteFrameAnimFrame : public ModelUndo
{
   public:
      MU_DeleteFrameAnimFrame();
      virtual ~MU_DeleteFrameAnimFrame();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void setAnimationData( const unsigned & anim );
      void deleteFrame( const unsigned & frame, Model::FrameAnimData * data );

   private:
      typedef struct _DeleteFrame_t
      {
         unsigned frame;
         Model::FrameAnimData * data;
      } DeleteFrameT;
      typedef list<DeleteFrameT> FrameDataList;

      FrameDataList    m_list;
      unsigned         m_anim;
};

class MU_SetPositionInfluence : public ModelUndo
{
   public:
      MU_SetPositionInfluence();
      virtual ~MU_SetPositionInfluence();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setPositionInfluence( bool isAdd, const Model::Position & pos, unsigned index, const Model::InfluenceT & influence );

   private:
      bool m_isAdd;
      Model::Position m_pos;
      unsigned m_index;
      Model::InfluenceT m_influence;
};

class MU_UpdatePositionInfluence : public ModelUndo
{
   public:
      MU_UpdatePositionInfluence();
      virtual ~MU_UpdatePositionInfluence();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void updatePositionInfluence( const Model::Position & pos, 
            const Model::InfluenceT & newInf,
            const Model::InfluenceT & oldInf );

   private:
      Model::Position m_pos;
      Model::InfluenceT m_newInf;
      Model::InfluenceT m_oldInf;
};

class MU_SetVertexBoneJoint : public ModelUndo
{
   public:
      MU_SetVertexBoneJoint();
      virtual ~MU_SetVertexBoneJoint();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setVertexBoneJoint( const unsigned & vertex, 
            const int & newBone, const int & oldBone );

   private:
      typedef struct _SetJoint_t
      {
         unsigned vertex;
         int newBone;
         int oldBone;
      } SetJointT;
      typedef list<SetJointT> SetJointList;

      SetJointList m_list;
};

class MU_SetPointBoneJoint : public ModelUndo
{
   public:
      MU_SetPointBoneJoint();
      virtual ~MU_SetPointBoneJoint();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setPointBoneJoint( const unsigned & point, 
            const int & newBone, const int & oldBone );

   private:
      typedef struct _SetJoint_t
      {
         unsigned point;
         int newBone;
         int oldBone;
      } SetJointT;
      typedef list<SetJointT> SetJointList;

      SetJointList m_list;
};

class MU_SetTriangleProjection : public ModelUndo
{
   public:
      MU_SetTriangleProjection();
      virtual ~MU_SetTriangleProjection();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setTriangleProjection( const unsigned & vertex, 
            const int & newProj, const int & oldProj );

   private:
      typedef struct _SetProjection_t
      {
         unsigned triangle;
         int newProj;
         int oldProj;
      } SetProjectionT;
      typedef list<SetProjectionT> SetProjectionList;

      SetProjectionList m_list;
};

class MU_AddAnimation : public ModelUndo
{
   public:
      MU_AddAnimation();
      virtual ~MU_AddAnimation();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void redoRelease();

      unsigned size();

      void addAnimation( const unsigned & anim, Model::SkelAnim  * skelanim  );
      void addAnimation( const unsigned & anim, Model::FrameAnim * frameanim );

   private:
      
      unsigned m_anim;
      Model::SkelAnim  * m_skelAnim;
      Model::FrameAnim * m_frameAnim;
};

class MU_DeleteAnimation : public ModelUndo
{
   public:
      MU_DeleteAnimation();
      virtual ~MU_DeleteAnimation();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      void undoRelease();

      unsigned size();

      void deleteAnimation( const unsigned & anim, Model::SkelAnim  * skelanim  );
      void deleteAnimation( const unsigned & anim, Model::FrameAnim * frameanim );

   private:
      
      unsigned m_anim;
      Model::SkelAnim  * m_skelAnim;
      Model::FrameAnim * m_frameAnim;
};

class MU_SetJointParent : public ModelUndo
{
   public:
      MU_SetJointParent();
      virtual ~MU_SetJointParent();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * ) { return false; };

      unsigned size() { return sizeof(MU_SetJointParent); };

      void setJointParent( unsigned joint, int newParent, int oldParent );

   protected:
      unsigned m_joint;
      int m_newParent;
      int m_oldParent;
};

class MU_SetJointRotation : public ModelUndo
{
   public:
      MU_SetJointRotation();
      virtual ~MU_SetJointRotation();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * ) { return false; };

      unsigned size() { return sizeof(MU_SetJointRotation); };

      void setJointRotation( const unsigned & joint, const double * newRotation, const double * oldRotation );

   protected:
      unsigned m_joint;
      double   m_newRotation[3];
      double   m_oldRotation[3];
};

class MU_SetJointTranslation : public ModelUndo
{
   public:
      MU_SetJointTranslation();
      virtual ~MU_SetJointTranslation();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * ) { return false; };

      unsigned size() { return sizeof(MU_SetJointTranslation); };

      void setJointTranslation( const unsigned & joint, const double * newTranslation, const double * oldTranslation );

   protected:
      unsigned m_joint;
      double   m_newTranslation[3];
      double   m_oldTranslation[3];
};

class MU_SetProjectionUp : public ModelUndo
{
   public:
      MU_SetProjectionUp();
      virtual ~MU_SetProjectionUp();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * ) { return false; };

      unsigned size() { return sizeof(MU_SetProjectionUp); };

      void setProjectionUp( const unsigned & proj, const double * newUp, const double * oldUp );

   protected:
      unsigned m_proj;
      double   m_newUp[3];
      double   m_oldUp[3];
};

class MU_SetProjectionSeam : public ModelUndo
{
   public:
      MU_SetProjectionSeam();
      virtual ~MU_SetProjectionSeam();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * ) { return false; };

      unsigned size() { return sizeof(MU_SetProjectionSeam); };

      void setProjectionSeam( const unsigned & proj, const double * newSeam, const double * oldSeam );

   protected:
      unsigned m_proj;
      double   m_newSeam[3];
      double   m_oldSeam[3];
};

class MU_SetProjectionRange : public ModelUndo
{
   public:
      MU_SetProjectionRange();
      virtual ~MU_SetProjectionRange();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * ) { return false; };

      unsigned size() { return sizeof(MU_SetProjectionRange); };

      void setProjectionRange( const unsigned & proj, 
            double newXMin, double newYMin, double newXMax, double newYMax,
            double oldXMin, double oldYMin, double oldXMax, double oldYMax );

   protected:
      unsigned m_proj;
      double   m_newRange[4];
      double   m_oldRange[4];
};

class MU_SetGroupSmooth : public ModelUndo
{
   public:
      MU_SetGroupSmooth();
      virtual ~MU_SetGroupSmooth();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setGroupSmooth( const unsigned & group, 
            const uint8_t & newSmooth, const uint8_t & oldSmooth );

   private:
      typedef struct _SetSmooth_t
      {
         unsigned group;
         uint8_t newSmooth;
         uint8_t oldSmooth;
      } SetSmoothT;
      typedef list<SetSmoothT> SetSmoothList;

      SetSmoothList m_list;
};

class MU_SetGroupAngle : public ModelUndo
{
   public:
      MU_SetGroupAngle();
      virtual ~MU_SetGroupAngle();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setGroupAngle( const unsigned & group, 
            const uint8_t & newAngle, const uint8_t & oldAngle );

   private:
      typedef struct _SetAngle_t
      {
         unsigned group;
         uint8_t newAngle;
         uint8_t oldAngle;
      } SetAngleT;
      typedef list<SetAngleT> SetAngleList;

      SetAngleList m_list;
};

class MU_SetGroupName : public ModelUndo
{
   public:

      MU_SetGroupName();
      virtual ~MU_SetGroupName();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setGroupName( unsigned groupNum, const char * newName, const char * oldName );

   private:

      unsigned m_groupNum;
      string   m_newName;
      string   m_oldName;
};

class MU_MoveAnimation : public ModelUndo
{
   public:

      MU_MoveAnimation();
      virtual ~MU_MoveAnimation();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void moveAnimation( const Model::AnimationModeE & mode, const unsigned & oldIndex, const unsigned & newIndex );

   private:

      Model::AnimationModeE m_mode;
      unsigned m_oldIndex;
      unsigned m_newIndex;
};

class MU_SetFrameAnimVertexCount : public ModelUndo
{
   public:

      MU_SetFrameAnimVertexCount();
      virtual ~MU_SetFrameAnimVertexCount();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * ) { return false; };

      unsigned size();

      void setCount( const unsigned & newCount, const unsigned & oldCount );

   private:

      unsigned m_newCount;
      unsigned m_oldCount;
};

class MU_SetFrameAnimPointCount : public ModelUndo
{
   public:

      MU_SetFrameAnimPointCount();
      virtual ~MU_SetFrameAnimPointCount();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * ) { return false; };

      unsigned size();

      void setCount( const unsigned & newCount, const unsigned & oldCount );

   private:

      unsigned m_newCount;
      unsigned m_oldCount;
};

class MU_SetMaterialClamp : public ModelUndo
{
   public:
      MU_SetMaterialClamp();
      virtual ~MU_SetMaterialClamp();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setMaterialClamp( const unsigned & material, const bool & isS,
            const bool & newClamp, const bool & oldClamp );

   private:
      unsigned m_material;
      bool m_isS;
      bool m_oldClamp;
      bool m_newClamp;
};

class MU_SetMaterialTexture : public ModelUndo
{
   public:
      MU_SetMaterialTexture();
      virtual ~MU_SetMaterialTexture();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setMaterialTexture( const unsigned & material, 
            Texture * newTexture, Texture * oldTexture );

   private:
      unsigned m_material;
      Texture * m_oldTexture;
      Texture * m_newTexture;
};

class MU_SetBackgroundImage : public ModelUndo
{
   public:
      MU_SetBackgroundImage();
      virtual ~MU_SetBackgroundImage();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setBackgroundImage( const unsigned & index, 
            const char * newFilename, const char * oldFilename );

   private:
      unsigned m_index;
      string   m_newFilename;
      string   m_oldFilename;
};

class MU_SetBackgroundScale : public ModelUndo
{
   public:
      MU_SetBackgroundScale();
      virtual ~MU_SetBackgroundScale();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setBackgroundScale( const unsigned & index, 
            float newScale, float oldScale );

   private:
      unsigned m_index;
      float    m_newScale;
      float    m_oldScale;
};

class MU_SetBackgroundCenter : public ModelUndo
{
   public:
      MU_SetBackgroundCenter();
      virtual ~MU_SetBackgroundCenter();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void setBackgroundCenter( const unsigned & index, 
            float newX, float newY, float newY,
            float oldX, float oldY, float oldY );

   private:
      unsigned m_index;
      float    m_new[3];
      float    m_old[3];
};

class MU_AddMetaData : public ModelUndo
{
   public:
      MU_AddMetaData();
      virtual ~MU_AddMetaData();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void addMetaData( const std::string & key,
            const std::string & value );

   private:
      std::string m_key;
      std::string m_value;
};

class MU_ClearMetaData : public ModelUndo
{
   public:
      MU_ClearMetaData();
      virtual ~MU_ClearMetaData();

      void undo( Model * );
      void redo( Model * );
      bool combine( Undo * );

      unsigned size();

      void clearMetaData( const Model::MetaDataList & list );

   private:
      Model::MetaDataList m_list;
};

#endif //  __MODELUNDO_H
