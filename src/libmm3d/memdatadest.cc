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

#include "memdatadest.h"

MemDataDest::MemDataDest(uint8_t *buf, size_t bufSize)
	: m_buf(buf),
	  m_bufSize(bufSize),
	  m_bufOffset(0),
	  m_dataLen(0)
{
	if(m_buf==nullptr)
	{
		setErrno(EBADF);
		return;
	}
}

MemDataDest::~MemDataDest()
{
}

bool MemDataDest::internalSeek(off_t off)
{
	if(errorOccurred())
		return false;

	if((size_t)off>m_bufSize)
	{
		setAtFileLimit(true);
		return false;
	}

	m_bufOffset = off;

	if(m_bufOffset>m_dataLen)
		m_dataLen = m_bufOffset;

	return true;
}

bool MemDataDest::internalWrite(const uint8_t *buf, size_t bufLen)
{
	if(errorOccurred())
		return false;

	if(m_bufOffset+bufLen>m_bufSize)
	{
		m_dataLen = m_bufOffset;
		setAtFileLimit(true);
		return false;
	}

	memcpy(&m_buf[m_bufOffset],buf,bufLen);
	m_bufOffset += bufLen;

	if(m_bufOffset>m_dataLen)
		m_dataLen = m_bufOffset;

	return true;
}

