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

#include "texture.h"
#include "translate.h"

int Texture::s_allocated = 0;

Texture::Texture()
	:
	  m_isBad(false),
	  m_height(0),
	  m_width(0),
	  m_format(FORMAT_RGBA)
{
	s_allocated++;
}

Texture::~Texture()
{
	s_allocated--;
}

bool Texture::compare(Texture *t1,Texture *t2,CompareResultT *res, unsigned fuzzyValue)
{
	return t1->compare(t2,res,fuzzyValue);
}

bool Texture::compare(Texture *tex,CompareResultT *res, unsigned fuzzyValue)
{
	Texture *t1 = this;
	Texture *t2 = tex;

	if(t1->m_width!=t2->m_width
		  ||t1->m_height!=t2->m_height
		  ||t1->m_format!=t2->m_format 
		  ||t1->m_isBad
		  ||t2->m_isBad)
	{
		res->comparable = false;
		return false;
	}

	res->comparable = true;
	bool hasAlpha = (t1->m_format==Texture::FORMAT_RGBA)? true : false;
	unsigned count = t1->m_width *t1->m_height;

	res->pixelCount = count;
	res->matchCount = 0;
	res->fuzzyCount = 0;

	unsigned bytespp = hasAlpha ? 4 : 3;
	unsigned off = 0;
	unsigned fuzzy = 0;

	for(unsigned p = 0; p<count; p++)
	{
		off = p *bytespp;
		fuzzy = 0;

		fuzzy += abs(t1->m_data[off+0]-t2->m_data[off+0]);
		fuzzy += abs(t1->m_data[off+1]-t2->m_data[off+1]);
		fuzzy += abs(t1->m_data[off+2]-t2->m_data[off+2]);
		if(hasAlpha)
		{
			fuzzy += abs(t1->m_data[off+3]-t2->m_data[off+3]);
		}

		if(hasAlpha 
			  &&t1->m_data[off+3]==0
			  &&t2->m_data[off+3]==0)
		{
			fuzzy = 0;
		}

		if(fuzzy==0)
			res->matchCount++;
		if(fuzzy<=fuzzyValue)
			res->fuzzyCount++;
	}

	return(res->pixelCount==res->matchCount);
}


const char *Texture::errorToString(Texture::ErrorE e)
{
	switch (e)
	{
		case ERROR_NONE:
			return TRANSLATE_NOOP("LowLevel","Success");
		case ERROR_NO_FILE:
			return TRANSLATE_NOOP("LowLevel","File does not exist");
		case ERROR_NO_ACCESS:
			return TRANSLATE_NOOP("LowLevel","Permission denied");
		case ERROR_FILE_OPEN:
			return TRANSLATE_NOOP("LowLevel","Could not open file");
		case ERROR_FILE_READ:
			return TRANSLATE_NOOP("LowLevel","Could not read from file");
		case ERROR_FILE_WRITE:
			return TRANSLATE_NOOP("LowLevel","Could not write file");
		case ERROR_BAD_MAGIC:
			return TRANSLATE_NOOP("LowLevel","File is the wrong type or corrupted");
		case ERROR_UNSUPPORTED_VERSION:
			return TRANSLATE_NOOP("LowLevel","Unsupported version");
		case ERROR_BAD_DATA:
			return TRANSLATE_NOOP("LowLevel","File contains invalid data");
		case ERROR_UNEXPECTED_EOF:
			return TRANSLATE_NOOP("LowLevel","Unexpected end of file");
		case ERROR_UNSUPPORTED_OPERATION:
			return TRANSLATE_NOOP("LowLevel","This operation is not supported");
		case ERROR_BAD_ARGUMENT:
			return TRANSLATE_NOOP("LowLevel","Invalid argument (internal error,probably null pointer argument)");
		case ERROR_UNKNOWN:
			return TRANSLATE_NOOP("LowLevel","Unknown error" );
		default:
			break;
	}
	return TRANSLATE_NOOP("LowLevel","Invalid error code");
}

void Texture::removeOpaqueAlphaChannel() //??? //JUSTIFY ME
{
		//DEBUGGING
		//Not only is this procedure of suspect utility, it is slowing
		//down loading to a crawl. Why?
		//return;

	if(m_data.empty()||m_format!=FORMAT_RGBA||m_width<1||m_height<1)
	{
		return;
	}

	//Accessing std::vector directly is super slow... in debug mode. 
	uint8_t *data = m_data.data();

	uint32_t srcImageSize = 4 *m_width *m_height;
	for(uint32_t src = 0; src<srcImageSize; src += 4)
	{
		if(data[src+3]<255)
		{
			// Alpha channel is used.
			return;
		}
	}

	m_format = FORMAT_RGB;
	for(uint32_t src = 0,dst = 0; src<srcImageSize; src += 4,dst += 3)
	{
		data[dst+0] = data[src+0];
		data[dst+1] = data[src+1];
		data[dst+2] = data[src+2];
	}

	m_data.resize(3*m_width*m_height); //NEW
}

//errorObj.cc
const char *textureErrStr(Texture::ErrorE err)
{
	switch(err)
	{
	case Texture::ERROR_NONE:
		return transll("Success");
	case Texture::ERROR_NO_FILE:
		return transll("File does not exist");
	case Texture::ERROR_NO_ACCESS:
		return transll("Permission denied");
	case Texture::ERROR_FILE_OPEN:
		return transll("Could not open file");
	case Texture::ERROR_FILE_READ:
		return transll("Could not read from file");
	case Texture::ERROR_FILE_WRITE:
		return transll("Could not write file");
	case Texture::ERROR_BAD_MAGIC:
		return transll("File is the wrong type or corrupted");
	case Texture::ERROR_UNSUPPORTED_VERSION:
		return transll("Unsupported version");
	case Texture::ERROR_BAD_DATA:
		return transll("File contains invalid data");
	case Texture::ERROR_UNEXPECTED_EOF:
		return transll("Unexpected end of file");
	case Texture::ERROR_UNSUPPORTED_OPERATION:
		return transll("This operation is not supported");
	case Texture::ERROR_BAD_ARGUMENT:
		return transll("Invalid argument (internal error)");
	case Texture::ERROR_UNKNOWN:
		return transll("Unknown error");
	default:
		break;
	}
	return "FIXME: Untranslated texture error";
}