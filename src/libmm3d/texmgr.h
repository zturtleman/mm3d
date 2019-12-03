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


#ifndef __TEXMGR_H
#define __TEXMGR_H

#include "texture.h"

extern void texture_manager_do_warning(class Model*m=nullptr);

extern bool texmgr_can_read_or_write(const char*,const char*); //utf8

//------------------------------------------------------------------
// About the TextureFilter class
//------------------------------------------------------------------
//
// The TextureFilter class is a base class for implementing filters to
// import and export texture images to various formats.  If you 
// implement a TextureFilter,you need to register the filter with the 
// TextureManager.  You only need one instance of your filter.
//
class TextureFilter
{
public:

	//REMOVE ME?
	//In case "libmm3d" code hasn't defined utf8.
	typedef const char *utf8;

	virtual ~TextureFilter(){};

	// Default error should be Texture::ERROR_FILE_OPEN,ERROR_FILE_READ,or
	// ERROR_FILE_WRITE depending on the context of operations being performed.
	static Texture::ErrorE errnoToTextureError(int err,Texture::ErrorE defaultError);

	virtual utf8 getReadTypes() = 0;

	bool canRead(utf8 filename_or_extension)
	{
		return texmgr_can_read_or_write
		(getReadTypes(),filename_or_extension);
	}

	// It is a good idea to override this if you implement
	// a filter as a plugin.
	virtual void release(){ delete this; };

	// readFile reads the contents of 'filename' and modifies 'texture' to
	// match the description in 'filename'.  This is the import function.
	//
	// The texture argument will be an empty texture instance.  If the file
	// cannot be loaded,return the appropriate TextureError error code.
	// If the load succeeds,return TextureError::ERROR_NONE.
	virtual Texture::ErrorE readData(Texture &texture, DataSource &source, utf8 format)= 0;


		//UNUSED//UNUSED//UNUSED//UNUSED//
		//REMOVE//REMOVE//REMOVE//REMOVE//

	virtual utf8 getWriteTypes(){ return ""; }

	bool canWrite(utf8 filename_or_extension)
	{
		return texmgr_can_read_or_write
		(getWriteTypes(),filename_or_extension);
	}

	//UNUSED
	// writeFile writes the contents of 'texture' to 'filename'.
	// This is the export function.
	//
	// This function is currently not called and doesn't really need to be
	// implemented at this time.  It may be used in the future for more
	// advanced texturing modes.
	virtual Texture::ErrorE writeData(Texture &texture, DataDest &dest, utf8 format)
	{
		return Texture::ERROR_UNSUPPORTED_OPERATION; 
	}
};

//------------------------------------------------------------------
// About the TextureManager class
//------------------------------------------------------------------
//
// The TextureManager class is a singleton that keeps a list of filters
// for importing and exporting texture images to various file formats.
// At this time only read support is used.
//
// To create a new filter you must derive from the TextureFilter class
// and register your new filter with the TextureManager instance.  If
// your filter is called MyFilter,you would register your filter with
// the following function:
//
//	 MyFilter *mf = new MyFilter();
//	 TextureManager::getInstance()->registerFilter(mf);
//
// This would usually be done in the plugin_init function of a plugin.
// You only need one instance of your filter.
//
class TextureManager
{
public:

	//In case "libmm3d" code hasn't defined utf8.
	typedef const char *utf8;

	void registerTextureFilter(TextureFilter *filter);

	static TextureManager *getInstance(); //???
	static void release(); //???

	// reloads all textures that have changed on disk since last load time.
	// returns true if any textures have been reloaded.
	bool reloadTextures();

	bool canRead(utf8 filename_or_extension)
	{
		return texmgr_can_read_or_write
		(getAllReadTypes(),filename_or_extension);
	}
	utf8 getAllReadTypes();
	
	//UNUSED
	//NOTE: This is changed to not set the texture's filename unless 
	//nullptr!=dynamic_cast<FileDataSource*>(&data).	
	Texture *getTexture(const char *name_and_format, DataSource &ds, bool warn=true);
	Texture *getTexture(const char *filename, bool noCache=false, bool warning=true);
	Texture *getBlankTexture(const char *filename);
	Texture *getDefaultTexture(const char *filename);
	Texture::ErrorE getLastError(){ return m_lastError; };
	
	Texture::ErrorE write(Texture *tex, utf8 filename);
	Texture::ErrorE write(Texture *tex, DataDest &data, utf8 format);
				
	//UNUSED
	bool canWrite(utf8 filename_or_extension)
	{
		return texmgr_can_read_or_write
		(getAllWriteTypes(),filename_or_extension);
	}
	utf8 getAllWriteTypes();

protected:
		
	TextureManager():m_lastError(){} //NEW //m_defaultTexture()
	~TextureManager();

	static TextureManager *s_instance; //???

	std::vector<TextureFilter*> m_filters;		
	std::vector<Texture*> m_textures;
	Texture::ErrorE	m_lastError;

	//Texture *m_defaultTexture; //UNUSED?

	std::string _read,_write; //NEW
};

#endif // __TEXMGR_H
