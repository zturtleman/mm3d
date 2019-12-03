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

#ifndef __GLHEADERS_H__
#define __GLHEADERS_H__

//#include "config.h" //IS_OSX

#ifndef GLU_VERSION_1_1 //2019

#if defined(_WIN32) && !defined(APIENTRY) //2019
#define __GLHEADERS_H__APIENTRY
#define WINGDIAPI __declspec(dllimport)
#define APIENTRY __stdcall
#define CALLBACK __stdcall //GLU
#endif

//NOTE: __APPLE__ is used elsewhere. Regardless,
//in this case, it's safe.
//#ifdef IS_OSX //config.h
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h> //GLU
#else
#include <GL/gl.h>
#include <GL/glu.h> //GLU
#endif //

#ifdef __GLHEADERS_H__APIENTRY
#undef WINGDIAPI
#undef APIENTRY
#undef CALLBACK
#endif

#endif //GLU_VERSION_1_1

#endif //__GLHEADERS_H__
