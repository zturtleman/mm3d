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

      QVERIFY_EQ( std::string("One"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("Two"), std::string(lhs->getTextureName(1)));
      QVERIFY_EQ( std::string("Three"), std::string(lhs->getTextureName(2)));

      QVERIFY_EQ( 0, lhs->getMaterialByName("One"));
      QVERIFY_EQ( 1, lhs->getMaterialByName("Two"));
      QVERIFY_EQ( 2, lhs->getMaterialByName("Three"));
      QVERIFY_EQ( -1, lhs->getMaterialByName("Renamed"));

      lhs->operationComplete( "Add materials" );

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

      lhs->operationComplete( "Add materials" );

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

   void testDeleteMaterial()
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
      lhs->addGroup( "One" );
      lhs->addGroup( "Two" );
      lhs->addGroup( "Three" );
      lhs->setGroupTextureId( 0, 2 );
      lhs->setGroupTextureId( 1, 1 );
      lhs->setGroupTextureId( 2, 0 );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_1->addGroup( "One" );
      rhs_1->addGroup( "Two" );
      rhs_1->addGroup( "Three" );
      rhs_1->setGroupTextureId( 0, 2 );
      rhs_1->setGroupTextureId( 1, 1 );
      rhs_1->setGroupTextureId( 2, 0 );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Three" );
      rhs_2->addGroup( "One" );
      rhs_2->addGroup( "Two" );
      rhs_2->addGroup( "Three" );
      rhs_2->setGroupTextureId( 0, 1 );
      rhs_2->setGroupTextureId( 2, 0 );

      QVERIFY_EQ( 3, lhs->getTextureCount() );

      lhs->operationComplete( "Add materials and groups" );

      lhs->deleteTexture( 1 );
      QVERIFY_EQ( 2, lhs->getTextureCount() );

      QVERIFY_EQ( 1, lhs->getGroupTextureId(0));
      QVERIFY_EQ( -1, lhs->getGroupTextureId(1));
      QVERIFY_EQ( 0, lhs->getGroupTextureId(2));

      QVERIFY_EQ( std::string("One"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("Three"), std::string(lhs->getTextureName(1)));

      lhs->operationComplete( "Delete material" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testAddRemoveTexture()
   {
      local_ptr<Texture> tex1 = loadTgaOrDie( "data/test_rgb_comp.tga" );

      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );
      rhs_list.push_back( rhs_1.get() );

      QVERIFY_EQ( 0, lhs->getTextureCount() );

      lhs->addColorMaterial( "One" );
      lhs->addColorMaterial( "Two" );
      lhs->addColorMaterial( "Three" );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Two" );
      rhs_2->addColorMaterial( "Three" );
      rhs_2->setMaterialTexture( 1, tex1.get() );

      QVERIFY_EQ( std::string("One"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("Two"), std::string(lhs->getTextureName(1)));
      QVERIFY_EQ( std::string("Three"), std::string(lhs->getTextureName(2)));

      QVERIFY_EQ( (int) Model::Material::MATTYPE_BLANK, (int) lhs->getMaterialType(1) );
      QVERIFY_TRUE( NULL == lhs->getTextureData(1) );

      lhs->operationComplete( "Add materials" );

      lhs->setMaterialTexture(1, tex1.get() );
      QVERIFY_TRUE( tex1.get() == lhs->getTextureData(1) );
      QVERIFY_EQ( std::string("data/test_rgb_comp.tga"), std::string(lhs->getTextureFilename(1)) );

      QVERIFY_EQ( (int) Model::Material::MATTYPE_TEXTURE, (int) lhs->getMaterialType(1) );

      QVERIFY_EQ( std::string("One"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("Two"), std::string(lhs->getTextureName(1)));
      QVERIFY_EQ( std::string("Three"), std::string(lhs->getTextureName(2)));

      lhs->operationComplete( "Set material texture" );

      lhs->removeMaterialTexture( 1 );

      QVERIFY_EQ( (int) Model::Material::MATTYPE_BLANK, (int) lhs->getMaterialType(1) );
      QVERIFY_TRUE( NULL == lhs->getTextureData(1) );

      QVERIFY_EQ( std::string("One"), std::string(lhs->getTextureName(0)));
      QVERIFY_EQ( std::string("Two"), std::string(lhs->getTextureName(1)));
      QVERIFY_EQ( std::string("Three"), std::string(lhs->getTextureName(2)));

      lhs->operationComplete( "Set material texture" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
   }

   void testSetGroupMaterial()
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
      lhs->addGroup( "One" );
      lhs->addGroup( "Two" );
      lhs->addGroup( "Three" );
      lhs->setGroupTextureId( 0, 0 );
      lhs->setGroupTextureId( 1, 1 );
      lhs->setGroupTextureId( 2, 2 );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_1->addGroup( "One" );
      rhs_1->addGroup( "Two" );
      rhs_1->addGroup( "Three" );
      rhs_1->setGroupTextureId( 0, 0 );
      rhs_1->setGroupTextureId( 1, 1 );
      rhs_1->setGroupTextureId( 2, 2 );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Two" );
      rhs_2->addColorMaterial( "Three" );
      rhs_2->addGroup( "One" );
      rhs_2->addGroup( "Two" );
      rhs_2->addGroup( "Three" );
      rhs_2->setGroupTextureId( 0, 2 );
      rhs_2->setGroupTextureId( 2, 0 );

      QVERIFY_EQ( 0, lhs->getGroupTextureId(0));
      QVERIFY_EQ( 1, lhs->getGroupTextureId(1));
      QVERIFY_EQ( 2, lhs->getGroupTextureId(2));

      lhs->operationComplete( "Add materials and groups" );

      lhs->setGroupTextureId( 0, 2 );
      lhs->setGroupTextureId( 1, -1 );
      lhs->setGroupTextureId( 2, 0 );

      QVERIFY_EQ( 2, lhs->getGroupTextureId(0));
      QVERIFY_EQ( -1, lhs->getGroupTextureId(1));
      QVERIFY_EQ( 0, lhs->getGroupTextureId(2));

      lhs->operationComplete( "Set group material ID" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testSetClamp()
   {
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

      QVERIFY_EQ( 0, lhs->getTextureCount() );

      lhs->addColorMaterial( "One" );
      lhs->addColorMaterial( "Two" );
      lhs->addColorMaterial( "Three" );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Two" );
      rhs_2->addColorMaterial( "Three" );
      rhs_3->addColorMaterial( "One" );
      rhs_3->addColorMaterial( "Two" );
      rhs_3->addColorMaterial( "Three" );

      lhs->setTextureSClamp( 1, false );
      rhs_1->setTextureSClamp( 1, false );
      rhs_2->setTextureSClamp( 1, true );
      rhs_3->setTextureSClamp( 1, true );
      lhs->setTextureTClamp( 1, false );
      rhs_1->setTextureTClamp( 1, false );
      rhs_2->setTextureTClamp( 1, false );
      rhs_3->setTextureTClamp( 1, true );

      QVERIFY_FALSE( lhs->getTextureSClamp(1) );
      QVERIFY_FALSE( lhs->getTextureTClamp(1) );

      lhs->operationComplete( "Add materials" );

      lhs->setTextureSClamp( 1, true );

      QVERIFY_TRUE( lhs->getTextureSClamp(1) );
      QVERIFY_FALSE( lhs->getTextureTClamp(1) );

      lhs->operationComplete( "Set material S clamp" );

      lhs->setTextureTClamp( 1, true );

      QVERIFY_TRUE( lhs->getTextureSClamp(1) );
      QVERIFY_TRUE( lhs->getTextureTClamp(1) );

      lhs->operationComplete( "Set material T clamp" );

      checkUndoRedo( 3, lhs.get(), rhs_list );
   }

   void testSetAmbient()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      lhs->addColorMaterial( "One" );
      lhs->addColorMaterial( "Two" );
      lhs->addColorMaterial( "Three" );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Two" );
      rhs_2->addColorMaterial( "Three" );

      const float orig[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
      const float change[4] = { 0.2f, 0.4f, 0.6f, 0.8f };
      float actual[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

      lhs->setTextureAmbient( 1, orig );
      rhs_1->setTextureAmbient( 1, orig );
      rhs_2->setTextureAmbient( 1, change );

      lhs->getTextureAmbient( 1, actual );
      QVERIFY_ARRAY_EQ( orig, 4, actual, 4 );

      lhs->operationComplete( "Add materials" );

      lhs->setTextureAmbient( 1, change );

      lhs->getTextureAmbient( 1, actual );
      QVERIFY_ARRAY_EQ( change, 4, actual, 4 );

      lhs->operationComplete( "Set material ambient" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testSetDiffuse()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      lhs->addColorMaterial( "One" );
      lhs->addColorMaterial( "Two" );
      lhs->addColorMaterial( "Three" );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Two" );
      rhs_2->addColorMaterial( "Three" );

      const float orig[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
      const float change[4] = { 0.2f, 0.4f, 0.6f, 0.8f };
      float actual[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

      lhs->setTextureDiffuse( 1, orig );
      rhs_1->setTextureDiffuse( 1, orig );
      rhs_2->setTextureDiffuse( 1, change );

      lhs->getTextureDiffuse( 1, actual );
      QVERIFY_ARRAY_EQ( orig, 4, actual, 4 );

      lhs->operationComplete( "Add materials" );

      lhs->setTextureDiffuse( 1, change );

      lhs->getTextureDiffuse( 1, actual );
      QVERIFY_ARRAY_EQ( change, 4, actual, 4 );

      lhs->operationComplete( "Set material diffuse" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testSetEmissive()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      lhs->addColorMaterial( "One" );
      lhs->addColorMaterial( "Two" );
      lhs->addColorMaterial( "Three" );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Two" );
      rhs_2->addColorMaterial( "Three" );

      const float orig[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
      const float change[4] = { 0.2f, 0.4f, 0.6f, 0.8f };
      float actual[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

      lhs->setTextureEmissive( 1, orig );
      rhs_1->setTextureEmissive( 1, orig );
      rhs_2->setTextureEmissive( 1, change );

      lhs->getTextureEmissive( 1, actual );
      QVERIFY_ARRAY_EQ( orig, 4, actual, 4 );

      lhs->operationComplete( "Add materials" );

      lhs->setTextureEmissive( 1, change );

      lhs->getTextureEmissive( 1, actual );
      QVERIFY_ARRAY_EQ( change, 4, actual, 4 );

      lhs->operationComplete( "Set material emissive" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testSetSpecular()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      lhs->addColorMaterial( "One" );
      lhs->addColorMaterial( "Two" );
      lhs->addColorMaterial( "Three" );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Two" );
      rhs_2->addColorMaterial( "Three" );

      const float orig[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
      const float change[4] = { 0.2f, 0.4f, 0.6f, 0.8f };
      float actual[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

      lhs->setTextureSpecular( 1, orig );
      rhs_1->setTextureSpecular( 1, orig );
      rhs_2->setTextureSpecular( 1, change );

      lhs->getTextureSpecular( 1, actual );
      QVERIFY_ARRAY_EQ( orig, 4, actual, 4 );

      lhs->operationComplete( "Add materials" );

      lhs->setTextureSpecular( 1, change );

      lhs->getTextureSpecular( 1, actual );
      QVERIFY_ARRAY_EQ( change, 4, actual, 4 );

      lhs->operationComplete( "Set material specular" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   void testSetShininess()
   {
      local_ptr<Model> lhs = newTestModel();
      local_ptr<Model> rhs_empty = newTestModel();
      local_ptr<Model> rhs_1 = newTestModel();
      local_ptr<Model> rhs_2 = newTestModel();

      ModelList rhs_list;
      rhs_list.push_back( rhs_empty.get() );
      rhs_list.push_back( rhs_1.get() );
      rhs_list.push_back( rhs_2.get() );

      lhs->addColorMaterial( "One" );
      lhs->addColorMaterial( "Two" );
      lhs->addColorMaterial( "Three" );
      rhs_1->addColorMaterial( "One" );
      rhs_1->addColorMaterial( "Two" );
      rhs_1->addColorMaterial( "Three" );
      rhs_2->addColorMaterial( "One" );
      rhs_2->addColorMaterial( "Two" );
      rhs_2->addColorMaterial( "Three" );

      const float orig = 1.0f;
      const float change = 0.5f;
      float actual = 1.0f;

      lhs->setTextureShininess( 1, orig );
      rhs_1->setTextureShininess( 1, orig );
      rhs_2->setTextureShininess( 1, change );

      lhs->getTextureShininess( 1, actual );
      QVERIFY_EQ( orig, actual );

      lhs->operationComplete( "Add materials" );

      lhs->setTextureShininess( 1, change );

      lhs->getTextureShininess( 1, actual );
      QVERIFY_EQ( change, actual );

      lhs->operationComplete( "Set material shininess" );

      checkUndoRedo( 2, lhs.get(), rhs_list );
   }

   // FIXME in another file:
   //   rendering
   //     type
   //     lighting
   //     clamp
   //    load textures
   //    invalidate textures

};

QTEST_MAIN(ModelMaterialTest)
#include "model_material_test.moc"

