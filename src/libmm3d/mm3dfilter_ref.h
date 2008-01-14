/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2007 Kevin Worcester
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


#ifndef __MM3DFILTER_REF_H
#define __MM3DFILTER_REF_H

//-----------------------------------------------------------------------------
// Begin reference code
//-----------------------------------------------------------------------------

#include "modelfilter.h"
#include <stdint.h>

class MisfitFilterRef : public ModelFilter
{
   public:
      MisfitFilterRef();
      virtual ~MisfitFilterRef();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o = NULL );

      bool canRead( const char * filename );
      bool canWrite( const char * filename );
      bool canExport( const char * filename );

      bool isSupported( const char * filename );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      static const char     MAGIC[];

      static const uint8_t WRITE_VERSION_MAJOR;
      static const uint8_t WRITE_VERSION_MINOR;

      static const uint16_t OFFSET_TYPE_MASK;
      static const uint16_t OFFSET_UNI_MASK;
      static const uint16_t OFFSET_DIRTY_MASK;

   protected:

      void write( uint32_t  val );
      void write( uint16_t  val );
      void write( uint8_t   val );
      void write( int32_t   val );
      void write( int16_t   val );
      void write( int8_t    val );
      void write( float32_t val );
      void writeBytes( const void * buf, size_t len );
      void writeHeaderA( uint16_t flags, uint32_t count );
      void writeHeaderB( uint16_t flags, uint32_t count, uint32_t size );

      void read( uint32_t  & val );
      void read( uint16_t  & val );
      void read( uint8_t   & val );
      void read( int32_t   & val );
      void read( int16_t   & val );
      void read( int8_t    & val );
      void read( float32_t & val );
      void readBytes( void * buf, size_t len );
      void readHeaderA( uint16_t & flags, uint32_t & count );
      void readHeaderB( uint16_t & flags, uint32_t & count, uint32_t & size );

      uint8_t  * m_bufPos;
      FILE     * m_fp;
      size_t     m_readLength;

};

//-----------------------------------------------------------------------------
// End reference code
//-----------------------------------------------------------------------------

#endif  // __MM3DFILTER_REF_H
