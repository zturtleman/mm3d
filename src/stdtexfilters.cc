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

#include "texmgr.h"
//#include "pcxtex.h"
//#include "tgatex.h"
//#include "rawtex.h" //REMOVED
//#include "qttex.h" //REMOVED
#include "log.h"
#include "datasource.h"

extern void init_std_texture_filters()
{
	log_debug("initializing standard texture filters\n");

	auto tm = TextureManager::getInstance();

	//2019: The code for this (made up?) filter/format is in the
	//source code of past versions of MMM3d if anyone is curious.		
	//tm->registerTextureFilter(new RawTextureFilter); //REMOVED
	//tm->registerTextureFilter(new QtTextureFilter);
	extern TextureFilter *ui_texfilter(); 
	TextureFilter *ui = ui_texfilter();
	if(!ui->canRead("TGA"))
	{
		extern TextureFilter *tgatex();
		tm->registerTextureFilter(tgatex()); //REMOVE?
	}
	if(!ui->canRead("PCX"))
	{
		extern TextureFilter *pcxtex();
		tm->registerTextureFilter(pcxtex()); //REMOVE?
	}
	tm->registerTextureFilter(ui);
}

