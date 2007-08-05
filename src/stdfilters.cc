/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2007 Kevin Worcester
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 *
 *  See the COPYING file for full license text.
 */


#include "stdfilters.h"
#include "filtermgr.h"
#include "md2filter.h"
#include "md3filter.h"
#include "lwofilter.h"
#include "cal3dfilter.h"
#include "cobfilter.h"
#include "dxffilter.h"
#include "objfilter.h"
#include "objprompt.h"
#include "mm3dfilter.h"
#include "ms3dfilter.h"
#include "txtfilter.h"
#include "log.h"

int init_std_filters()
{
   log_debug( "initializing standard filters\n" );

   FilterManager * mgr = FilterManager::getInstance();

   ModelFilter * filter;

   filter = new MisfitFilter();
   mgr->registerFilter( filter );
   
   filter = new Ms3dFilter();
   mgr->registerFilter( filter );
   
   filter = new TextFilter();
   mgr->registerFilter( filter );
   
   filter = new ObjFilter();
   // For plugins you would do this in your plugin_init function
   filter->setOptionsPrompt( objprompt_show );
   mgr->registerFilter( filter );
   
   filter = new LwoFilter();
   mgr->registerFilter( filter );
   
   filter = new Md2Filter();
   mgr->registerFilter( filter );
   
   filter = new Md3Filter();
   mgr->registerFilter( filter );
   
   filter = new Cal3dFilter();
   mgr->registerFilter( filter );
   
   filter = new CobFilter();
   mgr->registerFilter( filter );
   
   filter = new DxfFilter();
   mgr->registerFilter( filter );
   
   return 0;
}

