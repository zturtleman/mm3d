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


#include "model.h"

#include "texmgr.h"
#include "texture.h"
#include "log.h"

#ifdef MM3D_EDIT
#include "modelstatus.h"
#include "undomgr.h"
#include "modelundo.h"
#endif // MM3D_EDIT

#include "translate.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <map>
#include <vector>

//#define MODEL_ANIMATION

using std::map;
using std::vector;

#ifdef MM3D_EDIT

typedef struct _SplitEdges_t
{
   unsigned a;
   unsigned b;
   unsigned vNew;
   bool operator< ( const struct _SplitEdges_t & rhs ) const
   {
      return ( this->a < rhs.a 
            || ( this->a == rhs.a && this->b < rhs.b ) );
   };
   bool operator== ( const struct _SplitEdges_t & rhs ) const
   {
      return ( this->a == rhs.a && this->b == rhs.b );
   };
} SplitEdgesT;

#endif // MM3D_EDIT

// used to calculate smoothed normals in calculateNormals
class NormAccum
{
public:
   NormAccum() { memset( norm, 0, sizeof(norm) ); }
   float norm[3];
};

// used to calculate smoothed normals in calculateNormals
class NormAngleAccum
{
public:
   NormAngleAccum() { memset( norm, 0, sizeof(norm) ); angle = 0.0f; }
   NormAngleAccum( const NormAngleAccum & rhs )
      : angle( rhs.angle )
   {
      memcpy( norm, rhs.norm, sizeof(norm) );
   }
   float norm[3];
   float angle;
};

std::string Model::s_lastFilterError = "No error";
static int _allocated = 0;

const double TOLERANCE = 0.00005;
const double ATOLERANCE = 0.00000001;

// returns:
//    1 = in front
//    0 = on plane
//   -1 = in back
static int _pointInPlane( double * coord, double * triCoord, double * triNorm )
{
   double btoa[3];

   for ( int j = 0; j < 3; j++ )
   {
      btoa[j] = coord[j] - triCoord[j];
   }
   normalize3( btoa );

   double c = dot3( btoa, triNorm );

   if ( c < -ATOLERANCE )
      return -1;
   else if ( c > ATOLERANCE )
      return 1;
   else
      return 0;
}

// NOTE: Assumes coord is in plane of p1,p2,p3
//
// returns:
//    1 = inside triangle
//    0 = on triangle edge
//   -1 = outside triangle
static int _pointInTriangle( double * coord, 
      double * p1,
      double * p2,
      double * p3 )
{
   int vside[3];

   int i;

   double avg[3];
   double normal[3];

   for ( i = 0; i < 3; i++ )
   {
      avg[i] = (p1[i] + p2[i] + p3[i]) / 3.0;
   }

   calculate_normal( normal,
         p1, p2, p3 );

   for ( i = 0; i < 3; i++ )
   {
      avg[i] += normal[i];
   }

   calculate_normal( normal,
         p1, p2, avg );
   vside[0] = _pointInPlane( coord, avg, normal );
   calculate_normal( normal,
         p2, p3, avg );
   vside[1] = _pointInPlane( coord, avg, normal );
   calculate_normal( normal,
         p3, p1, avg );
   vside[2] = _pointInPlane( coord, avg, normal );

   if ( vside[0] < 0 && vside[1] < 0 && vside[2] < 0 )
   {
      return 1;
   }
   if ( vside[0] > 0 && vside[1] > 0 && vside[2] > 0 )
   {
      return 1;
   }
   if ( vside[0] == 0 && vside[1] == vside[2] )
   {
      return 0;
   }
   if ( vside[1] == 0 && vside[0] == vside[2] )
   {
      return 0;
   }
   if ( vside[2] == 0 && vside[0] == vside[1] )
   {
      return 0;
   }
   if ( vside[0] == 0 && vside[1] == 0 )
   {
      return 0;
   }
   if ( vside[0] == 0 && vside[2] == 0 )
   {
      return 0;
   }
   if ( vside[1] == 0 && vside[2] == 0 )
   {
      return 0;
   }
   return -1;
}

// returns:
//    ipoint = coordinates of edge line and plane
//    int    = 0 is no intersection, +1 is toward p2 from p1, -1 is away from p2 from p1
static int _findEdgePlaneIntersection( double * ipoint, double * p1, double * p2, 
      double * triCoord, double * triNorm )
{
   double edgeVec[3];
   edgeVec[0] = p2[0] - p1[0];
   edgeVec[1] = p2[1] - p1[1];
   edgeVec[2] = p2[2] - p1[2];

   /*
   log_debug( "finding intersection with plane with line from %f,%f,%f to %f,%f,%f\n",
         p1[0], p1[1], p1[2], 
         p2[0], p2[1], p2[2] );
   */

   double a =   dot3( edgeVec, triNorm );
   double d = - dot3( triCoord, triNorm );

   // prevent divide by zero
   if ( fabs(a) < TOLERANCE )
   {
      // edge is parallel to plane, the value of ipoint is undefined
      return 0;
   }

   // distance along edgeVec p1 to plane
   double dist = -(d + dot3( p1, triNorm )) / a;

   // dist is % of velocity vector, scale to get impact point
   edgeVec[0] *= dist;
   edgeVec[1] *= dist;
   edgeVec[2] *= dist;

   ipoint[0] = p1[0] + edgeVec[0];
   ipoint[1] = p1[1] + edgeVec[1];
   ipoint[2] = p1[2] + edgeVec[2];

   // Distance is irrelevant, we just want to know where 
   // the intersection is
   if ( dist >= 0.0 )
   {
      return 1;
   }
   else
   {
      return -1;
   }
}


Model::Observer::~Observer()
{
}

Model::Model()
   : m_initialized( true ),
     m_filename( "" ),
     m_validBspTree( false ),
     m_canvasDrawMode( 0 ),
     m_perspectiveDrawMode( 3 ),
     m_drawJoints( JOINTMODE_BONES ),
     m_drawProjections( true ),
     m_validNormals( false ),
     m_validJoints( false ),
     m_forceAddOrDelete( false ),
     m_animationMode( ANIMMODE_NONE ),
     m_currentFrame( 0 ),
     m_currentAnim( 0 ),
     m_currentTime( 0.0 )
#ifdef MM3D_EDIT
   ,
     m_saved( true ),
     m_selectionMode( SelectVertices ),
     m_selecting( false ),
     m_changeBits( ChangeAll ),
     m_undoEnabled( false )
#endif // MM3D_EDIT
{
   _allocated++;

   m_localMatrix.loadIdentity();

#ifdef MM3D_EDIT
   for ( unsigned t = 0; t < MAX_BACKGROUND_IMAGES; t++ )
   {
      m_background[t] = new BackgroundImage();
   }

   m_undoMgr     = new UndoManager();
   //m_animUndoMgr = new UndoManager();
#endif // MM3D_EDIT

}

Model::~Model()
{
   m_bspTree.clear();
   m_selectedUv.clear();

   log_debug( "deleting model\n" );
   DrawingContextList::iterator it;
   for ( it = m_drawingContexts.begin(); it != m_drawingContexts.end(); it++ )
   {
      deleteGlTextures( (*it)->m_context );
   }
   m_drawingContexts.clear();

   while ( ! m_vertices.empty() )
   {
      m_vertices.back()->release();
      m_vertices.pop_back();
   }

   while ( ! m_triangles.empty() )
   {
      m_triangles.back()->release();
      m_triangles.pop_back();
   }

   while ( ! m_groups.empty() )
   {
      m_groups.back()->release();
      m_groups.pop_back();
   }

   while ( ! m_materials.empty() )
   {
      m_materials.back()->release();
      m_materials.pop_back();
   }

   while ( ! m_skelAnims.empty() )
   {
      m_skelAnims.back()->release();
      m_skelAnims.pop_back();
   }

   while ( ! m_joints.empty() )
   {
      m_joints.back()->release();
      m_joints.pop_back();
   }

   while ( ! m_points.empty() )
   {
      m_points.back()->release();
      m_points.pop_back();
   }

   while ( ! m_projections.empty() )
   {
      m_projections.back()->release();
      m_projections.pop_back();
   }

   while ( ! m_frameAnims.empty() )
   {
      m_frameAnims.back()->release();
      m_frameAnims.pop_back();
   }

#ifdef MM3D_EDIT
   for ( unsigned t = 0; t < 6; t++ )
   {
      delete m_background[t];
      m_background[t] = NULL;
   }

   delete m_undoMgr;
   //delete m_animUndoMgr;
#endif // MM3D_EDIT

   _allocated--;
}

const char * Model::errorToString( Model::ModelErrorE e, Model * model )
{
   switch ( e )
   {
      case ERROR_NONE:
         return QT_TRANSLATE_NOOP( "LowLevel", "Success" );
      case ERROR_CANCEL:
         return QT_TRANSLATE_NOOP( "LowLevel", "Canceled" );
      case ERROR_UNKNOWN_TYPE:
         return QT_TRANSLATE_NOOP( "LowLevel", "Unrecognized file extension (unknown type)" );
      case ERROR_UNSUPPORTED_OPERATION:
         return QT_TRANSLATE_NOOP( "LowLevel", "Operation not supported for this file type" );
      case ERROR_BAD_ARGUMENT:
         return QT_TRANSLATE_NOOP( "LowLevel", "Invalid argument (internal error, probably null pointer argument)" );
      case ERROR_NO_FILE:
         return QT_TRANSLATE_NOOP( "LowLevel", "File does not exist" );
      case ERROR_NO_ACCESS:
         return QT_TRANSLATE_NOOP( "LowLevel", "Permission denied" );
      case ERROR_FILE_OPEN:
         return QT_TRANSLATE_NOOP( "LowLevel", "Could not open file" );
      case ERROR_FILE_READ:
         return QT_TRANSLATE_NOOP( "LowLevel", "Could not read from file" );
      case ERROR_BAD_MAGIC:
         return QT_TRANSLATE_NOOP( "LowLevel", "File is the wrong type or corrupted" );
      case ERROR_UNSUPPORTED_VERSION:
         return QT_TRANSLATE_NOOP( "LowLevel", "Unsupported version" );
      case ERROR_BAD_DATA:
         return QT_TRANSLATE_NOOP( "LowLevel", "File contains invalid data" );
      case ERROR_UNEXPECTED_EOF:
         return QT_TRANSLATE_NOOP( "LowLevel", "Unexpected end of file" );
      case ERROR_EXPORT_ONLY:
         return QT_TRANSLATE_NOOP( "LowLevel", "Write not supported, try \"Export...\"" );
      case ERROR_FILTER_SPECIFIC:
         if ( model )
         {
            return model->getFilterSpecificError();
         }
         else
         {
            return getLastFilterSpecificError();
         }
      case ERROR_UNKNOWN:
         return QT_TRANSLATE_NOOP( "LowLevel", "Unknown error"  );
   }

   return QT_TRANSLATE_NOOP( "LowLevel", "Invalid error code" );
}

bool Model::operationFailed( Model::ModelErrorE err )
{
   return (err != ERROR_NONE && err != ERROR_CANCEL );
}

void Model::pushError( const std::string & err )
{
   m_loadErrors.push_back( err );
}

std::string Model::popError()
{
   std::string rval = "";
   if ( !m_loadErrors.empty() )
   {
      rval = m_loadErrors.front();
      m_loadErrors.pop_front();
   }
   return rval;
}

#ifdef MM3D_EDIT

void Model::updateObservers()
{
   for ( ObserverList::iterator it = m_observers.begin(); it != m_observers.end(); it++ )
   {
      (*it)->modelChanged( m_changeBits );
   }
   m_changeBits = 0;
}

void Model::addObserver( Model::Observer * o )
{
   m_observers.push_back( o );
}

void Model::removeObserver( Model::Observer * o )
{
   ObserverList::iterator it;
   for ( it = m_observers.begin(); it != m_observers.end(); it++ )
   {
      if ( *it == o )
      {
         m_observers.erase( it );
         return;
      }
   }
}

void Model::setUndoSizeLimit( unsigned sizeLimit )
{
   m_undoMgr->setSizeLimit( sizeLimit );
}

void Model::setUndoCountLimit( unsigned countLimit )
{
   m_undoMgr->setCountLimit( countLimit );
}

bool Model::isVertexVisible( unsigned v ) const
{
   LOG_PROFILE();

   if ( v < m_vertices.size() )
   {
      return m_vertices[v]->m_visible;
   }
   else
   {
      return false;
   }
}

bool Model::isTriangleVisible( unsigned v ) const
{
   LOG_PROFILE();

   if ( v < m_triangles.size() )
   {
      return m_triangles[v]->m_visible;
   }
   else
   {
      return false;
   }
}

bool Model::isGroupVisible( unsigned v ) const
{
   LOG_PROFILE();

   if ( v < m_groups.size() )
   {
      return m_groups[v]->m_visible;
   }
   else
   {
      return false;
   }
}

bool Model::isBoneJointVisible( unsigned j ) const
{
   LOG_PROFILE();

   if ( j < m_joints.size() )
   {
      return m_joints[j]->m_visible;
   }
   else
   {
      return false;
   }
}

bool Model::isPointVisible( unsigned p ) const
{
   LOG_PROFILE();

   if ( p < m_points.size() )
   {
      return m_points[p]->m_visible;
   }
   else
   {
      return false;
   }
}

int Model::addVertex( double x, double y, double z )
{
   if ( m_animationMode )
   {
      return -1;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return -1;
   }

   int num = m_vertices.size();

   m_changeBits |= AddGeometry;

   Vertex * vertex = Vertex::get();

   vertex->m_coord[0] = x;
   vertex->m_coord[1] = y;
   vertex->m_coord[2] = z;
   vertex->m_free = false;
   m_vertices.push_back( vertex );

   MU_AddVertex * undo = new MU_AddVertex();
   undo->addVertex( num, vertex );
   sendUndo( undo );

   return num;
}

void Model::setVertexFree( unsigned v, bool o )
{
   if (v < m_vertices.size() )
   {
      m_vertices[v]->m_free = o;
   }
}

bool Model::isVertexFree( unsigned v ) const
{
   if (v < m_vertices.size() )
   {
      return m_vertices[v]->m_free;
   }
   return false;
}

int Model::addTriangle( unsigned v1, unsigned v2, unsigned v3 )
{
   if ( m_animationMode )
   {
      return -1;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return -1;
   }

   m_changeBits |= AddGeometry;

   if ( v1 < m_vertices.size() && v2 < m_vertices.size() && v3 < m_vertices.size() )
   {
      int num = m_triangles.size();
      //log_debug( "adding triangle %d for vertices %d,%d,%d\n", num, v1, v2, v3 );

      Triangle * triangle = Triangle::get();
      triangle->m_vertexIndices[0] = v1;
      triangle->m_vertexIndices[1] = v2;
      triangle->m_vertexIndices[2] = v3;

      m_triangles.push_back( triangle );

      invalidateNormals();

      MU_AddTriangle * undo = new MU_AddTriangle();
      undo->addTriangle( num, triangle );
      sendUndo( undo );

      return num;
   }
   else
   {
      return -1;
   }
}

int Model::addBoneJoint( const char * name, double x, double y, double z, 
      double xrot, double yrot, double zrot, int parent )
{
   if ( m_animationMode || name == NULL || parent >= (int) m_joints.size() )
   {
      return -1;
   }

   m_changeBits |= AddOther;

   int num = m_joints.size();

   Joint * joint = Joint::get();

   joint->m_parent = parent;
   joint->m_name   = name;

   double trans[3] = { x, y, z };
   double rot[3]   = { xrot, yrot, zrot };

   log_debug( "New joint at %f,%f,%f\n", x, y, z );

   joint->m_absolute.loadIdentity();
   joint->m_absolute.setRotation( rot );
   joint->m_absolute.setTranslation( trans );

   if ( parent >= 0 )
   {
      m_joints[ parent ]->m_absolute.inverseTranslateVector( trans );
      m_joints[ parent ]->m_absolute.inverseRotateVector( trans );
      Matrix minv = m_joints[ parent ]->m_absolute.getInverse();
      Matrix mr;
      mr.setRotation( rot );
      mr = mr * minv;
      mr.getRotation( rot );
   }

   for ( int t = 0; t < 3; t++ )
   {
      joint->m_localTranslation[t] = trans[t];
      joint->m_localRotation[t]    = rot[t];
   }
   //joint->m_localRotation[0] = xrot;
   //joint->m_localRotation[1] = yrot;
   //joint->m_localRotation[2] = zrot;

   //m_joints.push_back( joint );
   num = m_joints.size();
   insertBoneJoint( num, joint );

   setupJoints();

   MU_AddBoneJoint * undo = new MU_AddBoneJoint();
   undo->addBoneJoint( num, joint );
   sendUndo( undo );

   return num;
}

int Model::addPoint( const char * name, double x, double y, double z, 
      double xrot, double yrot, double zrot, int boneId )
{
   if ( m_animationMode || name == NULL || boneId >= (int) m_joints.size())
   {
      return -1;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return -1;
   }

   m_changeBits |= AddOther;

   int num = m_points.size();

   log_debug( "New point at %f,%f,%f\n", x, y, z );

   Point * point = Point::get();

   point->m_name   = name;

   point->m_trans[0] = x;
   point->m_trans[1] = y;
   point->m_trans[2] = z;
   point->m_rot[0]   = xrot;
   point->m_rot[1]   = yrot;
   point->m_rot[2]   = zrot;

   num = m_points.size();
   insertPoint( num, point );

   MU_AddPoint * undo = new MU_AddPoint();
   undo->addPoint( num, point );
   sendUndo( undo );

   setupJoints();

   return num;
}

int Model::addProjection( const char * name, int type, double x, double y, double z )
{
   if ( m_animationMode || name == NULL )
   {
      return -1;
   }

   m_changeBits |= AddOther;

   int num = m_projections.size();

   log_debug( "New projection at %f,%f,%f\n", x, y, z );

   TextureProjection * proj = TextureProjection::get();

   proj->m_name   = name;
   proj->m_type   = type;

   proj->m_pos[0] = x;
   proj->m_pos[1] = y;
   proj->m_pos[2] = z;

   proj->m_upVec[0] = 0.0;
   proj->m_upVec[1] = 1.0;
   proj->m_upVec[2] = 0.0;

   proj->m_seamVec[0] =  0.0;
   proj->m_seamVec[1] =  0.0;
   proj->m_seamVec[2] = -1.0;

   proj->m_range[0][0] =  0.0;  // min x
   proj->m_range[0][1] =  0.0;  // min y
   proj->m_range[1][0] =  1.0;  // max x
   proj->m_range[1][1] =  1.0;  // max y

   insertProjection( num, proj );

   MU_AddProjection * undo = new MU_AddProjection();
   undo->addProjection( num, proj );
   sendUndo( undo );

   return num;
}

bool Model::setTriangleVertices( unsigned triangleNum, unsigned vert1, unsigned vert2, unsigned vert3 )
{
   if ( m_animationMode )
   {
      return false;
   }

   unsigned vertexCount = m_vertices.size();
   if ( triangleNum < m_triangles.size() 
         && (vert1 < vertexCount && vert2 < vertexCount && vert3 < vertexCount ) )
   {
      MU_SetTriangleVertices * undo = new MU_SetTriangleVertices();
      undo->setTriangleVertices( triangleNum,
            vert1, vert2, vert3,
            m_triangles[ triangleNum ]->m_vertexIndices[0],
            m_triangles[ triangleNum ]->m_vertexIndices[1],
            m_triangles[ triangleNum ]->m_vertexIndices[2]
            );
      sendUndo( undo );

      m_triangles[triangleNum]->m_vertexIndices[0] = vert1;
      m_triangles[triangleNum]->m_vertexIndices[1] = vert2;
      m_triangles[triangleNum]->m_vertexIndices[2] = vert3;

      invalidateNormals();

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getTriangleVertices( unsigned triangleNum, unsigned & vert1, unsigned & vert2, unsigned & vert3 ) const
{
   if ( triangleNum < m_triangles.size() )
   {
      vert1 = m_triangles[ triangleNum ] ->m_vertexIndices[0];
      vert2 = m_triangles[ triangleNum ] ->m_vertexIndices[1];
      vert3 = m_triangles[ triangleNum ] ->m_vertexIndices[2];

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::deleteVertex( unsigned vertexNum )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return false;
   }

   if ( vertexNum >= m_vertices.size() )
   {
      return false;
   }

   MU_DeleteVertex * undo = new MU_DeleteVertex();
   undo->deleteVertex( vertexNum, m_vertices[vertexNum] );
   sendUndo( undo );

   removeVertex( vertexNum );
   return true;
}

bool Model::deleteTriangle( unsigned triangleNum )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return false;
   }

   if ( triangleNum >= m_triangles.size() )
   {
      return false;
   }

   // remove it from any groups
   for ( unsigned g = 0; g < m_groups.size(); g++ )
   {
      removeTriangleFromGroup( g, triangleNum );
   }

   // Delete triangle
   MU_DeleteTriangle * undo = new MU_DeleteTriangle();
   undo->deleteTriangle( triangleNum, m_triangles[ triangleNum ] );
   sendUndo( undo );

   removeTriangle( triangleNum );
   return true;
}

bool Model::deleteBoneJoint( unsigned joint )
{
   if ( m_animationMode )
   {
      return false;
   }

   if ( joint >= m_joints.size() )
   {
      return false;
   }

   unsigned count = m_joints.size();

   // Break out early if this is a root joint and it has a child
   if ( m_joints[joint]->m_parent < 0 )
   {
      for ( unsigned j = 0; j < count; j++ )
      {
         if ( j != joint )
         {
            if ( m_joints[j]->m_parent == (int) joint )
            {
               model_status( this, StatusError, STATUSTIME_LONG, transll( QT_TRANSLATE_NOOP( "LowLevel", "Cannot delete root joint" )).c_str() );
               return false;
            }
         }
      }
   }

   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      removeVertexInfluence( v, joint );
   }

   for ( unsigned p = 0; p < m_points.size(); p++ )
   {
      removePointInfluence( p, joint );
   }

   Matrix m;
   int parent = joint;
   do
   {
      parent = m_joints[ parent ]->m_parent;
   } while ( parent >= 0 && m_joints[ parent ]->m_selected );

   if ( parent >= 0 )
   {
      m = m_joints[ m_joints[joint]->m_parent ]->m_absolute.getInverse();
   }

   for ( unsigned j = 0; j < m_joints.size(); j++ )
   {
      if ( m_joints[j]->m_parent == (int) joint )
      {
         setBoneJointParent( j, m_joints[joint]->m_parent );

         m_joints[j]->m_absolute = m_joints[j]->m_absolute * m;
         double rot[3];
         double trans[3];

         m_joints[j]->m_absolute.getRotation( rot );
         m_joints[j]->m_absolute.getTranslation( trans );

         setBoneJointRotation( j, rot );
         setBoneJointTranslation( j, trans );
      }
   }

   Joint * ptr = m_joints[joint];
   removeBoneJoint( joint );
   m_validJoints = false;

   MU_DeleteBoneJoint * undo = new MU_DeleteBoneJoint();
   undo->deleteBoneJoint( joint, ptr );
   sendUndo( undo );

   log_debug( "parent was %d\n", parent );

   setupJoints();

   return true;
}

bool Model::deletePoint( unsigned point )
{
   if ( m_animationMode )
   {
      return false;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return false;
   }

   if ( point >= m_points.size() )
   {
      return false;
   }

   Point * ptr = m_points[point];
   removePoint( point );

   MU_DeletePoint * undo = new MU_DeletePoint();
   undo->deletePoint( point, ptr );
   sendUndo( undo );

   setupJoints();
   return true;
}

bool Model::deleteProjection( unsigned proj )
{
   if ( m_animationMode )
   {
      return false;
   }

   if ( proj >= m_projections.size() )
   {
      return false;
   }

   TextureProjection * ptr = m_projections[ proj ];
   removeProjection( proj );

   MU_DeleteProjection * undo = new MU_DeleteProjection();
   undo->deleteProjection( proj, ptr );
   sendUndo( undo );
   return true;
}

void Model::deleteOrphanedVertices()
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return;
   }

   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      m_vertices[v]->m_marked = false;
   }

   for ( unsigned t = 0; t< m_triangles.size(); t++ )
   {
      for ( unsigned v = 0; v < 3; v++ )
      {
         m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_marked = true;
      }
   }

   for ( int v = m_vertices.size() - 1; v >= 0; v-- )
   {
      if ( ! m_vertices[v]->m_marked 
            && ! m_vertices[v]->m_free )
      {
         deleteVertex( v );
      }
   }
}

void Model::deleteFlattenedTriangles()
{
   LOG_PROFILE();

   // Delete any triangles that have two or more vertex indices that point
   // at the same vertex (could happen as a result of welding vertices
   
   if ( m_animationMode )
   {
      return;
   }

   int count = m_triangles.size();
   for ( int t = count-1; t >= 0; t-- )
   {
      if ( m_triangles[t]->m_vertexIndices[0] == m_triangles[t]->m_vertexIndices[1]
        || m_triangles[t]->m_vertexIndices[0] == m_triangles[t]->m_vertexIndices[2]
        || m_triangles[t]->m_vertexIndices[1] == m_triangles[t]->m_vertexIndices[2] )
      {
         deleteTriangle( t );
      }
   }
}

void Model::deleteSelected()
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      m_changeBits |= MoveGeometry;

      if ( m_animationMode == ANIMMODE_SKELETAL && m_currentAnim < m_skelAnims.size() && m_currentFrame < m_skelAnims[ m_currentAnim ]->m_frameCount )
      {
         for ( int j = m_joints.size() - 1; j >= 0; j-- )
         {
            if ( m_joints[j]->m_selected )
            {
               deleteSkelAnimKeyframe( m_currentAnim, m_currentFrame, j, true );
               deleteSkelAnimKeyframe( m_currentAnim, m_currentFrame, j, false );
            }
         }
      }
      return;
   }

   m_changeBits |= AddGeometry;

   for ( int v = m_vertices.size() - 1; v >= 0; v-- )
   {
      m_vertices[v]->m_marked = false;
   }

   for ( int t = m_triangles.size() - 1; t >= 0; t-- )
   {
      if ( m_triangles[t]->m_selected )
      {
         m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_marked = true;
         m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_marked = true;
         m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_marked = true;
         deleteTriangle( t );
      }
   }

   for ( int t = m_triangles.size() - 1; t >= 0; t-- )
   {
      if ( m_triangles[t]->m_visible )
      {
         for ( int v = 0; v < 3; v++ )
         {
            if ( 
                  m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_selected 
                  && !m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_marked )
            {
               deleteTriangle( t );
               break;
            }
         }
      }
   }

   for ( int v = m_vertices.size() - 1; v >= 0; v-- )
   {
      if ( m_vertices[v]->m_selected && !m_vertices[v]->m_marked )
      {
         deleteVertex( v );
      }
   }

   deleteOrphanedVertices();
   bool doJointSetup = false;

   for ( int j = m_joints.size() - 1; j >= 0; j-- )
   {
      if ( m_joints[j]->m_selected )
      {
         deleteBoneJoint( j );
         doJointSetup = true;
      }
   }

   for ( int p = m_points.size() - 1; p >= 0; p-- )
   {
      if ( m_points[p]->m_selected )
      {
         deletePoint( p );
      }
   }

   for ( int r = m_projections.size() - 1; r >= 0; r-- )
   {
      if ( m_projections[r]->m_selected )
      {
         deleteProjection( r );
      }
   }

   // Some selected vertices may not be deleted if their parent
   // triangle was deleted
   unselectAll();
}

bool Model::movePosition( const Position & pos, double x, double y, double z )
{
   switch ( pos.type )
   {
      case PT_Vertex:
         return moveVertex( pos.index, x, y, z );

      case PT_Joint:
         return moveBoneJoint( pos.index, x, y, z );

      case PT_Point:
         return movePoint( pos.index, x, y, z );

      case PT_Projection:
         return moveProjection( pos.index, x, y, z );

      default:
         log_error( "do not know how to move position of type %d\n", pos.type );
         break;
   }
   return false;
}

bool Model::moveVertex( unsigned v, double x, double y, double z )
{
   switch ( m_animationMode )
   {
      case ANIMMODE_NONE:
         if ( v < m_vertices.size() )
         {
            double old[3];

            old[0] = m_vertices[v]->m_coord[0];
            old[1] = m_vertices[v]->m_coord[1];
            old[2] = m_vertices[v]->m_coord[2];

            m_vertices[v]->m_coord[0] = x;
            m_vertices[v]->m_coord[1] = y;
            m_vertices[v]->m_coord[2] = z;

            invalidateNormals();

            MU_MovePrimitive * undo = new MU_MovePrimitive;
            undo->addMovePrimitive( MU_MovePrimitive::MT_Vertex, v, x, y, z,
                  old[0], old[1], old[2] );
            sendUndo( undo, true );

            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( v < m_vertices.size() && m_currentAnim < m_frameAnims.size() )
         {
            setFrameAnimVertexCoords( m_currentAnim, m_currentFrame, v,
                  x, y, z );

            return true;
         }
         break;
      default:
         break;
   }
   return false;
}

bool Model::moveBoneJoint( unsigned j, double x, double y, double z )
{
   if ( m_animationMode == ANIMMODE_NONE && j < m_joints.size() )
   {
      MU_MovePrimitive * undo = new MU_MovePrimitive;
      undo->addMovePrimitive( MU_MovePrimitive::MT_Joint, j, x, y, z,
            m_joints[j]->m_absolute.get(3, 0),
            m_joints[j]->m_absolute.get(3, 1),
            m_joints[j]->m_absolute.get(3, 2) );
      sendUndo( undo, true );

      bool rval = relocateBoneJoint( j, x, y, z );

      setupJoints();

      return rval;
   }
   else
   {
      return false;
   }
}

bool Model::movePoint( unsigned p, double x, double y, double z )
{
   if ( m_animationMode == ANIMMODE_NONE && p < m_points.size() )
   {
      MU_MovePrimitive * undo = new MU_MovePrimitive;
      undo->addMovePrimitive( MU_MovePrimitive::MT_Point, p, x, y, z,
            m_points[p]->m_trans[0],
            m_points[p]->m_trans[1],
            m_points[p]->m_trans[2] );
      sendUndo( undo, true );

      m_points[p]->m_trans[0] = x;
      m_points[p]->m_trans[1] = y;
      m_points[p]->m_trans[2] = z;

      return true;
   }
   else if ( m_animationMode == ANIMMODE_FRAME )
   {
      setFrameAnimPointCoords( m_currentAnim, m_currentFrame, 
            p, x, y, z );

      return true;
   }
   else
   {
      log_debug("point %d does not exist\n", p );
      return false;
   }
}

bool Model::relocateBoneJoint( unsigned j, double x, double y, double z )
{
   if ( j < m_joints.size() )
   {
      double old[3];
      double diff[3];
      double tran[3];

      old[0] = m_joints[j]->m_absolute.get(3, 0);
      old[1] = m_joints[j]->m_absolute.get(3, 1);
      old[2] = m_joints[j]->m_absolute.get(3, 2);

      tran[0] = diff[0] = (x - old[0]);
      tran[1] = diff[1] = (y - old[1]);
      tran[2] = diff[2] = (z - old[2]);

      if ( m_joints[j]->m_parent >= 0 )
      {
         m_joints[ m_joints[j]->m_parent ]->m_absolute.inverseRotateVector( tran );
      }

      m_joints[j]->m_localTranslation[0] += tran[0];
      m_joints[j]->m_localTranslation[1] += tran[1];
      m_joints[j]->m_localTranslation[2] += tran[2];

      for ( unsigned t = 0; t < m_joints.size(); t++ )
      {
         if ( m_joints[t]->m_parent == (signed) j )
         {
            tran[0] = diff[0];
            tran[1] = diff[1];
            tran[2] = diff[2];

            m_joints[ m_joints[t]->m_parent ]->m_absolute.inverseRotateVector( tran );

            m_joints[t]->m_localTranslation[0] -= tran[0];
            m_joints[t]->m_localTranslation[1] -= tran[1];
            m_joints[t]->m_localTranslation[2] -= tran[2];
         }
      }

      m_validJoints = false;

      return true;
   }
   else
   {
      return false;
   }
}

void Model::beginHideDifference()
{
   LOG_PROFILE();

   if ( m_undoEnabled )
   {
      unsigned t;
      for ( t = 0; t < m_vertices.size(); t++ )
      {
         m_vertices[t]->m_marked = m_vertices[t]->m_visible;
      }
      for ( t = 0; t < m_triangles.size(); t++ )
      {
         m_triangles[t]->m_marked = m_triangles[t]->m_visible;
      }
   }
}

void Model::endHideDifference()
{
   LOG_PROFILE();

   if ( m_undoEnabled )
   {
      switch ( m_selectionMode )
      {
         case SelectVertices:
            {
               MU_Hide * undo = new MU_Hide( SelectVertices );
               for ( unsigned t = 0; t < m_vertices.size(); t++ )
               {
                  if ( m_vertices[t]->m_visible != m_vertices[t]->m_marked )
                  {
                     undo->setHideDifference( t, m_vertices[t]->m_visible );
                  }
               }
               sendUndo( undo );
            }
            break;
         case SelectTriangles:
         case SelectGroups:
            {
               MU_Hide * undo = new MU_Hide( SelectTriangles );
               for ( unsigned t = 0; t < m_triangles.size(); t++ )
               {
                  if ( m_triangles[t]->m_visible != m_triangles[t]->m_marked )
                  {
                     undo->setHideDifference( t, m_triangles[t]->m_visible );
                  }
               }
               sendUndo( undo );
            }
            break;
         default:
            break;
      }
   }
}

void Model::translateSelected( const Matrix & m )
{
   LOG_PROFILE();

   list<unsigned> newJointList;

   m_changeBits |= MoveGeometry;

   if ( m_animationMode )
   {
      if ( m_animationMode == ANIMMODE_SKELETAL )
      {
         for ( unsigned j = 0; j < m_joints.size(); j++ )
         {
            if ( m_joints[j]->m_selected && !parentJointSelected(j) )
            {
               Matrix cur = m_joints[j]->m_final;
               Matrix absInv = m_joints[j]->m_absolute.getInverse();

               cur = cur * m;

               if ( m_joints[j]->m_parent >= 0 )
               {
                  Joint * parent = m_joints[ m_joints[j]->m_parent ];
                  absInv = m_joints[j]->m_relative * parent->m_final;
                  absInv = absInv.getInverse();
               }

               cur = cur * absInv;

               double coords[3] = { 0,0,0 };
               cur.getTranslation( coords );

               setSkelAnimKeyframe( m_currentAnim, m_currentFrame, j, false, coords[0], coords[1], coords[2] );
            }

         }

         setCurrentAnimationFrame( m_currentFrame );

         // setSkelAnimKeyframe handles undo
      }
      else
      {
         if ( m_currentAnim < m_frameAnims.size() && m_currentFrame < m_frameAnims[ m_currentAnim ]->m_frameData.size() )
         {
            vector<FrameAnimVertex *> & vertices = (*m_frameAnims[ m_currentAnim ]->m_frameData[ m_currentFrame ]->m_frameVertices);
            for ( unsigned v = 0; v < vertices.size(); v++ )
            {
               if ( m_vertices[ v ]->m_selected )
               {
                  double coord[3];
                  getFrameAnimVertexCoords( m_currentAnim, m_currentFrame, v,
                        coord[0], coord[1], coord[2] );

                  Vector vec( vertices[v]->m_coord );

                  vec.translate( m );
                  const double * val = vec.getVector();

                  setFrameAnimVertexCoords( m_currentAnim, m_currentFrame, v,
                        val[0], val[1], val[2] );
               }
            }

            for ( unsigned p = 0; p < m_points.size(); p++ )
            {
               if ( m_points[p]->m_selected )
               {
                  double coord[3];

                  getFrameAnimPointCoords( m_currentAnim, m_currentFrame, p,
                        coord[0], coord[1], coord[2] );

                  Vector vec( coord );
                  vec.translate( m );
                  const double * val = vec.getVector();

                  setFrameAnimPointCoords( m_currentAnim, m_currentFrame, p,
                        val[0], val[1], val[2] );
               }
            }

         }
      }
   }
   else
   {
      bool vertices = false;
      bool joints   = false;

      for ( unsigned v = 0; v < m_vertices.size(); v++ )
      {
         if ( m_vertices[ v ]->m_selected )
         {
            Vector vec( m_vertices[v]->m_coord );

            vec.translate( m );
            const double * val = vec.getVector();

            m_vertices[v]->m_coord[0] = val[0];
            m_vertices[v]->m_coord[1] = val[1];
            m_vertices[v]->m_coord[2] = val[2];

            vertices = true;
         }
      }

      for ( unsigned j = 0; j < m_joints.size(); j++ )
      {
         if ( m_joints[j]->m_selected )
         {
            double tran[3];
            double diff[3];

            tran[0] = diff[0] = m.get(3,0);
            tran[1] = diff[1] = m.get(3,1);
            tran[2] = diff[2] = m.get(3,2);

            if ( m_joints[j]->m_parent >= 0 )
            {
               m_joints[ m_joints[j]->m_parent ]->m_absolute.inverseRotateVector( tran );
            }

            m_joints[j]->m_localTranslation[0] += tran[0];
            m_joints[j]->m_localTranslation[1] += tran[1];
            m_joints[j]->m_localTranslation[2] += tran[2];

            for ( unsigned t = 0; t < m_joints.size(); t++ )
            {
               if ( m_joints[t]->m_parent == (signed) j )
               {
                  tran[0] = diff[0];
                  tran[1] = diff[1];
                  tran[2] = diff[2];

                  m_joints[ m_joints[t]->m_parent ]->m_absolute.inverseRotateVector( tran );

                  m_joints[t]->m_localTranslation[0] -= tran[0];
                  m_joints[t]->m_localTranslation[1] -= tran[1];
                  m_joints[t]->m_localTranslation[2] -= tran[2];
               }
            }

            joints = true;
         }
      }

      for ( unsigned p = 0; p < m_points.size(); p++ )
      {
         if ( m_points[p]->m_selected )
         {
            m_points[p]->m_trans[0] += m.get( 3, 0 );
            m_points[p]->m_trans[1] += m.get( 3, 1 );
            m_points[p]->m_trans[2] += m.get( 3, 2 );

            joints = true;
         }
      }

      for ( unsigned p = 0; p < m_projections.size(); p++ )
      {
         if ( m_projections[p]->m_selected )
         {
            m_projections[p]->m_pos[0] += m.get( 3, 0 );
            m_projections[p]->m_pos[1] += m.get( 3, 1 );
            m_projections[p]->m_pos[2] += m.get( 3, 2 );

            applyProjection( p );
         }
      }

      if ( vertices )
      {
         invalidateNormals();
      }
      if ( joints )
      {
         setupJoints();
      }

      MU_TranslateSelected * undo = new MU_TranslateSelected();
      undo->setMatrix( m );
      sendUndo( undo, true );
   }
}

void Model::rotateSelected( const Matrix & m, double * point )
{
   LOG_PROFILE();

   m_changeBits |= MoveGeometry;

   if ( m_animationMode )
   {
      if ( m_animationMode == ANIMMODE_SKELETAL )
      {
         unsigned joint = 0;
         for ( joint = 0; joint < m_joints.size(); joint++ )
         {
            if ( m_joints[ joint ]->m_selected )
            {
               Matrix cur = m_joints[joint]->m_final;
               Matrix absInv = m_joints[joint]->m_absolute.getInverse();

               cur = cur * m;

               if ( m_joints[joint]->m_parent >= 0 )
               {
                  Joint * parent = m_joints[ m_joints[joint]->m_parent ];
                  absInv = m_joints[joint]->m_relative * parent->m_final;
                  absInv = absInv.getInverse();
               }

               cur = cur * absInv;

               double rot[3] = { 0,0,0 };
               cur.getRotation( rot );

               setSkelAnimKeyframe( m_currentAnim, m_currentFrame, joint, true, rot[0], rot[1], rot[2] );
               setCurrentAnimationFrame( m_currentFrame );

               // setSkelAnimKeyframe handles undo

               // TODO: should I really allow multiple joints here?
               //break;
            }
         }
      }
      else if ( m_animationMode == ANIMMODE_FRAME )
      {
         for ( unsigned v = 0; v < m_vertices.size(); v++ )
         {
            if ( m_vertices[ v ]->m_selected )
            {
               double coord[3];
               getFrameAnimVertexCoords( m_currentAnim, m_currentFrame, v,
                     coord[0], coord[1], coord[2] );

               coord[0] -= point[0];
               coord[1] -= point[1];
               coord[2] -= point[2];

               Vector vec( coord );

               vec.translate( m );

               coord[0] = vec[0];
               coord[1] = vec[1];
               coord[2] = vec[2];

               coord[0] += point[0];
               coord[1] += point[1];
               coord[2] += point[2];

               setFrameAnimVertexCoords( m_currentAnim, m_currentFrame, v,
                     coord[0], coord[1], coord[2] );
            }
         }

         for ( unsigned p = 0; p < m_points.size(); p++ )
         {
            if ( m_points[p]->m_selected )
            {
               double coord[3];
               double rot[3];

               getFrameAnimPointCoords( m_currentAnim, m_currentFrame, p,
                     coord[0], coord[1], coord[2] );
               getFrameAnimPointRotation( m_currentAnim, m_currentFrame, p,
                     rot[0], rot[1], rot[2] );

               /*
               coord[0] -= point[0];
               coord[1] -= point[1];
               coord[2] -= point[2];
               */
               Matrix pm;
               pm.setTranslation( 
                     coord[0] - point[0],
                     coord[1] - point[1],
                     coord[2] - point[2] );
               pm.setRotation( rot );

               pm = pm * m;

               pm.getTranslation( coord );
               pm.getRotation( rot );

               coord[0] += point[0];
               coord[1] += point[1];
               coord[2] += point[2];

               setFrameAnimPointCoords( m_currentAnim, m_currentFrame, p,
                     coord[0], coord[1], coord[2] );
               setFrameAnimPointRotation( m_currentAnim, m_currentFrame, p,
                     rot[0], rot[1], rot[2] );
            }
         }
      }
   }
   else
   {
      Matrix inv = m.getInverse();

      for ( unsigned v = 0; v < m_vertices.size(); v++ )
      {
         if ( m_vertices[ v ]->m_selected )
         {
            m_vertices[v]->m_coord[0] -= point[0];
            m_vertices[v]->m_coord[1] -= point[1];
            m_vertices[v]->m_coord[2] -= point[2];

            Vector vec( m_vertices[v]->m_coord );

            vec.translate( m );
            const double * val = vec.getVector();

            m_vertices[v]->m_coord[0] = val[0];
            m_vertices[v]->m_coord[1] = val[1];
            m_vertices[v]->m_coord[2] = val[2];

            m_vertices[v]->m_coord[0] += point[0];
            m_vertices[v]->m_coord[1] += point[1];
            m_vertices[v]->m_coord[2] += point[2];

         }
      }

      for ( unsigned j = 0; j < m_joints.size(); j++ )
      {
         // NOTE: This code assumes that if a bone joint is rotated,
         // all children are rotated with it. That may not be what
         // the user expects. To prevent this I would have to find
         // unselected joints whose direct parent was selected
         // and invert the operation on those joints.
         if ( m_joints[j]->m_selected && !parentJointSelected(j) )
         {
            Joint * joint = m_joints[j];

            Matrix trans;
            trans.setTranslation( point );

            joint->m_absolute = joint->m_absolute * trans.getInverse();
            joint->m_absolute = joint->m_absolute * m;
            joint->m_absolute = joint->m_absolute * trans;

            joint->m_absolute.getTranslation( joint->m_localTranslation );

            if ( joint->m_parent >= 0 )
            {
               Matrix pinv = m_joints[ joint->m_parent ]->m_absolute.getInverse();

               Vector v( joint->m_localTranslation );
               v = v * pinv;
               joint->m_localTranslation[0] = v.get(0);
               joint->m_localTranslation[1] = v.get(1);
               joint->m_localTranslation[2] = v.get(2);
               //pinv.apply3( joint->m_localTranslation );
               Matrix mr = joint->m_absolute * pinv;
               mr.getRotation( joint->m_localRotation );
            }
            else
            {
               joint->m_absolute.getRotation( joint->m_localRotation );
            }
         }
      }

      for ( unsigned p = 0; p < m_points.size(); p++ )
      {
         if ( m_points[p]->m_selected )
         {
            Point * pnt = m_points[p];

            Matrix pm;
            pm.setTranslation( 
                  pnt->m_trans[0] - point[0],
                  pnt->m_trans[1] - point[1],
                  pnt->m_trans[2] - point[2] );
            pm.setRotation( 
                  pnt->m_rot );

            pm = pm * m;

            pm.getTranslation( pnt->m_trans );
            pm.getRotation( pnt->m_rot );

            pnt->m_trans[0] += point[0];
            pnt->m_trans[1] += point[1];
            pnt->m_trans[2] += point[2];
         }
      }

      for ( unsigned r = 0; r < m_projections.size(); r++ )
      {
         if ( m_projections[r]->m_selected )
         {
            TextureProjection * proj = m_projections[r];

            double vec[4];

            vec[0] = proj->m_pos[0] - point[0];
            vec[1] = proj->m_pos[1] - point[1];
            vec[2] = proj->m_pos[2] - point[2];
            vec[3] = 1.0;

            m.apply( vec );
            proj->m_pos[0] = vec[0] + point[0];
            proj->m_pos[1] = vec[1] + point[1];
            proj->m_pos[2] = vec[2] + point[2];

            m.apply3( proj->m_upVec );
            m.apply3( proj->m_seamVec );

            applyProjection( r );
         }
      }

      setupJoints();

      invalidateNormals();

      MU_RotateSelected * undo = new MU_RotateSelected();
      undo->setMatrixPoint( m, point );
      sendUndo( undo, true );
   }
}

void Model::applyMatrix( const Matrix & m, OperationScopeE scope, bool animations, bool undoable )
{
   LOG_PROFILE();

   m_changeBits |= MoveGeometry;

   bool global = (scope == OS_Global );

   unsigned vcount = m_vertices.size();
   unsigned bcount = m_joints.size();
   unsigned pcount = m_points.size();
   unsigned rcount = m_projections.size();
   unsigned facount = m_frameAnims.size();

   unsigned v;
   unsigned b;
   unsigned p;
   unsigned r;
   unsigned a;
   unsigned f;

   for ( v = 0; v < vcount; v++ )
   {
      if ( global || m_vertices[ v ]->m_selected )
      {
         m.apply3x( m_vertices[v]->m_coord );
      }
   }

   Matrix * matArray = new Matrix[ bcount ];
   for ( b = 0; b < bcount; b++ )
   {
      matArray[b].loadIdentity();
      matArray[b].setRotation( m_joints[b]->m_localRotation );
      matArray[b].setTranslation( m_joints[b]->m_localTranslation );

      Matrix inv;
      int p = m_joints[b]->m_parent;
      if ( p >= 0 && (global || parentJointSelected(p)) )
      {
         // undo rotation and translation (keep scale)
         matArray[b] = matArray[b] * matArray[p];
         inv = matArray[p];
         inv.normalizeRotation();
         inv = inv.getInverse();
      }
      else
      {
         matArray[b] = matArray[b] * m;
      }
      Matrix rel = matArray[b] * inv;

      if ( global || parentJointSelected(b) )
      {
         rel.normalizeRotation();
         rel.getTranslation( m_joints[b]->m_localTranslation );
         rel.getRotation( m_joints[b]->m_localRotation );
      }
   }

   delete[] matArray;

   for ( p = 0; p < pcount; p++ )
   {
      if ( global || m_points[ p ]->m_selected )
      {
         Matrix pmat;
         pmat.setRotation( m_points[p]->m_rot );
         pmat.setTranslation( m_points[p]->m_trans );

         pmat = pmat * m;

         pmat.normalizeRotation();
         pmat.getRotation( m_points[p]->m_rot );
         pmat.getTranslation( m_points[p]->m_trans );
      }
   }

   for ( r = 0; r < rcount; r++ )
   {
      if ( global || m_projections[ r ]->m_selected )
      {
         m.apply3x( m_projections[ r ]->m_pos );
         m.apply3( m_projections[ r ]->m_upVec );
         m.apply3( m_projections[ r ]->m_seamVec );
      }
   }

   for ( a = 0; a < facount; a++ )
   {
      unsigned fcount = m_frameAnims[a]->m_frameData.size();

      for ( f = 0; f < fcount; f++ )
      {
         for ( v = 0; v < vcount; v++ )
         {
            if ( global || m_vertices[ v ]->m_selected )
            {
               FrameAnimVertex * fav = (*m_frameAnims[a]->m_frameData[f]->m_frameVertices)[v];
               m.apply3x( fav->m_coord );
            }
         }

         for ( p = 0; p < pcount; p++ )
         {
            if ( global || m_points[ p ]->m_selected )
            {
               FrameAnimPoint * fap = (*m_frameAnims[a]->m_frameData[f]->m_framePoints)[p];

               Matrix pmat;
               pmat.setRotation( fap->m_rot );
               pmat.setTranslation( fap->m_trans );

               pmat = pmat * m;

               pmat.getRotation( fap->m_rot );
               pmat.getTranslation( fap->m_trans );
            }
         }
      }
   }

   // Skeletal animations are handled by bone joint

   invalidateNormals();
   setupJoints();

   if ( undoable )
   {
      MU_ApplyMatrix * undo = new MU_ApplyMatrix();
      undo->setMatrix( m, scope, animations );
      sendUndo( undo, true );
   }
   else
   {
      clearUndo();
   }
}

void Model::subdivideSelectedTriangles()
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return;
   }

   sorted_list<SplitEdgesT> seList;
   vector<unsigned> verts;

   MU_SubdivideSelected * undo = new MU_SubdivideSelected();
   sendUndo( undo );

   unsigned vertexStart = m_vertices.size();
   unsigned tlen = m_triangles.size();

   for ( unsigned t = 0; t < tlen; t++ )
   {
      if ( m_triangles[t]->m_selected )
      {
         for ( unsigned v = 0; v < 3; v++ )
         {
            unsigned a = m_triangles[t]->m_vertexIndices[ v ];
            unsigned b = m_triangles[t]->m_vertexIndices[ (v+1) % 3 ];

            unsigned index;

            if ( b < a )
            {
               unsigned c = a;
               a = b;
               b = c;
            }

            SplitEdgesT e;
            e.a = a;
            e.b = b;

            int vNew = -1;
            if ( seList.find_sorted( e, index ) )
            {
               vNew = seList[index].vNew;
            }
            else
            {
               double pa[3];
               double pb[3];

               getVertexCoords( a, pa );
               getVertexCoords( b, pb );

               // TODO this should really use addVertex()
               Vertex * vertex = Vertex::get();

               vertex->m_coord[0] = (pa[0] + pb[0]) / 2;
               vertex->m_coord[1] = (pa[1] + pb[1]) / 2;
               vertex->m_coord[2] = (pa[2] + pb[2]) / 2;

               vertex->m_selected = true;

               vNew = m_vertices.size();
               m_vertices.push_back( vertex );

               e.vNew = vNew;

               seList.insert_sorted( e );
            }

            verts.push_back( vNew );
         }

         unsigned btri = m_triangles.size();

         Triangle * tptr = Triangle::get();
         tptr->m_selected = true;
         tptr->m_vertexIndices[0] = m_triangles[t]->m_vertexIndices[1];
         tptr->m_vertexIndices[1] = verts[1];
         tptr->m_vertexIndices[2] = verts[0];
         m_triangles.push_back( tptr );

         unsigned ctri = m_triangles.size();

         tptr = Triangle::get();
         tptr->m_selected = true;
         tptr->m_vertexIndices[0] = m_triangles[t]->m_vertexIndices[2];
         tptr->m_vertexIndices[1] = verts[2];
         tptr->m_vertexIndices[2] = verts[1];
         m_triangles.push_back( tptr );

         unsigned dtri = m_triangles.size();

         tptr = Triangle::get();
         tptr->m_selected = true;
         tptr->m_vertexIndices[0] = verts[0];
         tptr->m_vertexIndices[1] = verts[1];
         tptr->m_vertexIndices[2] = verts[2];
         m_triangles.push_back( tptr );

         m_triangles[t]->m_vertexIndices[1] = verts[0];
         m_triangles[t]->m_vertexIndices[2] = verts[2];

         MU_SubdivideTriangle * undo = new MU_SubdivideTriangle();
         undo->subdivide( t, btri, ctri, dtri );
         sendUndo( undo );

         verts.clear();
      }
   }

   invalidateNormals();

   MU_SubdivideTriangle * vundo = new MU_SubdivideTriangle();
   for ( unsigned i = vertexStart; i < m_vertices.size(); i++ )
   {
      vundo->addVertex( i );
   }
   sendUndo( vundo );
}

void Model::unsubdivideTriangles( unsigned t1, unsigned t2, unsigned t3, unsigned t4 )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return;
   }


   m_triangles[t1]->m_vertexIndices[1] = m_triangles[t2]->m_vertexIndices[0];
   m_triangles[t1]->m_vertexIndices[2] = m_triangles[t3]->m_vertexIndices[0];

   Triangle * doomed;
   doomed = m_triangles[ t4 ];
   removeTriangle( t4 );
   doomed->release();

   doomed = m_triangles[ t3 ];
   removeTriangle( t3 );
   doomed->release();

   doomed = m_triangles[ t2 ];
   removeTriangle( t2 );
   doomed->release();

   invalidateNormals();
}

bool Model::setPointName( unsigned point, const char * name )
{
   if ( point < m_points.size() && name && name[0] )
   {
      MU_SetPointName * undo = new MU_SetPointName();
      undo->setName( point, name, m_points[point]->m_name.c_str() );
      sendUndo( undo );

      m_points[point]->m_name = name;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setPointType( unsigned point, int type )
{
   if ( point < m_points.size() )
   {
      /* // TODO: undo (if this is ever actually used)
      MU_SetPointName * undo = new MU_SetPointName();
      undo->setName( point, name, m_points[point]->m_name.c_str() );
      sendUndo( undo );
      */

      m_points[point]->m_type = type;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setBoneJointName( unsigned joint, const char * name )
{
   if ( joint < m_joints.size() && name && name[0] )
   {
      MU_SetJointName * undo = new MU_SetJointName();
      undo->setName( joint, name, m_joints[joint]->m_name.c_str() );
      sendUndo( undo );

      m_joints[joint]->m_name = name;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setBoneJointParent( unsigned joint, int parent )
{
   if ( joint < m_joints.size() && parent >= -1 )
   {
      MU_SetJointParent * undo = new MU_SetJointParent();
      undo->setJointParent( joint, parent, m_joints[joint]->m_parent );
      sendUndo( undo );

      m_joints[joint]->m_parent = parent;

      setupJoints();
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setBoneJointRotation( unsigned j, const double * rot )
{
   if ( j < m_joints.size() && rot )
   {
      MU_SetJointRotation * undo = new MU_SetJointRotation();
      undo->setJointRotation( j, rot, m_joints[j]->m_localRotation );
      sendUndo(undo);

      for ( unsigned i = 0; i < 3; i++ )
      {
         m_joints[j]->m_localRotation[i] = rot[i];
      }

      setupJoints();
      return true;
   }
   return false;
}

bool Model::setBoneJointTranslation( unsigned j, const double * trans )
{
   if ( j < m_joints.size() && trans )
   {
      MU_SetJointTranslation * undo = new MU_SetJointTranslation();
      undo->setJointTranslation( j, trans, m_joints[j]->m_localTranslation );
      sendUndo(undo);

      for ( unsigned i = 0; i < 3; i++ )
      {
         m_joints[j]->m_localTranslation[i] = trans[i];
      }

      setupJoints();
      return true;
   }
   return false;
}

void Model::operationComplete( const char * opname )
{
   if ( m_selecting )
   {
      endSelectionDifference();
   }

   m_undoMgr->operationComplete( opname );
   updateObservers();
}

void Model::forceAddOrDelete( bool o )
{
   if ( !o && m_forceAddOrDelete )
      clearUndo();

   m_forceAddOrDelete = o;
}

bool Model::setUndoEnabled( bool o )
{
   bool old = m_undoEnabled;
   m_undoEnabled = o;
   return old;
}

void Model::clearUndo()
{
   m_undoMgr->clear();
}

bool Model::canUndo() const
{
   //if ( m_animationMode )
   //{
   //   return m_animUndoMgr->canUndo();
   //}
   //else
   {
      return m_undoMgr->canUndo();
   }
}

bool Model::canRedo() const
{
   //if ( m_animationMode )
   //{
   //   return m_animUndoMgr->canRedo();
   //}
   //else
   {
      return m_undoMgr->canRedo();
   }
}

const char * Model::getUndoOpName() const
{
   //if ( m_animationMode )
   //{
   //   return m_animUndoMgr->getUndoOpName();
   //}
   //else
   {
      return m_undoMgr->getUndoOpName();
   }
}

const char * Model::getRedoOpName() const
{
   //if ( m_animationMode )
   //{
   //   return m_animUndoMgr->getRedoOpName();
   //}
   //else
   {
      return m_undoMgr->getRedoOpName();
   }
}

void Model::undo()
{
   LOG_PROFILE();

   //UndoList * list = (m_animationMode) ? m_animUndoMgr->undo() : m_undoMgr->undo();
   UndoList * list = m_undoMgr->undo();

   if ( list )
   {
      log_debug( "got atomic undo list\n" );
      setUndoEnabled( false );

      // process back to front
      UndoList::reverse_iterator it;

      for ( it = list->rbegin(); it != list->rend(); it++ )
      {
         ModelUndo * undo = static_cast<ModelUndo *>( (*it) );
         undo->undo( this );
      }

      if ( !m_validJoints )
      {
         setupJoints();
      }

      setUndoEnabled( true );

      updateObservers();
   }

   m_selecting = false;
}

void Model::redo()
{
   LOG_PROFILE();

   //UndoList * list = (m_animationMode) ? m_animUndoMgr->redo() : m_undoMgr->redo();
   UndoList * list = m_undoMgr->redo();

   if ( list )
   {
      log_debug( "got atomic redo list\n" );
      setUndoEnabled( false );

      // process front to back
      UndoList::iterator it;
      for ( it = list->begin(); it != list->end(); it++ )
      {
         ModelUndo * undo = static_cast<ModelUndo *>( (*it) );
         undo->redo( this );
      }

      if ( !m_validJoints )
      {
         setupJoints();
      }

      setUndoEnabled( true );

      updateObservers();
   }

   m_selecting = false;
}

void Model::undoCurrent()
{
   LOG_PROFILE();

   //UndoList * list = (m_animationMode) ? m_animUndoMgr->undoCurrent() : m_undoMgr->undoCurrent();
   UndoList * list = m_undoMgr->undoCurrent();

   if ( list )
   {
      log_debug( "got atomic undo list\n" );
      setUndoEnabled( false );

      // process back to front
      UndoList::reverse_iterator it;
      for ( it = list->rbegin(); it != list->rend(); it++ )
      {
         ModelUndo * undo = static_cast<ModelUndo *>( (*it) );
         undo->undo( this );
      }

      if ( !m_validJoints )
      {
         setupJoints();
      }

      setUndoEnabled( true );

      updateObservers();
   }

   m_selecting = false;
}

bool Model::hideVertex( unsigned v )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }


   if ( v < m_vertices.size() )
   {
      m_vertices[ v ]->m_visible = false;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::hideTriangle( unsigned t )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   if ( t < m_triangles.size() )
   {
      m_triangles[ t ]->m_visible = false;

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::hideJoint( unsigned j )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   if ( j < m_joints.size() )
   {
      m_joints[ j ]->m_visible = false;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::hidePoint( unsigned p )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   if ( p < m_points.size() )
   {
      m_points[ p ]->m_visible = false;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unhideVertex( unsigned v )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   if ( v < m_vertices.size() )
   {
      m_vertices[ v ]->m_visible = true;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unhideTriangle( unsigned t )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   if ( t < m_triangles.size() )
   {
      m_triangles[ t ]->m_visible = true;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unhideJoint( unsigned j )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   if ( j < m_joints.size() )
   {
      m_joints[ j ]->m_visible = true;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::unhidePoint( unsigned p )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   if ( p < m_points.size() )
   {
      m_points[ p ]->m_visible = true;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::hideSelected()
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   unsigned t = 0;
   unsigned v = 0;

   // Need to track whether we are hiding a vertex and any triangles attached to it,
   // or hiding a triangle, and only vertices if they are orphaned
   for ( v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->m_selected )
      {
         m_vertices[v]->m_marked = true;
      }
      else
      {
         m_vertices[v]->m_marked = false;
      }
   }

   // Hide selected triangles
   for ( t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_selected )
      {
         m_triangles[t]->m_visible = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_marked  = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_marked  = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_marked  = false;

         MU_Hide * undo = new MU_Hide( SelectTriangles );
         undo->setHideDifference( t, m_triangles[t]->m_visible );
         sendUndo( undo );
      }
   }

   // Hide triangles with a lone vertex that is selected
   for ( t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_visible &&
           (    m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_marked 
             || m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_marked 
             || m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_marked ) )
      {
         m_triangles[t]->m_visible = false;

         MU_Hide * undo = new MU_Hide( SelectTriangles );
         undo->setHideDifference( t, m_triangles[t]->m_visible );
         sendUndo( undo );
      }
   }

   // Find orphaned vertices
   for ( v = 0; v < m_vertices.size(); v++ )
   {
      m_vertices[v]->m_marked = true;
   }
   for ( t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_visible )
      {
         // Triangle is visible, vertices must be too
         m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_marked = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_marked = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_marked = false;
      }
   }

   // Hide selected vertices
   for ( v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->m_visible && m_vertices[v]->m_marked )
      {
         m_vertices[v]->m_visible = false;

         MU_Hide * undo = new MU_Hide( SelectVertices );
         undo->setHideDifference( v, m_vertices[v]->m_visible );
         sendUndo( undo );
      }
   }

   for ( unsigned j = 0; j < m_joints.size(); j++ )
   {
      if ( m_joints[j]->m_selected )
      {
         m_joints[j]->m_visible = false;

         MU_Hide * undo = new MU_Hide( SelectJoints );
         undo->setHideDifference( j, m_joints[j]->m_visible );
         sendUndo( undo );
      }
   }

   for ( unsigned p = 0; p < m_points.size(); p++ )
   {
      if ( m_points[p]->m_selected )
      {
         m_points[p]->m_visible = false;

         MU_Hide * undo = new MU_Hide( SelectPoints );
         undo->setHideDifference( p, m_points[p]->m_visible );
         sendUndo( undo );
      }
   }

   unselectAll();

   return true;
}

bool Model::hideUnselected()
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   unsigned t = 0;
   unsigned v = 0;

   // Need to track whether we are hiding a vertex and any triangles attached to it,
   // or hiding a triangle, and only vertices if they are orphaned
   // We could be doing both at the same
   for ( v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->m_selected )
      {
         m_vertices[v]->m_marked = false;
      }
      else
      {
         m_vertices[v]->m_marked = true;
      }
   }

   // Hide triangles with any unselected vertices
   for ( t = 0; t < m_triangles.size(); t++ )
   {
      if ( !m_triangles[t]->m_selected )
      {
         log_debug( "triangle %d is unselected, hiding\n", t );
         m_triangles[t]->m_visible = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_marked  = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_marked  = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_marked  = false;

         MU_Hide * undo = new MU_Hide( SelectTriangles );
         undo->setHideDifference( t, m_triangles[t]->m_visible );
         sendUndo( undo );
      }
   }

   // Find orphaned vertices
   for ( v = 0; v < m_vertices.size(); v++ )
   {
      m_vertices[v]->m_marked = true;
   }
   for ( t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_visible )
      {
         // Triangle is visible, vertices must be too
         m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_marked = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_marked = false;
         m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_marked = false;
      }
   }

   // Hide selected vertices
   for ( v = 0; v < m_vertices.size(); v++ )
   {
      if ( m_vertices[v]->m_visible && m_vertices[v]->m_marked )
      {
         log_debug( "vertex %d is visible and marked, hiding\n", v );
         m_vertices[v]->m_visible = false;

         MU_Hide * undo = new MU_Hide( SelectVertices );
         undo->setHideDifference( v, m_vertices[v]->m_visible );
         sendUndo( undo );
      }
   }

   for ( unsigned j = 0; j < m_joints.size(); j++ )
   {
      if ( m_joints[j]->m_visible && !m_joints[j]->m_selected )
      {
         m_joints[j]->m_visible = false;

         MU_Hide * undo = new MU_Hide( SelectJoints );
         undo->setHideDifference( j, m_joints[j]->m_visible );
         sendUndo( undo );
      }
   }

   for ( unsigned p = 0; p < m_points.size(); p++ )
   {
      if ( m_points[p]->m_visible && !m_points[p]->m_selected )
      {
         m_points[p]->m_visible = false;

         MU_Hide * undo = new MU_Hide( SelectPoints );
         undo->setHideDifference( p, m_points[p]->m_visible );
         sendUndo( undo );
      }
   }

   unselectAll();

   return true;
}

bool Model::unhideAll()
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return false;
   }

   for ( unsigned v = 0; v < m_vertices.size(); v++ )
   {
      if ( !m_vertices[v]->m_visible )
      {
         m_vertices[v]->m_visible = true;

         MU_Hide * undo = new MU_Hide( SelectVertices );
         undo->setHideDifference( v, true );
         sendUndo( undo );
      }
   }
   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      if ( !m_triangles[t]->m_visible )
      {
         m_triangles[t]->m_visible = true;

         MU_Hide * undo = new MU_Hide( SelectTriangles );
         undo->setHideDifference( t, true );
         sendUndo( undo );
      }
   }
   for ( unsigned j = 0; j < m_joints.size(); j++ )
   {
      if ( !m_joints[j]->m_visible )
      {
         m_joints[j]->m_visible = true;

         MU_Hide * undo = new MU_Hide( SelectJoints );
         undo->setHideDifference( j, true );
         sendUndo( undo );
      }
   }
   for ( unsigned p = 0; p < m_points.size(); p++ )
   {
      if ( !m_points[p]->m_visible )
      {
         m_points[p]->m_visible = true;

         MU_Hide * undo = new MU_Hide( SelectPoints );
         undo->setHideDifference( p, true );
         sendUndo( undo );
      }
   }

   return true;
}

void Model::hideTrianglesFromVertices()
{
   LOG_PROFILE();

   // Hide triangles with at least one vertex hidden
   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      if (    !m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_visible
            || !m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_visible
            || !m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_visible
         )
      {
         m_triangles[t]->m_visible = false;
      }
   }
}

void Model::unhideTrianglesFromVertices()
{
   LOG_PROFILE();

   // Hide triangles with at least one vertex hidden
   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      if (    m_vertices[ m_triangles[t]->m_vertexIndices[0] ]->m_visible
            && m_vertices[ m_triangles[t]->m_vertexIndices[1] ]->m_visible
            && m_vertices[ m_triangles[t]->m_vertexIndices[2] ]->m_visible
         )
      {
         m_triangles[t]->m_visible = true;
      }
   }
}

void Model::hideVerticesFromTriangles()
{
   LOG_PROFILE();

   // Hide vertices with all triangles hidden
   unsigned t;

   for ( t = 0; t < m_vertices.size(); t++ )
   {
      m_vertices[t]->m_visible = false;
   }

   for ( t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_visible )
      {
         for ( int v = 0; v < 3; v++ )
         {
            m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_visible = true;
         }
      }
   }
}

void Model::unhideVerticesFromTriangles()
{
   LOG_PROFILE();

   // Unhide vertices with at least one triangle visible
   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_visible )
      {
         for ( int v = 0; v < 3; v++ )
         {
            m_vertices[ m_triangles[t]->m_vertexIndices[v] ]->m_visible = true;
         }
      }
   }
}

bool Model::getBoundingRegion( double *x1, double *y1, double *z1, double *x2, double *y2, double *z2 ) const
{
   if ( x1 && y1 && z1 && x2 && y2 && z2 )
   {
      int visible = 0;
      bool havePoint = false;
      *x1 = *y1 = *z1 = *x2 = *y2 = *z2 = 0.0;

      for ( unsigned v = 0; v < m_vertices.size(); v++ )
      {
            if ( m_vertices[v]->m_visible )
            {
               if ( havePoint )
               {
                  if ( m_vertices[v]->m_coord[0] < *x1 )
                  {
                     *x1 = m_vertices[v]->m_coord[0];
                  }
                  if ( m_vertices[v]->m_coord[0] > *x2 )
                  {
                     *x2 = m_vertices[v]->m_coord[0];
                  }
                  if ( m_vertices[v]->m_coord[1] < *y1 )
                  {
                     *y1 = m_vertices[v]->m_coord[1];
                  }
                  if ( m_vertices[v]->m_coord[1] > *y2 )
                  {
                     *y2 = m_vertices[v]->m_coord[1];
                  }
                  if ( m_vertices[v]->m_coord[2] < *z1 )
                  {
                     *z1 = m_vertices[v]->m_coord[2];
                  }
                  if ( m_vertices[v]->m_coord[2] > *z2 )
                  {
                     *z2 = m_vertices[v]->m_coord[2];
                  }
               }
               else
               {
                  *x1 = *x2 = m_vertices[v]->m_coord[0];
                  *y1 = *y2 = m_vertices[v]->m_coord[1];
                  *z1 = *z2 = m_vertices[v]->m_coord[2];
                  havePoint = true;
               }
               visible++;
            }
      }

      for ( unsigned j = 0; j < m_joints.size(); j++ )
      {
         double coord[3];
         m_joints[j]->m_absolute.getTranslation( coord );

         if ( havePoint )
         {
            if ( coord[0] < *x1 )
            {
               *x1 = coord[0];
            }
            if ( coord[0] > *x2 )
            {
               *x2 = coord[0];
            }
            if ( coord[1] < *y1 )
            {
               *y1 = coord[1];
            }
            if ( coord[1] > *y2 )
            {
               *y2 = coord[1];
            }
            if ( coord[2] < *z1 )
            {
               *z1 = coord[2];
            }
            if ( coord[2] > *z2 )
            {
               *z2 = coord[2];
            }
         }
         else
         {
            *x1 = *x2 = coord[0];
            *y1 = *y2 = coord[1];
            *z1 = *z2 = coord[2];
            havePoint = true;
         }

         visible++;
      }

      for ( unsigned p = 0; p < m_points.size(); p++ )
      {
         double coord[3];
         coord[0] = m_points[p]->m_trans[0];
         coord[1] = m_points[p]->m_trans[1];
         coord[2] = m_points[p]->m_trans[2];

         if ( havePoint )
         {
            if ( coord[0] < *x1 )
            {
               *x1 = coord[0];
            }
            if ( coord[0] > *x2 )
            {
               *x2 = coord[0];
            }
            if ( coord[1] < *y1 )
            {
               *y1 = coord[1];
            }
            if ( coord[1] > *y2 )
            {
               *y2 = coord[1];
            }
            if ( coord[2] < *z1 )
            {
               *z1 = coord[2];
            }
            if ( coord[2] > *z2 )
            {
               *z2 = coord[2];
            }
         }
         else
         {
            *x1 = *x2 = coord[0];
            *y1 = *y2 = coord[1];
            *z1 = *z2 = coord[2];
            havePoint = true;
         }

         visible++;
      }

      for ( unsigned p = 0; p < m_projections.size(); p++ )
      {
         double coord[3];
         double m = mag3(m_projections[p]->m_upVec);

         coord[0] = m_projections[p]->m_pos[0] + m;
         coord[1] = m_projections[p]->m_pos[1] + m;
         coord[2] = m_projections[p]->m_pos[2] + m;

         if ( havePoint )
         {
            if ( coord[0] < *x1 )
            {
               *x1 = coord[0];
            }
            if ( coord[0] > *x2 )
            {
               *x2 = coord[0];
            }
            if ( coord[1] < *y1 )
            {
               *y1 = coord[1];
            }
            if ( coord[1] > *y2 )
            {
               *y2 = coord[1];
            }
            if ( coord[2] < *z1 )
            {
               *z1 = coord[2];
            }
            if ( coord[2] > *z2 )
            {
               *z2 = coord[2];
            }
         }
         else
         {
            *x1 = *x2 = coord[0];
            *y1 = *y2 = coord[1];
            *z1 = *z2 = coord[2];
            havePoint = true;
         }

         coord[0] = m_projections[p]->m_pos[0] - m;
         coord[1] = m_projections[p]->m_pos[1] - m;
         coord[2] = m_projections[p]->m_pos[2] - m;

         if ( coord[0] < *x1 )
         {
            *x1 = coord[0];
         }
         if ( coord[0] > *x2 )
         {
            *x2 = coord[0];
         }
         if ( coord[1] < *y1 )
         {
            *y1 = coord[1];
         }
         if ( coord[1] > *y2 )
         {
            *y2 = coord[1];
         }
         if ( coord[2] < *z1 )
         {
            *z1 = coord[2];
         }
         if ( coord[2] > *z2 )
         {
            *z2 = coord[2];
         }

         visible++;
      }

      return ( visible != 0 ) ? true : false;
   }

   return false;
}

void Model::invertNormals( unsigned triangleNum )
{
   LOG_PROFILE();

   if ( triangleNum < m_triangles.size() )
   {
      int temp = m_triangles[ triangleNum ]->m_vertexIndices[0];
      m_triangles[ triangleNum ]->m_vertexIndices[0] = m_triangles[ triangleNum ]->m_vertexIndices[2];
      m_triangles[ triangleNum ]->m_vertexIndices[2] = temp;

      float texTemp = m_triangles[ triangleNum ]->m_s[0];
      m_triangles[ triangleNum ]->m_s[0] = m_triangles[ triangleNum ]->m_s[2];
      m_triangles[ triangleNum ]->m_s[2] = texTemp;

      texTemp = m_triangles[ triangleNum ]->m_t[0];
      m_triangles[ triangleNum ]->m_t[0] = m_triangles[ triangleNum ]->m_t[2];
      m_triangles[ triangleNum ]->m_t[2] = texTemp;

      invalidateNormals();

      MU_InvertNormal * undo = new MU_InvertNormal();
      undo->addTriangle( triangleNum );
      sendUndo( undo );
   }
}

bool Model::triangleFacesIn( unsigned triangleNum )
{
   if ( ! m_validNormals )
   {
      calculateNormals();
   }

   unsigned int tcount = m_triangles.size();
   if ( triangleNum < tcount )
   {
      int inFront = 0;
      int inBack  = 0;
      int sideInFront  = 0;
      int sideInBack   = 0;

      double p1[3] = { 0, 0, 0 };
      double p2[3] = { 0, 0, 0 };

      Triangle * tri = m_triangles[ triangleNum ];
      for ( int i = 0; i < 3; i++ )
      {
         for ( int j = 0; j < 3; j++ )
         {
            p1[i] += m_vertices[ tri->m_vertexIndices[j] ]->m_coord[i];
         }
         p1[i] /= 3.0;
      }

      float norm[3] = { 0.0f, 0.0f, 0.0f };
      getFlatNormal( triangleNum, norm );

      p2[0] = norm[0] + p1[0];
      p2[1] = norm[1] + p1[1];
      p2[2] = norm[2] + p1[2];

      for ( unsigned int t = 0; t < tcount; t++ )
      {
         double tpoint[3][3];
         double tnorm[3];
         getVertexCoords( m_triangles[t]->m_vertexIndices[0], tpoint[0] );
         getFlatNormal( t, norm );

         tnorm[0] = norm[0];
         tnorm[1] = norm[1];
         tnorm[2] = norm[2];

         if ( t != triangleNum )
         {
            double ipoint[3] = { 0, 0, 0 };
            int val = _findEdgePlaneIntersection( ipoint, p1, p2,
                  tpoint[0], tnorm );

            if ( val != 0 )
            {
               getVertexCoords( m_triangles[t]->m_vertexIndices[1], tpoint[1] );
               getVertexCoords( m_triangles[t]->m_vertexIndices[2], tpoint[2] );

               int inTri = _pointInTriangle( ipoint,
                     tpoint[0], tpoint[1], tpoint[2] );

               if ( inTri >= 0 )
               {
                  if ( val > 0 )
                  {
                     if ( inTri == 0 )
                     {
                        sideInFront++;
                     }
                     else
                     {
                        inFront++;
                     }
                  }
                  else
                  {
                     if ( inTri == 0 )
                     {
                        sideInBack++;
                     }
                     else
                     {
                        inBack++;
                     }
                  }
               }
            }
         }
      }

      if ( (sideInFront & 1) == 0 )
      {
         inFront += sideInFront / 2;
      }
      if ( (sideInBack & 1) == 0 )
      {
         inBack += sideInBack / 2;
      }

      //log_debug( "front = %d    back = %d\n", inFront, inBack );

      return ((inFront & 1) != 0) && ((inBack & 1) == 0);
   }

   return false;
}

void Model::sendUndo( Undo * undo, bool listCombine )
{
   setSaved( false );
   if ( m_undoEnabled )
   {
      //if ( m_animationMode )
      //{
      //   m_animUndoMgr->addUndo( undo );
      //}
      //else
      {
         m_undoMgr->addUndo( undo, listCombine );
      }
   }
   else
   {
      undo->undoRelease();
      undo->release();
   }
}

void Model::displayFrameAnimPrimitiveError()
{
   model_status( this, StatusError, STATUSTIME_LONG, transll(QT_TRANSLATE_NOOP("LowLevel", "Cannot add or delete because you have frame animations.  Try \"Merge...\" instead." )).c_str() );
}

#endif // MM3D_EDIT

int Model::addFormatData( FormatData * fd )
{
   m_changeBits |= AddOther;

   int n = -1;
   if ( fd )
   {
      n = m_formatData.size();
      m_formatData.push_back( fd );
   }

   return n;
}

bool Model::deleteFormatData( unsigned index )
{
   vector< FormatData * >::iterator it;

   for ( it = m_formatData.begin(); it != m_formatData.end(); it++ )
   {
      while ( index && it != m_formatData.end() )
      {
         it++;
         index--;
      }

      if ( it != m_formatData.end() )
      {
         delete (*it);
         m_formatData.erase( it );
         return true;
      }
   }

   return false;
}

unsigned Model::getFormatDataCount() const
{
   return m_formatData.size();
}

Model::FormatData * Model::getFormatData( unsigned index ) const
{
   if ( index < m_formatData.size() )
   {
      return m_formatData[index];
   }
   return NULL;
}

Model::FormatData * Model::getFormatDataByFormat( const char * format, unsigned index ) const
{
   unsigned count = m_formatData.size();
   for ( unsigned n = 0; n < count; n++ )
   {
      FormatData * fd = m_formatData[n];

      if ( strcasecmp( fd->format.c_str(), format ) == 0 && fd->index == index )
      {
         return fd;
      }
   }

   return NULL;
}

int Model::getBoneJointParent( unsigned j ) const
{
   if ( j < m_joints.size() )
   {
      return m_joints[j]->m_parent;
   }
   else
   {
      return -1;
   }
}

const char * Model::getBoneJointName( unsigned joint ) const
{
   if ( joint < m_joints.size() )
   {
      return m_joints[joint]->m_name.c_str();
   }
   else
   {
      return NULL;
   }
}

const char * Model::getPointName( unsigned point ) const
{
   if ( point < m_points.size() )
   {
      return m_points[point]->m_name.c_str();
   }
   else
   {
      return NULL;
   }
}

int Model::getPointByName( const char * name ) const
{
   for ( unsigned int point = 0; point < m_points.size(); point++ )
   {
      if ( strcmp( name, m_points[point]->m_name.c_str() ) == 0 )
      {
         return point;
      }
   }
   return -1;
}

int Model::getPointType( unsigned point ) const
{
   if ( point < m_points.size() )
   {
      return m_points[point]->m_type;
   }
   else
   {
      return 0;
   }
}

int Model::getPointBoneJoint( unsigned p ) const
{
   return getPrimaryPointInfluence( p );
}

bool Model::getPositionCoords( const Position & pos, double *coord ) const
{
   switch ( pos.type )
   {
      case PT_Vertex:
         return getVertexCoords( pos.index, coord );
      case PT_Joint:
         return getBoneJointCoords( pos.index, coord );
      case PT_Point:
         return getPointCoords( pos.index, coord );
      case PT_Projection:
         return getProjectionCoords( pos.index, coord );
      default:
         log_error( "do not know how to get the position for position type %d\n", pos.type );
         break;
   }
   return false;
}

bool Model::getVertexCoords( unsigned vertexNumber, double *coord ) const
{
   switch ( m_animationMode )
   {
      case ANIMMODE_NONE:
         if ( coord && vertexNumber < m_vertices.size() )
         {
            for ( int t = 0; t < 3; t++ )
            {
               coord[t] = m_vertices[ vertexNumber ]->m_coord[t];
            }
            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( coord && vertexNumber < m_vertices.size() )
         {
            getFrameAnimVertexCoords( m_currentAnim, m_currentFrame, vertexNumber,
                  coord[0], coord[1], coord[2] );
            return true;
         }
         break;
      case ANIMMODE_SKELETAL:
         if ( coord )
         {
            if ( vertexNumber < m_vertices.size() )
            {
               Vertex * vertex = m_vertices[vertexNumber];
               for ( int t = 0; t < 3; t++ )
               {
                  coord[t] = vertex->m_drawSource[t];
               }
            }
         }
         break;
      default:
         break;
   }
   return false;
}

bool Model::getVertexCoordsUnanimated( unsigned vertexNumber, double *coord ) const
{
   if ( coord && vertexNumber < m_vertices.size() )
   {
      for ( int t = 0; t < 3; t++ )
      {
         coord[t] = m_vertices[ vertexNumber ]->m_coord[t];
      }
      return true;
   }
   return false;
}

bool Model::getVertexCoords2d( unsigned vertexNumber, ProjectionDirectionE dir, double *coord ) const
{
   if ( coord && vertexNumber >= 0 && (unsigned) vertexNumber < m_vertices.size() )
   {
      Vertex * vert = m_vertices[ vertexNumber ];
      switch ( dir )
      {
         case ViewFront:
            coord[0] =  vert->m_coord[0];
            coord[1] =  vert->m_coord[1];
            break;
         case ViewBack:
            coord[0] = -vert->m_coord[0];
            coord[1] =  vert->m_coord[1];
            break;
         case ViewLeft:
            coord[0] = -vert->m_coord[2];
            coord[1] =  vert->m_coord[1];
            break;
         case ViewRight:
            coord[0] =  vert->m_coord[2];
            coord[1] =  vert->m_coord[1];
            break;
         case ViewTop:
            coord[0] =  vert->m_coord[0];
            coord[1] = -vert->m_coord[2];
            break;
         case ViewBottom:
            coord[0] =  vert->m_coord[0];
            coord[1] =  vert->m_coord[2];
            break;

         // Not an orthogonal view
         default:
            return false;
            break;
      }
      return true;
   }
   else
   {
      return false;
   }
}

int Model::getVertexBoneJoint( unsigned vertexNumber ) const
{
   return getPrimaryVertexInfluence( vertexNumber );
}

bool Model::getPointCoords( unsigned pointNumber, double *coord ) const
{
   if ( coord && pointNumber < m_points.size() )
   {
      int t;
      switch ( m_animationMode )
      {
         case ANIMMODE_SKELETAL:
            {
               for ( t = 0; t < 3; t++ )
               {
                  coord[t] = m_points[ pointNumber ]->m_kfTrans[ t ];
               }
               /*
               Matrix mat;
               Point * point = m_points[ pointNumber ];

               mat.setTranslation( point->m_localTranslation );
               mat.setRotation( point->m_localRotation );

               int j = point->m_boneId;
               if ( j >= 0 )
               {
                  mat = mat * m_joints[j]->m_final;
               }

               for ( t = 0; t < 3; t++ )
               {
                  coord[t] = mat.get(3, t);
               }
               */
            }
            break;
         case ANIMMODE_FRAME:
            getFrameAnimPointCoords( m_currentAnim, m_currentFrame,
                  pointNumber, coord[0], coord[1], coord[2] );
            break;
         case ANIMMODE_NONE:
         default:
            for ( t = 0; t < 3; t++ )
            {
               coord[t] = m_points[ pointNumber ]->m_trans[ t ];
            }
            break;
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getPointOrientation( unsigned pointNumber, double * rot ) const
{
   if ( rot && pointNumber < m_points.size() )
   {
      int t;

      switch ( m_animationMode )
      {
         case ANIMMODE_SKELETAL:
            {
               Matrix mat;
               Point * point = m_points[ pointNumber ];

               mat.setTranslation( point->m_kfTrans );
               mat.setRotation( point->m_kfRot );

               mat.getRotation( rot );
               /*
               Matrix mat;
               Point * point = m_points[ pointNumber ];

               mat.setTranslation( point->m_localTranslation );
               mat.setRotation( point->m_localRotation );

               int j = point->m_boneId;
               if ( j >= 0 )
               {
                  mat = mat * m_joints[j]->m_final;
               }

               mat.getRotation( rot );
               */
            }
            break;
         case ANIMMODE_FRAME:
            getFrameAnimPointCoords( m_currentAnim, m_currentFrame,
                  pointNumber, rot[0], rot[1], rot[2] );
            break;
         case ANIMMODE_NONE:
         default:
            for ( t = 0; t < 3; t++ )
            {
               rot[t] = m_points[ pointNumber ]->m_rot[ t ];
            }
            break;
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getPointRotation( unsigned pointNumber, double * rot ) const
{
   if ( rot && pointNumber < m_points.size() )
   {
      switch ( m_animationMode )
      {
         case ANIMMODE_NONE:
            rot[0] = m_points[ pointNumber ]->m_rot[ 0 ];
            rot[1] = m_points[ pointNumber ]->m_rot[ 1 ];
            rot[2] = m_points[ pointNumber ]->m_rot[ 2 ];
            return true;

         case ANIMMODE_FRAME:
            getFrameAnimPointRotation( m_currentAnim, m_currentFrame, pointNumber,
                  rot[0], rot[1], rot[2] );
            return true;

         default:
            break;
      }
   }
   return false;
}

bool Model::getPointTranslation( unsigned pointNumber, double * trans ) const
{
   if ( trans && pointNumber < m_points.size() )
   {
      trans[0] = m_points[ pointNumber ]->m_trans[ 0 ];
      trans[1] = m_points[ pointNumber ]->m_trans[ 1 ];
      trans[2] = m_points[ pointNumber ]->m_trans[ 2 ];
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setPointRotation( unsigned pointNumber, const double * rot )
{
   if ( rot && pointNumber < m_points.size() )
   {
      switch ( m_animationMode )
      {
         case ANIMMODE_NONE:
            {
               Point * pnt = m_points[ pointNumber ];

               MU_SetPointRotation * undo = new MU_SetPointRotation();
               undo->setPointRotation( pointNumber, rot[0], rot[1], rot[2],
                     pnt->m_rot[0], pnt->m_rot[1], pnt->m_rot[2] );
               sendUndo( undo );

               pnt->m_rot[ 0 ] = rot[ 0 ];
               pnt->m_rot[ 1 ] = rot[ 1 ];
               pnt->m_rot[ 2 ] = rot[ 2 ];
            }
            return true;

         case ANIMMODE_FRAME:
            setFrameAnimPointRotation( m_currentAnim, m_currentFrame, pointNumber,
               rot[0], rot[1], rot[2] );
            return true;

         default:
            break;
      }
   }

   return false;
}

bool Model::setPointTranslation( unsigned pointNumber, const double * trans )
{
   if ( trans && pointNumber < m_points.size() )
   {
      Point * pnt = m_points[ pointNumber ];

      MU_SetPointTranslation * undo = new MU_SetPointTranslation();
      undo->setPointTranslation( pointNumber, trans[0], trans[1], trans[2],
            pnt->m_trans[0], pnt->m_trans[1], pnt->m_trans[2] );
      sendUndo( undo );

      pnt->m_trans[ 0 ] = trans[0];
      pnt->m_trans[ 1 ] = trans[1];
      pnt->m_trans[ 2 ] = trans[2];
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getBoneJointCoords( unsigned jointNumber, double * coord ) const
{
   if ( coord && jointNumber < m_joints.size() )
   {
      if ( m_animationMode == ANIMMODE_SKELETAL )
      {
         for ( int t = 0; t < 3; t++ )
         {
            coord[t] = m_joints[ jointNumber ]->m_final.get(3, t);
         }
      }
      else
      {
         for ( int t = 0; t < 3; t++ )
         {
            coord[t] = m_joints[ jointNumber ]->m_absolute.get(3, t);
         }
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getBoneJointFinalMatrix( unsigned jointNumber, Matrix & m ) const
{
   if ( jointNumber < m_joints.size() )
   {
      m = m_joints[ jointNumber ]->m_final;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getBoneJointAbsoluteMatrix( unsigned jointNumber, Matrix & m ) const
{
   if ( jointNumber < m_joints.size() )
   {
      m = m_joints[ jointNumber ]->m_absolute;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getBoneJointRelativeMatrix( unsigned jointNumber, Matrix & m ) const
{
   if ( jointNumber < m_joints.size() )
   {
      m = m_joints[ jointNumber ]->m_relative;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getPointFinalMatrix( unsigned pointNumber, Matrix & m ) const
{
   if ( pointNumber < m_points.size() )
   {
      Matrix mat;
      Point * p = m_points[ pointNumber ];

      if ( m_animationMode == ANIMMODE_SKELETAL )
      {
         /*
         mat.setTranslation( p->m_localTranslation );
         mat.setRotation( p->m_localRotation );

         if ( (unsigned) p->m_boneId < m_joints.size() )
         {
            mat = mat * m_joints[ p->m_boneId ]->m_final;
         }
         else
         {
            mat = mat * m_localMatrix;
         }
         */
         mat.setTranslation( p->m_kfTrans );
         mat.setRotation( p->m_kfRot );

         mat = mat * m_localMatrix;
      }
      else
      {
         mat.setTranslation( p->m_trans );
         mat.setRotation( p->m_rot );

         mat = mat * m_localMatrix;
      }

      m = mat;
      return true;
   }
   return false;
}

bool Model::getPointAbsoluteMatrix( unsigned pointNumber, Matrix & m ) const
{
   if ( pointNumber < m_points.size() )
   {
      Matrix mat;
      Point * p = m_points[ pointNumber ];

      mat.setTranslation( p->m_trans );
      mat.setRotation( p->m_rot );

      mat = mat * m_localMatrix;

      m = mat;
      return true;
   }
   return false;
}

int Model::getTriangleVertex( unsigned triangleNumber, unsigned vertexIndex ) const
{
   if ( triangleNumber < m_triangles.size() && vertexIndex < 3 )
   {
      return m_triangles[ triangleNumber ]->m_vertexIndices[ vertexIndex ];
   }
   else
   {
      return -1;
   }
}

bool Model::getNormal( unsigned triangleNum, unsigned vertexIndex, float *normal ) const
{
   if ( triangleNum < m_triangles.size() && vertexIndex < 3 )
   {
      for ( int t = 0; t < 3; t++ )
      {
         normal[t] = m_triangles[ triangleNum ]->m_vertexNormals[vertexIndex][t];
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getFlatNormal( unsigned t, float *normal ) const
{
   if ( t < m_triangles.size() )
   {
      float x1 = m_vertices[m_triangles[t]->m_vertexIndices[0]]->m_coord[0];
      float y1 = m_vertices[m_triangles[t]->m_vertexIndices[0]]->m_coord[1];
      float z1 = m_vertices[m_triangles[t]->m_vertexIndices[0]]->m_coord[2];
      float x2 = m_vertices[m_triangles[t]->m_vertexIndices[1]]->m_coord[0];
      float y2 = m_vertices[m_triangles[t]->m_vertexIndices[1]]->m_coord[1];
      float z2 = m_vertices[m_triangles[t]->m_vertexIndices[1]]->m_coord[2];
      float x3 = m_vertices[m_triangles[t]->m_vertexIndices[2]]->m_coord[0];
      float y3 = m_vertices[m_triangles[t]->m_vertexIndices[2]]->m_coord[1];
      float z3 = m_vertices[m_triangles[t]->m_vertexIndices[2]]->m_coord[2];

      float A = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
      float B = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
      float C = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);

      // Get flat normal
      float len = sqrt((A * A) + (B * B) + (C * C));

      A = A / len;
      B = B / len;
      C = C / len;

      normal[0] = A;
      normal[1] = B;
      normal[2] = C;

      return true;
   }
   else
   {
      return false;
   }
}

float Model::cosToPoint( unsigned t, double * point ) const
{
   if ( t < m_triangles.size() )
   {
      float x1 = m_vertices[m_triangles[t]->m_vertexIndices[0]]->m_coord[0];
      float y1 = m_vertices[m_triangles[t]->m_vertexIndices[0]]->m_coord[1];
      float z1 = m_vertices[m_triangles[t]->m_vertexIndices[0]]->m_coord[2];
      float x2 = m_vertices[m_triangles[t]->m_vertexIndices[1]]->m_coord[0];
      float y2 = m_vertices[m_triangles[t]->m_vertexIndices[1]]->m_coord[1];
      float z2 = m_vertices[m_triangles[t]->m_vertexIndices[1]]->m_coord[2];
      float x3 = m_vertices[m_triangles[t]->m_vertexIndices[2]]->m_coord[0];
      float y3 = m_vertices[m_triangles[t]->m_vertexIndices[2]]->m_coord[1];
      float z3 = m_vertices[m_triangles[t]->m_vertexIndices[2]]->m_coord[2];

      float A = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
      float B = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
      float C = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);

      // Get flat normal
      float len = sqrt((A * A) + (B * B) + (C * C));

      A = A / len;
      B = B / len;
      C = C / len;

      float normal[3];
      normal[0] = A;
      normal[1] = B;
      normal[2] = C;

      float vec[3];

      vec[0] = point[0] - x1;
      vec[1] = point[1] - y1;
      vec[2] = point[2] - z1;

      // normalized vector from plane to point
      normalize3( vec );

      float f = dot3( normal, vec );
      log_debug( "  dot3( %f,%f,%f  %f,%f,%f )\n",
            normal[0], normal[1], normal[2],
            vec[0], vec[1], vec[2] );
      log_debug( "  behind triangle dot check is %f\n", f );
      return f;
   }
   else
   {
      return 0.0f;
   }
}

bool Model::getBoneVector( unsigned joint, double * vec, const double * coord ) const
{
   if ( joint >= m_joints.size() )
   {
      return false;
   }

   double jcoord[3] = { 0, 0, 0 };
   getBoneJointCoords( joint, jcoord );

   double cdist = 0.0;
   int child = -1;
   int bcount = m_joints.size();

   // find best child bone joint vector based on the min distance between
   // the target coordinate and the child bone joint coordinate
   for ( int b = 0; b < bcount; b++ )
   {
      if ( getBoneJointParent( b ) == (int) joint )
      {
         double ccoord[3];
         getBoneJointCoords( b, ccoord );
         double d = distance( ccoord, coord );

         if ( child < 0 || d < cdist )
         {
            child = b;
            cdist = d;
            vec[0] = ccoord[0] - jcoord[0];
            vec[1] = ccoord[1] - jcoord[1];
            vec[2] = ccoord[2] - jcoord[2];
         }
      }
   }

   if ( child < 0 )
   {
      int parent = getBoneJointParent( joint );
      if ( parent < 0 )
      {
         // no children, no parent
         // return max influence
         return false;
      }

      getBoneJointCoords( parent, vec );

      cdist = distance( vec, coord );

      // We're using the parent instead of the child, so invert the direction
      vec[0] = jcoord[0] - vec[0];
      vec[1] = jcoord[1] - vec[1];
      vec[2] = jcoord[2] - vec[2];
   }

   normalize3( vec );
   return true;
}

void Model::calculateNormals()
{
   LOG_PROFILE();

   vector< vector<NormAngleAccum> > acl_normmap;
   acl_normmap.resize( m_vertices.size() );

   // accumulate normals
   std::vector<Triangle *>::iterator tri_it;

   for ( tri_it = m_triangles.begin(); tri_it != m_triangles.end(); ++tri_it )
   {
      Triangle * tri = *tri_it;
      tri->m_marked = false;

      float x1 = m_vertices[tri->m_vertexIndices[0]]->m_coord[0];
      float y1 = m_vertices[tri->m_vertexIndices[0]]->m_coord[1];
      float z1 = m_vertices[tri->m_vertexIndices[0]]->m_coord[2];
      float x2 = m_vertices[tri->m_vertexIndices[1]]->m_coord[0];
      float y2 = m_vertices[tri->m_vertexIndices[1]]->m_coord[1];
      float z2 = m_vertices[tri->m_vertexIndices[1]]->m_coord[2];
      float x3 = m_vertices[tri->m_vertexIndices[2]]->m_coord[0];
      float y3 = m_vertices[tri->m_vertexIndices[2]]->m_coord[1];
      float z3 = m_vertices[tri->m_vertexIndices[2]]->m_coord[2];

      float A = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
      float B = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
      float C = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);

      // Get flat normal
      float len = sqrt((A * A) + (B * B) + (C * C));

      A = A / len;
      B = B / len;
      C = C / len;

      tri->m_flatNormals[0] = A;
      tri->m_flatNormals[1] = B;
      tri->m_flatNormals[2] = C;

      // Accumulate for smooth normal, weighted by face angle
      for ( int vert = 0; vert < 3; vert++ )
      {
         unsigned index = tri->m_vertexIndices[vert];
         vector< NormAngleAccum > & acl = acl_normmap[index];

         float ax = 0.0f;
         float ay = 0.0f;
         float az = 0.0f;
         float bx = 0.0f;
         float by = 0.0f;
         float bz = 0.0f;

         switch ( vert )
         {
            case 0:
               {
                  ax = x2 - x1;
                  ay = y2 - y1;
                  az = z2 - z1;
                  bx = x3 - x1;
                  by = y3 - y1;
                  bz = z3 - z1;
               }
               break;
            case 1:
                  ax = x1 - x2;
                  ay = y1 - y2;
                  az = z1 - z2;
                  bx = x3 - x2;
                  by = y3 - y2;
                  bz = z3 - z2;
               break;
            case 2:
                  ax = x1 - x3;
                  ay = y1 - y3;
                  az = z1 - z3;
                  bx = x2 - x3;
                  by = y2 - y3;
                  bz = z2 - z3;
               break;
            default:
               break;
         }

         float ad = sqrt( ax*ax + ay*ay + az*az );
         float bd = sqrt( bx*bx + by*by + bz*bz );

         NormAngleAccum aacc;
         aacc.norm[0] = A;
         aacc.norm[1] = B;
         aacc.norm[2] = C;
         aacc.angle   = fabs( acos( (ax*bx + ay*by + az*bz) / (ad * bd) ) );

         acl.push_back( aacc );
      }
   }

   // Apply accumulated normals to triangles

   for ( unsigned g = 0; g < m_groups.size(); g++ )
   {
      Group * grp = m_groups[g];

      float maxAngle = grp->m_angle;
      if ( maxAngle < 0.50f )
      {
         maxAngle = 0.50f;
      }
      maxAngle *= PIOVER180;

      for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
            it != grp->m_triangleIndices.end();
            ++it )
      {
         Triangle * tri = m_triangles[*it];
         tri->m_marked = true;
         for ( int vert = 0; vert < 3; vert++ )
         {
            unsigned v = tri->m_vertexIndices[vert];

            {
               vector< NormAngleAccum > & acl = acl_normmap[v];

               float A = 0.0f;
               float B = 0.0f;
               float C = 0.0f;

               unsigned count = acl.size();
               unsigned n;

               // Use const_iterator instead?
               for ( n = 0; n < count; n++ )
               {
                  float crossprod = tri->m_flatNormals[0] * acl[n].norm[0] 
                     + tri->m_flatNormals[1] * acl[n].norm[1] 
                     + tri->m_flatNormals[2] * acl[n].norm[2];

                  // Don't allow it to go over 1.0f
                  float angle = 0.0f;
                  if ( crossprod < 0.99999f )
                  {
                     angle = fabs( acos( crossprod ) );
                  }

                  if ( angle <= maxAngle )
                  {
                     A += acl[n].norm[0];
                     B += acl[n].norm[1];
                     C += acl[n].norm[2];
                  }
               }

               float len = sqrt( A*A + B*B + C*C );

               if ( len >= 0.0001f )
               {
                  tri->m_vertexNormals[vert][0] = A / len;
                  tri->m_vertexNormals[vert][1] = B / len;
                  tri->m_vertexNormals[vert][2] = C / len;
               }
               else
               {
                  tri->m_vertexNormals[vert][0] = tri->m_flatNormals[0];
                  tri->m_vertexNormals[vert][1] = tri->m_flatNormals[1];
                  tri->m_vertexNormals[vert][2] = tri->m_flatNormals[2];
               }
               // May be overridden by group smoothing below
               tri->m_finalNormals[vert][0] = tri->m_vertexNormals[vert][0];
               tri->m_finalNormals[vert][1] = tri->m_vertexNormals[vert][1];
               tri->m_finalNormals[vert][2] = tri->m_vertexNormals[vert][2];
            }
         }
      }
   }

   for ( tri_it = m_triangles.begin(); tri_it != m_triangles.end(); ++tri_it )
   {
      Triangle * tri = *tri_it;
      if ( !tri->m_marked )
      {
         for ( int vert = 0; vert < 3; vert++ )
         {
            unsigned v = tri->m_vertexIndices[vert];

            {
               vector< NormAngleAccum > & acl = acl_normmap[v];

               float A = 0.0f;
               float B = 0.0f;
               float C = 0.0f;

               unsigned count = acl.size();
               unsigned n;

               for ( n = 0; n < count; n++ )
               {
                  float crossprod = tri->m_flatNormals[0] * acl[n].norm[0] 
                     + tri->m_flatNormals[1] * acl[n].norm[1] 
                     + tri->m_flatNormals[2] * acl[n].norm[2];

                  // Don't allow it to go over 1.0f
                  float angle = 0.0f;
                  if ( crossprod < 0.99999f )
                  {
                     angle = fabs( acos( crossprod ) );
                  }

                  if ( angle <= 45.0f * PIOVER180 )
                  {
                     A += acl[n].norm[0];
                     B += acl[n].norm[1];
                     C += acl[n].norm[2];
                  }
               }

               float len = sqrt( A*A + B*B + C*C );

               NormAccum acc;

               acc.norm[0] = A / len;
               acc.norm[1] = B / len;
               acc.norm[2] = C / len;

               tri->m_vertexNormals[vert][0] = acc.norm[0];
               tri->m_vertexNormals[vert][1] = acc.norm[1];
               tri->m_vertexNormals[vert][2] = acc.norm[2];

               // May be overridden by group smoothing below
               tri->m_finalNormals[vert][0] = acc.norm[0];
               tri->m_finalNormals[vert][1] = acc.norm[1];
               tri->m_finalNormals[vert][2] = acc.norm[2];
            }
         }
      }
      tri->m_marked = false;
   }

   for ( unsigned m = 0; m < m_groups.size(); m++ )
   {
      Group * grp = m_groups[m];

      double percent = (double) grp->m_smooth / 255.0;
      for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
            it != grp->m_triangleIndices.end();
            ++it )
      {
         Triangle * tri = m_triangles[ *it ];

         for ( int v = 0; v < 3; v++ )
         {
            if ( grp->m_smooth > 0 )
            {
               for ( unsigned i = 0; i < 3; i++ )
               {
                  tri->m_finalNormals[v][i] = tri->m_flatNormals[i]
                     + ( tri->m_vertexNormals[v][i] - tri->m_flatNormals[i] ) * percent;
               }
               normalize3( tri->m_finalNormals[v] );
            }
            else
            {
               tri->m_finalNormals[v][0] = tri->m_flatNormals[0];
               tri->m_finalNormals[v][1] = tri->m_flatNormals[1];
               tri->m_finalNormals[v][2] = tri->m_flatNormals[2];
            }
         }
      }
   }

   m_validNormals = true;
   m_validBspTree = false;
}

void Model::calculateSkelNormals()
{
   LOG_PROFILE();
   log_debug( "calculateSkelNormals()\n" );

   if ( ! m_validNormals )
   {
      calculateNormals();
   }
}

void Model::calculateFrameNormals( unsigned anim )
{
   LOG_PROFILE();

   if ( ! m_validNormals )
   {
      calculateNormals();
   }

   if ( anim < m_frameAnims.size() )
   {
      FrameAnim * fa = m_frameAnims[ anim ];

      for ( unsigned frame = 0; frame < fa->m_frameData.size(); frame++ )
      {
         FrameAnimVertexList & favl = *fa->m_frameData[frame]->m_frameVertices;
         for ( unsigned v = 0; v < m_vertices.size(); v++ )
         {
            favl[v]->m_normal[0] = 0.0f;
            favl[v]->m_normal[1] = 0.0f;
            favl[v]->m_normal[2] = 0.0f;
         }

         // accumulate normals
         for ( unsigned t = 0; t < m_triangles.size(); t++ )
         {
            Triangle * tri = m_triangles[t];
            float x1 = favl[tri->m_vertexIndices[0]]->m_coord[0];
            float y1 = favl[tri->m_vertexIndices[0]]->m_coord[1];
            float z1 = favl[tri->m_vertexIndices[0]]->m_coord[2];
            float x2 = favl[tri->m_vertexIndices[1]]->m_coord[0];
            float y2 = favl[tri->m_vertexIndices[1]]->m_coord[1];
            float z2 = favl[tri->m_vertexIndices[1]]->m_coord[2];
            float x3 = favl[tri->m_vertexIndices[2]]->m_coord[0];
            float y3 = favl[tri->m_vertexIndices[2]]->m_coord[1];
            float z3 = favl[tri->m_vertexIndices[2]]->m_coord[2];

            float A = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
            float B = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
            float C = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);

            // Get flat normal
            float len = sqrt((A * A) + (B * B) + (C * C));

            A = A / len;
            B = B / len;
            C = C / len;

            // Accumulate for smooth normal
            for ( int vert = 0; vert < 3; vert++ )
            {
               unsigned v = tri->m_vertexIndices[vert];
               favl[v]->m_normal[0] += A;
               favl[v]->m_normal[1] += B;
               favl[v]->m_normal[2] += C;
            }
         }

         // Normalize the accumulated normals
         for ( unsigned v = 0; v < m_vertices.size(); v++ )
         {
            normalize3(favl[v]->m_normal);
         }

         fa->m_validNormals = true;
      }
   }
}

void Model::invalidateNormals()
{
   m_changeBits |= MoveGeometry;

   if ( m_validNormals )
   {
      unsigned anim;
      for ( anim = 0; anim < m_skelAnims.size(); anim++ )
      {
         m_skelAnims[anim]->m_validNormals = false;
      }
      for ( anim = 0; anim < m_frameAnims.size(); anim++ )
      {
         m_frameAnims[anim]->m_validNormals = false;
      }
      m_validNormals = false;
   }

   m_validBspTree = false;
}

void Model::calculateBspTree()
{
   log_debug( "calculating BSP tree\n" );
   m_bspTree.clear();

   if ( m_animationMode == ANIMMODE_SKELETAL && m_currentAnim >= m_skelAnims.size() )
   {
      return;
   }
   else if ( m_animationMode == ANIMMODE_FRAME && m_currentAnim >= m_frameAnims.size() )
   {
      return;
   }

   for ( unsigned m = 0; m < m_groups.size(); m++ )
   {
      Group * grp = m_groups[m];
      if ( grp->m_materialIndex >= 0 )
      {
         int index = grp->m_materialIndex;

         if ( m_materials[ index ]->m_type == Model::Material::MATTYPE_TEXTURE
               && m_materials[ index ]->m_textureData->m_format == Texture::FORMAT_RGBA )
         {
            for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
                  it != grp->m_triangleIndices.end();
                  ++it )
            {
               Triangle * triangle = m_triangles[ *it ];
               triangle->m_marked = true;

               BspTree::Poly * poly = BspTree::Poly::get();

               if ( m_animationMode == ANIMMODE_SKELETAL )
               {
                  for (int i = 0; i < 3; i++ )
                  {
                     poly->coord[0][i] = m_vertices[ triangle->m_vertexIndices[0] ]->m_drawSource[i];
                     poly->coord[1][i] = m_vertices[ triangle->m_vertexIndices[1] ]->m_drawSource[i];
                     poly->coord[2][i] = m_vertices[ triangle->m_vertexIndices[2] ]->m_drawSource[i];

                     poly->drawNormals[0][i] = triangle->m_normalSource[0][i];
                     poly->drawNormals[1][i] = triangle->m_normalSource[1][i];
                     poly->drawNormals[2][i] = triangle->m_normalSource[2][i];

                     poly->norm[i] = triangle->m_flatSource[i];
                  }
               }
               else if ( m_animationMode == ANIMMODE_FRAME )
               {
                  FrameAnimVertex * vertex0 = ((*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[ triangle->m_vertexIndices[0] ]);
                  FrameAnimVertex * vertex1 = ((*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[ triangle->m_vertexIndices[1] ]);
                  FrameAnimVertex * vertex2 = ((*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[ triangle->m_vertexIndices[2] ]);

                  for (int i = 0; i < 3; i++ )
                  {
                     poly->coord[0][i] = vertex0->m_coord[i];
                     poly->coord[1][i] = vertex1->m_coord[i];
                     poly->coord[2][i] = vertex2->m_coord[i];

                     poly->drawNormals[0][i] = vertex0->m_normal[i];
                     poly->drawNormals[1][i] = vertex1->m_normal[i];
                     poly->drawNormals[2][i] = vertex2->m_normal[i];
                  }

                  float x1 = vertex0->m_coord[0];
                  float y1 = vertex0->m_coord[1];
                  float z1 = vertex0->m_coord[2];
                  float x2 = vertex1->m_coord[0];
                  float y2 = vertex1->m_coord[1];
                  float z2 = vertex1->m_coord[2];
                  float x3 = vertex2->m_coord[0];
                  float y3 = vertex2->m_coord[1];
                  float z3 = vertex2->m_coord[2];

                  float A = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
                  float B = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
                  float C = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);

                  // Get flat normal
                  float len = sqrt((A * A) + (B * B) + (C * C));

                  A = A / len;
                  B = B / len;
                  C = C / len;

                  poly->norm[0] = A;
                  poly->norm[1] = B;
                  poly->norm[2] = C;
               }
               else
               {
                  for (int i = 0; i < 3; i++ )
                  {
                     poly->coord[0][i] = m_vertices[ triangle->m_vertexIndices[0] ]->m_coord[i];
                     poly->coord[1][i] = m_vertices[ triangle->m_vertexIndices[1] ]->m_coord[i];
                     poly->coord[2][i] = m_vertices[ triangle->m_vertexIndices[2] ]->m_coord[i];

                     poly->drawNormals[0][i] = triangle->m_finalNormals[0][i];
                     poly->drawNormals[1][i] = triangle->m_finalNormals[1][i];
                     poly->drawNormals[2][i] = triangle->m_finalNormals[2][i];

                     poly->norm[i] = triangle->m_flatNormals[i];
                  }
               }

               for (int i = 0; i < 3; i++ )
               {
                  poly->s[i] = triangle->m_s[i];
                  poly->t[i] = triangle->m_t[i];
               }
               poly->texture = index;
               poly->material = static_cast< void *>( m_materials[ index ] );
               poly->triangle = static_cast< void *>( triangle );
               poly->calculateD();
               m_bspTree.addPoly( poly );
            }
         }
      }
   }

   m_validBspTree = true;
}

void Model::invalidateBspTree()
{
   m_validBspTree = false;
}

bool Model::isTriangleMarked( unsigned int t ) const
{
   if ( t < m_triangles.size() )
   {
      return m_triangles[t]->m_userMarked;
   }
   return false;
}

void Model::setTriangleMarked( unsigned t, bool marked )
{
   if ( t < m_triangles.size() )
   {
      m_triangles[t]->m_userMarked = marked;
   }
}

void Model::clearMarkedTriangles()
{
   unsigned tcount = m_triangles.size();
   for ( unsigned t = 0; t < tcount; t++ )
   {
      m_triangles[t]->m_userMarked = false;
   }
}

void model_show_alloc_stats()
{
   log_debug( "\n" );
   log_debug( "primitive allocation stats (recycler/total)\n" );

   log_debug( "Model: none/%d\n", _allocated );
   Model::Vertex::stats();
   Model::Triangle::stats();
   Model::Group::stats();
   Model::Material::stats();
   Model::Keyframe::stats();
   Model::Joint::stats();
   Model::Point::stats();
   Model::TextureProjection::stats();
   Model::SkelAnim::stats();
   Model::FrameAnim::stats();
   Model::FrameAnimVertex::stats();
   Model::FrameAnimPoint::stats();
   BspTree::Poly::stats();
   BspTree::Node::stats();
   log_debug( "Textures: none/%d\n", Texture::s_allocated );
   log_debug( "GlTextures: none/%d\n", Model::s_glTextures );
#ifdef MM3D_EDIT
   log_debug( "ModelUndo: none/%d\n", ModelUndo::s_allocated );
#endif // MM3D_EDIT
   log_debug( "\n" );
}

int model_free_primitives()
{
   int c = 0;

   log_debug( "purging primitive recycling lists\n" );

   c += Model::Vertex::flush();
   c += Model::Triangle::flush();
   c += Model::Group::flush();
   c += Model::Material::flush();
   c += Model::Joint::flush();
   c += Model::Point::flush();
   c += Model::Keyframe::flush();
   c += Model::SkelAnim::flush();
   c += BspTree::Poly::flush();
   c += BspTree::Node::flush();
   c += Model::FrameAnim::flush();
   c += Model::FrameAnimVertex::flush();
   c += Model::FrameAnimPoint::flush();

   return c;
}


