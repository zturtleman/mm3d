/*  Misfit Model 3D
 * 
 *  Copyright (c) 2007-2008 Kevin Worcester
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


// This file tests the MD3 model file filter.

#include <QtTest/QtTest>
#include <unistd.h>

#include "test_common.h"
#include "testfilefactory.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "log.h"
#include "msg.h"
#include "md3filter.h"
#include "mm3dfilter.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"

#include "texmgr.h"
#include "pcxtex.h"
#include "tgatex.h"
#include "rawtex.h"
#include "qttex.h"

int init_std_texture_filters()
{
   log_debug( "initializing standard texture filters\n" );

   TextureManager * textureManager = TextureManager::getInstance();

   TextureFilter * filter;

   filter = new TgaTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new RawTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new PcxTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new QtTextureFilter;
   textureManager->registerTextureFilter( filter );

   return 0;
}


Model * loadModelOrDie( ModelFilter & filter, const char * filename )
{
   Model * model = new Model;
   model->forceAddOrDelete( true );

   Model::ModelErrorE err = filter.readFile( model, filename );

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "fatal: read %s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      exit( -1 );
   }

   model->loadTextures();
   model->forceAddOrDelete( true );
   return model;
}

Model * loadMm3dOrDie( const char * filename, FileFactory * factory = NULL )
{
   MisfitFilter f;
   if ( factory )
      f.setFactory( factory );
   return loadModelOrDie( f, filename );
}

Model * loadMd3OrDie( const char * filename, FileFactory * factory = NULL )
{
   Md3Filter f;
   if ( factory )
      f.setFactory( factory );
   return loadModelOrDie( f, filename );
}

void saveMd3OrDie( Model * model, const char * filename, FileFactory * factory = NULL )
{
   Md3Filter filter;
   if ( factory )
      filter.setFactory( factory );

   Model::ModelErrorE err = filter.writeFile( model, filename );

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "fatal: write %s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      exit( -1 );
   }
}

char prompt_no( const char * str, const char * opts )
{
   return 'N';
}


class FilterMd3Test : public QObject
{
   Q_OBJECT
private:
   void testModelFile( const char * lhs_file, const Model * rhs, bool equivOk = false )
   {
      // The lhs pointer is from the original filter
      local_ptr<Model> lhs = loadMm3dOrDie( lhs_file );

      if ( equivOk )
      {
         QVERIFY_TRUE( lhs->equivalent( rhs, 0.03 ) );
      }
      else
      {
         QVERIFY_TRUE( lhs->propEqual( rhs, Model::PartAll, Model::PropAll, 0.001 ) );
      }
   }

   void testReadAndWrite( const char * infile, const char * outfile,
         const char * reffile )
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMd3OrDie( infile, &factory );
      testModelFile( reffile, m.get() );

      saveMd3OrDie( m.get(), outfile, &factory );
      m = loadMd3OrDie( outfile, &factory );
      testModelFile( reffile, m.get(), true );
   }

private slots:

   void initTestCase()
   {
      init_std_texture_filters();
      log_enable_debug( false );
      log_enable_warning( true );
      log_enable_error( true );
   }

   // Load a player model
   void testMd3ModelA()
   {
      msg_register_prompt( NULL, NULL, NULL );
      testReadAndWrite(
            "filtertest/md3/cloud/head.md3",
            "filtertest/md3/cloud_out/head.md3",
            "filtertest/md3/cloud/cloud.mm3d" );
   }

   // Load just the head
   void testMd3ModelAHead()
   {
      msg_register_prompt( prompt_no, prompt_no, prompt_no );
      testReadAndWrite(
            "filtertest/md3/cloud/head.md3",
            "filtertest/md3/cloud_out/head.md3",
            "filtertest/md3/cloud/head.mm3d" );
      msg_register_prompt( NULL, NULL, NULL );
   }

   // Load just the lower body
   void testMd3ModelALower()
   {
      msg_register_prompt( prompt_no, prompt_no, prompt_no );
      testReadAndWrite(
            "filtertest/md3/cloud/lower.md3",
            "filtertest/md3/cloud_out/lower.md3",
            "filtertest/md3/cloud/lower.mm3d" );
      msg_register_prompt( NULL, NULL, NULL );
   }

   // FIXME add tests:
   //   MD3_PATH handling:
   //      common path
   //      unique paths
   //   Make sure that single frame MD3 does not have any animations
   //   error handling
};

QTEST_MAIN(FilterMd3Test)
#include "filter_md3_test.moc"

