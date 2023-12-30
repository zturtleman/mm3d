/*  Maverick Model 3D
 * 
 *  Copyright (c) 2004-2008 Kevin Worcester
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

#include "glmath.h"
#include "log.h"
#include "texmgr.h"

#include <algorithm>
#include <unordered_map>

using std::unordered_map;


static bool isValueInList( const std::list<int> & list, int value )
{
   return std::find( list.begin(), list.end(), value ) != list.end();
}

Model * Model::copySpecified( int rootPoint, const std::list<int> & tri,
   const std::list<int> & vert, const std::list<int> & groups,
   const std::list<int> & materials, const std::list<int> & joints,
   const std::list<int> & points, const std::list<int> & proj,
   const std::list<int> & skelAnims, const std::list<int> & frameAnims ) const
{
   Model * m = new Model;

   unordered_map<int,int> projMap;
   unordered_map<int,int> vertMap;
   unordered_map<int,int> triMap;
   unordered_map<int,int> jointMap;
   unordered_map<int,int> pointMap;
   unordered_map<int,int> groupMap;
   unordered_map<int,int> materialMap;

   list<int>::const_iterator lit;

   Matrix rootMatrix;

   if ( rootPoint >= 0 )
   {
      getPointAbsoluteMatrix( rootPoint, rootMatrix );
      rootMatrix = rootMatrix.getInverse();

      Matrix up;
      up.setRotationInDegrees( -90, -90, 0 );
      rootMatrix = rootMatrix * up;
   }

   if ( !proj.empty() )
   {
      for ( lit = proj.begin(); lit != proj.end(); ++lit )
      {
         const char * name = getProjectionName( *lit );
         int type = getProjectionType( *lit );

         double pos[3] = { 0, 0, 0 };
         double up[3] = { 0, 0, 0 };
         double seam[3] = { 0, 0, 0 };
         double range[2][2] = { { 0, 0 }, { 0, 0 } };

         getProjectionCoords( *lit, pos );
         getProjectionUp( *lit, up );
         getProjectionSeam( *lit, seam );
         getProjectionRange( *lit,
               range[0][0], range[0][1], range[1][0], range[1][1] );

         int np = m->addProjection( name, type, pos[0], pos[1], pos[2] );
         m->setProjectionUp( np, up );
         m->setProjectionSeam( np, seam );
         m->setProjectionRange( np,
               range[0][0], range[0][1], range[1][0], range[1][1] );

         if ( this->isProjectionSelected( *lit ) )
         {
            m->selectProjection( np );
         }

         projMap[ *lit ] = np;
      }
   }

   if ( !tri.empty() )
   {
      // Copy vertices
      log_debug( "Copying %" PORTuSIZE " vertices\n", vert.size() );
      for ( lit = vert.begin(); lit != vert.end(); lit++ )
      {
         double coords[4] = { 0,0,0,1 };
         this->getVertexCoords( *lit, coords );

         rootMatrix.apply( coords );

         int nv = m->addVertex( coords[0], coords[1], coords[2] );
         m->setVertexFree( nv, this->isVertexFree(*lit) );

         if ( this->isVertexSelected( *lit ) )
         {
            m->selectVertex( nv );
         }

         vertMap[ *lit ] = nv;
      }

      // Copy faces
      log_debug( "Copying %" PORTuSIZE " faces\n", tri.size() );
      for ( lit = tri.begin(); lit != tri.end(); lit++ )
      {
         unsigned v[3];

         for ( int t = 0; t < 3; t++ )
         {
            v[t] = this->getTriangleVertex( *lit, t );
         }
         int nt = m->addTriangle( vertMap[v[0]] , vertMap[v[1]], vertMap[v[2]] );
         if ( this->isTriangleSelected( *lit ) )
         {
            m->selectTriangle( nt );
         }

         triMap[ *lit ] = nt;
      }

      // Copy texture coords
      log_debug( "Copying %" PORTuSIZE " face texture coordinates\n", tri.size() );
      for ( lit = tri.begin(); lit != tri.end(); lit++ )
      {
         float s;
         float t;

         for ( unsigned i = 0; i < 3; i++ )
         {
            this->getTextureCoords( (unsigned) *lit, i, s, t );
            m->setTextureCoords( (unsigned) triMap[ *lit ], i, s, t );
         }
      }

      // Copy textures
      log_debug( "Copying %" PORTuSIZE " textures\n", materials.size() );
      for ( lit = materials.begin(); lit != materials.end(); lit++ )
      {
         int newMaterial;

         switch ( this->getMaterialType( *lit ) )
         {
            case Model::Material::MATTYPE_TEXTURE:
               {
                  Texture * tex = TextureManager::getInstance()->getTexture( this->getTextureFilename( *lit ) );
                  newMaterial = m->addTexture( tex );
                  m->setTextureName( newMaterial, getTextureName( *lit ) );
               }
               break;
            default:
               log_error( "Unknown material type %d in duplicate\n", this->getMaterialType( *lit ) );
               /* fall-through */
            case Model::Material::MATTYPE_BLANK:
               newMaterial = m->addColorMaterial( this->getTextureName( *lit ) );
               break;
         }

         materialMap[*lit] = newMaterial;

         float c[4];
         this->getTextureAmbient( *lit, c );
         m->setTextureAmbient( newMaterial, c );
         this->getTextureDiffuse( *lit, c );
         m->setTextureDiffuse( newMaterial, c );
         this->getTextureSpecular( *lit, c );
         m->setTextureSpecular( newMaterial, c );
         this->getTextureEmissive( *lit, c );
         m->setTextureEmissive( newMaterial, c );
         this->getTextureShininess( *lit, c[0] );
         m->setTextureShininess( newMaterial, c[0] );

         // TODO Material m_color (if ever used)

         m->setTextureSClamp( newMaterial, this->getTextureSClamp( *lit ) );
         m->setTextureTClamp( newMaterial, this->getTextureTClamp( *lit ) );
      }

      // Copy groups
      log_debug( "Copying %" PORTuSIZE " groups\n", groups.size() );
      for ( lit = groups.begin(); lit != groups.end(); lit++ )
      {
         int ng = m->addGroup( this->getGroupName( *lit ) );
         groupMap[ *lit ] = ng;

         m->setGroupSmooth( ng, this->getGroupSmooth(*lit) );
         m->setGroupAngle( ng, this->getGroupAngle(*lit) );

         int materialId = this->getGroupTextureId(*lit);
         if ( isValueInList( materials, materialId ) )
         {
            m->setGroupTextureId( ng, materialMap[materialId] );
         }
      }

      if ( groups.size() > 0 )
      {
         // Set groups
         log_debug( "Setting %" PORTuSIZE " triangle groups\n", tri.size() );
         for ( lit = tri.begin(); lit != tri.end(); lit++ )
         {
            // This works, even if triangle group == -1
            int gid = this->getTriangleGroup(*lit);
            if ( gid >= 0 && isValueInList( groups, gid ) )
            {
               m->addTriangleToGroup( groupMap[ gid ], triMap[*lit] );
            }
         }
      }

   }

   if ( !points.empty() )
   {
      // Copy points
      log_debug( "Copying %" PORTuSIZE " points\n", points.size() );
      for ( lit = points.begin(); lit != points.end(); lit++ )
      {
         Matrix pointMatrix;
         double coord[3];
         double rot[3];
         this->getPointAbsoluteMatrix( *lit, pointMatrix );
         pointMatrix = pointMatrix * rootMatrix;
         pointMatrix.getTranslation( coord );
         pointMatrix.getRotation( rot );

         // TODO point type (if it is ever used)
         int np = m->addPoint( this->getPointName( *lit ),
               coord[0], coord[1], coord[2], rot[0], rot[1], rot[2], -1 );
         pointMap[ *lit ] = np;

         if ( this->isPointSelected( *lit ) )
         {
            m->selectPoint( np );
         }
      }
   }

   if ( !joints.empty() )
   {
      // Copy joints
      log_debug( "Copying %" PORTuSIZE " joints\n", joints.size() );
      for ( lit = joints.begin(); lit != joints.end(); lit++ )
      {
         int parent = this->getBoneJointParent( *lit );

         // TODO this will not work if parent joint comes after child
         // joint.  That shouldn't happen... but...
         if ( isValueInList( joints, parent ) )
         {
            parent = jointMap[ parent ];
         }
         else
         {
            if ( m->getBoneJointCount() > 0 )
               parent = 0;
            else
               parent = -1;
         }

         Matrix jointMatrix;
         double coord[3];
         double rot[3];
         this->getBoneJointAbsoluteMatrix( *lit, jointMatrix );
         jointMatrix = jointMatrix * rootMatrix;
         jointMatrix.getTranslation( coord );
         jointMatrix.getRotation( rot );

         int nj = m->addBoneJoint( this->getBoneJointName( *lit ),
               coord[0], coord[1], coord[2], rot[0], rot[1], rot[2], parent );
         jointMap[ *lit ] = nj;

         if ( this->isBoneJointSelected( *lit ) )
         {
            m->selectBoneJoint( nj );
         }
      }

      for ( unordered_map<int,int>::iterator it = vertMap.begin();
            it != vertMap.end(); ++it )
      {
         InfluenceList il;

         getVertexInfluences( it->first, il );

         for ( InfluenceList::iterator iit = il.begin(); iit != il.end(); ++iit )
         {
            if ( isValueInList( joints, iit->m_boneId ) )
            {
               m->addVertexInfluence( it->second, jointMap[iit->m_boneId],
                     iit->m_type, iit->m_weight );
            }
         }
      }

      for ( unordered_map<int,int>::iterator it = pointMap.begin();
            it != pointMap.end(); ++it )
      {
         InfluenceList il;

         getPointInfluences( it->first, il );

         for ( InfluenceList::iterator iit = il.begin(); iit != il.end(); ++iit )
         {
            if ( isValueInList( joints, iit->m_boneId ) )
            {
               m->addPointInfluence( it->second, jointMap[iit->m_boneId],
                     iit->m_type, iit->m_weight );
            }
         }
      }
   }

   if ( !skelAnims.empty() && !joints.empty() )
   {
      log_debug( "Copying %" PORTuSIZE " skeletal animations\n", skelAnims.size() );
      for ( lit = skelAnims.begin(); lit != skelAnims.end(); lit++ )
      {
         int num = m->addAnimation( ANIMMODE_SKELETAL, this->getAnimName( ANIMMODE_SKELETAL, *lit ) );
         if ( num >= 0 )
         {
            m->setAnimFrameCount( ANIMMODE_SKELETAL, num, this->getAnimFrameCount( ANIMMODE_SKELETAL, *lit ) );
            m->setAnimFPS( ANIMMODE_SKELETAL, num, this->getAnimFPS( ANIMMODE_SKELETAL, *lit ) );
            m->setAnimLooping( ANIMMODE_SKELETAL, num, this->getAnimLooping( ANIMMODE_SKELETAL, *lit ) );

            SkelAnim * sa = this->m_skelAnims[*lit];

            for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
            {
               if ( !isValueInList( joints, j ) )
                  continue;

               // FIXME: should only skip control bone(s) for rootPoint.
               if ( rootPoint != -1 && m->m_joints[ jointMap[j] ]->m_parent == -1 )
               {
                  continue;
               }

               for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
               {
                  Keyframe * kf = sa->m_jointKeyframes[j][k];

                  m->setSkelAnimKeyframe( num, kf->m_frame, jointMap[j], kf->m_isRotation,
                        kf->m_parameter[0], kf->m_parameter[1], kf->m_parameter[2] );
               }
            }
         }
      }
   }

   if ( !frameAnims.empty() && !tri.empty() )
   {
      log_debug( "Copying %" PORTuSIZE " frame animations\n", frameAnims.size() );
      for ( lit = frameAnims.begin(); lit != frameAnims.end(); lit++ )
      {
         int num = m->addAnimation( ANIMMODE_FRAME, this->getAnimName( ANIMMODE_FRAME, *lit ) );
         if ( num >= 0 )
         {
            m->setAnimFrameCount( ANIMMODE_FRAME, num, this->getAnimFrameCount( ANIMMODE_FRAME, *lit ) );
            m->setAnimFPS( ANIMMODE_FRAME, num, this->getAnimFPS( ANIMMODE_FRAME, *lit ) );
            m->setAnimLooping( ANIMMODE_FRAME, num, this->getAnimLooping( ANIMMODE_FRAME, *lit ) );

            FrameAnim * fa = this->m_frameAnims[*lit];

            for ( unsigned f = 0; f < fa->m_frameData.size(); f++ )
            {
               Matrix saveMatrix;

               if ( rootPoint != -1 )
               {
                  double coord[3];
                  double rot[3];

                  this->getFrameAnimPointCoords( *lit, f, rootPoint, coord[0], coord[1], coord[2] );
                  this->getFrameAnimPointRotation( *lit, f, rootPoint, rot[0], rot[1], rot[2] );

                  saveMatrix.setTranslation( coord );
                  saveMatrix.setRotation( rot );
                  saveMatrix = saveMatrix.getInverse();

                  Matrix up;
                  up.setRotationInDegrees( -90, -90, 0 );
                  saveMatrix = saveMatrix * up;
               }

               for ( unsigned v = 0; v < fa->m_frameData[f]->m_frameVertices->size(); v++ )
               {
                  if ( !isValueInList( vert, v ) )
                     continue;

                  FrameAnimVertex * fav = (*fa->m_frameData[f]->m_frameVertices)[v];

                  double coord[4];
                  coord[0] = fav->m_coord[0];
                  coord[1] = fav->m_coord[1];
                  coord[2] = fav->m_coord[2];
                  saveMatrix.apply3x( coord );

                  m->setFrameAnimVertexCoords( num, f, vertMap[v],
                        coord[0], coord[1], coord[2] );
               }
               for ( unsigned p = 0; p < fa->m_frameData[f]->m_framePoints->size(); p++ )
               {
                  if ( !isValueInList( points, p ) )
                     continue;

                  FrameAnimPoint * fap = (*fa->m_frameData[f]->m_framePoints)[p];

                  Matrix pointMatrix;
                  double trans[3];
                  double rot[3];
                  pointMatrix.setTranslation( fap->m_trans );
                  pointMatrix.setRotation( fap->m_rot );
                  pointMatrix = pointMatrix * saveMatrix;
                  pointMatrix.getTranslation( trans );
                  pointMatrix.getRotation( rot );

                  m->setFrameAnimPointCoords( num, f, pointMap[p],
                        trans[0], trans[1], trans[2] );
                  m->setFrameAnimPointRotation( num, f, pointMap[p],
                        rot[0], rot[1], rot[2] );
               }
            }
         }
      }
   }

   m->invalidateNormals();
   m->calculateNormals();
   m->setupJoints();

   return m;
}

Model * Model::copySelected() const
{
   int rootPoint = -1;
   list<int> tri;
   list<int> vert;
   list<int> groups;
   list<int> materials;
   list<int> joints;
   list<int> points;
   list<int> proj;
   list<int> skelAnims;
   list<int> frameAnims;

   this->getSelectedTriangles( tri );
   this->getSelectedVertices( vert );
   this->getSelectedBoneJoints( joints );
   this->getSelectedPoints( points );
   this->getSelectedProjections( proj );

   // TODO Only copy selected groups?
   // It's easier here to just copy the groups and textures 
   // even if not needed, the user can delete the unecessary parts
   unsigned gcount = this->getGroupCount();
   for ( unsigned g = 0; g < gcount; g++ )
   {
      groups.push_back( g );
   }

   // TODO only copy textures used by groups?
   unsigned tcount = this->getTextureCount();
   for ( unsigned t = 0; t < tcount; t++ )
   {
      materials.push_back( t );
   }

   // TODO what about animations?

   return copySpecified( rootPoint, tri, vert, groups, materials, joints, points, proj, skelAnims, frameAnims );
}

// MD3 export doesn't use this because copying frame animations is extermly slow.
Model * Model::copyQuake3PlayerSection( MeshSectionE section, AnimationModeE mode ) const
{
   int rootPoint;
   list<int> tri;
   list<int> vert;
   list<int> groups;
   list<int> materials;
   list<int> joints;
   list<int> points;
   list<int> proj;
   list<int> skelAnims;
   list<int> frameAnims;

   const char *groupPrefix;
   std::list<std::string> animationPrefixes;
   //std::list<std::string> pointNames;
   std::list<std::string> ignoreFramePoints;
   const char *rootPointName;
   const char *splitPointName;

   switch ( section )
   {
      default:
         return NULL;
      case MS_Lower:
         groupPrefix = "l_";
         animationPrefixes.push_back( "ALL_" );
         animationPrefixes.push_back( "BOTH_" );
         animationPrefixes.push_back( "LEGS_" );

         rootPointName = NULL;
         splitPointName = "tag_torso";

         //pointNames.push_back( "tag_torso" );

         ignoreFramePoints.push_back( "tag_head" );
         ignoreFramePoints.push_back( "tag_weapon" );
         // ZTM: FIXME?: Game specific
         // Support Team Arena tag point
         ignoreFramePoints.push_back( "tag_flag" );
         // Support Turtle Arena tag points
         ignoreFramePoints.push_back( "tag_hand_primary" );
         ignoreFramePoints.push_back( "tag_hand_secondary" );
         ignoreFramePoints.push_back( "tag_wp_away_primary" );
         ignoreFramePoints.push_back( "tag_wp_away_secondary" );
         break;
      case MS_Upper:
         groupPrefix = "u_";
         animationPrefixes.push_back( "ALL_" );
         animationPrefixes.push_back( "BOTH_" );
         animationPrefixes.push_back( "TORSO_" );

         rootPointName = "tag_torso";
         splitPointName = "tag_head";

         //pointNames.push_back( "tag_torso" );
         //pointNames.push_back( "tag_head" );
         //pointNames.push_back( "tag_weapon" );
         // ZTM: FIXME?: Game specific
         // Support Team Arena tag point
         //pointNames.push_back( "tag_flag" );
         // Support Turtle Arena tag points
         //pointNames.push_back( "tag_hand_primary" );
         //pointNames.push_back( "tag_hand_secondary" );
         //pointNames.push_back( "tag_wp_away_primary" );
         //pointNames.push_back( "tag_wp_away_secondary" );
         break;
      case MS_Head:
         groupPrefix = "h_";
         animationPrefixes.push_back( "ALL_" );
         animationPrefixes.push_back( "HEAD_" );

         rootPointName = "tag_head";
         splitPointName = NULL;

         //pointNames.push_back( "tag_head" );

         ignoreFramePoints.push_back( "tag_torso" );
         ignoreFramePoints.push_back( "tag_weapon" );
         // ZTM: FIXME?: Game specific
         // Support Team Arena tag point
         ignoreFramePoints.push_back( "tag_flag" );
         // Support Turtle Arena tag points
         ignoreFramePoints.push_back( "tag_hand_primary" );
         ignoreFramePoints.push_back( "tag_hand_secondary" );
         ignoreFramePoints.push_back( "tag_wp_away_primary" );
         ignoreFramePoints.push_back( "tag_wp_away_secondary" );
         break;
   }

   // TODO: Require root point and attach to be have a 1 or 0 bone influences and be at the bone origin,
   //       this won't work correctly otherwise.

   rootPoint = rootPointName ? getPointByName( rootPointName ) : -1;
   if ( rootPoint >= 0 )
   {
      InfluenceList il;
      InfluenceList::iterator it;
      getPointInfluences( rootPoint, il );

      for ( it = il.begin(); it != il.end(); it++ )
      {
         if ( it->m_weight > 0 )
         {
            joints.push_back( it->m_boneId );
         }
      }
   }
   else
   {
      // Add all root joints
      unsigned jcount = this->getBoneJointCount();
      for ( unsigned j = 0; j < jcount; j++ )
      {
         if ( getBoneJointParent( j ) == -1 )
         {
            joints.push_back( j );
         }
      }
   }

   int splitPoint = splitPointName ? getPointByName( splitPointName ) : -1;
   list<int> ignoreChildJoints;
   if ( splitPoint >= 0 )
   {
      InfluenceList il;
      InfluenceList::iterator it;
      getPointInfluences( splitPoint, il );

      for ( it = il.begin(); it != il.end(); it++ )
      {
         if ( it->m_weight > 0 )
         {
            ignoreChildJoints.push_back( it->m_boneId );
         }
      }
   }

   // Add child joints of root point but skip children of the split point
   unsigned jcount = this->getBoneJointCount();
   for ( unsigned j = 0; j < jcount; j++ )
   {
      if ( isValueInList( joints, j ) )
         continue;

      int parent = getBoneJointParent( j );

      if ( isValueInList( ignoreChildJoints, parent ) )
         continue;

      if ( isValueInList( joints, parent ) )
      {
         joints.push_back( j );
      }
   }

   joints.sort();
   joints.unique();

   if ( mode == ANIMMODE_SKELETAL )
   {
      // Add points connected to the bone joints
      unsigned pcount = this->getPointCount();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         int joint = getPointBoneJoint( p );
         if ( joint == -1 )
         {
            if ( rootPoint == -1 )
            {
               points.push_back( p );
            }
            continue;
         }

         if ( isValueInList( joints, joint ) )
         {
            points.push_back( p );
         }
      }
   }

   if ( mode == ANIMMODE_FRAME )
   {
      // Add all points unless their known to not go in this section
      unsigned pcount = this->getPointCount();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         const char *pname = getPointName( p );
         bool ignorePoint = false;
         for ( list<std::string>::iterator it = ignoreFramePoints.begin(); it != ignoreFramePoints.end(); ++it )
         {
            if ( strcasecmp( (*it).c_str(), pname ) == 0 )
            {
               ignorePoint = true;
               break;
            }
         }
         if ( !ignorePoint )
         {
            points.push_back( p );
         }
      }
   }

   for ( unsigned g = 0; g < m_groups.size(); g++ )
   {
      if ( strncasecmp( groupPrefix, m_groups[g]->m_name.c_str(), strlen( groupPrefix ) ) == 0 )
      {
         list<int> groupTris = getGroupTriangles( g );

         for ( std::list<int>::iterator lit = groupTris.begin(); lit != groupTris.end(); ++lit )
         {
            for ( int t = 0; t < 3; t++ )
            {
               int v = this->getTriangleVertex( *lit, t );
               vert.push_back( v );
            }
         }

         tri.splice(tri.end(), groupTris);
         groups.push_back( g );
      }
   }

   tri.sort();
   vert.sort();
   vert.unique();

   // TODO only copy textures used by groups?
   unsigned tcount = this->getTextureCount();
   for ( unsigned t = 0; t < tcount; t++ )
   {
      materials.push_back( t );
   }

   if ( mode == ANIMMODE_SKELETAL )
   {
      for ( unsigned anim = 0; anim < m_skelAnims.size(); anim++ )
      {
         for ( list<std::string>::iterator it = animationPrefixes.begin(); it != animationPrefixes.end(); ++it )
         {
            if ( strncasecmp( (*it).c_str(), m_skelAnims[anim]->m_name.c_str(), (*it).length() ) == 0 )
            {
               skelAnims.push_back( anim );
               break;
            }
         }
      }
   }

   if ( mode == ANIMMODE_FRAME )
   {
      for ( unsigned anim = 0; anim < m_frameAnims.size(); anim++ )
      {
         for ( list<std::string>::iterator it = animationPrefixes.begin(); it != animationPrefixes.end(); ++it )
         {
            if ( strncasecmp( (*it).c_str(), m_frameAnims[anim]->m_name.c_str(), (*it).length() ) == 0 )
            {
               frameAnims.push_back( anim );
               break;
            }
         }
      }
   }

   return copySpecified( rootPoint, tri, vert, groups, materials, joints, points, proj, skelAnims, frameAnims );
}
