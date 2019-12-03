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
#include "filtermgr.h"
#include "texmgr.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"
#include "sysconf.h"
#include "misc.h"
#include "command.h"

struct PasteCommand : Command
{
	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Command","Paste from Clipboard"); 
	}

	virtual const char *getKeymap(int){ return "Ctrl+V"; }

	virtual bool activated(int,Model*);
};

extern Command *pastecmd(){ return new PasteCommand; }

bool PasteCommand::activated(int arg, Model *model)
{
	Model *merge = new Model;

	std::string clipfile = getMm3dHomeDirectory();

	clipfile += "/clipboard";
	mkpath(clipfile.c_str()/*,0755*/);
	clipfile += "/clipboard.mm3d";

	if(auto err=FilterManager::getInstance()->readFile(merge,clipfile.c_str()))
	{
		if(Model::operationFailed(err))
		msg_error("%s:\n%s",clipfile.c_str(),modelErrStr(err,merge));
		delete merge; 
		return false;
	}
	model->mergeModels(merge,true,Model::AM_NONE,false);
	model_status(model,StatusNormal,STATUSTIME_SHORT,
	TRANSLATE("Command","Paste complete"));
	delete merge;
	return true;
}
