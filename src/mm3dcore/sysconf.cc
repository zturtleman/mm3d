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

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <cstring>
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <string.h>
#include <stdlib.h>

#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
#include <initguid.h>
#include <knownfolders.h>
#endif

#elif defined(__APPLE__)

#include <errno.h>
#include <sys/syslimits.h>
#include <mach-o/dyld.h> // _NSGetExecutablePath

#endif

#include "sysconf.h"
#include "mm3dconfig.h"
#include "mm3dport.h"
#include "misc.h"
#include "log.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static std::string s_mm3dHomeDir;
static std::string s_docDir;
static std::string s_translationsDir;
static std::string s_pluginDir;
static std::string s_sharedPluginDir;

static std::string s_configFile;

#ifdef WIN32

// Gets the path to the current process's exe file
static std::string getExecutablePath()
{
   std::string rval = "";
   WCHAR *wexecpath;
   DWORD length, maxLength;

   maxLength = MAX_PATH;
   wexecpath = (WCHAR *)malloc( maxLength * sizeof( WCHAR ) );

   while ( 1 )
   {
      length = GetModuleFileNameW( NULL, wexecpath, maxLength );

      if ( length >= maxLength )
      {
         maxLength *= 2;
         wexecpath = (WCHAR *)realloc( wexecpath, maxLength * sizeof( WCHAR ) );
      }
      else
      {
         break;
      }
   }

   if ( length == 0 )
   {
      log_debug( "getExecutablePath: GetModuleFileNameW() failed: 0x%x\n", GetLastError() );
   }
   else
   {
      std::string execpath = widePathToUtf8( wexecpath );
      std::string::size_type pathsep = execpath.rfind( '\\' );

      if ( pathsep != std::string::npos )
      {
         execpath.resize( pathsep );
         rval = execpath;
      }
   }

   free( wexecpath );

   return rval;
}

static std::string getAppDataPath()
{
   std::string utf8Path;
#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
   HMODULE libShell32 = LoadLibraryW( L"Shell32.dll" );

   if ( libShell32 ) {
      typedef HRESULT ( APIENTRY *pfnSHGetKnownFolderPath )( REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR * );
      // Use union to avoid function cast warning.
      union {
         FARPROC generic;
         pfnSHGetKnownFolderPath proc;
      } procTypeCast;
      pfnSHGetKnownFolderPath qSHGetKnownFolderPath;
      WCHAR *knownWidePath = NULL;

      procTypeCast.generic = GetProcAddress( libShell32, "SHGetKnownFolderPath" );
      qSHGetKnownFolderPath = procTypeCast.proc;

      // Windows Vista or later
      // (KF_FLAG_CREATE requires _WIN32_WINNT 0x0600)
      if ( qSHGetKnownFolderPath && qSHGetKnownFolderPath( FOLDERID_RoamingAppData, 0x00008000 /*KF_FLAG_CREATE*/, NULL, &knownWidePath ) == S_OK ) {
         utf8Path = widePathToUtf8( knownWidePath );
         CoTaskMemFree( knownWidePath );
      }

      FreeLibrary( libShell32 );
   }
#else
#warning "Missing FOLDERID_RoamingAppData required for long path support for AppData"
#endif

   if ( utf8Path.empty() ) {
      // Windows 2000 Professional or later
      wchar_t appdata[MAX_PATH];
      if ( SHGetFolderPathW( NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, appdata ) == S_OK ) {
         utf8Path = widePathToUtf8( appdata );
      }
   }

   return utf8Path;
}

#elif defined(__APPLE__)

static std::string getExecutablePath()
{
   char actualpath[PATH_MAX];
   char tmppath[PATH_MAX];
   char *ptr;
   uint32_t size;

   size = sizeof( tmppath );

   if ( _NSGetExecutablePath( tmppath, &size ) == 0 ) {
      if ( realpath( tmppath, actualpath ) == NULL ) {
         log_debug( "getExecutablePath: realpath(%s) failed: %s\n", tmppath, strerror( errno ) );
         actualpath[0] = '.';
         actualpath[1] = '\0';
      }
   } else {
      log_debug( "getExecutablePath: _NSGetExecutablePath() failed: %s\n", strerror( errno ) );
      actualpath[0] = '.';
      actualpath[1] = '\0';
   }

   // Remove executable name and trailing directory separator
   ptr = strrchr( actualpath, '/' );
   if ( ptr ) {
      ptr[0] = '\0';
   }

   return actualpath;
}

static std::string getAppBundlePath()
{
   std::string path = getExecutablePath();
   std::string endsWith = ".app/Contents/MacOS";

   // Check if executable is not in an app bundle
   if ( path.length() < endsWith.length()
      || path.compare( path.length() - endsWith.length(), endsWith.length(), endsWith ) != 0 ) {
      return "";
   }

   return path.substr( 0, path.length() - endsWith.length() + strlen( ".app" ) );
}

#endif // __APPLE__

void init_sysconf()
{
   char * majorMinor = strdup( VERSION );  
   int off = strlen( majorMinor ) - 1;

   while ( off > 0 && majorMinor[ off ] != '.' )
   {
      off--;
   }
   majorMinor[ off ] = '\0';


   std::string s_dataDir;

#ifdef WIN32
   std::string path = getExecutablePath();

   if ( path.length() == 0 )
   {
      // Fall back to current directory
      char *cwd = PORT_get_current_dir_name();

      if ( cwd ) {
         path = cwd;

         free( cwd );
      } else {
         path = ".";
      }
   }

   std::string appdata = getAppDataPath();
   if ( !appdata.empty() ) {
      s_mm3dHomeDir  = appdata;
      s_mm3dHomeDir += HOME_MM3D;
   } else {
      s_mm3dHomeDir  = path;
      s_mm3dHomeDir += "\\userhome";
   }

   s_dataDir     = s_mm3dHomeDir;
   s_pluginDir   = s_dataDir + HOME_PLUGINS;
   s_pluginDir  += "\\";
   s_pluginDir  += majorMinor;

   s_docDir      = path + DOC_ROOT;
   s_translationsDir  = path + TRANSLATIONS_ROOT;
   s_sharedPluginDir  = path + SHARED_PLUGINS;
   s_sharedPluginDir += "\\";
   s_sharedPluginDir += majorMinor;
#elif defined __APPLE__
   s_mm3dHomeDir = getenv( "HOME" );
   s_mm3dHomeDir += HOME_MM3D;
   s_dataDir   = s_mm3dHomeDir;
   s_pluginDir = s_dataDir + HOME_PLUGINS;
   s_pluginDir += "/";
   s_pluginDir += majorMinor;

   std::string appPath = getAppBundlePath();

   if ( appPath.length() > 0 ) {
      s_docDir           = appPath + "/Contents/SharedSupport/mm3d/doc/html";
      s_translationsDir  = appPath + "/Contents/SharedSupport/mm3d/translations";
      s_sharedPluginDir  = appPath + "/Contents/PlugIns/mm3d";
      s_sharedPluginDir += "/";
      s_sharedPluginDir += majorMinor;
   } else {
      s_docDir           = DOC_ROOT;
      s_translationsDir  = TRANSLATIONS_ROOT;
      s_sharedPluginDir  = SHARED_PLUGINS;
      s_sharedPluginDir += "/";
      s_sharedPluginDir += majorMinor;
   }
#else
   // I don't want to deal with moving the existing config
   // so non-flatpak still uses ~/.mm3d. --zturtleman
   const char *var = getenv( "FLATPAK_ID" );
   if ( var && var[0] ) {
      var = getenv( "XDG_CONFIG_HOME" );
      s_mm3dHomeDir = var ? var : "";
      if ( s_mm3dHomeDir.empty() ) {
         s_mm3dHomeDir = getenv( "HOME" );
         s_mm3dHomeDir += "/.config";
      }
      s_mm3dHomeDir += "/";
      s_mm3dHomeDir += HOME_MM3D;

      var = getenv( "XDG_DATA_HOME" );
      s_dataDir = var ? var : "";
      if ( s_dataDir.empty() ) {
         s_dataDir = getenv( "HOME" );
         s_dataDir += "/.local/share";
      }
      s_dataDir += "/";
      s_dataDir += HOME_MM3D;
   } else {
      s_mm3dHomeDir = getenv( "HOME" );
      s_mm3dHomeDir += "/.";
      s_mm3dHomeDir += HOME_MM3D;
      s_dataDir = s_mm3dHomeDir;
   }
   s_docDir    = DOC_ROOT;
   s_translationsDir = TRANSLATIONS_ROOT;
   s_pluginDir = s_dataDir + HOME_PLUGINS;
   s_pluginDir += "/";
   s_pluginDir += majorMinor;
   s_sharedPluginDir  = SHARED_PLUGINS;
   s_sharedPluginDir += "/";
   s_sharedPluginDir += majorMinor;
#endif

   s_configFile = s_mm3dHomeDir + HOME_RC;

   /*
   log_debug( "mm3d home is %s\n", s_mm3dHomeDir.c_str() );
   log_debug( "doc dir is %s\n", s_docDir.c_str() );
   log_debug( "plugin dir is %s\n", s_pluginDir.c_str() );
   log_debug( "shared plugin dir is %s\n", s_sharedPluginDir.c_str() );
   log_debug( "config file is %s\n", s_configFile.c_str() );
   */

   free( majorMinor );
}

const std::string & getConfigFile()
{
   return s_configFile;
}

const std::string & getMm3dHomeDirectory()
{
   return s_mm3dHomeDir;
}

const std::string & getDocDirectory()
{
   return s_docDir;
}

const std::string & getTranslationsDirectory()
{
   return s_translationsDir;
}

const std::string & getPluginDirectory()
{
   return s_pluginDir;
}

const std::string & getSharedPluginDirectory()
{
   return s_sharedPluginDir;
}

