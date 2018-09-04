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


// This file tests animation methods in the Model class

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

class ModelAnimTest : public QObject
{
   Q_OBJECT
private:

   Model * loadTestModel()
   {
      Model * model = loadMm3dOrDie( "data/model_equal_test.mm3d" );
      model->setUndoEnabled( true );
      return model;
   }

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
      log_enable_warning( true );
   }

   void testAnimAdd()
   {
      for ( int m = Model::ANIMMODE_SKELETAL; m <= Model::ANIMMODE_FRAME; ++m )
      {
         const Model::AnimationModeE mode =
            static_cast<Model::AnimationModeE>( m );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs_empty = newTestModel();
         local_ptr<Model> rhs_1 = newTestModel();
         local_ptr<Model> rhs_2 = newTestModel();
         local_ptr<Model> rhs_3 = newTestModel();

         ModelList rhs_list;
         rhs_list.push_back( rhs_empty.get() );
         rhs_list.push_back( rhs_1.get() );
         rhs_list.push_back( rhs_2.get() );
         rhs_list.push_back( rhs_3.get() );

         QVERIFY_EQ( 0, (int) lhs->getAnimCount( mode ) );

         lhs->addAnimation( mode, "Animation A" );
         rhs_1->addAnimation( mode, "Animation A" );
         rhs_2->addAnimation( mode, "Animation A" );
         rhs_3->addAnimation( mode, "Animation A" );

         QVERIFY_EQ( 1, (int) lhs->getAnimCount( mode ) );

         lhs->operationComplete( "Add animation A" );

         lhs->addAnimation( mode, "Animation B" );
         rhs_2->addAnimation( mode, "Animation B" );
         rhs_3->addAnimation( mode, "Animation B" );

         QVERIFY_EQ( 2, (int) lhs->getAnimCount( mode ) );

         lhs->operationComplete( "Add animation B" );

         lhs->addAnimation( mode, "Animation C" );
         rhs_3->addAnimation( mode, "Animation C" );

         QVERIFY_EQ( 3, (int) lhs->getAnimCount( mode ) );

         lhs->operationComplete( "Add animation C" );

         checkUndoRedo( 3, lhs.get(), rhs_list );
      }
   }

   void testAnimDelete()
   {
      for ( int m = Model::ANIMMODE_SKELETAL; m <= Model::ANIMMODE_FRAME; ++m )
      {
         const Model::AnimationModeE mode =
            static_cast<Model::AnimationModeE>( m );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs_empty = newTestModel();
         local_ptr<Model> rhs_1 = newTestModel();
         local_ptr<Model> rhs_2 = newTestModel();
         local_ptr<Model> rhs_3 = newTestModel();

         ModelList rhs_list;
         rhs_list.push_back( rhs_empty.get() );
         rhs_list.push_back( rhs_1.get() );
         rhs_list.push_back( rhs_2.get() );
         rhs_list.push_back( rhs_3.get() );
         rhs_list.push_back( rhs_empty.get() );

         lhs->addAnimation( mode, "Animation A" );
         rhs_1->addAnimation( mode, "Animation A" );
         rhs_2->addAnimation( mode, "Animation A" );
         rhs_3->addAnimation( mode, "Animation A" );

         lhs->addAnimation( mode, "Animation B" );
         rhs_1->addAnimation( mode, "Animation B" );

         lhs->addAnimation( mode, "Animation C" );
         rhs_1->addAnimation( mode, "Animation C" );
         rhs_2->addAnimation( mode, "Animation C" );

         QVERIFY_EQ( 3, (int) lhs->getAnimCount( mode ) );

         lhs->operationComplete( "Add animations" );

         lhs->deleteAnimation( mode, 1 );
         lhs->operationComplete( "Delete animation B" );

         QVERIFY_EQ( 2, (int) lhs->getAnimCount( mode ) );

         lhs->deleteAnimation( mode, 1 );
         lhs->operationComplete( "Delete animation C" );

         QVERIFY_EQ( 1, (int) lhs->getAnimCount( mode ) );

         lhs->deleteAnimation( mode, 0 );
         lhs->operationComplete( "Delete animation A" );

         QVERIFY_EQ( 0, (int) lhs->getAnimCount( mode ) );

         checkUndoRedo( 4, lhs.get(), rhs_list );
      }
   }

   void testAnimSetName()
   {
      for ( int m = Model::ANIMMODE_SKELETAL; m <= Model::ANIMMODE_FRAME; ++m )
      {
         const Model::AnimationModeE mode =
            static_cast<Model::AnimationModeE>( m );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs_empty = newTestModel();
         local_ptr<Model> rhs_1 = newTestModel();
         local_ptr<Model> rhs_2 = newTestModel();
         local_ptr<Model> rhs_3 = newTestModel();

         ModelList rhs_list;
         rhs_list.push_back( rhs_empty.get() );
         rhs_list.push_back( rhs_1.get() );
         rhs_list.push_back( rhs_2.get() );
         rhs_list.push_back( rhs_3.get() );

         lhs->addAnimation( mode, "Animation A" );
         rhs_1->addAnimation( mode, "Animation A" );
         rhs_2->addAnimation( mode, "Animation A" );
         rhs_3->addAnimation( mode, "Rename A" );

         lhs->addAnimation( mode, "Animation B" );
         rhs_1->addAnimation( mode, "Animation B" );
         rhs_2->addAnimation( mode, "Rename B" );
         rhs_3->addAnimation( mode, "Rename B" );

         lhs->operationComplete( "Add animations" );

         QVERIFY_EQ( std::string("Animation A"), std::string(lhs->getAnimName(mode, 0)));
         QVERIFY_EQ( std::string("Animation B"), std::string(lhs->getAnimName(mode, 1)));

         lhs->setAnimName( mode, 1, "Rename B" );
         lhs->operationComplete( "Rename animation B" );

         QVERIFY_EQ( std::string("Animation A"), std::string(lhs->getAnimName(mode, 0)));
         QVERIFY_EQ( std::string("Rename B"), std::string(lhs->getAnimName(mode, 1)));

         lhs->setAnimName( mode, 0, "Rename A" );
         lhs->operationComplete( "Rename animation A" );

         QVERIFY_EQ( std::string("Rename A"), std::string(lhs->getAnimName(mode, 0)));
         QVERIFY_EQ( std::string("Rename B"), std::string(lhs->getAnimName(mode, 1)));

         checkUndoRedo( 3, lhs.get(), rhs_list );
      }
   }

   void testAnimSetFPS()
   {
      for ( int m = Model::ANIMMODE_SKELETAL; m <= Model::ANIMMODE_FRAME; ++m )
      {
         const Model::AnimationModeE mode =
            static_cast<Model::AnimationModeE>( m );

         local_ptr<Model> lhs = newTestModel();
         local_ptr<Model> rhs_empty = newTestModel();
         local_ptr<Model> rhs_1 = newTestModel();
         local_ptr<Model> rhs_2 = newTestModel();
         local_ptr<Model> rhs_3 = newTestModel();

         ModelList rhs_list;
         rhs_list.push_back( rhs_empty.get() );
         rhs_list.push_back( rhs_1.get() );
         rhs_list.push_back( rhs_2.get() );
         rhs_list.push_back( rhs_3.get() );

         lhs->addAnimation( mode, "Animation A" );
         rhs_1->addAnimation( mode, "Animation A" );
         rhs_2->addAnimation( mode, "Animation A" );
         rhs_3->addAnimation( mode, "Animation A" );

         lhs->addAnimation( mode, "Animation B" );
         rhs_1->addAnimation( mode, "Animation B" );
         rhs_2->addAnimation( mode, "Animation B" );
         rhs_3->addAnimation( mode, "Animation B" );

         lhs->operationComplete( "Add animations" );

         double defaultFps = mode == Model::ANIMMODE_FRAME ? 10.0 : 30.0;
         QVERIFY_TRUE( float_equiv(defaultFps, lhs->getAnimFPS(mode, 0)));
         QVERIFY_TRUE( float_equiv(defaultFps, lhs->getAnimFPS(mode, 1)));

         lhs->setAnimFPS( mode, 0, 7.0 );
         rhs_2->setAnimFPS( mode, 0, 7.0 );
         rhs_3->setAnimFPS( mode, 0, 7.0 );
         lhs->operationComplete( "Set FPS" );

         QVERIFY_TRUE( float_equiv(7.0, lhs->getAnimFPS(mode, 0)));
         QVERIFY_TRUE( float_equiv(defaultFps, lhs->getAnimFPS(mode, 1)));

         lhs->setAnimFPS( mode, 1, 8.0 );
         rhs_3->setAnimFPS( mode, 1, 8.0 );
         lhs->operationComplete( "Set FPS" );

         QVERIFY_TRUE( float_equiv(7.0, lhs->getAnimFPS(mode, 0)));
         QVERIFY_TRUE( float_equiv(8.0, lhs->getAnimFPS(mode, 1)));

         checkUndoRedo( 3, lhs.get(), rhs_list );
      }
   }

   void testSkelAnimSetFrameCount()
   {
      local_ptr<Model> lhs = loadTestModel();
      local_ptr<Model> rhs_1 = loadTestModel();
      local_ptr<Model> rhs_2 = loadTestModel();
      local_ptr<Model> rhs_3 = loadTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );
      rhs_list.push_back( rhs_3.get() );

      const Model::AnimationModeE mode = Model::ANIMMODE_SKELETAL;

      const int animIndex = 1;

      QVERIFY_EQ( 20, (int) lhs->getAnimFrameCount( mode, 0 ) );
      QVERIFY_EQ( 8, (int) lhs->getAnimFrameCount( mode, 1 ) );

      const int reduceCount = 4;
      lhs->setAnimFrameCount( mode, animIndex, reduceCount );
      rhs_2->setAnimFrameCount( mode, animIndex, reduceCount );
      rhs_3->setAnimFrameCount( mode, animIndex, reduceCount );

      QVERIFY_EQ( 20, (int) lhs->getAnimFrameCount( mode, 0 ) );
      QVERIFY_EQ( reduceCount, (int) lhs->getAnimFrameCount( mode, 1 ) );

      lhs->operationComplete( "Reduce frame count" );

      const int increaseCount = 10;
      lhs->setAnimFrameCount( mode, animIndex, increaseCount );
      rhs_3->setAnimFrameCount( mode, animIndex, increaseCount );

      QVERIFY_EQ( 20, (int) lhs->getAnimFrameCount( mode, 0 ) );
      QVERIFY_EQ( increaseCount, (int) lhs->getAnimFrameCount( mode, 1 ) );

      lhs->operationComplete( "Increase frame count" );

      // Make sure increasing the frame count gave us blank frames
      // (instead of restoring deleted frames).
      const int bcount = lhs->getBoneJointCount();
      for ( int f = reduceCount; f < increaseCount; f++ )
      {
         for ( int b = 0; b < bcount; ++b )
         {
            QVERIFY_FALSE( lhs->hasSkelAnimKeyframe( animIndex, f, b, true ) );
            QVERIFY_FALSE( lhs->hasSkelAnimKeyframe( animIndex, f, b, false ) );
         }
      }

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testFrameAnimSetFrameCount()
   {
      local_ptr<Model> lhs = loadTestModel();
      local_ptr<Model> rhs_1 = loadTestModel();
      local_ptr<Model> rhs_2 = loadTestModel();
      local_ptr<Model> rhs_3 = loadTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );
      rhs_list.push_back( rhs_3.get() );

      const Model::AnimationModeE mode = Model::ANIMMODE_FRAME;

      const int animIndex = 1;

      QVERIFY_EQ( 20, (int) lhs->getAnimFrameCount( mode, 0 ) );
      QVERIFY_EQ( 8, (int) lhs->getAnimFrameCount( mode, 1 ) );

      const int reduceCount = 4;
      lhs->setAnimFrameCount( mode, animIndex, reduceCount );
      rhs_2->setAnimFrameCount( mode, animIndex, reduceCount );
      rhs_3->setAnimFrameCount( mode, animIndex, reduceCount );

      QVERIFY_EQ( 20, (int) lhs->getAnimFrameCount( mode, 0 ) );
      QVERIFY_EQ( reduceCount, (int) lhs->getAnimFrameCount( mode, 1 ) );

      lhs->operationComplete( "Reduce frame count" );

      const int increaseCount = 10;
      lhs->setAnimFrameCount( mode, animIndex, increaseCount );
      rhs_3->setAnimFrameCount( mode, animIndex, increaseCount );

      QVERIFY_EQ( 20, (int) lhs->getAnimFrameCount( mode, 0 ) );
      QVERIFY_EQ( increaseCount, (int) lhs->getAnimFrameCount( mode, 1 ) );

      lhs->operationComplete( "Increase frame count" );

      // Make sure increasing the frame count gave us blank frames
      // (instead of restoring deleted frames).
      double coords[3] = { 0, 0, 0 };
      double acoords[3] = { 0, 0, 0 };
      const int vcount = lhs->getVertexCount();
      for ( int f = reduceCount; f < increaseCount; f++ )
      {
         for ( int v = 0; v < vcount; ++v )
         {
            lhs->getVertexCoordsUnanimated( v, coords );
            lhs->getFrameAnimVertexCoords( animIndex, f, v,
                  acoords[0], acoords[1], acoords[2] );
            QVERIFY_ARRAY_EQ( coords, 3, acoords, 3 );
         }
      }

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testFrameAnimSetPosition()
   {
      // Vertex
      {
         local_ptr<Model> lhs = loadTestModel();
         local_ptr<Model> rhs_1 = loadTestModel();
         local_ptr<Model> rhs_2 = loadTestModel();

         ModelList rhs_list;
         rhs_list.push_back( rhs_1.get() );
         rhs_list.push_back( rhs_2.get() );

         const int animIndex = 1;
         const int animFrame = 2;
         const int animVertex = 3;

         lhs->setFrameAnimVertexCoords( animIndex, animFrame, animVertex,
               1.0, 2.0, 3.0 );
         rhs_2->setFrameAnimVertexCoords( animIndex, animFrame, animVertex,
               1.0, 2.0, 3.0 );

         lhs->operationComplete( "Move vertex" );

         checkUndoRedo( 1, lhs.get(), rhs_list );
      }

      // Point Translation
      {
         local_ptr<Model> lhs = loadTestModel();
         local_ptr<Model> rhs_1 = loadTestModel();
         local_ptr<Model> rhs_2 = loadTestModel();

         ModelList rhs_list;
         rhs_list.push_back( rhs_1.get() );
         rhs_list.push_back( rhs_2.get() );

         const int animIndex = 1;
         const int animFrame = 2;
         const int animPoint = 0;

         lhs->setFrameAnimPointCoords( animIndex, animFrame, animPoint,
               1.0, 2.0, 3.0 );
         rhs_2->setFrameAnimPointCoords( animIndex, animFrame, animPoint,
               1.0, 2.0, 3.0 );

         lhs->operationComplete( "Move point" );

         checkUndoRedo( 1, lhs.get(), rhs_list );
      }

      // Point Rotation
      {
         local_ptr<Model> lhs = loadTestModel();
         local_ptr<Model> rhs_1 = loadTestModel();
         local_ptr<Model> rhs_2 = loadTestModel();

         ModelList rhs_list;
         rhs_list.push_back( rhs_1.get() );
         rhs_list.push_back( rhs_2.get() );

         const int animIndex = 1;
         const int animFrame = 2;
         const int animPoint = 0;

         lhs->setFrameAnimPointRotation( animIndex, animFrame, animPoint,
               1.0, 2.0, 3.0 );
         rhs_2->setFrameAnimPointRotation( animIndex, animFrame, animPoint,
               1.0, 2.0, 3.0 );

         lhs->operationComplete( "Rotate point" );

         checkUndoRedo( 1, lhs.get(), rhs_list );
      }
   }

   // FIXME add tests:
   //  X add animation
   //  X delete animation
   //  x set anim name
   //  x set anim frame count
   //     x increase
   //     x decrease
   //     x get
   //  x set anim fps
   //     - with keyframes (check frame time adjusted)
   //       get
   //  ? set frame anim point count
   //  ? set frame anim vertex count
   //  - set frame anim point coords
   //     x with point
   //     ? without point
   //       get frame anim point coords
   //  - set frame anim point rot
   //     x with point
   //     ? without point
   //       get frame anim point rot
   //  - set frame anim vertex coords
   //     x with vertex
   //     ? without vertex
   //       get
   //       get normals
   //  - set quick frame anim vertex coords
   //       with vertex
   //     - without vertex (test with undo !?)
   //  - copy animation
   //  - split animation
   //  - join animations
   //  - merge animations
   //  - move animation
   //  - convert anim to frame
   //  - clear anim frame
   //    set skel anim keyframe
   //       add new keyframe (trans & rot)
   //       update existing keyframe (trans & rot)
   //       hasSkelAnimKeyframe
   //       getSkelAnimKeyframe
   //       interpSkelAnimKeyframe
   //       interpSkelAnimKeyframeTime
   //          with looping
   //        - without looping
   //  - delete skel anim keyframe
   //    insert frame anim frame
   //     - not at end of list
   //  - set current animation by name
   //    set current animation by index
   //       combine name + index (is name even used?)
   //       get
   //  - set no animation
   //    set current animation frame
   //       get
   //    set current animation time
   //       get
   //  - set/get looping
   //    setup joints
   //  x get anim count
   //  x get anim name
   //    undo

};

QTEST_MAIN(ModelAnimTest)
#include "model_anim_test.moc"

