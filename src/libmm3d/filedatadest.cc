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

#include "filedatadest.h"
#include "misc.h"

#ifdef WIN32
FileDataDest::FileDataDest(const char *filename)
	: m_startOffset(0),
	  m_mustClose(false)
{
	if(filename==nullptr||filename[0]=='\0')
	{
		sendErrno(EINVAL);
		return;
	}

	std::wstring wideString = utf8PathToWide(filename);
	if(wideString.empty())
	{
		setErrno(EINVAL);
		return;
	}

	m_handle = CreateFileW(&wideString[0],GENERIC_WRITE,FILE_SHARE_READ,nullptr,
								 CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
	if(m_handle==INVALID_HANDLE_VALUE||m_handle==nullptr)
	{
		m_handle = nullptr;

		if(GetLastError()==ERROR_ACCESS_DENIED)
		{
			sendErrno(EACCES);
		}
		else
		{
			sendErrno(ENOENT);
		}
		return;
	}

	m_mustClose = true;
}

FileDataDest::~FileDataDest()
{
	if(m_mustClose)
		close();
}

void FileDataDest::internalClose()
{
	if(m_handle!=nullptr)
	{
		CloseHandle(m_handle);
		m_handle = nullptr;
	}
}

void FileDataDest::sendErrno(int err)
{
	setErrno(err);
}

bool FileDataDest::internalSeek(off_t off)
{
	if(errorOccurred())
		return false;

	if(m_handle==nullptr)
	{
		sendErrno(EBADF);
		return false;
	}

	LARGE_INTEGER li;
	li.QuadPart = off+m_startOffset;

	li.LowPart = SetFilePointer(m_handle,li.LowPart,&li.HighPart,FILE_BEGIN);
	if(li.LowPart==INVALID_SET_FILE_POINTER&&GetLastError()!=NO_ERROR)
		return false;

	return true;
}

bool FileDataDest::internalWrite(const uint8_t *buf, size_t bufLen)
{
	if(errorOccurred())
		return false;

	if(m_handle==nullptr)
	{
		sendErrno(EBADF);
		return false;
	}

	if(bufLen==0)
		return true;

	DWORD wrote;
	if(WriteFile(m_handle,buf,bufLen,&wrote,nullptr)==FALSE||wrote!=bufLen)
	{
		sendErrno(EPERM);
		return false;
	}

	return true;
}
#else
FileDataDest::FileDataDest(const char *filename)
	: m_startOffset(0),
	  m_mustClose(false)
{
	if(filename==nullptr||filename[0]=='\0')
	{
		sendErrno(EINVAL);
		return;
	}

	m_fp = fopen(filename,"wb");
	if(m_fp==nullptr)
	{
		sendErrno(errno);
		return;
	}

	m_mustClose = true;
}

FileDataDest::~FileDataDest()
{
	if(m_mustClose)
		close();
}

void FileDataDest::internalClose()
{
	if(m_fp!=nullptr)
	{
		fclose(m_fp);
		m_fp = nullptr;
	}
}

void FileDataDest::sendErrno(int err)
{
	setErrno(err);
}

bool FileDataDest::internalSeek(off_t off)
{
	if(errorOccurred())
		return false;

	if(m_fp==nullptr)
	{
		sendErrno(EBADF);
		return false;
	}

	if(fseek(m_fp,off+m_startOffset,SEEK_SET)!=0)
		return false;

	return true;
}

bool FileDataDest::internalWrite(const uint8_t *buf, size_t bufLen)
{
	if(errorOccurred())
		return false;

	if(m_fp==nullptr)
	{
		sendErrno(EBADF);
		return false;
	}

	if(fwrite(buf,1,bufLen,m_fp)==0&&bufLen!=0)
	{
		sendErrno(errno);
		return false;
	}

	return true;
}
#endif // WIN32

