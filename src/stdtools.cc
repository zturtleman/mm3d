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

#include "toolbox.h"
#include "log.h"

static void stdtools_func(Toolbox *tb)
{
	log_debug("initializing standard tools\n");

	#define _(x) \
	extern Tool *x##tool(); tb->addTool(x##tool());

	_(select) //C V B F G cluster (O P)

	tb->addSeparator();

	_(scale)_(shear) //S Shift+S
	_(rotate)_(move)_(dragvertex) //R T Shift+T

	_(attract)

	tb->addSeparator();
	
	//2019: Inverting this from "Create Other" submenu to
	//Create Shapes menu instead.
	//_(vertex)_(cube)_(ellipse)_(cylinder)_(torus)_(poly)
	//_(rectangle)_(joint)_(point)_(proj)
	_(poly)_(vertex)
	_(extrude) //X? E? Extrude makes geometry, like poly.
	_(rectangle)_(cube)_(ellipse)_(cylinder)_(torus) //Shapes
	_(joint)_(point)_(proj)
 
	tb->addSeparator();

	_(background)

	#undef _
}
extern void stdtools_init()
{
	Toolbox::registerToolFunction(stdtools_func);
}
