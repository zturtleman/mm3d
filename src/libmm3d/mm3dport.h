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


#ifndef __MM3DPORT_H
#define __MM3DPORT_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdarg.h>
#include "config.h"


struct _PORT_timeval
{
   unsigned long  tv_sec;  // seconds
   unsigned short tv_msec; // milliseconds
};
typedef struct _PORT_timeval PORT_timeval;

char * PORT_realpath( const char * path, char * resolved_path, size_t len );
struct tm * PORT_localtime_r( const time_t * timep, struct tm * result );
void   PORT_gettimeofday( PORT_timeval * tv );
char * PORT_asctime_r( const struct tm * tmval, char * buf );
int    PORT_symlink( const char * oldpath, const char * newpath );
int    PORT_mkdir( const char * pathname, mode_t mode );
int    PORT_snprintf( char * dest, size_t len, const char * fmt, ... );
int    PORT_vsnprintf( char * dest, size_t len, const char * fmt, va_list args );
char * PORT_basename( const char * path );
char * PORT_dirname( const char * path );


#ifdef WIN32
inline bool S_ISLNK( mode_t m )
{
   return false;
}
#endif // WIN32

#endif //__MM3DPORT_H
