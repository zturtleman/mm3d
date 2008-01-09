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


// This file tests adding and removing model components, along with undo/redo.

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

class ModelAddTest : public QObject
{
   Q_OBJECT
private:

   void undoRedo( Model * lhs, Model * rhs1, Model * rhs2 )
   {
      int bits = Model::CompareAll;
      QVERIFY_EQ( bits, lhs->equal( rhs2, bits ) );
      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs1, bits ) );
      lhs->redo();
      QVERIFY_EQ( bits, lhs->equal( rhs2, bits ) );
      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs1, bits ) );
      lhs->redo();
      QVERIFY_EQ( bits, lhs->equal( rhs2, bits ) );
   }

   void addTriangleVertices( Model * m )
   {
      for ( int t = 0; t < 6; ++t )
      {
         double c = (double) (t + 1);
         m->addVertex( c, c, c );
         m->setVertexFree( t, true );
      }
      m->operationComplete( "Add triangle test vertices" );
   }

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   void testVertex()
   {
      int bits = Model::CompareAll;

      local_ptr<Model> lhs = newTestModel();

      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1v = newTestModel();
      local_ptr<Model> rhs_2v = newTestModel();
      local_ptr<Model> rhs_3v = newTestModel();
      local_ptr<Model> rhs_deleted_v1 = newTestModel();
      local_ptr<Model> rhs_deleted_v2 = newTestModel();
      local_ptr<Model> rhs_deleted_v3 = newTestModel();

      QVERIFY_EQ( bits, lhs->equal( rhs_empty.get(), bits ) );
      QVERIFY_EQ( 0, (int) lhs->getVertexCount() );

      lhs->addVertex( 1, 1, 1 );
      rhs_1v->addVertex( 1, 1, 1 );
      rhs_2v->addVertex( 1, 1, 1 );
      rhs_3v->addVertex( 1, 1, 1 );
      rhs_deleted_v2->addVertex( 1, 1, 1 );
      rhs_deleted_v3->addVertex( 1, 1, 1 );
      lhs->operationComplete( "Add Vertex 1" );

      QVERIFY_EQ( bits, lhs->equal( rhs_1v.get(), bits ) );
      QVERIFY_EQ( 1, (int) lhs->getVertexCount() );
      undoRedo( lhs.get(), rhs_empty.get(), rhs_1v.get() );

      lhs->addVertex( 2, 2, 2 );
      rhs_2v->addVertex( 2, 2, 2 );
      rhs_3v->addVertex( 2, 2, 2 );
      rhs_deleted_v1->addVertex( 2, 2, 2 );
      rhs_deleted_v3->addVertex( 2, 2, 2 );
      lhs->operationComplete( "Add Vertex 2" );

      QVERIFY_EQ( bits, lhs->equal( rhs_2v.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getVertexCount() );
      undoRedo( lhs.get(), rhs_1v.get(), rhs_2v.get() );

      lhs->addVertex( 3, 3, 3 );
      rhs_3v->addVertex( 3, 3, 3 );
      rhs_deleted_v1->addVertex( 3, 3, 3 );
      rhs_deleted_v2->addVertex( 3, 3, 3 );
      lhs->operationComplete( "Add Vertex 3" );

      QVERIFY_EQ( bits, lhs->equal( rhs_3v.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getVertexCount() );
      undoRedo( lhs.get(), rhs_2v.get(), rhs_3v.get() );

      lhs->deleteVertex( 0 );
      lhs->operationComplete( "Delete Vertex 1" );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_v1.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getVertexCount() );
      undoRedo( lhs.get(), rhs_3v.get(), rhs_deleted_v1.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3v.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getVertexCount() );

      lhs->deleteVertex( 1 );
      lhs->operationComplete( "Delete Vertex 2" );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_v2.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getVertexCount() );
      undoRedo( lhs.get(), rhs_3v.get(), rhs_deleted_v2.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3v.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getVertexCount() );

      lhs->deleteVertex( 2 );
      lhs->operationComplete( "Delete Vertex 3" );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_v3.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getVertexCount() );
      undoRedo( lhs.get(), rhs_3v.get(), rhs_deleted_v3.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3v.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getVertexCount() );

      undoRedo( lhs.get(), rhs_2v.get(), rhs_3v.get() );
      lhs->undo();
      QVERIFY_EQ( 2, (int) lhs->getVertexCount() );
      undoRedo( lhs.get(), rhs_1v.get(), rhs_2v.get() );
      lhs->undo();
      QVERIFY_EQ( 1, (int) lhs->getVertexCount() );
      undoRedo( lhs.get(), rhs_empty.get(), rhs_1v.get() );
   }

   void testTriangle()
   {
      int bits = Model::CompareAll;

      local_ptr<Model> lhs = newTestModel();

      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1t = newTestModel();
      local_ptr<Model> rhs_2t = newTestModel();
      local_ptr<Model> rhs_3t = newTestModel();
      local_ptr<Model> rhs_deleted_t1 = newTestModel();
      local_ptr<Model> rhs_deleted_t2 = newTestModel();
      local_ptr<Model> rhs_deleted_t3 = newTestModel();

      addTriangleVertices( lhs.get() );
      addTriangleVertices( rhs_empty.get() );
      addTriangleVertices( rhs_1t.get() );
      addTriangleVertices( rhs_2t.get() );
      addTriangleVertices( rhs_3t.get() );
      addTriangleVertices( rhs_deleted_t1.get() );
      addTriangleVertices( rhs_deleted_t2.get() );
      addTriangleVertices( rhs_deleted_t3.get() );

      QVERIFY_EQ( bits, lhs->equal( rhs_empty.get(), bits ) );

      lhs->addTriangle( 0, 1, 2 );
      rhs_1t->addTriangle( 0, 1, 2 );
      rhs_2t->addTriangle( 0, 1, 2 );
      rhs_3t->addTriangle( 0, 1, 2 );
      rhs_deleted_t2->addTriangle( 0, 1, 2 );
      rhs_deleted_t3->addTriangle( 0, 1, 2 );
      lhs->operationComplete( "Add Triangle 1" );

      QVERIFY_EQ( bits, lhs->equal( rhs_1t.get(), bits ) );
      QVERIFY_EQ( 1, (int) lhs->getTriangleCount() );

      lhs->addTriangle( 3, 4, 5 );
      rhs_2t->addTriangle( 3, 4, 5 );
      rhs_3t->addTriangle( 3, 4, 5 );
      rhs_deleted_t1->addTriangle( 3, 4, 5 );
      rhs_deleted_t3->addTriangle( 3, 4, 5 );
      lhs->operationComplete( "Add Triangle 2" );

      QVERIFY_EQ( bits, lhs->equal( rhs_2t.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getTriangleCount() );

      lhs->addTriangle( 1, 3, 5 );
      rhs_3t->addTriangle( 1, 3, 5 );
      rhs_deleted_t1->addTriangle( 1, 3, 5 );
      rhs_deleted_t2->addTriangle( 1, 3, 5 );
      lhs->operationComplete( "Add Triangle 3" );

      QVERIFY_EQ( bits, lhs->equal( rhs_3t.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getTriangleCount() );

      lhs->deleteTriangle( 0 );
      lhs->operationComplete( "Delete Triangle 1" );
      QVERIFY_EQ( 2, (int) lhs->getTriangleCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_t1.get(), bits ) );
      undoRedo( lhs.get(), rhs_3t.get(), rhs_deleted_t1.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3t.get(), bits ) );

      lhs->deleteTriangle( 1 );
      lhs->operationComplete( "Delete Triangle 2" );
      QVERIFY_EQ( 2, (int) lhs->getTriangleCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_t2.get(), bits ) );
      undoRedo( lhs.get(), rhs_3t.get(), rhs_deleted_t2.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3t.get(), bits ) );

      lhs->deleteTriangle( 2 );
      lhs->operationComplete( "Delete Triangle 3" );
      QVERIFY_EQ( 2, (int) lhs->getTriangleCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_t3.get(), bits ) );
      undoRedo( lhs.get(), rhs_3t.get(), rhs_deleted_t3.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3t.get(), bits ) );

      QVERIFY_EQ( 3, (int) lhs->getTriangleCount() );
      undoRedo( lhs.get(), rhs_2t.get(), rhs_3t.get() );
      lhs->undo();
      QVERIFY_EQ( 2, (int) lhs->getTriangleCount() );
      undoRedo( lhs.get(), rhs_1t.get(), rhs_2t.get() );
      lhs->undo();
      QVERIFY_EQ( 1, (int) lhs->getTriangleCount() );
      undoRedo( lhs.get(), rhs_empty.get(), rhs_1t.get() );
   }

   void testGroup()
   {
      int bits = Model::CompareAll;

      local_ptr<Model> lhs = newTestModel();

      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();
      local_ptr<Model> rhs_3 = newTestModel();
      local_ptr<Model> rhs_deleted_1 = newTestModel();
      local_ptr<Model> rhs_deleted_2 = newTestModel();
      local_ptr<Model> rhs_deleted_3 = newTestModel();

      QVERIFY_EQ( bits, lhs->equal( rhs_empty.get(), bits ) );

      lhs->addGroup( "Group 1" );
      rhs_1->addGroup( "Group 1" );
      rhs_2->addGroup( "Group 1" );
      rhs_3->addGroup( "Group 1" );
      rhs_deleted_2->addGroup( "Group 1" );
      rhs_deleted_3->addGroup( "Group 1" );
      lhs->operationComplete( "Add Group 1" );

      QVERIFY_EQ( bits, lhs->equal( rhs_1.get(), bits ) );
      QVERIFY_EQ( 1, (int) lhs->getGroupCount() );

      lhs->addGroup( "Group 2" );
      rhs_2->addGroup( "Group 2" );
      rhs_3->addGroup( "Group 2" );
      rhs_deleted_1->addGroup( "Group 2" );
      rhs_deleted_3->addGroup( "Group 2" );
      lhs->operationComplete( "Add Group 2" );

      QVERIFY_EQ( bits, lhs->equal( rhs_2.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getGroupCount() );

      lhs->addGroup( "Group 3" );
      rhs_3->addGroup( "Group 3" );
      rhs_deleted_1->addGroup( "Group 3" );
      rhs_deleted_2->addGroup( "Group 3" );
      lhs->operationComplete( "Add Group 3" );

      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getGroupCount() );

      lhs->deleteGroup( 0 );
      lhs->operationComplete( "Delete Group 1" );
      QVERIFY_EQ( 2, (int) lhs->getGroupCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_1.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_1.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deleteGroup( 1 );
      lhs->operationComplete( "Delete Group 2" );
      QVERIFY_EQ( 2, (int) lhs->getGroupCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_2.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_2.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deleteGroup( 2 );
      lhs->operationComplete( "Delete Group 3" );
      QVERIFY_EQ( 2, (int) lhs->getGroupCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_3.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_3.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      QVERIFY_EQ( 3, (int) lhs->getGroupCount() );
      undoRedo( lhs.get(), rhs_2.get(), rhs_3.get() );
      lhs->undo();
      QVERIFY_EQ( 2, (int) lhs->getGroupCount() );
      undoRedo( lhs.get(), rhs_1.get(), rhs_2.get() );
      lhs->undo();
      QVERIFY_EQ( 1, (int) lhs->getGroupCount() );
      undoRedo( lhs.get(), rhs_empty.get(), rhs_1.get() );
   }

   void testMaterial()
   {
      int bits = Model::CompareAll;

      local_ptr<Model> lhs = newTestModel();

      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();
      local_ptr<Model> rhs_3 = newTestModel();
      local_ptr<Model> rhs_deleted_1 = newTestModel();
      local_ptr<Model> rhs_deleted_2 = newTestModel();
      local_ptr<Model> rhs_deleted_3 = newTestModel();

      QVERIFY_EQ( bits, lhs->equal( rhs_empty.get(), bits ) );

      lhs->addColorMaterial( "Material 1" );
      rhs_1->addColorMaterial( "Material 1" );
      rhs_2->addColorMaterial( "Material 1" );
      rhs_3->addColorMaterial( "Material 1" );
      rhs_deleted_2->addColorMaterial( "Material 1" );
      rhs_deleted_3->addColorMaterial( "Material 1" );
      lhs->operationComplete( "Add Material 1" );

      QVERIFY_EQ( bits, lhs->equal( rhs_1.get(), bits ) );
      QVERIFY_EQ( 1, (int) lhs->getTextureCount() );

      lhs->addColorMaterial( "Material 2" );
      rhs_2->addColorMaterial( "Material 2" );
      rhs_3->addColorMaterial( "Material 2" );
      rhs_deleted_1->addColorMaterial( "Material 2" );
      rhs_deleted_3->addColorMaterial( "Material 2" );
      lhs->operationComplete( "Add Material 2" );

      QVERIFY_EQ( bits, lhs->equal( rhs_2.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getTextureCount() );

      lhs->addColorMaterial( "Material 3" );
      rhs_3->addColorMaterial( "Material 3" );
      rhs_deleted_1->addColorMaterial( "Material 3" );
      rhs_deleted_2->addColorMaterial( "Material 3" );
      lhs->operationComplete( "Add Material 3" );

      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getTextureCount() );

      lhs->deleteTexture( 0 );
      lhs->operationComplete( "Delete Material 1" );
      QVERIFY_EQ( 2, (int) lhs->getTextureCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_1.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_1.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deleteTexture( 1 );
      lhs->operationComplete( "Delete Material 2" );
      QVERIFY_EQ( 2, (int) lhs->getTextureCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_2.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_2.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deleteTexture( 2 );
      lhs->operationComplete( "Delete Material 3" );
      QVERIFY_EQ( 2, (int) lhs->getTextureCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_3.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_3.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      QVERIFY_EQ( 3, (int) lhs->getTextureCount() );
      undoRedo( lhs.get(), rhs_2.get(), rhs_3.get() );
      lhs->undo();
      QVERIFY_EQ( 2, (int) lhs->getTextureCount() );
      undoRedo( lhs.get(), rhs_1.get(), rhs_2.get() );
      lhs->undo();
      QVERIFY_EQ( 1, (int) lhs->getTextureCount() );
      undoRedo( lhs.get(), rhs_empty.get(), rhs_1.get() );
   }

   void testJoint()
   {
      int bits = Model::CompareAll;

      local_ptr<Model> lhs = newTestModel();

      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();
      local_ptr<Model> rhs_3 = newTestModel();
      local_ptr<Model> rhs_deleted_2 = newTestModel();
      local_ptr<Model> rhs_deleted_3 = newTestModel();

      QVERIFY_EQ( bits, lhs->equal( rhs_empty.get(), bits ) );

      lhs->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_1->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_2->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_3->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_deleted_2->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_deleted_3->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0, -1 );
      lhs->operationComplete( "Add Joint 1" );

      QVERIFY_EQ( bits, lhs->equal( rhs_1.get(), bits ) );
      QVERIFY_EQ( 1, (int) lhs->getBoneJointCount() );

      lhs->addBoneJoint( "Joint 2", 2, 2, 2, 0, 0, 0, 0 );
      rhs_2->addBoneJoint( "Joint 2", 2, 2, 2, 0, 0, 0, 0 );
      rhs_3->addBoneJoint( "Joint 2", 2, 2, 2, 0, 0, 0, 0 );
      rhs_deleted_3->addBoneJoint( "Joint 2", 2, 2, 2, 0, 0, 0, 0 );
      lhs->operationComplete( "Add Joint 2" );

      QVERIFY_EQ( bits, lhs->equal( rhs_2.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getBoneJointCount() );

      lhs->addBoneJoint( "Joint 3", 3, 3, 3, 0, 0, 0, 0 );
      rhs_3->addBoneJoint( "Joint 3", 3, 3, 3, 0, 0, 0, 0 );
      rhs_deleted_2->addBoneJoint( "Joint 3", 3, 3, 3, 0, 0, 0, 0 );
      lhs->operationComplete( "Add Joint 3" );

      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getBoneJointCount() );

      lhs->deleteBoneJoint( 1 );
      lhs->operationComplete( "Delete Joint 2" );
      QVERIFY_EQ( 2, (int) lhs->getBoneJointCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_2.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_2.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deleteBoneJoint( 2 );
      lhs->operationComplete( "Delete Joint 3" );
      QVERIFY_EQ( 2, (int) lhs->getBoneJointCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_3.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_3.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      QVERIFY_EQ( 3, (int) lhs->getBoneJointCount() );
      undoRedo( lhs.get(), rhs_2.get(), rhs_3.get() );
      lhs->undo();
      QVERIFY_EQ( 2, (int) lhs->getBoneJointCount() );
      undoRedo( lhs.get(), rhs_1.get(), rhs_2.get() );
      lhs->undo();
      QVERIFY_EQ( 1, (int) lhs->getBoneJointCount() );
      undoRedo( lhs.get(), rhs_empty.get(), rhs_1.get() );
   }

   void testPoint()
   {
      int bits = Model::CompareAll;

      local_ptr<Model> lhs = newTestModel();

      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();
      local_ptr<Model> rhs_3 = newTestModel();
      local_ptr<Model> rhs_deleted_1 = newTestModel();
      local_ptr<Model> rhs_deleted_2 = newTestModel();
      local_ptr<Model> rhs_deleted_3 = newTestModel();

      QVERIFY_EQ( bits, lhs->equal( rhs_empty.get(), bits ) );

      lhs->addPoint( "Point 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_1->addPoint( "Point 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_2->addPoint( "Point 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_3->addPoint( "Point 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_deleted_2->addPoint( "Point 1", 1, 1, 1, 0, 0, 0, -1 );
      rhs_deleted_3->addPoint( "Point 1", 1, 1, 1, 0, 0, 0, -1 );
      lhs->operationComplete( "Add Point 1" );

      QVERIFY_EQ( bits, lhs->equal( rhs_1.get(), bits ) );
      QVERIFY_EQ( 1, (int) lhs->getPointCount() );

      lhs->addPoint( "Point 2", 2, 2, 2, 0, 0, 0, -1 );
      rhs_2->addPoint( "Point 2", 2, 2, 2, 0, 0, 0, -1 );
      rhs_3->addPoint( "Point 2", 2, 2, 2, 0, 0, 0, -1 );
      rhs_deleted_1->addPoint( "Point 2", 2, 2, 2, 0, 0, 0, -1 );
      rhs_deleted_3->addPoint( "Point 2", 2, 2, 2, 0, 0, 0, -1 );
      lhs->operationComplete( "Add Point 2" );

      QVERIFY_EQ( bits, lhs->equal( rhs_2.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getPointCount() );

      lhs->addPoint( "Point 3", 3, 3, 3, 0, 0, 0, -1 );
      rhs_3->addPoint( "Point 3", 3, 3, 3, 0, 0, 0, -1 );
      rhs_deleted_1->addPoint( "Point 3", 3, 3, 3, 0, 0, 0, -1 );
      rhs_deleted_2->addPoint( "Point 3", 3, 3, 3, 0, 0, 0, -1 );
      lhs->operationComplete( "Add Point 3" );

      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getPointCount() );

      lhs->deletePoint( 0 );
      lhs->operationComplete( "Delete Point 1" );
      QVERIFY_EQ( 2, (int) lhs->getPointCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_1.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_1.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deletePoint( 1 );
      lhs->operationComplete( "Delete Point 2" );
      QVERIFY_EQ( 2, (int) lhs->getPointCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_2.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_2.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deletePoint( 2 );
      lhs->operationComplete( "Delete Point 3" );
      QVERIFY_EQ( 2, (int) lhs->getPointCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_3.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_3.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      QVERIFY_EQ( 3, (int) lhs->getPointCount() );
      undoRedo( lhs.get(), rhs_2.get(), rhs_3.get() );
      lhs->undo();
      QVERIFY_EQ( 2, (int) lhs->getPointCount() );
      undoRedo( lhs.get(), rhs_1.get(), rhs_2.get() );
      lhs->undo();
      QVERIFY_EQ( 1, (int) lhs->getPointCount() );
      undoRedo( lhs.get(), rhs_empty.get(), rhs_1.get() );
   }

   void testProjection()
   {
      int bits = Model::CompareAll;

      local_ptr<Model> lhs = newTestModel();

      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();
      local_ptr<Model> rhs_3 = newTestModel();
      local_ptr<Model> rhs_deleted_1 = newTestModel();
      local_ptr<Model> rhs_deleted_2 = newTestModel();
      local_ptr<Model> rhs_deleted_3 = newTestModel();

      QVERIFY_EQ( bits, lhs->equal( rhs_empty.get(), bits ) );

      lhs->addProjection( "Projection 1", Model::TPT_Cylinder, 1, 1, 1);
      rhs_1->addProjection( "Projection 1", Model::TPT_Cylinder, 1, 1, 1);
      rhs_2->addProjection( "Projection 1", Model::TPT_Cylinder, 1, 1, 1);
      rhs_3->addProjection( "Projection 1", Model::TPT_Cylinder, 1, 1, 1);
      rhs_deleted_2->addProjection( "Projection 1", Model::TPT_Cylinder, 1, 1, 1);
      rhs_deleted_3->addProjection( "Projection 1", Model::TPT_Cylinder, 1, 1, 1);
      lhs->operationComplete( "Add Projection 1" );

      QVERIFY_EQ( bits, lhs->equal( rhs_1.get(), bits ) );
      QVERIFY_EQ( 1, (int) lhs->getProjectionCount() );

      lhs->addProjection( "Projection 2", Model::TPT_Cylinder, 2, 2, 2);
      rhs_2->addProjection( "Projection 2", Model::TPT_Cylinder, 2, 2, 2);
      rhs_3->addProjection( "Projection 2", Model::TPT_Cylinder, 2, 2, 2);
      rhs_deleted_1->addProjection( "Projection 2", Model::TPT_Cylinder, 2, 2, 2);
      rhs_deleted_3->addProjection( "Projection 2", Model::TPT_Cylinder, 2, 2, 2);
      lhs->operationComplete( "Add Projection 2" );

      QVERIFY_EQ( bits, lhs->equal( rhs_2.get(), bits ) );
      QVERIFY_EQ( 2, (int) lhs->getProjectionCount() );

      lhs->addProjection( "Projection 3", Model::TPT_Cylinder, 3, 3, 3);
      rhs_3->addProjection( "Projection 3", Model::TPT_Cylinder, 3, 3, 3);
      rhs_deleted_1->addProjection( "Projection 3", Model::TPT_Cylinder, 3, 3, 3);
      rhs_deleted_2->addProjection( "Projection 3", Model::TPT_Cylinder, 3, 3, 3);
      lhs->operationComplete( "Add Projection 3" );

      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );
      QVERIFY_EQ( 3, (int) lhs->getProjectionCount() );

      lhs->deleteProjection( 0 );
      lhs->operationComplete( "Delete Projection 1" );
      QVERIFY_EQ( 2, (int) lhs->getProjectionCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_1.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_1.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deleteProjection( 1 );
      lhs->operationComplete( "Delete Projection 2" );
      QVERIFY_EQ( 2, (int) lhs->getProjectionCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_2.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_2.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      lhs->deleteProjection( 2 );
      lhs->operationComplete( "Delete Projection 3" );
      QVERIFY_EQ( 2, (int) lhs->getProjectionCount() );

      QVERIFY_EQ( bits, lhs->equal( rhs_deleted_3.get(), bits ) );
      undoRedo( lhs.get(), rhs_3.get(), rhs_deleted_3.get() );

      lhs->undo();
      QVERIFY_EQ( bits, lhs->equal( rhs_3.get(), bits ) );

      QVERIFY_EQ( 3, (int) lhs->getProjectionCount() );
      undoRedo( lhs.get(), rhs_2.get(), rhs_3.get() );
      lhs->undo();
      QVERIFY_EQ( 2, (int) lhs->getProjectionCount() );
      undoRedo( lhs.get(), rhs_1.get(), rhs_2.get() );
      lhs->undo();
      QVERIFY_EQ( 1, (int) lhs->getProjectionCount() );
      undoRedo( lhs.get(), rhs_empty.get(), rhs_1.get() );
   }

   void testForceAddOrDelete()
   {
      local_ptr<Model> m = newTestModel();

      m->forceAddOrDelete( false );

      // Make sure we can add/delete before having frame anims.
      QVERIFY_EQ( 0, m->addVertex( 1, 1, 1 ) );
      QVERIFY_EQ( 1, m->addVertex( 2, 2, 2 ) );
      QVERIFY_EQ( 2, m->addVertex( 3, 3, 3 ) );
      QVERIFY_EQ( 3, (int) m->getVertexCount() );

      QVERIFY_EQ( 0, m->addAnimation( Model::ANIMMODE_FRAME, "frame anim" ) );

      // Now creates and deletes should fail
      QVERIFY_EQ( -1, m->addVertex( 4, 4, 4 ) );
      QVERIFY_EQ( 3, (int) m->getVertexCount() );
      QVERIFY_EQ( -1, m->addTriangle( 0, 1, 2 ) );
      QVERIFY_EQ( 0, (int) m->getTriangleCount() );
      QVERIFY_EQ( -1, m->addGroup( "failed group" ) );
      QVERIFY_EQ( 0, (int) m->getGroupCount() );
      QVERIFY_EQ( -1, m->addPoint( "failed point", 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 0, (int) m->getPointCount() );
      QVERIFY_EQ( 0, m->addColorMaterial( "first material" ) );
      QVERIFY_EQ( 1, (int) m->getTextureCount() );
      QVERIFY_EQ( 0, m->addBoneJoint( "first joint", 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 1, (int) m->getBoneJointCount() );
      QVERIFY_EQ( 0, m->addProjection( "first projection", Model::TPT_Cylinder, 1, 1, 1 ) );
      QVERIFY_EQ( 1, (int) m->getProjectionCount() );

      m->forceAddOrDelete( true );

      QVERIFY_EQ( 3, m->addVertex( 4, 4, 4 ) );
      QVERIFY_EQ( 4, (int) m->getVertexCount() );
      QVERIFY_EQ( 0, m->addTriangle( 0, 1, 2 ) );
      QVERIFY_EQ( 1, (int) m->getTriangleCount() );
      QVERIFY_EQ( 0, m->addGroup( "success group" ) );
      QVERIFY_EQ( 1, (int) m->getGroupCount() );
      QVERIFY_EQ( 0, m->addPoint( "success point", 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 1, (int) m->getPointCount() );
      QVERIFY_EQ( 1, m->addColorMaterial( "second material" ) );
      QVERIFY_EQ( 2, (int) m->getTextureCount() );
      QVERIFY_EQ( 1, m->addBoneJoint( "second joint", 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 2, (int) m->getBoneJointCount() );
      QVERIFY_EQ( 1, m->addProjection( "second projection", Model::TPT_Cylinder, 1, 1, 1 ) );
      QVERIFY_EQ( 2, (int) m->getProjectionCount() );

      // FIXME test delete
      m->forceAddOrDelete( false );

      m->deleteVertex(0);
      QVERIFY_EQ( 4, (int) m->getVertexCount() );
      m->deleteTriangle(0);
      QVERIFY_EQ( 1, (int) m->getTriangleCount() );
      m->deleteGroup(0);
      QVERIFY_EQ( 1, (int) m->getGroupCount() );
      m->deletePoint(0);
      QVERIFY_EQ( 1, (int) m->getPointCount() );
      m->deleteTexture(0);
      QVERIFY_EQ( 1, (int) m->getTextureCount() );
      m->deleteBoneJoint(1);
      QVERIFY_EQ( 1, (int) m->getBoneJointCount() );
      m->deleteProjection(0);
      QVERIFY_EQ( 1, (int) m->getProjectionCount() );

      m->forceAddOrDelete( true );

      m->deleteVertex(0);
      QVERIFY_EQ( 3, (int) m->getVertexCount() );
      m->deleteTriangle(0);
      QVERIFY_EQ( 0, (int) m->getTriangleCount() );
      m->deleteGroup(0);
      QVERIFY_EQ( 0, (int) m->getGroupCount() );
      m->deletePoint(0);
      QVERIFY_EQ( 0, (int) m->getPointCount() );
      m->deleteTexture(0);
      QVERIFY_EQ( 0, (int) m->getTextureCount() );
      m->deleteBoneJoint(0);
      QVERIFY_EQ( 0, (int) m->getBoneJointCount() );
      m->deleteProjection(0);
      QVERIFY_EQ( 0, (int) m->getProjectionCount() );
   }

   void testVertexFailure()
   {
      local_ptr<Model> m = newTestModel();

      QVERIFY_EQ( 0, m->addAnimation( Model::ANIMMODE_SKELETAL, "anim name" ) );
      QVERIFY_EQ( 0, m->addVertex( 0, 0, 0 ) );

      // Works
      QVERIFY_EQ( 1, (int) m->getVertexCount() );

      // In Animation mode
      m->setCurrentAnimation( Model::ANIMMODE_SKELETAL, (unsigned int) 0 );
      QVERIFY_EQ( -1, m->addVertex( 1, 1, 1 ) );
      QVERIFY_EQ( 1, (int) m->getVertexCount() );
   }

   void testTriangleFailure()
   {
      local_ptr<Model> m = newTestModel();

      m->addVertex( 1, 1, 1 );
      m->addVertex( 2, 2, 2 );
      m->addVertex( 3, 3, 3 );
      m->addVertex( 4, 4, 4 );

      QVERIFY_EQ( 0, m->addAnimation( Model::ANIMMODE_SKELETAL, "anim name" ) );

      // Works
      QVERIFY_EQ( 0, m->addTriangle( 1, 2, 3 ) );
      QVERIFY_EQ( 1, (int) m->getTriangleCount() );

      // Vertex out of range
      QVERIFY_EQ( -1, m->addTriangle( 1, 2, 4 ) );
      QVERIFY_EQ( 1, (int) m->getTriangleCount() );

      // In animation mode
      m->setCurrentAnimation( Model::ANIMMODE_SKELETAL, (unsigned int) 0 );
      QVERIFY_EQ( -1, m->addTriangle( 1, 2, 3 ) );
      QVERIFY_EQ( 1, (int) m->getTriangleCount() );
   }

   void testGroupFailure()
   {
      local_ptr<Model> m = newTestModel();

      QVERIFY_EQ( 0, m->addAnimation( Model::ANIMMODE_SKELETAL, "anim name" ) );
      QVERIFY_EQ( 0, m->addGroup( "group name" ) );
      QVERIFY_EQ( 1, (int) m->getGroupCount() );

      // NULL name
      QVERIFY_EQ( -1, m->addGroup( NULL ) );
      QVERIFY_EQ( 1, (int) m->getGroupCount() );

      // In animation mode
      m->setCurrentAnimation( Model::ANIMMODE_SKELETAL, (unsigned int) 0 );
      QVERIFY_EQ( -1, m->addVertex( 1, 1, 1 ) );
      QVERIFY_EQ( 1, (int) m->getGroupCount() );
   }

   void testMaterialFailure()
   {
      local_ptr<Model> m = newTestModel();

      QVERIFY_EQ( 0, m->addAnimation( Model::ANIMMODE_SKELETAL, "anim name" ) );
      QVERIFY_EQ( 0, m->addColorMaterial( "material name" ) );
      QVERIFY_EQ( 1, (int) m->getTextureCount() );

      // NULL texture
      QVERIFY_EQ( -1, m->addTexture( NULL ) );
      QVERIFY_EQ( 1, (int) m->getTextureCount() );

      // NULL name
      QVERIFY_EQ( -1, m->addColorMaterial( NULL ) );
      QVERIFY_EQ( 1, (int) m->getTextureCount() );

      // In animation mode
      m->setCurrentAnimation( Model::ANIMMODE_SKELETAL, (unsigned int) 0 );
      QVERIFY_EQ( -1, m->addColorMaterial( "failed material" ) );
      QVERIFY_EQ( 1, (int) m->getTextureCount() );
   }

   void testJointFailure()
   {
      local_ptr<Model> m = newTestModel();

      QVERIFY_EQ( 0, m->addAnimation( Model::ANIMMODE_SKELETAL, "anim name" ) );
      QVERIFY_EQ( 0, m->addBoneJoint( "joint name", 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 1, (int) m->getBoneJointCount() );

      // Bad parent
      QVERIFY_EQ( -1, m->addBoneJoint( "failed joint", 1, 1, 1, 0, 0, 0, 1 ) );
      QVERIFY_EQ( 1, (int) m->getBoneJointCount() );

      // NULL name
      QVERIFY_EQ( -1, m->addBoneJoint( NULL, 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 1, (int) m->getBoneJointCount() );

      // In animation mode
      m->setCurrentAnimation( Model::ANIMMODE_SKELETAL, (unsigned int) 0 );
      QVERIFY_EQ( -1, m->addBoneJoint( "anim joint", 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 1, (int) m->getBoneJointCount() );
   }

   void testPointFailure()
   {
      local_ptr<Model> m = newTestModel();

      QVERIFY_EQ( 0, m->addAnimation( Model::ANIMMODE_SKELETAL, "anim name" ) );
      QVERIFY_EQ( 0, m->addPoint( "point name", 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 1, (int) m->getPointCount() );

      // Bad parent
      QVERIFY_EQ( -1, m->addPoint( "failed point", 1, 1, 1, 0, 0, 0, 1 ) );
      QVERIFY_EQ( 1, (int) m->getPointCount() );

      // NULL name
      QVERIFY_EQ( -1, m->addPoint( NULL, 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 1, (int) m->getPointCount() );

      // In animation mode
      m->setCurrentAnimation( Model::ANIMMODE_SKELETAL, (unsigned int) 0 );
      QVERIFY_EQ( -1, m->addPoint( "anim point", 1, 1, 1, 0, 0, 0, -1 ) );
      QVERIFY_EQ( 1, (int) m->getPointCount() );
   }

   void testProjectionFailure()
   {
      local_ptr<Model> m = newTestModel();

      QVERIFY_EQ( 0, m->addAnimation( Model::ANIMMODE_SKELETAL, "anim name" ) );
      QVERIFY_EQ( 0, m->addProjection( "point name", Model::TPT_Cylinder, 1, 1, 1 ) );
      QVERIFY_EQ( 1, (int) m->getProjectionCount() );

      // NULL name
      QVERIFY_EQ( -1, m->addProjection( NULL, Model::TPT_Cylinder, 1, 1, 1 ) );
      QVERIFY_EQ( 1, (int) m->getProjectionCount() );

      // In animation mode
      m->setCurrentAnimation( Model::ANIMMODE_SKELETAL, (unsigned int) 0 );
      QVERIFY_EQ( -1, m->addProjection( "anim point", Model::TPT_Cylinder, 1, 1, 1 ) );
      QVERIFY_EQ( 1, (int) m->getProjectionCount() );
   }

   // FIXME deletion
   //   * Delete selected
   //   * Remove orphaned vertices
   //   * Remove flattened triangles
   //   * Index adjustments

   // FIXME test joint reparenting and vertex/point assignments

   // FIXME undo/redo release

};

QTEST_MAIN(ModelAddTest)
#include "model_add_test.moc"

