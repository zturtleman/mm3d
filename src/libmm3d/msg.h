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


#ifndef __MSG_H
#define __MSG_H

#include "mm3dtypes.h"

extern "C" void msg_error(  const char *str);
extern "C" void msg_warning(const char *str);
extern "C" void msg_info(	const char *str);

template<class SFINAE>
inline void msg_error(const SFINAE *fmt,...)
{
	va_list va;
	va_start(va,fmt);
	char buf[2048];
	if(vsnprintf(buf,sizeof(buf),fmt,va)>0)
	msg_error(buf); //NOTE: May truncate.
	else assert(0);
	va_end(va);	
}
template<class SFINAE>
inline void msg_warning(const SFINAE *fmt,...)
{
	va_list va;
	va_start(va,fmt);
	char buf[2048];
	if(vsnprintf(buf,sizeof(buf),fmt,va)>0)
	msg_warning(buf); //NOTE: May truncate.
	else assert(0);
	va_end(va);
}
template<class SFINAE>
inline void msg_info(const SFINAE *fmt,...)
{
	va_list va;
	va_start(va,fmt);
	char buf[2048];
	if(vsnprintf(buf,sizeof(buf),fmt,va)>0)
	msg_info(buf); //NOTE: May truncate.
	else assert(0);
	va_end(va);
}

// O = Ok
// Y = Yes
// N = No
// C = Cancel
// A = Abort
// R = Retry
// I = Ignore
//
// Upper-case means default choice
// C (cancel)is automatically associated with the Escape key
extern "C" char msg_error_prompt(  const char *str, const char *opts = "Ync");
extern "C" char msg_warning_prompt(const char *str, const char *opts = "Ync");
extern "C" char msg_info_prompt(	const char *str, const char *opts = "Ync");

typedef void (*msg_func)(const char *);
typedef char (*msg_prompt_func)(const char *, const char *);

extern "C" void msg_register(msg_func infomsg,msg_func warnmsg,msg_func errmsg);
extern "C" void msg_register_prompt(msg_prompt_func infomsg,
		msg_prompt_func warnmsg,msg_prompt_func errmsg);

#endif // __MSG_H

