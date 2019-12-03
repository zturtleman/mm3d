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

#ifndef __DRAWCONTEXT_H
#define __DRAWCONTEXT_H

#include <list>
#include <map>
#include <string>
#include <vector>

typedef void *ContextT;

typedef std::map<std::string,int> FileTextureMap;
typedef std::vector<int> MaterialTextureList;

	//NOTE: How this works is it duplicates video 
	//memory and OpenGL lists. It's best to avoid.
	//In the original MM3D every viewport used it
	//so they all had their own hardware textures.
	//It would include glGenBuffers also if added.

class DrawingContext //Qt throwback (UNUSED)
{
	public:
		ContextT	 m_context;
		FileTextureMap m_fileTextures;
		MaterialTextureList m_matTextures;
		bool		  m_valid;

		int			m_currentTexture;
};

typedef std::vector<DrawingContext*> DrawingContextList;

#endif // __DRAWCONTEXT_H
