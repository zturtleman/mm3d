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
#include "makefacecmd.h"
#include "model.h"
#include "msg.h"
#include "modelstatus.h"
#include "cmdmgr.h"
#include "log.h"

#include <QtCore/QObject>
#include <QtWidgets/QApplication>

MakeFaceCommand::MakeFaceCommand()
{
}

MakeFaceCommand::~MakeFaceCommand()
{
}

const char * MakeFaceCommand::getPath()
{
   return GEOM_VERTICES_MENU;
}

const char * MakeFaceCommand::getName( int arg )
{
   if ( arg == 0 )
   {
      return QT_TRANSLATE_NOOP( "Command", "Make Face From Vertices" );
   }
   else
   {
      return "[Out of range]";
   }
}

bool MakeFaceCommand::getKeyBinding( int arg, int & keyBinding )
{
   return false;
}

bool MakeFaceCommand::activated( int arg, Model * model )
{
   if ( arg == 0 && model )
   {
      if ( model->getAnimationMode() == Model::ANIMMODE_NONE )
      {
         std::list<int> verts;
         model->getSelectedVertices( verts );
         if ( verts.size() == 3 )
         {
            model_status( model, StatusNormal, STATUSTIME_SHORT, "%s", qApp->translate( "Command", "Face created" ).toUtf8().data() );
            int v1, v2, v3;
            std::list<int>::iterator it = verts.begin();

            v1 = *it;
            it++;
            v2 = *it;
            it++;
            v3 = *it;

            model->addTriangle( v1, v2, v3 );
            return true;
         }
         else
         {
            model_status( model, StatusError, STATUSTIME_LONG, "%s", qApp->translate( "Command", "Must select exactly 3 vertices" ).toUtf8().data() );
         }
      }
   }
   return false;
}

