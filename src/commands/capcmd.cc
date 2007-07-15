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


#include "menuconf.h"
#include "capcmd.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include <qobject.h>
#include <qapplication.h>

CapCommand::CapCommand()
{
}

CapCommand::~CapCommand()
{
}

bool CapCommand::activated( int arg, Model * model )
{
   int added = 0;

   // Algorithm:
   //
   // Selected edges
   // For each selected vertex, check every vertex connected to it
   //
   //   For each connected edge that does not belong to two triangles,
   //   add a triangle using a third vertex from another triangle
   //   that uses this vertex and another selected vertex

   list<int> vertList;
   list<int> triList;  // triangles using target vertex
   list<int> conList;  // vertices connected to target vertex

   model->getSelectedVertices( vertList );

   std::list<int>::iterator it;
   for ( it = vertList.begin(); it != vertList.end(); it++ )
   {
      getConnected( model, *it, conList, triList );

      unsigned int vcount = conList.size();
      unsigned int tcount = triList.size();  
      if ( vcount > 2 && vcount != tcount )
      {
         // If I'm connected to more than two vertices, the
         // triangle and vertex count should match, otherwise
         // there is a gap/hole.

         int newTri = 0;
         while ( tcount != vcount && newTri >= 0 )
         {
            newTri = createMissingTriangle( model, *it, conList, triList );
            if ( newTri >= 0 )
            {
               added++;
               tcount++;
               triList.push_back( newTri );
            }
         }
      }
   }

   if ( added != 0 )
   {
      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Cap Holes complete").utf8() );
      return true;
   }
   else
   {
      model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "Could not find gap in selected region" ).utf8() );
   }
   return false;
}

void CapCommand::getConnected( Model * model, int v, 
      std::list<int> & conList,
      std::list<int> & triList )
{
   conList.clear();
   triList.clear();

   unsigned int tcount = model->getTriangleCount();
   for ( unsigned int tri = 0; tri < tcount; tri++ )
   {
      unsigned int verts[3];
      model->getTriangleVertices( tri, verts[0], verts[1], verts[2] );

      for ( int i = 0; i < 3; i++ )
      {
         if ( (int) verts[i] == v )
         {
            triList.push_back( tri );

            addToList( conList, v, verts[0] );
            addToList( conList, v, verts[1] );
            addToList( conList, v, verts[2] );

            break;
         }
      }
   }
}

void CapCommand::addToList( std::list<int> & l, int ignore, int val )
{
   if ( ignore == val )
   {
      return;
   }

   std::list<int>::iterator it;
   for ( it = l.begin(); it != l.end(); it++ )
   {
      if ( *it == val )
      {
         return;
      }
   }
   l.push_back( val );
}

int CapCommand::createMissingTriangle( Model * model, unsigned int v,
      std::list<int> & conList, std::list<int> & triList )
{
   std::list<int>::iterator cit1;
   std::list<int>::iterator cit2;
   int tri = 0;

   for ( cit1 = conList.begin(); cit1 != conList.end(); cit1++ )
   {
      if ( model->isVertexSelected( *cit1 ) )
      {
         if ( triangleCount( model, v, *cit1, triList, tri ) == 1 )
         {
            cit2 = cit1;
            cit2++;

            // Find third triangle vertex for normal test below
            int tri1Vert = 0; 
            unsigned int verts[3];
            model->getTriangleVertices( tri, verts[0], verts[1], verts[2] );

            for ( int i = 0; i < 3; i++ )
            {
               if ( verts[i] != v && verts[i] != (unsigned int) *cit1 )
               {
                  tri1Vert = verts[i];
               }
            }

            for ( ; cit2 != conList.end(); cit2++ )
            {
               if ( model->isVertexSelected( *cit2 ) )
               {
                  if ( triangleCount( model, v, *cit2, triList, tri ) == 1 )
                  {
                     int newTri = model->addTriangle( v, *cit1, *cit2 );

                     float norm1[3];
                     float norm2[3];
                     double coord[3];

                     model->getFlatNormal( tri, norm1 );
                     model->getFlatNormal( newTri, norm2 );
                     model->getVertexCoords( *cit1, coord );

                     // this is why we needed the third vertex above,
                     // so we can check to see if we need to invert
                     // the new triangle (are the triangles behind
                     // each other or in front of each other?)
                     float f = model->cosToPoint( tri, coord );
                     if ( fabs( f ) < 0.0001f )
                     {
                        // Points are nearly co-planar,
                        // normals should face the same way
                        if ( dot3( norm1, norm2 ) < 0.0f )
                        {
                           model->invertNormals( newTri );
                        }
                     }
                     else
                     {
                        if ( f < 0.0f )
                        {
                           model->getVertexCoords( tri1Vert, coord );
                           if ( model->cosToPoint( newTri, coord ) > 0.0f )
                           {
                              model->invertNormals( newTri );
                           }
                        }
                        else
                        {
                           model->getVertexCoords( tri1Vert, coord );
                           if ( model->cosToPoint( newTri, coord ) < 0.0f )
                           {
                              model->invertNormals( newTri );
                           }
                        }
                     }

                     return newTri;
                  }
               }
            }
         }
      }
   }
   return -1;
}

int CapCommand::triangleCount( Model * model, 
      unsigned int v1, unsigned int v2, std::list<int> & triList, int & tri ) 
{
   std::list<int>::iterator it;

   unsigned int verts[3];
   int triCount = 0;

   for ( it = triList.begin(); it != triList.end(); it++ )
   {
      model->getTriangleVertices( *it, verts[0], verts[1], verts[2] );

      bool have1 = false;
      bool have2 = false;

      for ( int i = 0; i < 3; i++ )
      {
         if ( verts[i] == v1 )
            have1 = true;
         else if ( verts[i] == v2 )
            have2 = true;
      }

      if ( have1 && have2 )
      {
         triCount++;
         tri = *it;
      }
   }
   return triCount;
}

const char * CapCommand::getPath()
{
   return GEOM_MESHES_MENU;
}

const char * CapCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Cap Holes" );
}

