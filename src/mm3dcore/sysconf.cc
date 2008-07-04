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

#ifdef WIN32

#include <windows.h>
#include <stdio.h>
#include <cstring>
#if 0 // Disabled for Win98
#include <psapi.h>
#endif // 0
#include <string>
#include "mm3dreg.h"
#include "version.h"

using namespace Mm3dReg;

#endif // WIN32

#include "sysconf.h"
#include "mm3dconfig.h"
#include "mm3dport.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static std::string s_mm3dHomeDir;
static std::string s_docDir;
static std::string s_i18nDir;
static std::string s_pluginDir;
static std::string s_sharedPluginDir;

static std::string s_configFile;

#ifdef WIN32

// Gets the path to the current process's exe file
static std::string getExecutablePath()
{
    std::string rval = "";
#if 0 // Disabled for Win98

    // Get a list of all the modules in this process.

    HANDLE hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_READ,
                                    FALSE, GetCurrentProcessId() );
    if ( hProcess )
    {
        DWORD cbNeeded = 32 * sizeof( HMODULE );
        DWORD cb = cbNeeded;
        HMODULE * hMods = NULL;
        int success = 0;

        // Allocate memory, make the call
        // If we didn't allocate enough memory, cbNeeded will
        // tell us how much we need, try again
        do {
            if ( hMods )
            {
                free( hMods );
            }

            cb = cbNeeded;
            hMods = (HMODULE *) malloc( cb );
            success = EnumProcessModules(hProcess, hMods, cb, &cbNeeded);

        } while ( cbNeeded > cb );

        if ( success )
        {
            // Run through exe and dlls to find the exe
            for ( unsigned i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
            {
                char name[MAX_PATH];

                // This gets the full path to exe or dll
                if ( GetModuleFileNameEx( hProcess, hMods[i], name,
                            sizeof(name)))
                {
                    // If exe, strip exe name to get path
                    if ( strstr( name, ".exe" ) )
                    {
                        char * ptr = strrchr( name, '\\' );

                        if ( ptr )
                        {
                            ptr[0] = '\0';
                            rval = name;
                            break;
                        }
                    }
                }
            }
        }

        free ( hMods );
        CloseHandle( hProcess );
    }

#endif // 0
    return rval;
}

#endif // WIN32

void init_sysconf()
{
   char * majorMinor = strdup( VERSION );  
   int off = strlen( majorMinor ) - 1;

   while ( off > 0 && majorMinor[ off ] != '.' )
   {
      off--;
   }
   majorMinor[ off ] = '\0';


#ifdef WIN32
   char cwd[PATH_MAX];
   getcwd( cwd, sizeof(cwd) );

   // Try registry first
   std::string path = Mm3dRegKey::getRegistryString( HKEY_CURRENT_USER, "Software\\Misfit Code\\Misfit Model 3D", "INSTDIR" );

   if ( path.length() == 0 )
   {
      // Fall back to executable path, if we can get it
      path = getExecutablePath();

      if ( path.length() == 0 )
      {
         // Last resort, current directory
         path = cwd;
      }
   }

   s_mm3dHomeDir = path + HOME_MM3D;
   s_docDir      = path + DOC_ROOT;
   s_i18nDir     = path + I18N_ROOT;
   s_pluginDir   = path + HOME_PLUGINS;

   unsigned len = s_docDir.size();
   for ( unsigned t = 0; t < len; t++ )
   {
      if ( s_docDir[t] == '/' )
      {
         s_docDir[t] = '\\';
      }
   }
   len = s_i18nDir.size();
   for ( unsigned t = 0; t < len; t++ )
   {
      if ( s_i18nDir[t] == '/' )
      {
         s_i18nDir[t] = '\\';
      }
   }
#else
   s_mm3dHomeDir = PORT_getenv( "HOME" );
   s_mm3dHomeDir += HOME_MM3D;
   s_docDir    = DOC_ROOT;
   s_i18nDir   = I18N_ROOT;
   s_pluginDir = s_mm3dHomeDir + HOME_PLUGINS;
   s_pluginDir += "/";
   s_pluginDir += majorMinor;
#endif // WIN32

   s_sharedPluginDir  = SHARED_PLUGINS;
   s_sharedPluginDir += "/";
   s_sharedPluginDir += majorMinor;

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

const std::string & getI18nDirectory()
{
   return s_i18nDir;
}

const std::string & getPluginDirectory()
{
   return s_pluginDir;
}

const std::string & getSharedPluginDirectory()
{
   return s_sharedPluginDir;
}

