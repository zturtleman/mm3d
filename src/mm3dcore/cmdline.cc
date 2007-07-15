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
#include "version.h"
#include "3dmprefs.h"
#include "modelutil.h"
#include "luascript.h"
#include "luaif.h"
#include "filtermgr.h"
#include "misc.h"
#include "msg.h"
#include "log.h"
#include "modeltest.h"
#include "translate.h"

#include "texturetest.h"

#include "modeltest.h"  // FIXME call this model filter test
#include "model_test.h"

#include <string.h>
#include <errno.h>

#include <string>
#include <list>
#include <vector>

using std::string;
using std::list;
using std::vector;

typedef std::list< std::string > StringList;

bool cmdline_runcommand = false;
bool cmdline_runui      = true;

static bool   _doConvert = false;
static char * _convertFormat = NULL;

static bool _doBatch = false;
static bool _doSave = false;

static bool       _doScripts = true;
static StringList _scripts;

static bool   _doFilterTest = false;
static char * _filterDir = NULL;

static bool   _doTextureTest = false;
static StringList _textureFiles;

static bool   _doModelTest = false;
static StringList _modelTests;

typedef std::vector< Model * > ModelList;
static ModelList _models;

static void _print_version( const char * progname )
{
   printf( "\nMisfit Model 3D, version %s\n\n", VERSION_STRING );
}

static void _print_help( const char * progname )
{
   _print_version( progname );

   printf( "Usage:\n  %s [options] [model_file] ...\n\n", progname );

   printf("Options:\n" );
   printf("  -h  --help             Print command line help and exit\n" );
   printf("  -v  --version          Print version information and exit\n" );
   printf("  -b  --batch            Batch mode (exit after processing command line)\n" );
   printf("  -s  --save             Save command line changes to model files\n" );
#ifdef HAVE_LUALIB
   printf("      --script [file]    Run script [file] on models\n" );
#endif // HAVE_LUALIB
   printf("      --convert [format] Save models to format [format]\n" );
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
   FILE * fp = NULL;
   char input[80];

   printf( "\nMisfit Model 3D, version %s\n\n", VERSION_STRING );

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

   exit(0);
}

int init_cmdline( int & argc, char * argv[] )
{
   int opts_done = 0;
   bool readingOptions = true;
   bool haveBadArgument = false;

   for ( int t = 1; t < argc; t++ )
   {
      if ( readingOptions && argv[t][0] == '-' )
      {
         if ( strcmp( argv[t], "--no-plugins" ) == 0 )
         {
            PluginManager::getInstance()->setInitializeAll( false );
         }
         else if ( strncmp( argv[t], "--no-plugin", 11 ) == 0 )
         {
            if ( argv[t][11] == '=' )
            {
               PluginManager::getInstance()->disable( &argv[t][12] );
            }
            else
            {
               t++;
               if ( t < argc )
               {
                  PluginManager::getInstance()->disable( argv[t] );
               }
               else
               {
                  fprintf( stderr, "Option %s requires plugin_name argument\n", argv[t] );
                  exit( -1 );
               }
            }
         }
         else if ( strncmp( argv[t], "--convert", 9 ) == 0 )
         {
            if ( argv[t][9] == '=' )
            {
               _convertFormat = &argv[t][10];
            }
            else
            {
               t++;
               if ( t < argc )
               {
                  _convertFormat = argv[t];
               }
               else
               {
                  fprintf( stderr, "Option %s requires argument of file format\n", argv[t-1] );
                  exit( -1 );
               }
            }

            _doConvert = true;

            cmdline_runcommand = true;
            cmdline_runui      = false;
         }
         else if ( strncmp( argv[t], "--script", 8 ) == 0 )
         {
            if ( argv[t][8] == '=' )
            {
               _scripts.push_back( &argv[t][9] );
            }
            else
            {
               t++;
               if ( t < argc )
               {
                  _scripts.push_back( argv[t] );
               }
               else
               {
                  fprintf( stderr, "Option %s requires argument of script name\n", argv[t-1] );
                  exit( -1 );
               }
            }

            _doScripts = true;

            cmdline_runcommand = true;
            cmdline_runui      = true;
         }
         else if ( strncmp( argv[t], "--filtertest", 11 ) == 0 )
         {
            if ( argv[t][11] == '=' )
            {
               _filterDir = strdup( &argv[t][12] );
            }
            else
            {
               t++;
               if ( t < argc )
               {
                  _filterDir = strdup( argv[t] );
               }
               else
               {
                  fprintf( stderr, "Option %s requires argument of directory with test models\n", argv[t-1] );
                  exit( -1 );
               }
            }

            _doFilterTest = true;

            cmdline_runcommand = true;
            cmdline_runui      = false;
         }
         else if ( strncmp( argv[t], "--testtexcompare", 16 ) == 0 )
         {
            if ( argc < t + 2 )
            {
               fprintf( stderr, "not enough arguments to texture test\n" );
               exit( -1 );
            }

            for ( int n = t+1; n < argc; n++ )
            {
               _textureFiles.push_back( argv[n] );
            }

            _doTextureTest = true;

            cmdline_runcommand = true;
            cmdline_runui      = false;

            t = argc;
         }
         else if ( strncmp( argv[t], "--testmodel", 11 ) == 0 )
         {
            if ( argc < t + 2 )
            {
               fprintf( stderr, "not enough arguments to model test\n" );
               exit( -1 );
            }

            for ( int n = t+1; n < argc; n++ )
            {
               _modelTests.push_back( argv[n] );
            }

            _doModelTest = true;

            cmdline_runcommand = true;
            cmdline_runui      = false;

            t = argc;
         }
         else if ( strncmp( argv[t], "--config", 8 ) == 0 )
         {
            char * key     = NULL;
            char * value   = NULL;
            if ( argv[t][8] == '=' )
            {
               key = strdup(&argv[t][9]);
            }
            else
            {
               t++;
               if ( t < argc )
               {
                  key = strdup( argv[t] );
               }
               else
               {
                  fprintf( stderr, "Option %s requires argument of format 'key=value'\n", argv[t] );
                  exit( -1 );
               }
            }

            if ( key[0] != '=' && (value = strchr( key, '=' )) != NULL )
            {
               value++; // skip '='
            }
            else
            {
               fprintf( stderr, "Option --config requires argument of format 'key=value'\n" );
               free( key );
               exit( -1 );
            }

            g_prefs(key) = value;

            free( key );
         }
         else if ( strcmp( argv[t], "--" ) == 0 )
         {
            readingOptions = false;
            opts_done = t+1;
            break;
         }
         else if ( strcmp( argv[t], "--help" ) == 0 || strcmp( argv[t], "-h" ) == 0 )
         {
            _print_help( argv[0] );
         }
         else if ( strcmp( argv[t], "--version" ) == 0 || strcmp( argv[t], "-v" ) == 0 )
         {
            _print_version( argv[0] );
            exit( 0 );
         }
         else if ( strcmp( argv[t], "--sysinfo" ) == 0 )
         {
            _print_sysinfo();
         }
         else if ( strcmp( argv[t], "--batch" ) == 0 || strcmp( argv[t], "-b" ) == 0 )
         {
            _doBatch = true;
            cmdline_runcommand = true;
         }
         else if ( strcmp( argv[t], "--save" ) == 0 || strcmp( argv[t], "-s" ) == 0 )
         {
            _doSave = true;
            cmdline_runcommand = true;
         }
         else if ( strcmp( argv[t], "--debug" ) == 0 )
         {
            log_enable_debug( true );
         }
         else if ( strcmp( argv[t], "--warnings" ) == 0 )
         {
            log_enable_warning( true );
         }
         else if ( strcmp( argv[t], "--errors" ) == 0 )
         {
            log_enable_error( true );
         }
         else if ( strcmp( argv[t], "--no-debug" ) == 0 )
         {
            log_enable_debug( false );
         }
         else if ( strcmp( argv[t], "--no-warnings" ) == 0 )
         {
            log_enable_warning( false );
         }
         else if ( strcmp( argv[t], "--no-errors" ) == 0 )
         {
            log_enable_error( false );
         }
         else
         {
            fprintf( stderr, "Unknown argument: %s\n", argv[t] );
            haveBadArgument = true;
         }
      }
      else
      {
         readingOptions = false;
         opts_done = t;
         break;
      }
   }

   if ( haveBadArgument )
   {
      // TODO do something sensible here
      //exit( -1 );
   }

   if ( readingOptions == true )
   {
      argc = 1;
   }
   else
   {
      if ( opts_done != 0 )
      {
         for ( int n = 0; opts_done + n < argc; n++ )
         {
            argv[n+1] = argv[ opts_done + n ];
         }
         argc = argc - opts_done + 1;
      }
   }

   return 0;
}

void shutdown_cmdline()
{
   cmdline_deleteOpenModels();
}

int cmdline_command( int & argc, char * argv[] )
{
   unsigned errors = 0;

   if ( _doFilterTest )
   {
      return modelTestRun( _filterDir );
   }

   if ( _doModelTest )
   {
      int failed = 0;
      StringList::iterator it = _modelTests.begin();
      for ( ; it != _modelTests.end(); it++ )
      {
         failed += model_test( (*it).c_str() );
      }
      return failed;
   }

   if ( _doTextureTest )
   {
      std::string master = _textureFiles.front();

      StringList::iterator it = _textureFiles.begin();
      it++;
      for ( ; it != _textureFiles.end(); it++ )
      {
         texture_test_compare( master.c_str(), (*it).c_str(), 10 );
      }
      return 0;
   }

   FilterManager * mgr = FilterManager::getInstance();

   for ( int t = 1; t < argc; t++ )
   {
      Model::ModelErrorE err = Model::ERROR_NONE;
      Model * m = new Model;
      if ( (err = mgr->readFile( m, argv[t] )) == Model::ERROR_NONE )
      {
         m->loadTextures( 0 );
         _models.push_back( m );
      }
      else
      {
         std::string reason = argv[t];
         reason += ": ";
         reason += transll( Model::errorToString( err, m ) );
         msg_error( reason.c_str() );
         delete m;
      }
   }
   
   if ( argc <= 1 )
   {
      if ( _doScripts )
      {
         Model * m = new Model();
         _models.push_back( m );
      }
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
         std::string outfile = replaceExtension( infile, _convertFormat );
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

      if ( _doSave )
      {
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

   if ( _doBatch )
   {
      cmdline_runui = false;
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

