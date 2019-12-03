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

#include "modelstatus.h"
#include "model.h"
#include "mm3dport.h"

#include "msg.h" //NEW

static StatusObjectFunction s_func = nullptr;

void model_status_register_function(StatusObjectFunction func)
{
	s_func = func;
}

StatusObject *model_status_get_object(Model *model)
{
	return s_func(model);
}

//2019: Found this in statusbar.cc. 
void model_status(Model *model, StatusTypeE type, int ms, const char *fmt,...)
{
	char temp[1024];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(temp,sizeof(temp),fmt,ap);
	//Implementing this renders model_status_get_object pointless?
	//if(StatusObject*bar=StatusBar::getStatusBarFromModel(model))
	if(StatusObject*bar=model_status_get_object(model))
	{
		bar->addText(type,ms,temp);
	}
	else if(type==StatusError)
	{
		if(!model)
		{
			//Something like this could work, except this pops up
			//a message box.
			//msg_error(msg);

			assert(model); //NEW
		}
		else model->pushError(temp);
	}
}