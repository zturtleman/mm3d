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


// This file tests initialization of the model compenents (inner classes).

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "local_array.h"

class ModelInitTest : public QObject
{
   Q_OBJECT
private:
   void vertexIsInitialized( Model::Vertex * v )
   {
      QVERIFY_TRUE( v->m_visible );
      QVERIFY_FALSE( v->m_selected );
      QVERIFY_FALSE( v->m_free );
      QVERIFY( v->m_drawSource == v->m_coord );
   }

   void triangleIsInitialized( Model::Triangle * t )
   {
      QVERIFY_TRUE( t->m_visible );
      QVERIFY_FALSE( t->m_selected );
      QVERIFY_EQ( -1, t->m_projection );
      // FIXME need fuzzy EQ for floats
      QVERIFY( t->m_flatSource == t->m_flatNormals );
      QVERIFY( t->m_normalSource[0] == t->m_finalNormals[0] );
      QVERIFY( t->m_normalSource[1] == t->m_finalNormals[1] );
      QVERIFY( t->m_normalSource[2] == t->m_finalNormals[2] );
   }

   void groupIsInitialized( Model::Group * g )
   {
      QVERIFY_EQ( std::string(""), g->m_name );
      QVERIFY_EQ( -1, g->m_materialIndex );
      QVERIFY_EQ( (size_t) 0, g->m_triangleIndices.size() );
      QVERIFY_EQ( (uint8_t) 255, g->m_smooth );
      QVERIFY_EQ( (uint8_t) 89, g->m_angle );
      QVERIFY_TRUE( g->m_visible );
      QVERIFY_FALSE( g->m_selected );
   }

   void materialIsInitialized( Model::Material * m )
   {
      QVERIFY_EQ( std::string(""), m->m_name );
      QVERIFY_EQ( Model::Material::MATTYPE_TEXTURE, m->m_type );
      // FIXME need fuzzy EQ for floats
      QVERIFY_FALSE( m->m_sClamp );
      QVERIFY_FALSE( m->m_tClamp );
      QVERIFY_EQ( (GLuint) 0, m->m_texture );
      QVERIFY_EQ( std::string(""), m->m_filename );
      QVERIFY_EQ( std::string(""), m->m_alphaFilename );
      QVERIFY_TRUE( NULL == m->m_textureData );
   }

private slots:
   // Many primitives are recycled. Test initial conditions, change conditions,
   // release, and re-get to make sure that the recyled primitives are properly
   // initialized.

   void testVertexInit()
   {
      Model::Vertex * v = Model::Vertex::get();
      QVERIFY_EQ(1, Model::Vertex::allocated() );
      QVERIFY_EQ(0, Model::Vertex::recycled() );

      vertexIsInitialized( v );

      v->m_visible = false;
      v->m_selected = true;
      v->m_free = true;
      v->m_drawSource = v->m_kfCoord;

      v->release();
      QVERIFY_EQ(1, Model::Vertex::allocated() );
      QVERIFY_EQ(1, Model::Vertex::recycled() );

      v = Model::Vertex::get();
      vertexIsInitialized( v );

      QVERIFY_EQ(1, Model::Vertex::allocated() );
      QVERIFY_EQ(0, Model::Vertex::recycled() );

      v->release();

      QVERIFY_EQ(1, Model::Vertex::allocated() );
      QVERIFY_EQ(1, Model::Vertex::recycled() );

      QVERIFY_EQ(1, Model::Vertex::flush() )
      QVERIFY_EQ(0, Model::Vertex::allocated() );
      QVERIFY_EQ(0, Model::Vertex::recycled() );
   }

   void testTriangleInit()
   {
      Model::Triangle * t = Model::Triangle::get();
      QVERIFY_EQ(1, Model::Triangle::allocated() );
      QVERIFY_EQ(0, Model::Triangle::recycled() );

      triangleIsInitialized( t );

      t->m_visible = false;
      t->m_selected = true;
      t->m_projection = 7;
      t->m_flatSource = t->m_flatNormals;
      t->m_normalSource[0] = t->m_finalNormals[0];
      t->m_normalSource[1] = t->m_finalNormals[1];
      t->m_normalSource[2] = t->m_finalNormals[2];

      t->release();

      QVERIFY_EQ(1, Model::Triangle::allocated() );
      QVERIFY_EQ(1, Model::Triangle::recycled() );

      t = Model::Triangle::get();
      triangleIsInitialized( t );

      QVERIFY_EQ(1, Model::Triangle::allocated() );
      QVERIFY_EQ(0, Model::Triangle::recycled() );

      t->release();

      QVERIFY_EQ(1, Model::Triangle::allocated() );
      QVERIFY_EQ(1, Model::Triangle::recycled() );

      QVERIFY_EQ(1, Model::Triangle::flush() )
      QVERIFY_EQ(0, Model::Triangle::allocated() );
      QVERIFY_EQ(0, Model::Triangle::recycled() );
   }

   void testGroupInit()
   {
      Model::Group * g = Model::Group::get();
      QVERIFY_EQ(1, Model::Group::allocated() );
      QVERIFY_EQ(0, Model::Group::recycled() );

      groupIsInitialized( g );

      g->m_name = "dummy name";
      g->m_materialIndex = 7;
      g->m_triangleIndices.insert(0);
      g->m_smooth = 8;
      g->m_angle = 9;
      g->m_visible = false;
      g->m_selected = true;

      g->release();

      QVERIFY_EQ(1, Model::Group::allocated() );
      QVERIFY_EQ(1, Model::Group::recycled() );

      g = Model::Group::get();
      groupIsInitialized( g );

      QVERIFY_EQ(1, Model::Group::allocated() );
      QVERIFY_EQ(0, Model::Group::recycled() );

      g->release();

      QVERIFY_EQ(1, Model::Group::allocated() );
      QVERIFY_EQ(1, Model::Group::recycled() );

      QVERIFY_EQ(1, Model::Group::flush() )
      QVERIFY_EQ(0, Model::Group::allocated() );
      QVERIFY_EQ(0, Model::Group::recycled() );
   }

   void testMaterialInit()
   {
      Model::Material * m = Model::Material::get();
      QVERIFY_EQ(1, Model::Material::allocated() );
      QVERIFY_EQ(0, Model::Material::recycled() );

      materialIsInitialized( m );

      m->m_name = "dummy name";
      m->m_type = Model::Material::MATTYPE_BLANK;
      // FIXME need fuzzy EQ for floats
      m->m_sClamp = true;
      m->m_tClamp = true;
      m->m_texture = 3;
      m->m_filename = "dummy file";
      m->m_alphaFilename = "dummy alpha";
      m->m_textureData = (Texture *) 100;

      m->release();

      QVERIFY_EQ(1, Model::Material::allocated() );
      QVERIFY_EQ(1, Model::Material::recycled() );

      m = Model::Material::get();
      materialIsInitialized( m );

      QVERIFY_EQ(1, Model::Material::allocated() );
      QVERIFY_EQ(0, Model::Material::recycled() );

      m->release();

      QVERIFY_EQ(1, Model::Material::allocated() );
      QVERIFY_EQ(1, Model::Material::recycled() );

      QVERIFY_EQ(1, Model::Material::flush() )
      QVERIFY_EQ(0, Model::Material::allocated() );
      QVERIFY_EQ(0, Model::Material::recycled() );
   }

   // FIXME test other inits

};

QTEST_MAIN(ModelInitTest)
#include "model_init_test.moc"

