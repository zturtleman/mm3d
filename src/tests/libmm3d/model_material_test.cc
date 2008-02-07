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
#include "tgatex.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"


// FIXME centralize this

Texture * loadTextureOrDie( TextureFilter * f, const char * filename )
{
   Texture * tex = new Texture;
   Texture::ErrorE err = f->readFile( tex, filename );

   if ( err != Texture::ERROR_NONE )
   {
      fprintf( stderr, "fatal: %s: %s\n", filename, Texture::errorToString( err ) );
      delete tex;
      exit( -1 );
   }

   return tex;
}

Texture * loadTgaOrDie( const char * filename )
{
   TgaTextureFilter f;
   return loadTextureOrDie( &f, filename );
}

class ModelMaterialTest : public QObject
{
   Q_OBJECT
private:

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   // FIXME add more tests

   void testAddColorMaterial()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      QVERIFY_EQ( 0, lhs->getTextureCount() );

      lhs->addColorMaterial( "One" );
      lhs->addColorMaterial( "Two" );
      lhs->addColorMaterial( "Three" );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Renamed" );
      rhs_2->addColorMaterial( "Three" );

      QVERIFY_EQ( 3, lhs->getTextureCount() );

      QVERIFY_EQ( (int) Model::Material::MATTYPE_BLANK, (int) lhs->getMaterialType(1) );
      QVERIFY_TRUE( NULL == lhs->getTextureData(1) );
      //QVERIFY_TRUE( NULL == lhs->getTextureFilename(1) );

      QVERIFY_EQ( std::string("One"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("Two"), std::string(lhs->getTextureName(1)));
      QVERIFY_EQ( std::string("Three"), std::string(lhs->getTextureName(2)));

      QVERIFY_EQ( 0, lhs->getMaterialByName("One"));
      QVERIFY_EQ( 1, lhs->getMaterialByName("Two"));
      QVERIFY_EQ( 2, lhs->getMaterialByName("Three"));
      QVERIFY_EQ( -1, lhs->getMaterialByName("Renamed"));

      lhs->operationComplete( "Add groups" );

      lhs->setTextureName(1, "Renamed" );

      QVERIFY_EQ( std::string("One"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("Renamed"), std::string(lhs->getTextureName(1)));
      QVERIFY_EQ( std::string("Three"), std::string(lhs->getTextureName(2)));

      QVERIFY_EQ( 0, lhs->getMaterialByName("One"));
      QVERIFY_EQ( 1, lhs->getMaterialByName("Renamed"));
      QVERIFY_EQ( 2, lhs->getMaterialByName("Three"));
      QVERIFY_EQ( -1, lhs->getMaterialByName("Two"));

      lhs->operationComplete( "Set material name" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testAddTextureMaterial()
   {
      local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );
      local_ptr<Texture> tex2 = loadTgaOrDie( "data/test_rgb_uncomp.tga" );
      local_ptr<Texture> tex3 = loadTgaOrDie( "data/test_rgba_comp.tga" );

      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      QVERIFY_EQ( 0, lhs->getTextureCount() );

      lhs->addTexture( tex1.get() );
      lhs->addTexture( tex2.get() );
      lhs->addTexture( tex3.get() );
      rhs_1->addTexture( tex1.get() );
      rhs_1->addTexture( tex2.get() );
      rhs_1->addTexture( tex3.get() );
      rhs_2->addTexture( tex1.get() );
      rhs_2->addTexture( tex2.get() );
      rhs_2->addTexture( tex3.get() );
      rhs_2->setTextureName( 1, "Renamed" );

      QVERIFY_EQ( 3, lhs->getTextureCount() );

      QVERIFY_EQ( (int) Model::Material::MATTYPE_TEXTURE, (int) lhs->getMaterialType(1) );
      QVERIFY_TRUE( tex2.get() == lhs->getTextureData(1) );
      QVERIFY_EQ( std::string("data/test_rgb_uncomp.tga"), std::string(lhs->getTextureFilename(1)) );

      QVERIFY_EQ( std::string("test_rgb_comp"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("test_rgb_uncomp"), std::string(lhs->getTextureName(1)));
      QVERIFY_EQ( std::string("test_rgba_comp"), std::string(lhs->getTextureName(2)));

      QVERIFY_EQ( 0, lhs->getMaterialByName("test_rgb_comp"));
      QVERIFY_EQ( 1, lhs->getMaterialByName("test_rgb_uncomp"));
      QVERIFY_EQ( 2, lhs->getMaterialByName("test_rgba_comp"));
      QVERIFY_EQ( -1, lhs->getMaterialByName("Renamed"));

      lhs->operationComplete( "Add groups" );

      lhs->setTextureName(1, "Renamed" );

      QVERIFY_EQ( std::string("test_rgb_comp"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("Renamed"), std::string(lhs->getTextureName(1)));
      QVERIFY_EQ( std::string("test_rgba_comp"), std::string(lhs->getTextureName(2)));

      QVERIFY_EQ( 0, lhs->getMaterialByName("test_rgb_comp"));
      QVERIFY_EQ( 1, lhs->getMaterialByName("Renamed"));
      QVERIFY_EQ( 2, lhs->getMaterialByName("test_rgba_comp"));
      QVERIFY_EQ( -1, lhs->getMaterialByName("test_rgb_uncomp"));

      lhs->operationComplete( "Set material name" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   // FIXME add tests:
   // x create material (of each type)
   // x get/set material name
   // x get type
   // x get count
   // x get by name
   //   delete material
   //     (implicitly test noTexture)
   //   setMaterialTexture
   //   removeMaterialTexture
   //   getTextureFilename
   //   getTextureData
   //   lighting
   //   clamp
   //   group material index
   //   load textures
   //   invalidate textures
   //   undo
   //
   // FIXME in another file:
   //   rendering
   //     type
   //     lighting
   //     clamp

};

QTEST_MAIN(ModelMaterialTest)
#include "model_material_test.moc"

