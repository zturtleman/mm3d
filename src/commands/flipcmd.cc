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

#include "model.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"
#include "command.h"

struct FlipCommand : Command
{
	FlipCommand():Command(3,
	TRANSLATE_NOOP("Command","Flip")){}

	virtual const char *getName(int arg)
	{
		switch(arg)
		{
		default: assert(0);
		case 0: return TRANSLATE_NOOP("Command","Flip X");
		case 1: return TRANSLATE_NOOP("Command","Flip Y");
		case 2: return TRANSLATE_NOOP("Command","Flip Z");
		}
	}

	virtual bool activated(int,Model*);
};

extern Command *flipcmd(){ return new FlipCommand; }

bool FlipCommand::activated(int index, Model *model)
{
	pos_list posList;
	model->getSelectedPositions(posList);

	if(posList.empty())
	{
		model_status(model,StatusError,STATUSTIME_LONG,TRANSLATE("Command","Need at least 1 vertex,joint,point,or face selected"));
		return false;
	}

	{
		 pos_list::iterator it;
		 for(it = posList.begin(); it!=posList.end(); it++)
		 {
			  double coords[3];
			  model->getPositionCoords(*it,coords);
			  coords[index] = -coords[index];
			  model->movePosition(*it,coords[0],coords[1],coords[2]);
		 }
	}

	{
		 int_list face;
		 int_list::iterator it;
		 model->getSelectedTriangles(face);

		 for(it = face.begin(); it!=face.end(); it++)
		 {
			  model->invertNormals(*it);
		 }
	}

	model_status(model,StatusNormal,STATUSTIME_SHORT,TRANSLATE("Command","Selected primitives flipped"));

	return true;
}

