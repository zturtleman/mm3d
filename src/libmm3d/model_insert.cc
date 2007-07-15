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

#ifdef MM3D_EDIT

void Model::insertVertex( unsigned index, Model::Vertex * vertex )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddGeometry;

   invalidateNormals();

   if ( index == m_vertices.size() )
   {
      m_vertices.push_back( vertex );
   }
   else if ( index < m_vertices.size() )
   {
      unsigned count = 0;
      vector<Vertex *>::iterator it;
      for ( it = m_vertices.begin(); it != m_vertices.end(); it++ )
      {
         if ( count == index )
         {
            m_vertices.insert( it, vertex );
            adjustVertexIndices( index, +1 );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "insertVertex( %d ) index out of range\n", index );
   }
}

void Model::removeVertex( unsigned index )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddGeometry;

   invalidateNormals();

   if ( index < m_vertices.size() )
   {
      unsigned count = 0;
      vector<Vertex *>::iterator it;
      for ( it = m_vertices.begin(); it != m_vertices.end(); it++ )
      {
         if ( count == index )
         {
            m_vertices.erase( it );
            adjustVertexIndices( index, -1 );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "removeVertex( %d ) index out of range\n", index );
   }
}

void Model::insertTriangle( unsigned index, Model::Triangle * triangle )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddGeometry;

   invalidateNormals();

   if ( index == m_triangles.size() )
   {
      m_triangles.push_back( triangle );
   }
   else if ( index < m_triangles.size() )
   {
      unsigned count = 0;
      vector<Triangle *>::iterator it;
      for ( it = m_triangles.begin(); it != m_triangles.end(); it++ )
      {
         if ( count == index )
         {
            m_triangles.insert( it, triangle );
            adjustTriangleIndices( index, +1 );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "insertTriangle( %d ) index out of range\n", index );
   }
}

void Model::removeTriangle( unsigned index )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddGeometry;

   invalidateNormals();

   if ( index < m_triangles.size() )
   {
      unsigned count = 0;
      vector<Triangle *>::iterator it;
      for ( it = m_triangles.begin(); it != m_triangles.end(); it++ )
      {
         if ( count == index )
         {
            m_triangles.erase( it );
            adjustTriangleIndices( index, -1 );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "removeTriangle( %d ) index out of range\n", index );
   }
}

void Model::insertGroup( unsigned index, Model::Group * group )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( index == m_groups.size() )
   {
      m_groups.push_back( group );
   }
   else if ( index < m_groups.size() )
   {
      unsigned count = 0;
      vector<Group *>::iterator it;
      for ( it = m_groups.begin(); it != m_groups.end(); it++ )
      {
         if ( count == index )
         {
            m_groups.insert( it, group );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "insertGroup( %d ) index out of range\n", index );
   }
}

void Model::removeGroup( unsigned index )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( index < m_groups.size() )
   {
      unsigned count = 0;
      vector<Group *>::iterator it;
      for ( it = m_groups.begin(); it != m_groups.end(); it++ )
      {
         if ( count == index )
         {
            m_groups.erase( it );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "removeGroup( %d ) index out of range\n", index );
   }
}

void Model::insertBoneJoint( unsigned index, Model::Joint * joint )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( index == m_joints.size() )
   {
      m_joints.push_back( joint );

      // Append keyframe list
      for ( unsigned anim = 0; anim < m_skelAnims.size(); anim++ )
      {
         SkelAnim * sa = m_skelAnims[anim];
         log_debug( "appended keyframe list for joint %d\n", index );
         sa->m_jointKeyframes.push_back( KeyframeList() );
      }
   }
   else if ( index < m_joints.size() )
   {
      // Adjust parent relationships
      unsigned j;
      for ( j = 0; j < m_joints.size(); j++ )
      {
         if ( m_joints[j]->m_parent >= (signed) index )
         {
            m_joints[j]->m_parent++;
         }
      }

      // Adjust joint indices of keyframes after this joint
      for ( unsigned anim = 0; anim < m_skelAnims.size(); anim++ )
      {
         SkelAnim * sa = m_skelAnims[anim];
         for ( j = index; j < sa->m_jointKeyframes.size(); j++ )
         {
            for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
            {
               sa->m_jointKeyframes[j][k]->m_jointIndex++;
            }
         }

         // Insert joint into keyframe list
         if ( index == sa->m_jointKeyframes.size() )
         {
            log_debug( "appended keyframe list for joint %d\n", j );
            sa->m_jointKeyframes.push_back( KeyframeList() );
         }
         else
         {
            JointKeyframeList::iterator it = sa->m_jointKeyframes.begin();
            for ( j = 0; j < sa->m_jointKeyframes.size(); j++ )
            {
               if ( j == index )
               {
                  log_debug( "inserted keyframe list for joint %d\n", j );
                  sa->m_jointKeyframes.insert( it, KeyframeList() );
                  break;
               }
               it++;
            }
         }
      }

      {
         InfluenceList::iterator it;

         // Adjust vertex assignments
         for ( unsigned v = 0; v < m_vertices.size(); v++ )
         {
            InfluenceList & l = m_vertices[v]->m_influences;
            for ( it = l.begin(); it != l.end(); it++ )
            {
               if ( (*it).m_boneId >= (signed) index )
               {
                  (*it).m_boneId++;
               }
            }
         }

         // Adjust point assignments
         for ( unsigned p = 0; p < m_points.size(); p++ )
         {
            InfluenceList & l = m_points[p]->m_influences;
            for ( it = l.begin(); it != l.end(); it++ )
            {
               if ( (*it).m_boneId >= (signed) index )
               {
                  (*it).m_boneId++;
               }
            }
         }
      }

      unsigned count = 0;
      vector<Joint *>::iterator it;
      for ( it = m_joints.begin(); it != m_joints.end(); it++ )
      {
         if ( count == index )
         {
            m_joints.insert( it, joint );
            break;
         }
         count++;
      }

      m_validJoints = false;
   }
   else
   {
      log_error( "insertBoneJoint( %d ) index out of range\n", index );
   }
}

void Model::removeBoneJoint( unsigned joint )
{
   log_debug( "removeBoneJoint( %d )\n", joint );

   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( joint < m_joints.size() )
   {
      // Adjust parent relationships
      unsigned j = 0;

      for ( j = 0; j < m_joints.size(); j++ )
      {
         if ( m_joints[j]->m_parent == (signed) joint )
         {
            m_joints[j]->m_parent = m_joints[joint]->m_parent;
         }
         else if ( m_joints[j]->m_parent > (signed) joint )
         {
            m_joints[j]->m_parent--;
         }
      }

      // Adjust skeletal animations
      unsigned anim = 0;
      for ( anim = 0; anim < m_skelAnims.size(); anim++ )
      {
         SkelAnim * sa = m_skelAnims[anim];
         if ( joint < sa->m_jointKeyframes.size() )
         {
            // Delete joint keyframes
            for ( int index = sa->m_jointKeyframes[joint].size() - 1; index >= 0; index-- )
            {
               log_debug( "deleting keyframe %d for joint %d\n", index, joint );
               deleteSkelAnimKeyframe( anim, sa->m_jointKeyframes[joint][index]->m_frame,
                     joint, sa->m_jointKeyframes[joint][index]->m_isRotation );
            }

            // Remove joint from keyframe list
            JointKeyframeList::iterator it = sa->m_jointKeyframes.begin();
            for ( j = 0; j < sa->m_jointKeyframes.size(); j++ )
            {
               if ( j == joint )
               {
                  log_debug( "removed keyframe list for joint %d\n", j );
                  sa->m_jointKeyframes.erase( it );
                  break;
               }
               it++;
            }
         }
      }

      // Adjust joint indices of keyframes after this joint
      for ( anim = 0; anim < m_skelAnims.size(); anim++ )
      {
         SkelAnim * sa = m_skelAnims[anim];
         for ( j = joint; j < sa->m_jointKeyframes.size(); j++ )
         {
            for ( unsigned index = 0; index < sa->m_jointKeyframes[j].size(); index++ )
            {
               sa->m_jointKeyframes[j][index]->m_jointIndex--;
            }
         }
      }

      {
         InfluenceList::iterator it;

         // Adjust vertex assignments
         for ( unsigned v = 0; v < m_vertices.size(); v++ )
         {
            InfluenceList & l = m_vertices[v]->m_influences;
            for ( it = l.begin(); it != l.end(); it++ )
            {
               if ( (*it).m_boneId > (signed) joint )
               {
                  (*it).m_boneId--;
               }
            }
         }

         // Adjust point assignments
         for ( unsigned p = 0; p < m_points.size(); p++ )
         {
            InfluenceList & l = m_points[p]->m_influences;
            for ( it = l.begin(); it != l.end(); it++ )
            {
               if ( (*it).m_boneId > (signed) joint )
               {
                  (*it).m_boneId--;
               }
            }
         }
      }

      vector<Joint *>::iterator it;
      unsigned count;
      for ( count = 0, it = m_joints.begin(); it != m_joints.end(); it++ )
      {
         if ( count == joint )
         {
            m_joints.erase( it );
            break;
         }
         count++;
      }

      m_validJoints = false;
   }
   else
   {
      log_error( "removeBoneJoint( %d ) index out of range\n", joint );
   }
}

void Model::insertInfluence( const Position & pos, unsigned index, const InfluenceT & influence )
{
   m_changeBits |= AddOther;

   InfluenceList * l = NULL;
   if ( pos.type == PT_Vertex )
   {
      if ( pos.index < m_vertices.size() )
      {
         l = &m_vertices[ pos.index ]->m_influences;
      }
   }
   else if ( pos.type == PT_Point )
   {
      if ( pos.index < m_points.size() )
      {
         l = &m_points[ pos.index ]->m_influences;
      }
   }

   if ( l == NULL )
   {
      return;
   }

   InfluenceList::iterator it = l->begin();

   while ( index > 0 )
   {
      index--;
      it++;
   }

   if ( it == l->end() )
   {
      l->push_back( influence );
   }
   else
   {
      l->insert( it, influence );
   }
}

void Model::removeInfluence( const Position & pos, unsigned index )
{
   m_changeBits |= AddOther;

   InfluenceList * l = NULL;
   if ( pos.type == PT_Vertex )
   {
      if ( pos.index < m_vertices.size() )
      {
         l = &m_vertices[ pos.index ]->m_influences;
      }
   }
   else if ( pos.type == PT_Point )
   {
      if ( pos.index < m_points.size() )
      {
         l = &m_points[ pos.index ]->m_influences;
      }
   }

   if ( l == NULL )
   {
      return;
   }

   InfluenceList::iterator it = l->begin();

   while ( index > 0 )
   {
      index--;
      it++;
   }

   if ( it != l->end() )
   {
      l->erase( it );
   }
}

void Model::insertPoint( unsigned index, Model::Point * point )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( index == m_points.size() )
   {
      m_points.push_back( point );
   }
   else if ( index < m_points.size() )
   {
      unsigned count = 0;
      vector<Point *>::iterator it;
      for ( it = m_points.begin(); it != m_points.end(); it++ )
      {
         if ( count == index )
         {
            m_points.insert( it, point );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "insertPoint( %d ) index out of range\n", index );
   }
}

void Model::removePoint( unsigned point )
{
   log_debug( "removePoint( %d )\n", point );

   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( point < m_points.size() )
   {
      vector<Point *>::iterator it;
      unsigned count;
      for ( count = 0, it = m_points.begin(); it != m_points.end(); it++ )
      {
         if ( count == point )
         {
            m_points.erase( it );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "removePoint( %d ) index out of range\n", point );
   }
}

void Model::insertProjection( unsigned index, Model::TextureProjection * proj )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( index == m_projections.size() )
   {
      m_projections.push_back( proj );
   }
   else if ( index < m_projections.size() )
   {
      unsigned count = 0;
      vector<TextureProjection *>::iterator it;
      for ( it = m_projections.begin(); it != m_projections.end(); it++ )
      {
         if ( count == index )
         {
            m_projections.insert( it, proj );
            adjustProjectionIndices( index, +1 );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "insertProjection( %d ) index out of range\n", index );
   }
}

void Model::removeProjection( unsigned proj )
{
   log_debug( "removeProjection( %d )\n", proj );

   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( proj < m_projections.size() )
   {
      vector<TextureProjection *>::iterator it;
      unsigned count;
      for ( count = 0, it = m_projections.begin(); it != m_projections.end(); it++ )
      {
         if ( count == proj )
         {
            m_projections.erase( it );
            adjustProjectionIndices( proj, -1 );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "removeProjection( %d ) index out of range\n", proj );
   }
}

void Model::insertTexture( unsigned index, Model::Material * texture )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( index == m_materials.size() )
   {
      m_materials.push_back( texture );
      invalidateTextures();
   }
   else if ( index < m_materials.size() )
   {
      unsigned count = 0;
      vector<Material *>::iterator it;
      for ( it = m_materials.begin(); it != m_materials.end(); it++ )
      {
         if ( count == index )
         {
            m_materials.insert( it, texture );
            invalidateTextures();
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "insertTexture( %d ) index out of range\n", index );
   }
}

void Model::removeTexture( unsigned index )
{
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( index < m_materials.size() )
   {
      unsigned count = 0;
      vector<Material *>::iterator it;
      for ( it = m_materials.begin(); it != m_materials.end(); it++ )
      {
         if ( count == index )
         {
            m_materials.erase( it );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "removeTexture( %d ) index out of range\n", index );
   }
}

void Model::adjustVertexIndices( unsigned index, int count )
{
   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      for ( unsigned v = 0; v < 3; v++ )
      {
         if ( m_triangles[t]->m_vertexIndices[v] >= index )
         {
            m_triangles[t]->m_vertexIndices[v] += count;
         }
      }
   }
}

void Model::adjustTriangleIndices( unsigned index, int count )
{
   for ( unsigned g = 0; g < m_groups.size(); g++ )
   {
      vector<int>::iterator it;
      it = m_groups[g]->m_triangleIndices.begin();
      while ( it != m_groups[g]->m_triangleIndices.end() )
      {
         /*
         if ( (unsigned) (*it) == index )
         {
            m_groups[g]->m_triangleIndices.erase( it );
         }
         else
         */
         if ( (unsigned) *it >= index )
         {
            (*it) += count;
            it++;
         }
         else
         {
            it++;
         }
      }
   }
}

void Model::adjustProjectionIndices( unsigned index, int count )
{
   for ( unsigned t = 0; t < m_triangles.size(); t++ )
   {
      if ( m_triangles[t]->m_projection >= (int) index )
      {
         m_triangles[t]->m_projection += count;
      }
   }
}

#endif // MM3D_EDIT

