/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2004-2007 Kevin Worcester
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License,or
 * (at your option)any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not,write to the Free Software
 * Foundation,Inc.,59 Temple Place-Suite 330,Boston,MA 02111-1307,
 * USA.
 *
 * See the COPYING file for full license text.
 */


#ifndef __MM3DPORT_H
#define __MM3DPORT_H

//#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdarg.h>

#ifdef WIN32
#define PATH_MAX MAX_PATH //2019
#endif

char *PORT_get_current_dir_name(void);
char *PORT_realpath(const char *path,char *resolved_path, size_t len);
char *PORT_basename(const char *path);
char *PORT_dirname(const char *path);
//int PORT_symlink(const char *oldpath, const char *newpath);
//TODO: Would like to remove UNUSED mode parameter.
//int PORT_mkdir(const char *pathname,mode_t mode);
int PORT_mkdir(const char *pathname, size_t mode=0755); //OCTAL

//objfilter.cc
struct tm *PORT_localtime_r(const time_t *timep,struct tm *result);
char *PORT_asctime_r(const struct tm *tmval,char *buf);

/*HAVE_GETTIMEOFDAY
struct PORT_timeval
{
	unsigned long  tv_sec;  // seconds
	unsigned short tv_msec; // milliseconds
};
void PORT_gettimeofday(PORT_timeval *tv);*/

/*UNUSED
#ifdef WIN32
inline bool S_ISLNK(mode_t m){ return false; }
#endif // WIN32
*/

//STANDARD
//int PORT_snprintf(char *dest, size_t len, const char *fmt,...);
//int PORT_vsnprintf(char *dest, size_t len, const char *fmt,va_list args);

#ifdef _MSC_VER //NEW
#define PORT_strcasecmp _stricmp
#define PORT_strncasecmp _strnicmp
#else
#define PORT_strcasecmp strcasecmp
#define PORT_strncasecmp strncasecmp
#endif

#endif //__MM3DPORT_H
