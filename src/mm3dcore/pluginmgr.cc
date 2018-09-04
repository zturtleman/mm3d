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


#include "pluginmgr.h"
#include "sysconf.h"
#include "log.h"
#include "mm3dport.h"
#include "misc.h"
#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#ifdef HAVE_DLOPEN
#include <dlfcn.h>
#define PLUGINS_ENABLED
#endif // HAVE_DLOPEN

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// FIXME: This requires Windows Vista.
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x80
#endif

#define PLUGINS_ENABLED
#endif // WIN32

#include <string>

static LibHandle _openLibrary( const char * filename )
{
   LibHandle h = NULL;
#ifdef WIN32
   h = LoadLibrary( filename );
#else
#ifdef HAVE_DLOPEN
   h = dlopen( filename, RTLD_NOW | RTLD_GLOBAL );
#endif // HAVE_DLOPEN
#endif // WIN32
   return h;
}

static void * _getFunction( LibHandle h, const char * funcName )
{
   void * ptr;
#ifdef WIN32
   ptr = (void *) GetProcAddress( h, funcName );
#else
#ifdef HAVE_DLOPEN
   ptr = dlsym( h, funcName );
#endif // HAVE_DLOPEN
#endif // WIN32
   return ptr;
}

static void _closeLibrary( LibHandle h )
{
#ifdef WIN32
   FreeLibrary( h );
#else
#ifdef HAVE_DLOPEN
   dlclose( h );
#endif // HAVE_DLOPEN
#endif // WIN32
}

static const char * _getError()
{
#ifdef WIN32
   return "windows error";
#else
#ifdef HAVE_DLOPEN
   return dlerror();
#endif // HAVE_DLOPEN
#endif // WIN32
   return "";
}

using std::string;

PluginManager * PluginManager::s_instance = NULL;

PluginManager::PluginManager()
   : m_nextId( 0 ),
     m_initialize( true )
{
}

PluginManager::~PluginManager()
{
   unloadPlugins();
}

PluginManager * PluginManager::getInstance()
{
   if ( s_instance == NULL )
   {
      s_instance = new PluginManager();
   }

   return s_instance;
}

void PluginManager::release()
{
   if ( s_instance != NULL )
   {
      delete s_instance;
      s_instance = NULL;
   }
}

bool PluginManager::loadPlugin( const char * pluginFile )
{
   bool enabled = true;

#ifdef PLUGINS_ENABLED
   LibHandle handle;
   bool (*init_function)();
   bool (*uninit_function)();
   const char * (*version_function)();
   const char * (*mm3d_version_function)();
   const char * (*desc_function)();
   PluginStatusE status = PluginError;
   string name;

   log_debug( "loading plugin file: %s\n", pluginFile );
   name = fileToName( pluginFile );

   if( (handle = _openLibrary( pluginFile ) ) == NULL )
   {
      log_warning( "openLibrary: %s\n", _getError() );
      return false;
   }

   if( (init_function = ( bool (*)())  _getFunction( handle, "plugin_init")) == NULL)
   {
      log_warning( "%s: no plugin_init symbol\n", pluginFile );
      enabled = false;
      status = PluginNotPlugin;
   }

   if( (uninit_function = ( bool (*)()) _getFunction( handle, "plugin_uninit")) == NULL)
   {
      log_warning( "%s: no plugin_uninit symbol\n", pluginFile );
      status = PluginNotPlugin;
      enabled = false;
   }

   if( (version_function = ( const char * (*)()) _getFunction( handle, "plugin_version")) == NULL)
   {
      // This is just a warning, don't fail over it
      log_warning( "%s: no plugin_version symbol\n", pluginFile );
   }

   if( (mm3d_version_function = ( const char * (*)()) _getFunction( handle, "plugin_mm3d_version")) == NULL)
   {
      // This is just a warning, don't fail over it
      log_warning( "%s: no plugin_mm3d_version symbol\n", pluginFile );
   }

   if( (desc_function = ( const char * (*)()) _getFunction( handle, "plugin_desc")) == NULL)
   {
      // This is just a warning, don't fail over it
      log_warning( "%s: no plugin_desc symbol\n", pluginFile );
   }

   // Try to initialize
   bool badVersion = true;
   if ( mm3d_version_function )
   {
      const char * mm3dver = mm3d_version_function();

      int major = 0;
      int minor = 0;
      int patch = 0;

      int count = sscanf( mm3dver, "%d.%d.%d", &major, &minor, &patch );

      if ( count == 3 )
      {
#if MM3D_DEVEL_VERSION // Devel/Stable plugin versioning logic

         // Development version
         if (     major == VERSION_MAJOR 
               && minor == VERSION_MINOR 
               && patch == VERSION_PATCH )
         {
            badVersion = false;
         }
         // We're a development version, plugin must be an exact match
         else
         {
            log_error( "plugin %s compiled for %d.%d.%d, %d.%d.%d required\n", pluginFile,
                  major, minor, patch, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );
         }

#else

         // Stable version
         if (     major == VERSION_MAJOR 
               && minor == VERSION_MINOR )
         {
            badVersion = false;
         }
         // We're a stable version, MAJOR/MINOR must match
         else
         {
            log_error( "plugin %s compiled for %d.%d.%d, %d.%d.x required\n", pluginFile,
                  major, minor, patch, VERSION_MAJOR, VERSION_MINOR );
         }

#endif // Devel/Stable plugin versioning logic

         if ( ! badVersion )
         {
            if ( m_initialize && !isDisabled( name.c_str() ) )
            { 
               if ( init_function && init_function() )
               {
                  status = PluginActive;
                  enabled = true;
               }
               else
               {
                  status = PluginError;
                  enabled = false;
                  log_error( "initialization failed for %s\n", pluginFile );
                  _closeLibrary(  handle );
                  handle = NULL;
               }
            }
            else
            {
               log_debug( "%s disabled\n", pluginFile );
               status = PluginUserDisabled;
               enabled = false;
            }
         }
      }
   }
   else
   {
      log_error( "plugin %s does not have a plugin_mm3d_version function\n", pluginFile );
   }

   if ( badVersion )
   {
      status = PluginVersionDisabled;
      enabled = false;
      log_error( "disabled %s due to bad version\n", pluginFile );
   }

   PluginDataT * data = new PluginDataT();

   data->m_id              = getNextId();
   data->m_status          = status;
   data->m_name            = name;
   data->m_enabled         = enabled;
   data->m_initFunction    = init_function;
   data->m_uninitFunction  = uninit_function;
   data->m_versionFunction = version_function;
   data->m_descFunction    = desc_function;
   data->m_fileHandle      = handle;

   m_plugins.push_back( data );
#else
   log_warning( "plugins disabled at compile time\n" );
#endif // PLUGINS_ENABLED

   return enabled;
}

bool PluginManager::loadPluginDir( const char * pluginDir )
{
   if ( pluginDir && pluginDir[0] )
   {
#ifdef WIN32
      std::string searchPath;
      searchPath += pluginDir;
      if ( searchPath[searchPath.size()-1] != DIR_SLASH )
      {
         searchPath += DIR_SLASH;
      }
      searchPath += '*';

      std::wstring wideSearch = utf8PathToWide( searchPath.c_str() );
      if ( wideSearch.empty() )
      {
         log_warning( "loadPluginDir(%s): utf8PathToWide() failed\n", pluginDir );
         return false;
      }

      WIN32_FIND_DATAW ffd;
      HANDLE hFind = FindFirstFileW( &wideSearch[0], &ffd );
      if ( hFind == INVALID_HANDLE_VALUE )
      {
         if ( GetLastError() != ERROR_FILE_NOT_FOUND )
         {
            log_warning( "loadPluginDir(%s): FindFirstFile() failed, error 0x%x\n", pluginDir, GetLastError() );
         }
         return false;
      }

      do {
         DWORD utf8Size = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, ffd.cFileName, -1, NULL, 0, NULL, NULL );

         std::string file( utf8Size, '\0' );
         if ( WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, ffd.cFileName, -1, &file[0], utf8Size, NULL, NULL ) == 0 )
         {
            log_warning( "loadPluginDir(%s): failed to convert filename (name length %d)\n", pluginDir, (int)utf8Size );
            continue;
         }

         if ( strcmp( file.c_str(), "." ) == 0 || strcmp( file.c_str(), ".." ) == 0 )
         {
            continue;
         }

         if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
         {
            loadPluginDir( file.c_str() );
         }
         else
         {
            loadPlugin( file.c_str() );
         }
      } while ( FindNextFileW( hFind, &ffd ) != 0 );

      FindClose( hFind );
      return true;
#else
      DIR * dp = opendir( pluginDir );
      if ( dp )
      {
         log_debug( "loading plugin directory: %s\n", pluginDir );

         struct dirent * d;
         while ( (d = readdir( dp ) ) != NULL )
         {
            if ( strcmp( d->d_name, "." ) == 0 || strcmp( d->d_name, "..") == 0 )
            {
               continue;
            }
            string file = string(pluginDir) + DIR_SLASH;
            file += d->d_name;

            struct stat statbuf;
            if ( lstat( file.c_str(), &statbuf ) == 0 )
            {
               if ( S_ISREG( statbuf.st_mode ) )
               {
                  loadPlugin( file.c_str() );
               }
               else if ( S_ISLNK( statbuf.st_mode ) )
               {
                  log_debug( "%s is a symlink\n", file.c_str() );
                  // Try directory, then regular file
                  if ( ! loadPluginDir( file.c_str() ) )
                  {
                     // Not a directory, let's try file
                     loadPlugin( file.c_str() );
                  }
               }
               else if ( S_ISDIR( statbuf.st_mode ) )
               {
                  loadPluginDir( file.c_str() );
               }
               else
               {
                  log_warning( "Don't know what to do with st_mode %08X\n", 
                        (unsigned) statbuf.st_mode );
               }
            }
            else
            {
               log_error( "stat: %s: %s\n", file.c_str(), strerror(errno) );
            }
         }

         closedir( dp );
         return true;
      }
      else
      {
         // This is called any time the file is not a regular file
         // If it turns out to not be a directory, that isn't really
         // an error.
         if ( errno != ENOTDIR )
         {
            log_warning( "%s: %s\n", pluginDir, strerror(errno) );
         }
      }
#endif
   }

   return false;
}

bool PluginManager::unloadPlugins()
{
   log_debug( "PluginManager unloading %d plugins\n", m_plugins.size() );
#ifdef PLUGINS_ENABLED
   PluginDataList::iterator it = m_plugins.begin();

   while ( it != m_plugins.end() )
   {
      if ( (*it)->m_fileHandle != NULL )
      {
         if ( (*it)->m_enabled )
         {
            (*it)->m_uninitFunction();
         }
         _closeLibrary( (*it)->m_fileHandle );
      }
      delete *it;
      it++;
   }

   m_plugins.clear();
#else
   log_warning( "plugins disabled at compile time\n" );
#endif // PLUGINS_ENABLED

   return true;
}

void PluginManager::loadPlugins()
{
   string plugin_dir = getPluginDirectory();

#if defined WIN32 || defined __APPLE__
   loadPluginDir( plugin_dir.c_str() );

   plugin_dir  = getSharedPluginDirectory();
   loadPluginDir( plugin_dir.c_str() );
#else
   // Check to see if we have a user-specific plugin directory
   // (By default it contains a symlink to the shared directory, see 3dmprefs.cc)
   if ( ! loadPluginDir( plugin_dir.c_str() ) )
   {
      // No, check shared directory
      plugin_dir  = getSharedPluginDirectory();
      loadPluginDir( plugin_dir.c_str() );
   }
#endif
}

int init_plugins()
{
   PluginManager * mgr = PluginManager::getInstance();
   mgr->loadPlugins();
   return 0;
}

int uninit_plugins()
{
   PluginManager * mgr = PluginManager::getInstance();
   mgr->unloadPlugins();
   return 0;
}

list<int> PluginManager::getPluginIds()
{
   list<int> pluginList;

   PluginDataList::iterator it;
   for ( it = m_plugins.begin(); it != m_plugins.end(); it++ )
   {
      pluginList.push_back( (*it)->m_id );
   }

   return pluginList;
}

const char * PluginManager::getPluginName( int id )
{
   PluginDataList::iterator it;
   for ( it = m_plugins.begin(); it != m_plugins.end(); it++ )
   {
      if ( (*it)->m_id == id )
      {
         return (*it)->m_name.c_str();
      }
   }

   return "Not found";
}

PluginManager::PluginStatusE PluginManager::getPluginStatus( int id )
{
   PluginDataList::iterator it;
   for ( it = m_plugins.begin(); it != m_plugins.end(); it++ )
   {
      if ( (*it)->m_id == id )
      {
         return (*it)->m_status;
      }
   }

   return PluginError;
}

const char * PluginManager::getPluginVersion( int id )
{
   PluginDataList::iterator it;
   for ( it = m_plugins.begin(); it != m_plugins.end(); it++ )
   {
      if ( (*it)->m_id == id )
      {
         if ( (*it)->m_versionFunction )
         {
            const char * str = (*it)->m_versionFunction();

            return str ? str : "Unavailable";
         }
         else
         {
            return "Unavailable";
         }
      }
   }

   return "Not found";
}

const char * PluginManager::getPluginDescription( int id )
{
   PluginDataList::iterator it;
   for ( it = m_plugins.begin(); it != m_plugins.end(); it++ )
   {
      if ( (*it)->m_id == id )
      {
         if ( (*it)->m_descFunction )
         {
            const char * str = (*it)->m_descFunction();

            return str ? str : "Unavailable";
         }
         else
         {
            return "Unavailable";
         }
      }
   }

   return "Not found";
}

string PluginManager::fileToName( const char * file )
{
   char * temp = strdup( file );
   char * s = strrchr( temp, DIR_SLASH );
   if ( s )
   {
      s += 1; // skip last slash
   }
   else
   {
      s = temp; // no slash, just start with filename
   }

   int len = (int) strlen( s );
   for ( int t = len - 1; t >= 0; t-- )
   {
      if ( s[t] == '.' )
      {
         s[t] = '\0';
         break;
      }
   }

   string str = s;
   free( temp );

   return str;
}

void PluginManager::disable( const char * pluginName )
{
   list<string>::iterator it;

   for ( it = m_disabled.begin(); it != m_disabled.end(); it++ )
   {
      if ( strcasecmp ( (*it).c_str(), pluginName  ) == 0 )
      {
         return;
      }
   }

   m_disabled.push_back( pluginName );
}

bool PluginManager::isDisabled( const char * pluginName )
{
   list<string>::iterator it;

   for ( it = m_disabled.begin(); it != m_disabled.end(); it++ )
   {
      if ( strcasecmp ( (*it).c_str(), pluginName  ) == 0 )
      {
         return true;
      }
   }

   return false;
}
