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


#include "3dmprefs.h"
#include "prefparse.h"
#include "log.h"
#include "mm3dport.h"
#include "sysconf.h"
#include "misc.h"
#include "filedatadest.h"
#include "filedatasource.h"

#include <string>
#include <sys/stat.h>
#include <sys/types.h>

Preferences g_prefs;

static void _3dmprefs_makerc()
{
   std::string temp = getMm3dHomeDirectory().c_str();

   mkpath( temp.c_str(), 0755 );

   temp = std::string( getPluginDirectory().c_str() );

   mkpath( temp.c_str(), 0755 );
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
   FileDataDest dst( _getFilename().c_str() );

   if ( dst.errorOccurred() )
   {
      // TODO: Use dst.getErrno() for config file save failure.
      log_warning( "could not save config file\n" );
      return;
   }

   g_prefs.write( dst );
}

bool prefs_load()
{
   FileDataSource src( _getFilename().c_str() );

   if ( src.errorOccurred() )
   {
      // TODO: Use src.getErrno() for config file load failure.
      log_warning( "no config file\n" );
      return false;
   }

    return prefparse_do_parse( src, g_prefs );
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

