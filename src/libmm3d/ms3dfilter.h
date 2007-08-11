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


#ifndef __MS3DFILTER_H
#define __MS3DFILTER_H

#include "modelfilter.h"
#include "mesh.h"

class Ms3dFilter : public ModelFilter
{
   public:
      class Ms3dOptions : public ModelFilter::Options
      {
         public:
            Ms3dOptions();

            virtual void release() { delete this; };

            int m_subVersion;
            uint32_t m_vertexExtra;
            uint32_t m_jointColor;

         protected:
            virtual ~Ms3dOptions(); // Use release() instead
      };

      Ms3dFilter() : ModelFilter(), m_options(NULL) {};
      virtual ~Ms3dFilter() {};

      struct _VertexWeight_t
      {
         int boneId;
         int weight;
      }; 
      typedef struct _VertexWeight_t VertexWeightT;
      typedef std::list<VertexWeightT> VertexWeightList;

      enum _CommentType_e
      {
         CT_GROUP,
         CT_MATERIAL,
         CT_JOINT,
         CT_MODEL,
         CT_MAX,
      };
      typedef enum _CommentType_e CommentTypeE;

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o = NULL );

      Options * getDefaultOptions() { return new Ms3dOptions; };

      bool canRead( const char * filename = NULL ) { return true; };
      bool canWrite( const char * filename = NULL ) { return true; };
      bool canExport( const char * filename = NULL ) { return true; };

      bool isSupported( const char * filename );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

   protected:

      void read( uint8_t & val );
      void read( uint16_t & val );
      void read( uint32_t & val );
      void read( int8_t & val );
      void read( int16_t & val );
      void read( int32_t & val );
      void read( float32_t & val );
      void readBytes( void * buf, size_t len );
      void readString( char * buf, size_t len );

      bool readCommentSection();
      bool readVertexWeightSection();

      bool readVertexWeight( int subVersion, int vertex,
            VertexWeightList & weightList );

      void skipBytes( size_t len );

      void write( uint8_t val );
      void write( uint16_t val );
      void write( uint32_t val );
      void write( int8_t val );
      void write( int16_t val );
      void write( int32_t val );
      void write( float32_t val );
      void writeBytes( const void * buf, size_t len );
      void writeString( const char * buf, size_t len );

      void writeCommentSection();
      void writeVertexWeightSection( const MeshList & ml );
      void writeJointColorSection();

      // The InfluenceList must be sorted before calling this function
      void writeVertexWeight( int subVersion,
            const Model::InfluenceList & ilist );

      uint8_t * m_buffer;
      uint8_t * m_bufPos;
      uint8_t * m_bufEnd;
      FILE    * m_fp;

      Model   * m_model;
      
      Ms3dOptions * m_options;

      static char const MAGIC_NUMBER[];
};

#endif // __MS3DFILTER_H
