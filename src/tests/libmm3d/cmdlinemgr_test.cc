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


// This file tests the CommandLineManager class.

#include <QtTest/QtTest>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "test_common.h"

#include "cmdlinemgr.h"

static bool ls_no_arg_called = false;
static bool ls_arg_called = false;

void NoArgOptFunction( const char * arg )
{
   ls_no_arg_called = true;
   QVERIFY_EQ( std::string(""), std::string(arg) );
}

void ArgOptFunction( const char * arg )
{
   ls_arg_called = true;
   QVERIFY_EQ( std::string("opt_arg"), std::string(arg) );
}

class CommandLineManagerTest : public QObject
{
   Q_OBJECT

private:
   void addStandardOptions( CommandLineManager * clm )
   {
      clm->addOption( 1, 'q', NULL, NULL, false );
      clm->addOption( 2, 'o', "outfile", NULL, true );
      clm->addOption( 3, 0, "verbose", "", false );
      clm->addOption( 4, 0, "level", "5", true );
   }

   class CustomOption : public CommandLineManager::Option
   {
      public:
         CustomOption( char shortOption, const char * longOption, bool takesArg,
               const char * expectedArg )
            : CommandLineManager::Option( shortOption, longOption, NULL, takesArg ),
              m_called( false ),
              m_expectedArg( expectedArg )
         {
            ++s_allocated;
         }

         virtual ~CustomOption()
         {
            --s_allocated;
         }

         void customParse( const char * arg )
         {
            m_called = true;
            QVERIFY_EQ( m_expectedArg, std::string(arg) );
            setStringValue( "custom" );
            setIntValue( 19 );
         }

         bool called() const { return m_called; }

         static int s_allocated;

      private:
         bool m_called;
         std::string m_expectedArg;
   };

private slots:
   void testEmpty()
   {
      const int kArgs = 1;
      const char *argv[kArgs] = {
         "./program",
      };

      CommandLineManager clm;
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( kArgs, clm.firstArgument() );
   }

   void testEmptyWithArgs()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "foo",
         "bar"
      };

      CommandLineManager clm;
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( 1, clm.firstArgument() );
   }

   void testNoOptionsOrArgs()
   {
      const int kArgs = 1;
      const char *argv[kArgs] = {
         "./program",
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( kArgs, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
   }

   void testNoOptions()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "foo",
         "bar",
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( 1, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
   }

   void testNoArgs()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "-q",
         "--verbose",
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( kArgs, clm.firstArgument() );
      QVERIFY_TRUE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_TRUE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
   }

   void testOptionEqualArg()
   {
      const int kArgs = 2;
      const char *argv[kArgs] = {
         "./program",
         "--level=14",
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_EQ( 5, clm.intValue(4) );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( kArgs, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_TRUE( clm.isSpecified( 4 ) );
      QVERIFY_EQ( 14, clm.intValue(4) );
      QVERIFY_EQ( std::string("14"), std::string(clm.stringValue(4)) );
   }

   void testOptionSpaceArg()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "--level",
         "14",
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_EQ( 5, clm.intValue(4) );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( kArgs, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_TRUE( clm.isSpecified( 4 ) );
      QVERIFY_EQ( 14, clm.intValue(4) );
      QVERIFY_EQ( std::string("14"), std::string(clm.stringValue(4)) );
   }

   void testOptionMissingArg()
   {
      const int kArgs = 2;
      const char *argv[kArgs] = {
         "./program",
         "--level",
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_FALSE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( CommandLineManager::MissingArgument == clm.error() );
      QVERIFY_EQ( 1, clm.errorArgument() );
   }

   void testOptionArgOption()
   {
      const int kArgs = 4;
      const char *argv[kArgs] = {
         "./program",
         "--level",
         "14",
         "-q"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_EQ( 5, clm.intValue(4) );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( kArgs, clm.firstArgument() );
      QVERIFY_TRUE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_TRUE( clm.isSpecified( 4 ) );
      QVERIFY_EQ( 14, clm.intValue(4) );
      QVERIFY_EQ( std::string("14"), std::string(clm.stringValue(4)) );
   }

   void testOptionFollowsArg()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "filename.txt",
         "-q"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( 1, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
   }

   void testOptionFollowsDash()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "-",
         "-q"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( 1, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
   }

   void testOptionFollowsDoubleDash()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "--",
         "-q"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
   }

   void testNoArgOptionFunction()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "--func",
         "something"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addFunctionOption( 5, 'f', "func", false, NoArgOptFunction );

      ls_no_arg_called = false;
      ls_arg_called = false;

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( ls_no_arg_called );
      QVERIFY_FALSE( ls_arg_called );

      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );
   }

   void testArgOptionFunction()
   {
      const int kArgs = 4;
      const char *argv[kArgs] = {
         "./program",
         "--func",
         "opt_arg",
         "something"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addFunctionOption( 5, 'f', "func", true, ArgOptFunction );

      ls_no_arg_called = false;
      ls_arg_called = false;

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_FALSE( ls_no_arg_called );
      QVERIFY_TRUE( ls_arg_called );

      QVERIFY_EQ( 3, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );
   }

   void testEmptyArgOptionFunction()
   {
      const int kArgs = 4;
      const char *argv[kArgs] = {
         "./program",
         "--func=",
         "something"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      // This function verifies that the option is empty, which is
      // really what we want to know
      clm.addFunctionOption( 5, 'f', "func", true, NoArgOptFunction );

      ls_no_arg_called = false;
      ls_arg_called = false;

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( ls_no_arg_called );
      QVERIFY_FALSE( ls_arg_called );

      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );
   }

   void testNoArgCustom()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "--cust",
         "something"
      };

      CustomOption * opt = new CustomOption( 'c', "cust", false, "" );
      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addCustomOption( 5, opt );

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( opt->called() );

      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );

      QVERIFY_EQ( 19, clm.intValue( 5 ) );
      QVERIFY_EQ( std::string("custom"), std::string(clm.stringValue( 5 )) );
   }

   void testEmptyArgCustom()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "--cust=",
         "something"
      };

      CustomOption * opt = new CustomOption( 'c', "cust", true, "" );
      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addCustomOption( 5, opt );

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( opt->called() );

      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );

      QVERIFY_EQ( 19, clm.intValue( 5 ) );
      QVERIFY_EQ( std::string("custom"), std::string(clm.stringValue( 5 )) );
   }

   void testEqualArgCustom()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "--cust=expected",
         "something"
      };

      CustomOption * opt = new CustomOption( 'c', "cust", true, "expected" );
      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addCustomOption( 5, opt );

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( opt->called() );

      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );

      QVERIFY_EQ( 19, clm.intValue( 5 ) );
      QVERIFY_EQ( std::string("custom"), std::string(clm.stringValue( 5 )) );
   }

   void testSpaceArgCustom()
   {
      const int kArgs = 4;
      const char *argv[kArgs] = {
         "./program",
         "--cust",
         "expected",
         "something"
      };

      CustomOption * opt = new CustomOption( 'c', "cust", true, "expected" );
      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addCustomOption( 5, opt );

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( opt->called() );

      QVERIFY_EQ( 3, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );

      QVERIFY_EQ( 19, clm.intValue( 5 ) );
      QVERIFY_EQ( std::string("custom"), std::string(clm.stringValue( 5 )) );
   }

   void testShortArgOptionFunction()
   {
      const int kArgs = 4;
      const char *argv[kArgs] = {
         "./program",
         "-f",
         "opt_arg",
         "something"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addFunctionOption( 5, 'f', "func", true, ArgOptFunction );

      ls_no_arg_called = false;
      ls_arg_called = false;

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_FALSE( ls_no_arg_called );
      QVERIFY_TRUE( ls_arg_called );

      QVERIFY_EQ( 3, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );
   }

   void testEmptyShortArgOptionFunction()
   {
      const int kArgs = 4;
      const char *argv[kArgs] = {
         "./program",
         "-f=",
         "something"
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      // This function verifies that the option is empty, which is
      // really what we want to know
      clm.addFunctionOption( 5, 'f', "func", true, NoArgOptFunction );

      ls_no_arg_called = false;
      ls_arg_called = false;

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( ls_no_arg_called );
      QVERIFY_FALSE( ls_arg_called );

      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );
   }

   void testEmptyShortArgCustom()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "-c=",
         "something"
      };

      CustomOption * opt = new CustomOption( 'c', "cust", true, "" );
      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addCustomOption( 5, opt );

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( opt->called() );

      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );

      QVERIFY_EQ( 19, clm.intValue( 5 ) );
      QVERIFY_EQ( std::string("custom"), std::string(clm.stringValue( 5 )) );
   }

   void testEqualShortArgCustom()
   {
      const int kArgs = 3;
      const char *argv[kArgs] = {
         "./program",
         "-c=expected",
         "something"
      };

      CustomOption * opt = new CustomOption( 'c', "cust", true, "expected" );
      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addCustomOption( 5, opt );

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( opt->called() );

      QVERIFY_EQ( 2, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );

      QVERIFY_EQ( 19, clm.intValue( 5 ) );
      QVERIFY_EQ( std::string("custom"), std::string(clm.stringValue( 5 )) );
   }

   void testSpaceShortArgCustom()
   {
      const int kArgs = 4;
      const char *argv[kArgs] = {
         "./program",
         "-c",
         "expected",
         "something"
      };

      CustomOption * opt = new CustomOption( 'c', "cust", true, "expected" );
      CommandLineManager clm;
      addStandardOptions( &clm );
      clm.addCustomOption( 5, opt );

      QVERIFY_TRUE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( opt->called() );

      QVERIFY_EQ( 3, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 1 ) );
      QVERIFY_FALSE( clm.isSpecified( 2 ) );
      QVERIFY_FALSE( clm.isSpecified( 3 ) );
      QVERIFY_FALSE( clm.isSpecified( 4 ) );
      QVERIFY_TRUE( clm.isSpecified( 5 ) );

      QVERIFY_EQ( 19, clm.intValue( 5 ) );
      QVERIFY_EQ( std::string("custom"), std::string(clm.stringValue( 5 )) );
   }

   void testOptionMemory()
   {
      // If an option with the same ID number is added, that option should
      // be deleted. If the clm goes out of scope, all option objects should
      // be deleted.
      {
         CommandLineManager clm;
         QVERIFY_EQ( 0, CustomOption::s_allocated );
         clm.addCustomOption( 1, new CustomOption( 'c', "cust", true, "" ) );
         QVERIFY_EQ( 1, CustomOption::s_allocated );
         clm.addCustomOption( 2, new CustomOption( 'c', "cust", true, "" ) );
         QVERIFY_EQ( 2, CustomOption::s_allocated );
         clm.addCustomOption( 1, new CustomOption( 'c', "cust", true, "" ) );
         QVERIFY_EQ( 2, CustomOption::s_allocated );
         clm.addOption( 1, 'h' );
         QVERIFY_EQ( 1, CustomOption::s_allocated );
      }
      QVERIFY_EQ( 0, CustomOption::s_allocated );
   }

   void testUniqueId()
   {
      // If these are not unique, the allocation will not increase.
      {
         CommandLineManager clm;
         QVERIFY_EQ( 0, CustomOption::s_allocated );
         clm.addCustomOption( clm.getUniqueId(), new CustomOption( 'c', "cust", true, "" ) );
         QVERIFY_EQ( 1, CustomOption::s_allocated );
         clm.addCustomOption( clm.getUniqueId(), new CustomOption( 'c', "cust", true, "" ) );
         QVERIFY_EQ( 2, CustomOption::s_allocated );
         clm.addCustomOption( clm.getUniqueId(), new CustomOption( 'c', "cust", true, "" ) );
         QVERIFY_EQ( 3, CustomOption::s_allocated );
      }
      QVERIFY_EQ( 0, CustomOption::s_allocated );
   }

   void testUnknownShortOption()
   {
      const int kArgs = 2;
      const char *argv[kArgs] = {
         "./program",
         "-h"
      };

      CommandLineManager clm;
      QVERIFY_FALSE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( CommandLineManager::UnknownOption == clm.error() );
      QVERIFY_EQ( 1, clm.errorArgument() );
   }

   void testUnknownLongOption()
   {
      const int kArgs = 2;
      const char *argv[kArgs] = {
         "./program",
         "--long-option"
      };

      CommandLineManager clm;
      QVERIFY_FALSE( clm.parse( kArgs, argv ) );

      QVERIFY_TRUE( CommandLineManager::UnknownOption == clm.error() );
      QVERIFY_EQ( 1, clm.errorArgument() );
   }

   void testOutOfRange()
   {
      const int kArgs = 1;
      const char *argv[kArgs] = {
         "./program",
      };

      CommandLineManager clm;
      addStandardOptions( &clm );
      QVERIFY_TRUE( clm.parse( kArgs, argv ) );
      QVERIFY_EQ( kArgs, clm.firstArgument() );
      QVERIFY_FALSE( clm.isSpecified( 5 ) );
      QVERIFY_EQ( 0, clm.intValue( 5 ) );
      QVERIFY_EQ( std::string(""), std::string(clm.stringValue( 5 )) );
   }

};

/* static */
int CommandLineManagerTest::CustomOption::s_allocated = 0;

QTEST_MAIN(CommandLineManagerTest)
#include "cmdlinemgr_test.moc"
