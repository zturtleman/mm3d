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


#include "pluginmgr.h"
#include "sysconf.h"
#include "qtmain.h"
#include "stdcmds.h"
#include "stdtools.h"
#include "stdfilters.h"
#include "stdtexfilters.h"
#include "cmdline.h"
#include "3dmprefs.h"
#include "model.h"
#include "log.h"
#include "cmdmgr.h"
#include "decalmgr.h"
#include "filtermgr.h"
#include "texmgr.h"
#include "allocstats.h"
#include "statusbar.h"
#include "modelstatus.h"
#include "keycfg.h"
#include "transimp.h"

#include "misc.h" // FIXME

#include <signal.h>

void segfault_handler( int sig )
{
   fprintf( stderr, "Segfault.  Exiting...\n" );
   exit( 0 );
}

int free_memory()
{
   return model_free_primitives();
}

int main( int argc, char * argv[] )
{
   int rval = 0;
   
   /*
   // FIXME begin test
   char cstr[]      = "hiragana: 平仮名foo";
   std::string sstr = "hiragana: 平仮名foo";

   printf( "cstr length = %d/%d\n", strlen( cstr ), utf8len( cstr ) );
   printf( "sstr length = %d/%d\n", strlen( sstr.c_str() ), utf8len( sstr ) );

   printf( "cstr is '%s'\n", cstr );
   printf( "sstr is '%s'\n", sstr.c_str() );

   utf8chrtrunc( cstr, 22 );
   utf8chrtrunc( sstr, 22 );

   printf( "cstr is '%s'\n", cstr );
   printf( "sstr is '%s'\n", sstr.c_str() );

   utf8chrtrunc( cstr, 15 );
   utf8chrtrunc( sstr, 15 );

   printf( "cstr length = %d/%d\n", strlen( cstr ), utf8len( cstr ) );
   printf( "sstr length = %d/%d\n", strlen( sstr.c_str() ), utf8len( sstr ) );

   printf( "cstr is '%s'\n", cstr );
   printf( "sstr is '%s'\n", sstr.c_str() );

   return 0;
   // FIXME end test
   */

   log_profile_init( "profile_data.txt" );

   signal( SIGSEGV, segfault_handler );

   init_sysconf();
   transimp_install_translator();

   ui_prep( argc, argv );

   {
      LOG_PROFILE();

      {
         LOG_PROFILE_STR( "Initialize" );

         init_prefs();
         init_cmdline( argc, argv );

         // set up keyboard shortcuts
         std::string keycfgFile = getMm3dHomeDirectory();
         keycfgFile += "/keycfg.in";
         keycfg_load_file( keycfgFile.c_str() );
         keycfg_set_defaults();

         init_std_filters();
         init_std_texture_filters();
         init_std_tools();
         init_std_cmds( CommandManager::getInstance() );

         init_plugins();

         model_status_register_function( StatusBar::getStatusBarFromModel );
      }

      rval = ui_init( argc, argv );

      {
         LOG_PROFILE_STR( "Uninitialize" );

         std::string keycfgFile = getMm3dHomeDirectory();
         keycfgFile += "/keycfg.out";
         keycfg_save_file( keycfgFile.c_str() );

         shutdown_cmdline();

         model_show_alloc_stats();

         DecalManager::release();
         CommandManager::release();
         FilterManager::release();
         TextureManager::release();
         PluginManager::release();

         free_memory();

         model_show_alloc_stats();
         show_alloc_stats();

         prefs_save();
      }
   }

   log_profile_shutdown();

   return rval;
}
