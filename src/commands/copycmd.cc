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


#include "copycmd.h"

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
#include <qobject.h>
#include <qapplication.h>

using std::list;
using std::map;

CopyCommand::CopyCommand()
{
}

CopyCommand::~CopyCommand()
{
}

bool CopyCommand::activated( int arg, Model * model )
{
   if ( !model )
      return false;

   if ( model->getSelectedTriangleCount() == 0
         && model->getSelectedPointCount() == 0
         && model->getSelectedProjectionCount() == 0 )
   {
      model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "You must have at least 1 face, joint, or point selected to Copy" ).utf8() );
      return false;
   }

   Model * m = model->copySelected();

   if ( !m ) 
      return false;

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Selected primitives copied" ).utf8() );
   std::string clipfile = getMm3dHomeDirectory();

   clipfile += "/clipboard";
   mkpath( clipfile.c_str(), 0755 );
   clipfile += "/clipboard.mm3d";

   FilterManager::getInstance()->writeFile( m, clipfile.c_str(), FilterManager::WO_ModelNoPrompt );

   delete m;

   return true;
}

const char * CopyCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Copy Selected to Clipboard" );
}

