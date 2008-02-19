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


#include "model.h"
#include "log.h"
#include "msg.h"
#include "texmgr.h"
#include "texture.h"
#include "translate.h"

#include <math.h>
#include <map>

#ifdef MM3D_EDIT
#include "modelundo.h"
#endif // MM3D_EDIT

static void _calculateNormal( float * normal,
      double * a, double * b, double * c )
{
   normal[0] = a[1] * (b[2] - c[2]) + b[1] * (c[2] - a[2]) + c[1] * (a[2] - b[2]);
   normal[1] = a[2] * (b[0] - c[0]) + b[2] * (c[0] - a[0]) + c[2] * (a[0] - b[0]);
   normal[2] = a[0] * (b[1] - c[1]) + b[0] * (c[1] - a[1]) + c[0] * (a[1] - b[1]);

   normalize3( normal );
}

static bool _float_equiv( double lhs, double rhs, double tolerance )
{
   return ( fabs( lhs - rhs ) < tolerance );
}

// FIXME centralize this
const double EQ_TOLERANCE = 0.00001;

// FIXME centralize this
template<typename T>
bool floatCompareVector( T * lhs, T * rhs, size_t len, double tolerance = EQ_TOLERANCE )
{
   for ( size_t index = 0; index < len; ++index )
      if ( fabs( lhs[index] - rhs[index] ) > tolerance )
         return false;
   return true;
}

// FIXME centralize this
// FIXME test this
static bool matrixEquiv( const Matrix & lhs, const Matrix & rhs, double tolerance = EQ_TOLERANCE )
{
   Vector lright( 2, 0, 0 );
   Vector lup( 0, 2, 0 );
   Vector lfront( 0, 0, 2 );

   Vector rright( 2, 0, 0 );
   Vector rup( 0, 2, 0 );
   Vector rfront( 0, 0, 2 );

   lhs.apply( lright );
   lhs.apply( lup );
   lhs.apply( lfront );

   rhs.apply( rright );
   rhs.apply( rup );
   rhs.apply( rfront );

   if ( (lright - rright).mag3() > tolerance )
      return false;
   if ( (lup - rup).mag3() > tolerance )
      return false;
   if ( (lfront - rfront).mag3() > tolerance )
      return false;

   return true;
}

static bool jointsMatch( const Model * lhs, int lb, const Model * rhs, int rb )
{
   // Check for root joints
   if ( lb < 0 && rb < 0 )
      return true;  // both are root joints, match
   else if ( lb < 0 )
      return false;  // only left is root, no match
   else if ( rb < 0 )
      return false;  // only right is root, no match

   Matrix lMat;
   Matrix rMat;

   lhs->getBoneJointFinalMatrix( lb, lMat );
   rhs->getBoneJointFinalMatrix( rb, rMat );

   if ( matrixEquiv( lMat, rMat ) )
   {
      // It's only a match if the parents match
      return jointsMatch(
            lhs, lhs->getBoneJointParent(lb),
            rhs, rhs->getBoneJointParent(rb) );
   }

   return false;
}

bool Model::equivalent( const Model * model, double tolerance ) const
{
   int lhsTCount    = m_triangles.size();
   int rhsTCount    = model->m_triangles.size();
   int lhsGCount    = m_groups.size();
   int rhsGCount    = model->m_groups.size();
   int lhsBCount    = m_joints.size();
   int rhsBCount    = model->m_joints.size();
   int lhsPCount    = m_points.size();
   int rhsPCount    = model->m_points.size();

   if ( lhsTCount != rhsTCount )
   {
      log_warning( "lhs triangle count %d does not match rhs %d\n", lhsTCount, rhsTCount );
      return false;
   }

   if ( lhsPCount != rhsPCount )
   {
      log_warning( "lhs point count %d does not match rhs %d\n", lhsPCount, rhsPCount );
      return false;
   }

   if ( lhsBCount != rhsBCount )
   {
      log_warning( "lhs bone joint count %d does not match rhs %d\n", lhsBCount, rhsBCount );
      return false;
   }

   int v = 0;
   int g = 0;
   int m = 0;
   int b = 0;
   int p = 0;

   // Mapping to a list of ints allows a single group on the lhs to be equal
   // several smaller groups on the rhs.
   typedef std::map<int,std::list<int> > IntListMap;
   typedef std::vector<bool> BoolList;
   typedef std::list<int> IntList;
   typedef std::map<int,int> IntMap;

   // Match skeletons

   BoolList jointMatched;
   jointMatched.resize( rhsBCount, false );

   // Save joint mapping for influence comparision.
   // Key = lhs joint, value = rhs joint
   IntMap jointMap;

   for ( b = 0; b < lhsBCount; ++b )
   {
      bool match = false;
      for ( int rb = 0; !match && rb < rhsBCount; ++rb )
      {
         if ( !jointMatched[rb] )
         {
            if ( jointsMatch( this, b, model, rb ) )
            {
               jointMatched[ rb ] = true;
               jointMap[ b ] = rb;
               match = true;
            }
         }
      }

      if ( !match )
      {
         std::string dstr;
         m_joints[ b ]->sprint( dstr );
         log_warning( "no match for lhs joint %d:\n%s\n", b, dstr.c_str() );
         return false;
      }
   }

   // Find groups that are equivalent.

   IntListMap groupMap;

   // FIXME need to check vertex influences.

   for ( g = 0; g < lhsGCount; ++g )
   {
      for ( int r = 0; r < rhsGCount; ++r )
      {
         if ( m_groups[g]->propEqual( *model->m_groups[r], Model::PropNormals ) )
         {
            bool match = false;

            m = getGroupTextureId(g);
            int rm = model->getGroupTextureId(r);
            if ( m < 0 && rm < 0 )
            {
               match = true;
            }
            if ( m >= 0 && rm >= 0 )
            {
               if ( m_materials[m]->propEqual( *model->m_materials[rm],
                          Model::PropType
                        | Model::PropLighting
                        | Model::PropClamp
                        | Model::PropPixels
                        | Model::PropDimensions) )
               {
                  match = true;
               }
            }

            if ( match )
               groupMap[g].push_back( r );
         }
      }
   }

   BoolList triMatched;
   triMatched.resize( rhsTCount, false );

   IntList gtris;
   IntList rgtris;

   // For each triangle in each group, find a match in a corresponding group.
   for ( g = -1; g < lhsGCount; ++g )
   {
      if ( g >= 0 )
      {
         gtris = getGroupTriangles( g );

         rgtris.clear();
         if ( !gtris.empty() )
         {
            // Add all triangles from all matching groups to rgtris
            IntListMap::const_iterator it = groupMap.find( g );

            if ( it != groupMap.end() )
            {
               for ( IntList::const_iterator lit = it->second.begin();
                     lit != it->second.end(); ++lit )
               {
                  IntList temp = model->getGroupTriangles( *lit );
                  rgtris.insert( rgtris.end(), temp.begin(), temp.end() );
               }
            }
         }
      }
      else
      {
         gtris = getUngroupedTriangles();
         rgtris = model->getUngroupedTriangles();
      }

      // Don't compare group sizes.  This causes a failure if the lhs has two
      // equal groups that are combined into one on the rhs.

      bool compareTexCoords = false;

      if ( g >= 0 )
      {
         // Only compare texture coordinates if the coordinates affect the
         // material at that vertex (texture map, gradient, etc).
         compareTexCoords =
            (getMaterialType(g) != Model::Material::MATTYPE_BLANK);
      }

      for ( IntList::const_iterator it = gtris.begin(); it != gtris.end(); ++it )
      {
         bool match = false;
         for ( IntList::const_iterator rit = rgtris.begin();
               !match && rit != rgtris.end(); ++rit )
         {
            if ( triMatched[ *rit ] )
               continue;

            for ( int i = 0; !match && i < 3; i++ )
            {
               match = true;
               for ( int j = 0; match && j < 3; ++j )
               {
                  double coords[3];
                  double rcoords[3];

                  v = m_triangles[*it]->m_vertexIndices[j];
                  int rv = model->m_triangles[*rit]->m_vertexIndices[ (j+i) % 3];
                  getVertexCoords( v, coords );
                  model->getVertexCoords( rv, rcoords );

                  if ( !floatCompareVector( coords, rcoords, 3, tolerance ) )
                  {
                     log_warning( "failed match coords for index %d\n", j );
                     match = false;
                  }

                  if ( match && compareTexCoords )
                  {
                     float s = 0.0f;
                     float t = 0.0f;
                     float rs = 0.0f;
                     float rt = 0.0f;

                     getTextureCoords( *it, j, s, t );
                     model->getTextureCoords( *rit, (j+i) % 3, rs, rt );

                     if ( fabs( s -rs ) > tolerance
                           || fabs( t -rt ) > tolerance )
                     {
                        match = false;
                     }
                  }
               }

               if ( match )
               {
                  triMatched[ *rit ] = true;
               }
            }
         }

         if ( !match )
         {
            std::string dstr;
            m_triangles[ *it ]->sprint( dstr );
            log_warning( "no match for lhs triangle %d: %s\n", *it, dstr.c_str() );
            return false;
         }
      }
   }

   // Find points that are equivalent.

   BoolList pointMatched;
   pointMatched.resize( rhsPCount, false );

   double trans[3] = { 0, 0, 0 };
   double rot[3] = { 0, 0, 0 };

   for ( p = 0; p < lhsPCount; ++p )
   {
      bool match = false;
      for ( int rp = 0; !match && rp < rhsPCount; ++rp )
      {
         if ( !pointMatched[rp] )
         {
            Matrix lMat;
            Matrix rMat;

            getPointRotation( p, rot );
            getPointTranslation( p, trans );
            lMat.setRotation( rot );
            lMat.setTranslation( trans );

            model->getPointRotation( rp, rot );
            model->getPointTranslation( rp, trans );
            rMat.setRotation( rot );
            rMat.setTranslation( trans );

            if ( matrixEquiv( lMat, rMat ) )
            {
               match = true;
               pointMatched[ rp ] = true;
            }
         }
      }

      if ( !match )
      {
         std::string dstr;
         m_points[ p ]->sprint( dstr );
         log_warning( "no match for lhs point %d:\n%s\n", p, dstr.c_str() );
         return false;
      }

      // FIXME match influences
   }

   // FIXME finish this (animations)
   // Don't need to check texture contents, already did that above.

#if 0
   if ( compareMask & CompareSkeleton )
   {
      if ( numJoints == model->m_joints.size() && numVertices == model->m_vertices.size() )
      {
         bool match = true;

         for ( unsigned j = 0; match && j < numJoints; j++ )
         {
            if ( m_joints[j]->m_parent != model->m_joints[j]->m_parent )
            {
               match = false;
               log_warning( "match failed at joint parent for joint %d\n", j );
            }
            for ( unsigned i = 0; i < 3; i++ )
            {
               if ( fabs( m_joints[j]->m_localRotation[i] - model->m_joints[j]->m_localRotation[i] ) > tolerance )
               {
                  log_warning( "match failed at joint rotation for joint %d\n", j );
                  match = false;
               }
               if ( fabs( m_joints[j]->m_localTranslation[i] - model->m_joints[j]->m_localTranslation[i] ) > tolerance )
               {
                  log_warning( "match failed at joint translation for joint %d\n", j );
                  match = false;
               }
            }
         }

         for ( v = 0; match && v < numVertices; v++ )
         {
            InfluenceList * ila = &m_vertices[v]->m_influences;
            InfluenceList * ilb = &model->m_vertices[v]->m_influences;

            if ( ila->size() == ilb->size() )
            {
               InfluenceList::iterator ita;
               InfluenceList::iterator itb;

               for ( ita = ila->begin(), itb = ilb->begin();
                     match && ita != ila->end() && itb != ilb->end();
                     ita++, itb++ )
               {
                  if ( (*ita).m_boneId != (*itb).m_boneId )
                  {
                     log_warning( "match failed at vertex influence joint for vertex %d\n", v );
                     match = false;
                  }
                  if ( (*ita).m_type != (*itb).m_type )
                  {
                     log_warning( "match failed at vertex influence type for vertex %d\n", v );
                     match = false;
                  }
                  if ( !_float_equiv( (*ita).m_weight, (*itb).m_weight, tolerance ) )
                  {
                     log_warning( "match failed at vertex influence weight for vertex %d\n", v );
                     match = false;
                  }
               }
            }
            else
            {
               match = false;
            }
         }

         for ( p = 0; match && p < numPoints; p++ )
         {
            InfluenceList * ila = &m_points[p]->m_influences;
            InfluenceList * ilb = &model->m_points[p]->m_influences;

            if ( ila->size() == ilb->size() )
            {
               InfluenceList::iterator ita;
               InfluenceList::iterator itb;

               for ( ita = ila->begin(), itb = ilb->begin();
                     match && ita != ila->end() && itb != ilb->end();
                     ita++, itb++ )
               {
                  if ( (*ita).m_boneId != (*itb).m_boneId )
                  {
                     log_warning( "match failed at point influence joint for point %d\n", p );
                     match = false;
                  }
                  if ( (*ita).m_type != (*itb).m_type )
                  {
                     log_warning( "match failed at point influence type for point %d\n", p );
                     match = false;
                  }
                  if ( !_float_equiv( (*ita).m_weight, (*itb).m_weight, tolerance ) )
                  {
                     log_warning( "match failed at point influence weight for point %d\n", p );
                     match = false;
                  }
               }
            }
            else
            {
               match = false;
            }
         }

         if ( match )
         {
            matchVal |= CompareSkeleton;
         }
      }
   }

   if ( compareMask & ComparePoints )
   {
      if ( numPoints == model->m_points.size() )
      {
         bool match = true;

         for ( unsigned p = 0; match && p < numPoints; p++ )
         {
            if ( m_points[p]->m_type != model->m_points[p]->m_type )
            {
               match = false;
               log_warning( "match failed at point type for point %d\n", p );
            }
            for ( unsigned i = 0; i < 3; i++ )
            {
               if ( fabs( m_points[p]->m_rot[i] - model->m_points[p]->m_rot[i] ) > tolerance )
               {
                  log_warning( "match failed at point rotation for point %d\n", p );
                  match = false;
               }
               if ( fabs( m_points[p]->m_trans[i] - model->m_points[p]->m_trans[i] ) > tolerance )
               {
                  log_warning( "match failed at point translation for point %d\n", p );
                  match = false;
               }
            }
         }

         if ( match )
         {
            matchVal |= ComparePoints;
         }
      }
   }

   if ( compareMask & CompareAnimData )
   {
      if (     numFrameAnims == model->m_frameAnims.size()
            && numSkelAnims == model->m_skelAnims.size()    )
      {
         bool match = true;

         for ( t = 0; match && t < numFrameAnims; t++ )
         {
            if ( m_frameAnims[t]->m_frameData.size() != model->m_frameAnims[t]->m_frameData.size() )
            {
               log_warning( "match failed at frame animation frame count for frame anim %d\n", t );
               match = false;
            }
            if ( fabs( m_frameAnims[t]->m_fps - model->m_frameAnims[t]->m_fps ) > tolerance )
            {
               log_warning( "match failed at frame animation fps for frame anim %d\n", t );
               match = false;
            }

            for ( unsigned i = 0; match && i < m_frameAnims[t]->m_frameData.size(); i++ )
            {
               for ( v = 0; match && v < numVertices; v++ )
               {
                  for ( unsigned n = 0; n < 3; n++ )
                  {
                     if ( fabs( ((*m_frameAnims[t]->m_frameData[i]->m_frameVertices)[v]->m_coord[n] )
                            - ((*model->m_frameAnims[t]->m_frameData[i]->m_frameVertices)[v]->m_coord[n] ) ) > tolerance )
                     {
                        log_warning( "match failed at frame animation coords for frame anim %d, vertex %d\n", t, v );
                        match = false;
                     }
                  }
               }
               for ( p = 0; match && p < numPoints; v++ )
               {
                  for ( unsigned n = 0; n < 3; n++ )
                  {
                     if ( fabs( ((*m_frameAnims[t]->m_frameData[i]->m_framePoints)[p]->m_trans[n] )
                            - ((*model->m_frameAnims[t]->m_frameData[i]->m_framePoints)[p]->m_trans[n] ) ) > tolerance )
                     {
                        log_warning( "match failed at frame animation coords for frame anim %d, point %d\n", t, p );
                        match = false;
                     }
                  }
               }
            }
         }

         for ( t = 0; match && t < numSkelAnims; t++ )
         {
            if ( m_skelAnims[t]->m_frameCount != model->m_skelAnims[t]->m_frameCount )
            {
               log_warning( "match failed at skel animation frame count for skel anim %d\n", t );
               match = false;
            }
            if ( fabs( m_skelAnims[t]->m_fps - model->m_skelAnims[t]->m_fps ) > tolerance )
            {
               log_warning( "match failed at skel animation fps for skel anim %d\n", t );
               match = false;
            }

            for ( unsigned j = 0; j < m_skelAnims[t]->m_jointKeyframes.size(); j++ )
            {
               for ( unsigned i = 0; i < m_skelAnims[t]->m_jointKeyframes[j].size(); i++ )
               {
                  if (    m_skelAnims[t]->m_jointKeyframes[j][i]->m_isRotation 
                       != model->m_skelAnims[t]->m_jointKeyframes[j][i]->m_isRotation )
                  {
                     log_warning( "match failed at skel animation keyframe rotation for skel anim %d joint %d, keyframe %i\n", t, j, i );
                     match = false;
                  }
                  if (   fabs( m_skelAnims[t]->m_jointKeyframes[j][i]->m_time 
                       - model->m_skelAnims[t]->m_jointKeyframes[j][i]->m_time ) > tolerance )
                  {
                     log_warning( "match failed at skel animation keyframe time for skel anim %d joint %d, keyframe %i\n", t, j, i );
                     match = false;
                  }
                  for ( unsigned n = 0; n < 3; n++ )
                  {
                     if (   fabs( m_skelAnims[t]->m_jointKeyframes[j][i]->m_parameter[n] 
                              - model->m_skelAnims[t]->m_jointKeyframes[j][i]->m_parameter[n] ) > tolerance )
                     {
                        log_warning( "match failed at skel animation keyframe parameter for skel anim %d joint %d, keyframe %i\n", t, j, i );
                        match = false;
                     }
                  }
               }
            }
         }

         if ( match )
         {
            matchVal |= CompareAnimData;
            matchVal |= CompareAnimSets; 
         }
      }
   }

   if ( (compareMask & CompareAnimSets) && !(matchVal & CompareAnimData) )
   {
      if (     numFrameAnims == model->m_frameAnims.size()
            && numSkelAnims == model->m_skelAnims.size()    )
      {
         bool match = true;

         for ( t = 0; match && t < numFrameAnims; t++ )
         {
            if ( m_frameAnims[t]->m_frameData.size() != model->m_frameAnims[t]->m_frameData.size() )
            {
               log_debug( "match failed at frame animation frame count for frame anim %d\n", t );
               match = false;
            }
            if ( fabs( m_frameAnims[t]->m_fps - model->m_frameAnims[t]->m_fps ) > tolerance )
            {
               log_debug( "match failed at frame animation fps for frame anim %d\n", t );
               match = false;
            }
         }

         for ( t = 0; match && t < numSkelAnims; t++ )
         {
            if ( m_skelAnims[t]->m_frameCount != model->m_skelAnims[t]->m_frameCount )
            {
               log_debug( "match failed at skel animation frame count for skel anim %d\n", t );
               match = false;
            }
            if ( fabs( m_skelAnims[t]->m_fps - model->m_skelAnims[t]->m_fps ) > tolerance )
            {
               log_debug( "match failed at skel animation fps for skel anim %d\n", t );
               match = false;
            }
         }

         if ( match )
         {
            matchVal |= CompareAnimSets;
         }
      }
   }

#endif  // 0
   return true;
}

bool Model::propEqual( const Model * model, int partBits, int propBits,
      double tolerance ) const
{
   unsigned numVertices    = m_vertices.size();
   unsigned numTriangles   = m_triangles.size();
   unsigned numGroups      = m_groups.size();
   unsigned numJoints      = m_joints.size();
   unsigned numPoints      = m_points.size();
   unsigned numTextures    = m_materials.size();
   unsigned numProjections = m_projections.size();
   unsigned numFrameAnims  = m_frameAnims.size();
   unsigned numSkelAnims   = m_skelAnims.size();

   unsigned t = 0;
   unsigned v = 0;

   std::string dstr;

   if ( partBits & PartVertices )
   {
      if (numVertices != model->m_vertices.size())
      {
         log_warning( "match failed at vertex count %d != %d\n",
               numVertices, model->m_vertices.size() );
         return false;
      }

      for ( v = 0; v < numVertices; v++ )
      {
         if ( !m_vertices[v]->propEqual( *model->m_vertices[v], partBits, tolerance ) )
         {
            log_warning( "match failed at vertex %d\n", v );
            m_vertices[v]->sprint( dstr );
            log_warning( "lhs:\n%s\n", dstr.c_str() );
            model->m_vertices[v]->sprint( dstr );
            log_warning( "rhs:\n%s\n", dstr.c_str() );
            return false;
         }
      }
   }

   if ( partBits & PartFaces )
   {
      if (numTriangles != model->m_triangles.size())
      {
         log_warning( "match failed at triangle count %d != %d\n",
               numTriangles, model->m_triangles.size() );
         return false;
      }

      for ( t = 0; t < numTriangles; t++ )
      {
         if ( !m_triangles[t]->propEqual( *model->m_triangles[t], propBits, tolerance ) )
         {
            log_warning( "match failed at triangle %d\n", t );
            m_triangles[t]->sprint( dstr );
            log_warning( "lhs:\n%s\n", dstr.c_str() );
            model->m_triangles[t]->sprint( dstr );
            log_warning( "rhs:\n%s\n", dstr.c_str() );
            return false;
         }
      }
   }

   if ( partBits & PartGroups )
   {
      if ( numGroups != (unsigned) model->getGroupCount() )
      {
         log_warning( "match failed at group count %d != %d\n",
               numGroups, model->m_groups.size() );
         return false;
      }

      for ( unsigned int g = 0; g < numGroups; g++ )
      {
         if ( !m_groups[g]->propEqual( *model->m_groups[g], propBits, tolerance ) )
         {
            log_warning( "match failed at group %d\n", g );
            m_groups[g]->sprint( dstr );
            log_warning( "lhs:\n%s\n", dstr.c_str() );
            model->m_groups[g]->sprint( dstr );
            log_warning( "rhs:\n%s\n", dstr.c_str() );
            return false;
         }
      }
   }

   if ( partBits & PartJoints )
   {
      if ( numJoints != model->m_joints.size() )
      {
         log_warning( "match failed at joint count %d != %d\n",
               numJoints, model->m_joints.size() );
         return false;
      }

      for ( unsigned j = 0; j < numJoints; j++ )
      {
         if ( !m_joints[j]->propEqual( *model->m_joints[j], propBits, tolerance ) )
         {
            log_warning( "match failed at joint %d\n", j );
            /*
            // FIXME
            m_joints[j]->sprint( dstr );
            log_warning( "lhs:\n%s\n", dstr.c_str() );
            model->m_joints[j]->sprint( dstr );
            log_warning( "rhs:\n%s\n", dstr.c_str() );
            */
            return false;
         }
      }
   }

   if ( partBits & PartPoints )
   {
      if ( numPoints != model->m_points.size() )
      {
         log_warning( "match failed at point count %d != %d\n",
               numPoints, model->m_points.size() );
         return false;
      }

      for ( unsigned p = 0; p < numPoints; p++ )
      {
         if ( !m_points[p]->propEqual( *model->m_points[p], propBits, tolerance ) )
         {
            log_warning( "match failed at point %d\n", p );
            m_points[p]->sprint( dstr );
            log_warning( "lhs:\n%s\n", dstr.c_str() );
            model->m_points[p]->sprint( dstr );
            log_warning( "rhs:\n%s\n", dstr.c_str() );
            return false;
         }
      }
   }

   if ( partBits & (PartMaterials | PartTextures) )
   {
      if (numTextures  != model->m_materials.size())
      {
         log_warning( "match failed at material count %d != %d\n",
               numTextures, model->m_materials.size() );
         return false;
      }

      for ( t = 0; t < numTextures; t++ )
      {
         if ( !m_materials[t]->propEqual( *model->m_materials[t], propBits, tolerance ) )
         {
            log_warning( "match failed at material %d\n", t );
            m_materials[t]->sprint( dstr );
            log_warning( "lhs:\n%s\n", dstr.c_str() );
            model->m_materials[t]->sprint( dstr );
            log_warning( "rhs:\n%s\n", dstr.c_str() );

            return false;
         }
      }
   }

   if ( partBits & PartProjections )
   {
      if ( numProjections != model->m_projections.size() )
      {
         log_warning( "match failed at projection count %d != %d\n",
               numVertices, model->m_vertices.size() );
         return false;
      }

      for ( t = 0; t < numProjections; t++ )
      {
         if ( !m_projections[t]->propEqual( *model->m_projections[t], propBits, tolerance ) )
         {
            log_warning( "match failed at projection %d\n", t );
            /*
            // FIXME
            m_projections[t]->sprint( dstr );
            log_warning( "lhs:\n%s\n", dstr.c_str() );
            model->m_projections[t]->sprint( dstr );
            log_warning( "rhs:\n%s\n", dstr.c_str() );
            */
            return false;
         }
      }
   }

   if ( partBits & PartSkelAnims )
   {
      if ( numSkelAnims != model->m_skelAnims.size() )
      {
         log_warning( "match failed at skel anim count %d != %d\n",
               numSkelAnims, model->m_skelAnims.size() );
         return false;
      }

      for ( t = 0; t < numSkelAnims; t++ )
      {
         if ( !m_skelAnims[t]->propEqual( *model->m_skelAnims[t], propBits, tolerance ) )
         {
            log_warning( "match failed at skel animation %d\n", t );
            return false;
         }
      }
   }

   if ( partBits & PartFrameAnims )
   {
      if ( numFrameAnims != model->m_frameAnims.size() )
      {
         log_warning( "match failed at frameAnim count %d != %d\n",
               numVertices, model->m_vertices.size() );
         return false;
      }

      for ( t = 0; t < numFrameAnims; t++ )
      {
         if ( !m_frameAnims[t]->propEqual( *model->m_frameAnims[t], propBits, tolerance ) )
         {
            log_warning( "match failed at frame animation %d\n", t );
            return false;
         }
      }
   }

   if ( partBits & PartMeta )
   {
      if ( getMetaDataCount() != model->getMetaDataCount() )
      {
         log_warning( "match failed at meta data count %d != %d\n",
               getMetaDataCount(), model->getMetaDataCount() );
         return false;
      }

      unsigned int mcount = getMetaDataCount();
      for ( unsigned int m = 0; m < mcount; ++m )
      {
         char key[1024];
         char value_lhs[1024];
         char value_rhs[1024];

         getMetaData( m, key, sizeof(key), value_lhs, sizeof(value_lhs) );

         if ( !model->getMetaData( m, key, sizeof(key), value_rhs, sizeof(value_rhs) ) )
         {
            log_warning( "missing meta data key: '%s'\n", key );
            return false;
         }
         else
         {
            if ( strcmp( value_lhs, value_rhs ) != 0 )
            {
               log_warning( "meta data value mismatch for '%s'\n", key );
               log_warning( "  '%s' != '%s'\n", value_lhs, value_rhs );
               return false;
            }
         }
      }
   }

   if ( partBits & PartBackgrounds )
   {
      for ( unsigned int b = 0; b < MAX_BACKGROUND_IMAGES; ++b )
      {
         if ( !m_background[b]->propEqual( *model->m_background[b], propBits, tolerance ) )
         {
            log_warning( "match failed at background image %d\n", t );
            /*
            // FIXME
            m_background[b]->sprint( dstr );
            log_warning( "lhs:\n%s\n", dstr.c_str() );
            model->m_background[b]->sprint( dstr );
            log_warning( "rhs:\n%s\n", dstr.c_str() );
            */
            return false;
         }
      }
   }

   return true;
}

#ifdef MM3D_EDIT

bool Model::mergeAnimations( Model * model )
{
   if ( m_animationMode )
   {
      return false;
   }

   unsigned count = model->getAnimCount( ANIMMODE_SKELETAL );
   unsigned ac1 = getAnimCount( ANIMMODE_SKELETAL );

   if ( count == 0 )
   {
      msg_warning( transll( QT_TRANSLATE_NOOP( "LowLevel", "Model contains no skeletal animations")).c_str() );
      return false;
   }

   unsigned j1 = getBoneJointCount();
   unsigned j2 = model->getBoneJointCount();


   std::string mismatchWarn = transll( QT_TRANSLATE_NOOP( "LowLevel", "Model skeletons do not match" ));
   if ( j1 != j2 )
   {
      msg_warning( mismatchWarn.c_str() );
      return false;
   }

   for ( unsigned j = 0; j < j1; j++ )
   {
      if ( m_joints[ j ]->m_parent != model->m_joints[j]->m_parent )
      {
         msg_warning( mismatchWarn.c_str() );
         return false;
      }
   }

   bool canAdd = canAddOrDelete();
   forceAddOrDelete( true );

   // Do skeletal add
   {
      for ( unsigned n = 0; n < count; n++ )
      {
         unsigned framecount = model->getAnimFrameCount( ANIMMODE_SKELETAL, n );

         unsigned index = addAnimation( ANIMMODE_SKELETAL, model->getAnimName( ANIMMODE_SKELETAL, n ) );
         setAnimFrameCount( ANIMMODE_SKELETAL, index, framecount );
         setAnimFPS( ANIMMODE_SKELETAL, n, model->getAnimFPS( ANIMMODE_SKELETAL, n ) );

         SkelAnim * sa = model->m_skelAnims[n];

         for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
         {
            for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
            {
               Keyframe * kf = sa->m_jointKeyframes[j][k];

               setSkelAnimKeyframe( ac1 + n, kf->m_frame, j, kf->m_isRotation,
                     kf->m_parameter[0], kf->m_parameter[1], kf->m_parameter[2] );
            }
         }
      }
   }

   invalidateNormals();

   forceAddOrDelete( canAdd && m_frameAnims.empty() );

   return true;
}

bool Model::mergeModels( Model * model, bool textures, AnimationMergeE animations, bool emptyGroups, double * trans, double * rot )
{
   if ( m_animationMode )
   {
      return false;
   }

   bool canAdd = canAddOrDelete();
   forceAddOrDelete( true );

   Matrix mat;

   if ( rot )
   {
      log_debug( "merge rotation: %f, %f, %f\n", rot[0], rot[1], rot[2] );
      mat.setRotation( rot );
   }

   if ( trans )
   {
      log_debug( "merge translation: %f, %f, %f\n", trans[0], trans[1], trans[2] );
      mat.setTranslation( trans );
   }

   unsigned vertbase   = 0;
   unsigned tribase    = 0;
   unsigned grpbase    = 0;
   unsigned jointbase  = 0;
   unsigned pointbase  = 0;
   unsigned projbase   = 0;
   unsigned matbase    = 0;

   unsigned n = 0;
   unsigned count = 0;

   std::map<int,int> m_groupMap;

   vertbase   = m_vertices.size();
   tribase    = m_triangles.size();
   grpbase    = m_groups.size();
   jointbase  = m_joints.size();
   pointbase  = m_points.size();
   projbase   = m_projections.size();
   matbase    = m_materials.size();

   unselectAll();

   count = model->m_vertices.size();
   for ( n = 0; n < count; n++ )
   {
      Vertex * vert = model->m_vertices[n];
      Vector vec( vert->m_coord );
      vec = vec * mat;
      addVertex( vec.get(0), vec.get(1), vec.get(2) );
   }

   count = model->m_triangles.size();
   for ( n = 0; n < count; n++ )
   {
      Triangle * tri = model->m_triangles[n];

      addTriangle( tri->m_vertexIndices[0] + vertbase,
            tri->m_vertexIndices[1] + vertbase, tri->m_vertexIndices[2] + vertbase );
   }

   count = model->m_groups.size();
   for ( n = 0; n < count; n++ )
   {
      if ( emptyGroups || !model->getGroupTriangles(n).empty() )
      {
         const char * name = model->getGroupName( n );
         m_groupMap[n] = addGroup( name );
      }
   }

   for ( n = 0; n < count; n++ )
   {
      uint8_t val = model->getGroupSmooth( n );
      setGroupSmooth( m_groupMap[n], val );
   }

   count = model->m_joints.size();
   if ( count > 0 )
   {
      model->setupJoints();
      for ( n = 0; n < count; n++ )
      {
         Joint * joint = model->m_joints[n];
         double rot[3];
         double tran[3];

         Matrix jabs = joint->m_absolute * mat;

         jabs.getRotation( rot );
         jabs.getTranslation( tran );

         int parent = joint->m_parent;

         if ( parent >= 0 )
         {
            parent += jointbase;
         }

         addBoneJoint( joint->m_name.c_str(), tran[0], tran[1], tran[2],
               rot[0], rot[1], rot[2], parent );
      }
   }

   count = model->m_points.size();
   if ( count > 0 )
   {
      for ( n = 0; n < count; n++ )
      {
         Point * point = model->m_points[n];

         Matrix abs;

         double rot[3];
         rot[0] = point->m_rot[0];
         rot[1] = point->m_rot[1];
         rot[2] = point->m_rot[2];

         abs.setTranslation( point->m_trans[0], point->m_trans[1], point->m_trans[2] );
         abs.setRotation( rot );

         Matrix pabs = abs * mat;

         double tran[3];

         pabs.getRotation( rot );
         pabs.getTranslation( tran );

         int pnum = addPoint( point->m_name.c_str(), tran[0], tran[1], tran[2],
               rot[0], rot[1], rot[2], -1 );

         InfluenceList * ilist = &model->m_points[n]->m_influences;
         InfluenceList::iterator it;

         for ( it = ilist->begin(); it != ilist->end(); it++ )
         {
            addPointInfluence( pnum, (*it).m_boneId + jointbase,
                  (*it).m_type, (*it).m_weight );
         }
      }
   }

   count = model->m_vertices.size();
   for ( n = 0; n < count; n++ )
   {
      InfluenceList * ilist = &model->m_vertices[n]->m_influences;
      InfluenceList::iterator it;

      for ( it = ilist->begin(); it != ilist->end(); it++ )
      {
         addVertexInfluence( n + vertbase, (*it).m_boneId + jointbase,
               (*it).m_type, (*it).m_weight );
      }
   }

   if ( textures )
   {
      TextureManager * texmgr = TextureManager::getInstance();

      count = model->getTextureCount();
      for ( n = 0; n < count; n++ )
      {
         if ( model->getMaterialType( n ) == Model::Material::MATTYPE_TEXTURE )
         {
            const char * name = model->getTextureFilename( n );
            Texture * newtex = texmgr->getTexture( name );

            addTexture( newtex );
         }
         else
         {
            const char * name = model->getTextureName( n );
            addColorMaterial( name );
         }
      }

      for ( n = 0; n < count; n++ )
      {
         float val[4] = { 0.0, 0.0, 0.0, 0.0 };
         float shin = 0.0;

         model->getTextureAmbient(  n, val );
         setTextureAmbient(  n + matbase, val );
         model->getTextureDiffuse(  n, val );
         setTextureDiffuse(  n + matbase, val );
         model->getTextureEmissive( n, val );
         setTextureEmissive( n + matbase, val );
         model->getTextureSpecular( n, val );
         setTextureSpecular( n + matbase, val );

         model->getTextureShininess( n, shin );
         setTextureShininess( n + matbase, shin );
      }

      count = model->m_groups.size();
      for ( n = 0; n < count; n++ )
      {
         int val = model->getGroupTextureId( n );
         setGroupTextureId( m_groupMap[n], val + matbase );
      }

      count = model->getProjectionCount();
      for ( n = 0; n < count; ++n )
      {
         const char * name = model->getProjectionName( n );
         int type = model->getProjectionType( n );

         double pos[3] = { 0, 0, 0 };
         double up[3] = { 0, 0, 0 };
         double seam[3] = { 0, 0, 0 };
         double range[2][2] = { { 0, 0 }, { 0, 0 } };

         model->getProjectionCoords( n, pos );
         model->getProjectionUp( n, up );
         model->getProjectionSeam( n, seam );
         model->getProjectionRange( n,
               range[0][0], range[0][1], range[1][0], range[1][1] );

         addProjection( name, type, pos[0], pos[1], pos[2] );
         setProjectionUp( n + projbase, up );
         setProjectionSeam( n + projbase, seam );
         setProjectionRange( n + projbase,
               range[0][0], range[0][1], range[1][0], range[1][1] );
      }

      int tpcount = getProjectionCount();

      count = model->getTriangleCount();
      float s = 0.0;
      float t = 0.0;
      for ( n = 0; n < count; n++ )
      {
         for ( unsigned i = 0; i < 3; i++ )
         {
            model->getTextureCoords( n,    i, s, t );
            setTextureCoords( n + tribase, i, s, t );
         }

         int grp = model->getTriangleGroup( n );
         if ( grp >= 0 )
         {
            addTriangleToGroup( m_groupMap[grp], n + tribase );
         }

         int prj = model->getTriangleProjection( n );
         if ( prj >= 0 && (prj + (int) projbase) < tpcount )
         {
            setTriangleProjection( n + tribase, prj + projbase );
         }
      }
   }

   bool frameAnimsNeeded = (getAnimCount( ANIMMODE_FRAME ) > 0 );

   if ( frameAnimsNeeded )
   {
      setFrameAnimVertexCount( m_vertices.size() );
      setFrameAnimPointCount( m_vertices.size() );
   }

   unsigned oldcount = getAnimCount( ANIMMODE_FRAME );
   if ( animations != AM_NONE )
   {
      // Do frame merge if possible
      unsigned ac1 = getAnimCount( ANIMMODE_FRAME );
      unsigned ac2 = model->getAnimCount( ANIMMODE_FRAME );

      bool match = false;

      if ( animations == AM_MERGE && ac1 == ac2 )
      {
         match = true; // Have to check frame counts too

         unsigned a = 0;

         for ( a = 0; match && a < ac1; a++ )
         {
            unsigned fc1 = getAnimFrameCount( ANIMMODE_FRAME, a );
            unsigned fc2 = model->getAnimFrameCount( ANIMMODE_FRAME, a );

            if ( fc1 != fc2 )
            {
               match = false;
            }
         }

         if ( match )
         {
            for ( a = 0; a < ac1; a++ )
            {
               unsigned fc1 = getAnimFrameCount( ANIMMODE_FRAME, a );
               unsigned f;

               unsigned vertcount = model->m_vertices.size();

               for ( f = 0; f < fc1; f++ )
               {
                  for ( unsigned v = 0; v < vertcount; v++ )
                  {
                     double coord[3] = { 0, 0, 0 };
                     model->getFrameAnimVertexCoords( a, f, v, coord[0], coord[1], coord[2] );
                     Vector vec( coord );
                     vec = vec * mat;
                     setFrameAnimVertexCoords( a, f, v + vertbase, vec.get(0), vec.get(1), vec.get(2) );
                  }
               }

               unsigned pointcount = model->m_points.size();

               for ( f = 0; f < fc1; f++ )
               {
                  for ( unsigned p = 0; p < pointcount; p++ )
                  {
                     double coord[3] = { 0, 0, 0 };
                     model->getFrameAnimPointCoords( a, f, p, coord[0], coord[1], coord[2] );
                     Vector vec( coord );
                     vec = vec * mat;
                     setFrameAnimPointCoords( a, f, p + pointbase, vec.get(0), vec.get(1), vec.get(2) );

                     model->getFrameAnimPointRotation( a, f, p, coord[0], coord[1], coord[2] );
                     Matrix m;
                     m.setRotation( coord );
                     m = m * mat;
                     m.getRotation( coord );
                     setFrameAnimPointRotation( a, f, p + pointbase, coord[0], coord[1], coord[2] );
                  }
               }
            }

            frameAnimsNeeded = false;
         }
      }

      // Do frame add otherwise
      if ( !match || animations == AM_ADD )
      {
         count = model->getAnimCount( ANIMMODE_FRAME );
         for ( n = 0; n < count; n++ )
         {
            unsigned framecount = model->getAnimFrameCount( ANIMMODE_FRAME, n );

            unsigned index = addAnimation( ANIMMODE_FRAME, model->getAnimName( ANIMMODE_FRAME, n ) );
            setAnimFrameCount( ANIMMODE_FRAME, index, framecount );

            unsigned f;

            unsigned vertcount = model->m_vertices.size();

            for ( f = 0; f < framecount; f++ )
            {
               for ( unsigned v = 0; v < vertcount; v++ )
               {
                  double coord[3] = { 0, 0, 0 };
                  model->getFrameAnimVertexCoords( n, f, v, coord[0], coord[1], coord[2] );
                  Vector vec( coord );
                  vec = vec * mat;
                  setFrameAnimVertexCoords( index, f, v + vertbase, vec.get(0), vec.get(1), vec.get(2) );
               }
            }

            unsigned pointcount = model->m_points.size();

            for ( f = 0; f < framecount; f++ )
            {
               for ( unsigned p = 0; p < pointcount; p++ )
               {
                  double coord[3] = { 0, 0, 0 };
                  model->getFrameAnimPointCoords( n, f, p, coord[0], coord[1], coord[2] );
                  Vector vec( coord );
                  vec = vec * mat;
                  setFrameAnimPointCoords( index, f, p + pointbase, vec.get(0), vec.get(1), vec.get(2) );

                  model->getFrameAnimPointRotation( n, f, p, coord[0], coord[1], coord[2] );
                  Matrix m;
                  m.setRotation( coord );
                  m = m * mat;
                  m.getRotation( coord );
                  setFrameAnimPointRotation( index, f, p + pointbase, coord[0], coord[1], coord[2] );
               }
            }
         }
      }

      // Do skeletal merge if possible
      ac1 = getAnimCount( ANIMMODE_SKELETAL );
      ac2 = model->getAnimCount( ANIMMODE_SKELETAL );

      match = false;
      if ( ac1 == ac2 && animations == AM_MERGE )
      {
         match = true; // Still need to check frame count

         unsigned a = 0;

         for ( a = 0; match && a < ac1; a++ )
         {
            unsigned fc1 = getAnimFrameCount( ANIMMODE_SKELETAL, a );
            unsigned fc2 = model->getAnimFrameCount( ANIMMODE_SKELETAL, a );

            if ( fc1 != fc2 )
            {
               match = false;
            }
         }

         if ( match )
         {
            for ( a = 0; a < ac1; a++ )
            {
               SkelAnim * sa = model->m_skelAnims[a];

               for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
               {
                  for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
                  {
                     Keyframe * kf = sa->m_jointKeyframes[j][k];

                     setSkelAnimKeyframe( a, kf->m_frame, j + jointbase, kf->m_isRotation,
                           kf->m_parameter[0], kf->m_parameter[1], kf->m_parameter[2] );
                  }
               }
            }
         }
      }

      // Do skeletal add otherwise
      if ( !match || animations == AM_ADD )
      {
         count = model->getAnimCount( ANIMMODE_SKELETAL );
         for ( n = 0; n < count; n++ )
         {
            unsigned framecount = model->getAnimFrameCount( ANIMMODE_SKELETAL, n );

            unsigned index = addAnimation( ANIMMODE_SKELETAL, model->getAnimName( ANIMMODE_SKELETAL, n ) );
            setAnimFrameCount( ANIMMODE_SKELETAL, index, framecount );
            setAnimFPS( ANIMMODE_SKELETAL, n, model->getAnimFPS( ANIMMODE_SKELETAL, n ) );

            SkelAnim * sa = model->m_skelAnims[n];

            for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
            {
               for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
               {
                  Keyframe * kf = sa->m_jointKeyframes[j][k];

                  setSkelAnimKeyframe( ac1 + n, kf->m_frame, j + jointbase, kf->m_isRotation,
                        kf->m_parameter[0], kf->m_parameter[1], kf->m_parameter[2] );
               }
            }
         }
      }
   }

   if ( frameAnimsNeeded )
   {
      // We have frame anims that don't have our new vertices.  
      // Must add them

      count = oldcount; // Only need to adjust original frame anims
      for ( n = 0; n < count; n++ )
      {
         unsigned framecount = getAnimFrameCount( ANIMMODE_FRAME, n );
         unsigned vertcount = model->m_vertices.size();

         for ( unsigned v = 0; v < vertcount; v++ )
         {
            double coord[3] = { 0, 0, 0 };
            model->getVertexCoords( v, coord );

            Vector vec( coord );
            vec = vec * mat;

            for ( unsigned f = 0; f < framecount; f++ )
            {
               setFrameAnimVertexCoords( n, f, v + vertbase, vec.get(0), vec.get(1), vec.get(2) );
            }
         }
      }
   }

   count = getTriangleCount();
   for ( n = tribase; n < count; ++n )
      selectTriangle( n );

   count = getBoneJointCount();
   for ( n = jointbase; n < count; ++n )
      selectBoneJoint( n );

   count = getPointCount();
   for ( n = pointbase; n < count; ++n )
      selectPoint( n );

   count = getProjectionCount();
   for ( n = projbase; n < count; ++n )
      selectProjection( n );

   invalidateNormals();
   setupJoints();

   forceAddOrDelete( canAdd && m_frameAnims.empty() );

   return true;
}

const int SE_POLY_MAX = 2;
typedef struct _SimplifyEdge_t
{
   unsigned int vFar;
   
   int polyCount;
   int poly[ SE_POLY_MAX ];
   float normal[ SE_POLY_MAX ][ 3 ];
} SimplifyEdgeT;
typedef std::list< SimplifyEdgeT > SimplifyEdgeList;

void Model::simplifySelectedMesh()
{
   // for each vertex
   //   find all edges
   //   if Va to V is same vector as V to Vb
   //     make sure every *other* edge has exactly two co-planar faces
   //     if so
   //       move V to Va and weld
   //       re-do loop at 

   unsigned int vcount = m_vertices.size();
   unsigned int v = 0;

   for ( v = 0; v < vcount; v++ )
   {
      m_vertices[v]->m_marked = false;
   }

   int tcount = m_triangles.size();
   int t = 0;

   for ( t = 0; t < tcount; t++ )
   {
      m_triangles[t]->m_marked = false;
   }

   SimplifyEdgeList edges;
   SimplifyEdgeList::iterator it;
   SimplifyEdgeList::iterator itA;
   SimplifyEdgeList::iterator itB;
   unsigned int verts[3];
   int idx[3];

   double coords[3];
   double tcoords[3][3];
   double vecA[3];
   double vecB[3];

   bool welded = false;
   bool valid  = true;

   v = 0;
   while ( v < vcount )
   {
      log_debug( "checking vertex %d\n", v );
      welded = false;

      //if ( !m_vertices[v]->m_marked )
      {
         valid = true; // valid weld candidate until we learn otherwise
         edges.clear();

         // build edge list
         for ( t = 0; valid && t < tcount; t++ )
         {
            // unflattened triangles only
            if ( m_triangles[t]->m_selected ) // && !m_triangles[t]->m_marked )
            {
               getTriangleVertices( t, verts[0], verts[1], verts[2] );
               idx[0] = -1;

               if ( verts[0] == v )
               {
                  idx[0] = 0;
                  idx[1] = 1;
                  idx[2] = 2;
               }
               if ( verts[1] == v )
               {
                  idx[0] = 1;
                  idx[1] = 0;
                  idx[2] = 2;
               }
               if ( verts[2] == v )
               {
                  idx[0] = 2;
                  idx[1] = 0;
                  idx[2] = 1;
               }

               // If triangle is using v as a vertex, add to edge list
               if ( idx[0] >= 0 )
               {
                  log_debug( "  triangle %d uses vertex %d\n", t, v );
                  // vert[idx[1]] and vert[idx[2]] are the opposite vertices
                  for ( int i = 1; i <= 2; i++ )
                  {
                     bool newEdge = true;
                     for ( it = edges.begin(); valid && it != edges.end(); it++ )
                     {
                        if ( (*it).vFar == verts[idx[i]] )
                        {
                           if ( (*it).polyCount < SE_POLY_MAX )
                           {
                              (*it).poly[ (*it).polyCount ] = t;
                              getFlatNormal( t, (*it).normal[ (*it).polyCount ] );

                              (*it).polyCount++;
                              newEdge = false;

                              log_debug( "  adding polygon to edge for %d\n",
                                    (*it).vFar );
                              break;
                           }
                           else
                           {
                              // more than two faces on this edge
                              // we can't weld at all, skip this vertex
                              log_debug( "  too many polygons connected to edge to %d\n",
                                    (*it).vFar );
                              valid = false;
                           }
                        }
                     }

                     if ( valid && newEdge )
                     {
                        log_debug( "  adding new edge for polygon for %d\n",
                              idx[i] );
                        SimplifyEdgeT se;
                        se.vFar = verts[ idx[i] ];
                        se.polyCount = 1;
                        se.poly[0] = t;
                        getFlatNormal( t, se.normal[ 0 ] );

                        edges.push_back( se );
                     }
                  }
               }
            }
         }

         if ( valid )
         {
            // use vectors from two edges to see if they are in a straight line
            getVertexCoords( v, coords );

            for ( itA = edges.begin(); valid && !welded && itA != edges.end(); itA++ )
            {
               getVertexCoords( (*itA).vFar, vecA );
               vecA[0] = coords[0] - vecA[0];
               vecA[1] = coords[1] - vecA[1];
               vecA[2] = coords[2] - vecA[2];
               normalize3( vecA );

               for ( itB = edges.begin(); valid && !welded && itB != edges.end(); itB++ )
               {
                  if ( itA != itB )
                  {
                     bool canWeld = true;

                     getVertexCoords( (*itB).vFar, vecB );
                     vecB[0] -= coords[0];
                     vecB[1] -= coords[1];
                     vecB[2] -= coords[2];
                     normalize3( vecB );

                     if ( equiv3( vecA, vecB ) )
                     {
                        log_debug( "  found a straight line\n" );
                        for ( it = edges.begin(); it != edges.end(); it++ )
                        {
                           if ( it != itA && it != itB )
                           {
                              // must have a face on each side of edge
                              if ( (*it).polyCount != 2 )
                              {
                                 log_debug( "    not enough polygons connected to edge\n" );
                                 canWeld = false;
                              }

                              // faces must be in the same plane
                              if ( canWeld && !equiv3( (*it).normal[0], (*it).normal[1] ) )
                              {
                                 log_debug( "    polygons on edge do not face the same direction\n" );
                                 canWeld = false;
                              }

                              // check inverted normals
                              for ( int i = 0; i < (*it).polyCount; i++ )
                              {
                                 getTriangleVertices( (*it).poly[i], 
                                       verts[0], verts[1], verts[2] );

                                 bool flat = false;

                                 for ( int n = 0; n < 3; n++ )
                                 {
                                    if ( verts[n] == v )
                                    {
                                       verts[n] = (*itA).vFar;
                                    }
                                    else if ( verts[n] == (*itA).vFar )
                                    {
                                       flat = true;
                                    }
                                 }

                                 if ( !flat )
                                 {
                                    getVertexCoords( verts[0], tcoords[0] );
                                    getVertexCoords( verts[1], tcoords[1] );
                                    getVertexCoords( verts[2], tcoords[2] );

                                    float norm[3];
                                    _calculateNormal( norm, tcoords[0], tcoords[1], tcoords[2] );

                                    log_debug( "-- %f,%f,%f  %f,%f,%f\n",
                                          norm[0], norm[1], norm[2],
                                          (*it).normal[i][0], (*it).normal[i][1], (*it).normal[i][2] );
                                    if ( !equiv3( norm, (*it).normal[i] ) )
                                    {
                                       log_debug( "normal gets inverted on collapse, skipping\n" );
                                       canWeld = false;
                                    }
                                 }
                              }
                           }
                        }

                        if ( canWeld )
                        {
                           // Yay! We can collapse v to va (itA)

                           log_debug( "*** vertex %d can be collapsed to %d\n",
                                 v, (*itA).vFar );
                           for ( it = edges.begin(); it != edges.end(); it++ )
                           {
                              // move v to va on each edge, mark flattened triangles
                              for ( int i = 0; i < (*it).polyCount; i++ )
                              {
                                 getTriangleVertices( (*it).poly[i], 
                                       verts[0], verts[1], verts[2] );

                                 log_debug( "finding %d in triangle %d\n", v, (*it).poly[i] );
                                 for ( int n = 0; n < 3; n++ )
                                 {
                                    if ( verts[n] == v )
                                    {
                                       log_debug( " vertex %d\n", n );
                                       verts[n] = (*itA).vFar;
                                       // don't break, we want to check for va also
                                    }
                                    else if ( verts[n] == (*itA).vFar )
                                    {
                                       log_debug( " triangle %d is flattened\n", (*it).poly[i] );
                                       // v and va are now the same
                                       // mark the triangle as flattened
                                       m_triangles[(*it).poly[i]]->m_marked = true;
                                    }
                                 }

                                 setTriangleVertices( (*it).poly[i], 
                                       verts[0], verts[1], verts[2] );
                              }
                           }

                           welded = true;

                           // v is now an orphan, next vertex
                           v++;
                           v = 0; // TODO let's start completely over for now
                           deleteFlattenedTriangles();
                           deleteOrphanedVertices();
                           vcount = m_vertices.size();
                           tcount = m_triangles.size();

                           if ( (*itA).vFar < v )
                           {
                              // The edges connected to va have changed
                              // we must back up to re-check that vertex
                              v = (*itA).vFar;
                           }
                        }
                     }
                  }
               }
            }
         }
      }

      if ( !welded )
      {
         v++;
      }
   }

}

#endif // MM3D_EDIT
