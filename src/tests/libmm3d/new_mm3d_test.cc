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


// This file tests the MM3D model file filter against a reference version.

// FIXME add more models (particularly textured ones)

#include <QtTest/QtTest>
#include <unistd.h>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "log.h"
#include "mm3dfilter.h"
#include "mm3dfilter_ref.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"


Model * loadModelOrDie( const char * filename, bool useReference )
{
   Model * model = new Model;

   local_ptr<ModelFilter> f;
   if ( useReference )
      f = new MisfitFilterRef;
   else
      f = new MisfitFilter;

   Model::ModelErrorE err = f->readFile( model, filename );

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "fatal: %s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      exit( -1 );
   }

   model->forceAddOrDelete( true );
   return model;
}

class NewMm3dTest : public QObject
{
   Q_OBJECT
private:
      void testModelFile( const char * file )
      {
         // The lhs pointer is from the original filter
         local_ptr<Model> lhs = loadModelOrDie( file, true );
         local_ptr<Model> rhs = loadModelOrDie( file, false );

         int bits = Model::CompareAll;
         QVERIFY_EQ( bits, lhs->equal( rhs.get(), bits ) );

         // FIXME should really use a temp file based on something
         // unique (hostname-pid?) so that multiple tests could run in parallel.
         const char tmpFile[] = "tmp_new_mm3d_test.mm3d";
         MisfitFilter f;
         QVERIFY_EQ( Model::ERROR_NONE, f.writeFile( rhs.get(), tmpFile ) );

         local_ptr<Model> written = loadModelOrDie( tmpFile, false );

         QVERIFY_EQ( 0, unlink( tmpFile ) );
      }

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   void testModelEqualTest()
   {
      testModelFile( "data/model_equal_test.mm3d" );
   }

   void testModelHiddenTest()
   {
      testModelFile( "data/model_hidden_test.mm3d" );
   }
};

QTEST_MAIN(NewMm3dTest)
#include "new_mm3d_test.moc"

