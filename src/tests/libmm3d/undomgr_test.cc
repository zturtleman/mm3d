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


// This file tests local_ptr.h, local_array.h, release_ptr.h,
// and file_closer.h

#include <QtTest/QtTest>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "test_common.h"

#include "log.h"

#include "undomgr.h"
#include "undo.h"

class TestUndo : public Undo
{
   public:
      TestUndo()
         : m_size( 0 ),
           m_useBaseSize( true ),
           m_undoReleaseCalled( false ),
           m_redoReleaseCalled( false ),
           m_releaseCalled( false ),
           m_canCombine( false ) { }

      virtual ~TestUndo() { }

      bool combine( Undo * u )
      {
         TestUndo * undo = static_cast<TestUndo*>(u);
         return m_canCombine && undo->m_canCombine;
      }

      void undoRelease() { m_undoReleaseCalled = true; }
      void redoRelease() { m_redoReleaseCalled = true; }
      void release() { m_releaseCalled = true; }

      unsigned size() { return m_size + (m_useBaseSize ? Undo::size() : 0); }

      unsigned int m_size;
      bool m_useBaseSize;
      bool m_undoReleaseCalled;
      bool m_redoReleaseCalled;
      bool m_releaseCalled;
      bool m_canCombine;
};

class UndoMgrTest : public QObject
{
   Q_OBJECT
private slots:
   void initTestCase()
   {
      log_enable_debug( false );
   }

   void testBasicUndoTest()
   {
      TestUndo u1;
      TestUndo u2;
      TestUndo u3;

      UndoManager mgr;

      QVERIFY_FALSE( mgr.canUndo() );
      QVERIFY_FALSE( mgr.canRedo() );

      mgr.addUndo( &u1 );
      mgr.operationComplete( "Op1" );

      QVERIFY_TRUE( mgr.canUndo() );
      QVERIFY_FALSE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op1" ), std::string( mgr.getUndoOpName() ) );

      mgr.addUndo( &u2 );
      mgr.operationComplete( "Op2" );

      QVERIFY_TRUE( mgr.canUndo() );
      QVERIFY_FALSE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op2" ), std::string( mgr.getUndoOpName() ) );

      mgr.addUndo( &u3 );
      mgr.operationComplete( "Op3" );

      QVERIFY_TRUE( mgr.canUndo() );
      QVERIFY_FALSE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op3" ), std::string( mgr.getUndoOpName() ) );

      QVERIFY_TRUE( NULL != mgr.undo() );

      QVERIFY_TRUE( mgr.canUndo() );
      QVERIFY_TRUE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op2" ), std::string( mgr.getUndoOpName() ) );
      QVERIFY_EQ( std::string( "Op3" ), std::string( mgr.getRedoOpName() ) );

      QVERIFY_TRUE( NULL != mgr.undo() );

      QVERIFY_TRUE( mgr.canUndo() );
      QVERIFY_TRUE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op1" ), std::string( mgr.getUndoOpName() ) );
      QVERIFY_EQ( std::string( "Op2" ), std::string( mgr.getRedoOpName() ) );

      QVERIFY_TRUE( NULL != mgr.undo() );

      QVERIFY_FALSE( mgr.canUndo() );
      QVERIFY_TRUE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op1" ), std::string( mgr.getRedoOpName() ) );

      QVERIFY_TRUE( NULL == mgr.undo() );

      QVERIFY_TRUE( NULL != mgr.redo() );

      QVERIFY_TRUE( mgr.canUndo() );
      QVERIFY_TRUE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op1" ), std::string( mgr.getUndoOpName() ) );
      QVERIFY_EQ( std::string( "Op2" ), std::string( mgr.getRedoOpName() ) );

      QVERIFY_TRUE( NULL != mgr.redo() );

      QVERIFY_TRUE( mgr.canUndo() );
      QVERIFY_TRUE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op2" ), std::string( mgr.getUndoOpName() ) );
      QVERIFY_EQ( std::string( "Op3" ), std::string( mgr.getRedoOpName() ) );

      QVERIFY_TRUE( NULL != mgr.redo() );

      QVERIFY_TRUE( mgr.canUndo() );
      QVERIFY_FALSE( mgr.canRedo() );
      QVERIFY_EQ( std::string( "Op3" ), std::string( mgr.getUndoOpName() ) );

      QVERIFY_TRUE( NULL == mgr.redo() );

      mgr.clear();

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_TRUE( u3.m_releaseCalled );
   }

   void testUndoRelease()
   {
      TestUndo u1;
      TestUndo u2;

      UndoManager mgr;

      mgr.addUndo( &u1 );
      mgr.operationComplete( "Op1" );
      mgr.addUndo( &u2 );
      mgr.operationComplete( "Op2" );

      mgr.undo();
      mgr.redo();
      mgr.undo();

      mgr.clear();

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_TRUE( u1.m_undoReleaseCalled );
      QVERIFY_TRUE( u2.m_redoReleaseCalled );
   }

   void testUndoCombine()
   {
      TestUndo u1;
      TestUndo u2;
      TestUndo u3;
      TestUndo u4;

      u3.m_canCombine = true;
      u4.m_canCombine = true;

      UndoManager mgr;

      mgr.addUndo( &u1 );
      mgr.addUndo( &u2 );
      mgr.addUndo( &u3 );
      mgr.addUndo( &u4 );
      mgr.operationComplete( "Op1" );

      UndoList * ul = mgr.undo();
      QVERIFY_EQ( 3, (int) ul->size() );
      QVERIFY_TRUE( NULL == mgr.undo() );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_TRUE( u4.m_releaseCalled );

      mgr.clear();

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_TRUE( u3.m_releaseCalled );
      QVERIFY_TRUE( u4.m_releaseCalled );
   }

   void testUndoNoCombine()
   {
      TestUndo u1;
      TestUndo u2;

      // Combine fails because they are separate operations
      u1.m_canCombine = true;
      u2.m_canCombine = true;

      UndoManager mgr;

      mgr.addUndo( &u1 );
      mgr.operationComplete( "Op1" );
      mgr.addUndo( &u2 );
      mgr.operationComplete( "Op2" );

      UndoList * ul;
      ul = mgr.undo();
      QVERIFY_EQ( 1, (int) ul->size() );
      ul = mgr.undo();
      QVERIFY_EQ( 1, (int) ul->size() );
      QVERIFY_TRUE( NULL == mgr.undo() );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );

      mgr.clear();

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
   }

   void testOperationCompleteWithoutOp()
   {
      TestUndo u1;
      TestUndo u2;

      UndoManager mgr;

      mgr.addUndo( &u1 );
      mgr.operationComplete( "Op1" );
      mgr.operationComplete( "No Op" );
      mgr.addUndo( &u2 );
      mgr.operationComplete( "Op2" );

      UndoList * ul;

      QVERIFY_EQ( std::string( "Op2" ), std::string( mgr.getUndoOpName() ) );
      ul = mgr.undo();
      QVERIFY_EQ( 1, (int) ul->size() );

      QVERIFY_EQ( std::string( "Op1" ), std::string( mgr.getUndoOpName() ) );
      ul = mgr.undo();
      QVERIFY_EQ( 1, (int) ul->size() );

      QVERIFY_FALSE( mgr.canUndo() );
      QVERIFY_TRUE( NULL == mgr.undo() );
   }

   void testUndoOperationComplete()
   {
      TestUndo u1;
      TestUndo u2;
      TestUndo u3;

      UndoManager mgr;

      mgr.addUndo( &u1 );
      mgr.operationComplete( "Op1" );
      mgr.addUndo( &u2 );
      mgr.operationComplete( "Op2" );
      QVERIFY_TRUE( NULL != mgr.undo() );
      mgr.addUndo( &u3 );
      mgr.operationComplete( "Op3" );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );

      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_TRUE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );

      QVERIFY_FALSE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );

      mgr.clear();

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_TRUE( u3.m_releaseCalled );

      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_TRUE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );

      QVERIFY_TRUE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_TRUE( u3.m_undoReleaseCalled );
   }

   void testSizeLimit()
   {
      TestUndo u1;
      TestUndo u2;
      TestUndo u3;
      TestUndo u4;
      TestUndo u5;

      u1.m_size = 10;
      u1.m_useBaseSize = false;
      u2.m_size = 10;
      u2.m_useBaseSize = false;
      u3.m_size = 10;
      u3.m_useBaseSize = false;
      u4.m_size = 10;
      u4.m_useBaseSize = false;
      u5.m_size = 10;
      u5.m_useBaseSize = false;

      UndoManager mgr;

      mgr.setSizeLimit( 35 );

      mgr.addUndo( &u1 );
      mgr.operationComplete( "Op1" );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_FALSE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.addUndo( &u2 );
      mgr.operationComplete( "Op2" );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_FALSE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.addUndo( &u3 );
      mgr.operationComplete( "Op3" );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_FALSE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.addUndo( &u4 );
      mgr.operationComplete( "Op4" );

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_TRUE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.addUndo( &u5 );
      mgr.operationComplete( "Op5" );

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_TRUE( u1.m_undoReleaseCalled );
      QVERIFY_TRUE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.clear();

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_TRUE( u3.m_releaseCalled );
      QVERIFY_TRUE( u4.m_releaseCalled );
      QVERIFY_TRUE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_TRUE( u1.m_undoReleaseCalled );
      QVERIFY_TRUE( u2.m_undoReleaseCalled );
      QVERIFY_TRUE( u3.m_undoReleaseCalled );
      QVERIFY_TRUE( u4.m_undoReleaseCalled );
      QVERIFY_TRUE( u5.m_undoReleaseCalled );
   }

   void testCountLimit()
   {
      TestUndo u1;
      TestUndo u2;
      TestUndo u3;
      TestUndo u4;
      TestUndo u5;

      UndoManager mgr;

      mgr.setCountLimit( 3 );

      mgr.addUndo( &u1 );
      mgr.operationComplete( "Op1" );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_FALSE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.addUndo( &u2 );
      mgr.operationComplete( "Op2" );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_FALSE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.addUndo( &u3 );
      mgr.operationComplete( "Op3" );

      QVERIFY_FALSE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_FALSE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.addUndo( &u4 );
      mgr.operationComplete( "Op4" );

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_FALSE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_TRUE( u1.m_undoReleaseCalled );
      QVERIFY_FALSE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.addUndo( &u5 );
      mgr.operationComplete( "Op5" );

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_FALSE( u3.m_releaseCalled );
      QVERIFY_FALSE( u4.m_releaseCalled );
      QVERIFY_FALSE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_TRUE( u1.m_undoReleaseCalled );
      QVERIFY_TRUE( u2.m_undoReleaseCalled );
      QVERIFY_FALSE( u3.m_undoReleaseCalled );
      QVERIFY_FALSE( u4.m_undoReleaseCalled );
      QVERIFY_FALSE( u5.m_undoReleaseCalled );

      mgr.clear();

      QVERIFY_TRUE( u1.m_releaseCalled );
      QVERIFY_TRUE( u2.m_releaseCalled );
      QVERIFY_TRUE( u3.m_releaseCalled );
      QVERIFY_TRUE( u4.m_releaseCalled );
      QVERIFY_TRUE( u5.m_releaseCalled );
      QVERIFY_FALSE( u1.m_redoReleaseCalled );
      QVERIFY_FALSE( u2.m_redoReleaseCalled );
      QVERIFY_FALSE( u3.m_redoReleaseCalled );
      QVERIFY_FALSE( u4.m_redoReleaseCalled );
      QVERIFY_FALSE( u5.m_redoReleaseCalled );
      QVERIFY_TRUE( u1.m_undoReleaseCalled );
      QVERIFY_TRUE( u2.m_undoReleaseCalled );
      QVERIFY_TRUE( u3.m_undoReleaseCalled );
      QVERIFY_TRUE( u4.m_undoReleaseCalled );
      QVERIFY_TRUE( u5.m_undoReleaseCalled );
   }

   void testUndoCurrent()
   {
      {
         TestUndo u1;

         UndoManager mgr;

         mgr.addUndo( &u1 );
         mgr.operationComplete( "Op1" );
         QVERIFY_TRUE( NULL == mgr.undoCurrent() );
         QVERIFY_TRUE( mgr.canUndo() );
         QVERIFY_FALSE( mgr.canRedo() );
      }

      {
         TestUndo u1;
         TestUndo u2;

         UndoManager mgr;

         mgr.addUndo( &u1 );
         mgr.operationComplete( "Op1" );
         mgr.addUndo( &u2 );
         QVERIFY_TRUE( NULL != mgr.undoCurrent() );
         QVERIFY_TRUE( mgr.canUndo() );
         QVERIFY_TRUE( mgr.canRedo() );

         mgr.clear();

         QVERIFY_TRUE( u1.m_releaseCalled );
         QVERIFY_TRUE( u1.m_undoReleaseCalled );
         QVERIFY_FALSE( u1.m_redoReleaseCalled );
         QVERIFY_TRUE( u2.m_releaseCalled );
         QVERIFY_FALSE( u2.m_undoReleaseCalled );
         QVERIFY_TRUE( u2.m_redoReleaseCalled );
      }

      {
         TestUndo u1;
         TestUndo u2;

         UndoManager mgr;

         mgr.addUndo( &u1 );
         mgr.operationComplete( "Op1" );
         mgr.addUndo( &u2 );
         QVERIFY_TRUE( NULL != mgr.undoCurrent() );
         QVERIFY_TRUE( NULL != mgr.redo() );
         QVERIFY_TRUE( mgr.canUndo() );
         QVERIFY_FALSE( mgr.canRedo() );

         mgr.clear();

         QVERIFY_TRUE( u1.m_releaseCalled );
         QVERIFY_TRUE( u1.m_undoReleaseCalled );
         QVERIFY_FALSE( u1.m_redoReleaseCalled );
         QVERIFY_TRUE( u2.m_releaseCalled );
         QVERIFY_TRUE( u2.m_undoReleaseCalled );
         QVERIFY_FALSE( u2.m_redoReleaseCalled );
      }
   }

   void testListCombine()
   {
      // Without list combine
      {
         TestUndo u1;
         TestUndo u2;
         TestUndo u3;
         TestUndo u4;

         u2.m_canCombine = true;
         u4.m_canCombine = true;

         UndoManager mgr;

         mgr.addUndo( &u1 );
         mgr.addUndo( &u2 );
         mgr.addUndo( &u3 );
         mgr.addUndo( &u4 );
         mgr.operationComplete( "Op1" );

         UndoList * ul = mgr.undo();
         QVERIFY_EQ( 4, (int) ul->size() );
         QVERIFY_TRUE( NULL == mgr.undo() );

         QVERIFY_FALSE( u1.m_releaseCalled );
         QVERIFY_FALSE( u2.m_releaseCalled );
         QVERIFY_FALSE( u3.m_releaseCalled );
         QVERIFY_FALSE( u4.m_releaseCalled );

         mgr.clear();

         QVERIFY_TRUE( u1.m_releaseCalled );
         QVERIFY_TRUE( u2.m_releaseCalled );
         QVERIFY_TRUE( u3.m_releaseCalled );
         QVERIFY_TRUE( u4.m_releaseCalled );
      }

      // With list combine
      {
         TestUndo u1;
         TestUndo u2;
         TestUndo u3;
         TestUndo u4;

         u2.m_canCombine = true;
         u4.m_canCombine = true;

         UndoManager mgr;

         mgr.addUndo( &u1, true );
         mgr.addUndo( &u2, true );
         mgr.addUndo( &u3, true );
         mgr.addUndo( &u4, true );
         mgr.operationComplete( "Op1" );

         UndoList * ul = mgr.undo();
         QVERIFY_EQ( 3, (int) ul->size() );
         QVERIFY_TRUE( NULL == mgr.undo() );

         QVERIFY_FALSE( u1.m_releaseCalled );
         QVERIFY_FALSE( u2.m_releaseCalled );
         QVERIFY_FALSE( u3.m_releaseCalled );
         QVERIFY_TRUE( u4.m_releaseCalled );

         mgr.clear();

         QVERIFY_TRUE( u1.m_releaseCalled );
         QVERIFY_TRUE( u2.m_releaseCalled );
         QVERIFY_TRUE( u3.m_releaseCalled );
         QVERIFY_TRUE( u4.m_releaseCalled );
      }
   }

   void testSetSaved()
   {
      TestUndo u1;
      TestUndo u2;
      TestUndo u3;

      UndoManager mgr;

      mgr.setSaved();

      mgr.addUndo( &u1 );
      mgr.operationComplete( "Op1" );
      mgr.addUndo( &u2 );
      mgr.operationComplete( "Op2" );
      mgr.addUndo( &u3 );
      mgr.operationComplete( "Op3" );

      // Save undo = 0
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.undo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.undo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.undo();
      QVERIFY_TRUE( mgr.isSaved() );

      // Save undo = 1
      mgr.redo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.setSaved();
      QVERIFY_TRUE( mgr.isSaved() );
      mgr.redo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.redo();
      QVERIFY_FALSE( mgr.isSaved() );

      // Save undo = 2
      mgr.undo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.setSaved();
      QVERIFY_TRUE( mgr.isSaved() );
      mgr.undo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.undo();
      QVERIFY_FALSE( mgr.isSaved() );

      mgr.redo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.redo();
      QVERIFY_TRUE( mgr.isSaved() );
      mgr.redo();
      QVERIFY_FALSE( mgr.isSaved() );

      // Save undo = 3
      mgr.setSaved();
      QVERIFY_TRUE( mgr.isSaved() );
      mgr.undo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.undo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.undo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.redo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.redo();
      QVERIFY_FALSE( mgr.isSaved() );
      mgr.redo();
      QVERIFY_TRUE( mgr.isSaved() );
   }

   // Cover last of the undomgr.cc file
   void testOpNameTest()
   {
      TestUndo u1;
      UndoManager mgr;
      QVERIFY_EQ(std::string(""), std::string(mgr.getUndoOpName()));
      QVERIFY_EQ(std::string(""), std::string(mgr.getRedoOpName()));

      // Don't call operation complete here
      mgr.addUndo( &u1 );
      QVERIFY_EQ(std::string(""), std::string(mgr.getUndoOpName()));
   }
};

QTEST_MAIN(UndoMgrTest)
#include "undomgr_test.moc"
