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


// This file tests material methods in the Model class.

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


class ModelGroupTest : public QObject
{
   Q_OBJECT
private:

   void checkGroupContents( std::list<int> & lhs, Model * m, int grp )
   {
      std::list<int> rhs;
      if ( grp >= 0 )
         rhs = m->getGroupTriangles( grp );
      else
         rhs = m->getUngroupedTriangles();
      lhs.sort();
      rhs.sort();

      QVERIFY_TRUE( lhs == rhs );

      std::list<int>::const_iterator it = lhs.begin();
      int tcount = m->getTriangleCount();
      for ( int t = 0; t < tcount; ++t )
      {
         if ( it != lhs.end() && *it == t )
         {
            QVERIFY_EQ( grp, m->getTriangleGroup( *it ) );
            ++it;
         }
         else
         {
            QVERIFY_NE( grp, m->getTriangleGroup( t ) );
         }
      }
   }

   void addTriangleTest( std::list<int> & lhs, Model * m, int grp, int tri )
   {
      m->addTriangleToGroup( grp, tri );
      lhs.push_back( tri );
      checkGroupContents( lhs, m, grp );
   }

   void removeTriangleTest( std::list<int> & lhs, Model * m, int grp, int tri )
   {
      m->removeTriangleFromGroup( grp, tri );
      lhs.remove( tri );
      checkGroupContents( lhs, m, grp );
      QVERIFY_NE( grp, m->getTriangleGroup( tri ) );
   }

   void removeAll( Model * m, int grp )
   {
      list<int> tris = m->getGroupTriangles( grp );
      while ( !tris.empty() )
      {
         m->removeTriangleFromGroup( grp, tris.front() );
         tris.pop_front();
      }
   }

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   // FIXME add more tests

   void testGroupName()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_firstname = newTestModel();
      local_ptr<Model> rhs_secondname = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_firstname.get() );
      rhs_list.push_back( rhs_secondname.get() );

      lhs->addGroup( "Group A" );
      lhs->addGroup( "First Name" );
      lhs->addGroup( "Group B" );
      rhs_firstname->addGroup( "Group A" );
      rhs_firstname->addGroup( "First Name" );
      rhs_firstname->addGroup( "Group B" );
      rhs_secondname->addGroup( "Group A" );
      rhs_secondname->addGroup( "Second Name" );
      rhs_secondname->addGroup( "Group B" );

      QVERIFY_EQ( std::string("Group A"), std::string(lhs->getGroupName(0)));
      QVERIFY_EQ( std::string("First Name"), std::string(lhs->getGroupName(1)));
      QVERIFY_EQ( std::string("Group B"), std::string(lhs->getGroupName(2)));
      QVERIFY_EQ( 0, lhs->getGroupByName("Group A"));
      QVERIFY_EQ( 1, lhs->getGroupByName("First Name"));
      QVERIFY_EQ( 2, lhs->getGroupByName("Group B"));
      QVERIFY_EQ( -1, lhs->getGroupByName("Second Name"));

      lhs->operationComplete( "Add groups" );

      lhs->setGroupName(1, "Second Name" );

      QVERIFY_EQ( std::string("Group A"), std::string(lhs->getGroupName(0)));
      QVERIFY_EQ( std::string("Second Name"), std::string(lhs->getGroupName(1)));
      QVERIFY_EQ( std::string("Group B"), std::string(lhs->getGroupName(2)));
      QVERIFY_EQ( 0, lhs->getGroupByName("Group A"));
      QVERIFY_EQ( 1, lhs->getGroupByName("Second Name"));
      QVERIFY_EQ( 2, lhs->getGroupByName("Group B"));
      QVERIFY_EQ( -1, lhs->getGroupByName("First Name"));

      lhs->operationComplete( "Set group name" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testGroupAngle()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_first = newTestModel();
      local_ptr<Model> rhs_second = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_first.get() );
      rhs_list.push_back( rhs_second.get() );

      lhs->addGroup( "Group A" );
      lhs->addGroup( "Group B" );
      lhs->addGroup( "Group C" );
      rhs_first->addGroup( "Group A" );
      rhs_first->addGroup( "Group B" );
      rhs_first->addGroup( "Group C" );
      rhs_second->addGroup( "Group A" );
      rhs_second->addGroup( "Group B" );
      rhs_second->addGroup( "Group C" );

      lhs->setGroupSmooth( 0, 10 );
      lhs->setGroupSmooth( 1, 20 );
      lhs->setGroupSmooth( 2, 30 );
      rhs_first->setGroupSmooth( 0, 10 );
      rhs_first->setGroupSmooth( 1, 20 );
      rhs_first->setGroupSmooth( 2, 30 );
      rhs_second->setGroupSmooth( 0, 10 );
      rhs_second->setGroupSmooth( 1, 80 );
      rhs_second->setGroupSmooth( 2, 30 );

      QVERIFY_EQ( (uint8_t) 10, lhs->getGroupSmooth(0));
      QVERIFY_EQ( (uint8_t) 20, lhs->getGroupSmooth(1));
      QVERIFY_EQ( (uint8_t) 30, lhs->getGroupSmooth(2));

      lhs->operationComplete( "Add groups" );

      lhs->setGroupSmooth(1, 80 );

      QVERIFY_EQ( (uint8_t) 10, lhs->getGroupSmooth(0));
      QVERIFY_EQ( (uint8_t) 80, lhs->getGroupSmooth(1));
      QVERIFY_EQ( (uint8_t) 30, lhs->getGroupSmooth(2));

      lhs->operationComplete( "Set group smoothness" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testAddRemove()
   {
      local_ptr<Model> m = loadModelOrDie( "data/model_hidden_test.mm3d" );

      m->unhideAll();
      int grp = m->addGroup( "New Group" );

      std::list<int> lhs;

      // Should be empty
      QVERIFY( lhs == m->getGroupTriangles( grp ) );

      addTriangleTest( lhs, m.get(), grp, 0 );
      addTriangleTest( lhs, m.get(), grp, 1 );
      addTriangleTest( lhs, m.get(), grp, 2 );

      removeTriangleTest( lhs, m.get(), grp, 2 );
      removeTriangleTest( lhs, m.get(), grp, 1 );
      removeTriangleTest( lhs, m.get(), grp, 0 );

      addTriangleTest( lhs, m.get(), grp, 0 );
      addTriangleTest( lhs, m.get(), grp, 1 );
      addTriangleTest( lhs, m.get(), grp, 2 );

      removeTriangleTest( lhs, m.get(), grp, 0 );
      removeTriangleTest( lhs, m.get(), grp, 1 );
      removeTriangleTest( lhs, m.get(), grp, 2 );

      addTriangleTest( lhs, m.get(), grp, 2 );
      addTriangleTest( lhs, m.get(), grp, 1 );
      addTriangleTest( lhs, m.get(), grp, 0 );

      removeTriangleTest( lhs, m.get(), grp, 0 );
      removeTriangleTest( lhs, m.get(), grp, 1 );
      removeTriangleTest( lhs, m.get(), grp, 2 );

      addTriangleTest( lhs, m.get(), grp, 2 );
      addTriangleTest( lhs, m.get(), grp, 1 );
      addTriangleTest( lhs, m.get(), grp, 0 );

      removeTriangleTest( lhs, m.get(), grp, 2 );
      removeTriangleTest( lhs, m.get(), grp, 1 );
      removeTriangleTest( lhs, m.get(), grp, 0 );

      addTriangleTest( lhs, m.get(), grp, 0 );
      addTriangleTest( lhs, m.get(), grp, 1 );
      addTriangleTest( lhs, m.get(), grp, 2 );

      removeTriangleTest( lhs, m.get(), grp, 1 );
      removeTriangleTest( lhs, m.get(), grp, 0 );
      removeTriangleTest( lhs, m.get(), grp, 2 );

      addTriangleTest( lhs, m.get(), grp, 0 );
      addTriangleTest( lhs, m.get(), grp, 1 );
      addTriangleTest( lhs, m.get(), grp, 2 );

      removeTriangleTest( lhs, m.get(), grp, 1 );
      removeTriangleTest( lhs, m.get(), grp, 2 );
      removeTriangleTest( lhs, m.get(), grp, 0 );
   }

   void testAddSelected()
   {
      local_ptr<Model> m = newTestModel();

      for ( int t = 0; t < 12; ++t )
      {
         m->addVertex( (double) t, (double) t + 1, (double) t - 1 );
      }
      for ( int t = 0; t < 10; ++t )
      {
         m->addTriangle( t, t + 1, t + 2 );
      }

      int grp = m->addGroup( "New Group" );

      std::list<int> lhs;

      // Should be empty
      QVERIFY( lhs == m->getGroupTriangles( grp ) );
      lhs.clear();
      lhs.push_back(0);
      lhs.push_back(1);
      lhs.push_back(2);
      lhs.push_back(3);
      lhs.push_back(4);
      lhs.push_back(5);
      lhs.push_back(6);
      lhs.push_back(7);
      lhs.push_back(8);
      lhs.push_back(9);
      checkGroupContents( lhs, m.get(), -1 );

      // Test addSelectedToGroup()
      m->selectTriangle( 0 );
      m->selectTriangle( 1 );
      m->selectTriangle( 2 );
      m->selectTriangle( 3 );
      lhs.clear();
      lhs.push_back( 0 );
      lhs.push_back( 1 );
      lhs.push_back( 2 );
      lhs.push_back( 3 );
      m->addSelectedToGroup( grp );
      checkGroupContents( lhs, m.get(), grp );
      lhs.clear();
      lhs.push_back(4);
      lhs.push_back(5);
      lhs.push_back(6);
      lhs.push_back(7);
      lhs.push_back(8);
      lhs.push_back(9);
      checkGroupContents( lhs, m.get(), -1 );
      m->unselectAll();

      // Test addSelectedToGroup() does not duplicate triangles in group
      m->selectTriangle( 0 );
      m->selectTriangle( 3 );
      m->selectTriangle( 4 );
      m->selectTriangle( 5 );
      lhs.clear();
      lhs.push_back( 0 );
      lhs.push_back( 1 );
      lhs.push_back( 2 );
      lhs.push_back( 3 );
      lhs.push_back( 4 );
      lhs.push_back( 5 );
      m->addSelectedToGroup( grp );
      checkGroupContents( lhs, m.get(), grp );
      lhs.clear();
      lhs.push_back(6);
      lhs.push_back(7);
      lhs.push_back(8);
      lhs.push_back(9);
      checkGroupContents( lhs, m.get(), -1 );
      m->unselectAll();

      // Test setSelectedAsGroup(), including removal of triangles that
      // are not in the new set and inclusion of triangles that were
      // in the group originally.
      m->selectTriangle( 1 );
      m->selectTriangle( 3 );
      m->selectTriangle( 5 );
      m->selectTriangle( 7 );
      m->selectTriangle( 9 );
      lhs.clear();
      lhs.push_back( 1 );
      lhs.push_back( 3 );
      lhs.push_back( 5 );
      lhs.push_back( 7 );
      lhs.push_back( 9 );
      m->setSelectedAsGroup( grp );
      checkGroupContents( lhs, m.get(), grp );
      lhs.clear();
      lhs.push_back(0);
      lhs.push_back(2);
      lhs.push_back(4);
      lhs.push_back(6);
      lhs.push_back(8);
      checkGroupContents( lhs, m.get(), -1 );
      m->unselectAll();

      // Test that addSelectedToGroup removes triangles from another group
      int grp2 = m->addGroup( "New Group 2" );
      m->selectTriangle( 3 );
      m->selectTriangle( 5 );
      m->addSelectedToGroup( grp2 );
      lhs.clear();
      lhs.push_back( 1 );
      lhs.push_back( 7 );
      lhs.push_back( 9 );
      checkGroupContents( lhs, m.get(), grp );
      lhs.clear();
      lhs.push_back( 3 );
      lhs.push_back( 5 );
      checkGroupContents( lhs, m.get(), grp2 );
      lhs.clear();
      lhs.push_back(0);
      lhs.push_back(2);
      lhs.push_back(4);
      lhs.push_back(6);
      lhs.push_back(8);
      checkGroupContents( lhs, m.get(), -1 );
      m->unselectAll();

      // Test that setSelecedAsGroup removes triangles from another group
      m->selectTriangle( 1 );
      m->setSelectedAsGroup( grp2 );
      lhs.clear();
      lhs.push_back( 1 );
      checkGroupContents( lhs, m.get(), grp2 );
      lhs.clear();
      lhs.push_back( 7 );
      lhs.push_back( 9 );
      checkGroupContents( lhs, m.get(), grp );
      lhs.clear();
      lhs.push_back(0);
      lhs.push_back(2);
      lhs.push_back(3);
      lhs.push_back(4);
      lhs.push_back(5);
      lhs.push_back(6);
      lhs.push_back(8);
      checkGroupContents( lhs, m.get(), -1 );
      m->unselectAll();

      // FIXME test undo
   }

   // FIXME add tests:
   //   deletion preserves triangle indices in group membership set
   //   normal blending
   //     smoothness
   //     max angle
   //   undo

};

QTEST_MAIN(ModelGroupTest)
#include "model_group_test.moc"

