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


#ifndef __MENUCONF_H
#define __MENUCONF_H

/*UNUSED
#define FILE_MENU		 "&File" "|"
#define VIEW_MENU		 "&View" "|"
#define TOOLS_MENU		"&Tools" "|"
#define GEOMETRY_MENU	"&Geometry" "|"
#define MATERIALS_MENU  "&Materials" "|"
#define INFLUENCES_MENU "&Influences" "|"
#define ANIMATION_MENU  "&Animations" "|"
#define HELP_MENU		 "&Help" "|" 
*/

#define TOOLS_SELECT_MENU		TRANSLATE_NOOP("Tool","Select")//;
#define TOOLS_ATTRACT_MENU	  TRANSLATE_NOOP("Tool","Attract")//;
#define TOOLS_BACKGROUND_MENU  TRANSLATE_NOOP("Tool","Background Image")//;
//#define TOOLS_CREATE_MENU		TRANSLATE_NOOP("Tool","Create Other")//;
#define TOOLS_CREATE_MENU		TRANSLATE_NOOP("Tool","Create Shape")//;

#define GEOM_VERTICES_MENU  TRANSLATE_NOOP("Command","Vertices")//;
#define GEOM_FACES_MENU	  TRANSLATE_NOOP("Command","Faces")//;
#define GEOM_GROUP_MENU	  TRANSLATE_NOOP("Command","Group") //NEW
#define GEOM_MESHES_MENU	 TRANSLATE_NOOP("Command","Meshes")//;
//#define GEOM_NORMALS_MENU	TRANSLATE_NOOP("Command","Normals")//;
#define GEOM_NORMALS_MENU GEOM_FACES_MENU

#endif // __MENUCONF_H
