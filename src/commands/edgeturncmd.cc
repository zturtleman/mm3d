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


#include "menuconf.h"
#include "edgeturncmd.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include <QtCore/QObject>
#include <QtWidgets/QApplication>

EdgeTurnCommand::EdgeTurnCommand()
{
}

EdgeTurnCommand::~EdgeTurnCommand()
{
}

bool EdgeTurnCommand::activated( int arg, Model * model )
{
   list<int> triList;
   model->getSelectedTriangles( triList );

   if ( triList.size() >= 2 )
   {
      // Only turns one edge, deal with it
      unsigned int edge_v1;
      unsigned int edge_v2;
      unsigned int tri1_v;
      unsigned int tri2_v;

      list<int>::iterator it1 = triList.begin();
      list<int>::iterator it2 = it1;

      for ( it1 = triList.begin(); it1 != triList.end(); it1++ )
      {
         it2 = it1;
         it2++;
         for ( ; it2 != triList.end(); it2++ )
         {
            if ( canTurnEdge( model, *it1, *it2, edge_v1, edge_v2, tri1_v, tri2_v ) )
            {
               log_debug( "turning edge on triangles %d and %d\n", *it1, *it2 );
               log_debug( "vertices %d,%d,%d %d,%d,%d\n", 
                     edge_v1, edge_v2, tri1_v,
                     edge_v1, edge_v2, tri2_v );

               int t1a = 0;
               int t1b = 0;
               int t1c = 0;
               int t2a = 0;
               int t2b = 0;
               int t2c = 0;

               getTriangleVertices( model, *it1, 
                     edge_v1, edge_v2, tri1_v,
                     t1a, t1b, t1c );
               getTriangleVertices( model, *it2, 
                     edge_v1, edge_v2, tri2_v,
                     t2a, t2b, t2c );

               log_debug( "indices %d,%d,%d %d,%d,%d\n", 
                     t1a, t1b, t1c,
                     t2a, t2b, t2c );

               // For triangle 1:
               unsigned int verts1[ 3 ];
               unsigned int verts2[3];

               verts1[ t1a ] = tri1_v;
               verts1[ t1b ] = tri2_v;
               verts1[ t1c ] = edge_v2;

               verts2[ t2a ] = tri1_v;
               verts2[ t2b ] = tri2_v;
               verts2[ t2c ] = edge_v1;

               model->setTriangleVertices( 
                     *it1, verts1[0], verts1[1], verts1[2] );
               model->setTriangleVertices( 
                     *it2, verts2[0], verts2[1], verts2[2] );

               model_status( model, StatusNormal, STATUSTIME_SHORT, "%s", qApp->translate( "Command", "Edge Turn complete" ).toUtf8().data() );
               return true;
            }
         }
      }
   }

   model_status( model, StatusError, STATUSTIME_LONG, "%s", qApp->translate( "Command", "You must have at least 2 adjacent faces to Edge Turn" ).toUtf8().data() );
   return false;
}

// edge_v1 and edge_v2 are the model vertices of the common edge
// tri1_v and tri2_v are the opposite vertices of the respective triangles
bool EdgeTurnCommand::canTurnEdge( Model * model, int tri1, int tri2,
      unsigned int & edge_v1, unsigned int & edge_v2, unsigned int & tri1_v, unsigned int & tri2_v )
{
   unsigned int verts1[3];
   unsigned int verts2[3];

   model->getTriangleVertices( tri1, verts1[0], verts1[1], verts1[2] );
   model->getTriangleVertices( tri2, verts2[0], verts2[1], verts2[2] );

   const unsigned int invalid = (unsigned) ~0;

   edge_v1 = invalid;
   edge_v2 = invalid;
   tri1_v  = invalid;
   tri2_v  = invalid;

   for ( int i1 = 0; i1 < 3; i1++ )
   {
      unsigned int v1 = verts1[i1];
      if ( v1 != edge_v1 && v1 != edge_v2 )
      {
         for ( int i2 = 0; i2 < 3; i2++ )
         {
            unsigned int v2 = verts2[i2];
            if ( v2 != edge_v1 && v2 != edge_v2 )
            {
               if ( v1 == v2 )
               {
                  if ( edge_v1 == invalid )
                     edge_v1 = v1;
                  else if ( edge_v2 == invalid )
                     edge_v2 = v1;
               }
            }
         }
      }
   }

   if ( edge_v1 != invalid && edge_v2 != invalid )
   {
      int i;
      for ( i = 0; i < 3; i++ )
      {
         if ( verts1[i] != edge_v1 && verts1[i] != edge_v2 )
         {
            tri1_v = verts1[i];
            break;
         }
      }
      for ( i = 0; i < 3; i++ )
      {
         if ( verts2[i] != edge_v1 && verts2[i] != edge_v2 )
         {
            tri2_v = verts2[i];
            break;
         }
      }
      return true;
   }
   else
   {
      return false;
   }
}

// Triangle index a is edge_v1
// Triangle index b is edge_v2
// Triangle index c is tri_v, the third triangle vertex
//
// Note that a, b, and c are the indices into the list of three triangle vertices,
// not the index of the model vertices
void EdgeTurnCommand::getTriangleVertices( Model * model, int tri,
      unsigned int edge_v1, unsigned int edge_v2, unsigned int tri_v,
      int & a, int & b, int & c )
{
   unsigned int v[3] = {0,0,0};

   model->getTriangleVertices( tri, v[0], v[1], v[2] );

   for ( int i = 0; i < 3; i++ )
   {
      if ( v[i] == edge_v1 )
         a = i;
      else if ( v[i] == edge_v2 )
         b = i;
      else if ( v[i] == tri_v )
         c = i;
   }
}

const char * EdgeTurnCommand::getPath()
{
   return GEOM_FACES_MENU;
}

const char * EdgeTurnCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Edge Turn" );
}

