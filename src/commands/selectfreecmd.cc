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
#include <QKeySequence>

#include "selectfreecmd.h"
#include "model.h"
#include "msg.h"
#include "modelstatus.h"

#include <qobject.h>
#include <qapplication.h>

SelectFreeCommand::SelectFreeCommand()
{
}

SelectFreeCommand::~SelectFreeCommand()
{
}

const char * SelectFreeCommand::getPath()
{
   return GEOM_VERTICES_MENU;
}

const char * SelectFreeCommand::getName( int arg )
{
   if ( arg == 0 )
   {
      return QT_TRANSLATE_NOOP( "Command", "Select Free Vertices" );
   }
   else
   {
      return "[Out of range]";
   }
}

bool SelectFreeCommand::activated( int arg, Model * model )
{
   if ( model )
   {
      model->selectFreeVertices();
      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Free-floating vertices selected" ).toUtf8() );
      return true;
   }
   else
   {
      return false;
   }
}


