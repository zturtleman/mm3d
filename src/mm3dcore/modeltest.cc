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


#include "modeltest.h"
#include "log.h"
#include "filtermgr.h"
#include "model.h"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int modelTestRun( const char * modelDir )
{
   int failures = 0;

   FilterManager * mgr = FilterManager::getInstance();

   DIR * dp = opendir( modelDir );

   if ( dp )
   {
      struct dirent * d;

      std::string filename   = "";
      std::string samename   = "";
      std::string nativename = "";

      while ( (d = readdir( dp )) != NULL )
      {
         if ( strcmp( d->d_name, "." ) != 0 
               && strcmp( d->d_name, ".." ) != 0 )
         {
            printf( "testing %s\n", d->d_name );
            if ( strstr( d->d_name, ".mm3d" ) )
            {
            }
            else
            {
               filename  = modelDir;
               filename += "/";

               samename   = filename + "temptestfile";
               nativename = filename + "temptestfile";

               filename += d->d_name;

               char * ptr = strrchr( d->d_name, '.' );

               if ( ptr )
               {
                  samename   += ptr;
                  nativename += ".mm3d";

                  Model * orig   = new Model;
                  Model * same   = new Model;
                  Model * native = new Model;

                  int bits = Model::CompareAll & (~Model::CompareMeta);

                  if ( mgr->readFile( orig, filename.c_str() ) == Model::ERROR_NONE )
                  {
                     mgr->writeFile( orig, nativename.c_str(), true, FilterManager::WO_ModelNoPrompt );
                     mgr->writeFile( orig, samename.c_str(), true, FilterManager::WO_ModelNoPrompt );

                     if ( mgr->readFile( same, samename.c_str() ) == Model::ERROR_NONE )
                     {
                        int b = orig->equivalent( same, bits, 0.0001 );
                        if ( b == bits )
                        {
                           printf( "  %s success\n", samename.c_str() );
                        }
                        else
                        {
                           printf( "  %s failed (%02x/%02x)\n", samename.c_str(), b, bits );
                           failures++;
                        }
                     }
                     else
                     {
                        printf( "  %s: could not read\n", samename.c_str() );
                        failures++;
                     }
                     unlink( samename.c_str() );

                     if ( mgr->readFile( native, nativename.c_str() ) == Model::ERROR_NONE )
                     {
                        int b = orig->equivalent( native, bits, 0.0001 );
                        if ( b == bits )
                        {
                           printf( "  %s success\n", nativename.c_str() );
                        }
                        else
                        {
                           printf( "  %s failed (%02x/%02x)\n", nativename.c_str(), b, bits );
                           failures++;
                        }
                     }
                     else
                     {
                        printf( "  %s: could not read\n", nativename.c_str() );
                        failures++;
                     }
                     unlink( nativename.c_str() );
                  }
                  else
                  {
                     printf( "  %s: could not read\n", filename.c_str() );
                  }

                  delete orig;
                  delete same;
                  delete native;
               }
            }
         }
      }

      closedir( dp );
   }
   else
   {
      log_error( "%s: %s\n", strerror(errno) );
   }

   return failures;
}
