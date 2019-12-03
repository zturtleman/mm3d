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

#include "filtermgr.h"
#include "log.h"

//REMOVE US
#include "mm3dfilter.h"
#include "md2filter.h"
#include "md3filter.h"
//#include "lwofilter.h"
//#include "cal3dfilter.h"
//#include "cobfilter.h"
//#include "dxffilter.h"
#include "objfilter.h"
#include "ms3dfilter.h"
//#include "txtfilter.h"
#include "iqefilter.h"
#include "smdfilter.h"

extern void init_std_filters()
{
	log_debug("initializing standard filters\n");

	FilterManager *mgr = FilterManager::getInstance();

	ModelFilter *filter;

	filter = new MisfitFilter();
	mgr->registerFilter(filter);
	
	filter = new Ms3dFilter();
	extern ModelFilter::PromptF ms3dprompt;
	filter->setOptionsPrompt(ms3dprompt);
	mgr->registerFilter(filter);
	
	//filter = new TextFilter();
	//mgr->registerFilter(filter);
	
	filter = new ObjFilter();
	extern ModelFilter::PromptF objprompt;
	filter->setOptionsPrompt(objprompt);
	mgr->registerFilter(filter);
	
	//filter = new LwoFilter();
	//mgr->registerFilter(filter);
	
	filter = new Md2Filter();
	mgr->registerFilter(filter);
	
	//Won't build.
	//https://github.com/zturtleman/mm3d/issues/74
	//filter = new Md3Filter();
	//mgr->registerFilter(filter);
	
	//filter = new Cal3dFilter();
	//filter->setOptionsPrompt(cal3dprompt_show);
	//mgr->registerFilter(filter);
	
	//filter = new CobFilter();
	//mgr->registerFilter(filter);
	
	//filter = new DxfFilter();
	//mgr->registerFilter(filter);
	
	filter = new IqeFilter();
	extern ModelFilter::PromptF iqeprompt;
	filter->setOptionsPrompt(iqeprompt);
	mgr->registerFilter(filter);
	
	filter = new SmdFilter();
	extern ModelFilter::PromptF smdprompt;
	filter->setOptionsPrompt(smdprompt);
	mgr->registerFilter(filter);
}

