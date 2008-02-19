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


// This file tests the equality functions of the model components (inner
// classes), and the whole model class.

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "log.h"
#include "mm3dfilter.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"


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

      QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
         lhs->addTriangleToGroup( 0, 0 );
         rhs->addTriangleToGroup( 0, 0 );
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
         lhs->addTriangleToGroup( 0, 0 );
         rhs->addTriangleToGroup( 0, 0 );
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
   }

   // FIXME test influences
   void testPointExact()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs = newTestModel();

      lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

      rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );

      QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
   }

   void testOnePointDoubled()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addPoint( "Left2", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addPoint( "Right2", 5, 6, 7, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addPoint( "Left2", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );
         rhs->addPoint( "Right2", 5, 6, 7, 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );
         lhs->addPoint( "Left2", 5, 6, 7, 0, 3, 2 );

         rhs->addPoint( "Right2", 5, 6, 7, 0, 3, 2 );
         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 2 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
   }

   void testPointCoordMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 6, 6, 7, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 5, 7, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 5, 0, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
   }

   void testPointRotMismatch()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 1, 1, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 2 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 1, 2 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 1, 0 );

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, PI, 0, 0 );

         rhs->addPoint( "Right", 5, 6, 7, -PI, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, PI, 0 );

         rhs->addPoint( "Right", 5, 6, 7, 0, -PI, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 0, PI );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, -PI );

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 2*PI, 0, 0 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 2*PI, 0 );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addPoint( "Left", 5, 6, 7, 0, 0, 2*PI );

         rhs->addPoint( "Right", 5, 6, 7, 0, 0, 0 );

         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
      QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
      QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 5, 7, 0, 1, 2 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 6, 5, 0, 1, 2 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, 2 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 1, 2 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 1, 1 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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
         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, PI, 0 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, -PI, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 0, PI );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, -PI );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 2*PI, 0, 0 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 2*PI, 0 );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
      {
         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs = newTestModel();

         lhs->addBoneJoint( "Left", 5, 6, 7, 0, 0, 2*PI );
         rhs->addBoneJoint( "Right", 5, 6, 7, 0, 0, 0 );

         lhs->setupJoints();
         rhs->setupJoints();
         QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

      QVERIFY_TRUE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
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

         QVERIFY_FALSE( lhs->equivalent( rhs.get(), 0.00001 ) );
      }
   }

   // FIXME
   //   * Test texture coordinate rotation
   //   * Test texture coordinate mismatch (significant and not)
   //   * Test group match with textures (match/mismatch)
   //   * Test vertex influences (match/mismatch)
   //   * Test point influences (match/mismatch)

};

// FIXME remove tolerance

QTEST_MAIN(ModelEquivTest)
#include "model_equiv_test.moc"

