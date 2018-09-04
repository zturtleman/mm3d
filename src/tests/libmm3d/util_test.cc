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


// This file tests libmm3d/util.h

#include <QtTest/QtTest>

#include "test_common.h"

#include "util.h"

class UtilTest : public QObject
{
   Q_OBJECT
private slots:
   void testClamp()
   {
      QVERIFY_EQ( 5, util_clamp(5, 2, 7 ) );
      QVERIFY_EQ( 2, util_clamp(2, 2, 7 ) );
      QVERIFY_EQ( 7, util_clamp(7, 2, 7 ) );
      QVERIFY_EQ( 2, util_clamp(1, 2, 7 ) );
      QVERIFY_EQ( 7, util_clamp(8, 2, 7 ) );
   }
   void testWrapUp()
   {
      QVERIFY_EQ( 5, util_wrap_up(5, 2, 7 ) );
      QVERIFY_EQ( 2, util_wrap_up(2, 2, 7 ) );
      QVERIFY_EQ( 7, util_wrap_up(7, 2, 7 ) );
      QVERIFY_EQ( 2, util_wrap_up(1, 2, 7 ) );
      QVERIFY_EQ( 2, util_wrap_up(8, 2, 7 ) );
   }
   void testWrapDown()
   {
      QVERIFY_EQ( 5, util_wrap_down(5, 2, 7 ) );
      QVERIFY_EQ( 2, util_wrap_down(2, 2, 7 ) );
      QVERIFY_EQ( 7, util_wrap_down(7, 2, 7 ) );
      QVERIFY_EQ( 7, util_wrap_down(1, 2, 7 ) );
      QVERIFY_EQ( 7, util_wrap_down(8, 2, 7 ) );
   }
};

QTEST_MAIN(UtilTest)
#include "util_test.moc"
