/*  Maverick Model 3D
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


#ifndef __VERSION_H
#define __VERSION_H

#include "config.h"

// If you change the version, also change the following files
// (relative to top source directory)
//   configure.ac (for x.x.x)
//   mm3d-win32-installer.nsi (for x.x.x)
//   Makefile.am (for Contents/PlugIns/mm3d/x.x)
//   Makefile.generic (for plugins/x.x)
//   plugins/Makefile.am (for plugins/x.x)

#ifndef VERSION
#define VERSION "1.3.10"
#endif

#define VERSION_MAJOR 1
#define VERSION_MINOR 3
#define VERSION_PATCH 10

// Set to 0 for stable releases (even minor version) and 1 for
// development versions (odd minor version)
#define MM3D_DEVEL_VERSION 1

#define VERSION_STRING VERSION

#endif // __VERSION_H
