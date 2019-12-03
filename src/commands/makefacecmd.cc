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
#include "msg.h"
#include "modelstatus.h"
#include "cmdmgr.h"
#include "log.h"
#include "command.h"

struct MakeFaceCommand : Command
{
	MakeFaceCommand():Command(1,GEOM_FACES_MENU){} //GEOM_VERTICES_MENU

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Command","Make Face From Vertices");
	}

	virtual const char *getKeymap(int){ return "Enter"; } //EXPERIMENTAL

	virtual bool activated(int,Model*);
};

extern Command *makefacecmd(){ return new MakeFaceCommand; }

bool MakeFaceCommand::activated(int, Model *model)
{
	if(model->getAnimationMode()==Model::ANIMMODE_NONE)
	{
		int_list verts;
		model->getSelectedVertices(verts);
		if(verts.size()==3)
		{
			model_status(model,StatusNormal,STATUSTIME_SHORT,TRANSLATE("Command","Face created"));
			int v1,v2,v3;
			int_list::iterator it = verts.begin();

			v1 = *it;
			it++;
			v2 = *it;
			it++;
			v3 = *it;

			int tri = model->addTriangle(v1,v2,v3);

			//2019: If winding is back-face it will be invisible, and may need to be flipped.
			model->selectTriangle(tri); 

			return true;
		}
		else
		{
			model_status(model,StatusError,STATUSTIME_LONG,TRANSLATE("Command","Must select exactly 3 vertices"));
		}
	}
	return false;
}

