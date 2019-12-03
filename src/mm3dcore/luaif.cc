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

#include "luaif.h"

#ifdef HAVE_LUALIB

#include "scriptif.h"

#include "log.h"
#include "model.h"
#include "filtermgr.h"

//-----------------------------------------------------------------------------
// Context functions
//-----------------------------------------------------------------------------

LuaContext::LuaContext(Model *model)
{
	m_currentModel = model;
	m_scriptModel  = model;
}

LuaContext::~LuaContext()
{
	for(unsigned m = 0; m<m_list.size(); m++)
	{
		delete m_list[m];
	}
	m_list.clear();

	for(unsigned t = 0; t<m_createdRectangles.size(); t++)
	{
		delete m_createdRectangles[t];
	}
	m_createdRectangles.clear();
}

//-----------------------------------------------------------------------------
// Script functions
//-----------------------------------------------------------------------------

void _luaif_error(lua_State *L, const char *str)
{
	lua_pushstring(L,str);
	lua_error(L);
}

extern "C" int luaif_die(lua_State *L)
{
	std::string errstr = "die called by script";

	if(lua_gettop(L)>0&&lua_isstring(L,1))
	{
		errstr = lua_tostring(L,1);
	}
	_luaif_error(L,errstr.c_str());

	return 0;
}

extern "C" int luaif_getModel(lua_State *L)
{
	log_debug("getModel called\n");
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	lua_pushlightuserdata(L,model);
	return 1;
}

extern "C" int luaif_modelGetName(lua_State *L)
{
	log_debug("modelGetName called\n");
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(model)
	{
		lua_pushstring(L,scriptif_modelGetName(model));
	}
	else
	{
		lua_pushstring(L,"[null]");
	}
	return 1;
}

extern "C" int luaif_modelSaveAs(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	bool success = false;
	if(lua_gettop(L)>0&&lua_isstring(L,1))
	{
		const char *filename = lua_tostring(L,1);

		log_debug("modelSaveAs called(%p,%s)\n",model,filename);

		success = scriptif_modelSaveAs(model,filename);
	}
	else
	{
		_luaif_error(L,"usage: modelSaveAs(file_string)");
	}

	lua_pushboolean(L,success);
	return 1;
}

//-----------------------------------------------------------------------------
// Primitive Creation
//-----------------------------------------------------------------------------

extern "C" int luaif_modelCreateMeshRectangle(lua_State *L)
{
	int index = -1;
	int args  = lua_gettop(L);

	log_debug("modelCreateMeshRectangle called()\n");

	bool argsCorrect = false;

	if(args==12||args==13)
	{
		argsCorrect = true;
		for(int n = 1; argsCorrect&&n<=args; n++)
		{
			if(!lua_isnumber(L,n))
			{
				argsCorrect = false;
			}
		}
	}

	if(argsCorrect)
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		double x0 = lua_tonumber(L, 1);
		double y0 = lua_tonumber(L, 2);
		double z0 = lua_tonumber(L, 3);
		double x1 = lua_tonumber(L, 4);
		double y1 = lua_tonumber(L, 5);
		double z1 = lua_tonumber(L, 6);
		double x2 = lua_tonumber(L, 7);
		double y2 = lua_tonumber(L, 8);
		double z2 = lua_tonumber(L, 9);
		double x3 = lua_tonumber(L,10);
		double y3 = lua_tonumber(L,11);
		double z3 = lua_tonumber(L,12);
		unsigned segments = 1;

		if(args==13)
		{
			segments = (unsigned)lua_tonumber(L,13);
		}

		MeshRectangle *mesh = scriptif_modelCreateMeshRectangle(
				model,x0,y0,z0,x1,y1,z1,
						 x2,y2,z2,x3,y3,z3,segments);
		if(mesh)
		{
			index = LC->m_createdRectangles.size();
			LC->m_createdRectangles.push_back(mesh);
		}
	}
	else
	{
		_luaif_error(L,"usage: modelCreateMeshRectangle(x1,y1,z1,... x4,y4,z4)");
	}

	lua_pushnumber(L,index);
	return 1;
}

extern "C" int luaif_modelCreateBoneJoint(lua_State *L)
{
	int index = -1;
	int args  = lua_gettop(L);

	log_debug("modelCreateBoneJoint called()\n");

	bool argsCorrect = false;
	if(args==5||args==8)
	{
		argsCorrect = true;
		if(!lua_isstring(L,1))
		{
			argsCorrect = false;
		}

		for(int n = 2; n<=args; n++)
		{
			if(!lua_isnumber(L,n))
			{
				argsCorrect = false;
			}
		}
	}

	if(argsCorrect)
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		const char *name	= lua_tostring(L,1);
		double		 x		= lua_tonumber(L,2);
		double		 y		= lua_tonumber(L,3);
		double		 z		= lua_tonumber(L,4);
		int			 parent = (int)lua_tonumber(L,5);

		double xrot = 0.0;
		double yrot = 0.0;
		double zrot = 0.0;

		if(args==8)
		{
			xrot	= lua_tonumber(L,6);
			yrot	= lua_tonumber(L,7);
			zrot	= lua_tonumber(L,8);
		}

		index = scriptif_modelCreateBoneJoint(model,
				name,x,y,z,xrot,yrot,zrot,parent);
	}
	else
	{
		_luaif_error(L,"usage: modelCreateBoneJoint(name_string,x,y,z,parent [,xrot,yrot,zrot])");
	}

	lua_pushnumber(L,index);
	return 1;
}

//-----------------------------------------------------------------------------
// Primitive Manipulation
//-----------------------------------------------------------------------------

extern "C" int luaif_vertexSetCoords(lua_State *L)
{
	if(lua_gettop(L)==4&&lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3)&&lua_isnumber(L,4))
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		unsigned vert = (unsigned)lua_tonumber(L,1);
		double x = (double)lua_tonumber(L,2);
		double y = (double)lua_tonumber(L,3);
		double z = (double)lua_tonumber(L,4);

		scriptif_vertexSetCoords(model,vert,x,y,z);
	}
	else
	{
		_luaif_error(L,"usage: vertexSetCoords(vertex_num,x,y,z)");
	}

	return 0;
}

extern "C" int luaif_modelAddTexture(lua_State *L)
{
	int index = -1;

	if(lua_gettop(L)==1&&lua_isstring(L,1))
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		const char *name	= lua_tostring(L,1);

		index = scriptif_modelAddTexture(model,name);
	}
	else
	{
		_luaif_error(L,"usage: modelAddTexture(name_string)");
	}

	lua_pushnumber(L,index);
	return 1;
}

extern "C" int luaif_groupSetTexture(lua_State *L)
{
	if(lua_gettop(L)==2&&lua_isnumber(L,1)&&lua_isnumber(L,2))
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		unsigned groupId	= (unsigned)lua_tonumber(L,1);
		unsigned textureId = (unsigned)lua_tonumber(L,2);

		scriptif_groupSetTexture(model,groupId,textureId);
	}
	else
	{
		_luaif_error(L,"usage: groupSetTexture(group_num,texture_num)");
	}

	return 0;
}

extern "C" int luaif_faceSetTextureCoords(lua_State *L)
{
	if(lua_gettop(L)==4&&lua_isnumber(L,1)&&lua_isnumber(L,2)
		 &&lua_isnumber(L,3)&&lua_isnumber(L,4))
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		unsigned faceId		= (unsigned)lua_tonumber(L,1);
		unsigned vertexIndex = (unsigned)lua_tonumber(L,2);
		double s = lua_tonumber(L,3);
		double t = lua_tonumber(L,4);

		scriptif_faceSetTextureCoords(model,faceId,vertexIndex,s,t);
	}
	else
	{
		_luaif_error(L,"usage: faceSetTextureCoords(group_num,texture_num,s_coord,t_coord)");
	}

	return 0;
}

extern "C" int luaif_selectedRotate(lua_State *L)
{
	if(lua_gettop(L)==3&&lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3))
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		double x = (double)lua_tonumber(L,1);
		double y = (double)lua_tonumber(L,2);
		double z = (double)lua_tonumber(L,3);

		scriptif_selectedRotate(model,x,y,z);
	}
	else
	{
		_luaif_error(L,"usage: selectedRotate(x,y,z)");
	}

	return 0;
}

extern "C" int luaif_selectedTranslate(lua_State *L)
{
	if(lua_gettop(L)==3&&lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3))
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		double x = (double)lua_tonumber(L,1);
		double y = (double)lua_tonumber(L,2);
		double z = (double)lua_tonumber(L,3);

		scriptif_selectedTranslate(model,x,y,z);
	}
	else
	{
		_luaif_error(L,"usage: selectedTranslate(x,y,z)");
	}

	return 0;
}

extern "C" int luaif_selectedScale(lua_State *L)
{
	if(lua_gettop(L)==3&&lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3))
	{
		LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
		Model *model = LC->m_currentModel;

		double x = (double)lua_tonumber(L,1);
		double y = (double)lua_tonumber(L,2);
		double z = (double)lua_tonumber(L,3);

		scriptif_selectedScale(model,x,y,z);
	}
	else
	{
		_luaif_error(L,"usage: selectedScale(x,y,z)");
	}

	return 0;
}

extern "C" int luaif_selectedApplyMatrix(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	int args = lua_gettop(L);

	bool argsCorrect = false;

	if(args==16)
	{
		argsCorrect = true;
		for(int n = 1; n<=args; n++)
		{
			if(!lua_isnumber(L,n))
			{
				argsCorrect = false;
			}
		}
	}

	if(argsCorrect)
	{
		double m0  = (double)lua_tonumber(L, 1);
		double m1  = (double)lua_tonumber(L, 2);
		double m2  = (double)lua_tonumber(L, 3);
		double m3  = (double)lua_tonumber(L, 4);
		double m4  = (double)lua_tonumber(L, 5);
		double m5  = (double)lua_tonumber(L, 6);
		double m6  = (double)lua_tonumber(L, 7);
		double m7  = (double)lua_tonumber(L, 8);
		double m8  = (double)lua_tonumber(L, 9);
		double m9  = (double)lua_tonumber(L,10);
		double m10 = (double)lua_tonumber(L,11);
		double m11 = (double)lua_tonumber(L,12);
		double m12 = (double)lua_tonumber(L,13);
		double m13 = (double)lua_tonumber(L,14);
		double m14 = (double)lua_tonumber(L,15);
		double m15 = (double)lua_tonumber(L,16);

		scriptif_selectedApplyMatrix(model,
				m0, m1, m2, m3,
				m4, m5, m6, m7,
				m8, m9, m10,m11,
				m12,m13,m14,m15);
	}
	else
	{
		_luaif_error(L,"usage: selectedApplyMatrix(m0 .. m15)");
	}

	return 0;
}

extern "C" int luaif_selectedWeldVertices(lua_State *L)
{
	log_debug("selectedWeldVertices\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_selectedWeldVertices(model);
	return 0;
}

extern "C" int luaif_selectedInvertNormals(lua_State *L)
{
	log_debug("selectedInvertNormals\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_selectedInvertNormals(model);
	return 0;
}

extern "C" int luaif_selectedGroupFaces(lua_State *L)
{
	log_debug("selectedGroupFaces\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int id = -1;

	if(lua_gettop(L)==1&&lua_isstring(L,1))
	{
		const char *name = lua_tostring(L,1);
		id = scriptif_selectedGroupFaces(model,name);
	}
	else
	{
		_luaif_error(L,"usage: selectedGroupFaces(name_string)");
	}
	
	lua_pushnumber(L,id);
	return 1;
}

extern "C" int luaif_selectedAddToGroup(lua_State *L)
{
	log_debug("selectedAddToGroup\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)&&lua_isnumber(L,1))
	{
		int groupId = (int)lua_tonumber(L,1);
		scriptif_selectedAddToGroup(model,groupId);
	}
	else
	{
		_luaif_error(L,"usage: selectedAddToGroup(group_num)");
	}

	return 0;
}

extern "C" int luaif_modelGetVertexCount(lua_State *L)
{
	log_debug("modelGetVertexCount\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int num = scriptif_modelGetVertexCount(model);
	lua_pushnumber(L,num);

	return 1;
}

extern "C" int luaif_modelGetFaceCount(lua_State *L)
{
	log_debug("modelGetFaceCount\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int num = scriptif_modelGetFaceCount(model);
	lua_pushnumber(L,num);

	return 1;
}

extern "C" int luaif_modelGetGroupCount(lua_State *L)
{
	log_debug("modelGetGroupCount\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int num = scriptif_modelGetGroupCount(model);
	lua_pushnumber(L,num);

	return 1;
}

extern "C" int luaif_modelGetJointCount(lua_State *L)
{
	log_debug("modelGetJointCount\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int num = scriptif_modelGetJointCount(model);
	lua_pushnumber(L,num);

	return 1;
}

extern "C" int luaif_modelGetTextureCount(lua_State *L)
{
	log_debug("modelGetTextureCount\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int num = scriptif_modelGetTextureCount(model);
	lua_pushnumber(L,num);

	return 1;
}

extern "C" int luaif_modelGetGroupByName(lua_State *L)
{
	log_debug("modelGetGroupByName\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int id = -1;

	if(lua_gettop(L)&&lua_isstring(L,1))
	{
		const char *name = lua_tostring(L,1);
		id = scriptif_modelGetGroupByName(model,name);
	}
	else
	{
		_luaif_error(L,"usage: modelGetGroupByName(name_string)");
	}

	lua_pushnumber(L,id);

	return 1;
}

extern "C" int luaif_groupGetName(lua_State *L)
{
	log_debug("groupGetName\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	const char *name = "";

	if(lua_gettop(L)&&lua_isnumber(L,1))
	{
		unsigned num = (unsigned)lua_tonumber(L,1);
		name = scriptif_groupGetName(model,num);
	}
	else
	{
		_luaif_error(L,"usage: groupGetName(group_num)");
	}

	lua_pushstring(L,name);

	return 1;
}

extern "C" int luaif_modelGetTextureByName(lua_State *L)
{
	log_debug("modelGetTextureByName\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int id = -1;

	if(lua_gettop(L)&&lua_isstring(L,1))
	{
		const char *name = lua_tostring(L,1);
		id = scriptif_modelGetTextureByName(model,name);
	}
	else
	{
		_luaif_error(L,"usage: modelGetTextureByName(name_string)");
	}

	lua_pushnumber(L,id);

	return 1;
}

extern "C" int luaif_textureGetName(lua_State *L)
{
	log_debug("textureGetName\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	const char *name = "";

	if(lua_gettop(L)&&lua_isnumber(L,1))
	{
		unsigned num = (unsigned)lua_tonumber(L,1);
		name = scriptif_textureGetName(model,num);
	}
	else
	{
		_luaif_error(L,"usage: textureGetName(texture_num)");
	}

	lua_pushstring(L,name);

	return 1;
}

extern "C" int luaif_textureGetFilename(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	const char *filename = "";

	if(lua_gettop(L)&&lua_isnumber(L,1))
	{
		unsigned num = (unsigned)lua_tonumber(L,1);
		filename = scriptif_textureGetFilename(model,num);
	}
	else
	{
		_luaif_error(L,"usage: textureGetFilename(texture_num)");
	}

	lua_pushstring(L,filename);

	return 1;
}

//-----------------------------------------------------------------------------
// Animation
//-----------------------------------------------------------------------------

static Model::AnimationModeE _stringToAnimationMode(const char *str)
{
	if(PORT_strcasecmp(str,"skeletal")==0)
	{
		return Model::ANIMMODE_SKELETAL;
	}
	else if(PORT_strcasecmp(str,"frame")==0)
	{
		return Model::ANIMMODE_FRAME;
	}
	else
	{
		log_error("Unknown animation type %s",str);
		return Model::ANIMMODE_NONE;
	}
}

extern "C" int luaif_setAnimByIndex(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int args = lua_gettop(L);
	bool success = false;

	if(args==2&&lua_isstring(L,1)&&lua_isnumber(L,2))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned index = (unsigned)lua_tonumber(L,2);

		success = scriptif_setAnimByIndex(model,mode,index);
	}
	else
	{
		_luaif_error(L,"usage: setAnimByIndex(type_string,anim_num)");
	}

	lua_pushboolean(L,success);

	return 1;
}

extern "C" int luaif_setAnimByName(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int args = lua_gettop(L);
	bool success = false;

	if(args==2&&lua_isstring(L,1)&&lua_isstring(L,2))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		const char *name = lua_tostring(L,2);

		success = scriptif_setAnimByName(model,mode,name);
	}
	else
	{
		_luaif_error(L,"usage: setAnimByName(type_string,name_string)");
	}

	lua_pushboolean(L,success);

	return 1;
}

extern "C" int luaif_animSetFrame(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==1&&lua_isnumber(L,1))
	{
		unsigned frame = (unsigned)lua_tonumber(L,1);
		scriptif_animSetFrame(model,frame);
	}
	else
	{
		_luaif_error(L,"usage: animSetFrame(type_string,frame_num)");
	}

	return 0;
}

extern "C" int luaif_setAnimTime(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==1&&lua_isnumber(L,1))
	{
		double seconds = lua_tonumber(L,1);
		scriptif_setAnimTime(model,seconds);
	}
	else
	{
		_luaif_error(L,"usage: setAnimTime(type_string,seconds)");
	}

	return 0;
}

extern "C" int luaif_modelCreateAnimation(lua_State *L)
{
	int index = -1;

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==4&&lua_isstring(L,1)&&lua_isstring(L,2)
		  &&lua_isnumber(L,3)&&lua_isnumber(L,4))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		const char *name = lua_tostring(L,2);
		unsigned	  frameCount = (unsigned)lua_tonumber(L,3);
		double		 fps = (double)lua_tonumber(L,4);
		index = scriptif_modelCreateAnimation(model,mode,name,frameCount,fps);
	}
	else
	{
		_luaif_error(L,"usage: modelCreateAnimation(type_string,name_string,frame_count,frames_per_second)");
	}

	lua_pushnumber(L,index);
	return 1;
}

extern "C" int luaif_animGetCount(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int count = 0;

	if(lua_gettop(L)==1&&lua_isstring(L,1))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));

		count = scriptif_animGetCount(model,mode);
	}
	else
	{
		_luaif_error(L,"usage: animGetCount(type_string)");
	}

	lua_pushnumber(L,count);
	return 1;
}

extern "C" int luaif_animGetName(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	const char *name = "";

	if(lua_gettop(L)==2&&lua_isstring(L,1)&&lua_isnumber(L,2))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned animIndex = (unsigned)lua_tonumber(L,2);

		name = scriptif_animGetName(model,mode,animIndex);
	}
	else
	{
		_luaif_error(L,"usage: animGetName(type_string,anim_num)");
	}

	lua_pushstring(L,name);
	return 1;
}

extern "C" int luaif_animGetFrameCount(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int count = 0;

	if(lua_gettop(L)==2&&lua_isstring(L,1)&&lua_isnumber(L,2))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned animIndex = (unsigned)lua_tonumber(L,2);

		count = scriptif_animGetFrameCount(model,mode,animIndex);
	}
	else
	{
		_luaif_error(L,"usage: animGetFrameCount(type_string,anim_num)");
	}

	lua_pushnumber(L,count);
	return 1;
}

extern "C" int luaif_animSetName(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==3&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isstring(L,3))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned animIndex = (unsigned)lua_tonumber(L,2);
		const char *name = lua_tostring(L,3);
		scriptif_animSetName(model,mode,animIndex,name);
	}
	else
	{
		_luaif_error(L,"usage: animSetName(type_string,anim_num,name_string)");
	}

	return 0;
}

extern "C" int luaif_animSetFrameCount(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==3&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isnumber(L,3))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned animIndex = (unsigned)lua_tonumber(L,2);
		unsigned frameCount = (unsigned)lua_tonumber(L,3);

		scriptif_animSetFrameCount(model,mode,animIndex,frameCount);
	}
	else
	{
		_luaif_error(L,"usage: animSetFrame(type_string,anim_num,frame_count)");
	}

	return 0;
}

extern "C" int luaif_animSetFPS(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==3&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isnumber(L,3))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned animIndex = (unsigned)lua_tonumber(L,2);
		double fps = (double)lua_tonumber(L,3);

		scriptif_animSetFPS(model,mode,animIndex,fps);
	}
	else
	{
		_luaif_error(L,"usage: animSetFPS(type_string,anim_num,frames_per_second)");
	}

	return 0;
}

extern "C" int luaif_animCopyFrame(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==4&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isnumber(L,3)&&lua_isnumber(L,4))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned anim = (unsigned)lua_tonumber(L,2);
		unsigned frame1 = (unsigned)lua_tonumber(L,3);
		unsigned frame2 = (unsigned)lua_tonumber(L,4);

		scriptif_animCopyFrame(model,mode,anim,frame1,frame2);
	}
	else
	{
		_luaif_error(L,"usage: animCopyFrame(type_string,anim_num,source_frame,dest_frame)");
	}

	return 0;
}

extern "C" int luaif_animClearFrame(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==3&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isnumber(L,3))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned animIndex = (unsigned)lua_tonumber(L,2);
		unsigned frame = (unsigned)lua_tonumber(L,3);

		scriptif_animClearFrame(model,mode,animIndex,frame);
	}
	else
	{
		_luaif_error(L,"usage: animClearFrame(type_string,anim_num,frame_num)");
	}
	return 0;
}

// Set manipulation

extern "C" int luaif_animMove(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==3&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isnumber(L,3))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned index1 = (unsigned)lua_tonumber(L,2);
		unsigned index2 = (unsigned)lua_tonumber(L,3);

		scriptif_animMove(model,mode,index1,index2);
	}
	else
	{
		_luaif_error(L,"usage: animMove(type_string,anim_old_index,anim_new_index)");
	}
	return 0;
}

extern "C" int luaif_animCopy(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int rval = -1;

	if(lua_gettop(L)==3&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isstring(L,3))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned anim = (unsigned)lua_tonumber(L,2);
		const char *name = lua_tostring(L,3);

		rval = scriptif_animCopy(model,mode,anim,name);
	}
	else
	{
		_luaif_error(L,"usage: animCopy(type_string,anim_num,name_string)");
	}

	lua_pushnumber(L,rval);
	return 1;
}

extern "C" int luaif_animSplit(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int rval = -1;

	if(lua_gettop(L)==4&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isstring(L,3)&&lua_isnumber(L,4))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned anim = (unsigned)lua_tonumber(L,2);
		const char *name = lua_tostring(L,3);
		unsigned frame = (unsigned)lua_tonumber(L,4);

		rval = scriptif_animSplit(model,mode,anim,name,frame);
	}
	else
	{
		_luaif_error(L,"usage: animSplit(type_string,anim_num,name_string,frame_num)");
	}

	lua_pushnumber(L,rval);
	return 1;
}

extern "C" int luaif_animJoin(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==3&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isnumber(L,3))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned index1 = (unsigned)lua_tonumber(L,2);
		unsigned index2 = (unsigned)lua_tonumber(L,3);

		scriptif_animJoin(model,mode,index1,index2);
	}
	else
	{
		_luaif_error(L,"usage: animSplit(type_string,anim_num1,anim_num2)");
	}

	return 0;
}

extern "C" int luaif_animConvertToFrame(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	int rval = -1;

	if(lua_gettop(L)==4&&lua_isstring(L,1)&&lua_isnumber(L,2)
		  &&lua_isstring(L,3)&&lua_isnumber(L,4))
	{
		Model::AnimationModeE mode = _stringToAnimationMode(lua_tostring(L,1));
		unsigned anim = (unsigned)lua_tonumber(L,2);
		const char *name = lua_tostring(L,3);
		unsigned frameCount = (unsigned)lua_tonumber(L,4);

		rval = scriptif_animConvertToFrame(model,mode,anim,name,frameCount);
	}
	else
	{
		_luaif_error(L,"usage: animSplit(type_string,anim_num,name_string,frame_count)");
	}

	lua_pushnumber(L,rval);
	return 1;
}

// Skeletal animation

extern "C" int luaif_skelAnimSetKeyframe(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==7 
		  &&lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3)
		  &&lua_isboolean(L,4)
		  &&lua_isnumber(L,5)&&lua_isnumber(L,6)&&lua_isnumber(L,7))
	{
		unsigned animIndex = (unsigned)lua_tonumber(L,1);
		unsigned frame = (unsigned)lua_tonumber(L,2);
		unsigned joint = (unsigned)lua_tonumber(L,3);
		bool	  isRotation = lua_toboolean(L,4);
		double x = (double)lua_tonumber(L,5);
		double y = (double)lua_tonumber(L,6);
		double z = (double)lua_tonumber(L,7);

		scriptif_skelAnimSetKeyframe(model,animIndex,frame,
				joint,isRotation,x,y,z);
	}
	else
	{
		_luaif_error(L,"usage: skelAnimSetKeyframe(anim_num,frame_num,joint_num,rotation_bool,x,y,z)");
	}

	return 0;
}

extern "C" int luaif_skelAnimDeleteKeyframe(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==4 
		  &&lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3)
		  &&lua_isboolean(L,4))
	{
		unsigned animIndex = (unsigned)lua_tonumber(L,1);
		unsigned frame = (unsigned)lua_tonumber(L,2);
		unsigned joint = (unsigned)lua_tonumber(L,3);
		bool	  isRotation = lua_toboolean(L,4);

		scriptif_skelAnimDeleteKeyframe(model,animIndex,frame,joint,isRotation);
	}
	else
	{
		_luaif_error(L,"usage: skelAnimDeleteKeyframe(anim_num,frame_num,joint_num,rotation_bool)");
	}

	return 0;
}

// Frame animation

extern "C" int luaif_frameAnimSetVertex(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	if(lua_gettop(L)==6 
		  &&lua_isnumber(L,1)&&lua_isnumber(L,2)&&lua_isnumber(L,3)
		  &&lua_isnumber(L,4)&&lua_isnumber(L,5)&&lua_isnumber(L,6))
	{
		unsigned animIndex = (unsigned)lua_tonumber(L,1);
		unsigned frame = (unsigned)lua_tonumber(L,2);
		unsigned vertex = (unsigned)lua_tonumber(L,3);
		double x = (double)lua_tonumber(L,4);
		double y = (double)lua_tonumber(L,5);
		double z = (double)lua_tonumber(L,6);

		scriptif_frameAnimSetVertex(model,animIndex,frame,
				vertex,x,y,z);
	}
	else
	{
		_luaif_error(L,"usage: frameAnimSetVertex(anim_num,frame_num,vertex_num,x,y,z)");
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Selection
//-----------------------------------------------------------------------------

extern "C" int luaif_modelSelectAll(lua_State *L)
{
	log_debug("modelSelectAll\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_modelSelectAll(model);
	return 0;
}

extern "C" int luaif_modelSelectAllVertices(lua_State *L)
{
	log_debug("modelSelectAllVertices\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_modelSelectAllVertices(model);
	return 0;
}

extern "C" int luaif_modelSelectAllFaces(lua_State *L)
{
	log_debug("modelSelectAllFaces\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_modelSelectAllFaces(model);
	return 0;
}

extern "C" int luaif_modelSelectAllGroups(lua_State *L)
{
	log_debug("modelSelectAllGroups\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_modelSelectAllGroups(model);
	return 0;
}

extern "C" int luaif_modelSelectAllJoints(lua_State *L)
{
	log_debug("modelSelectAllJoints\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_modelSelectAllJoints(model);
	return 0;
}

extern "C" int luaif_modelSelectVertex(lua_State *L)
{
	log_debug("modelSelectVertex\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	if(lua_gettop(L)==1&&lua_isnumber(L,1))
	{
		int num = (int)lua_tonumber(L,1);
		scriptif_modelSelectVertex(model,num);
	}
	else
	{
		_luaif_error(L,"usage: modelSelectVertex(vertex_num)");
	}
	
	return 0;
}

extern "C" int luaif_modelSelectFace(lua_State *L)
{
	log_debug("modelSelectFace\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	if(lua_gettop(L)==1&&lua_isnumber(L,1))
	{
		int num = (int)lua_tonumber(L,1);
		scriptif_modelSelectFace(model,num);
	}
	else
	{
		_luaif_error(L,"usage: modelSelectFace(face_num)");
	}
	
	return 0;
}

extern "C" int luaif_modelSelectGroup(lua_State *L)
{
	log_debug("modelSelectGroup\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	if(lua_gettop(L)==1&&lua_isnumber(L,1))
	{
		int num = (int)lua_tonumber(L,1);
		scriptif_modelSelectGroup(model,num);
	}
	else
	{
		_luaif_error(L,"usage: modelSelectGroup(group_num)");
	}
	
	return 0;
}

extern "C" int luaif_modelSelectJoint(lua_State *L)
{
	log_debug("modelSelectJoint\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	if(lua_gettop(L)==1&&lua_isnumber(L,1))
	{
		int num = (int)lua_tonumber(L,1);
		scriptif_modelSelectJoint(model,num);
	}
	else
	{
		_luaif_error(L,"usage: modelSelectJoint(joint_num)");
	}
	
	return 0;
}

extern "C" int luaif_modelSelectMesh(lua_State *L)
{
	log_debug("modelSelectMesh\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	if(lua_gettop(L)==1&&lua_isnumber(L,1))
	{
		int num = (int)lua_tonumber(L,1);

		if((unsigned)num<LC->m_createdRectangles.size())
		{
			for(unsigned t = 0; t<LC->m_createdRectangles[num]->m_faces.size(); t++)
			{
				scriptif_modelSelectFace(model,LC->m_createdRectangles[num]->m_faces[t]);
			}
		}
	}
	else
	{
		_luaif_error(L,"usage: modelSelectMesh(mesh_num)");
	}
	
	return 0;
}

extern "C" int luaif_modelUnselectAll(lua_State *L)
{
	log_debug("modelUnselectAll\n");

	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_modelUnselectAll(model);
	return 0;
}

//-----------------------------------------------------------------------------
// Logging
//-----------------------------------------------------------------------------

extern "C" int luaif_logDebug(lua_State *L)
{
	if(lua_gettop(L)==1&&lua_isstring(L,1))
	{
		scriptif_logDebug(lua_tostring(L,1));
	}
	else
	{
		_luaif_error(L,"usage: logDebug(string)");
	}
	return 0;
}

extern "C" int luaif_logWarning(lua_State *L)
{
	if(lua_gettop(L)==1&&lua_isstring(L,1))
	{
		scriptif_logWarning(lua_tostring(L,1));
	}
	else
	{
		_luaif_error(L,"usage: logWarning(string)");
	}
	return 0;
}

extern "C" int luaif_logError(lua_State *L)
{
	if(lua_gettop(L)==1&&lua_isstring(L,1))
	{
		scriptif_logError(lua_tostring(L,1));
	}
	else
	{
		_luaif_error(L,"usage: logError(string)");
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Debugging
//-----------------------------------------------------------------------------

extern "C" void luaif_lineHook(lua_State *L,lua_Debug *ar)
{
	lua_getinfo(L,"S",ar);
	log_debug("executing line %d\n",ar->currentline);
	if(ar->source)
	{
		printf("%s\n",ar->source);
	}
}

//-----------------------------------------------------------------------------
// Testing
//-----------------------------------------------------------------------------

#ifdef _LUASCRIPT_TEST

extern "C" int luaif_undo(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	model->undo();
	return 0;
}

extern "C" int luaif_undoCurrent(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	model->undoCurrent();
	return 0;
}

extern "C" int luaif_redo(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	model->redo();
	return 0;
}

extern "C" int luaif_operationComplete(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	const char *name = "Unnamed Operation";
	if(lua_gettop(L)==1)
	{
		name = lua_tostring(L,1);
	}
	model->operationComplete(name);
	return 0;
}

extern "C" int luaif_modelLoad(lua_State *L)
{
	log_debug("modelLoad\n");
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	
	if(lua_gettop(L)==1&&lua_isstring(L,1))
	{
		const char *filename = lua_tostring(L,1);

		Model::ModelErrorE err = Model::ERROR_NONE;
		Model *newModel = new Model;

		if(Model::ERROR_NONE==(err = FilterManager::getInstance()->readFile(newModel,filename)))
		{
			LC->m_list.push_back(newModel);
			LC->m_currentModel = newModel;
		}
		else
		{
			char str[64];
			snprintf(str,sizeof(str),"Error loading file '%s'",filename);
			_luaif_error(L,str);
			delete newModel;
			newModel = nullptr;
		}

		lua_pushlightuserdata(L,(void *)newModel);
	}
	else
	{
		_luaif_error(L,"usage: modelLoad(file_string)");
		lua_pushnil(L);
	}
	return 1;
}

extern "C" int luaif_modelCompareToCurrent(lua_State *L)
{
	log_debug("model compare to current\n");
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;

	bool match = false;
	if(lua_gettop(L)>=1&&lua_islightuserdata(L,1))
	{
		Model *otherModel = (Model *)lua_topointer(L,1);

		if(model->equivalent(otherModel))
		{
			match = true;
		}
	}
	else
	{
		_luaif_error(L,"usage: modelCompareToCurrent([compare_string] ... )");
	}


	lua_pushboolean(L,(int)match);
	return 1;
}

extern "C" int luaif_modelClose(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));

	bool found = false;

	if(lua_gettop(L)==1&&lua_islightuserdata(L,1))
	{
		Model *model = (Model *)lua_topointer(L,1);

		vector<Model *>::iterator it = LC->m_list.begin();
		while(it!=LC->m_list.end())
		{
			if((*it)==model)
			{
				found = true;
				if(model==LC->m_currentModel)
				{
					LC->m_currentModel = LC->m_scriptModel;
				}

				LC->m_list.erase(it);
				delete model;

				break;
			}
			it++;
		}
	}
	else
	{
		_luaif_error(L,"usage: modelClose(model)");
	}

	lua_pushboolean(L,(int)found);
	return 1;
}

extern "C" int luaif_selectedDelete(lua_State *L)
{
	LuaContext *LC = (LuaContext *)lua_topointer(L,lua_upvalueindex(1));
	Model *model = LC->m_currentModel;
	scriptif_selectedDelete(model);
	return 0;
}

#endif // _LUASCRIPT_TEST

//-----------------------------------------------------------------------------
// Registration function
//-----------------------------------------------------------------------------

void luaif_registerfunctions(LuaScript *lua,LuaContext *context)
{
	lua->registerClosure(context,"die",&luaif_die);
	lua->registerClosure(context,"getModel",&luaif_getModel);

	// General
	lua->registerClosure(context,"modelGetName",&luaif_modelGetName);

	// Conversion
	lua->registerClosure(context,"modelSaveAs",&luaif_modelSaveAs);

	// Primitive Creation
	lua->registerClosure(context,"modelCreateMeshRectangle",&luaif_modelCreateMeshRectangle);
	lua->registerClosure(context,"modelCreateBoneJoint",&luaif_modelCreateBoneJoint);

	// Primitive Manipulation
	lua->registerClosure(context,"vertexSetCoords",&luaif_vertexSetCoords);
	lua->registerClosure(context,"modelAddTexture",&luaif_modelAddTexture);
	lua->registerClosure(context,"groupSetTexture",&luaif_groupSetTexture);
	lua->registerClosure(context,"faceSetTextureCoords",&luaif_faceSetTextureCoords);

	lua->registerClosure(context,"selectedRotate",&luaif_selectedRotate);
	lua->registerClosure(context,"selectedTranslate",&luaif_selectedTranslate);
	lua->registerClosure(context,"selectedScale",&luaif_selectedScale);
	lua->registerClosure(context,"selectedApplyMatrix",&luaif_selectedApplyMatrix);

	lua->registerClosure(context,"selectedWeldVertices",&luaif_selectedWeldVertices);

	lua->registerClosure(context,"selectedInvertNormals",&luaif_selectedInvertNormals);
	lua->registerClosure(context,"selectedGroupFaces",&luaif_selectedGroupFaces);
	lua->registerClosure(context,"selectedAddToGroup",&luaif_selectedAddToGroup);

	lua->registerClosure(context,"modelGetVertexCount",&luaif_modelGetVertexCount);
	lua->registerClosure(context,"modelGetFaceCount",&luaif_modelGetFaceCount);
	lua->registerClosure(context,"modelGetGroupCount",&luaif_modelGetGroupCount);
	lua->registerClosure(context,"modelGetJointCount",&luaif_modelGetJointCount);
	lua->registerClosure(context,"modelGetTextureCount",&luaif_modelGetTextureCount);

	lua->registerClosure(context,"modelGetGroupByName",&luaif_modelGetGroupByName);
	lua->registerClosure(context,"groupGetName",&luaif_groupGetName);
	lua->registerClosure(context,"modelGetTextureByName",&luaif_modelGetTextureByName);
	lua->registerClosure(context,"textureGetName",&luaif_textureGetName);
	lua->registerClosure(context,"textureGetFilename",&luaif_textureGetFilename);

	// Animation
	lua->registerClosure(context,"setAnimByName",&luaif_setAnimByName);
	lua->registerClosure(context,"setAnimByIndex",&luaif_setAnimByIndex);
	lua->registerClosure(context,"animSetFrame",&luaif_animSetFrame);
	lua->registerClosure(context,"setAnimTime",&luaif_setAnimTime);

	lua->registerClosure(context,"modelCreateAnimation",&luaif_modelCreateAnimation);

	lua->registerClosure(context,"animGetCount",&luaif_animGetCount);
	lua->registerClosure(context,"animGetName",&luaif_animGetName);
	lua->registerClosure(context,"animGetFrameCount",&luaif_animGetFrameCount);

	lua->registerClosure(context,"animSetName",&luaif_animSetName);
	lua->registerClosure(context,"animSetFrameCount",&luaif_animSetFrameCount);
	lua->registerClosure(context,"animSetFPS",&luaif_animSetFPS);
	lua->registerClosure(context,"animClearFrame",&luaif_animClearFrame);
	lua->registerClosure(context,"animCopyFrame",&luaif_animCopyFrame);

	lua->registerClosure(context,"animMove",&luaif_animMove);
	lua->registerClosure(context,"animCopy",&luaif_animCopy);
	lua->registerClosure(context,"animSplit",&luaif_animSplit);
	lua->registerClosure(context,"animJoin",&luaif_animJoin);
	lua->registerClosure(context,"animConvertToFrame",&luaif_animConvertToFrame);

	lua->registerClosure(context,"skelAnimSetKeyframe",&luaif_skelAnimSetKeyframe);
	lua->registerClosure(context,"skelAnimDeleteKeyframe",&luaif_skelAnimDeleteKeyframe);
	lua->registerClosure(context,"frameAnimSetVertex",&luaif_frameAnimSetVertex);

	// Selection
	lua->registerClosure(context,"modelSelectAll",&luaif_modelSelectAll);
	lua->registerClosure(context,"modelSelectAllVertices",&luaif_modelSelectAllVertices);
	lua->registerClosure(context,"modelSelectAllFaces",&luaif_modelSelectAllFaces);
	lua->registerClosure(context,"modelSelectAllGroups",&luaif_modelSelectAllGroups);
	lua->registerClosure(context,"modelSelectAllJoints",&luaif_modelSelectAllJoints);

	lua->registerClosure(context,"modelSelectVertex",&luaif_modelSelectVertex);
	lua->registerClosure(context,"modelSelectFace",&luaif_modelSelectFace);
	lua->registerClosure(context,"modelSelectGroup",&luaif_modelSelectGroup);
	lua->registerClosure(context,"modelSelectJoint",&luaif_modelSelectJoint);
	lua->registerClosure(context,"modelSelectMesh",&luaif_modelSelectMesh);

	lua->registerClosure(context,"modelUnselectAll",&luaif_modelUnselectAll);

	// Logging
	lua->registerClosure(context,"logDebug",&luaif_logDebug);
	lua->registerClosure(context,"logWarning",&luaif_logWarning);
	lua->registerClosure(context,"logError",&luaif_logError);

#ifdef _LUASCRIPT_TEST
	lua->registerClosure(context,"undo",&luaif_undo);
	lua->registerClosure(context,"undoCurrent",&luaif_undoCurrent);
	lua->registerClosure(context,"redo",&luaif_redo);
	lua->registerClosure(context,"operationComplete",&luaif_operationComplete);

	lua->registerClosure(context,"modelLoad",&luaif_modelLoad);
	lua->registerClosure(context,"modelClose",&luaif_modelClose);
	lua->registerClosure(context,"modelCompareToCurrent",&luaif_modelCompareToCurrent);

	lua->registerClosure(context,"selectedDelete",&luaif_selectedDelete);
#endif // _LUASCRIPT_TEST

	// Debugging hooks
	//lua->registerHook(luaif_lineHook,LUA_MASKLINE);
}

#endif // HAVE_LUALIB
