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


// This file tests vertex methods in the Model class.

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

#include <vector>

void model_status( Model * model, StatusTypeE type, unsigned ms, const char * fmt, ... )
{
   // FIXME hack
}

Model * loadModelOrDie( const char * filename )
{
   MisfitFilter f;

   Model * model = new Model;
   Model::ModelErrorE err = f.readFile( model, filename );

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "fatal: %s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      exit( -1 );
   }

   model->setUndoEnabled( true );
   model->forceAddOrDelete( true );
   return model;
}

Model * newTestModel()
{
   Model * model = new Model;
   model->setUndoEnabled( true );
   model->forceAddOrDelete( true );
   return model;
}

typedef std::vector<Model *> ModelList;

void checkUndoRedo( int operations, Model * lhs, const ModelList & rhs_list )
{
   // N operations, N+1 models in the list to compare against
   QVERIFY_EQ( (int) rhs_list.size(), operations + 1 );

   int bits = Model::CompareAll;
   QVERIFY_EQ( bits, lhs->equal( rhs_list.back(), bits ) );

   for ( int iter = 0; iter < 2; ++iter )
   {
      for ( int i = operations - 1; i >= 0; --i )
      {
         //printf( "undo operation %d\n", i );
         lhs->undo();
         QVERIFY_EQ( bits, lhs->equal( rhs_list[i], bits ) );
      }

      for ( int i = 1; i <= operations; ++i )
      {
         //printf( "redo operation %d\n", i );
         lhs->redo();
         QVERIFY_EQ( bits, lhs->equal( rhs_list[i], bits ) );
      }
   }
}

class ModelVertexTest : public QObject
{
   Q_OBJECT
private:

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   void testSelectVertex()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_unselected = newTestModel();
      local_ptr<Model> rhs_selected = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_unselected.get() );
      rhs_list.push_back( rhs_selected.get() );
      rhs_list.push_back( rhs_unselected.get() );
      rhs_list.push_back( rhs_selected.get() );
      rhs_list.push_back( rhs_unselected.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      rhs_unselected->addVertex( 0, 0, 0 );
      rhs_unselected->addVertex( 1, 0, 0 );
      rhs_unselected->addVertex( 1, 1, 0 );
      rhs_selected->addVertex( 0, 0, 0 );
      rhs_selected->addVertex( 1, 0, 0 );
      rhs_selected->addVertex( 1, 1, 0 );
      rhs_selected->selectVertex( 1 );

      lhs->operationComplete( "Add vertices" );
      lhs->selectVertex( 1 );
      QVERIFY_FALSE( lhs->isVertexSelected( 0 ) );
      QVERIFY_TRUE( lhs->isVertexSelected( 1 ) );
      QVERIFY_FALSE( lhs->isVertexSelected( 2 ) );
      lhs->operationComplete( "Select vertex" );
      lhs->unselectVertex( 1 );
      QVERIFY_FALSE( lhs->isVertexSelected( 0 ) );
      QVERIFY_FALSE( lhs->isVertexSelected( 1 ) );
      QVERIFY_FALSE( lhs->isVertexSelected( 2 ) );
      lhs->operationComplete( "Unselect vertex" );
      lhs->selectVertex( 1 );
      QVERIFY_FALSE( lhs->isVertexSelected( 0 ) );
      QVERIFY_TRUE( lhs->isVertexSelected( 1 ) );
      QVERIFY_FALSE( lhs->isVertexSelected( 2 ) );
      lhs->operationComplete( "Select vertex" );
      lhs->unselectAll();
      QVERIFY_FALSE( lhs->isVertexSelected( 0 ) );
      QVERIFY_FALSE( lhs->isVertexSelected( 1 ) );
      QVERIFY_FALSE( lhs->isVertexSelected( 2 ) );
      lhs->operationComplete( "Unselect vertex" );

      checkUndoRedo( 5, lhs.get(), rhs_list );
   }

   void testSetFree()
   {
      // setVertexFree is not an undoable operation
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );

      QVERIFY_FALSE( lhs->isVertexFree( 0 ) );
      QVERIFY_FALSE( lhs->isVertexFree( 1 ) );
      QVERIFY_FALSE( lhs->isVertexFree( 2 ) );

      lhs->setVertexFree( 1, true );

      QVERIFY_FALSE( lhs->isVertexFree( 0 ) );
      QVERIFY_TRUE( lhs->isVertexFree( 1 ) );
      QVERIFY_FALSE( lhs->isVertexFree( 2 ) );

      lhs->setVertexFree( 1, false );

      QVERIFY_FALSE( lhs->isVertexFree( 0 ) );
      QVERIFY_FALSE( lhs->isVertexFree( 1 ) );
      QVERIFY_FALSE( lhs->isVertexFree( 2 ) );
   }

   void testDeleteSelectedNoFreeNoneSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_full = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_full.get() );
      rhs_list.push_back( rhs_empty.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      rhs_full->addVertex( 0, 0, 0 );
      rhs_full->addVertex( 1, 0, 0 );
      rhs_full->addVertex( 1, 1, 0 );

      lhs->operationComplete( "Add vertices" );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete selected" );

      // No free vertices, they should be deleted even though they weren't
      // selected.

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testDeleteSelectedNoFreeAllSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_full = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_full.get() );
      rhs_list.push_back( rhs_empty.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      rhs_full->addVertex( 0, 0, 0 );
      rhs_full->addVertex( 1, 0, 0 );
      rhs_full->addVertex( 1, 1, 0 );

      lhs->operationComplete( "Add vertices" );
      lhs->selectVertex( 0 );
      lhs->selectVertex( 1 );
      lhs->selectVertex( 2 );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete selected" );

      // No free vertices, they should be deleted even though they weren't
      // selected.

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testDeleteSelectedFreeNoneSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_full = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_full.get() );
      // No change on the second operation, so there is nothing to undo

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->setVertexFree( 0, true );
      lhs->setVertexFree( 1, true );
      lhs->setVertexFree( 2, true );
      rhs_full->addVertex( 0, 0, 0 );
      rhs_full->addVertex( 1, 0, 0 );
      rhs_full->addVertex( 1, 1, 0 );
      rhs_full->setVertexFree( 0, true );
      rhs_full->setVertexFree( 1, true );
      rhs_full->setVertexFree( 2, true );

      lhs->operationComplete( "Add vertices" );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete selected" );

      // No free vertices, they should be deleted even though they weren't
      // selected.

      checkUndoRedo( 1, lhs.get(), rhs_list );
   }

   void testDeleteSelectedFreeAllSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_full = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_full.get() );
      rhs_list.push_back( rhs_empty.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->setVertexFree( 0, true );
      lhs->setVertexFree( 1, true );
      lhs->setVertexFree( 2, true );
      rhs_full->addVertex( 0, 0, 0 );
      rhs_full->addVertex( 1, 0, 0 );
      rhs_full->addVertex( 1, 1, 0 );
      rhs_full->setVertexFree( 0, true );
      rhs_full->setVertexFree( 1, true );
      rhs_full->setVertexFree( 2, true );

      lhs->operationComplete( "Add vertices" );
      lhs->selectVertex( 0 );
      lhs->selectVertex( 1 );
      lhs->selectVertex( 2 );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete selected" );

      // No free vertices, they should be deleted even though they weren't
      // selected.

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testHideVertexHidesTriangle()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_visible = newTestModel();
      local_ptr<Model> rhs_hidden = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_visible.get() );
      rhs_list.push_back( rhs_hidden.get() );
      rhs_list.push_back( rhs_visible.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );
      rhs_visible->addVertex( 0, 0, 0 );
      rhs_visible->addVertex( 1, 0, 0 );
      rhs_visible->addVertex( 1, 1, 0 );
      rhs_visible->addTriangle( 0, 1, 2 );
      rhs_hidden->addVertex( 0, 0, 0 );
      rhs_hidden->addVertex( 1, 0, 0 );
      rhs_hidden->addVertex( 1, 1, 0 );
      rhs_hidden->addTriangle( 0, 1, 2 );

      lhs->operationComplete( "Add vertices" );

      // Hiding one vertex will hide the triangle, which hides the
      // other vertices too.
      lhs->selectVertex( 0 );
      lhs->hideSelected();
      rhs_hidden->hideVertex( 0 );
      rhs_hidden->hideVertex( 1 );
      rhs_hidden->hideVertex( 2 );
      rhs_hidden->hideTriangle( 0 );
      QVERIFY_FALSE( lhs->isVertexVisible(0) );
      QVERIFY_FALSE( lhs->isVertexVisible(1) );
      QVERIFY_FALSE( lhs->isVertexVisible(2) );
      QVERIFY_FALSE( lhs->isTriangleVisible(0) );
      lhs->operationComplete( "Hide Vertices" );
      lhs->unhideAll();
      QVERIFY_TRUE( lhs->isVertexVisible(0) );
      QVERIFY_TRUE( lhs->isVertexVisible(1) );
      QVERIFY_TRUE( lhs->isVertexVisible(2) );
      QVERIFY_TRUE( lhs->isTriangleVisible(0) );
      lhs->operationComplete( "Unhide All" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
   }

   void testHideVertexHidesSelf()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_visible = newTestModel();
      local_ptr<Model> rhs_hidden = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_visible.get() );
      rhs_list.push_back( rhs_hidden.get() );
      rhs_list.push_back( rhs_visible.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 1, 1 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 0, 2, 3 );
      rhs_visible->addVertex( 0, 0, 0 );
      rhs_visible->addVertex( 1, 0, 0 );
      rhs_visible->addVertex( 1, 1, 0 );
      rhs_visible->addVertex( 0, 1, 1 );
      rhs_visible->addTriangle( 0, 1, 2 );
      rhs_visible->addTriangle( 0, 2, 3 );
      rhs_hidden->addVertex( 0, 0, 0 );
      rhs_hidden->addVertex( 1, 0, 0 );
      rhs_hidden->addVertex( 1, 1, 0 );
      rhs_hidden->addVertex( 0, 1, 1 );
      rhs_hidden->addTriangle( 0, 1, 2 );
      rhs_hidden->addTriangle( 0, 2, 3 );

      lhs->operationComplete( "Add vertices" );

      // Hiding one vertex will hide one triangle, but not the other.
      // Other vertices remain visible.
      lhs->selectVertex( 1 );
      lhs->hideSelected();
      rhs_hidden->hideVertex( 1 );
      rhs_hidden->hideTriangle( 0 );
      QVERIFY_TRUE( lhs->isVertexVisible(0) );
      QVERIFY_FALSE( lhs->isVertexVisible(1) );
      QVERIFY_TRUE( lhs->isVertexVisible(2) );
      QVERIFY_TRUE( lhs->isVertexVisible(3) );
      QVERIFY_FALSE( lhs->isTriangleVisible(0) );
      QVERIFY_TRUE( lhs->isTriangleVisible(1) );
      lhs->operationComplete( "Hide Vertex" );
      lhs->unhideAll();
      QVERIFY_TRUE( lhs->isVertexVisible(0) );
      QVERIFY_TRUE( lhs->isVertexVisible(1) );
      QVERIFY_TRUE( lhs->isVertexVisible(2) );
      QVERIFY_TRUE( lhs->isVertexVisible(3) );
      QVERIFY_TRUE( lhs->isTriangleVisible(0) );
      QVERIFY_TRUE( lhs->isTriangleVisible(1) );
      lhs->operationComplete( "Unhide All" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
   }

   // FIXME add tests
   // FIXME undo
   // move coords
   //
   // Triangle tests:
   //   Deleting triangle deletes vertex
   //   Deleting triangle does not delete free vertex
   //   Deleting triangle does not delete shared vertex
   //   Set triangle vertex
   // 
   // add/remove influences
   // Frame anim vertex tests
   // Vertex animated with (weighted) bone joints
   // Vertex indicies updated in triangles when vertices deleted

};

QTEST_MAIN(ModelVertexTest)
#include "model_vertex_test.moc"

