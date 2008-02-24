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


// This file tests vertex welding and unwelding

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "weld.h"
#include "modelstatus.h"
#include "log.h"
#include "mm3dfilter.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"


class ModelWeldTest : public QObject
{
   Q_OBJECT
private:
   void addWeldTriangles( Model * m, int vertCount )
   {
      if ( vertCount != 3 && vertCount != 4 && vertCount != 5 && vertCount != 6 )

      {
         QTest::qFail( "Vert count was not 3-6", __FILE__, __LINE__ );
         exit( -1 );
      }

      m->addVertex( 0, 0, 0 );
      m->addVertex( 1, 1, 0 );
      m->addVertex( 1, 0, 0 );

      if ( vertCount > 3 )
         m->addVertex( 2, 0, 0 );

      if ( vertCount == 5 )
      {
         m->addVertex( 1, 0, 0 );
      }
      else if ( vertCount == 6 )
      {
         m->addVertex( 1, 1, 0 );
         m->addVertex( 1, 0, 0 );
      }

      if ( vertCount == 3 )
      {
         m->addTriangle( 0, 1, 2 );
      }
      else if ( vertCount == 4 )
      {
         m->addTriangle( 0, 1, 2 );
         m->addTriangle( 1, 3, 2 );
      }
      else if ( vertCount == 5 )
      {
         m->addTriangle( 0, 1, 2 );
         m->addTriangle( 1, 3, 4 );
      }
      else
      {
         m->addTriangle( 0, 1, 2 );
         m->addTriangle( 4, 3, 5 );
      }
   }

   void selectAllVertices( Model * m )
   {
      size_t vcount = m->getVertexCount();
      for ( size_t v = 0; v < vcount; ++v )
      {
         m->selectVertex( v );
      }
   }

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
      log_enable_warning( true );
   }

   void testWeldAllSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_fivevert = newTestModel();
      local_ptr<Model> rhs_fourvert = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_fivevert.get() );
      rhs_list.push_back( rhs_fourvert.get() );

      addWeldTriangles( lhs.get(), 5 );
      addWeldTriangles( rhs_fivevert.get(), 5 );
      addWeldTriangles( rhs_fourvert.get(), 4 );
      selectAllVertices( lhs.get() ); 
      selectAllVertices( rhs_fivevert.get() ); 
      selectAllVertices( rhs_fourvert.get() ); 

      lhs->selectTriangle( 0 );
      rhs_fivevert->selectTriangle( 0 );
      rhs_fourvert->selectTriangle( 0 );

      lhs->setTextureCoords( 0, 2, 0.3f, 0.4f );
      lhs->setTextureCoords( 1, 2, 0.5f, 0.6f );
      rhs_fivevert->setTextureCoords( 0, 2, 0.3f, 0.4f );
      rhs_fivevert->setTextureCoords( 1, 2, 0.5f, 0.6f );
      rhs_fourvert->setTextureCoords( 0, 2, 0.3f, 0.4f );
      rhs_fourvert->setTextureCoords( 1, 2, 0.5f, 0.6f );

      QVERIFY_TRUE( lhs->propEqual( rhs_fivevert.get() ) );
      QVERIFY_FALSE( lhs->propEqual( rhs_fourvert.get() ) );

      lhs->operationComplete( "Add triangles" );

      int before = 0;
      int after = 0;
      weldSelectedVertices( lhs.get(), 0.0001, before, after );

      QVERIFY_EQ( 2, before );
      QVERIFY_EQ( 1, after );

      QVERIFY_FALSE( lhs->propEqual( rhs_fivevert.get() ) );
      QVERIFY_TRUE( lhs->propEqual( rhs_fourvert.get() ) );

      lhs->operationComplete( "Weld selected" );

      // Should be equivalent to both
      QVERIFY_TRUE( lhs->equivalent( rhs_fivevert.get() ) );
      QVERIFY_TRUE( lhs->equivalent( rhs_fourvert.get() ) );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   // Nothing selected, nothing to weld
   void testWeldNoneSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_fivevert = newTestModel();

      addWeldTriangles( lhs.get(), 5 );
      addWeldTriangles( rhs_fivevert.get(), 5 );

      QVERIFY_TRUE( lhs->propEqual( rhs_fivevert.get() ) );

      weldSelectedVertices( lhs.get() );

      QVERIFY_TRUE( lhs->propEqual( rhs_fivevert.get() ) );
   }

   void testWeldTolerance()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_fivevert = newTestModel();
      local_ptr<Model> rhs_fourvert = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_fivevert.get() );
      rhs_list.push_back( rhs_fourvert.get() );

      addWeldTriangles( lhs.get(), 5 );
      addWeldTriangles( rhs_fivevert.get(), 5 );
      addWeldTriangles( rhs_fourvert.get(), 4 );
      selectAllVertices( lhs.get() ); 
      selectAllVertices( rhs_fivevert.get() ); 
      selectAllVertices( rhs_fourvert.get() ); 

      lhs->moveVertex( 4, 1.1, 0, 0 );
      rhs_fivevert->moveVertex( 4, 1.1, 0, 0 );

      QVERIFY_TRUE( lhs->propEqual( rhs_fivevert.get() ) );
      QVERIFY_FALSE( lhs->propEqual( rhs_fourvert.get() ) );

      lhs->operationComplete( "Add triangles" );

      // Not close enough to weld
      weldSelectedVertices( lhs.get(), 0.09 );

      QVERIFY_TRUE( lhs->propEqual( rhs_fivevert.get() ) );
      QVERIFY_FALSE( lhs->propEqual( rhs_fourvert.get() ) );

      // Close enough to weld
      weldSelectedVertices( lhs.get(), 0.11 );

      QVERIFY_FALSE( lhs->propEqual( rhs_fivevert.get() ) );
      QVERIFY_TRUE( lhs->propEqual( rhs_fourvert.get() ) );

      lhs->operationComplete( "Weld selected" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   // Tests that triangles that get two vertices merged into one
   // are deleted.
   void testWeldDeleteFlattened()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_fivevert = newTestModel();
      local_ptr<Model> rhs_threevert = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_fivevert.get() );
      rhs_list.push_back( rhs_threevert.get() );

      addWeldTriangles( lhs.get(), 5 );
      addWeldTriangles( rhs_fivevert.get(), 5 );
      addWeldTriangles( rhs_threevert.get(), 3 );
      selectAllVertices( lhs.get() ); 
      selectAllVertices( rhs_fivevert.get() ); 
      selectAllVertices( rhs_threevert.get() ); 

      lhs->moveVertex( 3, 1, 0, 0 );
      rhs_fivevert->moveVertex( 3, 1, 0, 0 );

      QVERIFY_TRUE( lhs->propEqual( rhs_fivevert.get() ) );
      QVERIFY_FALSE( lhs->propEqual( rhs_threevert.get() ) );

      lhs->operationComplete( "Add triangles" );

      int before = 0;
      int after = 0;
      weldSelectedVertices( lhs.get(), 0.0001, before, after );

      QVERIFY_EQ( 3, before );
      QVERIFY_EQ( 1, after );

      QVERIFY_FALSE( lhs->propEqual( rhs_fivevert.get() ) );
      QVERIFY_TRUE( lhs->propEqual( rhs_threevert.get() ) );

      lhs->operationComplete( "Weld selected" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testUnweldAllSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_fourvert = newTestModel();
      local_ptr<Model> rhs_sixvert = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_fourvert.get() );
      rhs_list.push_back( rhs_sixvert.get() );

      addWeldTriangles( lhs.get(), 4 );
      addWeldTriangles( rhs_fourvert.get(), 4 );
      addWeldTriangles( rhs_sixvert.get(), 6 );
      selectAllVertices( lhs.get() ); 
      selectAllVertices( rhs_sixvert.get() ); 
      selectAllVertices( rhs_fourvert.get() ); 

      lhs->selectTriangle( 0 );
      rhs_sixvert->selectTriangle( 0 );
      rhs_fourvert->selectTriangle( 0 );

      lhs->setTextureCoords( 0, 2, 0.3f, 0.4f );
      lhs->setTextureCoords( 1, 2, 0.5f, 0.6f );
      rhs_sixvert->setTextureCoords( 0, 2, 0.3f, 0.4f );
      rhs_sixvert->setTextureCoords( 1, 2, 0.5f, 0.6f );
      rhs_fourvert->setTextureCoords( 0, 2, 0.3f, 0.4f );
      rhs_fourvert->setTextureCoords( 1, 2, 0.5f, 0.6f );

      lhs->addBoneJoint( "Parent", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Child",  1, 1, 1, 0, 0, 0, 0 );
      rhs_sixvert->addBoneJoint( "Parent", 1, 1, 1, 0, 0, 0 );
      rhs_sixvert->addBoneJoint( "Child",  1, 1, 1, 0, 0, 0, 0 );
      rhs_fourvert->addBoneJoint( "Parent", 1, 1, 1, 0, 0, 0 );
      rhs_fourvert->addBoneJoint( "Child",  1, 1, 1, 0, 0, 0, 0 );

      lhs->addVertexInfluence( 2, 0, Model::IT_Custom, 0.7 );
      lhs->addVertexInfluence( 2, 0, Model::IT_Custom, 0.3 );
      rhs_sixvert->addVertexInfluence( 2, 0, Model::IT_Custom, 0.7 );
      rhs_sixvert->addVertexInfluence( 2, 0, Model::IT_Custom, 0.3 );
      rhs_sixvert->addVertexInfluence( 5, 0, Model::IT_Custom, 0.7 );
      rhs_sixvert->addVertexInfluence( 5, 0, Model::IT_Custom, 0.3 );
      rhs_fourvert->addVertexInfluence( 2, 0, Model::IT_Custom, 0.7 );
      rhs_fourvert->addVertexInfluence( 2, 0, Model::IT_Custom, 0.3 );

      QVERIFY_TRUE( lhs->propEqual( rhs_fourvert.get() ) );
      QVERIFY_FALSE( lhs->propEqual( rhs_sixvert.get() ) );

      lhs->operationComplete( "Add triangles" );

      int before = 0;
      int after = 0;
      unweldSelectedVertices( lhs.get(), after, before );

      QVERIFY_EQ( 4, before );
      QVERIFY_EQ( 6, after );

      QVERIFY_FALSE( lhs->propEqual( rhs_fourvert.get() ) );
      QVERIFY_TRUE( lhs->propEqual( rhs_sixvert.get() ) );

      lhs->operationComplete( "Unweld selected" );

      // Should be equivalent to both
      QVERIFY_TRUE( lhs->equivalent( rhs_sixvert.get() ) );
      QVERIFY_TRUE( lhs->equivalent( rhs_fourvert.get() ) );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   // Nothing selected, nothing to unweld
   void testUnweldNoneSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_fourvert = newTestModel();

      addWeldTriangles( lhs.get(), 4 );
      addWeldTriangles( rhs_fourvert.get(), 4 );

      QVERIFY_TRUE( lhs->propEqual( rhs_fourvert.get() ) );

      unweldSelectedVertices( lhs.get() );

      QVERIFY_TRUE( lhs->propEqual( rhs_fourvert.get() ) );
   }

   // FIXME test
   //   X Weld
   //   X Unweld
   //   X Vertex count is right
   //   X welded/unwelded count is right
   //   X Model equivalanece
   //   X Undo/redo
   //   X Unweld preserves influences
   //   X Tolerance respected
   //   X UV coords unchanged
   //   X Selected/unselected respected
   //   X Deletes flattened

};

QTEST_MAIN(ModelWeldTest)
#include "model_weld_test.moc"

