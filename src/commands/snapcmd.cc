/*  Misfit Model 3D
 * 
 *  Copyright (c) 2005 Johannes Kroll
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

// Integrated into MM3D by Kevin Worcester
// This code is modified from the snap_together plugin written by Johannes Kroll

#include "menuconf.h"
#include "snapcmd.h"

#include "cmdmgr.h"
#include "model.h"
#include "pluginapi.h"
#include "version.h"
#include "log.h"
#include "weld.h"

#include <list>

#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <QtCore/QObject>
#include <QtWidgets/QApplication>


SnapCommand::SnapCommand()
{
}

SnapCommand::~SnapCommand()
{
}


// set the coordinates of all the selected vertices to their arithmetic mean.
static void snap_together(Model *model, list<int> selection)
{
   double coord[3]= { 0, 0, 0 }, coord_temp[3];
   list<int>::iterator iter;

   for(iter= selection.begin(); iter!=selection.end(); iter++)
   {
      model->getVertexCoords(*iter, coord_temp);
      coord[0]+= coord_temp[0];
      coord[1]+= coord_temp[1];
      coord[2]+= coord_temp[2];
   }

   coord[0]/= selection.size();
   coord[1]/= selection.size();
   coord[2]/= selection.size();

   for(iter= selection.begin(); iter!=selection.end(); iter++)
   {
      model->moveVertex(*iter, coord[0], coord[1], coord[2]);
   }
}


// snap 2 vertices together, delete one of them, and take care of affected triangles.
static void snap_together_two(Model *model, int v0, int v1)
{
   double coord_v0[3], coord_v1[3];
   model->getVertexCoords(v0, coord_v0);
   model->getVertexCoords(v1, coord_v1);
   coord_v0[0]= (coord_v0[0]+coord_v1[0]) / 2;
   coord_v0[1]= (coord_v0[1]+coord_v1[1]) / 2;
   coord_v0[2]= (coord_v0[2]+coord_v1[2]) / 2;
   model->moveVertex(v0, coord_v0[0], coord_v0[1], coord_v0[2]);
   model->moveVertex(v1, coord_v0[0], coord_v0[1], coord_v0[2]);
}


// find the nearest vertex in the selection which is not contained in the exclude (already processed) list.
#define DISTANCE(x, y, z) sqrt( (x)*(x) + (y)*(y) + (z)*(z) )
int find_nearest_vertex(Model *model, int vertex, list<int> selection, list<int> exclude)
{
   double smallest_distance= 0xdeadbeef;
   int nearest_vertex= -1;
   double coord[3], coord_temp[3];
   list<int>::iterator iter;

   model->getVertexCoords(vertex, coord);

   for(iter= selection.begin(); iter!=selection.end(); iter++)
   {
      if(*iter!=vertex)
      {
         list<int>::iterator it= exclude.begin();
         while(it!=exclude.end() && *it != *iter) it++;
         if( *it != *iter )
         {
            model->getVertexCoords( *iter, coord_temp );
            double distance= DISTANCE(coord_temp[0]-coord[0], coord_temp[1]-coord[1], coord_temp[2]-coord[2]);
            if(distance < smallest_distance)
            {
               smallest_distance= distance;
               nearest_vertex= *iter;
            }
         }
      }
   }
   return nearest_vertex;
}


const char * SnapCommand::getPath()
{
   return GEOM_VERTICES_MENU;
}

const char * SnapCommand::getName( int arg )
{
   switch ( arg )
   {
      case 0:
      default:
         return QT_TRANSLATE_NOOP( "Command", "Snap Vertices Together" );
         break;
      case 1:
         return QT_TRANSLATE_NOOP( "Command", "Snap All Selected" );
         break;
      case 2:
         return QT_TRANSLATE_NOOP( "Command", "Snap Nearest Selected" );
         break;
      case 3:
         return QT_TRANSLATE_NOOP( "Command", "Snap All and Weld" );
         break;
      case 4:
         return QT_TRANSLATE_NOOP( "Command", "Snap Nearest and Weld" );
         break;
   }
   return "";
}

bool SnapCommand::activated( int arg, Model * model )
{
   list<int> selection;
   model->getSelectedVertices( selection );

   if ( selection.size() >= 2 )
   {
      if ( arg == 1 || arg == 3 )
      {
         snap_together(model, selection);
      }
      else
      {
         list<int> excludelist;
         list<int>::iterator sel_iter;

         for ( sel_iter = selection.begin(); sel_iter != selection.end(); sel_iter++ )
         {
            list<int>::iterator it = excludelist.begin();
            while ( it != excludelist.end() && *it != *sel_iter )
            {
               it++;
            }

            if ( *it != *sel_iter )
            {
               int nearest_vertex= find_nearest_vertex(model, *sel_iter, selection, excludelist);
               if ( nearest_vertex != -1 )
               {
                  snap_together_two( model, *sel_iter, nearest_vertex );
                  excludelist.push_back(*sel_iter);
                  excludelist.push_back(nearest_vertex);
               }
            }
         }
      }

      if ( arg == 3 || arg == 4 )
      {
         weldSelectedVertices( model );
      }

      model->deleteOrphanedVertices();
      return true;
   }
   else
   {
      return false;
   }
}



#ifdef PLUGIN

//------------------------------------------------------------------
// Plugin functions
//------------------------------------------------------------------

PLUGIN_API bool plugin_init()
{
   CommandManager::getInstance()->registerCommand( new SnapCommand() );

   log_debug( "Snap Together Plugin Initialized\n" );
   return true;
}

// We have no cleanup to do
PLUGIN_API bool plugin_uninit()
{
   return true;
}

PLUGIN_API const char * plugin_mm3d_version()
{
   return VERSION_STRING;
}

PLUGIN_API const char * plugin_version()
{
   return "1.0.0";
}

PLUGIN_API const char * plugin_desc()
{
   return "Snap Vertices Together";
}

#endif // PLUGIN

