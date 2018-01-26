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


#include "flattencmd.h"

#include "model.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"

#include <list>
#include <QtCore/QObject>
#include <QtWidgets/QApplication>

using std::list;

FlattenCommand::FlattenCommand()
{
}

FlattenCommand::~FlattenCommand()
{
}

const char * FlattenCommand::getName( int arg )
{
   switch ( arg )
   {
      case 0:
         return QT_TRANSLATE_NOOP( "Command", "Flatten" );
         break;
      case 1:
         return QT_TRANSLATE_NOOP( "Command", "Flatten X" );
         break;
      case 2:
         return QT_TRANSLATE_NOOP( "Command", "Flatten Y" );
         break;
      case 3:
         return QT_TRANSLATE_NOOP( "Command", "Flatten Z" );
         break;
      default:
         break;
   }

   return "[Out of range]";
}

bool FlattenCommand::activated( int arg, Model * model )
{
   int index;

   index = arg - 1;

   // Check for index out of range
   if ( index < 0 || index > 2 )
   {
      log_error( "flatten on index %d out of range", index );
      return false;
   }

   list<Model::Position> posList;
   model->getSelectedPositions( posList );

   if ( posList.empty() )
   {
      model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "Need at least 1 vertex, joint, point, or face selected" ).toUtf8() );
      return false;
   }

   float newVal   = 0.0f;
   int   countVal = 0;

   double coords[3];
   list<Model::Position>::iterator it;
   for ( it = posList.begin(); it != posList.end(); it++ )
   {
      model->getPositionCoords( *it, coords );
      newVal += coords[index];
      countVal++;
   }

   newVal = newVal / (float) countVal;

   for ( it = posList.begin(); it != posList.end(); it++ )
   {
      model->getPositionCoords( *it, coords );
      coords[index] = newVal;
      model->movePosition( *it, coords[0], coords[1], coords[2] );
   }

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Selected primitives flattened" ).toUtf8() );

   return true;
}

