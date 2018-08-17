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


#ifndef __MISC_H
#define __MISC_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string>
#include <list>

extern bool pathIsAbsolute( const char * path );

extern void normalizePath( const char * filename, 
      std::string & fullName, std::string & fullPath, std::string & baseName );

extern void replaceBackslash( std::string & str );
extern void replaceBackslash( char * str );
extern void replaceSlash( std::string & str );
extern void replaceSlash( char * str );

extern std::string getFilePathFromPath( const char * path );
extern std::string getFileNameFromPath( const char * path );
extern std::string normalizePath( const char * filename, const char * pwd = NULL );

extern std::string getRelativePath( const char * base, const char * path );
extern std::string getAbsolutePath( const char * base, const char * path );
extern std::string fixAbsolutePath( const char * base, const char * path );

extern std::string fixFileCase( const char * path, const char * file );

std::string replaceExtension( const char * infile, const char * ext );
std::string removeExtension( const char * infile );

extern void getFileList( std::list<std::string> &l, const char * const path, const char * const name );

extern bool file_modifiedtime( const char * filename, time_t * modifiedTime );
extern bool file_exists( const char * filename );
extern bool is_directory( const char * filename );

extern int  mkpath( const char * filename, mode_t mode );

extern void chomp( char * str );

// returns length of string in number of utf8 characters
size_t utf8len( const char * str );
size_t utf8len( const std::string & str );

// Truncate at "len" utf8 characters (FIXME doesn't work yet)
extern void utf8strtrunc( char * str, size_t len );
extern void utf8strtrunc( std::string & str, size_t len );

// Truncate at "len" bytes (C characters)
extern void utf8chrtrunc( char * str, size_t len );
extern void utf8chrtrunc( std::string & str, size_t len );

#ifdef WIN32
std::wstring utf8PathToWide( const char *filename );
#endif // WIN32

#endif // __MISC_H
