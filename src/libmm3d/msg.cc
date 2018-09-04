/*  Maverick Model 3D
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


#include "msg.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

static msg_func _user_info;
static msg_func _user_warn;
static msg_func _user_err;

static msg_prompt_func _user_info_prompt;
static msg_prompt_func _user_warn_prompt;
static msg_prompt_func _user_err_prompt;


extern "C" void msg_register( msg_func infomsg, msg_func warnmsg, msg_func errmsg )
{
   _user_info = infomsg;
   _user_warn = warnmsg;
   _user_err = errmsg;
}

extern "C" void msg_register_prompt( msg_prompt_func infomsg, 
      msg_prompt_func warnmsg, msg_prompt_func errmsg )
{
   _user_info_prompt = infomsg;
   _user_warn_prompt = warnmsg;
   _user_err_prompt = errmsg;
}

extern "C" void msg_info( const char * str )
{
   if ( _user_info )
   {
      _user_info( str );
   }
   else
   {
      printf( "info: %s\n", str );
   }
}

extern "C" void msg_warning( const char * str )
{
   if ( _user_warn )
   {
      _user_warn( str );
   }
   else
   {
      printf( "warning: %s\n", str );
   }
}

extern "C" void msg_error( const char * str )
{
   if ( _user_err )
   {
      _user_err( str );
   }
   else
   {
      printf( "error: %s\n", str );
   }
}

static char return_caps( const char * opts )
{
   if ( opts == NULL )
      return '\0';

   size_t len = strlen( opts );

   if ( len == 0 )
      return '\0';

   for ( size_t n = 0; n < len; ++n )
   {
      if ( isupper( opts[n] ) )
         return toupper( opts[n] );
   }
   return toupper( opts[0] );
}

// Do you want to save first (yes, no, cancel) [Y/n/c]?
// Do you want to save first (abort, retry, ignore) [A/r/i]?
extern "C" char msg_info_prompt( const char * str, const char * opts )
{
   if ( _user_info_prompt )
   {
      return _user_info_prompt( str, opts );
   }
   else
   {
      printf( "%s\n", str );
      return return_caps(opts);
   }
}

extern "C" char msg_warning_prompt( const char * str, const char * opts )
{
   if ( _user_warn_prompt )
   {
      return _user_warn_prompt( str, opts );
   }
   else
   {
      printf( "%s\n", str );
      return return_caps(opts);
   }
}

extern "C" char msg_error_prompt( const char * str, const char * opts )
{
   if ( _user_err_prompt )
   {
      return _user_err_prompt( str, opts );
   }
   else
   {
      printf( "%s\n", str );
      return return_caps(opts);
   }
}

