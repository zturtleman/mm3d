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


#ifndef __SYSCONF_H
#define __SYSCONF_H

/*???
#ifndef WIN32
#define HAVE_BASENAME
#define HAVE_DIRNAME
#endif*/

#include <string>

void init_sysconf();

const std::string &getMm3dHomeDirectory();
const std::string &getDocDirectory();
const std::string &getI18nDirectory();
const std::string &getPluginDirectory();
const std::string &getSharedPluginDirectory();

const std::string &getConfigFile();

#endif // __SYSCONF_H
