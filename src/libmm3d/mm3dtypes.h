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

#ifndef __MM3DTYPES_H_INC__
#define __MM3DTYPES_H_INC__

typedef float float32_t;


//// https://github.com/zturtleman/mm3d/issues/71 ////

//#include "config.h" //AutoTools or something, not sure.

#include <set>
#include <map>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <functional> //bind2nd/greater
//2019
#include <array>
#include <memory> 
#include <unordered_map>
#include <unordered_set>

#include <math.h>
#include <limits.h> //INT_MAX
#include <float.h> //DBL_MAX
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h> //2019

//2019: Assuming have precompiled header.
#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN //sysconf.cc
#include <Windows.h> //NEW
#include <shlobj.h> //sysconf.cc
#include <shlwapi.h> //misc.cc
#endif

//filedatadest.cc
#ifndef _MSC_VER
#include <unistd.h>
#include <dirent.h> //md3filter.cc
#endif
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

//const double PI = 3.1415926; //???
const double PI = 3.14159265358979323846;
const double PIOVER180 = PI/180;

//typedef const char *utf8; //USE ME?

typedef std::vector<int> int_list; //REMOVE ME?

//typedef Model::pos_list pos_list;
//typedef Model::infl_list infl_list;

class Command;
class CommandManager;
class DataDest;
class DataSource;
class FileFactory;
class Matrix;
class Model;
class ModelFilter;
class Quaternion;
class Script;
class Texture;
class Tool;
class Undo;
class Vector;

#endif  //__MM3DTYPES_H_INC__

