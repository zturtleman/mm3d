/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2009 Kevin Worcester
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
#include "win.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"


//Note: Renaming to match help file URL.
struct GroupCleanWin : Win
{
	void submit(int);

	GroupCleanWin(Model *model)
		:
	Win("Clean Up"),model(model),
	a(main,"Merge identical materials"),
	b(main,"Remove unused materials"),
	c(main,"Merge identical groups"),
	d(main,"Remove unused groups"),
	ok_cancel(main)
	{
		for(auto*ea=&a;ea<=&d;ea++) ea->set();

		active_callback = &GroupCleanWin::submit; 
	}

	Model *model;

	boolean a,b,c,d; 
	ok_cancel_panel ok_cancel; 
};
void GroupCleanWin::submit(int id)
{
	if(id==id_ok)
	{
		int m = model->getTextureCount();
		int n = model->getGroupCount();

		int mim = a?model->mergeIdenticalMaterials():0;
		int rum = b?model->removeUnusedMaterials():0;
		int mig = c?model->mergeIdenticalGroups():0;
		int rug = d?model->removeUnusedGroups():0;

		//::tr("Merged %1 groups,%2 materials; Removed %3 of %4 groups,%5 of %6 materials");
		utf8 fmt = ::tr("Merged %d groups, %d materials; Removed %d of %d groups, %d of %d materials.");
		model_status(model,StatusNormal,STATUSTIME_LONG,fmt,mig,mim,rug,n,rum,m); 

		model->operationComplete(::tr("Group Clean-up","operation complete"));		
	}
	else if(id==id_cancel)
	{
		model->undoCurrent();
	}
	basic_submit(id);
}

extern void groupclean(Model *m){ GroupCleanWin(m).return_on_close(); }
