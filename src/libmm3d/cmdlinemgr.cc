/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2008 Kevin Worcester
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


#include "cmdlinemgr.h"

#include <string.h>

namespace 
{
   class FunctionOption : public CommandLineManager::Option
   {
      public:
         FunctionOption( char shortOption, const char * longOption,
               bool takesArg,
               CommandLineManager::OptionFunctionF optFunc )
            : CommandLineManager::Option( shortOption, longOption, NULL, takesArg ),
              m_optFunc( optFunc )
         {
         }

         virtual ~FunctionOption()
         {
         }

         void customParse( const char * arg )
         {
            if ( m_optFunc )
               m_optFunc( arg );
         }

      private:
         CommandLineManager::OptionFunctionF m_optFunc;
   };
} // namespace

CommandLineManager::Option::Option( char shortOption, const char * longOption,
      const char * defaultValue, bool takesArg )
   : m_shortOption( shortOption ),
     m_longOption( longOption ),
     m_takesArg( takesArg ),
     m_isSpecified( false ),
     m_intValue( 0 ),
     m_stringValue( "" )
{
   if ( defaultValue )
   {
      m_stringValue = defaultValue;
      m_intValue = atoi( defaultValue );
   }
}

CommandLineManager::Option::~Option()
{
}

void CommandLineManager::Option::parseOption( const char * arg )
{
   if ( arg )
   {
      m_isSpecified = true;
      m_stringValue = arg;
      m_intValue = atoi( arg );
      customParse( arg );
   }
}

void CommandLineManager::Option::customParse( const char * arg )
{
   // Do nothing
}

CommandLineManager::CommandLineManager()
   : m_uniqueId( 0 ),
     m_firstArg( 1 ),
     m_error( NoError ),
     m_errorArg( 0 )
{
}

CommandLineManager::~CommandLineManager()
{
}

bool CommandLineManager::parse( int argc, const char ** argv )
{
   for ( int a = 1; a < argc; a++ )
   {
      const char * str = argv[a];

      // If this string doesn't start with a dash, or is just a single
      // dash (stdin as file input), then we're done.
      if ( str[0] != '-' || str[1] == '\0' )
      {
         m_firstArg = a;
         return true;
      }

      if ( strcmp("--", str) == 0 )
      {
         // End of options
         m_firstArg = a + 1;
         return true;
      }

      Option * opt = NULL;
      const char * arg = NULL;
      int nextOpt = a;

      // It is an option
      if ( str[1] == '-' )
      {
         // Long option
         std::string longOpt = &str[2];
         const char * end = strchr( str, '=' );
         if ( end )
         {
            arg = end + 1;
            longOpt.resize( end - &str[2] );
         }
         else
         {
            arg = argv[a+1];
            nextOpt = a + 1;
         }
         opt = lookupOptionByLong( longOpt.c_str() );
      }
      else
      {
         // Short option
         const char * end = strchr( str, '=' );
         if ( end )
         {
            arg = end + 1;
         }
         else
         {
            arg = argv[a+1];
            nextOpt = a + 1;
         }
         opt = lookupOptionByShort( str[1] );
      }

      if ( !opt )
      {
         m_error = UnknownOption;
         m_errorArg = a;
         return false;
      }

      if ( opt->takesArgument() )
      {
         a = nextOpt;
         if ( a >= argc )
         {
            m_error = MissingArgument;
            m_errorArg = a - 1;
            return false;
         }
      }
      else
      {
         arg = "";
      }

      opt->parseOption( arg );
   }

   // Ran out of command line options, there are no arguments
   m_firstArg = argc;
   return true;
}

void CommandLineManager::addOption( int id, char shortOption,
      const char * longOption, const char * defaultValue, bool takesArg )
{
   m_optionList[ id ] = new Option( shortOption, longOption, defaultValue, takesArg );
}

void CommandLineManager::addCustomOption( int id, Option * opt )
{
   m_optionList[ id ] = opt;
}

void CommandLineManager::addFunctionOption( int id, char shortOption,
      const char * longOption, bool takesArg, OptionFunctionF optFunc )
{
   m_optionList[ id ] = new FunctionOption( shortOption, longOption,
         takesArg, optFunc );
}

bool CommandLineManager::isSpecified( int optionId ) const
{
   const Option * opt = lookupOptionById( optionId );

   if ( opt )
      return opt->isSpecified();

   return false;
}

int CommandLineManager::intValue( int optionId ) const
{
   const Option * opt = lookupOptionById( optionId );

   if ( opt )
      return opt->intValue();

   return 0;
}

const char * CommandLineManager::stringValue( int optionId ) const
{
   const Option * opt = lookupOptionById( optionId );

   if ( opt )
      return opt->stringValue();

   return "";
}

int CommandLineManager::getUniqueId()
{
   return --m_uniqueId;
}

const CommandLineManager::Option * CommandLineManager::lookupOptionById( int optionId ) const
{
   OptionListT::const_iterator it = m_optionList.find( optionId );

   if ( it != m_optionList.end() )
      return it->second.get();
   else
      return NULL;
}

CommandLineManager::Option * CommandLineManager::lookupOptionByShort( char shortOption )
{
   for ( OptionListT::iterator it = m_optionList.begin();
         it != m_optionList.end(); ++it )
   {
      const char opt = it->second->shortOption();
      if ( opt != 0 && opt == shortOption )
      {
         return it->second.get();
      }
   }

   return NULL;
}

CommandLineManager::Option * CommandLineManager::lookupOptionByLong( const char * longOption )
{
   for ( OptionListT::iterator it = m_optionList.begin();
         it != m_optionList.end(); ++it )
   {
      const char * opt = it->second->longOption();
      if ( opt && strcmp( opt, longOption ) == 0 )
      {
         return it->second.get();
      }
   }

   return NULL;
}

