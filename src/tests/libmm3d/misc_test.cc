/*  Maverick Model 3D
 * 
 *  Copyright (c) 2007-2008 Kevin Worcester
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


// This file tests misc.h

#include <QtTest/QtTest>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string>

#include "misc.h"
#include "log.h"

#include "test_common.h"

class MiscTest : public QObject
{
   Q_OBJECT
private slots:
   void initTestCase()
   {
      log_enable_debug( false );
   }

   void testPathIsAbsolute()
   {
      // Win32
      QVERIFY_TRUE( pathIsAbsolute( "c:\\windows\\system32" ) );
      QVERIFY_TRUE( pathIsAbsolute( "c:\\" ) );
      QVERIFY_TRUE( pathIsAbsolute( "a:\\" ) );
      QVERIFY_TRUE( pathIsAbsolute( "z:\\" ) );
      QVERIFY_TRUE( pathIsAbsolute( "A:\\" ) );
      QVERIFY_TRUE( pathIsAbsolute( "C:\\" ) );
      QVERIFY_TRUE( pathIsAbsolute( "Z:\\" ) );
      QVERIFY_TRUE( pathIsAbsolute( "\\" ) );
      QVERIFY_TRUE( pathIsAbsolute( "\\windows\\system32" ) );
      QVERIFY_FALSE( pathIsAbsolute( "windows\\system32" ) );

      // Unix
      QVERIFY_TRUE( pathIsAbsolute( "/" ) );
      QVERIFY_TRUE( pathIsAbsolute( "/etc/hosts" ) );
      QVERIFY_FALSE( pathIsAbsolute( "etc/hosts" ) );
      QVERIFY_FALSE( pathIsAbsolute( "" ) );
      QVERIFY_FALSE( pathIsAbsolute( "." ) );
      QVERIFY_FALSE( pathIsAbsolute( "./" ) );
   }

   void testGetFilePath()
   {
      QVERIFY_EQ( std::string("/"), getFilePathFromPath( "/" ) );
      QVERIFY_EQ( std::string("/"), getFilePathFromPath( "/etc" ) );
      QVERIFY_EQ( std::string("/etc"), getFilePathFromPath( "/etc/hosts" ) );
      QVERIFY_EQ( std::string("/home/user/.mm3d"),
            getFilePathFromPath( "/home/user/.mm3d/mm3drc" ) );
      QVERIFY_EQ( std::string(".mm3d"),
            getFilePathFromPath( ".mm3d/mm3drc" ) );
      QVERIFY_EQ( std::string("some/longer/dir"),
            getFilePathFromPath( "some/longer/dir/file" ) );
      QVERIFY_EQ( std::string("."), getFilePathFromPath( "./file.txt" ) );
      QVERIFY_EQ( std::string("."), getFilePathFromPath( "file.txt" ) );
      QVERIFY_EQ( std::string("."), getFilePathFromPath( "./" ) );
      QVERIFY_EQ( std::string("some/dir"), getFilePathFromPath( "some/dir/" ) );

      QVERIFY_EQ( std::string("\\"), getFilePathFromPath( "\\" ) );
      QVERIFY_EQ( std::string("\\"), getFilePathFromPath( "\\etc" ) );
      QVERIFY_EQ( std::string("\\etc"), getFilePathFromPath( "\\etc\\hosts" ) );
      QVERIFY_EQ( std::string("\\home\\user\\.mm3d"),
            getFilePathFromPath( "\\home\\user\\.mm3d\\mm3drc" ) );
      QVERIFY_EQ( std::string(".mm3d"),
            getFilePathFromPath( ".mm3d\\mm3drc" ) );
      QVERIFY_EQ( std::string("some\\longer\\dir"),
            getFilePathFromPath( "some\\longer\\dir\\file" ) );
      QVERIFY_EQ( std::string("."), getFilePathFromPath( ".\\file.txt" ) );
      QVERIFY_EQ( std::string("."), getFilePathFromPath( "file.txt" ) );
      QVERIFY_EQ( std::string("."), getFilePathFromPath( ".\\" ) );
      QVERIFY_EQ( std::string("some\\dir"), getFilePathFromPath( "some\\dir\\" ) );
   }

   void testGetFileName()
   {
      QVERIFY_EQ( std::string(""), getFileNameFromPath( "/" ) );
      QVERIFY_EQ( std::string("etc"), getFileNameFromPath( "/etc" ) );
      QVERIFY_EQ( std::string("hosts"), getFileNameFromPath( "/etc/hosts" ) );
      QVERIFY_EQ( std::string("mm3drc"),
            getFileNameFromPath( "/home/user/.mm3d/mm3drc" ) );
      QVERIFY_EQ( std::string("mm3drc"),
            getFileNameFromPath( ".mm3d/mm3drc" ) );
      QVERIFY_EQ( std::string("file"),
            getFileNameFromPath( "some/longer/dir/file" ) );
      QVERIFY_EQ( std::string("file.txt"), getFileNameFromPath( "./file.txt" ) );
      QVERIFY_EQ( std::string("file.txt"), getFileNameFromPath( "file.txt" ) );
      QVERIFY_EQ( std::string(""), getFileNameFromPath( "./" ) );
      QVERIFY_EQ( std::string(""), getFileNameFromPath( "some/dir/" ) );

      QVERIFY_EQ( std::string(""), getFileNameFromPath( "\\" ) );
      QVERIFY_EQ( std::string("etc"), getFileNameFromPath( "\\etc" ) );
      QVERIFY_EQ( std::string("hosts"), getFileNameFromPath( "\\etc\\hosts" ) );
      QVERIFY_EQ( std::string("mm3drc"),
            getFileNameFromPath( "\\home\\user\\.mm3d\\mm3drc" ) );
      QVERIFY_EQ( std::string("mm3drc"),
            getFileNameFromPath( ".mm3d\\mm3drc" ) );
      QVERIFY_EQ( std::string("file"),
            getFileNameFromPath( "some\\longer\\dir\\file" ) );
      QVERIFY_EQ( std::string("file.txt"), getFileNameFromPath( ".\\file.txt" ) );
      QVERIFY_EQ( std::string("file.txt"), getFileNameFromPath( "file.txt" ) );
      QVERIFY_EQ( std::string(""), getFileNameFromPath( ".\\" ) );
      QVERIFY_EQ( std::string(""), getFileNameFromPath( "some\\dir\\" ) );
   }

   void testRemoveExtension()
   {
      QVERIFY_EQ( std::string("file"), removeExtension("file.txt") );
      QVERIFY_EQ( std::string("a"), removeExtension("a.txt") );
      QVERIFY_EQ( std::string("/etc/file"), removeExtension("/etc/file.txt") );
      QVERIFY_EQ( std::string("file"), removeExtension("file") );
      QVERIFY_EQ( std::string("/etc/file"), removeExtension("/etc/file") );
      QVERIFY_EQ( std::string("dir/file"), removeExtension("dir/file.txt") );
      QVERIFY_EQ( std::string("dir/file"), removeExtension("dir/file") );
      QVERIFY_EQ( std::string("dir.ext/file"), removeExtension("dir.ext/file.txt") );
      QVERIFY_EQ( std::string("dir.ext/file"), removeExtension("dir.ext/file") );
      QVERIFY_EQ( std::string("dir.ext/.dotfile"), removeExtension("dir.ext/.dotfile") );
      QVERIFY_EQ( std::string("dir.ext/a"), removeExtension("dir.ext/a.ext") );
   }

   void testReplaceExtension()
   {
      QVERIFY_EQ( std::string("file.new"), replaceExtension("file.txt", "new") );
      QVERIFY_EQ( std::string("a.new"), replaceExtension("a.txt", "new") );
      QVERIFY_EQ( std::string("/etc/file.new"), replaceExtension("/etc/file.txt", "new") );
      QVERIFY_EQ( std::string("file.new"), replaceExtension("file", "new") );
      QVERIFY_EQ( std::string("/etc/file.new"), replaceExtension("/etc/file", "new") );
      QVERIFY_EQ( std::string("dir/file.new"), replaceExtension("dir/file.txt", "new") );
      QVERIFY_EQ( std::string("dir/file.new"), replaceExtension("dir/file", "new") );
      QVERIFY_EQ( std::string("dir.ext/file.new"), replaceExtension("dir.ext/file.txt", "new") );
      QVERIFY_EQ( std::string("dir.ext/file.new"), replaceExtension("dir.ext/file", "new") );
      QVERIFY_EQ( std::string("dir.ext/.dotfile.new"), replaceExtension("dir.ext/.dotfile", "new") );
      QVERIFY_EQ( std::string("dir.ext/a.new"), replaceExtension("dir.ext/a.ext", "new") );
   }

   void testReplaceBackslash()
   {
      std::string str;

      str = "";
      replaceBackslash( str );
      QVERIFY_EQ( std::string(""), str );

      str = "some str";
      replaceBackslash( str );
      QVERIFY_EQ( std::string("some str"), str );

      str = "\\some\\str\\";
      replaceBackslash( str );
      QVERIFY_EQ( std::string("/some/str/"), str );

      str = "/some/str/";
      replaceBackslash( str );
      QVERIFY_EQ( std::string("/some/str/"), str );

      char cstr[20];
      
      strcpy( cstr, "" );
      replaceBackslash( cstr );
      QVERIFY_EQ( std::string(""), std::string(cstr) );

      strcpy( cstr, "some cstr" );
      replaceBackslash( cstr );
      QVERIFY_EQ( std::string("some cstr"), std::string(cstr) );

      strcpy( cstr, "\\some\\cstr\\" );
      replaceBackslash( cstr );
      QVERIFY_EQ( std::string("/some/cstr/"), std::string(cstr) );

      strcpy( cstr, "/some/cstr/" );
      replaceBackslash( cstr );
      QVERIFY_EQ( std::string("/some/cstr/"), std::string(cstr) );
   }

   void testReplaceSlash()
   {
      std::string str;

      str = "";
      replaceSlash( str );
      QVERIFY_EQ( std::string(""), str );

      str = "some str";
      replaceSlash( str );
      QVERIFY_EQ( std::string("some str"), str );

      str = "\\some\\str\\";
      replaceSlash( str );
      QVERIFY_EQ( std::string("\\some\\str\\"), str );

      str = "/some/str/";
      replaceSlash( str );
      QVERIFY_EQ( std::string("\\some\\str\\"), str );

      char cstr[20];
      
      strcpy( cstr, "" );
      replaceSlash( cstr );
      QVERIFY_EQ( std::string(""), std::string(cstr) );

      strcpy( cstr, "some cstr" );
      replaceSlash( cstr );
      QVERIFY_EQ( std::string("some cstr"), std::string(cstr) );

      strcpy( cstr, "\\some\\cstr\\" );
      replaceSlash( cstr );
      QVERIFY_EQ( std::string("\\some\\cstr\\"), std::string(cstr) );

      strcpy( cstr, "/some/cstr/" );
      replaceSlash( cstr );
      QVERIFY_EQ( std::string("\\some\\cstr\\"), std::string(cstr) );
   }

   void testGetRelativePath()
   {
      QVERIFY_EQ( std::string("file"), getRelativePath("", "file") );
      QVERIFY_EQ( std::string("file"), getRelativePath("/home/dir", "file") );
      QVERIFY_EQ( std::string("subdir/file"), getRelativePath("/home/dir", "subdir/file") );
      QVERIFY_EQ( std::string("./file"), getRelativePath("/home/dir", "/home/dir/file") );
      QVERIFY_EQ( std::string("./file"), getRelativePath("/home/dir/", "/home/dir/file") );
      QVERIFY_EQ( std::string("./subdir/file"), getRelativePath("/home/dir/", "/home/dir/subdir/file") );
      QVERIFY_EQ( std::string("file"), getRelativePath("\\home\\dir", "file") );
      QVERIFY_EQ( std::string("subdir/file"), getRelativePath("\\home\\dir", "subdir\\file") );
      QVERIFY_EQ( std::string("./file"), getRelativePath("\\home\\dir", "\\home\\dir\\file") );
      QVERIFY_EQ( std::string("./file"), getRelativePath("\\home\\dir\\", "\\home\\dir\\file") );
      QVERIFY_EQ( std::string("./subdir/file"), getRelativePath("\\home\\dir\\", "\\home\\dir\\subdir\\file") );
      QVERIFY_EQ( std::string("file"), getRelativePath("c:\\home\\dir", "file") );
      QVERIFY_EQ( std::string("subdir/file"), getRelativePath("c:\\home\\dir", "subdir\\file") );
      QVERIFY_EQ( std::string("./file"), getRelativePath("c:\\home\\dir", "c:\\home\\dir\\file") );
      QVERIFY_EQ( std::string("./file"), getRelativePath("c:\\home\\dir\\", "c:\\home\\dir\\file") );
      QVERIFY_EQ( std::string("./subdir/file"), getRelativePath("c:\\home\\dir\\", "c:\\home\\dir\\subdir\\file") );
   }

   void testGetAbsolutePath()
   {
      QVERIFY_EQ( std::string("/home/dir/file"), getAbsolutePath("/home/dir", "file") );
      QVERIFY_EQ( std::string("/home/dir/file"), getAbsolutePath("/home/dir/", "file") );
      QVERIFY_EQ( std::string("/home/dir/file"), getAbsolutePath("/home/dir", "./file") );
      QVERIFY_EQ( std::string("/home/dir/file"), getAbsolutePath("/home/dir/", "./file") );
      QVERIFY_EQ( std::string("/home/file"), getAbsolutePath("/home/dir", "../file") );
      QVERIFY_EQ( std::string("/home/file"), getAbsolutePath("/home/dir/", "../file") );
      QVERIFY_EQ( std::string("/file"), getAbsolutePath("/home/dir", "../../file") );
      QVERIFY_EQ( std::string("/file"), getAbsolutePath("/home/dir/", "../../file") );
      QVERIFY_EQ( std::string("/home/dir2/file"), getAbsolutePath("/home/dir", "../dir2/file") );
      QVERIFY_EQ( std::string("/home/dir2/file"), getAbsolutePath("/home/dir/", "../dir2/file") );
      QVERIFY_EQ( std::string("/file"), getAbsolutePath("/home/dir", "/file") );
      QVERIFY_EQ( std::string("/file"), getAbsolutePath("/home/dir/", "/file") );
      QVERIFY_EQ( std::string("c:/home/dir/file"), getAbsolutePath("c:\\home\\dir", "file") );
      QVERIFY_EQ( std::string("c:/home/dir/file"), getAbsolutePath("c:\\home\\dir\\", "file") );

      // FIXME these should probably be / instead
      /*
      QVERIFY_EQ( std::string("c:/home/dir/file"), getAbsolutePath("c:\\home\\dir", ".\\file") );
      QVERIFY_EQ( std::string("c:/home/dir/file"), getAbsolutePath("c:\\home\\dir\\", ".\\file") );
      QVERIFY_EQ( std::string("c:\\file"), getAbsolutePath("c:\\home\\dir", "c:\\file") );
      QVERIFY_EQ( std::string("c:\\file"), getAbsolutePath("c:\\home\\dir\\", "c:\\file") );
      */
   }

   void testUtf8ChrTrunc()
   {
      std::string str;

      // each char is 3 bytes

      str = "ひらがな";
      utf8chrtrunc( str, 13 );
      QVERIFY_EQ( std::string("ひらがな"), str );

      str = "ひらがな";
      utf8chrtrunc( str, 12 );
      QVERIFY_EQ( std::string("ひらがな"), str );

      str = "ひらがな";
      utf8chrtrunc( str, 11 );
      QVERIFY_EQ( std::string("ひらが"), str );

      str = "ひらがな";
      utf8chrtrunc( str, 10 );
      QVERIFY_EQ( std::string("ひらが"), str );

      str = "ひらがな";
      utf8chrtrunc( str, 9 );
      QVERIFY_EQ( std::string("ひらが"), str );

      str = "ひらがな";
      utf8chrtrunc( str, 8 );
      QVERIFY_EQ( std::string("ひら"), str );

      char cstr[20];
      
      strcpy( cstr, "ひらがな" );
      utf8chrtrunc( cstr, 13 );
      QVERIFY_EQ( std::string("ひらがな"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8chrtrunc( cstr, 12 );
      QVERIFY_EQ( std::string("ひらがな"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8chrtrunc( cstr, 11 );
      QVERIFY_EQ( std::string("ひらが"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8chrtrunc( cstr, 10 );
      QVERIFY_EQ( std::string("ひらが"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8chrtrunc( cstr, 9 );
      QVERIFY_EQ( std::string("ひらが"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8chrtrunc( cstr, 8 );
      QVERIFY_EQ( std::string("ひら"), std::string(cstr) );
   }

   void testUtf8StrTrunc()
   {
      std::string str;

      // each char is 3 bytes

      str = "ひらがな";
      utf8strtrunc( str, 5 );
      QVERIFY_EQ( std::string("ひらがな"), str );

      str = "ひらがな";
      utf8strtrunc( str, 4 );
      QVERIFY_EQ( std::string("ひらがな"), str );

      str = "ひらがな";
      utf8strtrunc( str, 3 );
      QVERIFY_EQ( std::string("ひらが"), str );

      str = "ひらがな";
      utf8strtrunc( str, 2 );
      QVERIFY_EQ( std::string("ひら"), str );

      str = "ひらがな";
      utf8strtrunc( str, 1 );
      QVERIFY_EQ( std::string("ひ"), str );

      str = "ひらがな";
      utf8strtrunc( str, 0 );
      QVERIFY_EQ( std::string(""), str );

      // mixed length

      str = "aひbע";
      utf8strtrunc( str, 5 );
      QVERIFY_EQ( std::string("aひbע"), str );

      str = "aひbע";
      utf8strtrunc( str, 4 );
      QVERIFY_EQ( std::string("aひbע"), str );

      str = "aひbע";
      utf8strtrunc( str, 3 );
      QVERIFY_EQ( std::string("aひb"), str );

      str = "aひbע";
      utf8strtrunc( str, 2 );
      QVERIFY_EQ( std::string("aひ"), str );

      str = "aひbע";
      utf8strtrunc( str, 1 );
      QVERIFY_EQ( std::string("a"), str );

      str = "aひbע";
      utf8strtrunc( str, 0 );
      QVERIFY_EQ( std::string(""), str );

      char cstr[20];
      
      strcpy( cstr, "ひらがな" );
      utf8strtrunc( cstr, 5 );
      QVERIFY_EQ( std::string("ひらがな"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8strtrunc( cstr, 4 );
      QVERIFY_EQ( std::string("ひらがな"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8strtrunc( cstr, 3 );
      QVERIFY_EQ( std::string("ひらが"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8strtrunc( cstr, 2 );
      QVERIFY_EQ( std::string("ひら"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8strtrunc( cstr, 1 );
      QVERIFY_EQ( std::string("ひ"), std::string(cstr) );

      strcpy( cstr, "ひらがな" );
      utf8strtrunc( cstr, 0 );
      QVERIFY_EQ( std::string(""), std::string(cstr) );

      // mixed length

      strcpy( cstr, "aひbע" );
      utf8strtrunc( cstr, 5 );
      QVERIFY_EQ( std::string("aひbע"), std::string(cstr) );

      strcpy( cstr, "aひbע" );
      utf8strtrunc( cstr, 4 );
      QVERIFY_EQ( std::string("aひbע"), std::string(cstr) );

      strcpy( cstr, "aひbע" );
      utf8strtrunc( cstr, 3 );
      QVERIFY_EQ( std::string("aひb"), std::string(cstr) );

      strcpy( cstr, "aひbע" );
      utf8strtrunc( cstr, 2 );   
      QVERIFY_EQ( std::string("aひ"), std::string(cstr) );

      strcpy( cstr, "aひbע" );
      utf8strtrunc( cstr, 1 );
      QVERIFY_EQ( std::string("a"), std::string(cstr) );

      strcpy( cstr, "aひbע" );
      utf8strtrunc( cstr, 0 );
      QVERIFY_EQ( std::string(""), std::string(cstr) );

   }

   // FIXME add tests
   //    normalizePath (4 args)
   //    normalizePath (2 args)
   //    getAbsolutePath
   //    fixAbsolutePath
   //    fixFileCase
   //    getFileList
   //    file_exists
   //    is_directory
   //    mkpath
   //    utf8len
   //    utf8strtrunc
   //    utf8chrtrunc
};

QTEST_MAIN(MiscTest)
#include "misc_test.moc"
