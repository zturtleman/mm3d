/*  Misfit Model 3D
 *
 *  Copyright (c) 2004-2007 Kevin Worcester
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

#ifndef __ASCIIFILTER_H
#define __ASCIIFILTER_H

#include "modelfilter.h"

#include <string>

#include <stdint.h>

class AsciiFilter : public ModelFilter
{
   public:
      AsciiFilter();
      virtual ~AsciiFilter();

   protected:

      bool readLine( char *, size_t len );

      // This function reads the next non-blank line and
      // removes leading and trailing whitespace before returning
      bool readStrippedLine( char *, size_t len );

      void chomp( char * ); // removing trailing CR/LF
      void stripWhitespace( char * ); // remove leading and trailing whitespace

      char * skipWhitespace( char * );
      char * skipDelimiters( char *, const char * delim ); // skip any characters in delim

      // These functions read a value from str, store the value in 'val',
      // store the number of bytes read in 'bytes', and return 
      // true if successful or false if failed
      bool readInt( int & val, char * str, size_t bytes );
      bool readNumber( int & val, char * str, size_t bytes ); // Translates 0xnnn to hex and 0nnn to octal
      bool readFloat( float & val, char * str, size_t bytes );
      bool readWord( std::string & val, char * str, size_t bytes );
      bool readToken( std::string & val, char * str, const char * delim, size_t bytes ); // read all characters until any char in 'delim' is found

      FILE * m_fp;

};
#endif // __ASCIIFILTER_H
