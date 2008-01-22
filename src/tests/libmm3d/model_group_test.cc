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


// This file tests grouping methods in the Model class.

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

class ModelSelectTest : public QObject
{
   Q_OBJECT
private:

   void checkGroupContents( std::list<int> & lhs, Model * m, int grp )
   {
      std::list<int> rhs = m->getGroupTriangles( grp );
      lhs.sort();
      rhs.sort();
      QVERIFY( lhs == rhs );

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
      local_ptr<Model> m = loadModelOrDie( "data/model_hidden_test.mm3d" );

      m->unhideAll();
      int grp = m->addGroup( "New Group" );

      std::list<int> lhs;

      // Should be empty
      QVERIFY( lhs == m->getGroupTriangles( grp ) );

      // Test addSelectedToGroup()
      m->selectTriangle( 0 );
      m->selectTriangle( 1 );
      m->selectTriangle( 2 );
      m->selectTriangle( 3 );
      lhs.push_back( 0 );
      lhs.push_back( 1 );
      lhs.push_back( 2 );
      lhs.push_back( 3 );
      m->addSelectedToGroup( grp );
      checkGroupContents( lhs, m.get(), grp );
      m->unselectAll();

      // Test addSelectedToGroup() does not duplicate triangles in group
      m->selectTriangle( 0 );
      m->selectTriangle( 3 );
      m->selectTriangle( 4 );
      m->selectTriangle( 5 );
      lhs.push_back( 4 );
      lhs.push_back( 5 );
      m->addSelectedToGroup( grp );
      checkGroupContents( lhs, m.get(), grp );
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
      m->unselectAll();
   }

   // FIXME undo
   // FIXME deletion preserves triangle indices

};

QTEST_MAIN(ModelSelectTest)
#include "model_group_test.moc"

