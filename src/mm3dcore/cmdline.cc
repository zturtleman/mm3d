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
#include "cmdline.h"
#include "cmdlinemgr.h"
#include "version.h"
#include "3dmprefs.h"
#include "modelutil.h"
#include "luascript.h"
#include "luaif.h"
#include "filtermgr.h"
#include "misc.h"
#include "msg.h"
#include "log.h"
#include "translate.h"
#include "mlocale.h"

#include "texturetest.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <string>
#include <list>
#include <vector>

using std::string;
using std::list;
using std::vector;

typedef std::list< std::string > StringList;

bool cmdline_runcommand = false;
bool cmdline_runui      = true;

static bool        _doConvert = false;
static std::string _convertFormat = "";

static bool _doBatch = false;

static bool _doScripts = false;
static bool _doTextureTest = false;

static StringList _scripts;
static StringList _argList;

typedef std::vector< Model * > ModelList;
static ModelList _models;

static void _print_version( const char * progname )
{
   printf( "\nMaverick Model 3D, version %s\n\n", VERSION_STRING );
}

static void _print_help( const char * progname )
{
   _print_version( progname );

   printf( "Usage:\n  %s [options] [model_file] ...\n\n", progname );

   printf("Options:\n" );
   printf("  -h  --help             Print command line help and exit\n" );
   printf("  -v  --version          Print version information and exit\n" );
#ifdef HAVE_LUALIB
   printf("      --script [file]    Run script [file] on models\n" );
#endif // HAVE_LUALIB
   printf("      --convert [format] Save models to format [format]\n" );
   printf("                         \n" );
   printf("      --language [code]  Use language [code] instead of system default\n" );
   printf("                         \n" );
   printf("      --no-plugins       Disable all plugins\n" );
   printf("      --no-plugin [foo]  Disable plugin [foo]\n" );
   printf("                         \n" );
   printf("      --sysinfo          Display system information (for bug reports)\n" );
   printf("      --debug            Display debug messages in console\n" );
   printf("      --warnings         Display warning messages in console\n" );
   printf("      --errors           Display error messages in console\n" );
   printf("      --no-debug         Do not display debug messages in console\n" );
   printf("      --no-warnings      Do not display warning messages in console\n" );
   printf("      --no-errors        Do not display error messages in console\n" );
   printf("\n" );

   exit(0);
}

static void _print_sysinfo()
{
   printf( "\nMaverick Model 3D, version %s\n\n", VERSION_STRING );

#ifdef WIN32
   // TODO: Get Windows version
#else
   FILE * fp = NULL;
   char input[80];

   printf( "uname output:\n" );
   fp = popen( "uname -a", "r" );
   if ( fp )
   {
      while( fgets( input, sizeof(input), fp ) )
      {
         printf( "%s", input );
      }
      fclose( fp );
   }
   else
   {
      printf( "error: uname: %s\n", strerror(errno) );
   }

   printf( "\n/etc/issue:\n" );
   fp = popen( "cat /etc/issue", "r" );
   if ( fp )
   {
      while( fgets( input, sizeof(input), fp ) )
      {
         printf( "%s", input );
      }
      fclose( fp );
   }
   else
   {
      printf( "error: cat /etc/issue: %s\n", strerror(errno) );
   }

   printf("\n" );
#endif

   exit(0);
}

enum Mm3dOptionsE 
{
   OptHelp,
   OptVersion,
   OptBatch,
   OptNoPlugins,
   OptNoPlugin,
   OptConvert,
   OptLanguage,
   OptScript,
   OptSysinfo,
   OptDebug,
   OptWarnings,
   OptErrors,
   OptNoDebug,
   OptNoWarnings,
   OptNoErrors,
   OptTestTextureCompare,
   OptMAX
};

int init_cmdline( int & argc, char * argv[] )
{
   CommandLineManager clm;

   clm.addOption( OptHelp, 'h', "help" );
   clm.addOption( OptVersion, 'v', "version" );
   clm.addOption( OptBatch, 'b', "batch" );

   clm.addOption( OptNoPlugins, 0, "no-plugins" );
   clm.addOption( OptNoPlugin, 0, "no-plugin", NULL, true );
   clm.addOption( OptConvert, 0, "convert", NULL, true );
   clm.addOption( OptLanguage, 0, "language", NULL, true );
   clm.addOption( OptScript, 0, "script", NULL, true );

   clm.addOption( OptSysinfo, 0, "sysinfo" );
   clm.addOption( OptDebug, 0, "debug" );
   clm.addOption( OptWarnings, 0, "warnings" );
   clm.addOption( OptErrors, 0, "errors" );
   clm.addOption( OptNoDebug, 0, "no-debug" );
   clm.addOption( OptNoWarnings, 0, "no-warnings" );
   clm.addOption( OptNoErrors, 0, "no-errors" );

   clm.addOption( OptTestTextureCompare, 0, "testtexcompare" );

   if ( !clm.parse( argc, (const char **) argv ) )
   {
      const char * opt = argv[ clm.errorArgument() ];

      switch ( clm.error() )
      {
         case CommandLineManager::MissingArgument:
            fprintf( stderr, "Option '%s' requires an argument. "
                             "See --help for details.\n", opt );
            break;
         case CommandLineManager::UnknownOption:
            fprintf( stderr, "Unknown option '%s'. "
                             "See --help for details.\n", opt );
            break;
         case CommandLineManager::NoError:
            fprintf( stderr, "BUG: CommandLineManager::parse returned false but "
                             "error code was not set.\n" );
            break;
         default:
            fprintf( stderr, "BUG: CommandLineManager::error returned an "
                             "unknown error code.\n" );
            break;
      }
      exit( -1 );
   }

   if ( clm.isSpecified( OptHelp ) )
      _print_help( argv[0] );

   if ( clm.isSpecified( OptVersion ) )
   {
      _print_version( argv[0] );
      exit( 0 );
   }

   if ( clm.isSpecified( OptBatch ) )
   {
      _doBatch = true;
      cmdline_runcommand = true;
   }

   if ( clm.isSpecified( OptNoPlugins ) )
      PluginManager::getInstance()->setInitializeAll( false );
   if ( clm.isSpecified( OptNoPlugin ) )
      PluginManager::getInstance()->disable( clm.stringValue( OptNoPlugin ) );
   if ( clm.isSpecified( OptConvert ) )
   {
      _convertFormat = clm.stringValue( OptConvert );

      _doConvert = true;

      cmdline_runcommand = true;
      cmdline_runui = false;
   }
   if ( clm.isSpecified( OptLanguage ) )
      mlocale_set( clm.stringValue( OptLanguage ) );

   if ( clm.isSpecified( OptScript ) )
   {
      _scripts.push_back( clm.stringValue( OptScript ) );

      _doScripts = true;

      cmdline_runcommand = true;
      cmdline_runui = true;
   }

   if ( clm.isSpecified( OptSysinfo ) )
      _print_sysinfo();

   if ( clm.isSpecified( OptDebug ) )
      log_enable_debug( true );
   if ( clm.isSpecified( OptWarnings ) )
      log_enable_warning( true );
   if ( clm.isSpecified( OptErrors ) )
      log_enable_error( true );
   if ( clm.isSpecified( OptNoDebug ) )
      log_enable_debug( false );
   if ( clm.isSpecified( OptNoWarnings ) )
      log_enable_warning( false );
   if ( clm.isSpecified( OptNoErrors ) )
      log_enable_error( false );

   if ( clm.isSpecified( OptTestTextureCompare ) )
   {
      _doTextureTest = true;

      cmdline_runcommand = true;
      cmdline_runui = false;
   }

   int opts_done = clm.firstArgument();
   int offset = 1;

   for ( int n = opts_done; n < argc; n++ )
   {
      _argList.push_back( argv[n] );
      argv[ offset ] = argv[n];
      ++offset;
   }

   argc = offset;

   return 0;
}

void shutdown_cmdline()
{
   cmdline_deleteOpenModels();
}

int cmdline_command()
{
   unsigned errors = 0;

   if ( _doTextureTest )
   {
      std::string master = _argList.front();

      StringList::iterator it = _argList.begin();
      it++;
      for ( ; it != _argList.end(); it++ )
      {
         texture_test_compare( master.c_str(), it->c_str(), 10 );
      }
      return 0;
   }

   FilterManager * mgr = FilterManager::getInstance();

   StringList::iterator it = _argList.begin();
   for ( ; it != _argList.end(); it++ )
   {
      Model::ModelErrorE err = Model::ERROR_NONE;
      Model * m = new Model;
      if ( (err = mgr->readFile( m, it->c_str() )) == Model::ERROR_NONE )
      {
         m->loadTextures( 0 );
         _models.push_back( m );
      }
      else
      {
         std::string reason = *it;
         reason += ": ";
         reason += transll( Model::errorToString( err, m ) );
         msg_error( reason.c_str() );
         delete m;
      }
   }
   
   if ( _argList.empty() && _doScripts )
   {
      Model * m = new Model();
      _models.push_back( m );
   }

   for ( unsigned n = 0; n < _models.size(); n++ )
   {
      Model * m = _models[n];
      if ( _doScripts )
      {
#ifdef HAVE_LUALIB
         LuaScript lua;
         LuaContext lc( m );
         luaif_registerfunctions( &lua, &lc );
         StringList::iterator it;
         for ( it = _scripts.begin(); it != _scripts.end(); it++ )
         {
            if ( lua.runFile( (*it).c_str() ) != 0 )
            {
               errors++;
            }
         }
#else
         fprintf( stderr, "scripts disabled at compile time\n" );
#endif // HAVE_LUALIB

      }

      if ( _doConvert )
      {
         const char * infile = m->getFilename();
         std::string outfile = replaceExtension( infile, _convertFormat.c_str() );
         Model::ModelErrorE err = Model::ERROR_NONE;
         if ( (err = mgr->writeFile( m, outfile.c_str(), true, FilterManager::WO_ModelNoPrompt )) != Model::ERROR_NONE )
         {
            errors++;
            std::string reason = outfile;
            reason += ": ";
            reason += transll( Model::errorToString( err, m ) );
            msg_error( reason.c_str() );
         }
      }

      if ( _doBatch )
      {
         cmdline_runui = false;
         std::string filename = m->getFilename();
         if ( filename.length() == 0 )
         {
            filename = "unnamed.mm3d";
         }

         Model::ModelErrorE err = Model::ERROR_NONE;
         if ( (err = mgr->writeFile( m, filename.c_str(), true, FilterManager::WO_ModelNoPrompt )) != Model::ERROR_NONE )
         {
            errors++;
            std::string reason = filename;
            reason += ": ";
            reason += transll( Model::errorToString( err, m ) );
            msg_error( reason.c_str() );
         }
      }
   }

   return errors;
}

extern int cmdline_getOpenModelCount()
{
   return _models.size();
}

extern Model * cmdline_getOpenModel( int n )
{
   return _models[n];
}

extern void cmdline_clearOpenModelList()
{
   _models.clear();
}

extern void cmdline_deleteOpenModels()
{
   unsigned t = 0;
   unsigned count = _models.size();
   for ( t = 0; t < count; t++ )
   {
      delete _models[t];
   }
   _models.clear();
}

