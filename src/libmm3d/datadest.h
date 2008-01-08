/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2008 Kevin Worcester
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 *
 *  See the COPYING file for full license text.
 */


#ifndef DATADEST_INC_H__
#define DATADEST_INC_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include "model.h"

//-----------------------------------------------------------------------------
// About DataDest
//-----------------------------------------------------------------------------
//
// A DataDest is used as export output for model or texture file filters.
// It can export binary or ASCII data. Binary data can be little endian
// (Intel byte order) or big endian (network byte order).
//
// DataDest provides FIXME advantages over direct file I/O:
//    * FIXME
//
//-----------------------------------------------------------------------------
// The DataDest API
//-----------------------------------------------------------------------------
//
// If you are implementing a model or texture file format filter for
// import, you should set the byte order that you expect for the
// file format you are reading. It is safe to change the byte order
// between write operations (assuming it makes sense to do so with the
// format of the export data).
//
// Write operations:
//   * FIXME
//
// All the write functions listed above return true on success and false on
// error. If any return false, either atFileLimit() will be true, or
// getErrno() will return a non-zero errno value. It is safe to use the
// functions to write a large number of unvalidated values and then check
// for errors.
//
// The offset() and seek() functions can be used to get and set the current
// write position, respectively.

//-----------------------------------------------------------------------------
// Implementing a DataDest
//-----------------------------------------------------------------------------
//
// In most cases, the DataDest will be a FileDataDest, which writes
// to a file on disk. Use the reference above or the class definition below
// to see the generic DataDest API that the FileDataDest conforms to.
//
// If you are implementing your own DataDest to write to some other type of
// output, here are the requirements a DataDest object must meet:
//
// * FIXME

class DataDest
{
   public:
      DataDest();
      virtual ~DataDest();

      enum EndiannessE {
         LittleEndian,    // Intel byte order, low-order byte first (default)
         BigEndian,       // Network byte order, high-order byte first
      };

      // Endianness of the input data.
      void setEndianness( EndiannessE e );
      EndiannessE getEndianness() { return m_endian; }

      // Returns the size of the output written so far.
      size_t getFileSize() { return m_fileSize; }

      // Set a limit on the maximum amount of data that can be written.
      void setFileSizeLimit( size_t bytes ); 

      // Returns the max size of the output
      size_t getFileSizeLimit() { return m_fileSizeLimit; }

      // Indicates if the output size is limited
      bool hasFileSizeLimit() { return m_hasLimit; }

      // Allow unrestricted file size
      void removeFileSizeLimit() { m_hasLimit = false; }

      // Move the current write position to 'offset' in the output.
      bool seek( off_t offset );

      // Returns the current write offset.
      off_t offset() { return m_offset; }

      // Write bufLen bytes to output
      // Returns false if a write error occurred.
      bool writeBytes( const uint8_t * buf, size_t bufLen );

      // Write a printf-style formatted string to output
      // Returns the size written in bytes (or -1 on error)
      ssize_t printf( const char * format, ... );

      // Writes a null-terminated string to output (including null)
      ssize_t writeAsciiz( const char * str );

      // Writes a null-terminated string to output (not including null)
      ssize_t writeString( const char * str );

      // Write an integer value of the specified size and store it in val.
      // Returns false if a read error occurred.
      bool write( const int8_t & val );
      bool write( const uint8_t & val );
      bool write( const int16_t & val );
      bool write( const uint16_t & val );
      bool write( const int32_t & val );
      bool write( const uint32_t & val );

      // An error occured, either atFileLimit() is true, or getErrno()
      // is not 0.
      bool errorOccurred() { return m_errorOccurred; }
      bool atFileLimit() { return m_atFileLimit; }
      int getErrno() { return m_errno; }

   protected:
      virtual bool internalWrite( const uint8_t * buf, size_t bufLen ) = 0;
      virtual bool internalSeek( off_t offset ) = 0;

      void setAtFileLimit( bool o );
      void setErrno( int err );

   private:
      bool canWrite( size_t bytes );

      enum
      {
         MAX_PRINTF_SIZE = 16 * 1024
      };

      EndiannessE m_endian;
      size_t m_fileSize;
      size_t m_fileSizeLimit;
      bool m_hasLimit;

      size_t m_offset;

      bool m_errorOccurred;
      bool m_atFileLimit;
      int m_errno;

      char m_strbuf[ MAX_PRINTF_SIZE ];

      typedef uint16_t (*EndianFunction16T)( uint16_t );
      typedef uint32_t (*EndianFunction32T) ( uint32_t );

      EndianFunction16T m_endfunc16;
      EndianFunction32T m_endfunc32;
};

#endif // DATADEST_INC_H__
