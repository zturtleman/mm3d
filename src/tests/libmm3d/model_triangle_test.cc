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


// This file tests triangle methods in the Model class.

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "log.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"

#include <vector>

class ModelTriangleTest : public QObject
{
   Q_OBJECT
private:

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   void testSelectTriangle()
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
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 2, 1, 0 );
      rhs_unselected->addVertex( 0, 0, 0 );
      rhs_unselected->addVertex( 1, 0, 0 );
      rhs_unselected->addVertex( 1, 1, 0 );
      rhs_unselected->addTriangle( 0, 1, 2 );
      rhs_unselected->addTriangle( 2, 1, 0 );
      rhs_selected->addVertex( 0, 0, 0 );
      rhs_selected->addVertex( 1, 0, 0 );
      rhs_selected->addVertex( 1, 1, 0 );
      rhs_selected->addTriangle( 0, 1, 2 );
      rhs_selected->addTriangle( 2, 1, 0 );
      rhs_selected->selectTriangle( 1 );

      lhs->operationComplete( "Add triangles" );
      lhs->selectTriangle( 1 );
      QVERIFY_FALSE( lhs->isTriangleSelected( 0 ) );
      QVERIFY_TRUE( lhs->isTriangleSelected( 1 ) );
      lhs->operationComplete( "Select triangle" );
      lhs->unselectTriangle( 1 );
      QVERIFY_FALSE( lhs->isTriangleSelected( 0 ) );
      QVERIFY_FALSE( lhs->isTriangleSelected( 1 ) );
      lhs->operationComplete( "Unselect triangle" );
      lhs->selectTriangle( 1 );
      QVERIFY_FALSE( lhs->isTriangleSelected( 0 ) );
      QVERIFY_TRUE( lhs->isTriangleSelected( 1 ) );
      lhs->operationComplete( "Select triangle" );
      lhs->unselectAll();
      QVERIFY_FALSE( lhs->isTriangleSelected( 0 ) );
      QVERIFY_FALSE( lhs->isTriangleSelected( 1 ) );
      lhs->operationComplete( "Unselect all" );

      checkUndoRedo( 5, lhs.get(), rhs_list );
   }

   void testGetSelectedList()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 1, 2, 0 );
      lhs->addTriangle( 2, 0, 1 );
      lhs->addTriangle( 2, 1, 0 );
      lhs->addTriangle( 1, 0, 2 );
      lhs->addTriangle( 0, 2, 1 );

      list<int> tris;
      list<int>::const_iterator it;

      lhs->getSelectedTriangles( tris );
      QVERIFY(tris.begin() == tris.end());

      QVERIFY_EQ( 0, (int) lhs->getSelectedTriangleCount() );
      lhs->selectTriangle( 0 );
      QVERIFY_EQ( 1, (int) lhs->getSelectedTriangleCount() );
      lhs->selectTriangle( 5 );
      QVERIFY_EQ( 2, (int) lhs->getSelectedTriangleCount() );
      lhs->selectTriangle( 3 );
      QVERIFY_EQ( 3, (int) lhs->getSelectedTriangleCount() );
      lhs->selectTriangle( 1 );
      QVERIFY_EQ( 4, (int) lhs->getSelectedTriangleCount() );

      lhs->getSelectedTriangles( tris );
      QVERIFY(tris.begin() != tris.end());

      it = tris.begin();
      QVERIFY(it != tris.end());
      QVERIFY_EQ(0, *it );
      ++it;
      QVERIFY(it != tris.end());
      QVERIFY_EQ(1, *it );
      ++it;
      QVERIFY(it != tris.end());
      QVERIFY_EQ(3, *it );
      ++it;
      QVERIFY(it != tris.end());
      QVERIFY_EQ(5, *it );
      ++it;
      QVERIFY(it == tris.end());
   }

   // Tests that a selected triangle is deleted, but shared vertices
   // are not.
   void testDeleteSelected()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_unselected = newTestModel();
      local_ptr<Model> rhs_selected = newTestModel();
      local_ptr<Model> rhs_deleted = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_unselected.get() );
      rhs_list.push_back( rhs_selected.get() );
      rhs_list.push_back( rhs_deleted.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 2, 0, 0 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 3, 2, 1 );
      rhs_unselected->addVertex( 0, 0, 0 );
      rhs_unselected->addVertex( 1, 0, 0 );
      rhs_unselected->addVertex( 1, 1, 0 );
      rhs_unselected->addVertex( 2, 0, 0 );
      rhs_unselected->addTriangle( 0, 1, 2 );
      rhs_unselected->addTriangle( 3, 2, 1 );
      rhs_selected->addVertex( 0, 0, 0 );
      rhs_selected->addVertex( 1, 0, 0 );
      rhs_selected->addVertex( 1, 1, 0 );
      rhs_selected->addVertex( 2, 0, 0 );
      rhs_selected->addTriangle( 0, 1, 2 );
      rhs_selected->addTriangle( 3, 2, 1 );
      rhs_selected->selectTriangle( 0 );
      rhs_deleted->addVertex( 1, 0, 0 );
      rhs_deleted->addVertex( 1, 1, 0 );
      rhs_deleted->addVertex( 2, 0, 0 );
      rhs_deleted->addTriangle( 2, 1, 0 );

      lhs->operationComplete( "Add triangles" );
      lhs->selectTriangle( 0 );
      lhs->operationComplete( "Select triangle" );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete selected" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
   }

   // Tests that vertices are deleted when no triangles are using them.
   void testDeleteSelectedNoFree()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_unselected = newTestModel();
      local_ptr<Model> rhs_selected = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_unselected.get() );
      rhs_list.push_back( rhs_selected.get() );
      rhs_list.push_back( rhs_empty.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );
      rhs_unselected->addVertex( 0, 0, 0 );
      rhs_unselected->addVertex( 1, 0, 0 );
      rhs_unselected->addVertex( 1, 1, 0 );
      rhs_unselected->addTriangle( 0, 1, 2 );
      rhs_selected->addVertex( 0, 0, 0 );
      rhs_selected->addVertex( 1, 0, 0 );
      rhs_selected->addVertex( 1, 1, 0 );
      rhs_selected->addTriangle( 0, 1, 2 );
      rhs_selected->selectTriangle( 0 );

      lhs->operationComplete( "Add triangles" );
      lhs->selectTriangle( 0 );
      lhs->operationComplete( "Select triangle" );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete selected" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
   }

   // Tests that vertices are not deleted if they are marked free
   void testDeleteSelectedFree()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_unselected = newTestModel();
      local_ptr<Model> rhs_selected = newTestModel();
      local_ptr<Model> rhs_deleted = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_unselected.get() );
      rhs_list.push_back( rhs_selected.get() );
      rhs_list.push_back( rhs_deleted.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->setVertexFree( 1, true );
      lhs->addTriangle( 0, 1, 2 );
      rhs_unselected->addVertex( 0, 0, 0 );
      rhs_unselected->addVertex( 1, 0, 0 );
      rhs_unselected->addVertex( 1, 1, 0 );
      rhs_unselected->setVertexFree( 1, true );
      rhs_unselected->addTriangle( 0, 1, 2 );
      rhs_selected->addVertex( 0, 0, 0 );
      rhs_selected->addVertex( 1, 0, 0 );
      rhs_selected->addVertex( 1, 1, 0 );
      rhs_selected->setVertexFree( 1, true );
      rhs_selected->addTriangle( 0, 1, 2 );
      rhs_selected->selectTriangle( 0 );
      rhs_deleted->addVertex( 1, 0, 0 );
      rhs_deleted->setVertexFree( 0, true );

      lhs->operationComplete( "Add triangles" );
      lhs->selectTriangle( 0 );
      lhs->operationComplete( "Select triangle" );
      lhs->deleteSelected();
      lhs->operationComplete( "Delete selected" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
   }

   void testHideTriangle()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_unselected = newTestModel();
      local_ptr<Model> rhs_selected = newTestModel();
      local_ptr<Model> rhs_hidden = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_unselected.get() );
      rhs_list.push_back( rhs_selected.get() );
      rhs_list.push_back( rhs_hidden.get() );
      rhs_list.push_back( rhs_unselected.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 2, 0, 0 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 1, 2, 3 );
      rhs_unselected->addVertex( 0, 0, 0 );
      rhs_unselected->addVertex( 1, 0, 0 );
      rhs_unselected->addVertex( 1, 1, 0 );
      rhs_unselected->addVertex( 2, 0, 0 );
      rhs_unselected->addTriangle( 0, 1, 2 );
      rhs_unselected->addTriangle( 1, 2, 3 );
      rhs_selected->addVertex( 0, 0, 0 );
      rhs_selected->addVertex( 1, 0, 0 );
      rhs_selected->addVertex( 1, 1, 0 );
      rhs_selected->addVertex( 2, 0, 0 );
      rhs_selected->addTriangle( 0, 1, 2 );
      rhs_selected->addTriangle( 1, 2, 3 );
      rhs_selected->selectTriangle( 1 );
      rhs_hidden->addVertex( 0, 0, 0 );
      rhs_hidden->addVertex( 1, 0, 0 );
      rhs_hidden->addVertex( 1, 1, 0 );
      rhs_hidden->addVertex( 2, 0, 0 );
      rhs_hidden->addTriangle( 0, 1, 2 );
      rhs_hidden->addTriangle( 1, 2, 3 );
      rhs_hidden->hideTriangle( 1 );
      rhs_hidden->hideVertex( 3 );  // Have to do this directly

      lhs->operationComplete( "Add triangles" );
      lhs->selectTriangle( 1 );
      QVERIFY_TRUE( lhs->isTriangleVisible( 0 ) );
      QVERIFY_TRUE( lhs->isTriangleVisible( 1 ) );
      lhs->operationComplete( "Select triangle" );
      lhs->hideSelected();
      QVERIFY_TRUE( lhs->isTriangleVisible( 0 ) );
      QVERIFY_FALSE( lhs->isTriangleVisible( 1 ) );
      lhs->operationComplete( "Hide selected" );
      lhs->unhideAll();
      QVERIFY_TRUE( lhs->isTriangleVisible( 0 ) );
      QVERIFY_TRUE( lhs->isTriangleVisible( 1 ) );
      lhs->operationComplete( "Unhide all" );

      checkUndoRedo( 4, lhs.get(), rhs_list );
   }

   void testDeleteTriangle()
   {
      // deleteTriangle does not delete orphaned vertices (and these
      // aren't orphaned anyway)

      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_full = newTestModel();
      local_ptr<Model> rhs_deleted = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_full.get() );
      rhs_list.push_back( rhs_deleted.get() );

      QVERIFY_EQ( 0, (int) lhs->getTriangleCount() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );
      QVERIFY_EQ( 1, (int) lhs->getTriangleCount() );
      lhs->addTriangle( 2, 1, 0 );
      QVERIFY_EQ( 2, (int) lhs->getTriangleCount() );

      rhs_full->addVertex( 0, 0, 0 );
      rhs_full->addVertex( 1, 0, 0 );
      rhs_full->addVertex( 1, 1, 0 );
      rhs_full->addTriangle( 0, 1, 2 );
      rhs_full->addTriangle( 2, 1, 0 );
      rhs_deleted->addVertex( 0, 0, 0 );
      rhs_deleted->addVertex( 1, 0, 0 );
      rhs_deleted->addVertex( 1, 1, 0 );
      rhs_deleted->addTriangle( 2, 1, 0 );

      lhs->operationComplete( "Add triangles" );

      lhs->deleteTriangle( 0 );
      QVERIFY_EQ( 1, (int) lhs->getTriangleCount() );
      lhs->operationComplete( "Delete triangle" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   // Tests that a triangle with an edge that uses the same vertex for
   // both end points is deleted by deleteFlattenedTriangles()
   void testDeleteFlattened()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_full = newTestModel();
      local_ptr<Model> rhs_deleted = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_full.get() );
      rhs_list.push_back( rhs_deleted.get() );

      QVERIFY_EQ( 0, (int) lhs->getTriangleCount() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 1, 2, 2 );
      lhs->addTriangle( 0, 0, 2 );
      lhs->addTriangle( 1, 0, 1 );
      lhs->addTriangle( 1, 1, 1 );
      rhs_full->addVertex( 0, 0, 0 );
      rhs_full->addVertex( 1, 0, 0 );
      rhs_full->addVertex( 1, 1, 0 );
      rhs_full->addTriangle( 0, 1, 2 );
      rhs_full->addTriangle( 1, 2, 2 );
      rhs_full->addTriangle( 0, 0, 2 );
      rhs_full->addTriangle( 1, 0, 1 );
      rhs_full->addTriangle( 1, 1, 1 );
      rhs_deleted->addVertex( 0, 0, 0 );
      rhs_deleted->addVertex( 1, 0, 0 );
      rhs_deleted->addVertex( 1, 1, 0 );
      rhs_deleted->addTriangle( 0, 1, 2 );

      lhs->operationComplete( "Add triangles" );

      lhs->deleteFlattenedTriangles();
      lhs->operationComplete( "Delete flattened" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testSetVertices()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_front = newTestModel();
      local_ptr<Model> rhs_back = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_front.get() );
      rhs_list.push_back( rhs_back.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );
      rhs_front->addVertex( 0, 0, 0 );
      rhs_front->addVertex( 1, 0, 0 );
      rhs_front->addVertex( 1, 1, 0 );
      rhs_front->addTriangle( 0, 1, 2 );
      rhs_back->addVertex( 0, 0, 0 );
      rhs_back->addVertex( 1, 0, 0 );
      rhs_back->addVertex( 1, 1, 0 );
      rhs_back->addTriangle( 2, 1, 0 );

      lhs->operationComplete( "Add triangles" );

      unsigned int vert[3];

      QVERIFY_EQ( 0, lhs->getTriangleVertex( 0, 0 ) );
      QVERIFY_EQ( 1, lhs->getTriangleVertex( 0, 1 ) );
      QVERIFY_EQ( 2, lhs->getTriangleVertex( 0, 2 ) );

      lhs->getTriangleVertices( 0, vert[0], vert[1], vert[2] );
      QVERIFY_EQ( 0, (int) vert[0] );
      QVERIFY_EQ( 1, (int) vert[1] );
      QVERIFY_EQ( 2, (int) vert[2] );

      lhs->setTriangleVertices( 0, 2, 1, 0 );

      QVERIFY_EQ( 2, lhs->getTriangleVertex( 0, 0 ) );
      QVERIFY_EQ( 1, lhs->getTriangleVertex( 0, 1 ) );
      QVERIFY_EQ( 0, lhs->getTriangleVertex( 0, 2 ) );

      lhs->getTriangleVertices( 0, vert[0], vert[1], vert[2] );
      QVERIFY_EQ( 2, (int) vert[0] );
      QVERIFY_EQ( 1, (int) vert[1] );
      QVERIFY_EQ( 0, (int) vert[2] );

      lhs->operationComplete( "Set vertices" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testTriangleProjection()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_none = newTestModel();
      local_ptr<Model> rhs_first = newTestModel();
      local_ptr<Model> rhs_second = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_none.get() );
      rhs_list.push_back( rhs_first.get() );
      rhs_list.push_back( rhs_second.get() );
      rhs_list.push_back( rhs_none.get() );

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addProjection( "first", 0, 0, 0, Model::TPT_Plane );
      lhs->addProjection( "second", 1, 1, 1, Model::TPT_Plane );
      rhs_none->addVertex( 0, 0, 0 );
      rhs_none->addVertex( 1, 0, 0 );
      rhs_none->addVertex( 1, 1, 0 );
      rhs_none->addTriangle( 0, 1, 2 );
      rhs_none->addProjection( "first", 0, 0, 0, Model::TPT_Plane );
      rhs_none->addProjection( "second", 1, 1, 1, Model::TPT_Plane );
      rhs_first->addVertex( 0, 0, 0 );
      rhs_first->addVertex( 1, 0, 0 );
      rhs_first->addVertex( 1, 1, 0 );
      rhs_first->addTriangle( 0, 1, 2 );
      rhs_first->addProjection( "first", 0, 0, 0, Model::TPT_Plane );
      rhs_first->addProjection( "second", 1, 1, 1, Model::TPT_Plane );
      rhs_first->setTriangleProjection( 0, 0 );
      rhs_second->addVertex( 0, 0, 0 );
      rhs_second->addVertex( 1, 0, 0 );
      rhs_second->addVertex( 1, 1, 0 );
      rhs_second->addTriangle( 0, 1, 2 );
      rhs_second->addProjection( "first", 0, 0, 0, Model::TPT_Plane );
      rhs_second->addProjection( "second", 1, 1, 1, Model::TPT_Plane );
      rhs_second->setTriangleProjection( 0, 1 );

      QVERIFY_EQ( -1, lhs->getTriangleProjection( 0 ) );
      lhs->operationComplete( "Add triangle and projections" );

      lhs->setTriangleProjection( 0, 0 );
      QVERIFY_EQ( 0, lhs->getTriangleProjection( 0 ) );
      lhs->operationComplete( "Set first projection" );

      lhs->setTriangleProjection( 0, 1 );
      QVERIFY_EQ( 1, lhs->getTriangleProjection( 0 ) );
      lhs->operationComplete( "Set second projection" );

      lhs->setTriangleProjection( 0, -1 );
      QVERIFY_EQ( -1, lhs->getTriangleProjection( 0 ) );
      lhs->operationComplete( "Set no projection" );

      checkUndoRedo( 4, lhs.get(), rhs_list );
   }

   void testFaceOutNone()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addTriangle( 0, 1, 2 );

      // Nothing in front, must face out
      QVERIFY_FALSE( lhs->triangleFacesIn( 0 ) );
   }

   void testFaceInBack()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 0, 1 );
      lhs->addVertex( 1, 0, 1 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 3, 4, 5 );

      QVERIFY_TRUE( lhs->triangleFacesIn( 0 ) );
      QVERIFY_FALSE( lhs->triangleFacesIn( 1 ) );
   }

   void testFaceInFront()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 0, 1 );
      lhs->addVertex( 1, 0, 1 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 5, 4, 3 );

      QVERIFY_TRUE( lhs->triangleFacesIn( 0 ) );
      QVERIFY_TRUE( lhs->triangleFacesIn( 1 ) );
   }

   void testFaceOutTwo()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 0, 1 );
      lhs->addVertex( 1, 0, 1 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 0, 0, 2 );
      lhs->addVertex( 1, 0, 2 );
      lhs->addVertex( 1, 1, 2 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 3, 4, 5 );
      lhs->addTriangle( 6, 7, 8 );

      // Hits an even number of triangles, must face out
      QVERIFY_FALSE( lhs->triangleFacesIn( 0 ) );
   }

   void testFaceOutBack()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 0, 1 );
      lhs->addVertex( 1, 0, 1 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addTriangle( 2, 1, 0 );
      lhs->addTriangle( 3, 4, 5 );

      QVERIFY_FALSE( lhs->triangleFacesIn( 0 ) );
      QVERIFY_FALSE( lhs->triangleFacesIn( 1 ) );
   }

   void testFaceInOneEdge()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 0, 1 );
      lhs->addVertex( 1, 0, 1 );
      lhs->addVertex( 0, 1, 1 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 5, 4, 3 );

      QVERIFY_FALSE( lhs->triangleFacesIn( 0 ) );
   }

   void testFaceInTwoEdges()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 0, 1 );
      lhs->addVertex( 1, 0, 1 );
      lhs->addVertex( 0, 1, 1 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 5, 4, 3 );
      lhs->addTriangle( 5, 6, 4 );

      // Hits two edges, must be inside
      QVERIFY_TRUE( lhs->triangleFacesIn( 0 ) );
   }

   void testFaceOutFrontAndBack()
   {
      local_ptr<Model> lhs = newTestModel();

      lhs->addVertex( 0, 0, 0 );
      lhs->addVertex( 1, 0, 0 );
      lhs->addVertex( 1, 1, 0 );
      lhs->addVertex( 0, 0, 1 );
      lhs->addVertex( 1, 0, 1 );
      lhs->addVertex( 1, 1, 1 );
      lhs->addVertex( 0, 0, -1 );
      lhs->addVertex( 1, 0, -1 );
      lhs->addVertex( 1, 1, -1 );
      lhs->addTriangle( 0, 1, 2 );
      lhs->addTriangle( 3, 4, 5 );
      lhs->addTriangle( 6, 7, 8 );

      // Hits one in front and one in back. Something screwy is going on.
      // Can't be certain it is facing inward.
      QVERIFY_FALSE( lhs->triangleFacesIn( 0 ) );
   }

   // FIXME Tests to add
   //
   // Triangle tests:
   //  x Add/delete triangle
   //  x Deleting triangle deletes vertex
   //  x Deleting triangle does not delete free vertex
   //  x Deleting triangle does not delete shared vertex
   //  x Set triangle vertex
   //    Set texture coords
   //    Normal calculation
   //       Flat
   //       Smoothed
   //       Blending
   //    Invert normal
   //  x Selection
   //  x getSelectedTriangles
   //  x Hiding/visbility
   //    Texture projection assignment
   //    cosToPoint
   //  x triangleFacesIn
   //  x triangle count
   //  x deleteFlattendTriangles
   //    subdivide
   //  x getTriangleVertex
   //
   // FIXME test in other files:
   //    simplify mesh?
   //    select vertices from triangles

};

QTEST_MAIN(ModelTriangleTest)
#include "model_triangle_test.moc"

