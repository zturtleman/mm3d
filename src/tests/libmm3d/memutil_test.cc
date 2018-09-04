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

#include "local_ptr.h"
#include "local_array.h"
#include "release_ptr.h"
#include "file_closer.h"

class CountObject
{
   public:
      CountObject( int val = 0 ) : m_val(val) { ++s_count; }
      ~CountObject() { --s_count; }

      static int s_count;
      int m_val;
};

/* static */
int CountObject::s_count = 0;

class ReleaseObject
{
   public:
      ReleaseObject( int val = 0 ) : m_val(val) { ++s_count; }
      void release() const { delete this; }

      static int s_count;
      int m_val;

   private:
      ~ReleaseObject() { --s_count; }
};

/* static */
int ReleaseObject::s_count = 0;

class MemUtilTest : public QObject
{
   Q_OBJECT
private slots:
   void testLocalPtr()
   {
      QVERIFY_EQ( 0, CountObject::s_count );

      // Mutable
      {
         local_ptr< CountObject > ptr( new CountObject(1) );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 1, CountObject::s_count );

         CountObject * obj = new CountObject(2);
         QVERIFY_EQ( 2, CountObject::s_count );
         QVERIFY_EQ( 1, ptr->m_val );
         QVERIFY_EQ( 1, (*ptr).m_val );

         CountObject * temp = ptr = obj;
         QVERIFY_EQ( 1, CountObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 2, ptr->m_val );
         QVERIFY_EQ( 2, temp->m_val );

         temp = NULL;

         ptr.reset( NULL );
         QVERIFY_EQ( 0, CountObject::s_count );
         QVERIFY_TRUE( ptr.isnull() );
         QVERIFY_TRUE( !ptr );
         QVERIFY_TRUE( NULL == ptr.get() );

         ptr = new CountObject(3);
         QVERIFY_EQ( 1, CountObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 3, ptr->m_val );

         ptr->m_val = 4;
         QVERIFY_EQ( 4, ptr->m_val );
      }

      QVERIFY_EQ( 0, CountObject::s_count );

      // Mutable, const object
      {
         local_ptr< const CountObject > ptr = new CountObject(1);
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 1, CountObject::s_count );

         CountObject * obj = new CountObject(2);
         QVERIFY_EQ( 2, CountObject::s_count );
         QVERIFY_EQ( 1, ptr->m_val );
         QVERIFY_EQ( 1, (*ptr).m_val );

         const CountObject * temp = ptr = obj;
         QVERIFY_EQ( 1, CountObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 2, ptr->m_val );
         QVERIFY_EQ( 2, temp->m_val );

         temp = NULL;

         ptr.reset( NULL );
         QVERIFY_EQ( 0, CountObject::s_count );
         QVERIFY_TRUE( ptr.isnull() );
         QVERIFY_TRUE( !ptr );
         QVERIFY_TRUE( NULL == ptr.get() );

         ptr = new CountObject(3);
         QVERIFY_EQ( 1, CountObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 3, ptr->m_val );
      }

      QVERIFY_EQ( 0, CountObject::s_count );

      // Const
      {
         const local_ptr< CountObject > ptr = new CountObject(1);
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 1, CountObject::s_count );
         QVERIFY_EQ( 1, ptr->m_val );
         QVERIFY_EQ( 1, (*ptr).m_val );
      }

      QVERIFY_EQ( 0, CountObject::s_count );
   }

   void testLocalArray()
   {
      QVERIFY_EQ( 0, CountObject::s_count );

      // Mutable
      {
         local_array< CountObject > ptr( new CountObject[10] );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 10, CountObject::s_count );

         CountObject * obj = new CountObject[20];
         obj[0].m_val = 7;
         QVERIFY_EQ( 30, CountObject::s_count );
         QVERIFY_EQ( 0, ptr.get()[0].m_val );

         CountObject * temp = ptr = obj;
         QVERIFY_EQ( 20, CountObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 7, ptr.get()[0].m_val );
         QVERIFY_EQ( 7, temp[0].m_val );

         temp = NULL;

         ptr.reset( NULL );
         QVERIFY_EQ( 0, CountObject::s_count );
         QVERIFY_TRUE( ptr.isnull() );
         QVERIFY_TRUE( !ptr );
         QVERIFY_TRUE( NULL == ptr.get() );

         ptr = new CountObject[5];
         QVERIFY_EQ( 5, CountObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
      }

      QVERIFY_EQ( 0, CountObject::s_count );

      // Mutable, const object
      {
         local_array< CountObject > ptr( new CountObject[10] );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 10, CountObject::s_count );

         CountObject * obj = new CountObject[20];
         obj[0].m_val = 7;
         QVERIFY_EQ( 30, CountObject::s_count );
         QVERIFY_EQ( 0, ptr.get()[0].m_val );

         const CountObject * temp = ptr = obj;
         QVERIFY_EQ( 20, CountObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 7, ptr.get()[0].m_val );
         QVERIFY_EQ( 7, temp[0].m_val );

         temp = NULL;

         ptr.reset( NULL );
         QVERIFY_EQ( 0, CountObject::s_count );
         QVERIFY_TRUE( ptr.isnull() );
         QVERIFY_TRUE( !ptr );
         QVERIFY_TRUE( NULL == ptr.get() );

         ptr = new CountObject[5];
         QVERIFY_EQ( 5, CountObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
      }

      QVERIFY_EQ( 0, CountObject::s_count );

      // Const
      {
         const local_array< CountObject > ptr( new CountObject[10] );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 10, CountObject::s_count );
         QVERIFY_EQ( 0, ptr.get()[0].m_val );
      }

      QVERIFY_EQ( 0, CountObject::s_count );
   }

   void testReleasePtr()
   {
      QVERIFY_EQ( 0, ReleaseObject::s_count );

      // Mutable
      {
         release_ptr< ReleaseObject > ptr( new ReleaseObject(1) );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 1, ReleaseObject::s_count );

         ReleaseObject * obj = new ReleaseObject(2);
         QVERIFY_EQ( 2, ReleaseObject::s_count );
         QVERIFY_EQ( 1, ptr->m_val );
         QVERIFY_EQ( 1, (*ptr).m_val );

         ReleaseObject * temp = ptr = obj;
         QVERIFY_EQ( 1, ReleaseObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 2, ptr->m_val );
         QVERIFY_EQ( 2, temp->m_val );

         temp = NULL;

         ptr.reset( NULL );
         QVERIFY_EQ( 0, ReleaseObject::s_count );
         QVERIFY_TRUE( ptr.isnull() );
         QVERIFY_TRUE( !ptr );
         QVERIFY_TRUE( NULL == ptr.get() );

         ptr = new ReleaseObject(3);
         QVERIFY_EQ( 1, ReleaseObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 3, ptr->m_val );

         ptr->m_val = 4;
         QVERIFY_EQ( 4, ptr->m_val );
      }

      QVERIFY_EQ( 0, ReleaseObject::s_count );

      // Mutable, const object
      {
         release_ptr< const ReleaseObject > ptr = new ReleaseObject(1);
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 1, ReleaseObject::s_count );

         ReleaseObject * obj = new ReleaseObject(2);
         QVERIFY_EQ( 2, ReleaseObject::s_count );
         QVERIFY_EQ( 1, ptr->m_val );
         QVERIFY_EQ( 1, (*ptr).m_val );

         const ReleaseObject * temp = ptr = obj;
         QVERIFY_EQ( 1, ReleaseObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 2, ptr->m_val );
         QVERIFY_EQ( 2, temp->m_val );

         temp = NULL;

         ptr.reset( NULL );
         QVERIFY_EQ( 0, ReleaseObject::s_count );
         QVERIFY_TRUE( ptr.isnull() );
         QVERIFY_TRUE( !ptr );
         QVERIFY_TRUE( NULL == ptr.get() );

         ptr = new ReleaseObject(3);
         QVERIFY_EQ( 1, ReleaseObject::s_count );
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 3, ptr->m_val );
      }

      QVERIFY_EQ( 0, ReleaseObject::s_count );

      // Const
      {
         const release_ptr< ReleaseObject > ptr = new ReleaseObject(1);
         QVERIFY_FALSE( ptr.isnull() );
         QVERIFY_FALSE( !ptr );
         QVERIFY_EQ( 1, ReleaseObject::s_count );
         QVERIFY_EQ( 1, ptr->m_val );
         QVERIFY_EQ( 1, (*ptr).m_val );
      }

      QVERIFY_EQ( 0, ReleaseObject::s_count );
   }

   void testFileCloser()
   {
      QVERIFY_EQ( 0, CountObject::s_count );

      int fd;

      // Use lseek to tell if the file descriptor associated with the
      // stream is invalid (stream closed).

      {
         FILE * fp = fopen( "memutil_test.cc", "r" );
         QVERIFY_TRUE( fp != NULL );
         if ( !fp )
            return;

         fd = fileno( fp );
         file_closer fc( fp );

         QVERIFY_EQ( 0, (int) lseek(fd, 0, SEEK_SET ) );

         FILE * fp2 = fopen( "memutil_test.cc", "r" );
         QVERIFY_TRUE( fp2 != NULL );
         if ( !fp2 )
            return;

         // This should close the original FILE*
         fc = fp2;
         QVERIFY_NE( 0, (int) lseek(fd, 0, SEEK_SET ) );
         QVERIFY_EQ( EBADF, errno );

         fd = fileno( fp2 );
         QVERIFY_EQ( 0, (int) lseek(fd, 0, SEEK_SET ) );

         // Going out of scope will close the second FILE*
      }

      QVERIFY_NE( 0, (int) lseek(fd, 0, SEEK_SET ) );
      QVERIFY_EQ( EBADF, errno );
   }

};

QTEST_MAIN(MemUtilTest)
#include "memutil_test.moc"
