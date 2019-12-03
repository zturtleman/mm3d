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

#include "endianconfig.h"
#include "mm3dconfig.h"
#include "log.h"
#include "texmgr.h"
#include "filedatasource.h"

struct TGAHeaderT
{
	uint8_t Header[12];
};

struct TGA
{
	uint16_t width;
	uint16_t height;
	uint8_t bpp;
	uint8_t reserved;
};

static TGAHeaderT tgaheader;
static TGA tga;

static uint8_t uTGAcompare[12] = {0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
static uint8_t cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};	// Compressed TGA Header

static bool SourceHasError(DataSource &src)
{
	if(src.unexpectedEof()) return true;
	if(src.getErrno()!=0) return true; return false;
}

static Texture::ErrorE SourceGetError(DataSource &src)
{
	if(src.unexpectedEof()) return Texture::ERROR_UNEXPECTED_EOF;
	if(src.getErrno()!=0) return Texture::ERROR_FILE_READ;
	return Texture::ERROR_NONE;
}

static Texture::ErrorE LoadUncompressedTGA(Texture &texture, DataSource &src)
{
	log_debug("loading uncompressed TGA\n");

	src.read(tga.width);
	src.read(tga.height);
	src.read(tga.bpp);
	src.read(tga.reserved);

	if(SourceHasError(src))
		return SourceGetError(src);

	texture.m_width  = tga.width;
	texture.m_height = tga.height;

	uint32_t width  = texture.m_width;
	uint32_t height = texture.m_height;
	uint8_t bpp	  = tga.bpp;

	if((width<=0)||(height<=0)||((bpp!=24)&&(bpp !=32)))
	{
		fprintf(stderr,"Invalid texture information");
		return Texture::ERROR_BAD_DATA;
	}

	log_debug("tga size: %d x %d,%d bbp\n",width,height,bpp);

	bool hasAlpha = (bpp==32);
	log_debug("Alpha channel: %s\n",hasAlpha ? "present" : "not present");

	uint8_t bytespp = (bpp/8);
	uint32_t imageSize  = (bytespp *width *height);
	texture.m_data.resize(imageSize);
	uint8_t *data = texture.m_data.data();
	texture.m_format = hasAlpha ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB;

	log_debug("image size = %d\n",imageSize);

	if(!src.readBytes(data,imageSize))
	{
		return SourceGetError(src);
	}

	for(uint32_t n = 0; n<imageSize; n += bytespp)
	{
		uint8_t temp = data[n+0];
		data[n+0] = data[n+2];
		data[n+2] = temp;
	}

	return Texture::ERROR_NONE;
}

static Texture::ErrorE LoadCompressedTGA(Texture &texture, DataSource &src)
{ 
	log_debug("loading compressed TGA\n");

	src.read(tga.width);
	src.read(tga.height);
	src.read(tga.bpp);
	src.read(tga.reserved);

	if(SourceHasError(src))
		return SourceGetError(src);

	texture.m_width  = tga.width;
	texture.m_height = tga.height;

	uint32_t width  = texture.m_width;
	uint32_t height = texture.m_height;
	uint8_t bpp	  = tga.bpp;

	if((width<=0)||(height<=0)||((bpp!=24)&&(bpp !=32)))
	{
		fprintf(stderr,"Invalid texture information");
		return Texture::ERROR_BAD_DATA;
	}

	bool hasAlpha = (bpp==32);
	log_debug("Alpha channel: %s\n",hasAlpha ? "present" : "not present");

	log_debug("tga size: %d x %d,%d bbp\n",width,height,bpp);

	uint8_t bytespp = (bpp/8);
	uint32_t imageSize  = (bytespp *width *height);
	texture.m_data.resize(imageSize);
	uint8_t *data = texture.m_data.data();

	uint32_t pixelcount = height *width;
	uint32_t currentpixel = 0;
	uint32_t currentbyte = 0;
	uint8_t colorbuffer[4];

	texture.m_format = hasAlpha ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB;

	do
	{
		uint8_t chunkheader = 0;
		if(!src.read(chunkheader))
			return SourceGetError(src);

		if(chunkheader<128)
		{
			chunkheader++;
			for(short counter = 0; counter<chunkheader&&currentpixel<pixelcount; counter++)
			{
				if(!src.readBytes(colorbuffer,bytespp))
					return SourceGetError(src);

				data[currentbyte+0] = colorbuffer[2];		  
				data[currentbyte+1] = colorbuffer[1];
				data[currentbyte+2] = colorbuffer[0];
				if(hasAlpha)
				{
					data[currentbyte+3] = colorbuffer[3];
				}

				currentbyte += bytespp;
				currentpixel++;
			}
		}
		else
		{
			chunkheader -= 127;
			if(!src.readBytes(colorbuffer,bytespp))
				return SourceGetError(src);

			for(short counter = 0; counter<chunkheader&&currentpixel<pixelcount; counter++)
			{
				data[currentbyte+0] = colorbuffer[2];		  
				data[currentbyte+1] = colorbuffer[1];
				data[currentbyte+2] = colorbuffer[0];
				if(hasAlpha)
				{
					data[currentbyte+3] = colorbuffer[3];
				}

				currentbyte += bytespp;
				currentpixel++;
			}
		}

	}while(currentpixel<pixelcount);

	log_debug("pixel count = %d,current pixel = %d\n",pixelcount,currentpixel);
	log_debug("image size = %d\n",imageSize);

	return Texture::ERROR_NONE;
}

struct TgaTextureFilter : TextureFilter
{
	virtual const char *getReadTypes(){ return "TGA"; }

	virtual Texture::ErrorE readData(Texture &texture, DataSource &src, const char*)
	{
		if(src.errorOccurred())
		{
			return errnoToTextureError(src.getErrno(),Texture::ERROR_FILE_OPEN);
		}

		if(!src.readBytes(tgaheader.Header,sizeof(tgaheader.Header)))
		{
			return Texture::ERROR_BAD_DATA;
		}

		Texture::ErrorE err = Texture::ERROR_NONE;

		if(memcmp(uTGAcompare,&tgaheader.Header,sizeof(tgaheader.Header))==0)
		{
			err = LoadUncompressedTGA(texture,src);
		}
		else if(memcmp(cTGAcompare,&tgaheader.Header,sizeof(tgaheader.Header))==0)
		{
			err = LoadCompressedTGA(texture,src);
		}
		else
		{
			fprintf(stderr,"Unknown file type (not compressed or uncompressed TGA\n");
			return Texture::ERROR_BAD_DATA;
		}

		return err;
	}
};

extern TextureFilter *tgatex(){ return new TgaTextureFilter; }
