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

#include "mm3dtypes.h" //PCH

//#include "config.h"
#include "cmdmgr.h"
#include "log.h"
#include "cmdmgr.h"

#include "menuconf.h"

extern void init_std_cmds(CommandManager *cmdMgr)
{
	log_debug("initializing standard commands\n");

	#define _(x) \
	extern Command *x##cmd(); cmdMgr->addCommand(x##cmd());

	_(copy)_(paste)

	cmdMgr->addSeparator();

	// Vertices
	
	//_(makeface)

	_(weld)_(snap)

	cmdMgr->addSeparator(); //NEW

	_(selectfree)


	// Faces
	
	_(makeface)
	//REMINDER: This is positioning to overlap 
	//with Group->Rotate Texture Coordinates.
	//_(rotatetex,"Faces")
	extern Command *rotatetexcmd(const char*);
	cmdMgr->addCommand(rotatetexcmd(GEOM_FACES_MENU));
	_(subdivide)
	_(edgediv)_(edgeturn)
	_(invnormal)_(faceout)
		
	// Groups
					
	//NOTE: This doesn't really have to do with groups.
	//It applies to the texture.
	//_(rotatetex,"Groups")
	extern Command *rotatetexcmd(const char*);
	cmdMgr->addCommand(rotatetexcmd(GEOM_GROUP_MENU));
	
	//cmdMgr->addSeparator(); //NEW

	// Meshes

	_(align)_(simplify)_(cap)_(spherify)

	// Other Geometry Commands

	cmdMgr->addSeparator();

	_(hide)_(flip)_(flatten)
		
	_(dup)_(delete) //Ctrl+D and Ctrl+Shift+D
		
	_(extrude)_(invert) //Invert Selection?

	#undef _
}

