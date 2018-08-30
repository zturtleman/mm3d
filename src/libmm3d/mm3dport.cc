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
#include "misc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <string>

#include "config.h"

#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#else
#include <sys/timeb.h>
#endif // HAVE_GETTIMEOFDAY

#include <unistd.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// FIXME: This requires Windows Vista.
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x80
#endif
#endif

// TODO: Cache this instead of mallocing each time but still have some way to free it at shutdown.
char * PORT_get_current_dir_name( void )
{
#ifdef WIN32
	DWORD length = GetCurrentDirectoryW( 0, NULL );
	if ( length == 0 )
	{
		return NULL;
	}

	wchar_t *wbuf = (wchar_t *)malloc( length * sizeof( wchar_t ) );
	if ( !wbuf )
	{
		return NULL;
	}

	if ( GetCurrentDirectoryW( length, wbuf ) == 0 )
	{
		free( wbuf );
		return NULL;
	}

	DWORD utf8Size = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, wbuf, length, NULL, 0, NULL, NULL );

	char * buf = (char *)malloc( utf8Size );
	if ( !buf )
	{
		free( wbuf );
		return NULL;
	}

	if ( WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, wbuf, length, buf, utf8Size, NULL, NULL ) == 0 )
	{
		free( wbuf );
		free( buf );
		return NULL;
	}

	return buf;
#else
	// malloc'ing buffer when passed NULL is a non-standard extension to POSIX.
	// It's supported by glibc, FreeBSD, and probably others.
	char * buf = getcwd( NULL, 0 );
	if ( buf )
	{
		return buf;
	}

	size_t size = 64;
	buf = (char*)malloc( size );
	while ( buf )
	{
		if ( getcwd( buf, size ) )
		{
			break;
		}
		else
		{
			if ( errno == ERANGE )
			{
				if ( size >= 1024 )
					size = size + 1024;
				else
					size = size + size;

				char * newbuf = (char*)realloc( buf, size );
				if ( newbuf )
				{
					buf = newbuf;
				}
				else
				{
					free( buf );
					buf = NULL;
				}
			}
			else
			{
				// unhandled error
				free( buf );
				buf = NULL;
			}
		}
	}
	return buf;
#endif
}

// FIXME add thorough testing for the manual part of this (the case where
// realpath fails because the directories don't exist).
char * PORT_realpath( const char * path, char * resolved_path, size_t len )
{
#ifdef WIN32
   char * rval = NULL;
   std::wstring widePath = utf8PathToWide( path );
   if ( !widePath.empty() )
   {
      DWORD requiredSize = GetFullPathNameW( &widePath[0], 0, NULL, NULL );
      std::wstring realpath( requiredSize, '\0' );
      if ( GetFullPathNameW( &widePath[0], requiredSize, &realpath[0], NULL ) != 0 )
      {
         if ( WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &realpath[0], -1, resolved_path, len, NULL, NULL ) != 0 ) {
            // \\?\ is added by utf8PathToWide() for Windows API but don't return it.
            if ( strncmp( resolved_path, "\\\\?\\", 4 ) == 0 )
            {
               memmove( &resolved_path[0], &resolved_path[4], strlen( &resolved_path[4] ) + 1 );
            }

            rval = resolved_path;
         }
      }
   }
#else
   char * rval = realpath( path, resolved_path );
#endif // WIN32
   if ( !rval )
   {
      if ( len > 0 )
      {
         if ( pathIsAbsolute( path ) )
         {
            strncpy( resolved_path, path, len );
         }
         else
         {
            char *pwd = PORT_get_current_dir_name();
            if ( pwd )
            {
               PORT_snprintf( resolved_path, len, "%s/%s", pwd, path );
               free( pwd );
            }
            else
            {
               PORT_snprintf( resolved_path, len, "./%s", path );
            }
         }
         resolved_path[ len - 1 ] = '\0';
         rval = resolved_path;

         replaceBackslash( resolved_path );

         // Remove "/./" and "//"
         char * end = rval + strlen(rval);

         char * s = rval;
         while ( (s = strstr(s, "/./")) != NULL )
         {
            memmove( s, s + 2, end - (s + 2) );
            end -= 2;
            end[0] = '\0';
         }

         s = rval;
         while ( (s = strstr(s, "//")) != NULL )
         {
            memmove( s, s + 1, end - (s + 1) );
            --end;
            end[0] = '\0';
         }

         // Remove "/../"
         s = rval;
         while ( (s = strstr(s, "/../")) != NULL )
         {
            char * lastSlash = s - 1;
            while ( lastSlash >= rval )
            {
               if ( *lastSlash != '/' )
                  --lastSlash;
               else
                  break;
            }

            if ( lastSlash >= rval )
            {
               int len = (end - s) - 3;
               memmove( lastSlash, s+3, len );
               end = lastSlash + len;
               end[0] = '\0';
               s = lastSlash;
            }
            else
            {
               return rval;
            }
         }
      }
   }
   return rval;
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
   std::wstring widePath = utf8PathToWide( pathname );
   if ( widePath.empty() )
   {
      errno = EINVAL;
      return -1;
   }

   if ( CreateDirectoryW( &widePath[0], NULL ) )
      return 0;

   if ( GetLastError() == ERROR_ALREADY_EXISTS )
      return 0;

   errno = ENOENT;
   return -1;
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

char * PORT_basename( const char * path )
{
   static char rval[ PATH_MAX ] = "";
   if ( path )
   {
      const char * start = strrchr( path, '/' );

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

