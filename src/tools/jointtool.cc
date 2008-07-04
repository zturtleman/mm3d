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
#include "jointtool.h"

#include "model.h"
#include "msg.h"
#include "log.h"
#include "modelstatus.h"

#include "pixmap/jointtool.xpm"

#include <QtCore/QObject>
#include <QtGui/QApplication>

JointTool::JointTool()
{
   m_joint.pos.type = Model::PT_Point;
}

JointTool::~JointTool()
{
}


void JointTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   double coord[3] = {0,0,0};

   parent->getParentXYValue( x, y, coord[0], coord[1], true );

   const Matrix & viewMatrix = parent->getParentViewMatrix();

   int p = -1;
   double pDist = 0.0;
   double parentCoords[4];
   int jointCount = model->getBoneJointCount();

   for ( int t = 0; t < jointCount; t++ )
   {
      model->getBoneJointCoords( t, parentCoords );
      parentCoords[3] = 1;
      viewMatrix.apply( parentCoords );

      double dist = distance( coord[0], coord[1], parentCoords[0], parentCoords[1] );
      if ( p == -1 || dist < pDist )
      {
         p = t;
         pDist = dist;
      }
   }

   // Find a unique name for the joint
   char name[32] = "Joint 1";
   unsigned c = model->getBoneJointCount();
   bool uniqueName = (c == 0) ? true : false;

   for ( unsigned i = 1; !uniqueName && i < 1000; i++ )
   {
      uniqueName = true;
      sprintf( name, "Joint %d", i );
      for ( unsigned j = 0; j < c; j++ )
      {
         if ( strcmp( name, model->getBoneJointName( j ) ) == 0 )
         {
            uniqueName = false;
            break;
         }
      }
   }

   // I give up, just call it "Joint"
   if ( ! uniqueName )
   {
      strcpy( name, "Joint" );
   }

   m_joint = addPosition( parent, Model::PT_Joint, name, 
         coord[0], coord[1], coord[2], 0, 0, 0, p );

   model->unselectAll();
   model->selectBoneJoint( m_joint.pos.index );

   parent->updateAllViews();

   if ( p >= 0 )
   {
      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Joint created" ).toUtf8() );
   }
   else
   {
      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Root joint created" ).toUtf8() );
   }
}

void JointTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   if ( m_joint.pos.type == Model::PT_Joint )
   {
      double coord[3] = {0,0,0};

      parent->getParentXYValue( x, y, coord[0], coord[1] );

      movePosition( parent, m_joint.pos, 
            coord[0], coord[1], coord[2] );

      parent->updateAllViews();
   }
}

void JointTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
}

const char ** JointTool::getPixmap()
{
   return (const char **) jointtool_xpm;
}

const char * JointTool::getPath()
{
   return TOOLS_CREATE_MENU;
}

const char * JointTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Create Bone Joint" );
}

