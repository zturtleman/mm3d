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
#include "faceoutcmd.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"

#include <list>
#include <QtCore/QObject>
#include <QtWidgets/QApplication>

using std::list;

FaceOutCommand::FaceOutCommand()
{
}

FaceOutCommand::~FaceOutCommand()
{
}

bool FaceOutCommand::activated( int arg, Model * model )
{
   // TODO this is slow, see if we can speed it up
   list<int> faces;
   model->getSelectedTriangles( faces );
   list<int>::iterator it;

   for ( it = faces.begin(); it != faces.end(); it++ )
   {
      if ( model->triangleFacesIn( *it ) )
      {
         model->invertNormals( *it );
      }
   }

   model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Normals Face Out" ).toUtf8() );

   return true;
}

const char * FaceOutCommand::getPath()
{
   return GEOM_NORMALS_MENU;
}

