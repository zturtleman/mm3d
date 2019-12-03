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


#ifndef __TRANSLATE_H
#define __TRANSLATE_H

#include "mm3dtypes.h"

//QT_TRANSLATE_NOOP
#undef TRANSLATE_NOOP 
#define TRANSLATE_NOOP(a,b,...) b

const char *transll(const char *ctxt, const char *msg);

inline const char *transll(const char *msg)
{
	//model.cc
	//texture.cc
	//cmdline.cc
	return transll("LowLevel",msg);
}

//https://github.com/zturtleman/mm3d/issues/101

//Might want to leave open the possibility of TRANSLATE being a 
//class enum constant, and ensure it's not treated as a literal.
//#define TRANSLATE transll
inline const char *TRANSLATE(const char *ctxt, const char *msg, ...)
{
	return transll(ctxt,msg);
}

typedef const char *(*TransLLCallbackF)(const char *ctxt, const char *msg);

void transll_install_handler(TransLLCallbackF);

#endif // __TRANSLATE_H
