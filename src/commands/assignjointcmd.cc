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


#include "assignjointcmd.h"

#include "jointwin.h"
#include "model.h"
#include "modelstatus.h"
#include "log.h"

#include <list>
#include <QtCore/QObject>
#include <QtGui/QApplication>

AssignJointCommand::AssignJointCommand()
{
}

AssignJointCommand::~AssignJointCommand()
{
}

bool AssignJointCommand::activated( int arg, Model * model )
{
   std::list<int> j;
   model->getSelectedBoneJoints( j );

   if ( j.size() == 1 )
   {
      unsigned int joint = j.front();

      list<int>::iterator it;

      list<int> vertList;
      model->getSelectedVertices( vertList );
      list<int> pointList;
      model->getSelectedPoints( pointList );

      log_debug( "assigning %d vertices and %d points to joint %d\n", vertList.size(), pointList.size(), joint );

      QString str = qApp->translate( "Command", "Assigning %1 vertices and %2 points to joint %3" );
      str = str
         .arg(vertList.size())
         .arg(pointList.size())
         .arg(joint);
      model_status( model, StatusNormal, STATUSTIME_SHORT, "%s",
            (const char *) str.toUtf8() );

      for ( it = vertList.begin(); it != vertList.end(); it++ )
      {
         model->setVertexBoneJoint( *it, joint );
      }

      for ( it = pointList.begin(); it != pointList.end(); it++ )
      {
         model->setPointBoneJoint( *it, joint );
      }
   }
   else
   {
      model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "You must have exactly 1 bone joint selected." ).toUtf8() );
      JointWin * win = new JointWin( model );  
      win->show();
   }
   return true;
}

bool AssignJointCommand::getKeyBinding( int arg, int & keyBinding )
{
   keyBinding = Qt::CTRL + Qt::Key_B;
   return true;
}


const char * AssignJointCommand::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Command", "Assign Selected to Bone Joint" );
}

