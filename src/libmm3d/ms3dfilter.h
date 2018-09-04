/*  Maverick Model 3D
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

            void setOptionsFromModel( Model * m );

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

      void readString( char * buf, size_t len );

      bool readCommentSection();
      bool readVertexWeightSection();

      bool readVertexWeight( int subVersion, int vertex,
            VertexWeightList & weightList );

      void writeCommentSection();
      void writeVertexWeightSection( const MeshList & ml );
      void writeJointColorSection();

      // The InfluenceList must be sorted before calling this function
      void writeVertexWeight( int subVersion,
            const Model::InfluenceList & ilist );

      DataDest   * m_dst;
      DataSource * m_src;

      Model   * m_model;
      
      Ms3dOptions * m_options;

      static char const MAGIC_NUMBER[];
};

#endif // __MS3DFILTER_H
