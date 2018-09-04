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


// This file tests libmm3d/endianconfig.h as big endian

#include <QtTest/QtTest>

#include "test_common.h"

#include "config.h"

#undef BYTEORDER
#define BYTEORDER 4321

#include "endianconfig.h"

class BigEndianTest : public QObject
{
   Q_OBJECT
private slots:
   void testBig32()
   {
      uint8_t in[4] = { 0x01, 0x02, 0x03, 0x04 };

      // big to host
      {
         uint32_t source = 0;
         memcpy( &source, in, sizeof(source) );
         uint32_t host = btoh_u32( source );

         QVERIFY_EQ( 0, memcmp( &host, in, sizeof(host) ) );
      }
      {
         int32_t source = 0;
         memcpy( &source, in, sizeof(source) );
         int32_t host = btoh_32( source );

         QVERIFY_EQ( 0, memcmp( &host, in, sizeof(host) ) );
      }

      // host to big
      {
         uint32_t host = 0;
         memcpy( &host, in, sizeof(host) );
         uint32_t dest = htob_u32( host );

         QVERIFY_EQ( 0, memcmp( &dest, in, sizeof(dest) ) );
      }
      {
         int32_t host = 0;
         memcpy( &host, in, sizeof(host) );
         int32_t dest = htob_32( host );

         QVERIFY_EQ( 0, memcmp( &dest, in, sizeof(dest) ) );
      }
   }

   void testBig16()
   {
      uint8_t in[2] = { 0x01, 0x02 };

      // big to host
      {
         uint16_t source = 0;
         memcpy( &source, in, sizeof(source) );
         uint16_t host = btoh_u16( source );

         QVERIFY_EQ( 0, memcmp( &host, in, sizeof(host) ) );
      }
      {
         int16_t source = 0;
         memcpy( &source, in, sizeof(source) );
         int16_t host = btoh_16( source );

         QVERIFY_EQ( 0, memcmp( &host, in, sizeof(host) ) );
      }

      // host to big
      {
         uint16_t host = 0;
         memcpy( &host, in, sizeof(host) );
         uint16_t dest = htob_u16( host );

         QVERIFY_EQ( 0, memcmp( &dest, in, sizeof(dest) ) );
      }
      {
         int16_t host = 0;
         memcpy( &host, in, sizeof(host) );
         int16_t dest = htob_16( host );

         QVERIFY_EQ( 0, memcmp( &dest, in, sizeof(dest) ) );
      }
   }

   void testBigFloat()
   {
      uint8_t in[4] = { 0x01, 0x02, 0x03, 0x04 };

      // big to host
      {
         float source = 0;
         memcpy( &source, in, sizeof(source) );
         float host = btoh_float( source );

         QVERIFY_EQ( 0, memcmp( &host, in, sizeof(host) ) );
      }

      // host to big
      {
         float host = 0;
         memcpy( &host, in, sizeof(host) );
         float dest = htob_float( host );

         QVERIFY_EQ( 0, memcmp( &dest, in, sizeof(dest) ) );
      }
   }

   void testLittle32()
   {
      uint8_t in[4] = { 0x01, 0x02, 0x03, 0x04 };
      uint8_t out[4] = { 0x04, 0x03, 0x02, 0x01 };

      // little to host
      {
         uint32_t source = 0;
         memcpy( &source, in, sizeof(source) );
         uint32_t host = ltoh_u32( source );

         QVERIFY_EQ( 0, memcmp( &host, out, sizeof(host) ) );
      }
      {
         int32_t source = 0;
         memcpy( &source, in, sizeof(source) );
         int32_t host = ltoh_32( source );

         QVERIFY_EQ( 0, memcmp( &host, out, sizeof(host) ) );
      }

      // host to little
      {
         uint32_t host = 0;
         memcpy( &host, in, sizeof(host) );
         uint32_t dest = htol_u32( host );

         QVERIFY_EQ( 0, memcmp( &dest, out, sizeof(dest) ) );
      }
      {
         int32_t host = 0;
         memcpy( &host, in, sizeof(host) );
         int32_t dest = htol_32( host );

         QVERIFY_EQ( 0, memcmp( &dest, out, sizeof(dest) ) );
      }
   }

   void testLittle16()
   {
      uint8_t in[2] = { 0x01, 0x02 };
      uint8_t out[2] = { 0x02, 0x01 };

      // little to host
      {
         uint16_t source = 0;
         memcpy( &source, in, sizeof(source) );
         uint16_t host = ltoh_u16( source );

         QVERIFY_EQ( 0, memcmp( &host, out, sizeof(host) ) );
      }
      {
         int16_t source = 0;
         memcpy( &source, in, sizeof(source) );
         int16_t host = ltoh_16( source );

         QVERIFY_EQ( 0, memcmp( &host, out, sizeof(host) ) );
      }

      // host to little
      {
         uint16_t host = 0;
         memcpy( &host, in, sizeof(host) );
         uint16_t dest = htol_u16( host );

         QVERIFY_EQ( 0, memcmp( &dest, out, sizeof(dest) ) );
      }
      {
         int16_t host = 0;
         memcpy( &host, in, sizeof(host) );
         int16_t dest = htol_16( host );

         QVERIFY_EQ( 0, memcmp( &dest, out, sizeof(dest) ) );
      }
   }

   void testLittleFloat()
   {
      uint8_t in[4] = { 0x01, 0x02, 0x03, 0x04 };
      uint8_t out[4] = { 0x04, 0x03, 0x02, 0x01 };

      // little to host
      {
         float source = 0;
         memcpy( &source, in, sizeof(source) );
         float host = ltoh_float( source );

         QVERIFY_EQ( 0, memcmp( &host, out, sizeof(host) ) );
      }

      // host to little
      {
         float host = 0;
         memcpy( &host, in, sizeof(host) );
         float dest = htol_float( host );

         QVERIFY_EQ( 0, memcmp( &dest, out, sizeof(dest) ) );
      }
   }

};

QTEST_MAIN(BigEndianTest)
#include "big_endian_test.moc"
