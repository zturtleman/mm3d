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


#include "pastecmd.h"

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
#include <QtCore/QObject>
#include <QtWidgets/QApplication>

using std::list;
using std::map;

PasteCommand::PasteCommand()
{
}

PasteCommand::~PasteCommand()
{
}

bool PasteCommand::activated( int arg, Model * model )
{
   if ( model )
   {
      Model * m = new Model;

      std::string clipfile = getMm3dHomeDirectory();

      clipfile += "/clipboard";
      mkpath( clipfile.c_str(), 0755 );
      clipfile += "/clipboard.mm3d";

      FilterManager::getInstance()->readFile( m, clipfile.c_str() );

      model->mergeModels( m, true, Model::AM_NONE, false );

      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Paste complete" ).toUtf8() );

      delete m;

      return true;
   }
   else
   {
      return false;
   }
}

const char * PasteCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Paste from Clipboard" );
}

