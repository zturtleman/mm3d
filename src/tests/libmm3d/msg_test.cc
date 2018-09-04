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


// This file tests msg.cc

#include <QtTest/QtTest>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <string>

#include "test_common.h"

#include "msg.h"

static int info_msg_count = 0;
static int warning_msg_count = 0;
static int error_msg_count = 0;

static std::string display_msg = "";

void custom_info_msg( const char * msg )
{
   ++info_msg_count;
   display_msg = msg;
}

void custom_warning_msg( const char * msg )
{
   ++warning_msg_count;
   display_msg = msg;
}

void custom_error_msg( const char * msg )
{
   ++error_msg_count;
   display_msg = msg;
}

char prompt_yes( const char * str, const char * opts )
{
   display_msg = str;
   return 'Y';
}

char prompt_no( const char * str, const char * opts )
{
   display_msg = str;
   return 'N';
}

char prompt_cancel( const char * str, const char * opts )
{
   display_msg = str;
   return 'C';
}

class MessageTest : public QObject
{
   Q_OBJECT
private slots:

   void init()
   {
      msg_register( NULL, NULL, NULL );
      display_msg = "";
      info_msg_count = 0;
      warning_msg_count = 0;
      error_msg_count = 0;
   }

   void testInfoMessage()
   {
      const std::string msg = "Info message";

      msg_info( msg.c_str() );
      QVERIFY_EQ( std::string(""), display_msg );
      QVERIFY_EQ( 0, info_msg_count );

      msg_register( custom_info_msg, custom_warning_msg, custom_error_msg );
      msg_info( msg.c_str() );
      QVERIFY_EQ( msg, display_msg );
      QVERIFY_EQ( 1, info_msg_count );
      QVERIFY_EQ( 0, warning_msg_count );
      QVERIFY_EQ( 0, error_msg_count );
   }

   void testWarningMessage()
   {
      const std::string msg = "Warning message";

      msg_warning( msg.c_str() );
      QVERIFY_EQ( std::string(""), display_msg );
      QVERIFY_EQ( 0, info_msg_count );

      msg_register( custom_info_msg, custom_warning_msg, custom_error_msg );
      msg_warning( msg.c_str() );
      QVERIFY_EQ( msg, display_msg );
      QVERIFY_EQ( 0, info_msg_count );
      QVERIFY_EQ( 1, warning_msg_count );
      QVERIFY_EQ( 0, error_msg_count );
   }

   void testErrorMessage()
   {
      const std::string msg = "Error message";

      msg_error( msg.c_str() );
      QVERIFY_EQ( std::string(""), display_msg );
      QVERIFY_EQ( 0, info_msg_count );

      msg_register( custom_info_msg, custom_warning_msg, custom_error_msg );
      msg_error( msg.c_str() );
      QVERIFY_EQ( msg, display_msg );
      QVERIFY_EQ( 0, info_msg_count );
      QVERIFY_EQ( 0, warning_msg_count );
      QVERIFY_EQ( 1, error_msg_count );
   }

   void testDefaultPrompt()
   {
      QVERIFY_EQ( 'Y', msg_info_prompt( "Yes message", "Ync" ) );
      QVERIFY_EQ( 'Y', msg_info_prompt( "Yes message", "nYc" ) );
      QVERIFY_EQ( 'Y', msg_info_prompt( "Yes message", "cnY" ) );
      QVERIFY_EQ( 'N', msg_warning_prompt( "No message", "yNc" ) );
      QVERIFY_EQ( 'N', msg_warning_prompt( "No message", "Nyc" ) );
      QVERIFY_EQ( 'N', msg_warning_prompt( "No message", "ycN" ) );
      QVERIFY_EQ( 'C', msg_error_prompt( "Cancel message", "ynC" ) );
      QVERIFY_EQ( 'C', msg_error_prompt( "Cancel message", "yCn" ) );
      QVERIFY_EQ( 'C', msg_error_prompt( "Cancel message", "Cyn" ) );
      QVERIFY_EQ( 'Y', msg_error_prompt( "No default", "ync" ) );
      QVERIFY_EQ( 'C', msg_error_prompt( "No default", "cny" ) );
      QVERIFY_EQ( '\0', msg_error_prompt( "Empty message", "" ) );
      QVERIFY_EQ( '\0', msg_error_prompt( "NULL message", NULL ) );
   }

   void testCustomPrompt()
   {
      msg_register_prompt( prompt_yes, prompt_no, prompt_cancel );

      std::string msg;

      msg = "Info prompt message";
      QVERIFY_EQ( 'Y', msg_info_prompt( msg.c_str() ) );
      QVERIFY_EQ( msg, display_msg );

      msg = "Warning prompt message";
      QVERIFY_EQ( 'N', msg_warning_prompt( msg.c_str() ) );
      QVERIFY_EQ( msg, display_msg );

      msg = "Error prompt message";
      QVERIFY_EQ( 'C', msg_error_prompt( msg.c_str() ) );
      QVERIFY_EQ( msg, display_msg );
   }
};

QTEST_MAIN(MessageTest)
#include "msg_test.moc"
