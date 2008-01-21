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

int Model::equivalent( const Model * model, int compareMask, double tolerance )
{
   int matchVal = 0;

   unsigned numVertices    = m_vertices.size();
   unsigned numTriangles   = m_triangles.size();
   unsigned numGroups      = m_groups.size();
   unsigned numJoints      = m_joints.size();
   unsigned numPoints      = m_points.size();
   unsigned numTextures    = m_materials.size();
   unsigned numFrameAnims  = m_frameAnims.size();
   unsigned numSkelAnims   = m_skelAnims.size();

   unsigned t = 0;
   unsigned v = 0;
   unsigned p = 0;

   if ( compareMask & CompareGeometry )
   {
      if ( numVertices == model->m_vertices.size() && numTriangles == model->m_triangles.size() )
      {
         bool match = true;
         for ( v = 0; match && v < numVertices; v++ )
         {
            for ( unsigned i = 0; i < 3; i++ )
            {
               if ( fabs( m_vertices[v]->m_coord[i] - model->m_vertices[v]->m_coord[i]) > tolerance )
               {
                  match = false;
                  log_debug( "match failed at vertex coordinates %d[%d]\n", v, i );
               }
            }
         }

         for ( t = 0; match && t < numTriangles; t++ )
         {
            if (  m_triangles[t]->m_vertexIndices[0] != model->m_triangles[t]->m_vertexIndices[0]
               || m_triangles[t]->m_vertexIndices[1] != model->m_triangles[t]->m_vertexIndices[1]
               || m_triangles[t]->m_vertexIndices[2] != model->m_triangles[t]->m_vertexIndices[2] )
            {
               match = false;
               log_debug( "match failed at triangle %d\n", t );
               log_debug( "   %d %d %d\n", 
                     m_triangles[t]->m_vertexIndices[0], 
                     m_triangles[t]->m_vertexIndices[1], 
                     m_triangles[t]->m_vertexIndices[2] );
               log_debug( "   %d %d %d\n", 
                     model->m_triangles[t]->m_vertexIndices[0], 
                     model->m_triangles[t]->m_vertexIndices[1], 
                     model->m_triangles[t]->m_vertexIndices[2] );
            }
         }

         if ( match )
         {
            matchVal |= CompareGeometry;
            matchVal |= CompareFaces;
         }
      }
   }
   else if ( compareMask & CompareFaces )
   {
      if ( numTriangles == model->m_triangles.size() )
      {
         for ( t = 0; t < numTriangles; t++ )
         {
            m_triangles[t]->m_marked = false;
            model->m_triangles[t]->m_marked = false;
         }

         for ( t = 0; t < numTriangles; t++ )
         {
            for ( unsigned i = 0; i < model->m_triangles.size(); i++ )
            {
               if ( !model->m_triangles[i]->m_marked )
               {
                  bool match = true;
                  for ( unsigned j = 0; j < 3; j++ )  // triangle vertex
                  {
                     Vertex * v1 = m_vertices[ m_triangles[t]->m_vertexIndices[j] ];
                     Vertex * v2 = model->m_vertices[ model->m_triangles[i]->m_vertexIndices[j] ];
                     for ( unsigned k = 0; k < 3; k++ ) // vertex coord
                     {
                        if (   fabs( v1->m_coord[k] - v2->m_coord[k] ) > tolerance )
                        {
                           match = false;
                        }
                     }
                  }
                  if ( match )
                  {
                     m_triangles[t]->m_marked = true;
                     model->m_triangles[i]->m_marked = true;
                     break; // break inner loop
                  }
               }
            }
         }

         bool match = true;
         for ( t = 0; t < numTriangles; t++ )
         {
            if ( !m_triangles[t]->m_marked )
            {
               match = false;
               log_debug( "match failed at triangle %d, no match for face\n", t );
            }
         }
         if ( match )
         {
            matchVal |= CompareFaces;
         }
      }
   }

   if ( compareMask & CompareGroups )
   {
      std::list<int> l1;
      std::list<int> l2;
      std::list<int>::iterator it1;
      std::list<int>::iterator it2;

      bool match = false;

      if ( getGroupCount() == model->getGroupCount() )
      {
         unsigned int t = 0;
         unsigned int gcount = getGroupCount();
         unsigned int tcount = getTriangleCount();
         match = true;

         for ( unsigned int g = 0; match && g < gcount; g++ )
         {
            for ( t = 0; t < tcount; t++ )
            {
               m_triangles[t]->m_marked = false;
            }

            std::list<int> l1 = getGroupTriangles( g );
            std::list<int> l2 = model->getGroupTriangles( g );

            if ( l1.size() == l2.size() )
            {
               for ( it1  = l1.begin(); match && it1 != l1.end(); it1++ )
               {
                  if ( m_triangles[ *it1 ]->m_marked )
                  {
                     log_debug( "match failed in groups, triangle %d is in group %d twice\n", (*it1), g );
                  }
                  else
                  {
                     m_triangles[ *it1 ]->m_marked = true;
                  }
               }
               for ( it2  = l2.begin(); match && it2 != l2.end(); it2++ )
               {
                  if ( !m_triangles[ *it2 ]->m_marked )
                  {
                     log_debug( "match failed in groups, triangle %d is in group %d for rhs but not lhs\n", (*it2), g );
                  }
                  else
                  {
                     m_triangles[ *it2 ]->m_marked = false;
                  }
               }
            }
            else
            {
               log_debug( "match failed in groups, triangle count %d != %d for group %d\n", l1.size(), l2.size(), g );
               match = false;
            }
         }
      }

      if ( match )
      {
         matchVal |= CompareGroups;
      }
   }

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
               log_debug( "match failed at joint parent for joint %d\n", j );
            }
            for ( unsigned i = 0; i < 3; i++ )
            {
               if ( fabs( m_joints[j]->m_localRotation[i] - model->m_joints[j]->m_localRotation[i] ) > tolerance )
               {
                  log_debug( "match failed at joint rotation for joint %d\n", j );
                  match = false;
               }
               if ( fabs( m_joints[j]->m_localTranslation[i] - model->m_joints[j]->m_localTranslation[i] ) > tolerance )
               {
                  log_debug( "match failed at joint translation for joint %d\n", j );
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
                     log_debug( "match failed at vertex influence joint for vertex %d\n", v );
                     match = false;
                  }
                  if ( (*ita).m_type != (*itb).m_type )
                  {
                     log_debug( "match failed at vertex influence type for vertex %d\n", v );
                     match = false;
                  }
                  if ( !_float_equiv( (*ita).m_weight, (*itb).m_weight, tolerance ) )
                  {
                     log_debug( "match failed at vertex influence weight for vertex %d\n", v );
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
                     log_debug( "match failed at point influence joint for point %d\n", p );
                     match = false;
                  }
                  if ( (*ita).m_type != (*itb).m_type )
                  {
                     log_debug( "match failed at point influence type for point %d\n", p );
                     match = false;
                  }
                  if ( !_float_equiv( (*ita).m_weight, (*itb).m_weight, tolerance ) )
                  {
                     log_debug( "match failed at point influence weight for point %d\n", p );
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
               log_debug( "match failed at point type for point %d\n", p );
            }
            for ( unsigned i = 0; i < 3; i++ )
            {
               if ( fabs( m_points[p]->m_rot[i] - model->m_points[p]->m_rot[i] ) > tolerance )
               {
                  log_debug( "match failed at point rotation for point %d\n", p );
                  match = false;
               }
               if ( fabs( m_points[p]->m_trans[i] - model->m_points[p]->m_trans[i] ) > tolerance )
               {
                  log_debug( "match failed at point translation for point %d\n", p );
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

   // TODO want to check texture contents?
   if ( compareMask & CompareMaterials )
   {
      if (     numTextures  == model->m_materials.size() 
            && numTriangles == model->m_triangles.size() 
            && numGroups    == model->m_groups.size()     )
      {
         bool match = true;

         for ( t = 0; t < numTextures; t++ )
         {
            for ( unsigned i = 0; i < 4; i++ )
            {
               if ( fabs( m_materials[t]->m_ambient[i] - model->m_materials[t]->m_ambient[i] ) > tolerance )
               {
                  log_debug( "match failed at texture ambient for texture %d\n", t );
                  match = false;
               }
               if ( fabs( m_materials[t]->m_diffuse[i] - model->m_materials[t]->m_diffuse[i] ) > tolerance )
               {
                  log_debug( "match failed at texture diffuse for texture %d\n", t );
                  match = false;
               }
               if ( fabs( m_materials[t]->m_specular[i] - model->m_materials[t]->m_specular[i] ) > tolerance )
               {
                  log_debug( "match failed at texture specular for texture %d\n", t );
                  match = false;
               }
               if ( fabs( m_materials[t]->m_emissive[i] - model->m_materials[t]->m_emissive[i] ) > tolerance )
               {
                  log_debug( "match failed at texture emissive for texture %d\n", t );
                  match = false;
               }

            }

            if ( m_materials[t]->m_shininess != model->m_materials[t]->m_shininess )
            {
               log_debug( "match failed at texture shininess for texture %d\n", t );
               match = false;
            }
         }

         for ( t = 0; match && t < numTriangles; t++ )
         {
            for ( unsigned i = 0; i < 3; i++ )
            {
               if ( fabs( m_triangles[t]->m_s[i] - model->m_triangles[t]->m_s[i] ) > tolerance )
               {
                  log_debug( "match failed at texture coordinates for triangle %d\n", t );
                  match = false;
               }
            }
         }

         for ( unsigned g = 0; match && g < numGroups; g++ )
         {
            if ( m_groups[g]->m_smooth != model->m_groups[g]->m_smooth )
            {
               log_debug( "match failed at group smoothness group %d\n", g );
               match = false;
            }
            if ( m_groups[g]->m_materialIndex != model->m_groups[g]->m_materialIndex )
            {
               log_debug( "match failed at group texture for group %d\n", g );
               match = false;
            }

            unsigned numGroupTriangles = m_groups[g]->m_triangleIndices.size();
            if ( numGroupTriangles == model->m_groups[g]->m_triangleIndices.size() )
            {
               for ( t = 0; match && t < numGroupTriangles; t++ )
               {
                  if ( m_groups[g]->m_triangleIndices[t] != model->m_groups[g]->m_triangleIndices[t] )
                  {
                     log_debug( "match failed at group triangle assignments for group %d triangle %d\n", g, t );
                     match = false;
                  }
               }
            }
            else
            {
               log_debug( "match failed at group triangle count for group %d\n", g );
               match = false;
            }
         }

         if ( match )
         {
            matchVal |= CompareMaterials;
         }
      }
      else
      {
         log_debug( "not comparing because of primitive mismatch\n" );
         log_debug( " (%d/%d) (%d/%d) (%d/%d)\n",
               numTextures,  model->m_materials.size(),
               numTriangles, model->m_triangles.size(), 
               numGroups,    model->m_groups.size() );
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
               log_debug( "match failed at frame animation frame count for frame anim %d\n", t );
               match = false;
            }
            if ( fabs( m_frameAnims[t]->m_fps - model->m_frameAnims[t]->m_fps ) > tolerance )
            {
               log_debug( "match failed at frame animation fps for frame anim %d\n", t );
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
                        log_debug( "match failed at frame animation coords for frame anim %d, vertex %d\n", t, v );
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
                        log_debug( "match failed at frame animation coords for frame anim %d, point %d\n", t, p );
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
               log_debug( "match failed at skel animation frame count for skel anim %d\n", t );
               match = false;
            }
            if ( fabs( m_skelAnims[t]->m_fps - model->m_skelAnims[t]->m_fps ) > tolerance )
            {
               log_debug( "match failed at skel animation fps for skel anim %d\n", t );
               match = false;
            }

            for ( unsigned j = 0; j < m_skelAnims[t]->m_jointKeyframes.size(); j++ )
            {
               for ( unsigned i = 0; i < m_skelAnims[t]->m_jointKeyframes[j].size(); i++ )
               {
                  if (    m_skelAnims[t]->m_jointKeyframes[j][i]->m_isRotation 
                       != model->m_skelAnims[t]->m_jointKeyframes[j][i]->m_isRotation )
                  {
                     log_debug( "match failed at skel animation keyframe rotation for skel anim %d joint %d, keyframe %i\n", t, j, i );
                     match = false;
                  }
                  if (   fabs( m_skelAnims[t]->m_jointKeyframes[j][i]->m_time 
                       - model->m_skelAnims[t]->m_jointKeyframes[j][i]->m_time ) > tolerance )
                  {
                     log_debug( "match failed at skel animation keyframe time for skel anim %d joint %d, keyframe %i\n", t, j, i );
                     match = false;
                  }
                  for ( unsigned n = 0; n < 3; n++ )
                  {
                     if (   fabs( m_skelAnims[t]->m_jointKeyframes[j][i]->m_parameter[n] 
                              - model->m_skelAnims[t]->m_jointKeyframes[j][i]->m_parameter[n] ) > tolerance )
                     {
                        log_debug( "match failed at skel animation keyframe parameter for skel anim %d joint %d, keyframe %i\n", t, j, i );
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

   if ( compareMask & CompareMeta )
   {
      if ( numGroups == model->m_groups.size()
            && numTextures == model->m_materials.size()
            && numJoints == model->m_joints.size()
            && numSkelAnims == model->m_skelAnims.size()
            && numFrameAnims == model->m_frameAnims.size() )
      {
         bool match = true;

         for ( t = 0; match && t < numGroups; t++ )
         {
            if ( strcmp( m_groups[t]->m_name.c_str(), model->m_groups[t]->m_name.c_str() ) != 0 )
            {
               match = false;
            }
         }

         for ( t = 0; match && t < numTextures; t++ )
         {
            if ( strcmp( m_materials[t]->m_name.c_str(), model->m_materials[t]->m_name.c_str() ) != 0 )
            {
               match = false;
            }
            if ( strcmp( m_materials[t]->m_filename.c_str(), model->m_materials[t]->m_filename.c_str() ) != 0 )
            {
               match = false;
            }
            if ( strcmp( m_materials[t]->m_alphaFilename.c_str(), model->m_materials[t]->m_alphaFilename.c_str() ) != 0 )
            {
               match = false;
            }
         }

         for ( t = 0; match && t < numJoints; t++ )
         {
            if ( strcmp( m_joints[t]->m_name.c_str(), model->m_joints[t]->m_name.c_str() ) != 0 )
            {
               match = false;
            }
         }

         for ( t = 0; match && t < numPoints; t++ )
         {
            if ( strcmp( m_points[t]->m_name.c_str(), model->m_points[t]->m_name.c_str() ) != 0 )
            {
               match = false;
            }
         }

         for ( t = 0; match && t < numSkelAnims; t++ )
         {
            if ( strcmp( m_skelAnims[t]->m_name.c_str(), model->m_skelAnims[t]->m_name.c_str() ) != 0 )
            {
               match = false;
            }
         }

         for ( t = 0; match && t < numFrameAnims; t++ )
         {
            if ( strcmp( m_frameAnims[t]->m_name.c_str(), model->m_frameAnims[t]->m_name.c_str() ) != 0 )
            {
               match = false;
            }
         }

         if ( match )
         {
            matchVal |= CompareMeta;
         }
      }
   }

   matchVal &= compareMask;
   return matchVal;
}

int Model::equal( const Model * model, int compareMask, double tolerance )
{
   int matchVal = compareMask;

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

   bool match = false;

   if ( (matchVal & (CompareGeometry | CompareMeta | CompareInfluences))
      && numVertices == model->m_vertices.size() )
   {
      match = true;

      for ( v = 0; match && v < numVertices; v++ )
      {
         if ( !m_vertices[v]->equal( *model->m_vertices[v], compareMask ) )
         {
            log_debug( "match failed at vertex %d\n", v );
            match = false;
         }
      }
   }

   if ( !match )
      matchVal &= ~(CompareGeometry | CompareMeta | CompareInfluences);

   match = false;

   if ( (matchVal & (CompareGeometry | CompareFaces | CompareTextures | CompareMeta))
         && numTriangles == model->m_triangles.size() )
   {
      match = true;

      for ( t = 0; t < numTriangles; t++ )
      {
         if ( !m_triangles[t]->equal( *model->m_triangles[t], compareMask ) )
         {
            log_debug( "match failed at triangle %d\n", t );
            match = false;
         }
      }
   }

   if ( !match )
      matchVal &= ~(CompareGeometry | CompareFaces | CompareMeta | CompareTextures );

   match = false;

   if ( (compareMask & (CompareGroups | CompareGeometry | CompareMaterials | CompareMeta))
         && getGroupCount() == model->getGroupCount() )
   {
      match = true;

      for ( unsigned int g = 0; match && g < numGroups; g++ )
      {
         if ( !m_groups[g]->equal( *model->m_groups[g], compareMask ) )
         {
            match = false;
            log_debug( "match failed at group %d\n", g );
         }
      }
   }

   if ( !match )
      matchVal &= ~(CompareGroups | CompareGeometry | CompareMaterials | CompareMeta);

   match = false;

   if ( (matchVal & (CompareSkeleton | CompareMeta))
         && numJoints == model->m_joints.size() )
   {
      match = true;

      for ( unsigned j = 0; match && j < numJoints; j++ )
      {
         if ( !m_joints[j]->equal( *model->m_joints[j], compareMask ) )
         {
            log_debug( "match failed at joint %d\n", j );
            match = false;
         }
      }
   }

   if ( !match )
      matchVal &= ~(CompareSkeleton | CompareMeta);

   match = false;

   if ( (matchVal & (ComparePoints | CompareInfluences | CompareMeta))
         && numPoints == model->m_points.size() )
   {
      match = true;

      for ( unsigned p = 0; match && p < numPoints; p++ )
      {
         if ( !m_points[p]->equal( *model->m_points[p], compareMask ) )
         {
            match = false;
            log_debug( "match failed at point %d\n", p );
         }
      }
   }

   if ( !match )
      matchVal &= ~(ComparePoints | CompareMeta | CompareInfluences);

   match = false;

   if ( (matchVal & (CompareMaterials | CompareTextures))
         && numTextures  == model->m_materials.size() 
         && numProjections == model->m_projections.size() )
   {
      match = true;

      for ( t = 0; t < numTextures; t++ )
      {
         if ( !m_materials[t]->equal( *model->m_materials[t], compareMask ) )
         {
            log_debug( "match failed at material %d\n", t );
            match = false;
         }
      }

      if ( matchVal & CompareTextures )
      {
         for ( t = 0; t < numProjections; t++ )
         {
            if ( !m_projections[t]->equal( *model->m_projections[t], compareMask ) )
            {
               log_debug( "match failed at projection %d\n", t );
               match = false;
            }
         }
      }
   }

   if ( !match )
      matchVal &= ~(CompareMaterials | CompareTextures);

   match = false;

   if ( (matchVal & (CompareAnimData | CompareAnimSets))
         && numFrameAnims == model->m_frameAnims.size()
         && numSkelAnims == model->m_skelAnims.size()    )
   {
      match = true;

      for ( t = 0; match && t < numSkelAnims; t++ )
      {
         if ( !m_skelAnims[t]->equal( *model->m_skelAnims[t], compareMask ) )
         {
            log_debug( "match failed at skel animation %d\n", t );
            match = false;
         }
      }

      for ( t = 0; match && t < numFrameAnims; t++ )
      {
         if ( !m_frameAnims[t]->equal( *model->m_frameAnims[t], compareMask ) )
         {
            log_debug( "match failed at frame animation %d\n", t );
            match = false;
         }
      }
   }

   if ( !match )
      matchVal &= ~(CompareAnimData | CompareAnimSets);

   match = false;

   if (matchVal & CompareMeta
         && getMetaDataCount() == model->getMetaDataCount() )
   {
      match = true;

      unsigned int mcount = getMetaDataCount();
      for ( unsigned int m = 0; m < mcount; ++m )
      {
         char key[1024];
         char value_lhs[1024];
         char value_rhs[1024];

         getMetaData( m, key, sizeof(key), value_lhs, sizeof(value_lhs) );

         if ( !model->getMetaData( m, key, sizeof(key), value_rhs, sizeof(value_rhs) ) )
         {
            log_debug( "missing meta data key: '%s'\n", key );
            match = false;
         }
         else
         {
            if ( strcmp( value_lhs, value_rhs ) != 0 )
            {
               log_debug( "meta data value mismatch for '%s'\n", key );
               match = false;
            }
         }
      }

      for ( unsigned int b = 0; b < MAX_BACKGROUND_IMAGES; ++b )
      {
         if ( !m_background[b]->equal( *model->m_background[b], compareMask ) )
         {
            log_debug( "match failed at background image %d\n", t );
            match = false;
         }
      }
   }

   if ( !match )
      matchVal &= ~CompareMeta;

   return matchVal;
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
