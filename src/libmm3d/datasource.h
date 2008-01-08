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


#ifndef DATASOURCE_INC_H__
#define DATASOURCE_INC_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include "model.h"

//-----------------------------------------------------------------------------
// About DataSource
//-----------------------------------------------------------------------------
//
// A DataSource is used as input for model or texture file filters.
// It can read binary or ASCII data. Binary data can be little endian
// (Intel byte order) or big endian (network byte order).
//
// DataSource provides three advantages over direct file I/O:
//    * Natively handles byte order conversions
//    * Offers convenience methods for common data sizes and other
//      read operations.
//    * Allows reading from non-file sources (memory buffers, etc)
//
//-----------------------------------------------------------------------------
// The DataSource API
//-----------------------------------------------------------------------------
//
// If you are implementing a model or texture file format filter for
// import, you should set the byte order that you expect for the
// file format you are reading. It is safe to change the byte order
// between read operations (assuming it makes sense to do so with the
// format of the input data).
//
// Read operations:
//   * readBytes() -- Reads a user-specified amount of data into a buffer.
//   * read(T) -- Overloaded, reads a value of type T from the input.
//   * readTo -- Reads until the next occurance of stopChar (or target buffer
//               is exhausted). If the foundChar pointer is not null it is
//               set to true if stopChar was found in the input and false
//               otherwise.
//   * readAsciiz -- Reads until the next NULL byte (or target buffer is 
//                   exhausted). This is the same as readTo with '\0' as the
//                   stopChar, except that the buffer is gauranteed to be
//                   NULL-terminated.
//
// All the read functions listed above return true on success and false on
// error. If any return false, either unexpectedEof() will be true, or
// getErrno() will return a non-zero errno value.
//
// The readXNN() functions are convenience functions that return the value
// read directly instead of returning a bool and modifying an int reference's
// value. It is safe to use these functions to read a large number of
// unvalidated values and then check for errors.
//
// The offset() and seek() functions can be used to get and set the current
// read position, respectively.

//-----------------------------------------------------------------------------
// Implementing a DataSource
//-----------------------------------------------------------------------------
//
// In most cases, the DataSource will be a FileDataSource, which reads
// from a file on disk. Use the reference above or the class definition below
// to see the generic DataSource API that the FileDataSource conforms to.
//
// If you are implementing your own DataSource to read from some other type of
// input, here are the requirements a DataSource object must meet:
//
// * Call setFileSize() as soon as the size of the input is known, generally
//   from the constructor of your object or from another initialization
//   function.
// * Implement internalReadAt, with the following behavior:
//     - It must read data at the offset specified.
//     - If there are less than 8 bytes remaining in the file, it must
//       return all of those bytes. If there are more than 8 bytes, it
//       may return any amount of data equal to or greater than 8 bytes.
//       (In short: always return more than 8 bytes, or the remainder of
//       the input data).
//     - It must modify buf and bufLen to hold a pointer to the read data
//       and the size of the data, respectively.
//     - The DataSource must be seekable. The offset argument may be
//       equal to, less than, or greater than a previous call. An offset
//       that has increased is not gauranteed to increase by the amount
//       of data returned by the last internalReadAt call.
//     - Before returning false, internalReadAt() must call setUnexpectedEof
//       with a 'true' argument, or setErrno() with a non-zero errno value,
//       depending on what sort of error occurred.
//     - internalReadAt() may be called again after returning false. In this
//       circumstance the DataSource may either: a) Continue returning the
//       same error, or b) Succeed if possible, as long as it meets the
//       requirements specified above. A successful read after an error
//       must not unset the unexpected eof or errno state (this requirement
//       enables the user to read data in several chunks and delay error
//       checking until all those chunks have been read).

class DataSource
{
   public:
      DataSource();
      virtual ~DataSource();

      enum EndiannessE {
         LittleEndian,    // Intel byte order, low-order byte first (default)
         BigEndian,       // Network byte order, high-order byte first
      };

      // Endianness of the input data.
      void setEndianness( EndiannessE e );
      EndiannessE getEndianness() { return m_endian; }

      // Returns the size of the input.
      size_t getFileSize() { return m_fileSize; }

      // Move the current read position to 'offset' in the input.
      bool seek( off_t offset );

      // Returns the current read offset.
      off_t offset() { return m_bufOffset; }

      // At end of source input (normal condition, or unexpected EOF)
      bool eof() { return m_bufOffset == m_fileSize; }

      // Read bufLen bytes from input and store it in buf.
      // Returns false if a read error occurred.
      bool readBytes( uint8_t * buf, size_t bufLen );

      // Copy input data into buf until stopChar is found (or bufLen is
      // exhausted). The stopChar itself is also copied. If the foundChar
      // pointer is not NULL, its value is set to true if foundChar was
      // found and false otherwise.
      // Returns false if a read error occurred.
      bool readTo( char stopChar, char * buf, size_t bufLen, bool * foundChar );

      // Copy input data into buf until a NULL byte is found (or bufLen is
      // exhausted). The NULL itself is also copied. This is the same as
      // readTo() above with '\0' as the stopChar, except that buf is
      // gauranteed to be null-terminated.
      // Returns false if a read error occurred.
      bool readAsciiz( char * buf, size_t bufLen, bool * foundNull );

      // Read an integer value of the specified size and store it in val.
      // Returns false if a read error occurred.
      bool read( int8_t & val );
      bool read( uint8_t & val );
      bool read( int16_t & val );
      bool read( uint16_t & val );
      bool read( int32_t & val );
      bool read( uint32_t & val );

      // For convinience, if you don't care about errors.
      // These are safe to use if you want to read a lot of unvalidated
      // data and then call unexpectedEof() at the end to make sure that
      // you didn't run out of data somewhere during your read calls.
      int8_t   readI8()  { int8_t rval;   read(rval); return rval; }
      uint8_t  readU8()  { uint8_t rval;  read(rval); return rval; }
      int16_t  readI16() { int16_t rval;  read(rval); return rval; }
      uint16_t readU16() { uint16_t rval; read(rval); return rval; }
      int32_t  readI32() { int32_t rval;  read(rval); return rval; }
      uint32_t readU32() { uint32_t rval; read(rval); return rval; }

      // An error occured, either unexpectedEof is true, or getErrno()
      // is not 0.
      bool errorOccurred() { return m_errorOccurred; }
      bool unexpectedEof() { return m_unexpectedEof; }
      int getErrno() { return m_errno; }

   protected:
      virtual bool internalReadAt( off_t offset, uint8_t ** buf, size_t * bufLen ) = 0;

      void setFileSize( size_t s ) { m_fileSize = s; }
      void setErrno( int err );
      void setUnexpectedEof( bool );

   private:
      bool requireBytes( size_t bytes );
      void advanceBytes( size_t bytes );
      bool fillBuffer();

      EndiannessE m_endian;
      uint8_t * m_buf;
      size_t m_fileSize;
      size_t m_bufLen;
      size_t m_bufOffset;

      bool m_errorOccurred;
      bool m_unexpectedEof;
      int m_errno;

      typedef uint16_t (*EndianFunction16T)( uint16_t );
      typedef uint32_t (*EndianFunction32T) ( uint32_t );

      EndianFunction16T m_endfunc16;
      EndianFunction32T m_endfunc32;
};

#endif // DATASOURCE_INC_H__
