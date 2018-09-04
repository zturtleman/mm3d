/*  Maverick Model 3D
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


// This file tests the equality functions of the model components (inner
// classes), and the whole model class.

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "log.h"
#include "mm3dfilter.h"
#include "tgatex.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"


Texture * loadTextureOrDie( TextureFilter * f, const char * filename )
{
   Texture * tex = new Texture;
   Texture::ErrorE err = f->readFile( tex, filename );

   if ( err != Texture::ERROR_NONE )
   {
      fprintf( stderr, "fatal: %s: %s\n", filename, Texture::errorToString( err ) );
      delete tex;
      exit( -1 );
   }

   return tex;
}

Texture * loadTgaOrDie( const char * filename )
{
   TgaTextureFilter f;
   return loadTextureOrDie( &f, filename );
}

class ModelEquivTest : public QObject
{
   Q_OBJECT
private:

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
      log_enable_warning( true );
      log_enable_error( false );
   }

   // Many primitives are recycled. Test initial conditions, change conditions,
   // release, and re-get to make sure that the recyled primitives are properly
   // initialized.

   void testEmpty()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testExactCompare()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 2, 1, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addTriangle( 2, 1, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      // Invert triangles 0 and 1 in rhs
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 2, 1, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 2, 1, 0 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   // Checks that one triangle doesn't match two triangles in the other model.
   void testNoDouble()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addTriangle( 2, 1, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 2, 1, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testDoulbedTriangle()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testOffset()
   {
      // Rotate the triangle's vertex indices and see if they still match
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 2, 0, 1 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 1, 2, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testVertexIndex()
   {
      // Re-order the vertex indices and see if they still match
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 2, 2, 2 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 0, 0, 0 );
         rhs->addTriangle( 2, 1, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addTriangle( 0, 2, 1 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testInvertedNormal()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 2, 1, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 1, 0, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 2, 1 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testUnusedGroup()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Unused 1" );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Unused 2" );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testBothGrouped()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Group A" );
         lhs->addTriangleToGroup( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Group B" );
         rhs->addTriangleToGroup( 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Group A" );
         lhs->addGroup( "Group B" );
         lhs->addTriangleToGroup( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Group A" );
         rhs->addGroup( "Group B" );
         rhs->addTriangleToGroup( 1, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Group A" );
         lhs->addGroup( "Group B" );
         lhs->addTriangleToGroup( 1, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Group A" );
         rhs->addGroup( "Group B" );
         rhs->addTriangleToGroup( 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testOneGrouped()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Unused 1" );
         lhs->addTriangleToGroup( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Unused 2" );
         rhs->addTriangleToGroup( 0, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testGroupMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Group A" );
         lhs->setGroupSmooth( 0, 1 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Group A" );
         rhs->setGroupSmooth( 0, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
         lhs->addTriangleToGroup( 0, 0 );
         rhs->addTriangleToGroup( 0, 0 );
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Group A" );
         lhs->setGroupAngle( 0, 1 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Group A" );
         rhs->setGroupAngle( 0, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
         lhs->addTriangleToGroup( 0, 0 );
         rhs->addTriangleToGroup( 0, 0 );
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testGroupSplit()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 2, 1, 0 );
         lhs->addGroup( "Single" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTriangleToGroup( 0, 1 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addTriangle( 2, 1, 0 );
         rhs->addGroup( "Group A" );
         rhs->addGroup( "Group B" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTriangleToGroup( 1, 1 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 2, 1, 0 );
         lhs->addGroup( "Single" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTriangleToGroup( 0, 1 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addTriangle( 2, 1, 0 );
         rhs->addGroup( "Group A" );
         rhs->addGroup( "Group B" );
         rhs->addTriangleToGroup( 0, 1 );
         rhs->addTriangleToGroup( 1, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 2, 1, 0 );
         lhs->addGroup( "Group A" );
         lhs->addGroup( "Group B" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTriangleToGroup( 1, 1 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addTriangle( 2, 1, 0 );
         rhs->addGroup( "Single" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTriangleToGroup( 0, 1 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addTriangle( 2, 1, 0 );
         lhs->addGroup( "Group A" );
         lhs->addGroup( "Group B" );
         lhs->addTriangleToGroup( 0, 1 );
         lhs->addTriangleToGroup( 1, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addTriangle( 2, 1, 0 );
         rhs->addGroup( "Single" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTriangleToGroup( 0, 1 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testCoordMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 1 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 0 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 0 );
         rhs->addTriangle( 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testMaterialMatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addColorMaterial( "Left Mat" );
         lhs->setGroupTextureId( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addColorMaterial( "Right Mat" );
         rhs->setGroupTextureId( 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex1.get() );
         rhs->setGroupTextureId( 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      // Texture file contents differ, but pixel data matches
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );
         local_ptr<Texture> tex2 = loadTgaOrDie( "data/test_rgb_uncomp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex2.get() );
         rhs->setGroupTextureId( 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testMaterialUnmatched()
   {
      // Material property mismatch
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addColorMaterial( "Left Mat" );
         lhs->setGroupTextureId( 0, 0 );
         lhs->setTextureShininess( 0, 0.5f );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addColorMaterial( "Right Mat" );
         rhs->setGroupTextureId( 0, 0 );
         rhs->setTextureShininess( 0, 0.6f );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      // Right Material unassigned
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addColorMaterial( "Left Mat" );
         lhs->setGroupTextureId( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addColorMaterial( "Right Mat" );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      // Left material unassigned
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addColorMaterial( "Left Mat" );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addColorMaterial( "Right Mat" );
         rhs->setGroupTextureId( 0, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      // Right group unassigned
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addColorMaterial( "Left Mat" );
         lhs->setGroupTextureId( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addColorMaterial( "Right Mat" );
         rhs->setGroupTextureId( 0, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      // Left group unassigned
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addColorMaterial( "Left Mat" );
         lhs->setGroupTextureId( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addColorMaterial( "Right Mat" );
         rhs->setGroupTextureId( 0, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      // Texture data mismatch
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );
         local_ptr<Texture> tex2 = loadTgaOrDie( "data/test_rgba_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex2.get() );
         rhs->setGroupTextureId( 0, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testTexCoordMatch()
   {
      local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 2, 2, 2 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addGroup( "Left Group" );
      lhs->addTriangleToGroup( 0, 0 );
      lhs->addTexture( tex1.get() );
      lhs->setGroupTextureId( 0, 0 );
      lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
      lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
      lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

      rhs->addVertex( 0, 0, 0 );
      rhs->addVertex( 1, 1, 1 );
      rhs->addVertex( 2, 2, 2 );
      rhs->addTriangle( 0, 1, 2 );
      rhs->addGroup( "Right Group" );
      rhs->addTriangleToGroup( 0, 0 );
      rhs->addTexture( tex1.get() );
      rhs->setGroupTextureId( 0, 0 );
      rhs->setTextureCoords( 0, 0, 0.1, 0.9 );
      rhs->setTextureCoords( 0, 1, 0.2, 0.8 );
      rhs->setTextureCoords( 0, 2, 0.3, 0.7 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testTexCoordMismatch()
   {
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );
         lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex1.get() );
         rhs->setGroupTextureId( 0, 0 );
         rhs->setTextureCoords( 0, 0, 0.2, 0.9 );
         rhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         rhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );
         lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex1.get() );
         rhs->setGroupTextureId( 0, 0 );
         rhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         rhs->setTextureCoords( 0, 1, 0.2, 0.9 );
         rhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );
         lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex1.get() );
         rhs->setGroupTextureId( 0, 0 );
         rhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         rhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         rhs->setTextureCoords( 0, 2, 0.1, 0.7 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testTexCoordMismatchIgnored()
   {
      // No group material
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex1.get() );
         rhs->setTextureCoords( 0, 0, 0.2, 0.9 );
         rhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         rhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      // Triangle not grouped
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );
         lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTexture( tex1.get() );
         rhs->setGroupTextureId( 0, 0 );
         rhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         rhs->setTextureCoords( 0, 1, 0.2, 0.9 );
         rhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      // Material is not texture map
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addColorMaterial( "Mat Left" );
         lhs->setGroupTextureId( 0, 0 );
         lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addColorMaterial( "Mat Right" );
         rhs->setGroupTextureId( 0, 0 );
         rhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         rhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         rhs->setTextureCoords( 0, 2, 0.1, 0.7 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testTexCoordOffset()
   {
      // Vert and tex coords rotated, match
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );
         lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 2, 0, 1 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex1.get() );
         rhs->setGroupTextureId( 0, 0 );
         rhs->setTextureCoords( 0, 0, 0.3, 0.7 );
         rhs->setTextureCoords( 0, 1, 0.1, 0.9 );
         rhs->setTextureCoords( 0, 2, 0.2, 0.8 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      // Only tex coords rotated, no match
      {
         local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addGroup( "Left Group" );
         lhs->addTriangleToGroup( 0, 0 );
         lhs->addTexture( tex1.get() );
         lhs->setGroupTextureId( 0, 0 );
         lhs->setTextureCoords( 0, 0, 0.1, 0.9 );
         lhs->setTextureCoords( 0, 1, 0.2, 0.8 );
         lhs->setTextureCoords( 0, 2, 0.3, 0.7 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addGroup( "Right Group" );
         rhs->addTriangleToGroup( 0, 0 );
         rhs->addTexture( tex1.get() );
         rhs->setGroupTextureId( 0, 0 );
         rhs->setTextureCoords( 0, 2, 0.1, 0.9 );
         rhs->setTextureCoords( 0, 0, 0.2, 0.8 );
         rhs->setTextureCoords( 0, 1, 0.3, 0.7 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   // FIXME test influences
   void testPointExact()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

      rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testOnePointDoubled()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addPoint( "Left2", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addPoint( "Right2", 5, 6, 7, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testBothPointsDoubled()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addPoint( "Left2", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addPoint( "Right2", 5, 6, 7, 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addPoint( "Left2", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addPoint( "Right2", 5, 6, 7, 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testPointsInverted()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addPoint( "Left2", 4, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 4, 6, 7, 0, 1, 2 );
         rhs->addPoint( "Right2", 5, 6, 7, 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addPoint( "Left2", 5, 6, 7, 0, 3, 2 );

         rhs->addPoint( "Right2", 5, 6, 7, 0, 3, 2 );
         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testPointCoordMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 6, 6, 7, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 5, 7, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 5, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testPointRotMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 1, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   // Test points where rotations angles are not equal, but describe
   // the same orientation.
   void testPointRotEquiv()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 0, 0 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, PI, 0, 0 );

         rhs->addPoint( "Right", 5, 6, 7, -PI, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, PI, 0 );

         rhs->addPoint( "Right", 5, 6, 7, 0, -PI, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 0, PI );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, -PI );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 2*PI, 0, 0 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 2*PI, 0 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 0, 2*PI );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testJointExact()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
      lhs->addBoneJoint( "Left A", 2, 3, 4, 1, 1, 1, 0);
      lhs->addBoneJoint( "Left B", 3, 4, 5, 2, 2, 2, 0);

      rhs->addBoneJoint( "Right", 5, 6, 7, 0, 1, 2 );
      rhs->addBoneJoint( "Right A", 2, 3, 4, 1, 1, 1, 0);
      rhs->addBoneJoint( "Right B", 3, 4, 5, 2, 2, 2, 0);

      lhs->setupJoints();
      rhs->setupJoints();
      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testOneJointDoubled()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addBoneJoint( "Left A", 2, 3, 4, 1, 1, 1, 0);
         lhs->addBoneJoint( "Left B", 3, 4, 5, 2, 2, 2, 0);

         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right A", 2, 3, 4, 1, 1, 1, 0);
         rhs->addBoneJoint( "Right B", 3, 4, 5, 2, 2, 2, 0);
         rhs->addBoneJoint( "Right B", 3, 4, 5, 2, 2, 2, 0);

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addBoneJoint( "Left A", 2, 3, 4, 1, 1, 1, 0);
         lhs->addBoneJoint( "Left B", 3, 4, 5, 2, 2, 2, 0);
         lhs->addBoneJoint( "Left B", 3, 4, 5, 2, 2, 2, 0);

         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right A", 2, 3, 4, 1, 1, 1, 0);
         rhs->addBoneJoint( "Right B", 3, 4, 5, 2, 2, 2, 0);

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testBothJointsDoubled()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
      lhs->addBoneJoint( "Left A", 2, 3, 4, 1, 1, 1, 0);
      lhs->addBoneJoint( "Left B", 2, 3, 4, 1, 1, 1, 0);

      rhs->addBoneJoint( "Right", 5, 6, 7, 0, 1, 2 );
      rhs->addBoneJoint( "Right A", 2, 3, 4, 1, 1, 1, 0);
      rhs->addBoneJoint( "Right B", 2, 3, 4, 1, 1, 1, 0);

      lhs->setupJoints();
      rhs->setupJoints();
      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testJointsInverted()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addBoneJoint( "Left A", 2, 3, 4, 1, 1, 1, 0);
         lhs->addBoneJoint( "Left B", 3, 4, 5, 2, 2, 2, 0);

         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right B", 3, 4, 5, 2, 2, 2, 0);
         rhs->addBoneJoint( "Right A", 2, 3, 4, 1, 1, 1, 0);

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addBoneJoint( "Left B", 3, 4, 5, 2, 2, 2, 0);
         lhs->addBoneJoint( "Left A", 2, 3, 4, 1, 1, 1, 0);

         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right A", 2, 3, 4, 1, 1, 1, 0);
         rhs->addBoneJoint( "Right B", 3, 4, 5, 2, 2, 2, 0);

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testJointCoordMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 6, 6, 7, 0, 1, 2 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 5, 7, 0, 1, 2 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 6, 5, 0, 1, 2 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testJointRotMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 1, 1, 2 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, 2 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 1, 1 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   // Test joints where rotations angles are not equal, but describe
   // the same orientation.
   void testJointRotEquiv()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, PI, 0, 0 );
         rhs->addBoneJoint( "Right", 5, 6, 7, -PI, 0, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, PI, 0 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, -PI, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 0, PI );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, -PI );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 2*PI, 0, 0 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 2*PI, 0 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 0, 2*PI );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testParentDoubled()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0);
      lhs->addBoneJoint( "Left B", 2, 2, 2, 0, 0, 0, 0);
      lhs->addBoneJoint( "Left AA", 3, 3, 3, 0, 0, 0, 1);

      rhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0);
      rhs->addBoneJoint( "Left B", 2, 2, 2, 0, 0, 0, 0);
      rhs->addBoneJoint( "Left BB", 3, 3, 3, 0, 0, 0, 2);

      lhs->setupJoints();
      rhs->setupJoints();

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testParentMismatch()
   {
      // Absolute position is the same, different parent
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0);
         lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0);
         lhs->addBoneJoint( "Left C", 4, 4, 4, 0, 0, 0, 1);

         rhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0);
         rhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0);
         rhs->addBoneJoint( "Left C", 4, 4, 4, 0, 0, 0, 2);

         lhs->setupJoints();
         rhs->setupJoints();

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      // Relative position is the same, different parent
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0);
         lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0);
         lhs->addBoneJoint( "Left C", 4, 4, 4, 0, 0, 0, 1);

         rhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0);
         rhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0);
         rhs->addBoneJoint( "Left C", 5, 5, 5, 0, 0, 0, 2);

         lhs->setupJoints();
         rhs->setupJoints();

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testVertexInfluence()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 2, 2, 2 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
      lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 1.0 );

      rhs->addVertex( 0, 0, 0 );
      rhs->addVertex( 1, 1, 1 );
      rhs->addVertex( 2, 2, 2 );
      rhs->addTriangle( 0, 1, 2 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
      rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 1.0 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testVertexInfluenceMissing()
   {
      // Missing Right
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 1.0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      // Missing Left
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 1.0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testVertexInfluenceMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 1.0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addVertexInfluence( 1, 1, Model::IT_Custom, 1.0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addVertexInfluence( 1, 1, Model::IT_Custom, 1.0 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 1.0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testVertexInfluenceOffset()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 2, 2, 2 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 1.0 );

      rhs->addVertex( 0, 0, 0 );
      rhs->addVertex( 1, 1, 1 );
      rhs->addVertex( 2, 2, 2 );
      rhs->addTriangle( 2, 0, 1 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.5 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testVertexInfluenceMultiple()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 2, 2, 2 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
      lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
      lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.7 );
      lhs->addVertexInfluence( 1, 1, Model::IT_Custom, 0.3 );

      rhs->addVertex( 0, 0, 0 );
      rhs->addVertex( 1, 1, 1 );
      rhs->addVertex( 2, 2, 2 );
      rhs->addTriangle( 0, 1, 2 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
      rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
      rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.7 );
      rhs->addVertexInfluence( 1, 1, Model::IT_Custom, 0.3 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testVertexInfluenceWeighted()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 2, 2, 2 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
      lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
      lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.50 );
      lhs->addVertexInfluence( 1, 1, Model::IT_Custom, 0.25 );

      rhs->addVertex( 0, 0, 0 );
      rhs->addVertex( 1, 1, 1 );
      rhs->addVertex( 2, 2, 2 );
      rhs->addTriangle( 0, 1, 2 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
      rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
      rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.20 );
      rhs->addVertexInfluence( 1, 1, Model::IT_Custom, 0.10 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testVertexInfluenceInverted()
   {
      // Invert bone joint order for 1 and 2
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
         lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.7 );
         lhs->addVertexInfluence( 1, 1, Model::IT_Custom, 0.3 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.7 );
         rhs->addVertexInfluence( 1, 2, Model::IT_Custom, 0.3 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      // Invert influence order.
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addVertex( 0, 0, 0 );
         lhs->addVertex( 1, 1, 1 );
         lhs->addVertex( 2, 2, 2 );
         lhs->addTriangle( 0, 1, 2 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
         lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.7 );
         lhs->addVertexInfluence( 1, 1, Model::IT_Custom, 0.3 );

         rhs->addVertex( 0, 0, 0 );
         rhs->addVertex( 1, 1, 1 );
         rhs->addVertex( 2, 2, 2 );
         rhs->addTriangle( 0, 1, 2 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
         rhs->addVertexInfluence( 1, 1, Model::IT_Custom, 0.3 );
         rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.7 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   // Test that a weight of zero counts as no influence
   void testVertexInfluenceZero()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 2, 2, 2 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
      lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
      lhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.50 );
      lhs->addVertexInfluence( 1, 1, Model::IT_Custom, 0.0 );

      rhs->addVertex( 0, 0, 0 );
      rhs->addVertex( 1, 1, 1 );
      rhs->addVertex( 2, 2, 2 );
      rhs->addTriangle( 0, 1, 2 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
      rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
      rhs->addVertexInfluence( 1, 0, Model::IT_Custom, 0.20 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testPointInfluence()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
      lhs->addPointInfluence( 1, 0, Model::IT_Custom, 1.0 );

      rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
      rhs->addPointInfluence( 1, 0, Model::IT_Custom, 1.0 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testPointInfluenceMissing()
   {
      // Missing Right
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addPointInfluence( 1, 0, Model::IT_Custom, 1.0 );

         rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      // Missing Left
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );

         rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addPointInfluence( 1, 0, Model::IT_Custom, 1.0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testPointInfluenceMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addPointInfluence( 1, 0, Model::IT_Custom, 1.0 );

         rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addPointInfluence( 1, 1, Model::IT_Custom, 1.0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addPointInfluence( 1, 1, Model::IT_Custom, 1.0 );

         rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addPointInfluence( 1, 0, Model::IT_Custom, 1.0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get() ) );
      }
   }

   void testPointInfluenceOffset()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addPointInfluence( 1, 0, Model::IT_Custom, 1.0 );

      rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.5 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testPointInfluenceMultiple()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
      lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
      lhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.7 );
      lhs->addPointInfluence( 1, 1, Model::IT_Custom, 0.3 );

      rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
      rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
      rhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.7 );
      rhs->addPointInfluence( 1, 1, Model::IT_Custom, 0.3 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testPointInfluenceWeighted()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
      lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
      lhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.50 );
      lhs->addPointInfluence( 1, 1, Model::IT_Custom, 0.25 );

      rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
      rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
      rhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.20 );
      rhs->addPointInfluence( 1, 1, Model::IT_Custom, 0.10 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   void testPointInfluenceInverted()
   {
      // Invert bone joint order for 1 and 2
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
         lhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.7 );
         lhs->addPointInfluence( 1, 1, Model::IT_Custom, 0.3 );

         rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.7 );
         rhs->addPointInfluence( 1, 2, Model::IT_Custom, 0.3 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
      // Invert influence order.
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
         lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
         lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
         lhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.7 );
         lhs->addPointInfluence( 1, 1, Model::IT_Custom, 0.3 );

         rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
         rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
         rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
         rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
         rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
         rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
         rhs->addPointInfluence( 1, 1, Model::IT_Custom, 0.3 );
         rhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.7 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
      }
   }

   // Test that a weight of zero counts as no influence
   void testPointInfluenceZero()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      lhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      lhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      lhs->addBoneJoint( "Left", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Left A", 2, 2, 2, 0, 0, 0, 0 );
      lhs->addBoneJoint( "Left B", 3, 3, 3, 0, 0, 0, 0 );
      lhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.50 );
      lhs->addPointInfluence( 1, 1, Model::IT_Custom, 0.0 );

      rhs->addPoint( "Point", 0, 0, 0, 0, 0, 0 );
      rhs->addPoint( "Point", 1, 1, 1, 0, 0, 0 );
      rhs->addPoint( "Point", 2, 2, 2, 0, 0, 0 );
      rhs->addBoneJoint( "Right", 1, 1, 1, 0, 0, 0 );
      rhs->addBoneJoint( "Right A", 2, 2, 2, 0, 0, 0, 0 );
      rhs->addBoneJoint( "Right B", 3, 3, 3, 0, 0, 0, 0 );
      rhs->addPointInfluence( 1, 0, Model::IT_Custom, 0.20 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get() ) );
   }

   // FIXME
   //   * Test influence weight mismatch
   //   * Test animations

};

// FIXME remove tolerance

QTEST_MAIN(ModelEquivTest)
#include "model_equiv_test.moc"

