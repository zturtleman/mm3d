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


#ifndef __DXFFILTER_H
#define __DXFFILTER_H

#include "modelfilter.h"

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>


class DxfFilter : public ModelFilter
{
   public:
      DxfFilter();
      virtual ~DxfFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o = NULL );

      bool canRead( const char * filename );
      bool canWrite( const char * filename );
      bool canExport( const char * filename );

      bool isSupported( const char * filename );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

   protected:
      enum _ReadState_e
      {
          RS_MAIN,
          RS_SECTION,
          RS_HEADER,
          RS_TABLES,
          RS_LAYER,
          RS_ENTITIES,
          RS_3DFACE,
          RS_POLYLINE,
          RS_VERTEX,
          RS_UNKNOWN,
          RS_DONE,
          RS_MAX
      };
      typedef enum _ReadState_e ReadStateE;

      bool readLine( const char * line );
      bool writeLine( const char * line, ... );

      bool readMain( const char * line );
      bool readSection( const char * line );
      bool readHeader( const char * line );
      bool readTables( const char * line );
      bool readLayer( const char * line );
      bool readEntities( const char * line );
      bool read3dface( const char * line );
      bool readPolyline( const char * line );
      bool readVertex( const char * line );
      bool readUnknown( const char * line );

      void setReadState( ReadStateE state );

      void writeHeader();
      void writeGroups();
      void writeFaces();

      void materialPrep();
      int getGroupByName( const char * name );
      int getGroupByNameColor( const char * name, int colorIndex );
      void setMaterialColor( unsigned int material, int colorIndex );
      int  getMaterialColor( unsigned int material );
      bool materialHasColor( unsigned int material );

      Model       * m_model;
      FILE        * m_fp;
      
      ReadStateE   m_state;
      int          m_lastCode;
      int          m_currentGroup;
      int          m_currentColor;
      bool         m_isPolyfaceMesh;
      bool         m_vertexIsValid;
      bool         m_vertexIsFace;

      float        m_coord[4][3];
      int          m_vertices[4];
      int          m_baseVertex;

      std::string  m_modelPath;
      std::string  m_modelBaseName;
      std::string  m_modelFullName;

      std::map<int,int> m_materialColor;
};

#endif // __DXFFILTER_H
