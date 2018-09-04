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


#include <QtCore/QObject>
#include <QtWidgets/QApplication>
#include <QtGui/QKeySequence>

#include "deletecmd.h"
#include "model.h"
#include "msg.h"
#include "modelstatus.h"

DeleteCommand::DeleteCommand()
{
}

DeleteCommand::~DeleteCommand()
{
}

const char * DeleteCommand::getName( int arg )
{
   if ( arg == 0 )
   {
      return QT_TRANSLATE_NOOP( "Command", "Delete" );
   }
   else
   {
      return "[Out of range]";
   }
}

bool DeleteCommand::getKeyBinding( int arg, int & keyBinding )
{
   if ( arg == 0 )
   {
      keyBinding = Qt::Key_Delete;
      return true;
   }
   else
   {
      return false;
   }
}

bool DeleteCommand::activated( int arg, Model * model )
{
   if ( arg == 0 && model )
   {
      static bool warnedAlready = false;
      std::list<int> joints;
      model->getSelectedBoneJoints( joints );

      bool doDelete = true;

      if ( model->getAnimationMode() == Model::ANIMMODE_NONE && joints.size() > 0 && model->getAnimCount( Model::ANIMMODE_SKELETAL ) > 0 && !warnedAlready )
      {
         QString s = qApp->translate( "Command", "Deleting joints may destroy skeletal animations\nDo you wish to continue?" );
         if ( msg_warning_prompt( (const char *) s.toUtf8(), "yN" ) == 'Y' )
         {
            warnedAlready = true;
         }
         else
         {
            doDelete = false;
         }
      }

      if ( doDelete )
      {
         model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Primitives deleted" ).toUtf8() );
         model->deleteSelected();
      }
      return doDelete;
   }
   else
   {
      return false;
   }
}

