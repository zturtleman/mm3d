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


#ifndef __OBJFILTER_H
#define __OBJFILTER_H

#include "modelfilter.h"

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

class DataSource;
class DataDest;

class ObjFilter : public ModelFilter
{
   public:

      // For a class derived from ModelFilter::Options, you'll
      // want to add member variables for everything the user
      // can control.  Whether or not you add accessors
      // for the member variables is left to your discretion.
      //
      // Set the default values for the filter options in
      // the constructor.
      class ObjOptions : public ModelFilter::Options
      {
         public:
            ObjOptions();

            virtual void release() { delete this; };

            bool m_saveNormals;
            int m_places;
            int m_texPlaces;
            int m_normalPlaces;

         protected:
            virtual ~ObjOptions(); // Use release() instead
      };

      ObjFilter();
      virtual ~ObjFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o = NULL );

      bool canRead( const char * filename );
      bool canWrite( const char * filename );
      bool canExport( const char * filename );

      bool isSupported( const char * filename );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      // Create a new options object that is specific to this filter
      ModelFilter::Options * getDefaultOptions() { return new ObjOptions; };

      class ObjMaterial
      {
         public:
            ObjMaterial();

            std::string name;
            float       diffuse[4];
            float       ambient[4];
            float       specular[4];
            float       shininess;
            float       alpha;
            std::string textureMap;
      };

      typedef struct _UvData_t
      {
         float u;
         float v;
      } UvDataT;
      typedef std::vector< UvDataT > UvDataList;

      typedef struct _MaterialGroup_t
      {
         unsigned material;
         unsigned group;
      } MaterialGroupT;
      typedef std::vector< MaterialGroupT > MaterialGroupList;

   protected:
      bool readLine( const char * line );
      bool readVertex( const char * line );
      bool readTextureCoord( const char * line );
      bool readFace( const char * line );
      bool readGroup( const char * line );
      bool readLibrary( const char * line );
      bool readMaterial( const char * line );

      bool readMaterialLibrary( const char * filename );

      void addObjMaterial( ObjMaterial * mat );
      const char * skipSpace( const char * str );

      bool writeLine( const char * line, ... );
      bool writeStripped( const char * line, ... );
      bool writeHeader();
      bool writeMaterials();
      bool writeGroups();

      Model       * m_model;
      ObjOptions  * m_options;
      FILE        * m_fp;
      DataSource  * m_src;
      DataDest    * m_dst;
      int           m_curGroup;
      int           m_curMaterial;
      bool          m_needGroup;
      int           m_vertices;
      int           m_faces;
      int           m_groups;
      UvDataList    m_uvList;
      MaterialGroupList m_mgList;

      std::string  m_groupName;

      std::string  m_modelPath;
      std::string  m_modelBaseName;
      std::string  m_modelFullName;
      std::string  m_materialFile;
      std::string  m_materialFullFile;
      std::vector< std::string > m_materialNames;
};

#endif // __OBJFILTER_H
