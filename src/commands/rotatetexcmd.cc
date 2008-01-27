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
#include "rotatetexcmd.h"
#include "model.h"
#include "msg.h"
#include "modelstatus.h"
#include "cmdmgr.h"
#include "log.h"

#include <QtCore/QObject>
#include <QtGui/QApplication>

RotateTextureCommand::RotateTextureCommand()
{
}

RotateTextureCommand::~RotateTextureCommand()
{
}

const char * RotateTextureCommand::getPath()
{
   return GEOM_FACES_MENU;
}

const char * RotateTextureCommand::getName( int arg )
{
   switch ( arg )
   {
      case 0:
         return QT_TRANSLATE_NOOP( "Command", "Rotate Texture Coordinates" );
         break;
      case 1:
         return QT_TRANSLATE_NOOP( "Command", "Face" );
         break;
      case 2:
         return QT_TRANSLATE_NOOP( "Command", "Group" );
         break;
      default:
         break;
   }
   return "[Out of range]";
}

bool RotateTextureCommand::getKeyBinding( int arg, int & keyBinding )
{
   return false;
}

bool RotateTextureCommand::activated( int arg, Model * model )
{
   if ( arg == 1 && model )
   {
      if ( model->getAnimationMode() == Model::ANIMMODE_NONE )
      {
         std::list<int> tris;
         model->getSelectedTriangles( tris );
         if ( tris.size() > 0 )
         {
            std::list<int>::iterator it;

            float s, t;
            float oldS, oldT;

            for ( it = tris.begin(); it != tris.end(); it++ )
            {
               model->getTextureCoords( *it, 2, oldS, oldT );

               model->getTextureCoords( *it, 1, s, t );
               model->setTextureCoords( *it, 2, s, t );

               model->getTextureCoords( *it, 0, s, t );
               model->setTextureCoords( *it, 1, s, t );

               model->setTextureCoords( *it, 0, oldS, oldT );
            }

            model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Texture coordinates rotated" ).toUtf8() );
            return true;
         }
         else
         {
            model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "Must select faces" ).toUtf8() );
         }
      }
   }
   if ( arg == 2 && model )
   {
      if ( model->getAnimationMode() == Model::ANIMMODE_NONE )
      {
         std::list<int> tris;
         model->getSelectedTriangles( tris );
         if ( tris.size() > 0 )
         {
            std::list<int>::iterator it;

            float s;
            float t;
            float temp;

            for ( it = tris.begin(); it != tris.end(); it++ )
            {
               model->getTextureCoords( *it, 0, s, t );
               temp = s;
               s = 0.5 - (t - 0.5);
               t = temp;
               model->setTextureCoords( *it, 0, s, t );

               model->getTextureCoords( *it, 1, s, t );
               temp = s;
               s = 0.5 - (t - 0.5);
               t = temp;
               model->setTextureCoords( *it, 1, s, t );

               model->getTextureCoords( *it, 2, s, t );
               temp = s;
               s = 0.5 - (t - 0.5);
               t = temp;
               model->setTextureCoords( *it, 2, s, t );

            }

            model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Command", "Texture coordinates rotated" ).toUtf8() );
            return true;
         }
         else
         {
            model_status( model, StatusError, STATUSTIME_LONG, qApp->translate( "Command", "Must select faces" ).toUtf8() );
         }
      }
   }
   return false;
}

