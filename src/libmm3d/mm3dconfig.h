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


//#include "config.h" //PREFIX //REMOVE ME
#include "copyright.h" //MM3D_PRODUCT/COPYRIGHT

#ifdef WIN32
//#define HOME_MM3D				 "/Maverick Model 3D" 
#define HOME_MM3D				 "/" MM3D_PRODUCT
#elif defined __APPLE__
//#define HOME_MM3D				 "/Library/Application Support/Maverick Model 3D"
#define HOME_MM3D				 "/Library/Application Support/" MM3D_PRODUCT
#else
#define HOME_MM3D				 "/.mm3d"
#endif // WIN32

#ifdef WIN32
#define SHARED_PLUGINS		  "/plugins" //"\\plugins"
#define HOME_PLUGINS			 "/plugins" //"\\plugins"
#else
// NOTE: macOS AppBundle uses shared "Contents/PlugIns/mm3d" path in the app bundle.
#if !defined(SHARED_PLUGINS)
#define SHARED_PLUGINS		  PREFIX "/share/mm3d/plugins"
#endif
#define HOME_PLUGINS			 "/plugins"
#endif // WIN32

#ifdef WIN32
#define HOME_RC					"/mm3drc" //"\\mm3drc"
#else
#define HOME_RC					"/mm3drc"
#endif // WIN32

#ifdef WIN32
#define DOC_ROOT				"/doc/html"  //"\\doc\\html"
#define I18N_ROOT				"/i18n" //"\\i18n"
#else
// NOTE: macOS AppBundle uses "Contents/SharedSupport/mm3d/..." paths in the app bundle.
#if !defined(DOC_ROOT)
#define DOC_ROOT				  PREFIX "/share/doc/mm3d/html"
#endif
#define I18N_ROOT				 PREFIX "/share/mm3d/i18n"
#endif // WIN32

#define DIR_SLASH				 '/'

#ifdef WIN32
#define FILE_NEWLINE			 "\r\n"
#else
#define FILE_NEWLINE			 "\n"
#endif // WIN32

