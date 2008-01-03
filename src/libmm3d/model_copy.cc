/*  Misfit Model 3D
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


Model * Model::copySelected()
{
   Model * m = new Model;

   list<int> tri;
   this->getSelectedTriangles( tri );
   list<int> joints;
   this->getSelectedBoneJoints( joints );
   list<int> points;
   this->getSelectedPoints( points );

   hash_map<int,int> vertMap;
   hash_map<int,int> triMap;
   hash_map<int,int> jointMap;
   hash_map<int,int> pointMap;

   list<int>::iterator lit;

   if ( !tri.empty() )
   {
      list<int> vert;
      this->getSelectedVertices( vert );

      // FIXME missing any other new properties?

      // Copy vertices
      log_debug( "Copying %d vertices\n", vert.size() );
      for ( lit = vert.begin(); lit != vert.end(); lit++ )
      {
         double coords[3];
         this->getVertexCoords( *lit, coords );
         int nv = m->addVertex( coords[0], coords[1], coords[2] );
         m->selectVertex( nv );
         m->setVertexFree( nv, this->isVertexFree(*lit) );

         // FIXME free vertices not supported here yet

         vertMap[ *lit ] = nv;
      }

      // FIXME Copy texture projections

      // Copy faces
      log_debug( "Copying %d faces\n", tri.size() );
      for ( lit = tri.begin(); lit != tri.end(); lit++ )
      {
         unsigned v[3];

         for ( int t = 0; t < 3; t++ )
         {
            v[t] = this->getTriangleVertex( *lit, t );
         }
         int nt = m->addTriangle( vertMap[v[0]] , vertMap[v[1]], vertMap[v[2]] );
         m->selectTriangle( nt );
         // FIXME assign triangles to texture projections (if selected)

         triMap[ *lit ] = nt;
      }

      // Copy texture coords
      log_debug( "Copying %d face texture coordinates\n", tri.size() );
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

      // FIXME only copy textures used by groups?
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
                  m->addTexture( tex );
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

         // FIXME Material m_color (if ever used)

         m->setTextureSClamp( t, this->getTextureSClamp( t ) );
         m->setTextureTClamp( t, this->getTextureTClamp( t ) );
      }

      // FIXME Only copy selected groups?
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
         log_debug( "Setting %d triangle groups\n", tri.size() );
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

   if ( !joints.empty() )
   {
      // Copy joints
      log_debug( "Copying %d joints\n", joints.size() );
      for ( lit = joints.begin(); lit != joints.end(); lit++ )
      {
         int parent = this->getBoneJointParent( *lit );

         // FIXME what if parent joint is not selected?

         // TODO this will not work if parent joint comes after child
         // joint.  That shouldn't happen... but...
         if ( this->isBoneJointSelected( parent ) )
         {
            parent = jointMap[ parent ];
         }

         double coord[3];
         double rot[3] = { 0, 0, 0 };
         this->getBoneJointCoords( *lit, coord );

         int nj = m->addBoneJoint( this->getBoneJointName( *lit ),
               coord[0], coord[1], coord[2], rot[0], rot[1], rot[2], parent );
         jointMap[ *lit ] = nj;

         m->selectBoneJoint( nj );

         // Assign copied vertices to copied bone joints
         // FIXME multiple bone joints
         list<int> vertlist = this->getBoneJointVertices( *lit );
         list<int>::iterator vit;
         for ( vit = vertlist.begin(); vit != vertlist.end(); vit++ )
         {
            if ( this->isVertexSelected( *vit ) )
            {
               m->setVertexBoneJoint( vertMap[ *vit ], nj );
            }
         }
      }
   }

   if ( !points.empty() )
   {
      // Copy points
      log_debug( "Copying %d points\n", points.size() );
      for ( lit = points.begin(); lit != points.end(); lit++ )
      {
         int parent = this->getPointBoneJoint( *lit );

         if ( this->isBoneJointSelected( parent ) )
         {
            parent = jointMap[ parent ];
         }

         double coord[3];
         double rot[3] = { 0, 0, 0 };
         this->getPointCoords( *lit, coord );
         this->getPointRotation( *lit, rot );

         // FIXME multiple bone joints
         // FIXME point type (if it is ever used)
         int np = m->addPoint( this->getPointName( *lit ),
               coord[0], coord[1], coord[2], rot[0], rot[1], rot[2], parent );
         pointMap[ *lit ] = np;

         m->selectPoint( np );
      }
   }

   // FIXME what about animations?

   m->invalidateNormals();

   // FIXME setupJoints?

   return m;
}
