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


#ifndef __CAL3DFILTER_H
#define __CAL3DFILTER_H

#include "modelfilter.h"

#include "mesh.h"

#include <list>
#include <vector>
#include <string>
#include <map>

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>


class Cal3dFilter : public ModelFilter
{
   public:
      // The default values for the filter options are in
      // the constructor.
      class Cal3dOptions : public ModelFilter::Options
      {
         public:
            Cal3dOptions();

            bool m_singleMeshFile;
            bool m_xmlMatFile;

            void setOptionsFromModel( Model * m );

         protected:
            virtual ~Cal3dOptions(); // Use release() instead
      };

      Cal3dFilter();
      virtual ~Cal3dFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o );

      bool canRead( const char * filename );
      bool canWrite( const char * filename );
      bool canExport( const char * filename );

      bool isSupported( const char * filename );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      ModelFilter::Options * getDefaultOptions() { return new Cal3dOptions; };

      struct _UVData_t
      {
          float u;
          float v;
      };
      typedef struct _UVData_t UVDataT;

      typedef std::vector< UVDataT > UVList;

   protected:
      enum _FileType_e
      {
          FT_Any,
          FT_Skeleton,
          FT_Animation,
          FT_Mesh,
          FT_Material,
          FT_MAX,
      };
      typedef enum _FileType_e FileTypeE;

      static bool listHas( const std::list<std::string> & l,
            const std::string & val );
      static std::string addExtension( const std::string file,
            const std::string ext );
      static bool versionIsValid( FileTypeE type, int version );
      static bool xversionIsValid( FileTypeE type, int version );

      // Sub file reads
      Model::ModelErrorE readSubFile( const char * filename );
      Model::ModelErrorE readXSubFile( uint8_t * buf, size_t len );

      Model::ModelErrorE readCal3dFile( uint8_t * buf, size_t len );
      Model::ModelErrorE readSkeletonFile( uint8_t * buf, size_t len );
      Model::ModelErrorE readMeshFile( uint8_t * buf, size_t len );
      Model::ModelErrorE readMaterialFile( uint8_t * buf, size_t len );
      Model::ModelErrorE readXMaterialFile( uint8_t * buf, size_t len );
      Model::ModelErrorE readAnimationFile( uint8_t * buf, size_t len );

      // Common read functions
      Model::ModelErrorE readFileToBuffer( const char * filename, uint8_t * & buf, size_t & len );
      void freeFileBuffer( uint8_t * buf );

      uint8_t readBUInt8();
      int16_t readBInt16();
      int32_t readBInt32();
      uint64_t readBUInt48();
      float readBFloat();
      Vector readBVector3();
      Vector readBVector4();

      bool readBString( std::string & );
      bool readBLine( std::string &, size_t maxLen );

      // common XML reading functions
      bool findXElement( const char * tag );            // Finds an XML element by tag
      std::string readXAttribute( const char * attr );  // Read a specified attribute of the current element
      std::string readXElement( const char * tag );     // Read the contents of the XML tag (if any)

      Vector readAVector3( const char * str );
      Vector readAVector4( const char * str );
      std::string readAString( const char * str );

      bool readBBone();
      bool readBSubMesh();
      bool readBAnimTrack();
      bool readBCompressedAnimTrack( double duration );

      std::string readLineKey( const char * str );
      std::string readLineFile( const char * str );

      // binary sub file writes
      Model::ModelErrorE writeCfgFile( const char * filename, Model * model, ModelFilter::Options * o );
      Model::ModelErrorE writeCal3dFile( const char * filename, Model * model, ModelFilter::Options * o );
      Model::ModelErrorE writeSkeletonFile( const char * filename, Model * model );
      Model::ModelErrorE writeMeshFile( const char * filename, Model * model );
      Model::ModelErrorE writeMeshListFile( const char * filename, Model * model, const MeshList & meshList );
      Model::ModelErrorE writeMaterialFile( const char * filename, Model * model, unsigned int materialId );
      Model::ModelErrorE writeAnimationFile( const char * filename, Model * model, unsigned int animationId );

      // XML sub file writes
      Model::ModelErrorE writeXMaterialFile( const char * filename, Model * model, unsigned int materialId );

      // Common binary write functions
      void writeBVector3( const Vector & vec );
      void writeBQuaternion( const Quaternion & quat );

      void writeBString( const std::string & );
      void writeBColor( const float * fval );

      void writeBBone( unsigned int b );
      void writeBAnimTrack( unsigned int anim, unsigned int bone );
      void writeBMesh( const Mesh & mesh );

      // Common XML write functions
      void writeXColor( const char * tag, const float * fval );

      int  timeToFrame( double tsec, double fps );
      int  findAnimation( const std::string& animName );
      
      Model         * m_model;
      Cal3dOptions  * m_options;

      DataDest   * m_dst;
      uint8_t    * m_bufPos;
      uint8_t    * m_fileBuf;
      size_t       m_fileLength;

      int          m_anim;
      bool         m_isBinary;
      bool         m_isLittleEndian;

      int          m_maxBinaryVersion;
      int          m_maxXrfVersion;

      std::string  m_modelPath;
      std::string  m_modelBaseName;
      std::string  m_modelFullName;

      std::string  m_currentPath;
      std::string  m_modelPartName;
      std::string  m_modelPartExt;

      std::list<std::string> m_formats;
};

#endif // __CAL3DFILTER_H
