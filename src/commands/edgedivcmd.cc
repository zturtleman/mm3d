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
#include "edgedivcmd.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include <QObject>
#include <QApplication>

EdgeDivideCommand::EdgeDivideCommand()
{
}

EdgeDivideCommand::~EdgeDivideCommand()
{
}

bool EdgeDivideCommand::activated( int arg, Model * model )
{
   int split = 0;

   list<int> vertList;
   model->getSelectedVertices( vertList );

   if ( vertList.size() >= 2 )
   {
      // Only divides one edge, deal with it
      unsigned int v1 = vertList.front();
      vertList.pop_front();
      unsigned int v2 = vertList.front();

      double coord1[3];
      double coord2[3];

      model->getVertexCoords( v1, coord1 );
      model->getVertexCoords( v2, coord2 );

      for ( int i = 0; i < 3; i++ )
      {
         coord1[i] = (coord1[i] + coord2[i]) / 2.0;
      }

      unsigned int v3 = model->addVertex( 
            coord1[0], coord1[1], coord1[2] );

      // The triangle count will grow while we're iterating, but that's okay
      // because we know that the new triangles don't need to be split
      unsigned int tcount = model->getTriangleCount();

      for ( unsigned int tri = 0; tri < tcount; tri++ )
      {
         const int INVALID = ~0;
         int a = INVALID;
         int b = INVALID;
         int c = INVALID;

         unsigned int tv[3] = {0,0,0};

         model->getTriangleVertices( tri, tv[0], tv[1], tv[2] );

         for ( int i = 0; i < 3; i++ )
         {
            if ( tv[i] == v1 )
               a = i;
            else if ( tv[i] == v2 )
               b = i;
            else
               c = i;
         }

         // don't assume 'c' is set, triangle may have one vertex
         // assigned to two corners
         if ( a != INVALID && b != INVALID && c != INVALID )
         {
            int g = model->getTriangleGroup( tri );

            int vert[3];

            vert[ a ] = v3;
            vert[ b ] = tv[b];
            vert[ c ] = tv[c];
            int newTri = model->addTriangle( vert[0], vert[1], vert[2] );
            if ( g >= 0 )
            {
               model->addTriangleToGroup( newTri, g );
            }

            vert[ a ] = tv[a];
            vert[ b ] = v3;
            vert[ c ] = tv[c];
            model->setTriangleVertices( tri, vert[0], vert[1], vert[2] );
            split++;
         }
      }

      model->unselectAllVertices();
      model->selectVertex( v3 );
   }

   if ( split > 0 )
   {
      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Edge Divide complete" ).toUtf8() );
      return true;
   }
   else
   {
      model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "You must have at 2 adjacent vertices to Edge Divide" ).toUtf8() );
   }
   return false;
}

const char * EdgeDivideCommand::getPath()
{
   return GEOM_FACES_MENU;
}

const char * EdgeDivideCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Edge Divide" );
}

