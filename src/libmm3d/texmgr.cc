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
#include "log.h"
#include "msg.h"
#include "texscale.h"
#include "misc.h"
#include "translate.h"
#include "filedatadest.h" //NEW
#include "filedatasource.h" //NEW

#include "modelstatus.h"

typedef const char *utf8; //REMOVE ME

static bool _doWarning = false;

void texture_manager_do_warning(Model *model) 
{
	if(!_doWarning) return; _doWarning = false;

	const char *msg =  
	TRANSLATE("LowLevel",
	"The model has a texture that's width or height is not a power of two (2,4,8,..,64,128,256,..).");

	if(!model) //ANNOYING
	{
		msg_warning(msg); //Produces a message box in editor.
	}
	else model_status(model,StatusNormal,STATUSTIME_LONG,msg); //NEW
}

Texture::ErrorE TextureFilter::errnoToTextureError(int err, Texture::ErrorE defaultError)
{
	switch(err)
	{
	case 0: return Texture::ERROR_NONE;
	case EINVAL: return Texture::ERROR_BAD_ARGUMENT;
	case EACCES:
	case EPERM: return Texture::ERROR_NO_ACCESS;
	case ENOENT:
	case EBADF: return Texture::ERROR_NO_FILE;
	case EISDIR: return Texture::ERROR_BAD_DATA;
	}
	return defaultError;
}

TextureManager *TextureManager::s_instance = nullptr; //???

TextureManager::~TextureManager()
{
	log_debug("TextureManager releasing %d textures and %d filters\n",
	m_textures.size(),m_filters.size());

	for(auto*ea:m_textures) delete ea;

	for(auto ea:m_filters) ea->release();
}

TextureManager *TextureManager::getInstance() //???
{
	if(!s_instance) s_instance = new TextureManager;

	return s_instance;
}
void TextureManager::release() //???
{
	delete s_instance; s_instance = nullptr;
}

void TextureManager::registerTextureFilter(TextureFilter *filter)
{
	_read.clear(); _write.clear(); 
	
	m_filters.push_back(filter); assert(filter);
}

Texture *TextureManager::getTexture(const char *filename, bool noCache, bool warning)
{
	if(!filename) return nullptr; //???

	if(!noCache)	
	for(auto*ea:m_textures)
	if(!strcmp(filename,ea->m_filename.c_str()))
	{
		log_debug("cached image %s\n",filename);
		return ea;
	}

	if(!*filename)
	{
		return getBlankTexture("blank");
	}

	FileDataSource lvalue(filename);
	Texture *ret = getTexture(filename,lvalue,warning);

	if(noCache&&ret) m_textures.pop_back(); return ret; //HACK
}
Texture *TextureManager::getTexture(const char *name_and_format, DataSource &src, bool warning)
{
	if(!name_and_format) return nullptr;
	Texture *newTexture = new Texture();

	void *is_file = dynamic_cast<FileDataSource*>(&src); //HACK
	if(is_file) newTexture->m_filename = name_and_format;

	const char *name = strrchr(name_and_format,'/');
	newTexture->m_name = name?name+1:name_and_format;
	newTexture->m_name.erase(newTexture->m_name.rfind('.'));
	newTexture->m_isBad = false;

	name = strchr(name_and_format,'.');
	name = name?name+1:name_and_format;
	newTexture->m_origFormat = name; //NEW

	for(auto ea:m_filters) if(ea->canRead(name))	
	if(Texture::ErrorE error=ea->readData(*newTexture,src,name))
	{
		m_lastError = error;
		log_error("filter failed to read texture: %d\n",error);
	}
	else
	{
		m_lastError = Texture::ERROR_NONE;
		log_debug("read from image source %s\n",name_and_format);
		newTexture->removeOpaqueAlphaChannel(); //???
		if(is_file)
		{
			time_t mtime;
			if(file_modifiedtime(name_and_format,&mtime))
			newTexture->m_loadTime = mtime;
		}
		newTexture->m_origWidth = newTexture->m_width;
		newTexture->m_origHeight = newTexture->m_height;	
		if(texture_scale_need_scale(newTexture->m_width,newTexture->m_height))
		{
			if(warning) _doWarning = true;

			/*2019: Disabling this NPOT (non-power-of-two) hack.
			//I'm uncomfortable upscaling. End-users can adjust.
			//NOTE: Won't work as-is with std::vector.
			uint8_t *oldData = newTexture->m_data;
			newTexture->m_data = texture_scale_auto
			(newTexture->m_data,newTexture->m_format,
			newTexture->m_width,newTexture->m_height);
			delete[] oldData;
			*/
		}

		//if(!noCache)
		{
			m_textures.push_back(newTexture);
		}

		return newTexture;
	}		

	delete newTexture; return nullptr;
}

bool TextureManager::reloadTextures()
{
	bool anyTextureChanged = false;
	
	//TODO: Make getTexture not push/pop m_textures.
	//for(auto*ea:m_textures)
	for(size_t i=m_textures.size();i-->0;)
	{
		Texture *ea = m_textures[i];

		time_t mtime;		
		if(!file_modifiedtime(ea->m_filename.c_str(),&mtime)||mtime<=ea->m_loadTime)
		continue;
		
		Texture *refreshedTexture = getTexture(ea->m_filename.c_str(),true);
		if(refreshedTexture)
		{
			log_debug("reloaded texture %s\n",ea->m_filename.c_str());

			refreshedTexture->removeOpaqueAlphaChannel(); //???
							
			ea->m_width = refreshedTexture->m_width;
			ea->m_height = refreshedTexture->m_height;
			ea->m_origWidth = refreshedTexture->m_origWidth;
			ea->m_origHeight = refreshedTexture->m_origHeight;
			ea->m_format = refreshedTexture->m_format;
			ea->m_data.swap(refreshedTexture->m_data);
			ea->m_loadTime = mtime;

			delete refreshedTexture;

			anyTextureChanged = true;
		}
		else msg_warning("%s %s",transll
		(TRANSLATE_NOOP("LowLevel","Could not load")),ea->m_filename.c_str());
	}
	
	return anyTextureChanged;
}

Texture *TextureManager::getBlankTexture(const char *name)
{
	for(auto*ea:m_textures)
	{
		//2019: Assuming this is erroneous.
		//if(!strcmp(name,""))
		if(ea->m_name.empty())
		{
			log_debug("cached image (blank)\n");
			return ea;
		}
	}

	Texture *tex = new Texture();
	tex->m_isBad = false;

	//tex->m_filename = strdup("");
	if(name) tex->m_name = name; //strdup(name);

	enum{ size=2 };
	tex->m_height = tex->m_origHeight = size;
	tex->m_width = tex->m_origWidth = size;

	//NOTE: Using malloc on m_data disagreed with ~Texture.
	//tex->m_data = (uint8_t*)malloc(sizeof(uint8_t)*4*4);
	//for(unsigned t=0;t<4*4;t++) tex->m_data[t] = 0xFF;
	tex->m_data.resize(4*size*size,0xFF);

	tex->removeOpaqueAlphaChannel(); //??? //??? //??? //??? //???

	m_textures.push_back(tex); return tex;
}

Texture *TextureManager::getDefaultTexture(const char *filename)
{
	Texture *tex = new Texture();
	tex->m_isBad = true;
	
	//string str = string("bad: ")+filename;
	//tex->m_filename = strdup(str.c_str()); //???
	tex->m_name.assign("bad: ").append(filename); 

	enum{ size=8 };
	tex->m_height = tex->m_origHeight = size;
	tex->m_width = tex->m_origWidth = size;

	//NOTE: Using malloc on m_data disagreed with ~Texture.
	//tex->m_data = (uint8_t*)malloc(sizeof(uint8_t)*4*64);
	tex->m_data.resize(4*size*size);
	uint8_t *data = tex->m_data.data();

	char pattern[] = "					  x	x	 x x		x		x x	 x	x			  ";
	for(int y=tex->m_height;y-->0;)	
	for(int x=tex->m_width;x-->0;) switch(pattern[y*8+x])
	{
	case ' ':
		data[(y*8+x)*4+0] = 0xFF;
		data[(y*8+x)*4+1] = 0x00;
		data[(y*8+x)*4+2] = 0x00;
		data[(y*8+x)*4+3] = 0xFF; break;
	case 'x':
		data[(y*8+x)*4+0] = 0x00;
		data[(y*8+x)*4+1] = 0x00;
		data[(y*8+x)*4+2] = 0x00;
		data[(y*8+x)*4+3] = 0xFF; break;
	}

	tex->removeOpaqueAlphaChannel(); //??? //??? //??? //??? //???

	m_textures.push_back(tex); return tex;
}

Texture::ErrorE TextureManager::write(Texture *tex, utf8 filename)
{
	if(!filename||!*filename||!tex) return Texture::ERROR_BAD_ARGUMENT;

	FileDataDest lvalue(filename); return write(tex,lvalue,filename);
}
Texture::ErrorE TextureManager::write(Texture *tex, DataDest &dest, utf8 format)
{
	if(!tex) return Texture::ERROR_BAD_ARGUMENT;

	if(format&&*format)
	{
		utf8 name = strrchr(format,'.');
		format = name?name+1:format;
	}
	else format = tex->m_origFormat.c_str();

	for(auto ea:m_filters) if(ea->canWrite(format))
	{
		if(Texture::ErrorE error=ea->writeData(*tex,dest,format))
		{
			m_lastError = error;
			log_error("filter could not write texture: %d\n",error);
		}
		else
		{
			m_lastError = Texture::ERROR_NONE;
			return Texture::ERROR_NONE;				
		}
	}

	return Texture::ERROR_UNSUPPORTED_OPERATION;
}

static utf8 texmgr_all_types //DUPLICATES filtermgr.cc
(std::vector<TextureFilter*> &f, std::string &g, utf8(TextureFilter::*mf)())
{
	if(!g.empty()) return g.c_str();

	//log_debug("have %d filters\n",f.size()); //???

	for(auto ea:f)
	{
		utf8 t = (ea->*mf)();

		if(!t||!*t){ assert(t); continue; }

		size_t dup = g.size();

		//NOT REMOVING NONEXISTANT DUPLICATES.
		g.append(t); g.push_back(' ');
	}

	if(!g.empty()) g.pop_back();

	return g.c_str();
}
utf8 TextureManager::getAllReadTypes()
{
	return texmgr_all_types(m_filters,_read,&TextureFilter::getReadTypes);
}
utf8 TextureManager::getAllWriteTypes()
{
	return texmgr_all_types(m_filters,_write,&TextureFilter::getWriteTypes);
}

extern bool texmgr_can_read_or_write(utf8 ext, utf8 cmp)
{
	if(!cmp||!ext) //ModelFilter
	{
		return !cmp&&ext&&*ext; //ModelFilter
	}

	if(utf8 dot=strchr(cmp,'.')) cmp = dot+1;

	for(utf8 p=cmp;;p++,ext++)
	{
		if(int P=toupper(*p))
		{
			if(P==toupper(*ext)) continue;
		}
		else if(!*ext||*ext==' ') return true;

		while(*ext&&*ext!=' ') ext++;

		if(*ext) p = cmp-1; else return false;
	}
}
