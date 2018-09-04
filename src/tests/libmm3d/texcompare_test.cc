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


// This file tests the equality of texture data

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "tgatex.h"
#include "log.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"


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

Texture * loadOldTgaOrDie( const char * filename )
{
   OldTgaTextureFilter f;
   return loadTextureOrDie( &f, filename );
}

class TextureCompareTest : public QObject
{
   Q_OBJECT
private slots:

      void initTestCase()
      {
         log_enable_debug( false );
      }

      void testTgaOldNewRgbUncomp()
      {
         local_ptr<Texture> lhs = loadOldTgaOrDie( "data/test_rgb_uncomp.tga" );
         local_ptr<Texture> rhs = loadTgaOrDie( "data/test_rgb_uncomp.tga" );

         Texture::CompareResultT res;
         QVERIFY_TRUE( lhs->compare( rhs.get(), &res, 0 ) );
         QVERIFY_TRUE( res.comparable );
         QVERIFY_TRUE( res.pixelCount > 0 );
         QVERIFY_EQ( res.pixelCount, res.matchCount );
         QVERIFY_EQ( res.pixelCount, res.fuzzyCount );
      }

      void testTgaOldNewRgbaUncomp()
      {
         local_ptr<Texture> lhs = loadOldTgaOrDie( "data/test_rgba_uncomp.tga" );
         local_ptr<Texture> rhs = loadTgaOrDie( "data/test_rgba_uncomp.tga" );

         Texture::CompareResultT res;
         QVERIFY_TRUE( lhs->compare( rhs.get(), &res, 0 ) );
         QVERIFY_TRUE( res.comparable );
         QVERIFY_TRUE( res.pixelCount > 0 );
         QVERIFY_EQ( res.pixelCount, res.matchCount );
         QVERIFY_EQ( res.pixelCount, res.fuzzyCount );
      }

      void testTgaOldNewRgbComp()
      {
         local_ptr<Texture> lhs = loadOldTgaOrDie( "data/test_rgb_comp.tga" );
         local_ptr<Texture> rhs = loadTgaOrDie( "data/test_rgb_comp.tga" );

         Texture::CompareResultT res;
         QVERIFY_TRUE( lhs->compare( rhs.get(), &res, 0 ) );
         QVERIFY_TRUE( res.comparable );
         QVERIFY_TRUE( res.pixelCount > 0 );
         QVERIFY_EQ( res.pixelCount, res.matchCount );
         QVERIFY_EQ( res.pixelCount, res.fuzzyCount );
      }

      void testTgaOldNewRgbaComp()
      {
         local_ptr<Texture> lhs = loadOldTgaOrDie( "data/test_rgba_comp.tga" );
         local_ptr<Texture> rhs = loadTgaOrDie( "data/test_rgba_comp.tga" );

         Texture::CompareResultT res;
         QVERIFY_TRUE( lhs->compare( rhs.get(), &res, 0 ) );
         QVERIFY_TRUE( res.comparable );
         QVERIFY_TRUE( res.pixelCount > 0 );
         QVERIFY_EQ( res.pixelCount, res.matchCount );
         QVERIFY_EQ( res.pixelCount, res.fuzzyCount );
      }
};

QTEST_MAIN(TextureCompareTest)
#include "texcompare_test.moc"

