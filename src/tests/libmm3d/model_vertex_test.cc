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
#include "log.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"

#include <vector>

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
      lhs->operationComplete( "Unselect all" );

      checkUndoRedo( 5, lhs.get(), rhs_list );
   }

   void testGetSelectedList()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 0, 1, 1 );
      lhs->addVertex( 0, 0, 1 );

      std::list<int> verts;
      std::list<int>::const_iterator it;

      lhs->getSelectedVertices( verts );
      QVERIFY(verts.begin() == verts.end());

      QVERIFY_EQ( 0, (int) lhs->getSelectedVertexCount() );
      lhs->selectVertex( 0 );
      QVERIFY_EQ( 1, (int) lhs->getSelectedVertexCount() );
      lhs->selectVertex( 5 );
      QVERIFY_EQ( 2, (int) lhs->getSelectedVertexCount() );
      lhs->selectVertex( 3 );
      QVERIFY_EQ( 3, (int) lhs->getSelectedVertexCount() );
      lhs->selectVertex( 2 );
      QVERIFY_EQ( 4, (int) lhs->getSelectedVertexCount() );

      lhs->getSelectedVertices( verts );
      QVERIFY(verts.begin() != verts.end());

      it = verts.begin();
      QVERIFY(it != verts.end());
      QVERIFY_EQ(0, *it );
      ++it;
      QVERIFY(it != verts.end());
      QVERIFY_EQ(2, *it );
      ++it;
      QVERIFY(it != verts.end());
      QVERIFY_EQ(3, *it );
      ++it;
      QVERIFY(it != verts.end());
      QVERIFY_EQ(5, *it );
      ++it;
      QVERIFY(it == verts.end());
   }

   void testMoveVertex()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_orig = newTestModel();
      local_ptr<Model> rhs_moved = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_orig.get() );
      rhs_list.push_back( rhs_moved.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      rhs_orig->addVertex( 0, 0, 0 );
      rhs_orig->addVertex( 1, 0, 0 );
      rhs_orig->addVertex( 1, 1, 0 );
      rhs_moved->addVertex( 3, 4, 5 );
      rhs_moved->addVertex( 2, 1, 0 );
      rhs_moved->addVertex( -3, -4, -5 );

      double expected[3] = { 0, 0, 0 };
      double actual[3] = { 0, 0, 0 };

      expected[0] = 0; expected[1] = 0; expected[2] = 0;
      lhs->getVertexCoords( 0, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      expected[0] = 1; expected[1] = 0; expected[2] = 0;
      lhs->getVertexCoords( 1, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      expected[0] = 1; expected[1] = 1; expected[2] = 0;
      lhs->getVertexCoords( 2, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );

      lhs->operationComplete( "Add vertices" );
      lhs->moveVertex( 0, 3, 4, 5 );
      lhs->moveVertex( 1, 2, 1, 0 );
      lhs->moveVertex( 2, -3, -4, -5 );

      expected[0] = 3; expected[1] = 4; expected[2] = 5;
      lhs->getVertexCoords( 0, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      expected[0] = 2; expected[1] = 1; expected[2] = 0;
      lhs->getVertexCoords( 1, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      expected[0] = -3; expected[1] = -4; expected[2] = -5;
      lhs->getVertexCoords( 2, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );

      lhs->operationComplete( "Move vertices" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
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

   void testDeleteVertexDeletesTriangle()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_triangle = newTestModel();
      local_ptr<Model> rhs_selected = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_triangle.get() );
      rhs_list.push_back( rhs_selected.get() );
      rhs_list.push_back( rhs_empty.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );
      rhs_triangle->addVertex( 0, 0, 0 );
      rhs_triangle->addVertex( 1, 0, 0 );
      rhs_triangle->addVertex( 1, 1, 0 );
      rhs_triangle->addTriangle( 0, 1, 2 );
      rhs_selected->addVertex( 0, 0, 0 );
      rhs_selected->addVertex( 1, 0, 0 );
      rhs_selected->addVertex( 1, 1, 0 );
      rhs_selected->addTriangle( 0, 1, 2 );
      rhs_selected->selectVertex( 1 );

      lhs->operationComplete( "Add vertices" );

      // Deleting one vertex will delete one triangle, but not the other.
      // Other vertices remain undeleted.
      lhs->selectVertex( 1 );
      lhs->operationComplete( "Select Vertex" );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete Selected" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
   }

   void testDeleteVertexDeletesSelf()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_triangle = newTestModel();
      local_ptr<Model> rhs_selected = newTestModel();
      local_ptr<Model> rhs_deleted = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_triangle.get() );
      rhs_list.push_back( rhs_selected.get() );
      rhs_list.push_back( rhs_deleted.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 1, 1 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 0, 2, 3 );
      rhs_triangle->addVertex( 0, 0, 0 );
      rhs_triangle->addVertex( 1, 0, 0 );
      rhs_triangle->addVertex( 1, 1, 0 );
      rhs_triangle->addVertex( 0, 1, 1 );
      rhs_triangle->addTriangle( 0, 1, 2 );
      rhs_triangle->addTriangle( 0, 2, 3 );
      rhs_selected->addVertex( 0, 0, 0 );
      rhs_selected->addVertex( 1, 0, 0 );
      rhs_selected->addVertex( 1, 1, 0 );
      rhs_selected->addVertex( 0, 1, 1 );
      rhs_selected->addTriangle( 0, 1, 2 );
      rhs_selected->addTriangle( 0, 2, 3 );
      rhs_selected->selectVertex( 1 );
      rhs_deleted->addVertex( 0, 0, 0 );
      rhs_deleted->addVertex( 1, 1, 0 );
      rhs_deleted->addVertex( 0, 1, 1 );
      rhs_deleted->addTriangle( 0, 1, 2 );

      lhs->operationComplete( "Add vertices" );

      // Deleting one vertex will delete one triangle, but not the other.
      // Other vertices remain undeleted.
      lhs->selectVertex( 1 );
      lhs->operationComplete( "Select Vertex" );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete Selected" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
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

   // FIXME Tests to add (in other files):
   //
   // add/remove influences
   // Frame anim vertex tests
   // Vertex animated with (weighted) bone joints
   // Vertex indicies updated in triangles when vertices deleted

};

QTEST_MAIN(ModelVertexTest)
#include "model_vertex_test.moc"

