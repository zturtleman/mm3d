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


#include "copycmd.h"

#include "model.h"
#include "filtermgr.h"
#include "texmgr.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"
#include "sysconf.h"
#include "misc.h"

#include <list>
#include <map>
#include <qobject.h>
#include <qapplication.h>

using std::list;
using std::map;

CopyCommand::CopyCommand()
{
}

CopyCommand::~CopyCommand()
{
}

bool CopyCommand::activated( int arg, Model * model )
{
   if ( model )
   {
      Model * m = new Model;

      list<int> tri;
      model->getSelectedTriangles( tri );
      list<int> joints;
      model->getSelectedBoneJoints( joints );
      list<int> points;
      model->getSelectedPoints( points );

      map<int,int> vertMap;
      map<int,int> triMap;
      map<int,int> jointMap;
      map<int,int> pointMap;

      list<int>::iterator lit;

      if ( !tri.empty() )
      {
         model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Selected primitives copied" ).utf8() );

         list<int> vert;
         model->getSelectedVertices( vert );

         // Copy vertices
         log_debug( "Copying %d vertices\n", vert.size() );
         for ( lit = vert.begin(); lit != vert.end(); lit++ )
         {
            double coords[3];
            model->getVertexCoords( *lit, coords );
            int nv = m->addVertex( coords[0], coords[1], coords[2] );
            m->selectVertex( nv );

            vertMap[ *lit ] = nv;
         }

         // Copy faces
         log_debug( "Copying %d faces\n", tri.size() );
         for ( lit = tri.begin(); lit != tri.end(); lit++ )
         {
            unsigned v[3];

            for ( int t = 0; t < 3; t++ )
            {
               v[t] = model->getTriangleVertex( *lit, t );
            }
            int nt = m->addTriangle( vertMap[v[0]] , vertMap[v[1]], vertMap[v[2]] );
            m->selectTriangle( nt );

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
               model->getTextureCoords( (unsigned) *lit, i, s, t );
               m->setTextureCoords( (unsigned) triMap[ *lit ], i, s, t );
            }
         }

         // Copy textures
         // It's easier here to just copy the groups and textures 
         // even if not needed, the user can delete the unecessary parts
         unsigned tcount = model->getTextureCount();
         log_debug( "Copying %d textures\n", tcount );
         for ( unsigned t = 0; t < tcount; t++ )
         {
            switch ( model->getMaterialType( t ) )
            {
               case Model::Material::MATTYPE_TEXTURE:
                  {
                     Texture * tex = TextureManager::getInstance()->getTexture( model->getTextureFilename( t ) );
                     m->addTexture( tex );
                  }
                  break;
               default:
                  log_error( "Unknown material type %d in duplicate\n", model->getMaterialType(t) );
               case Model::Material::MATTYPE_BLANK:
                  m->addColorMaterial( model->getTextureName( t ) );
                  break;
            }

            float c[4];
            model->getTextureAmbient( t, c );
            m->setTextureAmbient( t, c );
            model->getTextureDiffuse( t, c );
            m->setTextureDiffuse( t, c );
            model->getTextureSpecular( t, c );
            m->setTextureSpecular( t, c );
            model->getTextureEmissive( t, c );
            m->setTextureEmissive( t, c );
            model->getTextureShininess( t, c[0] );
            m->setTextureShininess( t, c[0] );

            m->setTextureSClamp( t, model->getTextureSClamp( t ) );
            m->setTextureTClamp( t, model->getTextureTClamp( t ) );
         }

         // Copy groups
         // It's easier here to just copy the groups and textures 
         // even if not needed, the user can delete the unecessary parts
         unsigned gcount = model->getGroupCount();
         log_debug( "Copying %d groups\n", gcount );
         for ( unsigned g = 0; g < gcount; g++ )
         {
            m->addGroup( model->getGroupName( g ) );
            m->setGroupSmooth( g, model->getGroupSmooth(g) );
            m->setGroupAngle( g, model->getGroupAngle(g) );
            m->setGroupTextureId( g, model->getGroupTextureId(g) );
         }

         if ( gcount > 0 )
         {
            // Set groups
            log_debug( "Setting %d triangle groups\n", tri.size() );
            for ( lit = tri.begin(); lit != tri.end(); lit++ )
            {
               // This works, even if triangle group == -1
               int gid = model->getTriangleGroup(*lit);
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
            int parent = model->getBoneJointParent( *lit );

            // TODO this will not work if parent joint comes after child
            // joint.  That shouldn't happen... but...
            if ( model->isBoneJointSelected( parent ) )
            {
               parent = jointMap[ parent ];
            }

            double coord[3];
            double rot[3] = { 0, 0, 0 };
            model->getBoneJointCoords( *lit, coord );

            int nj = m->addBoneJoint( model->getBoneJointName( *lit ),
                     coord[0], coord[1], coord[2], rot[0], rot[1], rot[2], parent );
            jointMap[ *lit ] = nj;

            m->selectBoneJoint( nj );

            // Assign copied vertices to copied bone joints
            list<int> vertlist = model->getBoneJointVertices( *lit );
            list<int>::iterator vit;
            for ( vit = vertlist.begin(); vit != vertlist.end(); vit++ )
            {
               if ( model->isVertexSelected( *vit ) )
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
            int parent = model->getPointBoneJoint( *lit );

            if ( model->isBoneJointSelected( parent ) )
            {
               parent = jointMap[ parent ];
            }

            double coord[3];
            double rot[3] = { 0, 0, 0 };
            model->getPointCoords( *lit, coord );
            model->getPointRotation( *lit, rot );

            int np = m->addPoint( model->getPointName( *lit ),
                     coord[0], coord[1], coord[2], rot[0], rot[1], rot[2], parent );
            pointMap[ *lit ] = np;

            m->selectPoint( np );
         }
      }

      m->invalidateNormals();

      std::string clipfile = getMm3dHomeDirectory();

      clipfile += "/clipboard";
      mkpath( clipfile.c_str(), 0755 );
      clipfile += "/clipboard.mm3d";

      FilterManager::getInstance()->writeFile( m, clipfile.c_str(), FilterManager::WO_ModelNoPrompt );

      if ( joints.empty() && tri.empty() && points.empty() )
      {
         model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "You must have at least 1 face, joint, or point selected to Copy" ).utf8() );
         return false;
      }
      else
      {
         model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Copy complete" ).utf8() );
      }

      return true;
   }
   else
   {
      return false;
   }
}

const char * CopyCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Copy Selected to Clipboard" );
}

