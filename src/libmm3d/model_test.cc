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

#include "model_test.h"
#include "model.h"
#include "filtermgr.h"

#include <stdarg.h>

typedef int (*_testfunc)();

struct _ModelTest_t
{
   char      name[32];
   _testfunc func;
};
typedef struct _ModelTest_t ModelTestT;

static void _error( const char * fmt, ... )
{
   va_list ap;

   va_start( ap, fmt );
   vfprintf( stderr, fmt, ap );
}

static int _loadModels( const char * source, const char * target,
      Model * m1, Model * m2, Model * m3 )
{
   FilterManager * mgr = FilterManager::getInstance();

   Model::ModelErrorE err1 = mgr->readFile( m1, source );
   Model::ModelErrorE err2 = mgr->readFile( m2, target );
   Model::ModelErrorE err3 = mgr->readFile( m3, source );

   if ( err1 != Model::ERROR_NONE )
   {
      const char * str = Model::errorToString( err1, m1 );
      _error( "%s: %s\n", source, str ); 
      return 1;
   }

   if ( err2 != Model::ERROR_NONE )
   {
      const char * str = Model::errorToString( err1, m2 );
      _error( "%s: %s\n", target, str ); 
      return 1;
   }

   if ( err3 != Model::ERROR_NONE )
   {
      const char * str = Model::errorToString( err1, m3 );
      _error( "%s: %s\n", source, str ); 
      return 1;
   }

   return 0;
}

static int _startCompare( const char * test, Model * m1, Model * m2, Model * m3 )
{
   if ( (m3->equivalent( m1 )) )
   {
      _error( "%s: initial models do not match\n", test );
      return 1;
   }
   if ( (m3->equivalent( m2 )) )
   {
      _error( "%s: source/target models match\n", test );
      return 1;
   }
   return 0;
}

static int _endCompare( const char * test, Model * m1, Model * m2, Model * m3 )
{
   if ( (m3->equivalent( m2 )) )
   {
      _error( "%s: model does not match target after operation\n", test );
      return 1;
   }
   m3->operationComplete( "Test Operation" );
   m3->undo();
   if ( (m3->equivalent( m1 )) )
   {
      _error( "%s: model does not match source after undo\n", test );
      return 1;
   }
   m3->redo();
   if ( (m3->equivalent( m2 )) )
   {
      _error( "%s: model does not match target after redo\n", test );
      return 1;
   }
   return 0;
}

static void _cleanUp( Model * m1, Model * m2, Model * m3 )
{
   delete m1;
   delete m2;
   delete m3;
}

int model_test_set_as_group()
{
   Model * m1 = new Model;
   Model * m2 = new Model;
   Model * m3 = new Model;

   int rval = 1;

   if ( 0 == _loadModels( "testmodels/mt_set_as_group_1.mm3d",
         "testmodels/mt_set_as_group_2.mm3d",
         m1, m2, m3 ) )
   {
      if ( 0 == _startCompare( "set_as_group", m1, m2, m3 ) )
      {
         m3->setSelectedAsGroup( 1 );
         if ( 0 == _endCompare( "set_as_group", m1, m2, m3 ) )
         {
            rval = 0;
         }
      }
   }
   _cleanUp( m1, m2, m3 );

   return rval;
}

int model_test_add_to_group()
{
   Model * m1 = new Model;
   Model * m2 = new Model;
   Model * m3 = new Model;

   int rval = 1;

   if ( 0 == _loadModels( "testmodels/mt_add_to_group_1.mm3d",
         "testmodels/mt_add_to_group_2.mm3d",
         m1, m2, m3 ) )
   {
      if ( 0 == _startCompare( "add_to_group", m1, m2, m3 ) )
      {
         m3->addSelectedToGroup( 1 );
         if ( 0 == _endCompare( "add_to_group", m1, m2, m3 ) )
         {
            rval = 0;
         }
      }
   }
   _cleanUp( m1, m2, m3 );

   return rval;
}

int model_test_delete_faces()
{
   Model * m1 = new Model;
   Model * m2 = new Model;
   Model * m3 = new Model;

   int rval = 1;

   if ( 0 == _loadModels( "testmodels/mt_delete_faces_1.mm3d",
         "testmodels/mt_delete_faces_2.mm3d",
         m1, m2, m3 ) )
   {
      if ( 0 == _startCompare( "delete_faces", m1, m2, m3 ) )
      {
         m3->deleteSelected();
         if ( 0 == _endCompare( "delete_faces", m1, m2, m3 ) )
         {
            rval = 0;
         }
      }
   }
   _cleanUp( m1, m2, m3 );

   return rval;
}

int model_test_delete_vertices()
{
   Model * m1 = new Model;
   Model * m2 = new Model;
   Model * m3 = new Model;

   int rval = 1;

   if ( 0 == _loadModels( "testmodels/mt_delete_vertices_1.mm3d",
         "testmodels/mt_delete_vertices_2.mm3d",
         m1, m2, m3 ) )
   {
      if ( 0 == _startCompare( "delete_vertices", m1, m2, m3 ) )
      {
         m3->deleteSelected();
         if ( 0 == _endCompare( "delete_vertices", m1, m2, m3 ) )
         {
            rval = 0;
         }
      }
   }
   _cleanUp( m1, m2, m3 );

   return rval;
}

static const unsigned int TEST_MAX = 4;

static ModelTestT _tests[ TEST_MAX ] =
{ 
   { "delete_vertices", model_test_delete_vertices },
   { "delete_faces",    model_test_delete_faces },
   { "set_as_group",    model_test_set_as_group },
   { "add_to_group",    model_test_add_to_group }
};

int model_test( const char * test )
{
   bool all   = false;
   bool known = false;
   int failed = 0;

   if ( strcmp( test, "all" ) == 0 )
   {
      all = true;
   }

   for ( unsigned int t = 0; t < TEST_MAX; t++ )
   {
      if ( all || strcmp( test, _tests[t].name ) == 0 )
      {
         known = true;
         int rval =  _tests[t].func();
         failed += rval;
         if ( rval != 0 )
         {
            printf( "%s failed\n", test );
         }
      }
   }

   if ( !known )
   {
      printf( "unknown test %s\n", test );
   }

   if ( known && failed == 0 )
   {
      printf( "%s passed\n", test );
   }

   return 0;
}

