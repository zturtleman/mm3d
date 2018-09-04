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


#include "modelutil.h"

#include "model.h"
#include "filtermgr.h"
#include "log.h"
#include "msg.h"
#include "translate.h"

#include <stdio.h>
#include <stdlib.h>

Model::ModelErrorE modelutil_saveAs( const char * infile, const char * outfile )
{
   Model * model = new Model;

   Model::ModelErrorE err = Model::ERROR_NONE;
   if ( Model::ERROR_NONE == (err = FilterManager::getInstance()->readFile( model, infile )) )
   {
      if ( (err = modelutil_saveAs( model, outfile )) != Model::ERROR_NONE )
      {
         std::string msg = infile;
         msg += ": ";
         msg += transll( Model::errorToString( err, model ) );
         msg += "\n";
         msg_error( msg.c_str() );
      }
   }
   else
   {
      std::string msg = infile;
      msg += ": ";
      msg += transll( Model::errorToString( err, model ) );
      msg += "\n";
      msg_error( msg.c_str() );
   }

   delete model;
   return err;
}

Model::ModelErrorE modelutil_saveAsFormat( const char * infile, const char * outformat )
{
   unsigned len = strlen( infile ) + strlen(outformat) + 2;
   char * filename = new char[len];
   strcpy( filename, infile );
   char * extension = strrchr( filename, '.' );
   if ( extension )
   {
      extension++;
   }
   else
   {
      extension = &filename[strlen(infile)];
      extension[0] = '.';
      extension++;
   }
   strcpy( extension, outformat );
   log_debug( "running convert on %s to %s\n", infile, filename );

   Model::ModelErrorE err = modelutil_saveAs( infile, filename );

   delete[] filename;
   return err;
}

Model::ModelErrorE modelutil_saveAs( Model * model, const char * outfile )
{
   Model::ModelErrorE err = Model::ERROR_NONE;
   if ( Model::ERROR_NONE != (err = FilterManager::getInstance()->writeFile( model, outfile )) )
   {
      std::string msg = outfile;
      msg += ": ";
      msg += transll( Model::errorToString( err, model ) );
      msg += "\n";
      msg_error( msg.c_str() );
   }
   return err;
}

Model::ModelErrorE modelutil_saveAsFormat( Model * model, const char * outformat )
{
   return Model::ERROR_UNSUPPORTED_OPERATION;
}
