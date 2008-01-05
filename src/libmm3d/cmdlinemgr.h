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


#ifndef CMDLINEMGR_H_INC__
#define CMDLINEMGR_H_INC__

#include <string>
#include <map>

#include "local_ptr.h"

class CommandLineManager
{
   public:
      class Option
      {
         public:
            Option( char shortOption, const char * longOption = NULL,
                  const char * defaultValue = NULL,
                  bool takesArg = false );
            virtual ~Option();

            char shortOption() const { return m_shortOption; }
            const char * longOption() const { return m_longOption; }
            bool takesArgument() const { return m_takesArg; }

            void parseOption(const char * arg);

            int intValue() const { return m_intValue; }
            const char * stringValue() const { return m_stringValue.c_str(); }

            bool isSpecified() const { return m_isSpecified; }

         protected:
            void setIntValue(int val) { m_intValue = val; }
            void setStringValue(const char * val) { m_stringValue = val; }

            virtual void customParse(const char * arg);

         private:
            char m_shortOption;
            const char * m_longOption;
            bool m_takesArg;

            bool m_isSpecified;

            int m_intValue;
            std::string m_stringValue;
      };

      enum ErrorE 
      {
         NoError,
         UnknownOption,
         MissingArgument,
      };

      typedef void (*OptionFunctionF)(const char *);

      CommandLineManager();
      virtual ~CommandLineManager();

      bool parse( int argc, const char ** argv );
      int firstArgument() const { return m_firstArg; }

      ErrorE error() const { return m_error; }
      int errorArgument() const { return m_errorArg; }

      void addOption( int id, char shortOption, const char * longOption = NULL,
            const char * defaultValue = NULL, bool takesArg = false );
      void addCustomOption( int id, Option * opt );
      void addFunctionOption( int id, char shortOption, const char * longOption, bool takesArg,
            OptionFunctionF optFunc );

      bool isSpecified( int optionId ) const;
      int intValue( int optionId ) const;
      const char * stringValue( int optionId ) const;

      int getUniqueId();

   private:
      typedef std::map< int, local_ptr<Option> > OptionListT;

      const Option * lookupOptionById( int optionId ) const;
      Option * lookupOptionByShort( char shortOption );
      Option * lookupOptionByLong( const char * longOption );

      OptionListT m_optionList;
      int m_uniqueId;
      int m_firstArg;

      ErrorE m_error;
      int m_errorArg;
};

#endif // CMDLINEMGR_H_INC__
