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

#include "menuconf.h"
#include "model.h"
#include "glmath.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"
#include "weld.h"
#include "command.h"

struct WeldCommand : Command
{
	WeldCommand():Command(2,GEOM_VERTICES_MENU){}

	virtual const char *getName(int arg)
	{
		switch(arg)
		{		
		default: assert(0);
		case 0: return TRANSLATE_NOOP("Command","Weld Vertices"); 
		//Changing to Explode helps with memorization.
		//case 1: return TRANSLATE_NOOP("Command","Unweld Vertices");
		case 1: return TRANSLATE_NOOP("Command","Explode Vertices");
		}
	}

	virtual const char *getKeymap(int arg)
	{
		return arg?"Ctrl+E":"Ctrl+W"; //Explode
	}

	virtual bool activated(int,Model*);
};

extern Command *weldcmd(){ return new WeldCommand; }

bool WeldCommand::activated(int arg, Model *model)
{
	int_list vert;
	model->getSelectedVertices(vert);

	if(arg==0) //Weld
	{		
		if(vert.size()>1)
		{
			int unwelded = 0, welded = 0;
			weldSelectedVertices(model,0.0001,unwelded,welded);
			//TRANSLATE("Command","Welded %1 vertices into %2 vertices").arg(unwelded).arg(welded)
			const char *fmt = TRANSLATE("Command","Welded %d vertices into %d vertices");
			model_status(model,StatusNormal,STATUSTIME_SHORT,fmt,unwelded,welded);
			return true;
		}
		model_status(model,StatusError,STATUSTIME_LONG,
		TRANSLATE("Command","You must have 2 or more vertices selected to weld."));
		return false;
	}
	else //Unweld
	{
		if(!vert.empty())
		{
			int unwelded = 0, welded = 0;
			unweldSelectedVertices(model,unwelded,welded);
			//TRANSLATE("Command","Unwelded %1 vertices into %2 vertices").arg(welded).arg(unwelded)
			const char *fmt = TRANSLATE("Command","Unwelded %d vertices into %d vertices");
			model_status(model,StatusNormal,STATUSTIME_SHORT,fmt,welded,unwelded);
			return true;
		}
		model_status(model,StatusError,STATUSTIME_LONG,
		TRANSLATE("Command","You must have 1 or more vertices selected to unweld."));
		return false;
	}
}