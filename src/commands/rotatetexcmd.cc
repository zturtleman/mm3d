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

struct RotateTextureCommand : Command
{
	//GEOM_FACES_MENU/GEOM_GROUP_MENU
	RotateTextureCommand(const char *menu):Command(1,menu){}

	virtual const char *getName(int)
	{
		if(getPath()==GEOM_FACES_MENU)
		return "Faces\nRotate Texture Coordinates"; 
		return "Groups\nRotate Texture Coordinates"; 

		(void)TRANSLATE_NOOP("Command","Rotate Texture Coordinates");
	}

	virtual const char *getKeymap(int)
	{
		//REMINDER: Misfit assigns Unhide to Shift+U. This is
		//the current default also.
		//NOTE: I don't know if U is useful. Theoretically it
		//should be as convenient as flipping normals (N) but
		//I don't know how often UVs are legitimately off, if
		//ever, aside from UV editing.
		//NOTE: USING Ctrl+Shift+U SO THE MENUS ARE SAME SIZE.
		return getPath()==GEOM_FACES_MENU?"U":"Ctrl+Shift+U"; 
	}

	virtual bool activated(int, Model *model);
};

extern Command *rotatetexcmd(const char *menu)
{
	return new RotateTextureCommand(menu); 
}
  
bool RotateTextureCommand::activated(int arg, Model *model)
{
	//FIX ME
	if(model->getAnimationMode()!=Model::ANIMMODE_NONE)
	return false;
	
	int_list tris;
	int_list::iterator it;
	model->getSelectedTriangles(tris);
	if(tris.empty())
	{
		model_status(model,StatusError,STATUSTIME_LONG,TRANSLATE("Command","Must select faces"));
		return false;
	}

	if(getPath()==GEOM_FACES_MENU)
	{
		float s,t,oldS,oldT;
		for(it = tris.begin(); it!=tris.end(); it++)
		{
			model->getTextureCoords(*it,2,oldS,oldT);

			model->getTextureCoords(*it,1,s,t);
			model->setTextureCoords(*it,2,s,t);

			model->getTextureCoords(*it,0,s,t);
			model->setTextureCoords(*it,1,s,t);

			model->setTextureCoords(*it,0,oldS,oldT);
		}
	}
	else if(getPath()==GEOM_GROUP_MENU)
	{
		float s,t,temp;
		for(it = tris.begin(); it!=tris.end(); it++)
		{
			model->getTextureCoords(*it,0,s,t);
			temp = s;
			s = 0.5-(t-0.5);
			t = temp;
			model->setTextureCoords(*it,0,s,t);

			model->getTextureCoords(*it,1,s,t);
			temp = s;
			s = 0.5-(t-0.5);
			t = temp;
			model->setTextureCoords(*it,1,s,t);

			model->getTextureCoords(*it,2,s,t);
			temp = s;
			s = 0.5-(t-0.5);
			t = temp;
			model->setTextureCoords(*it,2,s,t);

		}	
	}
	else
	{
		assert(0); return false;
	}
	model_status(model,StatusNormal,STATUSTIME_SHORT,TRANSLATE("Command","Texture coordinates rotated"));
	return true;		
}

