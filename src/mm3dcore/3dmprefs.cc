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


#include "3dmprefs.h"
#include "prefparse.h"
#include "log.h"
#include "mm3dport.h"
#include "sysconf.h"
#include "misc.h"

#include <string>
#include <sys/stat.h>
#include <sys/types.h>

Preferences g_prefs;

static void _3dmprefs_makerc()
{
   struct stat stbuf;

   std::string temp = getMm3dHomeDirectory().c_str();

   mkpath( temp.c_str(), 0755 );

   temp = std::string( getPluginDirectory().c_str() );

   bool doSymlink = false;
   if ( stat( temp.c_str(), &stbuf ) != 0 )
   {
      doSymlink = true;
   }

   mkpath( temp.c_str(), 0755 );

   if ( doSymlink )
   {
      // This bit is ignored on Win32
      temp += "/shared";
      PORT_symlink( getSharedPluginDirectory().c_str(), temp.c_str() );
   }
}

static void _3dmprefs_defaults()
{
   // None, most modules handle their own defaults
}

static std::string _getFilename()
{
   std::string filename = getConfigFile().c_str();

   return filename;
}

void prefs_set_pref( const char * key, const char * value )
{
   g_prefs( key ) = value;
}

void prefs_save()
{
   FILE * fp = fopen( _getFilename().c_str(), "w" );
   g_prefs.print( fp );
   fclose( fp );
}

bool prefs_load()
{
   FILE * fp = fopen( _getFilename().c_str(), "r" );
   if ( fp )
   {
      bool rval = prefparse_do_parse( fp, g_prefs );
      fclose( fp );
      return rval;
   }
   else
   {
      log_warning( "no config file\n" );
      return false;
   }
}

void prefs_recent_model( const std::string & filename )
{
   for ( unsigned i = 0; i < g_prefs("mru").count(); )
   {
      if ( g_prefs( "mru" )[i].stringValue() == filename )
      {
         g_prefs("mru").remove( i );
      }
      else
      {
         i++;
      }
   }

   PrefItem item;
   item = std::string(filename);
   g_prefs( "mru" ).insert( 0, item );

   while( g_prefs("mru").count() > 25 )
   {
      g_prefs("mru").remove( 25 );
   }
}

void prefs_recent_script( const std::string & filename )
{
   for ( unsigned i = 0; i < g_prefs("script_mru").count(); )
   {
      if ( g_prefs( "script_mru" )[i].stringValue() == filename )
      {
         g_prefs("script_mru").remove( i );
      }
      else
      {
         i++;
      }
   }

   PrefItem item;
   item = std::string(filename);
   g_prefs( "script_mru" ).insert( 0, item );

   while( g_prefs("script_mru").count() > 25 )
   {
      g_prefs("script_mru").remove( 25 );
   }
}

int init_prefs()
{
   _3dmprefs_makerc();
   _3dmprefs_defaults();
   prefs_load();
   return 0;
}

