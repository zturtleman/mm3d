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


// This file tests the DataDest class.

#include <QtTest/QtTest>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "test_common.h"

#include "datadest.h"
#include "endianconfig.h"

#include "local_array.h"

class DataSourceTest : public QObject
{
   Q_OBJECT

private:
   class TestDest : public DataDest
   {
      public:
         TestDest( size_t dataSize )
            : DataDest(),
              m_dataSize( dataSize ),
              m_buf( new uint8_t[dataSize] ),
              m_offset( 0 ),
              m_errorAttempt( 0 ),
              m_err( EBADF )
         {
            memset( m_buf.get(), 'X', dataSize );
         }

         virtual ~TestDest()
         {
         }

         bool internalWrite( const uint8_t * buf, size_t bufLen )
         {
            // TODO should assert on buf
            if ( errorOccurred() )
               return false;

            if ( m_errorAttempt > 0 )
            {
               --m_errorAttempt;
               if ( m_errorAttempt == 0 )
               {
                  setErrno( m_err );
                  return false;
               }
            }
            if ( bufLen + m_offset > m_dataSize )
            {
               setAtFileLimit( true );
               return false;
            }
            memcpy( &m_buf.get()[ m_offset ], buf, bufLen );
            m_offset += bufLen;

            return true;
         }

         bool internalSeek( off_t offset )
         {
            if ( (size_t) offset > m_dataSize )
            {
               setAtFileLimit( true );
               return false;
            }

            m_offset = offset;

            return true;
         }

         uint8_t * getBuffer() { return m_buf.get(); }

         void setErrorAttempt( int errorAttempt, int err )
         {
            m_errorAttempt = errorAttempt;
            m_err = err;
         }

      private:
         size_t m_dataSize;
         local_array<uint8_t> m_buf;
         size_t m_offset;
         int m_errorAttempt;
         int m_err;
   };

private slots:
   void testWriteIntLittleEndian()
   {
      TestDest dest( 1024 );

      dest.setEndianness( DataDest::LittleEndian );
      QVERIFY_TRUE( DataDest::LittleEndian == dest.getEndianness() );

      int8_t   i8  = 1;
      uint8_t  u8  = 2;
      int16_t  i16 = 3;
      uint16_t u16 = 4;
      int32_t  i32 = 5;
      uint32_t u32 = 6;

      QVERIFY_TRUE( dest.write( i8 ) );
      QVERIFY_EQ( (size_t) 1, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( u8 ) );
      QVERIFY_EQ( (size_t) 2, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( i16 ) );
      QVERIFY_EQ( (size_t) 4, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( u16 ) );
      QVERIFY_EQ( (size_t) 6, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( i32 ) );
      QVERIFY_EQ( (size_t) 10, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( u32 ) );
      QVERIFY_EQ( (size_t) 14, dest.getFileSize() );

      const size_t expectedSize = 14;
      const uint8_t expectedData[ expectedSize ] = {
         0x01, 0x02,
         0x03, 0x00, 0x04, 0x00,
         0x05, 0x00, 0x00, 0x00,
         0x06, 0x00, 0x00, 0x00,
      };

      QVERIFY_TRUE( 0 == memcmp( expectedData, dest.getBuffer(), expectedSize ) );
   }

   void testWriteIntBigEndian()
   {
      TestDest dest( 1024 );

      dest.setEndianness( DataDest::BigEndian );
      QVERIFY_TRUE( DataDest::BigEndian == dest.getEndianness() );

      int8_t   i8  = 1;
      uint8_t  u8  = 2;
      int16_t  i16 = 3;
      uint16_t u16 = 4;
      int32_t  i32 = 5;
      uint32_t u32 = 6;

      QVERIFY_TRUE( dest.write( i8 ) );
      QVERIFY_EQ( (size_t) 1, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( u8 ) );
      QVERIFY_EQ( (size_t) 2, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( i16 ) );
      QVERIFY_EQ( (size_t) 4, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( u16 ) );
      QVERIFY_EQ( (size_t) 6, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( i32 ) );
      QVERIFY_EQ( (size_t) 10, dest.getFileSize() );
      QVERIFY_TRUE( dest.write( u32 ) );
      QVERIFY_EQ( (size_t) 14, dest.getFileSize() );

      const size_t expectedSize = 14;
      const uint8_t expectedData[ expectedSize ] = {
         0x01, 0x02,
         0x00, 0x03, 0x00, 0x04,
         0x00, 0x00, 0x00, 0x05,
         0x00, 0x00, 0x00, 0x06,
      };

      QVERIFY_TRUE( 0 == memcmp( expectedData, dest.getBuffer(), expectedSize ) );
   }

   void testWrite()
   {
      TestDest dest( 1024 );

      const size_t bufSize = 10;
      uint8_t buf[bufSize + 1] = "ABCDEFGHIJ";

      QVERIFY_TRUE( dest.writeBytes( buf, bufSize ) );
      QVERIFY_EQ( (size_t) 10, dest.getFileSize() );
      QVERIFY_EQ( 10, (int) dest.offset() );
      QVERIFY_TRUE( dest.seek( 5 ) );
      QVERIFY_EQ( 5, (int) dest.offset() );

      std::string str = "foo";

      QVERIFY_EQ( 4, dest.writeAsciiz( str.c_str() ) );
      QVERIFY_EQ( (size_t) 10, dest.getFileSize() );
      QVERIFY_EQ( 9, (int) dest.offset() );
      QVERIFY_EQ( 3, dest.writeString( str.c_str() ) );
      QVERIFY_EQ( (size_t) 12, dest.getFileSize() );
      QVERIFY_EQ( 12, (int) dest.offset() );
      QVERIFY_EQ( 13, dest.printf( "printf %s %d", str.c_str(), 42 ) );
      QVERIFY_EQ( (size_t) 25, dest.getFileSize() );
      QVERIFY_EQ( 25, (int) dest.offset() );

      const size_t expectedSize = 25;
      uint8_t expectedData[ expectedSize + 1] = "ABCDEfoo\0fooprintf foo 42";

      QVERIFY_TRUE( 0 == memcmp( expectedData, dest.getBuffer(), expectedSize ) );
   }

   void testFileSizeLimit()
   {
      {
         TestDest dest( 1024 );
         dest.setFileSizeLimit( 7 );
         QVERIFY_EQ( (size_t) 7, dest.getFileSizeLimit() );
         uint32_t val = 0;
         QVERIFY_TRUE( dest.write( val ) );
         QVERIFY_FALSE( dest.errorOccurred( ) );
         QVERIFY_FALSE( dest.write( val ) );
         QVERIFY_TRUE( dest.errorOccurred( ) );
         QVERIFY_TRUE( dest.atFileLimit( ) );
      }

      {
         TestDest dest( 1024 );
         dest.setFileSizeLimit( 7 );
         QVERIFY_EQ( (size_t) 7, dest.getFileSizeLimit() );
         uint32_t val = 0;
         QVERIFY_TRUE( dest.write( val ) );
         QVERIFY_TRUE( dest.seek( 3 ) );
         QVERIFY_TRUE( dest.write( val ) );
         QVERIFY_FALSE( dest.errorOccurred( ) );
         QVERIFY_FALSE( dest.write( val ) );
         QVERIFY_TRUE( dest.errorOccurred( ) );
         QVERIFY_TRUE( dest.atFileLimit( ) );
      }

      {
         TestDest dest( 1024 );
         dest.setFileSizeLimit( 7 );
         QVERIFY_EQ( (size_t) 7, dest.getFileSizeLimit() );
         uint32_t val = 0;
         QVERIFY_TRUE( dest.write( val ) );
         uint8_t val8 = 0;
         QVERIFY_TRUE( dest.write( val8 ) );
         QVERIFY_TRUE( dest.write( val8 ) );
         QVERIFY_TRUE( dest.write( val8 ) );
         QVERIFY_FALSE( dest.errorOccurred( ) );
         QVERIFY_FALSE( dest.write( val8 ) );
         QVERIFY_TRUE( dest.errorOccurred( ) );
         QVERIFY_TRUE( dest.atFileLimit( ) );
      }
   }

   void testHasLimit()
   {
      {
         TestDest dest( 1024 );
         QVERIFY_FALSE( dest.hasFileSizeLimit() );
         dest.setFileSizeLimit( 7 );
         QVERIFY_TRUE( dest.hasFileSizeLimit() );
         dest.removeFileSizeLimit();
         QVERIFY_FALSE( dest.hasFileSizeLimit() );

         uint32_t val = 0;
         QVERIFY_TRUE( dest.write( val ) );
         QVERIFY_TRUE( dest.write( val ) );
      }

      {
         TestDest dest( 1024 );
         QVERIFY_FALSE( dest.hasFileSizeLimit() );
         dest.setFileSizeLimit( 7 );
         QVERIFY_TRUE( dest.hasFileSizeLimit() );
         dest.removeFileSizeLimit();
         QVERIFY_FALSE( dest.hasFileSizeLimit() );

         dest.setFileSizeLimit( 7 );
         uint32_t val = 0;
         QVERIFY_TRUE( dest.write( val ) );
         QVERIFY_FALSE( dest.write( val ) );
      }
   }

   void testErrno()
   {
      TestDest dest( 1024 );
      dest.setErrorAttempt( 3, EPIPE );

      uint32_t val = 0;

      QVERIFY_TRUE( dest.write( val ) );
      QVERIFY_TRUE( dest.write( val ) );
      QVERIFY_FALSE( dest.errorOccurred() );

      // Error happens now
      QVERIFY_FALSE( dest.write( val ) );
      QVERIFY_TRUE( dest.errorOccurred() );
      QVERIFY_EQ( EPIPE, dest.getErrno() );
   }

   // FIXME test FileDataDest
};

QTEST_MAIN(DataSourceTest)
#include "datadest_test.moc"
