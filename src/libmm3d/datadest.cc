/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2004-2008 Kevin Worcester
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

#include "datadest.h"
#include "endianconfig.h"
#include "mm3dport.h"

DestCloser::DestCloser(DataDest *src)
	: m_src(src)
{
}

DestCloser::~DestCloser()
{
	m_src->close();
}

DataDest::DataDest()
	: m_endian(LittleEndian),
	  m_fileSize(0),
	  m_fileSizeLimit(0),
	  m_hasLimit(false),
	  m_offset(0),
	  m_errorOccurred(false),
	  m_atFileLimit(false),
	  m_errno(0),
	  m_endfunc16(htol_u16),
	  m_endfunc32(htol_u32),
	  m_endfuncfl(htol_float)
{
}

DataDest::~DataDest()
{
}

void DataDest::setEndianness(EndiannessE e)
{
	m_endian = e;

	if(m_endian==LittleEndian)
	{
		m_endfunc16 = htol_u16;
		m_endfunc32 = htol_u32;
		m_endfuncfl = htol_float;
	}
	else
	{
		m_endfunc16 = htob_u16;
		m_endfunc32 = htob_u32;
		m_endfuncfl = htob_float;
	}
}

void DataDest::setFileSizeLimit(size_t bytes)
{
	m_fileSizeLimit = bytes;
	m_hasLimit = true;
}

bool DataDest::seek(off_t offset)
{
	if(m_hasLimit&&(size_t)offset>m_fileSizeLimit)
	{
		setAtFileLimit(true);
		m_errorOccurred = true;
		return false;
	}

	m_offset = offset;

	if(!internalSeek(m_offset))
	{
		// TODO should probably have an assert here to make sure that
		// the actual source set an error condition.
		return false;
	}

	return true;
}

void DataDest::setAtFileLimit(bool o)
{
	if(o)
		m_errorOccurred = true;
	m_atFileLimit = o;
}

void DataDest::setErrno(int err)
{
	m_errorOccurred = true;
	m_errno = err;
}

bool DataDest::canWrite(size_t bytes)
{
	m_offset += bytes;

	if(m_hasLimit&&(size_t)m_offset>m_fileSizeLimit)
	{
		m_offset -= bytes;
		setAtFileLimit(true);
		m_errorOccurred = true;
		return false;
	}

	m_fileSize = (m_offset>m_fileSize)? m_offset : m_fileSize;
	return true;
}

bool DataDest::write(int8_t val)
{
	if(!canWrite(sizeof(val)))
		return false;

	return internalWrite((const uint8_t *)&val,sizeof(val));
}

bool DataDest::write(uint8_t val)
{
	if(!canWrite(sizeof(val)))
		return false;

	return internalWrite(&val,sizeof(val));
}

bool DataDest::write(int16_t val)
{
	if(!canWrite(sizeof(val)))
		return false;

	uint16_t wval = m_endfunc16(val);
	return internalWrite((const uint8_t *)&wval,sizeof(wval));
}

bool DataDest::write(uint16_t val)
{
	if(!canWrite(sizeof(val)))
		return false;

	uint16_t wval = m_endfunc16(val);
	return internalWrite((const uint8_t *)&wval,sizeof(wval));
}

bool DataDest::write(int32_t val)
{
	if(!canWrite(sizeof(val)))
		return false;

	uint32_t wval = m_endfunc32(val);
	return internalWrite((const uint8_t *)&wval,sizeof(wval));
}

bool DataDest::write(uint32_t val)
{
	if(!canWrite(sizeof(val)))
		return false;

	uint32_t wval = m_endfunc32(val);
	return internalWrite((const uint8_t *)&wval,sizeof(wval));
}

bool DataDest::write(float val)
{
	if(!canWrite(sizeof(val)))
		return false;

	float wval = m_endfuncfl(val);
	return internalWrite((const uint8_t *)&wval,sizeof(wval));
}

bool DataDest::writeBytes(const void *buf, size_t bufLen)
{
	if(!canWrite(bufLen)) return false;

	return internalWrite((uint8_t*)buf,bufLen);
}

/*ssize_t*/intptr_t DataDest::writePrintf(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rval = writeVPrintf(fmt,ap);
	va_end(ap);
	return rval;
}

/*ssize_t*/intptr_t DataDest::writeVPrintf(const char *fmt, va_list ap)
{
	//int rval = vsnprintf(m_strbuf,MAX_PRINTF_SIZE,fmt,ap);
	int rval; for(size_t sz;sz=m_vpfbuf.size();)
	{
		va_list va; va_copy(va,ap); //C++11		
		rval = vsnprintf(m_vpfbuf.data(),sz,fmt,va); //ap	
		va_end(va);

		if(rval>=0&&rval<(int)sz)
		break;

		if(sz<MAX_PRINTF_SIZE)
		m_vpfbuf.resize((m_vpfbuf.empty()?1024:sz)*2);
		else break;
	}

	if(rval>=0)
	if(rval<MAX_PRINTF_SIZE) //C++11 (don't truncate?)
	{
		if(!writeBytes(m_vpfbuf.data(),(unsigned int)rval))
		rval = -1;
	}

	return rval;
}

/*ssize_t*/intptr_t DataDest::writeAsciiz(const char *buf)
{
	intptr_t len = strlen(buf)+1;

	return writeBytes(buf,len)?INT_MAX&len : -1;
}

/*ssize_t*/intptr_t DataDest::writeString(const char *buf)
{
	intptr_t len = strlen(buf);

	return writeBytes(buf,len)?len:-1;
}

