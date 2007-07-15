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

#include "asciifilter.h"

#include "model.h"
#include "texture.h"
#include "texmgr.h"
#include "log.h"
#include "binutil.h"
#include "misc.h"
#include "filtermgr.h"

#include "mm3dport.h"
#include "endianconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <vector>

AsciiFilter::AsciiFilter()
{
}

AsciiFilter::~AsciiFilter()
{
}

bool AsciiFilter::readLine( char * dest, size_t len )
{
   if ( fgets( dest, len, m_fp ) )
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool AsciiFilter::readStrippedLine( char * dest, size_t len )
{
   while ( readLine( dest, len ) )
   {
      stripWhitespace( dest );

      if ( dest[0] != '\0' )
      {
         return true;
      }
   }
   return false;
}


void AsciiFilter::chomp( char * str )
{
   int len = strlen( str );
   while ( len-1 >= 0 
         && (str[len-1] == '\r' || str[len-1] == '\n' ) )
   {
      str[len-1] = '\0';
      len--;
   }
}

void AsciiFilter::stripWhitespace( char * str )
{
   int len = strlen( str );

   int offset = 0;
   while ( isspace( str[offset] ) )
   {
      offset++;
   }

   if ( offset > 0 )
   {
      memmove( str, &str[offset], len - offset );
      len -= offset;
   }

   while ( len-1 >= 0 && isspace( str[len-1] ) )
   {
      str[len-1] = '\0';
   }
}

char * AsciiFilter::skipWhitespace( char * str )
{
   while ( isspace( *str ) )
   {
      str++;
   }
   return str;
}

char * AsciiFilter::skipDelimiters( char * str, const char * delim )
{
   if ( str )
   {
      size_t len = strspn( str, delim );
      return &str[len];
   }
   return str;
}

bool AsciiFilter::readInt( int & val, char * str, size_t bytes )
{
   int len = 0;
   if ( sscanf( str, "%d%n", &val, &len ) > 0 )
   {
      bytes = len;
      return true;
   }
   return false;
}

bool AsciiFilter::readNumber( int & val, char * str, size_t bytes )
{
   int len = 0;
   if ( sscanf( str, "%i%n", &val, &len ) > 0 )
   {
      bytes = len;
      return true;
   }
   return false;
}

bool AsciiFilter::readFloat( float & val, char * str, size_t bytes )
{
   int len = 0;
   if ( sscanf( str, "%f%n", &val, &len ) > 0 )
   {
      bytes = len;
      return true;
   }
   return false;
}

bool AsciiFilter::readWord( std::string & val, char * str, size_t bytes )
{
   if ( str )
   {
      int len = 0;
      while ( isspace( str[len] ) )
      {
         len++;
      }
      bytes = len;

      if ( bytes > 0 )
      {
         val.assign( str, len );
         return true;
      }
   }
   return false;
}

bool AsciiFilter::readToken( std::string & val, char * str, const char * delim, size_t bytes )
{
   size_t len = strcspn( str, delim );

   if ( len > 0 )
   {
      val.assign( str, len );
      bytes = len;
      return true;
   }
   else
   {
      val = "";
      return false;
   }
}

