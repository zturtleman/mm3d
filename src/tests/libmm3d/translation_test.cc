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


// This file tests translate.cc and mlocale.cc

#include <QtTest/QtTest>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "test_common.h"

#include "translate.h"
#include "mlocale.h"

std::string test_translate_function( const char * sourceStr )
{
   if ( strcmp( sourceStr, "foo" ) == 0 )
      return "goo";
   else if ( strcmp( sourceStr, "bar" ) == 0 )
      return "ber";
   return sourceStr;
}

class TranslationTest : public QObject
{
   Q_OBJECT
private slots:

   void testNoTranslate()
   {
      transll_install_handler( NULL );
      QVERIFY_EQ( std::string("foo"), transll(QT_TRANSLATE_NOOP("Test", "foo")) );
      QVERIFY_EQ( std::string("bar"), transll(QT_TRANSLATE_NOOP("Test", "bar")) );
      QVERIFY_EQ( std::string("baz"), transll(QT_TRANSLATE_NOOP("Test", "baz")) );
   }

   void testTranslate()
   {
      transll_install_handler( test_translate_function );
      QVERIFY_EQ( std::string("goo"), transll(QT_TRANSLATE_NOOP("Test", "foo")) );
      QVERIFY_EQ( std::string("ber"), transll(QT_TRANSLATE_NOOP("Test", "bar")) );
      QVERIFY_EQ( std::string("baz"), transll(QT_TRANSLATE_NOOP("Test", "baz")) );
      transll_install_handler( NULL );
   }

   void testLocale()
   {
      QVERIFY_EQ( std::string(""), mlocale_get() );
      mlocale_set("en-GB");
      QVERIFY_EQ( std::string("en-GB"), mlocale_get() );
      mlocale_set("");
      QVERIFY_EQ( std::string(""), mlocale_get() );
   }
};

QTEST_MAIN(TranslationTest)
#include "translation_test.moc"
