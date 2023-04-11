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
#include "offsetcmd.h"

#include "offsetwin.h"
#include "log.h"

#include <QtCore/QObject>
#include <QtWidgets/QApplication>

OffsetCommand::OffsetCommand()
{
}

OffsetCommand::~OffsetCommand()
{
}

bool OffsetCommand::activated( int arg, Model * model )
{
   OffsetWin * win = new OffsetWin( model, NULL );  
   win->show();
   return true;
}

const char * OffsetCommand::getPath()
{
   return GEOM_VERTICES_MENU;
}

const char * OffsetCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Offset by Normal..." );
}

