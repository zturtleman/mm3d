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


#include "misc.h"

#include "mm3dconfig.h"
//#include "sysconf.h"
#include "log.h"
#include "mm3dport.h"

#include "sorted_list.h"

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

using std::string;

void replaceSlash( char * str )
{
   if ( str )
   {
      for ( int n = 0; str[n]; n++ )
      {
         if ( str[n] == '/' )
         {
            str[n] = '\\';
         }
      }
   }
}

void replaceSlash( std::string & str )
{
   for ( unsigned n = 0; n < str.size(); n++ )
   {
      if ( str[n] == '/' )
      {
         str[n] = '\\';
      }
   }
}

void replaceBackslash( std::string & str )
{
   for ( unsigned n = 0; n < str.size(); n++ )
   {
      if ( str[n] == '\\' )
      {
         str[n] = '/';
      }
   }
}

void replaceBackslash( char * str )
{
   if ( str )
   {
      for ( int n = 0; str[n]; n++ )
      {
         if ( str[n] == '\\' )
         {
            str[n] = '/';
         }
      }
   }
}

string getFilePathFromPath( const char * path )
{
   string rval;
   char * temp = strdup( path );

   rval = PORT_dirname( temp );

   free( temp );

   return rval;
}

string getFileNameFromPath( const char * path )
{
   string rval;
   char * temp = strdup( path );

   rval = PORT_basename( temp );

   free( temp );

   return rval;
}

bool pathIsAbsolute( const char * path )
{
   if ( path == NULL )
   {
      return false;
   }
   if ( isalpha( path[0] ) && path[1] == ':' )
   {
      return true;
   }
   return ( path[0] == '/' || path[0] == '\\' );
}

string normalizePath( const char * path, const char * pwd )
{
   string rval;
   string fullPath;

   if ( pathIsAbsolute( path ) || pwd == NULL )
   {
      fullPath = path;
   }
   else
   {
      char * tempPwd = NULL;
      tempPwd = (char *) malloc( sizeof(char) * PATH_MAX );
      getcwd( tempPwd, PATH_MAX );

      fullPath  = tempPwd;
      fullPath += DIR_SLASH;
      fullPath += path;

      free( tempPwd );
   }

   char resolved[ PATH_MAX ];
   if ( PORT_realpath( fullPath.c_str(), resolved, PATH_MAX ) )
   {
      rval = resolved;
   }
   else
   {
      rval = path;
   }

   return rval;
}

string getRelativePath( const char * b, const char * p )
{
   if ( b && p )
   {
      std::string base(b);
      std::string path(p);

      replaceBackslash(base);
      replaceBackslash(path);

      if ( pathIsAbsolute( path.c_str() ) && !base.empty() )
      {
         // Append a / if that's not the last char
         if ( base[ base.size() - 1 ] != '/' )
         {
            base += "/";
         }

         int lastSlash = 0;

         int t = 0;
         int len = (int) (base.size() < path.size()) ? base.size() : path.size();
         for ( t = 0; t < len; t++ )
         {
            if ( base[t] != path[t] )
            {
               break;
            }
            else
            {
               if ( base[t] == '/' )
               {
                  lastSlash = t;
               }
            }
         }

         // Back up to lastSlash and prepend "../" for each '/' remaining in base
         string rval = "./";
         size_t offset = lastSlash;

         while ( (offset = base.find('/', offset + 1 )) < base.size() )
         {
            rval += "../";
         }

         rval += &path[ lastSlash + 1 ];

         log_debug( "relative path is %s\n", rval.c_str() );
         return rval;
      }
      
      return path; // already relative
   }

   return ""; // Error
}

string getAbsolutePath( const char * base, const char * path )
{
   if ( base && path )
   {
      if ( !pathIsAbsolute( path ) )
      {
         char * bptr = (char *) malloc(sizeof(char) * (strlen(base) + 2) );
         strcpy( bptr, base );
         replaceBackslash(bptr);

         // Append a / if that's not the last char
         if ( bptr[ strlen(base) - 1 ] != '/' )
         {
            strcat( bptr, "/" );
         }

         int off = 0;
         bool removed = false;
         do 
         {
            removed = false;

            if ( strncmp( &path[off], "./", 2 ) == 0 )
            {
               off += 2;
               removed = true;
            }
            else if ( strncmp( &path[off], "../", 3 ) == 0 )
            {
               removed = true;
               off += 3;

               // remove trailing slash
               bptr[ strlen(bptr) - 1 ] = '\0';

               // truncate after new last slash
               char * temp = strrchr( bptr, '/' );
               if ( temp )
               {
                  temp += 1;
                  temp[0] = '\0';
               }
            }

         } while( removed );

         string rval = string( bptr ) + &path[off];

         free( bptr );

         log_debug( "absolute path is %s\n", rval.c_str() );
         return rval;
      }
      else
      {
         return path; // Already absolute
      }
   }

   return ""; // Error
}

string fixAbsolutePath( const char * base, const char * path )
{
   if ( base && path )
   {
      if ( pathIsAbsolute( path ) )
      {
         string rval;
         char * temp = strrchr( path, '/' );
         if ( temp )
         {
            temp++;
            if ( temp[0] )
            {
               string rval = temp;
               DIR * dp = opendir( base );
               if ( dp )
               {
                  struct dirent * d;
                  while ( (d = readdir(dp )) != NULL )
                  {
                     if ( strcasecmp( d->d_name, temp ) == 0 )
                     {
                        rval = d->d_name;
                        break;
                     }
                  }
                  closedir( dp );
               }
               else
               {
                  log_error( "%s: %s\n", base, strerror(errno) );
               }
               return rval;
            }
         }
      }
      
      return path;
   }

   return ""; // error

   /*
   if ( base && path )
   {
      if ( path[0] == '/' || strcasecmp( path, "c:" ) == 0 )
      {
         char * bptr = (char *) malloc(sizeof(char) * (strlen(base) + 2) );
         strcpy( bptr, base );

         // Append a / if that's not the last char
         if ( bptr[ strlen(base) - 1 ] != '/' )
         {
            strcat( bptr, "/" );
         }

         bool stillLooking = true;
         string newBase;

         char * ptr;

         while ( stillLooking )
         {
            newBase = bptr;

            ptr = strchr( ptr, '/' );

            while ( ptr ) 
            {
               ptr++;

               if ( ptr[0] )
               {
                  stillLooking = false;
                  break;
               }
               else
               {
                  string dirent = _findDirent( &ptr[0] );

                  if ( dirent.c_str()[0] != '\0' )
                  {
                  }
               }
               ptr = strchr( ptr, '/' );
            }
         }

         if ( strcmp( base, newBase.c_str() ) != 0 )
         {
            // newBase isn't equal
            return newBase;
         }
      }
      
      return path;
   }
   else
   {
      return "";
   }
   */
}

string fixFileCase( const char * path, const char * file )
{
   std::string rval = file;

   DIR * dp = opendir( path );
   if ( dp )
   {
      struct dirent * d;
      while ( (d = readdir( dp )) != NULL )
      {
         if ( strcasecmp( d->d_name, file ) == 0 )
         {
            rval = d->d_name;
            break;
         }
      }
      closedir( dp );
   }
   return rval;
}

void normalizePath( const char * filename, std::string & fullName, std::string & fullPath, std::string & baseName )
{
   char path[PATH_MAX];
   PORT_realpath( filename, path, PATH_MAX );
   replaceBackslash( path );
   fullName = path;
   fullPath = PORT_dirname( path );
   strcpy( path, fullName.c_str() );
   baseName = PORT_basename( (char *) filename );
   replaceBackslash( baseName );
}

std::string replaceExtension( const char * infile, const char * ext )
{
   std::string rval = removeExtension( infile );
   rval += '.';
   rval += ext;
   return rval;
}

std::string removeExtension( const char * infile )
{
   std::string rval = infile;
   size_t i = rval.rfind( '.' );
   size_t len = rval.size();

   if ( i > 0 && i < len )
   {
      size_t slash = rval.rfind( '/' );
      if ( slash > len )
         slash = rval.rfind( '\\' );

      if ( slash > len || i > slash + 1 ) 
         rval.resize(i);
   }
   return rval;
}

void getFileList( std::list<std::string> & l, const char * const path, const char * const name )
{
   sorted_list<string> sl;

   unsigned len = strlen( name );

   DIR * dp = opendir( path );
   if ( dp )
   {
      struct dirent * d;
      while ( (d = readdir( dp )) != NULL )
      {
         if ( strncasecmp( d->d_name, name, len ) == 0 )
         {
            sl.insert_sorted( d->d_name );
         }
      }
      closedir( dp );
   }

   for ( unsigned t  = 0; t < sl.size(); t++ )
   {
      l.push_back( sl[t] );
   }
}

bool file_exists( const char * filename )
{
   struct stat statbuf;
   if ( PORT_lstat( filename, &statbuf ) == 0 )
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool is_directory( const char * filename )
{
   struct stat statbuf;
   if ( PORT_lstat( filename, &statbuf ) == 0 )
   {
      DIR * dp = opendir( filename ); // Don't check S_ISDIR, could be symlink
      if ( dp )
      {
         closedir( dp );
         return true;
      }
   }
   return false;
}

int mkpath( const char * filename, mode_t mode )
{
   if ( filename && filename[0] )
   {
      char * str = strdup( filename );
      size_t len = strlen( filename );
      size_t offset = 1;

      do
      {
         while (  filename[ offset ] != '\0' 
               && filename[ offset ] != '/' 
               && filename[ offset ] != '\\' )
         {
            offset++;
         }
         strcpy ( str, filename );
         str[ offset ] = '\0';

         PORT_mkdir( str, mode );

         offset++;
      } while ( offset <= len );

      free( str );
   }

   return 0;
}

size_t utf8len( const char * str )
{
   size_t len = 0;
   size_t pos = 0;
   while ( str && str[pos] != 0 )
   {
      unsigned char ch = ((unsigned char) str[pos]) & 0xc0;

      if (     ch == 0 
            || ch == 0x40 
            || ch == 0xc0 )
      {
         len++;
      }
      pos++;
   }
   return len;
}

size_t utf8len( const std::string & str )
{
   return utf8len( str.c_str() );
}

// Truncate at "len" utf8 characters
void utf8strtrunc( char * str, size_t len )
{
   if ( str )
   {
      size_t pos = 0;
      size_t n = 0;
      size_t slen = strlen(str);
      while ( pos < slen && n < len )
      {
         unsigned char ch = ((unsigned char) str[pos]) & 0xc0;

         if ( (ch & 0xc0) == 0xc0 )
         {
            n++;
            ch = str[pos];
            if ( (ch & 0xe0) == 0xc0 )
               pos += 2;
            else if ( (ch & 0xf0) == 0xe0 )
               pos += 3;
            else if ( (ch & 0xf8) == 0xf0 )
               pos += 4;
         }
         else
         {
            n++;
            pos++;
         }
      }
      utf8chrtrunc( str, pos );
   }
}

void utf8strtrunc( std::string & str, size_t len )
{
   size_t pos = 0;
   size_t n = 0;
   size_t slen = str.size();
   while ( pos < slen && n < len )
   {
      unsigned char ch = ((unsigned char) str[pos]) & 0xc0;

      if ( (ch & 0xc0) == 0xc0 )
      {
         n++;
         ch = str[pos];
         if ( (ch & 0xe0) == 0xc0 )
            pos += 2;
         else if ( (ch & 0xf0) == 0xe0 )
            pos += 3;
         else if ( (ch & 0xf8) == 0xf0 )
            pos += 4;
      }
      else
      {
         n++;
         pos++;
      }
   }
   utf8chrtrunc( str, pos );
}

// Truncate at "len" bytes (C characters)
void utf8chrtrunc( char * str, size_t len )
{
   // If we're truncating
   while ( len > 0 
         && (((unsigned char) str[len] & 0xc0 )) == 0x80 )
   {
      len--;
   }
   str[len] = '\0';
}

void utf8chrtrunc( std::string & str, size_t len )
{
   while ( len > 0 
         && (((unsigned char) str[len] & 0xc0 )) == 0x80 )
   {
      len--;
   }
   if ( len < str.size() )
      str.resize(len);
}

