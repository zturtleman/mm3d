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

#include <vector>
#include <ext/hash_map>

using __gnu_cxx::hash_map;


Model * Model::copySelected() const
{
   Model * m = new Model;

   list<int> tri;
   this->getSelectedTriangles( tri );
   list<int> joints;
   this->getSelectedBoneJoints( joints );
   list<int> points;
   this->getSelectedPoints( points );
   list<int> proj;
   this->getSelectedProjections( proj );

   hash_map<int,int> projMap;
   hash_map<int,int> vertMap;
   hash_map<int,int> triMap;
   hash_map<int,int> jointMap;
   hash_map<int,int> pointMap;

   list<int>::iterator lit;

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

         m->selectProjection( np );

         projMap[ *lit ] = np;
      }
   }

   if ( !tri.empty() )
   {
      list<int> vert;
      this->getSelectedVertices( vert );

      // Copy vertices
      log_debug( "Copying %" PORTuSIZE " vertices\n", vert.size() );
      for ( lit = vert.begin(); lit != vert.end(); lit++ )
      {
         double coords[3];
         this->getVertexCoords( *lit, coords );
         int nv = m->addVertex( coords[0], coords[1], coords[2] );
         m->selectVertex( nv );
         m->setVertexFree( nv, this->isVertexFree(*lit) );

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
         m->selectTriangle( nt );

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

      // TODO only copy textures used by groups?
      // Copy textures
      // It's easier here to just copy the groups and textures 
      // even if not needed, the user can delete the unecessary parts
      unsigned tcount = this->getTextureCount();
      log_debug( "Copying %d textures\n", tcount );
      for ( unsigned t = 0; t < tcount; t++ )
      {
         switch ( this->getMaterialType( t ) )
         {
            case Model::Material::MATTYPE_TEXTURE:
               {
                  Texture * tex = TextureManager::getInstance()->getTexture( this->getTextureFilename( t ) );
                  int num = m->addTexture( tex );
                  m->setTextureName(num, getTextureName( t ) );
               }
               break;
            default:
               log_error( "Unknown material type %d in duplicate\n", this->getMaterialType(t) );
            case Model::Material::MATTYPE_BLANK:
               m->addColorMaterial( this->getTextureName( t ) );
               break;
         }

         float c[4];
         this->getTextureAmbient( t, c );
         m->setTextureAmbient( t, c );
         this->getTextureDiffuse( t, c );
         m->setTextureDiffuse( t, c );
         this->getTextureSpecular( t, c );
         m->setTextureSpecular( t, c );
         this->getTextureEmissive( t, c );
         m->setTextureEmissive( t, c );
         this->getTextureShininess( t, c[0] );
         m->setTextureShininess( t, c[0] );

         // TODO Material m_color (if ever used)

         m->setTextureSClamp( t, this->getTextureSClamp( t ) );
         m->setTextureTClamp( t, this->getTextureTClamp( t ) );
      }

      // TODO Only copy selected groups?
      // Copy groups
      // It's easier here to just copy the groups and textures 
      // even if not needed, the user can delete the unecessary parts
      unsigned gcount = this->getGroupCount();
      log_debug( "Copying %d groups\n", gcount );
      for ( unsigned g = 0; g < gcount; g++ )
      {
         m->addGroup( this->getGroupName( g ) );
         m->setGroupSmooth( g, this->getGroupSmooth(g) );
         m->setGroupAngle( g, this->getGroupAngle(g) );
         m->setGroupTextureId( g, this->getGroupTextureId(g) );
      }

      if ( gcount > 0 )
      {
         // Set groups
         log_debug( "Setting %" PORTuSIZE " triangle groups\n", tri.size() );
         for ( lit = tri.begin(); lit != tri.end(); lit++ )
         {
            // This works, even if triangle group == -1
            int gid = this->getTriangleGroup(*lit);
            if ( gid >= 0 )
            {
               m->addTriangleToGroup( gid, triMap[*lit] );
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
         double coord[3];
         double rot[3] = { 0, 0, 0 };
         this->getPointCoords( *lit, coord );
         this->getPointRotation( *lit, rot );

         // TODO point type (if it is ever used)
         int np = m->addPoint( this->getPointName( *lit ),
               coord[0], coord[1], coord[2], rot[0], rot[1], rot[2], -1 );
         pointMap[ *lit ] = np;

         m->selectPoint( np );
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
         if ( this->isBoneJointSelected( parent ) )
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

         double coord[3];
         double rot[3] = { 0, 0, 0 };
         this->getBoneJointCoords( *lit, coord );

         int nj = m->addBoneJoint( this->getBoneJointName( *lit ),
               coord[0], coord[1], coord[2], rot[0], rot[1], rot[2], parent );
         jointMap[ *lit ] = nj;

         m->selectBoneJoint( nj );
      }

      for ( hash_map<int,int>::iterator it = vertMap.begin();
            it != vertMap.end(); ++it )
      {
         InfluenceList il;

         getVertexInfluences( it->first, il );

         for ( InfluenceList::iterator iit = il.begin(); iit != il.end(); ++iit )
         {
            if ( isBoneJointSelected( iit->m_boneId ) )
            {
               m->addVertexInfluence( it->second, jointMap[iit->m_boneId],
                     iit->m_type, iit->m_weight );
            }
         }
      }

      for ( hash_map<int,int>::iterator it = pointMap.begin();
            it != pointMap.end(); ++it )
      {
         InfluenceList il;

         getPointInfluences( it->first, il );

         for ( InfluenceList::iterator iit = il.begin(); iit != il.end(); ++iit )
         {
            if ( isBoneJointSelected( iit->m_boneId ) )
            {
               m->addPointInfluence( it->second, jointMap[iit->m_boneId],
                     iit->m_type, iit->m_weight );
            }
         }
      }
   }

   // TODO what about animations?

   m->invalidateNormals();
   m->calculateNormals();
   m->setupJoints();

   return m;
}
