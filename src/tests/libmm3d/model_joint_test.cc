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


// This file tests bone joint methods in the Model class.

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "log.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"

#include <vector>

class ModelJointTest : public QObject
{
   Q_OBJECT
private:

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   void testAddBoneJoint()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      rhs_1->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      rhs_1->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      rhs_2->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      rhs_2->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      rhs_2->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );
      rhs_2->addBoneJoint( "Joint 4", 4, 4, 4, 0, 0, 1, 0 );

      QVERIFY_EQ( 0, (int) lhs->getBoneJointCount() );

      lhs->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      QVERIFY_EQ( 2, (int) lhs->getBoneJointCount() );
      QVERIFY_EQ( std::string("Joint 1"), std::string(lhs->getBoneJointName(0)) );
      QVERIFY_EQ( std::string("Joint 2"), std::string(lhs->getBoneJointName(1)) );

      lhs->operationComplete( "Add joints 1 and 2" );

      lhs->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );
      lhs->addBoneJoint( "Joint 4", 4, 4, 4, 0, 0, 1, 0 );
      QVERIFY_EQ( 4, (int) lhs->getBoneJointCount() );
      QVERIFY_EQ( std::string("Joint 1"), std::string(lhs->getBoneJointName(0)) );
      QVERIFY_EQ( std::string("Joint 2"), std::string(lhs->getBoneJointName(1)) );
      QVERIFY_EQ( std::string("Joint 3"), std::string(lhs->getBoneJointName(2)) );
      QVERIFY_EQ( std::string("Joint 4"), std::string(lhs->getBoneJointName(3)) );

      lhs->operationComplete( "Add joints 3 and 4" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testDeleteBoneJoint()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      rhs_1->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      rhs_1->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      rhs_1->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );
      rhs_2->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      // FIXME: This doesn't account for adding bone with rotated parent applying
      //        counter rotation or deleting bone moving translation.
      rhs_2->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 0 );

      QVERIFY_EQ( 0, (int) lhs->getBoneJointCount() );

      lhs->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );

      QVERIFY_EQ( 3, (int) lhs->getBoneJointCount() );
      QVERIFY_EQ( 1, (int) lhs->getBoneJointParent(2) );
      QVERIFY_EQ( std::string("Joint 1"), std::string(lhs->getBoneJointName(0)) );
      QVERIFY_EQ( std::string("Joint 2"), std::string(lhs->getBoneJointName(1)) );
      QVERIFY_EQ( std::string("Joint 3"), std::string(lhs->getBoneJointName(2)) );

      lhs->operationComplete( "Add joints" );

      lhs->deleteBoneJoint( 1 );
      QVERIFY_EQ( 2, (int) lhs->getBoneJointCount() );
      QVERIFY_EQ( 0, (int) lhs->getBoneJointParent(1) );
      QVERIFY_EQ( std::string("Joint 1"), std::string(lhs->getBoneJointName(0)) );
      QVERIFY_EQ( std::string("Joint 3"), std::string(lhs->getBoneJointName(1)) );

      lhs->operationComplete( "Delete joint" );

#warning checkUndoRedo() for testDeleteBoneJoint() is broken
#if 0 // See FIXME above
      checkUndoRedo( 2, lhs.get(), rhs_list );
#endif
   }

   void testRenameBoneJoint()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      rhs_1->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      rhs_1->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      rhs_1->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );
      rhs_2->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      rhs_2->addBoneJoint( "Renamed", 2, 2, 2, 1, 0, 0, 0 );
      rhs_2->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );

      QVERIFY_EQ( 0, (int) lhs->getBoneJointCount() );

      lhs->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );
      QVERIFY_EQ( 3, (int) lhs->getBoneJointCount() );
      QVERIFY_EQ( std::string("Joint 1"), std::string(lhs->getBoneJointName(0)) );
      QVERIFY_EQ( std::string("Joint 2"), std::string(lhs->getBoneJointName(1)) );
      QVERIFY_EQ( std::string("Joint 3"), std::string(lhs->getBoneJointName(2)) );

      lhs->operationComplete( "Add joints 1 and 2" );

      lhs->setBoneJointName( 1, "Renamed" );
      QVERIFY_EQ( std::string("Joint 1"), std::string(lhs->getBoneJointName(0)) );
      QVERIFY_EQ( std::string("Renamed"), std::string(lhs->getBoneJointName(1)) );
      QVERIFY_EQ( std::string("Joint 3"), std::string(lhs->getBoneJointName(2)) );

      lhs->operationComplete( "Renamed bone joint" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testMoveBoneJoint()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      rhs_1->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      rhs_1->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      rhs_1->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );
      rhs_2->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      rhs_2->addBoneJoint( "Joint 2", 4, 5, 6, 1, 0, 0, 0 );
      rhs_2->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );

      lhs->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Joint 2", 2, 2, 2, 1, 0, 0, 0 );
      lhs->addBoneJoint( "Joint 3", 3, 3, 3, 0, 1, 0, 1 );

      double expected[3] = { 0, 0, 0 };
      double actual[3] = { 0, 0, 0 };

      expected[0] = 1; expected[1] = 1; expected[2] = 1;
      lhs->getBoneJointCoords( 0, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      expected[0] = 2; expected[1] = 2; expected[2] = 2;
      lhs->getBoneJointCoords( 1, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      expected[0] = 3; expected[1] = 3; expected[2] = 3;
      lhs->getBoneJointCoords( 2, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );

      lhs->operationComplete( "Add joints 1 and 2" );

      lhs->moveBoneJoint( 1, 4, 5, 6 );

      expected[0] = 1; expected[1] = 1; expected[2] = 1;
      lhs->getBoneJointCoords( 0, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      expected[0] = 4; expected[1] = 5; expected[2] = 6;
      lhs->getBoneJointCoords( 1, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      expected[0] = 3; expected[1] = 3; expected[2] = 3;
      lhs->getBoneJointCoords( 2, actual );
      QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );

      lhs->operationComplete( "Move bone joint" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testGetLocalTranslation()
   {
      {
         local_ptr<Model> lhs = newTestModel();
         lhs->addBoneJoint( "Joint 1", 1, 1, 1, 0, 0, 0 );
         lhs->setupJoints();
         const double trans[3] = { 1, 2, 3 };
         lhs->setBoneJointTranslation( 0, trans );
         lhs->setupJoints();
         const double expected[3] = { 1, 2, 3 };
         double actual[3] = { 0, 0, 0 };
         lhs->getBoneJointCoords( 0, actual );
         QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      }

      {
         local_ptr<Model> lhs = newTestModel();
         lhs->addBoneJoint( "Joint 1", 1, 2, 3, 0, 0, 0 );
         lhs->addBoneJoint( "Joint 1", 2, 2, 2, 0, 0, 0, 0 );
         lhs->setupJoints();
         const double trans[3] = { 4, 3, 2 };
         lhs->setBoneJointTranslation( 1, trans );
         lhs->setupJoints();
         const double expected[3] = { 5, 5, 5 };
         double actual[3] = { 0, 0, 0 };
         lhs->getBoneJointCoords( 1, actual );
         QVERIFY_ARRAY_EQ( expected, 3, actual, 3 );
      }

      // FIXME rotation
      // FIXME multiple levels of joints
      // FIXME undo
   }

   // FIXME Tests to add:
   //  x add joint
   //  x delete joint
   //  x rename joint
   //  x get/set joint coords
   //  x moveBoneJoint
   //  x get joint parent
   //    selection (parent joint selected)
   //    hide/unhide
   //    joint matrix is correct
   //       Final
   //       Absolute
   //       Relative
   //  - setBoneJointTranslation
   //    setBoneJointRotation
   //    get/setVertexBoneJoint
   //    get/setPointBoneJoint
   //    getBoneJointVertices
   //
   // FIXME Tests to add (in other files):
   //    add/removeInfluence
   //       Single
   //       Multiple
   //       Too many
   //    getInfluenceList
   //    getPrimaryInfluence
   //    get/setInfluenceType
   //    get/setInfluenceWeight
   //    autoSetInfluences
   //    calcRemainderWeight
   //    Calculate weight
   //
   // FIXME Tests to add (in other files):
   //    Skeletal animation
   //    Setting keyframes directly
   //    Modifying keyframes with transforms
   //    Weight has proper effect with single joint
   //    Weight has proper effect with multiple joints
   //    Deleting a bone joint adjust influence indices

};

QTEST_MAIN(ModelJointTest)
#include "model_joint_test.moc"

