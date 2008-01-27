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

#include "mm3dport.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#include "config.h"

#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#else
#include <sys/timeb.h>
#endif // HAVE_GETTIMEOFDAY

#include <unistd.h>

char * PORT_getenv( const char * name )
{
#ifdef WIN32
   return "C:/";  // TODO do registry lookup (if it's ever needed)
#else
   return getenv( name );
#endif // WIN32
}

int PORT_lstat( const char * filename, struct stat * buf )
{
#ifdef WIN32
   return stat( filename, buf );
#else
   return lstat( filename, buf );
#endif // WIN32
}

char * PORT_realpath( const char * path, char * resolved_path, size_t len )
{
#ifdef WIN32
   _fullpath( resolved_path, path, len );
#else
   realpath( path, resolved_path );
#endif // WIN32
   return resolved_path;
}

struct tm * PORT_localtime_r( const time_t * timep, struct tm * result )
{
#ifdef WIN32
   *result = *localtime( timep );
#else
   localtime_r( timep, result );
#endif
   return result;
}

void PORT_gettimeofday( PORT_timeval * tv )
{
#ifdef HAVE_GETTIMEOFDAY
   struct timeval tval;
   gettimeofday( &tval, NULL );
   tv->tv_sec  = tval.tv_sec;
   tv->tv_msec = tval.tv_usec / 1000;
#else
   struct timeb tb;
   ftime( &tb );
   tv->tv_sec = tb.time;
   tv->tv_msec = tb.millitm;
#endif // HAVE_GETTIMEOFDAY
}

char * PORT_asctime_r( const struct tm * tmval, char * buf )
{
#ifdef WIN32
   char *tmptmstr = asctime( tmval );
   strcpy( buf, tmptmstr );
#else
   asctime_r( tmval, buf );
#endif
   return buf;
}

int PORT_symlink( const char * oldpath, const char * newpath )
{
#ifdef WIN32
   return 0;
#else
   return symlink( oldpath, newpath );
#endif // WIN32
}

int PORT_mkdir( const char * pathname, mode_t mode )
{
#ifdef WIN32
   return mkdir( pathname );
#else
   return mkdir( pathname, mode );
#endif // WIN32
}

int PORT_snprintf( char * dest, size_t len, const char * fmt, ... )
{
   int rval = -1;
   if ( dest && fmt && len > 0 )
   {
      va_list args;
      va_start( args, fmt );
      rval = PORT_vsnprintf( dest, len, fmt, args );
   }
   return rval;
}

int PORT_vsnprintf( char * dest, size_t len, const char * fmt, va_list args )
{
   int rval = -1;
   if ( dest && fmt && len > 0 )
   {
      rval = vsnprintf( dest, len, fmt, args );
      dest[ len - 1 ] = '\0';
   }
   return rval;
}

#ifdef WIN32
char * PORT_strcasestr( const char * haystack, const char * needle )
{
   bool match;

   size_t hlen = strlen( haystack );
   size_t nlen = strlen( needle );

   match = false;

   size_t i;
   size_t j;

   for ( i = 0; !match && i < hlen - nlen; i++ )
   {
      match = true;
      for ( j = 0; match && j < nlen; j++ )
      {
         if ( toupper(haystack[i+j]) != toupper(needle[j]) ) 
         {
            match = false;
         }
      }
   }

   if ( match )
   {
      return (char *) &haystack[i];
   }
   else
   {
      return NULL;
   }
}
#else
char * PORT_strcasestr( const char * haystack, const char * needle )
{
   return strcasestr( haystack, needle );
}
#endif // WIN32

char * PORT_basename( const char * path )
{
   static char rval[ PATH_MAX ] = "";
   if ( path )
   {
      char * start = strrchr( path, '/' );

      if ( !start )
      {
         // no forward slash, try backslash
         start = strrchr( path, '\\' );
      }

      if ( start )
      {
         start++;
         strncpy( rval, start, PATH_MAX );
         rval[ PATH_MAX - 1 ] = '\0';
         return rval;
      }

      // no directory, just filename
      strncpy( rval, path, PATH_MAX );
      rval[ PATH_MAX - 1 ] = '\0';
      return rval;
   }

   // path not set
   rval[0] = '\0';
   return rval;
}

char * PORT_dirname( const char * path )
{
   static char rval[ PATH_MAX ] = "";
   if ( path )
   {
      strncpy( rval, path, PATH_MAX );
      rval[ PATH_MAX - 1 ] = '\0';

      char * end = strrchr( rval, '/' );

      if ( !end )
      {
         // no forward slash, try backslash
         end = strrchr( rval, '\\' );
      }

      if ( end )
      {
         if ( end == rval )
            end[1] = '\0';
         else
            end[0] = '\0';
         return rval;
      }
   }

   // path not set, or no slash character
   strcpy( rval, "." );
   return rval;
}

