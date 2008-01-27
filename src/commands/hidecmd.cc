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


#include "hidecmd.h"

#include "model.h"
#include "modelstatus.h"
#include <QtCore/QObject>
#include <QtGui/QApplication>
#include <QtGui/QKeySequence>

HideCommand::HideCommand()
{
}

HideCommand::~HideCommand()
{
}

const char * HideCommand::getName( int arg )
{
   switch ( arg )
   {
      case 0:
         return QT_TRANSLATE_NOOP( "Command", "Hide" );
      case 1:
         return QT_TRANSLATE_NOOP( "Command", "Hide Unselected" );
      case 2:
         return QT_TRANSLATE_NOOP( "Command", "Hide Selected" );
      case 3:
         return QT_TRANSLATE_NOOP( "Command", "Unhide All" );
      default:
         break;
   }

   return "[Out of range]";
}

bool HideCommand::getKeyBinding( int arg, int & keyBinding )
{
   switch ( arg )
   {
      case 0:
         break;
      case 1:
         keyBinding = Qt::Key_H;
         return true;
      case 2:
         keyBinding = Qt::Key_H + Qt::SHIFT;
         return true;
      case 3:
         keyBinding = Qt::Key_U;
         return true;
      default:
         break;
   }

   return false;
}

bool HideCommand::activated( int arg, Model * model )
{
   switch ( arg )
   {
      case 2:
         model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Selected primitives hidden" ).toUtf8() );
         model->hideSelected();
         return true;
         break;
      case 3:
         model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Primitives unhidden" ).toUtf8() );
         model->unhideAll();
         return true;
         break;
      default:
         model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Unselected primitives hidden" ).toUtf8() );
         model->hideUnselected();
         return true;
         break;
   }

   return false;
}

