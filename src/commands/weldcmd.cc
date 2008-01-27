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
#include "weldcmd.h"

#include "model.h"
#include "glmath.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"

#include "weld.h"

#include <list>
#include <map>
#include <QtCore/QObject>
#include <QtGui/QApplication>

using std::list;
using std::map;

WeldCommand::WeldCommand()
{
}

WeldCommand::~WeldCommand()
{
}

bool WeldCommand::activated( int arg, Model * model )
{
   if ( model )
   {
      list<int> vert;
      model->getSelectedVertices( vert );

      if ( vert.size() > 1 )
      {
         int unwelded = 0;
         int welded = 0;
         weldSelectedVertices( model, 0.0001, unwelded, welded );
         model_status( model, StatusNormal, STATUSTIME_SHORT, (qApp->translate( "Command", "Welded %1 vertices into %2 vertices" ).arg(unwelded).arg(welded)).toUtf8() );
         return true;
      }
      else
      {
         model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "You must have 2 or more vertices selected to weld." ).toUtf8() );
      }
   }
   
   return false;
}

const char * WeldCommand::getPath()
{
   return GEOM_VERTICES_MENU;
}

const char * WeldCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Weld Vertices" );
}

