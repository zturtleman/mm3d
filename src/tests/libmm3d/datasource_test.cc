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


// This file tests the DataSource class.

#include <QtTest/QtTest>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "test_common.h"

#include "datasource.h"
#include "endianconfig.h"

#include "local_array.h"

class DataSourceTest : public QObject
{
   Q_OBJECT

private:
   class TestSource : public DataSource
   {
      public:
         TestSource( size_t dataSize, size_t returnSize )
            : DataSource(),
              m_dataSize( dataSize ),
              m_returnSize( returnSize ),
              m_errorAttempt( 0 ),
              m_err( EBADF )
         {
            m_buf = new uint8_t[m_dataSize];

            for (size_t i = 0; i < m_dataSize / 4; ++i )
            {
               // Write little-endian data to the buffer
               uint32_t * ptr = &((uint32_t *) m_buf.get())[i];
               *ptr = htol_u32( i + 1 );
            }

            setFileSize( m_dataSize );
         }

         virtual ~TestSource()
         {
         }

         bool internalReadAt( off_t offset, const uint8_t ** buf, size_t * bufLen )
         {
            // TODO should assert on buf and bufLen

            if ( m_errorAttempt > 0 )
            {
               --m_errorAttempt;
               if ( m_errorAttempt == 0 )
               {
                  setErrno( m_err );
                  return false;
               }
            }

            if ( offset > (off_t) m_dataSize )
            {
               *buf = NULL;
               *bufLen = 0;
               return false;
            }

            *bufLen = m_returnSize;

            if ( (int) *bufLen > ((int) m_dataSize - offset) )
               *bufLen = m_dataSize - offset;

            *buf = &(m_buf.get()[offset]);

            return true;
         }

         void setErrorAttempt( int errorAttempt, int err )
         {
            m_errorAttempt = errorAttempt;
            m_err = err;
         }

      private:
         size_t m_dataSize;
         size_t m_returnSize;
         local_array<uint8_t> m_buf;
         int m_errorAttempt;
         int m_err;
   };

   class AsciiSource : public DataSource
   {
      public:
         AsciiSource( size_t returnSize )
            : DataSource(),
              m_dataSize( 30 ),
              m_returnSize( returnSize )
         {
            m_buf = new uint8_t[ m_dataSize ];

            memcpy( (char *) m_buf.get(),
                  "123456789\0ABCDEFGHI\0abcdefghi\0",
                  m_dataSize);

            setFileSize( m_dataSize );
         }

         virtual ~AsciiSource()
         {
         }

         bool internalReadAt( off_t offset, const uint8_t ** buf, size_t * bufLen )
         {
            // TODO should assert on buf and bufLen

            if ( offset > (off_t) m_dataSize )
            {
               *buf = NULL;
               *bufLen = 0;
               return false;
            }

            *bufLen = m_returnSize;

            if ( (int) *bufLen > ((int) m_dataSize - offset) )
               *bufLen = m_dataSize - offset;

            *buf = &(m_buf.get()[offset]);

            return true;
         }

      private:
         size_t m_dataSize;
         size_t m_returnSize;
         local_array<uint8_t> m_buf;
   };

   template<typename T> void readAll( TestSource & src, size_t size )
   {
      QVERIFY_EQ( size, src.getFileSize() );

      T val = 0;
      while ( size >= sizeof(val) )
      {
         QVERIFY_FALSE( src.eof() );
         QVERIFY_TRUE( src.read(val) );
         size -= sizeof(val);
      }
      QVERIFY_TRUE( src.eof() );
      QVERIFY_FALSE( src.unexpectedEof() );
      QVERIFY_FALSE( src.read(val) );
      QVERIFY_TRUE( src.eof() );
      QVERIFY_TRUE( src.unexpectedEof() );
   }

private slots:
   void testSize()
   {
      const size_t fileSize = 1024;
      const size_t returnSize = 200;

      {
         TestSource src( fileSize, returnSize );
         readAll<uint8_t>(src, fileSize);
      }
      {
         TestSource src( fileSize, returnSize );
         QVERIFY_EQ( fileSize, src.getFileSize() );
         readAll<int8_t>(src, fileSize);
      }
      {
         TestSource src( fileSize, returnSize );
         QVERIFY_EQ( fileSize, src.getFileSize() );
         readAll<uint16_t>(src, fileSize);
      }
      {
         TestSource src( fileSize, returnSize );
         QVERIFY_EQ( fileSize, src.getFileSize() );
         readAll<int16_t>(src, fileSize);
      }
      {
         TestSource src( fileSize, returnSize );
         QVERIFY_EQ( fileSize, src.getFileSize() );
         readAll<uint32_t>(src, fileSize);
      }
      {
         TestSource src( fileSize, returnSize );
         QVERIFY_EQ( fileSize, src.getFileSize() );
         readAll<int32_t>(src, fileSize);
      }
   }

   void testRead()
   {
      const size_t fileSize = 1024;
      const size_t returnSize = 200;

      TestSource src( fileSize, returnSize );

      QVERIFY_EQ( (uint32_t) 1, src.readU32() );
      QVERIFY_EQ( (int32_t)  2, src.readI32() );
      QVERIFY_EQ( (uint16_t) 3, src.readU16() );
      QVERIFY_EQ( (uint16_t) 0, src.readU16() );
      QVERIFY_EQ( (int16_t)  4, src.readI16() );
      QVERIFY_EQ( (int16_t)  0, src.readI16() );
      QVERIFY_EQ( (uint8_t)  5, src.readU8() );
      QVERIFY_EQ( (uint8_t)  0, src.readU8() );
      QVERIFY_EQ( (uint8_t)  0, src.readU8() );
      QVERIFY_EQ( (uint8_t)  0, src.readU8() );
      QVERIFY_EQ( (int8_t)   6, src.readI8() );
      QVERIFY_EQ( (int8_t)   0, src.readI8() );
      QVERIFY_EQ( (int8_t)   0, src.readI8() );
      QVERIFY_EQ( (int8_t)   0, src.readI8() );

      uint8_t  u8  = 0;
      src.read( u8 );
      QVERIFY_EQ( (uint8_t) 7, u8 );
      src.read( u8 ); src.read( u8 ); src.read( u8 );
      QVERIFY_EQ( (uint8_t) 0, u8 );

      int8_t   i8  = 0;
      src.read( i8 );
      QVERIFY_EQ( (int8_t) 8, i8 );
      src.read( i8 ); src.read( i8 ); src.read( i8 );
      QVERIFY_EQ( (int8_t) 0, i8 );

      uint16_t u16 = 0;
      src.read( u16 );
      QVERIFY_EQ( (uint16_t) 9, u16 );
      src.read( u16 );
      QVERIFY_EQ( (uint16_t) 0, u16 );

      int16_t  i16 = 0;
      src.read( i16 );
      QVERIFY_EQ( (int16_t) 10, i16 );
      src.read( i16 );
      QVERIFY_EQ( (int16_t) 0, i16 );

      uint32_t u32 = 0;
      src.read( u32 );
      QVERIFY_EQ( (uint32_t) 11, u32 );

      int32_t  i32 = 0;
      src.read( i32 );
      QVERIFY_EQ( (int32_t) 12, i32 );
   }

   void testWrapRead()
   {
      const size_t fileSize = 128;
      const size_t returnSize = 16;

      TestSource src( fileSize, returnSize );

      // The data size returned wraps at 16 bytes, get onto an odd boundary
      // and make sure reading works properly
      QVERIFY_EQ( (uint8_t)   1, src.readU8() );
      for ( int i = 2; i <= 32; ++i )
      {
         QVERIFY_EQ( (uint32_t) i << 24, src.readU32() );
      }
      QVERIFY_FALSE( src.eof() );
      QVERIFY_EQ( (uint8_t) 0, src.readU8() );
      QVERIFY_EQ( (uint8_t) 0, src.readU8() );
      QVERIFY_EQ( (uint8_t) 0, src.readU8() );
      QVERIFY_TRUE( src.eof() );
      src.readU8();
      QVERIFY_TRUE( src.unexpectedEof() );
   }

   void testSeek()
   {
      const size_t fileSize = 1024;
      const size_t returnSize = 200;

      TestSource src( fileSize, returnSize );

      QVERIFY_TRUE( src.seek( 200 ) );
      QVERIFY_EQ( (uint32_t) 51, src.readU32() );

      QVERIFY_TRUE( src.seek( 1 ) );
      QVERIFY_EQ( (uint32_t) 2 << 24, src.readU32() );

      QVERIFY_TRUE( src.seek( 1024 ) );
      QVERIFY_TRUE( src.eof() );
      QVERIFY_FALSE( src.unexpectedEof() );
      QVERIFY_FALSE( src.seek( 1025 ) );
      QVERIFY_TRUE( src.eof() );
      QVERIFY_TRUE( src.unexpectedEof() );
   }

   void testEndianness()
   {
      const size_t fileSize = 1024;
      const size_t returnSize = 200;

      TestSource src( fileSize, returnSize );

      src.setEndianness( DataSource::LittleEndian );
      QVERIFY_EQ( DataSource::LittleEndian, src.getEndianness() );
      QVERIFY_EQ( (uint32_t) 1, src.readU32() );

      // Data is little endian, if we treat it like big endian the bytes will
      // be reversed (regardless of whether we're on big endian or little
      // endian hardware).
      uint32_t val = 0x02000000;
      src.setEndianness( DataSource::BigEndian );
      QVERIFY_EQ( DataSource::BigEndian, src.getEndianness() );
      QVERIFY_EQ( val, src.readU32() );
   }

   // Read size that is smaller than the size returned by the test source
   void testReadBytesSmall()
   {
      const size_t returnSize = 8;

      AsciiSource src( returnSize );

      const size_t strSize = 6;
      char str[strSize] = "";

      strcpy( str, "00000" );
      QVERIFY_TRUE( src.readBytes( (uint8_t *) str, strSize ) );
      QVERIFY_TRUE( memcmp( str, "123456", strSize ) == 0 );
      strcpy( str, "00000" );
      QVERIFY_TRUE( src.readBytes( (uint8_t *) str, strSize ) );
      QVERIFY_TRUE( memcmp( str, "789\0AB", strSize ) == 0 );

      src.seek( 25 );
      QVERIFY_FALSE( src.readBytes( (uint8_t *) str, strSize ) );
      QVERIFY_TRUE( src.unexpectedEof() );
   }

   // Read size that is larger than the size returned by the test source
   void testReadBytesLarge()
   {
      const size_t returnSize = 8;

      AsciiSource src( returnSize );

      const size_t strSize = 14;
      char str[strSize] = "";

      strcpy( str, "0000000000000" );
      QVERIFY_TRUE( src.readBytes( (uint8_t *) str, strSize ) );
      QVERIFY_TRUE( memcmp( str, "123456789\0ABCD", strSize ) == 0 );
      strcpy( str, "0000000000000" );
      QVERIFY_TRUE( src.readBytes( (uint8_t *) str, strSize ) );
      QVERIFY_TRUE( memcmp( str, "EFGHI\0abcdefgh", strSize ) == 0 );
      strcpy( str, "0000000000000" );
      QVERIFY_FALSE( src.readBytes( (uint8_t *) str, strSize ) );
      QVERIFY_TRUE( src.unexpectedEof() );
   }

   // Read size that is exactly the size returned by the test source
   void testReadBytesAll()
   {
      const size_t returnSize = 8;

      AsciiSource src( returnSize );

      const size_t strSize = 30;
      char str[strSize] = "";

      strcpy( str, "0000000000000" );
      QVERIFY_TRUE( src.readBytes( (uint8_t *) str, strSize ) );
      QVERIFY_TRUE( memcmp( str, "123456789\0ABCDEFGHI\0abcdefghi\0", strSize ) == 0 );
      QVERIFY_TRUE( src.eof() );
      QVERIFY_FALSE( src.unexpectedEof() );
   }

   void testReadTo()
   {
      const size_t returnSize = 8;

      AsciiSource src( returnSize );

      // The data size returned wraps at 16 bytes, get onto an odd boundary
      // and make sure reading works properly
      bool found = true;
      const size_t strSize = 32;
      char str[strSize] = "";

      strcpy( str, "000000000000000000000000000" );
      QVERIFY_TRUE( src.readAsciiz( str, strSize, &found ) );
      QVERIFY_TRUE( found );
      QVERIFY_TRUE( memcmp( str, "123456789\0", 10 ) == 0 );
      QVERIFY_EQ( (int8_t) 'A', src.readI8() );
      QVERIFY_EQ( (int8_t) 'B', src.readI8() );
      strcpy( str, "000000000000000000000000000" );
      QVERIFY_TRUE( src.readTo( 'a', str, strSize, &found ) );
      QVERIFY_TRUE( found );
      QVERIFY_TRUE( memcmp( str, "CDEFGHI\0a", 9 ) == 0 );
      strcpy( str, "000000000000000000000000000" );
      QVERIFY_TRUE( src.readAsciiz( str, strSize, &found ) );
      QVERIFY_TRUE( found );
      QVERIFY_TRUE( memcmp( str, "bcdefghi\0", 9 ) == 0 );
      QVERIFY_TRUE( src.eof() );
      QVERIFY_FALSE( src.unexpectedEof() );
      strcpy( str, "000000000000000000000000000" );
      QVERIFY_FALSE( src.readAsciiz( str, strSize, &found ) );
      QVERIFY_EQ( '0', str[0] );
      QVERIFY_FALSE( found );
      QVERIFY_TRUE( src.unexpectedEof() );
   }

   void testReadToNoNull()
   {
      const size_t returnSize = 8;

      AsciiSource src( returnSize );

      // The data size returned wraps at 16 bytes, get onto an odd boundary
      // and make sure reading works properly
      bool found = true;
      const size_t strSize = 8;
      char str[strSize] = "0000000";

      QVERIFY_FALSE( src.readAsciiz( str, strSize, &found ) );
      QVERIFY_FALSE( found );
      QVERIFY_FALSE( src.eof() );
      QVERIFY_FALSE( src.unexpectedEof() );
      QVERIFY_TRUE( memcmp( str, "1234567\0", 8 ) == 0 );

      QVERIFY_TRUE( src.seek(0) );
      QVERIFY_FALSE( src.readTo( '@', str, strSize, &found ) );
      QVERIFY_FALSE( found );
      QVERIFY_FALSE( src.eof() );
      QVERIFY_FALSE( src.unexpectedEof() );
      QVERIFY_TRUE( memcmp( str, "12345678", 8 ) == 0 );
   }

   void testReadToCharNotFound()
   {
      const size_t returnSize = 8;

      AsciiSource src( returnSize );

      // The data size returned wraps at 16 bytes, get onto an odd boundary
      // and make sure reading works properly
      bool found = true;
      const size_t strSize = 64;
      char str[strSize] = "";

      for ( size_t t = 0; t < strSize; ++t )
      {
         str[t] = '0';
      }

      // There is no '@' in the input
      QVERIFY_FALSE( src.readTo( '@', str, strSize, &found ) );
      QVERIFY_FALSE( found );
      QVERIFY_EQ( (off_t) 30, src.offset() );
      QVERIFY_TRUE( src.unexpectedEof() );
      QVERIFY_TRUE( memcmp( str, "123456789\0ABCDEFGHI\0abcdefghi\0", 30 ) == 0 );
   }

   void testSourceErrno()
   {
      const size_t fileSize = 1024;
      const size_t returnSize = 64;

      TestSource src( fileSize, returnSize );

      src.setErrorAttempt( 3, EPIPE );

      uint32_t val = 0;

      // Read all of the first two 64 byte blocks returned by the
      // test srouce.
      for ( int t = 0; t < (64 * 2 / 4); ++t )
      {
         QVERIFY_TRUE( src.read(val) );
         QVERIFY_FALSE( src.unexpectedEof() );
      }

      QVERIFY_FALSE( src.eof() );

      // The next read will cause the third read from the test source.
      // The third read is the error attempt, so we should get that
      // error at this time.
      QVERIFY_FALSE( src.read(val) );
      QVERIFY_EQ( (int) EPIPE, src.getErrno() );
      QVERIFY_FALSE( src.unexpectedEof() );
      QVERIFY_FALSE( src.eof() );
   }

};

QTEST_MAIN(DataSourceTest)
#include "datasource_test.moc"
