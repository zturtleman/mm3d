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


// Cal 3D notes:
//
// Main file may be .cal or .cfg
//
// The .cal file may be ascii text .ini style, or XML. The XML .cal file
// format is not supported.
//
// Versions:
//
// Version numbers are decimal
//
// Binary version 700 = XML Version 900 or 1000
//
// XML version 900:
//   File has a <HEADER/> element with version
// XML version 1000:
//   No header, version is part of top-level tag
//
// Binary version 1200:
//   Animation has an extra flag field at the start that indicates if
//   the animation tracks are compressed, plus possibly compressed tracks.

// FIXME things to test:
//
// Write version based on meta data (TEST)
// Read compressed tracks (TEST)
// Allow user to specify binary version (TEST)
//    Write '0' flags, don't bother writing compressed tracks (TEST)
// Allow user to specify XML version (TEST)
//    Omit header if >= 1000 (TEST)
// Are .cal and .cfg different? Looks like mesh/animation keys
//    don't have names (mesh= instead of mesh_foo=)

#include "cal3dfilter.h"

#include "model.h"
#include "texture.h"
#include "log.h"
#include "endianconfig.h"
#include "misc.h"
#include "filtermgr.h"
#include "texmgr.h"
#include "mesh.h"
#include "modelstatus.h"
#include "mm3dport.h"
#include "translate.h"
#include "datadest.h"
#include "datasource.h"
#include "file_closer.h"
#include "local_array.h"
#include "release_ptr.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif // WIN32

#include <map>
#include <vector>

using std::list;
using std::string;

#ifdef PLUGIN
static Cal3dFilter * s_filter = NULL;
#endif // PLUGIN

// These values are decimal, not hexadecimal
#define CAL3D_MIN_BVERSION  700
#define CAL3D_MAX_BVERSION  1200

#define CAL3D_MIN_XVERSION  900
#define CAL3D_MAX_XVERSION  1000

// Versions where formats changed. Files with versions equal to or later than
// these may make use of newer features.
#define CAL3D_COMP_ANIM_VERSION   1200  // Compressed animation tracks
#define CAL3D_NO_XHEADER_VERSION  1000  // XML files without <HEADER/> tags (version goes in top-level tag)

// File magic number values
#define CAL3D_MAGIC_SIZE      4
#define CAL3D_MAGIC_MATERIAL  "CRF\0"
#define CAL3D_MAGIC_MESH      "CMF\0"
#define CAL3D_MAGIC_SKELETON  "CSF\0"
#define CAL3D_MAGIC_ANIMATION "CAF\0"

// FIXME centralize this
template<typename T>
class FunctionCaller
{
   public:
      FunctionCaller( T * obj, void (T::*method)(void) ) { m_obj = obj; m_method = method; }
      ~FunctionCaller() { (m_obj->*m_method)(); }
   private:
      T * m_obj;
      void (T::*m_method)(void);
};
const Model::AnimationModeE MODE = Model::ANIMMODE_SKELETAL;

static char * _skipSpace( char * str )
{
   while ( isspace( str[0] ) )
   {
      str++;
   }
   return str;
}

static bool isallowed( char ch )
{
   switch ( ch ) 
   {
      case '_':
      case '-':
      case '.':
      case '+':
         return true;
      default:
         break;
   }
   return false;
}

static void _escapeCal3dName( std::string & name )
{
   size_t i = 0;
   while ( i < name.size() )
   {
      if ( isspace( name[i] ) )
      {
         name[i] = '_';
         i++;
      }
      else if ( !isalnum( name[i] ) && !isallowed( name[i] ) )
      {
         name.erase(i,1);
      }
      else
      {
         i++;
      }
   }
}

static void _escapeFileName( std::string & file )
{
   _escapeCal3dName( file ); // for now this is the same code
}

static void _strtolower( std::string & str )
{
   size_t i = 0;
   for ( i = 0; i < str.size(); i++ )
   {
      str[i] = tolower( str[i] );
   }
}

Cal3dFilter::Cal3dOptions::Cal3dOptions()
   : m_singleMeshFile( true ),
     m_xmlMatFile( true )
{
}

Cal3dFilter::Cal3dOptions::~Cal3dOptions()
{
}

void Cal3dFilter::Cal3dOptions::setOptionsFromModel( Model * m )
{
   char value[32];
   if ( m->getMetaData( "cal3d_single_mesh_file", value, sizeof(value) ) )
   {
      // Non-zero, single mesh
      m_singleMeshFile = atoi(value) != 0;
   }
   if ( m->getMetaData( "cal3d_xml_material", value, sizeof(value) ) )
   {
      // Non-zero, use XML format for material
      m_xmlMatFile = atoi(value) != 0;
   }
}

Cal3dFilter::Cal3dFilter()
   : m_model( NULL ),
     m_options( NULL )
{
   m_formats.push_back( "cal" );
   m_formats.push_back( "cfg" );
}

Cal3dFilter::~Cal3dFilter()
{
}

Model::ModelErrorE Cal3dFilter::readFile( Model * model, const char * const filename )
{
   Model::ModelErrorE err = Model::ERROR_NONE;

   // Use these to determine what versions of files to write.
   m_maxBinaryVersion = CAL3D_MIN_BVERSION;
   m_maxXrfVersion = CAL3D_MIN_XVERSION;

   if ( (err = readFileToBuffer( filename, m_fileBuf, m_fileLength )) == Model::ERROR_NONE )
   {
      local_array<uint8_t> releaseBuf( m_fileBuf );

      m_bufPos = m_fileBuf;

      m_modelPath = "";
      m_modelBaseName = "";
      m_modelFullName = "";

      normalizePath( filename, m_modelFullName, m_modelPath, m_modelBaseName );

      m_currentPath = m_modelPath;

      model->setFilename( m_modelFullName.c_str() );

      m_model = model;

      // Read specified file based on magic
      if ( memcmp( m_fileBuf, CAL3D_MAGIC_SKELETON, CAL3D_MAGIC_SIZE ) == 0 )
      {
         err = readSkeletonFile( m_fileBuf, m_fileLength );
      }
      else if ( memcmp( m_fileBuf, CAL3D_MAGIC_ANIMATION, CAL3D_MAGIC_SIZE ) == 0 )
      {
         err = readAnimationFile( m_fileBuf, m_fileLength );
      }
      else if ( memcmp( m_fileBuf, CAL3D_MAGIC_MESH, CAL3D_MAGIC_SIZE ) == 0 )
      {
         m_model->updateMetaData( "cal3d_single_mesh_file", "1" );
         err = readMeshFile( m_fileBuf, m_fileLength );
      }
      else if ( memcmp( m_fileBuf, CAL3D_MAGIC_MATERIAL, CAL3D_MAGIC_SIZE ) == 0 )
      {
         err = readMaterialFile( m_fileBuf, m_fileLength );
      }
      else if ( m_fileBuf[0] == '<' )
      {
         // probably an XML file, we only support material files
         err = readXSubFile( m_fileBuf, m_fileLength );
      }
      else
      {
         err = readCal3dFile( m_fileBuf, m_fileLength );
      }

      m_model->setupJoints();

      log_debug( "Cal3D Model:\n" );
      log_debug( "  vertices:  %d\n", m_model->getVertexCount() );
      log_debug( "  faces:     %d\n", m_model->getTriangleCount() );
      log_debug( "  groups:    %d\n", m_model->getGroupCount() );
      log_debug( "  bones:     %d\n", m_model->getBoneJointCount() );
      log_debug( "  materials: %d\n", m_model->getTextureCount() );

      m_fileBuf = NULL;
      m_bufPos  = NULL;
   }

   char version[32];

   PORT_snprintf( version, sizeof(version), "%d", m_maxXrfVersion );
   model->updateMetaData( "cal3d_xrf_version", version );

   PORT_snprintf( version, sizeof(version), "%d", m_maxBinaryVersion );
   model->updateMetaData( "cal3d_binary_version", version );

   return err;
}

Model::ModelErrorE Cal3dFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o  )
{
   if ( filename == NULL || filename[0] == '\0' )
   {
      return Model::ERROR_BAD_ARGUMENT;
   }

   m_modelPath = "";
   m_modelBaseName = "";
   m_modelFullName = "";

   normalizePath( filename, m_modelFullName, m_modelPath, m_modelBaseName );

   m_model = model;
   m_options = NULL;

   const char * ext = strrchr( filename, '.' );
   if ( ext )
   {
      ext++;

      if ( strcasecmp( ext, "CSF" ) == 0 )
      {
         return writeSkeletonFile( filename, model );
      }
      else if ( strcasecmp( ext, "CAF" ) == 0 )
      {
         if ( model->getAnimCount( MODE ) > 0 )
         {
            return writeAnimationFile( filename, model, 0 );
         }
         return Model::ERROR_BAD_DATA;
      }
      else if ( strcasecmp( ext, "CMF" ) == 0 )
      {
         return writeMeshFile( filename, model );
      }
      else if ( strcasecmp( ext, "CRF" ) == 0 )
      {
         if ( model->getTextureCount() > 0 )
         {
            return writeMaterialFile( filename, model, 0 );
         }
         return Model::ERROR_BAD_DATA;
      }
      else if ( strcasecmp( ext, "XRF" ) == 0 )
      {
         if ( model->getTextureCount() > 0 )
         {
            return writeXMaterialFile( filename, model, 0 );
         }
         return Model::ERROR_BAD_DATA;
      }
      else if ( toupper(ext[0]) == 'X' )
      {
         // Assume XML file
         return Model::ERROR_UNSUPPORTED_VERSION;
      }

      // Assume Cal3D master file
      return writeCal3dFile( filename, model, o );
   }
   else
   {
      return writeCal3dFile( filename, model, o );
   }
}

bool Cal3dFilter::canRead( const char * filename )
{
   log_debug( "canRead( %s )\n", filename );
   log_debug( "  true\n" );
   return true;
}

bool Cal3dFilter::canWrite( const char * filename )
{
   log_debug( "canWrite( %s )\n", filename );
   log_debug( "  false\n" );
   return false;
}

bool Cal3dFilter::canExport( const char * filename )
{
   log_debug( "canExport( %s )\n", filename );
   log_debug( "  true\n" );
   return true;
}

bool Cal3dFilter::isSupported( const char * filename )
{
   log_debug( "isSupported( %s )\n", filename );
   unsigned len = strlen(filename);
   for ( std::list<std::string>::const_iterator it = m_formats.begin();
         it != m_formats.end(); ++it )
   {
      const std::string fmt = std::string(".") + *it;
      unsigned fmtlen = fmt.size();
      if ( len >= fmtlen && strcasecmp( &filename[len-fmtlen], fmt.c_str() ) == 0 )
      {
         log_debug( "  true\n" );
         return true;
      }
   }
   log_debug( "  false\n" );
   return false;
}

list< string > Cal3dFilter::getReadTypes()
{
   list<string> rval;
   for ( std::list<std::string>::const_iterator it = m_formats.begin();
         it != m_formats.end(); ++it )
   {
      rval.push_back( std::string("*.") + *it );
   }
   return rval;
}

list< string > Cal3dFilter::getWriteTypes()
{
   list<string> rval;
   for ( std::list<std::string>::const_iterator it = m_formats.begin();
         it != m_formats.end(); ++it )
   {
      rval.push_back( std::string("*.") + *it );
   }
   return rval;
}

//------------------------------------------------------------------
// Protected Methods
//------------------------------------------------------------------

bool Cal3dFilter::listHas( const std::list<std::string> & l,
      const std::string & val )
{
   std::list<std::string>::const_iterator it = l.begin();
   while ( it != l.end() )
   {
      if ( (*it) == val )
      {
         return true;
      }
      it++;
   }
   return false;
}

std::string Cal3dFilter::addExtension( const std::string file, const std::string ext )
{
   if ( file.size() > ext.size() )
   {
      std::string cmp = std::string(".") + ext;
      size_t len = cmp.size() - file.size();
      if ( strcasecmp( &(file.c_str()[file.size() - len]),
               ext.c_str() ) == 0 )
      {
         return file;
      }
   }
   return file + std::string(".") + ext;
}

bool Cal3dFilter::versionIsValid( FileTypeE type, int version )
{
   if ( version >= CAL3D_MIN_BVERSION && version <= CAL3D_MAX_BVERSION )
   {
      return true;
   }
   return false;
}

bool Cal3dFilter::xversionIsValid( FileTypeE type, int version )
{
   if ( version >= CAL3D_MIN_XVERSION && version <= CAL3D_MAX_XVERSION )
   {
      return true;
   }
   return false;
}

//------------------------------------------------------------------
// Common read functions

Model::ModelErrorE Cal3dFilter::readSubFile( const char * filename )
{
   Model::ModelErrorE err = Model::ERROR_NONE;

   std::string oldPath = m_currentPath;

   std::string fullPath = m_currentPath + std::string( "/" ) + filename;

   uint8_t * buf;
   size_t    len;

   if ( (err = readFileToBuffer( fullPath.c_str(), buf, len )) == Model::ERROR_NONE )
   {
      local_array<uint8_t> releaseBuf(buf);

      std::string baseName;
      std::string fullName;
      normalizePath( fullPath.c_str(), fullName, m_currentPath, baseName );

      const char * part = strrchr( filename, '/' );
      if ( part )
      {
         part++;
      }
      else
      {
         part = filename;
      }
      const char * ext = strrchr( filename, '.' );
      if ( ext )
      {
         m_modelPartName.assign( part, ext - part );
         m_modelPartExt = &ext[1];
      }
      else
      {
         m_modelPartName = part;
      }

      // Read specified file based on magic
      if ( memcmp( buf, CAL3D_MAGIC_SKELETON, CAL3D_MAGIC_SIZE ) == 0 )
      {
         err = readSkeletonFile( buf, len );
      }
      else if ( memcmp( buf, CAL3D_MAGIC_ANIMATION, CAL3D_MAGIC_SIZE ) == 0 )
      {
         err = readAnimationFile( buf, len );
      }
      else if ( memcmp( buf, CAL3D_MAGIC_MESH, CAL3D_MAGIC_SIZE ) == 0 )
      {
         err = readMeshFile( buf, len );
      }
      else if ( memcmp( buf, CAL3D_MAGIC_MATERIAL, CAL3D_MAGIC_SIZE ) == 0 )
      {
         err = readMaterialFile( buf, len );
      }
      else if ( buf[0] == '<' )
      {
         // probably an XML file, try to parse it
         err = readXSubFile( buf, len );
      }
      else
      {
         err = readCal3dFile( buf, len );
      }

      m_currentPath = oldPath;
   }

   if ( err != Model::ERROR_NONE )
   {
      std::string errStr = filename;
      errStr += ": ";
      errStr += Model::errorToString( err );

      model_status( m_model, StatusError, STATUSTIME_LONG, errStr.c_str() );
   }

   return err;
}

Model::ModelErrorE Cal3dFilter::readXSubFile( uint8_t * buf, size_t len )
{
   Model::ModelErrorE err = Model::ERROR_UNSUPPORTED_VERSION;

   uint8_t * oldFileBuf = m_fileBuf;
   uint8_t * oldBufPos  = m_bufPos;
   size_t    oldLen     = m_fileLength;

   m_fileBuf = buf;
   m_bufPos  = buf;
   m_fileLength = len;

   // Only support material XML files

   // Some versions of CAL3D files have a header element, some do not
   // If we see a HEADER element, get the MAGIC attribute
   if ( findXElement( "HEADER" ) )
   {
      log_debug( "XML file has a <HEADER/> element\n" );
      std::string magic = readXAttribute( "MAGIC" );

      if ( strcmp( magic.c_str(), "XRF" ) == 0 )
      {
         log_debug( "XML file is a material file\n" );
         err = readXMaterialFile( buf, len );
      }
      else
      {
         log_warning( "XML file is an unknown type: %s\n", magic.c_str() );
      }
   }
   else if ( findXElement( "MATERIAL" ) )
   {
      log_debug( "XML file does not have a header\n" );
      log_debug( "XML file is a material file\n" );
      err = readXMaterialFile( buf, len );
   }
   else
   {
      log_debug( "Could not determine XML file type\n" );
      log_warning( "XML file is an unsupported type (unrecognized root tag)\n" );
   }

   m_fileBuf    = oldFileBuf;
   m_bufPos     = oldBufPos;
   m_fileLength = oldLen;

   return err;
}

Model::ModelErrorE Cal3dFilter::readCal3dFile( uint8_t * buf, size_t len )
{
   log_debug( "reading cal3d config file\n" );

   Model::ModelErrorE rval = Model::ERROR_NONE;

   uint8_t * oldFileBuf = m_fileBuf;
   uint8_t * oldBufPos  = m_bufPos;
   size_t    oldLen     = m_fileLength;

   m_fileBuf = buf;
   m_bufPos  = buf;
   m_fileLength = len;

   string line    = "";
   string subfile = "";

   // We start in the [model] section. Technically this is
   // incorrect, but it's an attempt to get around possible
   // bad files or bad parsing logic. This shouldn't hurt anything.
   // If there is a non-model section before the model section
   // it will be skipped.
   bool modelSection = true;

   // Save mesh, material, and animation files so we can
   // load them in our own preferred order. The only thing
   // we load as soon as we see it is the skeleton file.
   std::list< std::string > meshFiles;
   std::list< std::string > matFiles;
   std::list< std::string > animFiles;

   // If no usable data is found, the bracket count (ie, number of lines
   // that start with '<') is used to determine if the file was in XML format
   // so that we can give a more useful error message to the user.
   int bracketCount = 0;

   bool noData = true;

   while ( readBLine( line, len - (m_bufPos - m_fileBuf) ) )
   {
      if ( modelSection )
      {
         const char * str = _skipSpace( (char *) line.c_str() );

         // only parse lines that are not empty or comments
         if ( str[0] && str[0] != '#' )
         {
            if ( strncasecmp( str, "skeleton", 8 ) == 0 )
            {
               noData = false;
               subfile = readLineFile( str );
               if ( !subfile.empty()  )
               {
                  log_debug( "loading skeleton file %s\n", subfile.c_str() );
                  readSubFile( subfile.c_str() );
               }
            }
            else if ( strncasecmp( str, "mesh", 4 ) == 0 )
            {
               noData = false;
               subfile = readLineFile( str );
               if ( !subfile.empty()  )
               {
                  meshFiles.push_back( subfile );
               }
            }
            else if ( strncasecmp( str, "animation", 9 ) == 0 )
            {
               noData = false;
               std::string animLabel = readLineKey( str );
               subfile = readLineFile( str );

               if ( !subfile.empty() && !animLabel.empty() )
               {
                  m_model->updateMetaData( animLabel.c_str(), subfile.c_str() );
                  if ( !listHas( animFiles, subfile ) )
                  {
                     animFiles.push_back( subfile );
                  }
               }
            }
            else if ( strncasecmp( str, "material", 8 ) == 0 )
            {
               noData = false;
               subfile = readLineFile( str );
               if ( !subfile.empty()  )
               {
                  matFiles.push_back( subfile );
               }
            }
            else if ( strncasecmp( str, "path", 4 ) == 0 )
            {
               noData = false;
               string value = readLineFile( str );
               m_model->updateMetaData( "cal3d_path", value.c_str() );
            }
            else if ( strncasecmp( str, "scale", 5 ) == 0 )
            {
               noData = false;
               string value = readLineFile( str );
               m_model->updateMetaData( "cal3d_scale", value.c_str() );
            }
            else if ( strncasecmp( str, "rotate", 6 ) == 0 )
            {
               noData = false;
               string value = readLineFile( str );
               m_model->updateMetaData( "cal3d_rotate", value.c_str() );
            }
            else if ( str[0] == '[' )
            {
               // looks like a new section, see if it's a model section
               str++;
               while ( isspace( str[0] ) )
               {
                  str++;
               }

               if ( strncasecmp( str, "model", 5 ) == 0 )
               {
                  modelSection = true;
               }
               else
               {
                  // Not a model section, skip it
                  log_debug( "skipping [%s section\n", str );
                  modelSection = false;
               }
            }
            else if ( str[0] == '<' )
            {
               ++bracketCount;
            }
         }

         if ( (m_bufPos - m_fileBuf) >= (int) m_fileLength )
         {
            break;
         }
      }
      else
      {
         // Not in the model section, the only parsing we want
         // to do is to find the model section...
         const char * str = _skipSpace( (char *) line.c_str() );
         if ( str[0] == '[' )
         {
            // looks like a new section, see if it's a model section

            str++;
            while ( isspace( str[0] ) )
            {
               str++;
            }

            if ( strncasecmp( str, "model", 5 ) == 0 )
            {
               // Yes, it's a model section, enable the parsing logic
               log_debug( "entering model section\n" );
               modelSection = true;
            }
            else
            {
               log_debug( "skipping [%s section\n", str );
               modelSection = false;
            }
         }

         if ( (m_bufPos - m_fileBuf) >= (int) m_fileLength )
         {
            break;
         }
      }
   }

   std::list< std::string >::iterator it;

   // If we didn't find any usable data, return an error.
   if ( noData )
   {
      if ( bracketCount > 2 )
         m_model->setFilterSpecificError( "MM3D does not support CAL3D files in XML format" );
      else
         m_model->setFilterSpecificError( "The file does not contain any mesh or animation data" );

      rval = Model::ERROR_FILTER_SPECIFIC;
      goto bail_out;
   }

   // Load materials first because meshes will reference them
   for ( it = matFiles.begin(); it != matFiles.end(); it++ )
   {
      log_debug( "loading material file %s\n", (*it).c_str() );
      readSubFile( (*it).c_str() );
   }

   // Now load meshes
   for ( it = meshFiles.begin(); it != meshFiles.end(); it++ )
   {
      log_debug( "loading mesh file %s\n", (*it).c_str() );
      readSubFile( (*it).c_str() );
   }

   m_model->updateMetaData( "cal3d_single_mesh_file",
         ( meshFiles.size() == 1 ) ? "1" : "0" );

   // Now load animations
   for ( it = animFiles.begin(); it != animFiles.end(); it++ )
   {
      log_debug( "loading animation file %s\n", (*it).c_str() );
      readSubFile( (*it).c_str() );
   }

bail_out:;
   m_fileBuf    = oldFileBuf;
   m_bufPos     = oldBufPos;
   m_fileLength = oldLen;

   return rval;
}

Model::ModelErrorE Cal3dFilter::readSkeletonFile( uint8_t * buf, size_t len )
{
   uint8_t * oldFileBuf = m_fileBuf;
   uint8_t * oldBufPos  = m_bufPos;
   size_t    oldLen     = m_fileLength;

   m_fileBuf = buf;
   m_bufPos  = buf;
   m_fileLength = len;

   Model::ModelErrorE rval = Model::ERROR_NONE;

   if ( memcmp( CAL3D_MAGIC_SKELETON, m_bufPos, CAL3D_MAGIC_SIZE ) == 0 )
   {
      m_bufPos += CAL3D_MAGIC_SIZE;

      int fileVersion = readBInt32();
      int numBones    = readBInt32();

      if ( fileVersion > m_maxBinaryVersion )
         m_maxBinaryVersion = fileVersion;

      log_debug( "skel version: %d (%x)\n", fileVersion, fileVersion );

      bool success = true;
      if ( versionIsValid( FT_Skeleton, fileVersion ) )
      {
         for ( int b = 0; success && b < numBones; b++ )
         {
            success = readBBone();
         }
      }
      else
      {
         rval = Model::ERROR_UNSUPPORTED_VERSION;
         log_error( "Unsupported CAL3D skeleton version %d\n", fileVersion );
      }
   }
   else
   {
      rval = Model::ERROR_BAD_MAGIC;
      log_error( "Bad magic in skeleton file\n" );
   }

   m_fileBuf    = oldFileBuf;
   m_bufPos     = oldBufPos;
   m_fileLength = oldLen;

   return rval;
}

Model::ModelErrorE Cal3dFilter::readMeshFile( uint8_t * buf, size_t len )
{
   uint8_t * oldFileBuf = m_fileBuf;
   uint8_t * oldBufPos  = m_bufPos;
   size_t    oldLen     = m_fileLength;

   m_fileBuf = buf;
   m_bufPos  = buf;
   m_fileLength = len;

   Model::ModelErrorE rval = Model::ERROR_NONE;

   if ( memcmp( CAL3D_MAGIC_MESH, m_bufPos, CAL3D_MAGIC_SIZE ) == 0 )
   {
      m_bufPos += CAL3D_MAGIC_SIZE;

      int fileVersion  = readBInt32();
      int numSubMeshes = readBInt32();

      if ( fileVersion > m_maxBinaryVersion )
         m_maxBinaryVersion = fileVersion;

      log_debug( "mesh version: %d (%x)\n", fileVersion, fileVersion );

      bool success = true;
      if ( versionIsValid( FT_Mesh, fileVersion ) )
      {
         for ( int m = 0; success && m < numSubMeshes; m++ )
         {
            success = readBSubMesh();
         }
      }
      else
      {
         rval = Model::ERROR_UNSUPPORTED_VERSION;
         log_error( "Unsupported CAL3D mesh version %d\n", fileVersion );
      }
   }
   else
   {
      rval = Model::ERROR_BAD_MAGIC;
      log_error( "Bad magic in mesh file\n" );
   }

   m_fileBuf    = oldFileBuf;
   m_bufPos     = oldBufPos;
   m_fileLength = oldLen;

   return rval;
}

Model::ModelErrorE Cal3dFilter::readMaterialFile( uint8_t * buf, size_t len )
{
   uint8_t * oldFileBuf = m_fileBuf;
   uint8_t * oldBufPos  = m_bufPos;
   size_t    oldLen     = m_fileLength;

   m_fileBuf = buf;
   m_bufPos  = buf;
   m_fileLength = len;

   m_model->updateMetaData( "cal3d_xml_material", "0" );

   Model::ModelErrorE rval = Model::ERROR_NONE;

   // We're going to add a material whether successful or not. Materials are
   // referenced by index so the material needs to exist even if it's not valid.
   Model::Material * mat = Model::Material::get();
   mat->m_type     = Model::Material::MATTYPE_BLANK; // assume
   mat->m_name     = m_modelPartName; // this should be a sensible name
   mat->m_filename = ""; // none by default

   if ( memcmp( CAL3D_MAGIC_MATERIAL, m_bufPos, CAL3D_MAGIC_SIZE ) == 0 )
   {
      m_bufPos += CAL3D_MAGIC_SIZE;

      int fileVersion = readBInt32();

      if ( fileVersion > m_maxBinaryVersion )
         m_maxBinaryVersion = fileVersion;

      log_debug( "mat version: %d (%x)\n", fileVersion, fileVersion );

      if ( versionIsValid( FT_Material, fileVersion ) )
      {
         Vector ambient;
         ambient[0] = ((float) readBUInt8()) / 255.0f;
         ambient[1] = ((float) readBUInt8()) / 255.0f;
         ambient[2] = ((float) readBUInt8()) / 255.0f;
         ambient[3] = ((float) readBUInt8()) / 255.0f;

         Vector diffuse;
         diffuse[0] = ((float) readBUInt8()) / 255.0f;
         diffuse[1] = ((float) readBUInt8()) / 255.0f;
         diffuse[2] = ((float) readBUInt8()) / 255.0f;
         diffuse[3] = ((float) readBUInt8()) / 255.0f;

         Vector specular;
         specular[0] = ((float) readBUInt8()) / 255.0f;
         specular[1] = ((float) readBUInt8()) / 255.0f;
         specular[2] = ((float) readBUInt8()) / 255.0f;
         specular[3] = ((float) readBUInt8()) / 255.0f;

         float  shiny    = readBFloat();

         int numMaps = readBInt32();

         if ( numMaps > 0 )
         {
            // Texture-map, change type and add filename
            string filename;
            readBString( filename );

            mat->m_type = Model::Material::MATTYPE_TEXTURE;
            mat->m_filename = normalizePath(filename.c_str(), m_currentPath.c_str());

            // ignore everything else in the file
         }

         log_debug( "Reading material %d\n", m_model->getTextureCount() );
         log_debug( "  name %s\n", mat->m_name.c_str() );
         log_debug( "  num maps %d\n", numMaps );
         log_debug( "  file %s\n", mat->m_filename.c_str() );
         log_debug( "  diffuse %f,%f,%f\n", 
               diffuse[0], diffuse[1], diffuse[2] );
         log_debug( "  ambient %f,%f,%f\n", 
               ambient[0], ambient[1], ambient[2] );
         log_debug( "  specular %f,%f,%f\n", 
               specular[0], specular[1], specular[2] );
         log_debug( "  shininess %f\n", shiny );

         if ( numMaps > 1 )
         {
            log_warning( "  ignoring %d maps for %s\n", 
                  numMaps - 1, mat->m_name.c_str() );
         }

         for ( int t = 0; t < 4; t++ )
         {
            mat->m_diffuse[t]  = diffuse[t];
            mat->m_ambient[t]  = ambient[t];
            mat->m_specular[t] = specular[t];
            mat->m_emissive[t] = 0.0f;
         }
         mat->m_shininess = shiny;
      }
      else
      {
         rval = Model::ERROR_UNSUPPORTED_VERSION;
         log_error( "Unsupported CAL3D material version %d\n", fileVersion );
      }
   }
   else
   {
      rval = Model::ERROR_BAD_MAGIC;
      log_error( "Bad magic in material file\n" );
   }

   getMaterialList( m_model ).push_back( mat );

   m_fileBuf    = oldFileBuf;
   m_bufPos     = oldBufPos;
   m_fileLength = oldLen;

   return rval;
}

Model::ModelErrorE Cal3dFilter::readXMaterialFile( uint8_t * buf, size_t len )
{
   uint8_t * oldFileBuf = m_fileBuf;
   uint8_t * oldBufPos  = m_bufPos;
   size_t    oldLen     = m_fileLength;

   m_fileBuf = buf;
   m_bufPos  = buf;
   m_fileLength = len;

   m_model->updateMetaData( "cal3d_xml_material", "1" );

   Model::ModelErrorE rval = Model::ERROR_NONE;

   // We're going to add a material whether successful or not. Materials are
   // referenced by index so the material needs to exist even if it's not valid.
   Model::Material * mat = Model::Material::get();
   mat->m_type     = Model::Material::MATTYPE_BLANK; // assume
   mat->m_name     = m_modelPartName; // this should be a sensible name
   mat->m_filename = ""; // none by default

   if ( len > 0 )
   {
      // don't worry about magic and version, just get to the material tag
      if ( findXElement( "MATERIAL" ) )
      {
         std::string versionStr = readXAttribute( "VERSION" );
         int xrfVersion = atoi( versionStr.c_str() );
         if ( xrfVersion > m_maxXrfVersion )
            m_maxXrfVersion = xrfVersion;

         Vector ambient( 0, 0, 0, 1 );
         Vector diffuse( 1, 1, 1, 1 );
         Vector specular( 0, 0, 0, 1 );
         string matFile = "";

         float  shiny = 0.0f;

         if ( findXElement( "AMBIENT" ) )
         {
            ambient = readAVector4( readXElement( "AMBIENT" ).c_str() );
            ambient[0] /= 255.0;
            ambient[1] /= 255.0;
            ambient[2] /= 255.0;
            ambient[3] /= 255.0;
         }
         if ( findXElement( "DIFFUSE" ) )
         {
            diffuse = readAVector4( readXElement( "DIFFUSE" ).c_str() );
            diffuse[0] /= 255.0;
            diffuse[1] /= 255.0;
            diffuse[2] /= 255.0;
            diffuse[3] /= 255.0;
         }
         if ( findXElement( "SPECULAR" ) )
         {
            specular = readAVector4( readXElement( "SPECULAR" ).c_str() );
            specular[0] /= 255.0;
            specular[1] /= 255.0;
            specular[2] /= 255.0;
            specular[3] /= 255.0;
         }
         if ( findXElement( "SHININESS" ) )
         {
            shiny = atof( readXElement( "SHININESS" ).c_str() );
         }
         if ( findXElement( "MAP" ) )
         {
            matFile = readAString( readXElement( "MAP" ).c_str() );
         }

         if ( !matFile.empty() )
         {
            mat->m_type = Model::Material::MATTYPE_TEXTURE;
            mat->m_filename = normalizePath(matFile.c_str(), m_currentPath.c_str());
         }

         log_debug( "Reading material %d\n", m_model->getTextureCount() );
         log_debug( "  name %s\n", mat->m_name.c_str() );
         log_debug( "  file %s\n", mat->m_filename.c_str() );
         log_debug( "  diffuse %f,%f,%f\n", 
               diffuse[0], diffuse[1], diffuse[2] );
         log_debug( "  ambient %f,%f,%f\n", 
               ambient[0], ambient[1], ambient[2] );
         log_debug( "  specular %f,%f,%f\n", 
               specular[0], specular[1], specular[2] );
         log_debug( "  shininess %f\n", shiny );

         for ( int t = 0; t < 4; t++ )
         {
            mat->m_diffuse[t]  = diffuse[t];
            mat->m_ambient[t]  = ambient[t];
            mat->m_specular[t] = specular[t];
            mat->m_emissive[t] = 0.0f;
         }
         mat->m_shininess = shiny;
      }
      else
      {
         rval = Model::ERROR_BAD_MAGIC;
         log_error( "Could not find MATERIAL element in material file\n" );
      }
   }
   else
   {
      rval = Model::ERROR_FILE_READ;
      log_error( "File is empty\n" );
   }

   getMaterialList( m_model ).push_back( mat );

   m_fileBuf    = oldFileBuf;
   m_bufPos     = oldBufPos;
   m_fileLength = oldLen;

   return rval;
}

Model::ModelErrorE Cal3dFilter::readAnimationFile( uint8_t * buf, size_t len )
{
   uint8_t * oldFileBuf = m_fileBuf;
   uint8_t * oldBufPos  = m_bufPos;
   size_t    oldLen     = m_fileLength;

   m_fileBuf = buf;
   m_bufPos  = buf;
   m_fileLength = len;

   Model::ModelErrorE rval = Model::ERROR_NONE;

   if ( memcmp( CAL3D_MAGIC_ANIMATION, m_bufPos, CAL3D_MAGIC_SIZE ) == 0 )
   {
      m_bufPos += CAL3D_MAGIC_SIZE;

      int fileVersion  = readBInt32();
      float duration   = readBFloat();
      int numTracks    = readBInt32();

      if ( fileVersion > m_maxBinaryVersion )
         m_maxBinaryVersion = fileVersion;

      log_debug( "anim version: %d (%x)\n", fileVersion, fileVersion );

      if ( versionIsValid( FT_Animation, fileVersion ) )
      {
         std::string name = m_modelPartName;

         bool compressed = false;
         if ( fileVersion >= CAL3D_COMP_ANIM_VERSION )
         {
            int flags = readBInt32();
            compressed = ( flags & 1 != 0 );
         }
         //name += ".";
         //name += m_modelPartExt;

         //log_debug( "  tracks: %d\n", numTracks );
         //log_debug( "  seconds: %f\n", duration );

         // create animation
         // NOTE: assume 30 fps, may want to be smarter about this
         m_anim = m_model->addAnimation( MODE, name.c_str() );
         m_model->setAnimFPS( MODE, m_anim, 30.0 );

         m_model->setAnimFrameCount( MODE, m_anim, timeToFrame(duration, 30.0) + 1);

         bool success = true;
         for ( int t = 0; success && t < numTracks; t++ )
         {
            if ( compressed )
               success = readBCompressedAnimTrack( duration );
            else
               success = readBAnimTrack();
         }
      }
      else
      {
         rval = Model::ERROR_UNSUPPORTED_VERSION;
         log_error( "Unsupported CAL3D animation version %d\n", fileVersion );
      }
   }
   else
   {
      rval = Model::ERROR_BAD_MAGIC;
      log_error( "Bad magic in mesh file\n" );
   }

   m_fileBuf    = oldFileBuf;
   m_bufPos     = oldBufPos;
   m_fileLength = oldLen;

   return rval;
}

Model::ModelErrorE Cal3dFilter::readFileToBuffer( const char * filename, uint8_t * & buf, size_t & len )
{
   buf = NULL;
   len = 0;

   Model::ModelErrorE err = Model::ERROR_NONE;
   DataSource * src = openInput( filename, err );
   FunctionCaller<DataSource> fc( src, &DataSource::close );

   if ( err != Model::ERROR_NONE )
      return err;

   len = src->getFileSize();
   buf = new uint8_t[len];
   src->readBytes( buf, len );

   return err;
}

bool Cal3dFilter::readBBone()
{
   log_debug( "Reading bone joint %d\n", m_model->getBoneJointCount() );

   // Bone name
   std::string name;
   readBString( name );

   log_debug( "  name %s\n", name.c_str() );

   // Bone position
   Vector trans   = readBVector3();
   Vector rotVec  = readBVector4();

   // Bone space translation
   Vector transLocal   = readBVector3();
   Vector rotQuatLocal = readBVector4();

   log_debug( "  pos = %.4f,%.4f,%.4f\n", trans[0], trans[1], trans[2] );
   log_debug( "  rot = %.4f,%.4f,%.4f,%.4f\n",
         rotVec[0], rotVec[1], rotVec[2], rotVec[3] );
   log_debug( "  lpos = %.4f,%.4f,%.4f\n", transLocal[0], transLocal[1], transLocal[2] );
   log_debug( "  lrot = %.4f,%.4f,%.4f,%.4f\n",
         rotQuatLocal[0], rotQuatLocal[1], rotQuatLocal[2], rotQuatLocal[3] );

   // Parent bone joint
   int parent = readBInt32();
   log_debug( "  parent %d\n", parent );

   // Children... we don't need this data, just skip past it
   int childCount = readBInt32();
   //log_debug( "  children %d\n", childCount );
   for ( int c = 0; c < childCount; c++ )
   {
      readBInt32();
   }

   Quaternion rotQuat( rotVec );
   Vector rot;

   // left-handed to right-handed conversion
   rotQuat = rotQuat.swapHandedness();

   Matrix bmat;
   bmat.setRotationQuaternion( rotQuat );

   bmat.setTranslation( trans );

   // NOTE: This will not work if we're adding a joint with a parent
   // that is defined after us
   if ( parent >= 0 )
   {
      if ( parent < m_model->getBoneJointCount() )
      {
         Matrix pmat;
         m_model->getBoneJointAbsoluteMatrix( parent, pmat );

         bmat = bmat * pmat;
      }
      else
      {
         log_error( "  parent %d does not exist (yet)\n", parent );
      }
   }

   //log_debug( "  pre trans %f,%f,%f\n", 
   //      (float) trans[0], (float) trans[1], (float) trans[2] );

   bmat.getTranslation( trans );
   bmat.getRotation( rot );

   //log_debug( "  post trans %f,%f,%f\n", 
   //      (float) trans[0], (float) trans[1], (float) trans[2] );

   /*
   log_debug( "  rot %f,%f,%f\n", 
   (float) rot[0], (float) rot[1], (float) rot[2] );

   log_debug( "  local trans %f,%f,%f\n", 
   (float) transLocal[0], (float) transLocal[1], (float) transLocal[2] );
   */

   m_model->addBoneJoint( name.c_str(), 
         trans[0], trans[1], trans[2],
         rot[0], rot[1], rot[2], parent );

   return true;
}

bool Cal3dFilter::readBSubMesh()
{
   log_debug( "Reading sub mesh %d\n", m_model->getGroupCount() );

   int vertBase   = m_model->getVertexCount();

   int matId      = readBInt32();
   int numVerts   = readBInt32();
   int numFaces   = readBInt32();
   int numLOD     = readBInt32();
   int numSprings = readBInt32();
   int numMaps    = readBInt32();

   string name = m_modelPartName;

   log_debug( "  name: %s\n", name.c_str() );
   log_debug( "  material: %d\n", matId );
   log_debug( "  verts: %d\n", numVerts );
   log_debug( "  faces: %d\n", numFaces );
   log_debug( "  LOD steps: %d\n", numLOD );
   log_debug( "  springs: %d\n", numSprings );
   log_debug( "  maps: %d\n", numMaps );

   if ( matId >= m_model->getTextureCount() )
   {
      log_error( "  material %d is out of range\n", matId );
   }

   int group = m_model->addGroup( name.c_str() );
   m_model->setGroupTextureId( group, matId );

   UVList uvlist;

   // read vertices
   for ( int v = 0; v < numVerts; v++ )
   {
      Vector pos  = readBVector3();
      readBVector3();  // normal, ignore it

      // LOD stuff, we ignore it
      readBInt32();  // vertex to collapse to
      readBInt32();  // number of faces collapsed

      // Add the vertex itself
      int vert = m_model->addVertex( pos[0], pos[1], pos[2] );

      // Read UV
      for ( int m = 0; m < numMaps; m++ )
      {
         float u = readBFloat();
         float v = readBFloat();

         // ignore unless it's the first one
         if ( m == 0 )
         {
            // our vertices are per face, not per vertex
            // save this for faces section
            UVDataT uv;
            uv.u = u;
            uv.v = v;
            uvlist.push_back( uv );
         }
      }

      // Read influences
      int numBones = readBInt32();
      for ( int b = 0; b < numBones; b++ )
      {
         int   boneId = readBInt32();
         float boneWeight = readBFloat();

         if ( boneId < m_model->getBoneJointCount() )
         {
            m_model->addVertexInfluence( vert, boneId, Model::IT_Custom, boneWeight );
         }
         else
         {
            log_warning( "  bone joint %d for vertex %d is out of range\n",
                  boneId, vert );
         }
      }

      if ( numSprings )
      {
         // spring weight, only present if springs, ignore it
         readBFloat();
      }
   }

   // read (and ignore) springs
   for ( int s = 0; s < numSprings; s++ )
   {
      readBInt32(); // vertex 1
      readBInt32(); // vertex 2
      readBFloat(); // spring coefficient
      readBFloat(); // length of spring at rest (idle)
   }

   int vcount  = m_model->getVertexCount();
   int uvcount = uvlist.size();

   // read faces
   for ( int f = 0; f < numFaces; f++ )
   {
      int v1 = readBInt32();
      int v2 = readBInt32();
      int v3 = readBInt32();

      if ( v1 < vcount && v2 < vcount && v3 < vcount )
      {
         int tri = m_model->addTriangle( 
               v1 + vertBase, v2 + vertBase, v3 + vertBase );
         m_model->addTriangleToGroup( group, tri );

         if ( v1 < uvcount
               && v2 < uvcount
               && v3 < uvcount )
         {
            m_model->setTextureCoords(
                  tri, 0, uvlist[v1].u, uvlist[v1].v );
            m_model->setTextureCoords(
                  tri, 1, uvlist[v2].u, uvlist[v2].v );
            m_model->setTextureCoords(
                  tri, 2, uvlist[v3].u, uvlist[v3].v );
         }
         else
         {
            if ( uvcount > 0 )
            {
               log_warning( "face vertex (%d,%d,%d) not in uvlist %d\n",
                     v1, v2, v3, uvcount );
            }
         }
      }
      else
      {
         log_error( "a vertex is out of range (%d,%d,%d) > %d\n",
               v1, v2, v3, vcount );
      }
   }

   return true;
}

bool Cal3dFilter::readBAnimTrack()
{
   //log_debug( "Reading animation track\n" );

   int bone      = readBInt32();
   int numFrames = readBInt32();

   //log_debug( "  bone: %d\n", bone );
   //log_debug( "  frames: %d\n", numFrames );

   for ( int f = 0; f < numFrames; f++ )
   {
      float tsec = readBFloat(); // time in seconds
      Vector trans  = readBVector3();
      Vector rotVec = readBVector4();

      Quaternion quat( rotVec );

      quat = quat.swapHandedness();

      Matrix m;

      m.setRotationQuaternion( quat );
      m.setTranslation( trans );

      // Cal3D anims are relative to joint parent
      // MM3D are relative to joint local position, convert
      Matrix inv;
      m_model->getBoneJointRelativeMatrix(  bone, inv );
      inv = inv.getInverse();

      m = m * inv;

      double rot[3] = { 0, 0, 0 };
      m.getRotation( rot );
      m.getTranslation( trans );

      int animFrame = timeToFrame( tsec, 30.0 );

      m_model->setSkelAnimKeyframe( m_anim, animFrame, bone,
            true, rot[0], rot[1], rot[2] );
      m_model->setSkelAnimKeyframe( m_anim, animFrame, bone,
            false, trans[0], trans[1], trans[2] );
   }

   return true;
}

bool Cal3dFilter::readBCompressedAnimTrack( double duration )
{
   log_debug( "Reading animation track (compressed)\n" );

   int bone      = readBInt32();
   int numFrames = readBInt32();

   double scale[3] = { 1.0, 1.0, 1.0 };
   double trans_min[3] = { 0.0, 0.0, 0.0 };
   int comp_bits[3] = { 10, 11, 11 };

   trans_min[0] = readBFloat();
   trans_min[1] = readBFloat();
   trans_min[2] = readBFloat();
   scale[0] = readBFloat();
   scale[1] = readBFloat();
   scale[2] = readBFloat();

   //log_debug( "  bone: %d\n", bone );
   //log_debug( "  frames: %d\n", numFrames );

   for ( int f = 0; f < numFrames; f++ )
   {
      uint16_t qsec = readBInt16();
      float tsec = (qsec / 65535.0) * duration;

      uint32_t trans_comp = readBInt32();
      uint64_t rot_comp = readBUInt48();

      log_debug( "compressed trans = %X  rot = %llX\n",
         trans_comp, rot_comp );

      Vector trans;
      Vector rotVec;

      for ( int i = 0; i < 3; ++i )
      {
         trans[i] = ((2 << comp_bits[i]) - 1) & trans_comp;
         trans_comp = trans_comp >> comp_bits[i];
         trans[i] /= (double) ((2 << comp_bits[i]) - 1);
         trans[i] *= scale[i];
      }

      for ( int i = 0; i < 4; ++i )
      {
         rotVec[i] = ((2 << 12) - 1) & rot_comp;
         rotVec[i] /= (double) ((2 << 12) - 1);
         rotVec[i] *= 2.0 - 1.0;
      }

      log_debug( "uncompressed quat = %f, %f, %f, %f  \n",
            (float) rotVec[0], (float) rotVec[1], (float) rotVec[2], (float) rotVec[3] );

      Quaternion quat( rotVec );

      quat = quat.swapHandedness();

      Matrix m;

      m.setRotationQuaternion( quat );
      m.setTranslation( trans );

      // Cal3D anims are relative to joint parent
      // MM3D are relative to joint local position, convert
      Matrix inv;
      m_model->getBoneJointRelativeMatrix(  bone, inv );
      inv = inv.getInverse();

      m = m * inv;

      double rot[3] = { 0, 0, 0 };
      m.getRotation( rot );
      m.getTranslation( trans );

      int animFrame = timeToFrame( tsec, 30.0 );

      log_debug( "rotation (%f,%f,%f)  translation (%f,%f,%f)\n",
            rot[0], rot[1], rot[2], trans[0], trans[1], trans[2] );

      m_model->setSkelAnimKeyframe( m_anim, animFrame, bone,
            true, rot[0], rot[1], rot[2] );
      m_model->setSkelAnimKeyframe( m_anim, animFrame, bone,
            false, trans[0], trans[1], trans[2] );
   }

   return true;
}


uint64_t Cal3dFilter::readBUInt48()
{
   uint32_t rval32 = readBInt32();
   uint16_t rval16 = readBInt16();
   uint64_t rval = ((uint64_t) rval16 << 32) | ((uint64_t) rval32);
   return rval;
}

int32_t Cal3dFilter::readBInt32()
{
   int32_t rval = *((int32_t *) m_bufPos);
   m_bufPos += sizeof( rval );
   rval = ltoh_u32( rval );
   return rval;
}

int16_t Cal3dFilter::readBInt16()
{
   int16_t rval = *((int16_t *) m_bufPos);
   m_bufPos += sizeof( rval );
   rval = ltoh_u16( rval );
   return rval;
}

uint8_t Cal3dFilter::readBUInt8()
{
   uint8_t rval = *((uint8_t *) m_bufPos);
   m_bufPos += sizeof( rval );
   return rval;
}

float Cal3dFilter::readBFloat()
{
   float32_t rval = 0.0f;
   memcpy( &rval, m_bufPos, sizeof(rval ) );
   m_bufPos += sizeof( rval );
   rval = ltoh_float( rval );
   return rval;
}

bool Cal3dFilter::readBString( std::string & str )
{
   size_t len = readBInt32();
   size_t advance = len;

   while (len > 0 && m_bufPos[len-1] == '\0')
      --len;
   str.assign( (char *) m_bufPos, len );
   //log_debug( "read string '%s' at %p\n", str.c_str(), m_bufPos );
   m_bufPos += advance;

   return true;
}

bool Cal3dFilter::readBLine( std::string & str, size_t maxLen )
{
   size_t p = 0;
   size_t skipBytes = 0;

   for ( p = 0; p < maxLen; p++ )
   {
      if ( m_bufPos[p] == '\n' )
      {
         skipBytes = 1;
         break;
      }
      if ( m_bufPos[p] == '\r' && m_bufPos[p+1] == '\n' )
      {
         skipBytes = 2;
         break;
      }
   }

   str.assign( (char *) m_bufPos, p );
   m_bufPos += p + skipBytes;

   //log_debug( "size: %d, pos %d\n", maxLen, m_bufPos - m_fileBuf );

   if ( (p + skipBytes) > 0 )
   {
      return true;
   }
   else
   {
      return false;
   }
}

Vector Cal3dFilter::readBVector3()
{
   Vector rval;
   rval[0] = readBFloat();
   rval[1] = readBFloat();
   rval[2] = readBFloat();
   return rval;
}

Vector Cal3dFilter::readBVector4()
{
   Vector rval;
   rval[0] = readBFloat();
   rval[1] = readBFloat();
   rval[2] = readBFloat();
   rval[3] = readBFloat();
   return rval;
}

Vector Cal3dFilter::readAVector3( const char * str )
{
   float fval[3];
   sscanf( str, "%f %f %f", &fval[0], &fval[1], &fval[2] );
   Vector rval;
   rval[0] = fval[0];
   rval[1] = fval[1];
   rval[2] = fval[2];
   return rval;
}

Vector Cal3dFilter::readAVector4( const char * str )
{
   float fval[4];
   sscanf( str, "%f %f %f %f", &fval[0], &fval[1], &fval[2], &fval[3] );
   Vector rval;
   rval[0] = fval[0];
   rval[1] = fval[1];
   rval[2] = fval[2];
   rval[3] = fval[3];
   return rval;
}

std::string Cal3dFilter::readAString( const char * str )
{
   while ( isspace( str[0] ) )
   {
      str++;
   }
   int len = strlen( str ) - 1;
   while ( len >= 0 && isspace( str[len] ) )
   {
      len--;
   }
   len++;

   std::string rval;
   rval.assign( str, len );
   return rval;
}

std::string Cal3dFilter::readLineFile( const char * str )
{
   const char * eq = strchr( str, '=' );
   if ( eq )
   {
      const char * start = strchr( eq, '"' );
      if ( start )
      {
         start += 1;
         const char * end = strchr( start, '"' );
         if ( end )
         {
            std::string rval;
            rval.assign( start, end - start );
            return rval;
         }
      }
      else
      {
         // No quotes... uh... do... something
         eq++; // skip '='
         while ( isspace( eq[0] ) )
         {
            eq++;
         }
         return eq;
      }
   }

   return "";
}

std::string Cal3dFilter::readLineKey( const char * str )
{
   const char * eq = strchr( str, '=' );
   if ( eq )
   {
      eq--;
      while (eq > str && isspace(*eq))
      {
         eq--;
      }
      std::string rval;
      rval.assign( str, eq - str + 1 );
      return rval;
   }

   return "";
}

//------------------------------------------------------------------
// XML read functions

// XXX: It is important to note that this XML reading functionality is
// extremely limited. It is just barely good enough to read Cal3D
// XML formats, and is not appropriate for any other uses.
//
// Why did I write my own instead of using a library? Primarily because
// I don't want to increase external dependencies (mostly for portability
// reasons).
//
// Why not use Qt's XML reading since MM3D is already using Qt? Because
// the core MM3D model functionality is used in other projects that
// do not link against Qt.
//
// So I wrote 3 XML parsing functions that are extremely limited use,
// but "good enough" for what I need.
//
// Some things that are broken:
//    - No validity checks on input
//    - Can't ensure that an element is a sub-element (just looks at the
//      tags, not the document hierarchy)
//    - Attribute names must not be substrings of other attribute names
//      or attribute values in the same element (if there is you could
//      get a false attribute lookup)
//    - Text in an attribute value that looks like an element will be
//      parsed as an element
//    - It doesn't handle comments (elements in comments will be parsed)

bool Cal3dFilter::findXElement( const char * tag )
{
   const char * buf = (const char *) m_fileBuf;
   size_t pos = m_bufPos - m_fileBuf;
   size_t tagLen = strlen( tag );

   while ( pos < m_fileLength )
   {
      if ( m_fileBuf[pos] == '<' )
      {
         if ( strncasecmp( &buf[pos+1], tag, tagLen ) == 0)
         {
            m_bufPos = &m_fileBuf[pos];
            //log_debug( "found tag %s at %p\n", tag, m_bufPos );
            return true;
         }
      }
      pos++;
   }
   return false;
}

std::string Cal3dFilter::readXAttribute( const char * attr )
{
   const char * buf = (const char *) m_fileBuf;
   size_t pos = m_bufPos - m_fileBuf;
   size_t attrLen = strlen( attr );

   size_t start = 0;
   bool inAttr = false;

   while ( pos < m_fileLength - 1 )
   {
      if ( inAttr )
      {
         if ( buf[pos] == '"' )
         {
            std::string rval;
            rval.assign( &buf[ start ], pos - start );
            //log_debug( "attribute: %s = %s\n", attr, rval.c_str() );
            return rval;
         }
      }
      else
      {
         if ( strncmp( &buf[pos], attr, attrLen ) == 0 )
         {
            pos += attrLen;
            while ( !inAttr && pos < m_fileLength - 1 )
            {
               // NOTE: This assumes that no attribute name occures 
               // as a sub-string of another attribute name or value.
               // It also assumes the '=' is present
               if ( buf[pos] == '"' )
               {
                  start = pos + 1;
                  inAttr = true;

                  // pos will get incremeneted at the end of the
                  // outter loop to skip the quote char
               }
               else
               {
                  pos++;
               }
            }
         }
         else if ( buf[pos] == '>' )
         {
            return "";
         }
      }
      pos++;
   }
   return "";
}

std::string Cal3dFilter::readXElement( const char * tag )
{
   const char * buf = (const char *) m_fileBuf;
   size_t pos = m_bufPos - m_fileBuf;

   std::string endTag = std::string( "/" ) + tag;
   size_t tagLen = endTag.size();

   bool inTag = true;
   size_t start = 0;

   while ( pos < m_fileLength - 1 )
   {
      if ( inTag )
      {
         if ( buf[pos] == '/' && buf[pos+1] == '>' )
         {
            m_bufPos = &m_fileBuf[pos+2];
            return "";
         }
         else if ( buf[pos] == '>' )
         {
            inTag = false;
            start = pos + 1;
         }
      }
      else
      {
         if ( buf[pos] == '<' )
         {
            if ( strncasecmp( &buf[pos+1], endTag.c_str(), tagLen ) == 0)
            {
               m_bufPos = &m_fileBuf[pos];
               //log_debug( "found end of tag %s at %p\n", tag, m_bufPos );

               std::string rval;
               rval.assign( &buf[ start ], pos - start );
               //log_debug( "tag data: %s\n", rval.c_str() );
               return rval;
            }
         }
      }
      pos++;
   }
   return "";
}

//------------------------------------------------------------------
// Format write functions

Model::ModelErrorE Cal3dFilter::writeCal3dFile( const char * filename, Model * model, ModelFilter::Options * o )
{
   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   FunctionCaller<DataDest> fc( m_dst, &DataDest::close );

   if ( err != Model::ERROR_NONE )
      return err;

   std::string base = m_modelPath + "/";

   // Use dynamic cast to determine if the object is of the proper type
   // If not, create new one that we will delete later.
   //
   // We need to create one to make sure that the default options we
   // use in the filter match the default options presented to the
   // user in the dialog box.
   m_options = dynamic_cast< Cal3dOptions *>( o );
   release_ptr<Cal3dOptions> freeOptions = NULL;
   if ( !m_options )
   {
      freeOptions = static_cast<Cal3dOptions *>( getDefaultOptions() );
      m_options = freeOptions.get();
   }

   m_model->updateMetaData( "cal3d_single_mesh_file",
         m_options->m_singleMeshFile ? "1" : "0" );
   m_model->updateMetaData( "cal3d_xml_material",
         m_options->m_xmlMatFile ? "1" : "0" );

   m_dst->writeString( "#\n# cal3d model configuration file\n#\n" );

   m_dst->writeString( "# File written by Misfit Model 3D\n" );
   m_dst->writeString( "# http://www.misfitcode.com/misfitmodel3d/\n\n" );

   m_dst->writeString( "[model]\n" );

   char value[64];
   if ( m_model->getMetaData( "cal3d_path", value, sizeof(value) ) )
   {
      m_dst->writePrintf( "path=\"%s\"\n", value );
   }
   if ( m_model->getMetaData( "cal3d_scale", value, sizeof(value) ) )
   {
      m_dst->writePrintf( "scale=\"%s\"\n", value );
   }
   if ( m_model->getMetaData( "cal3d_rotate", value, sizeof(value) ) )
   {
      m_dst->writePrintf( "rotate=\"%s\"\n", value );
   }

   m_dst->writeString( "\n# --- Skeleton ---\n" );
   std::string skelFile = replaceExtension( m_modelBaseName.c_str(), "csf" );
   m_dst->writePrintf( "skeleton = \"%s\"\n\n", skelFile.c_str() );
   writeSkeletonFile( (base + skelFile).c_str(), model );

   // To write animations:
   //
   // * Make a map of filenames written (case-sensitive)
   // and map of animations written.
   //
   // * Run through meta data and write file if not already
   // in map. 
   //
   // * Run through animations. If any not written, write them
   // and create an animation line in the Cal3D file.

   typedef std::map<std::string, bool> AnimFileMap;
   AnimFileMap fileWritten;
   std::vector<bool> animWritten;

   animWritten.resize( model->getAnimCount( MODE ) );

   m_dst->writePrintf( "# --- Animations ---\n" );
   std::string animFile;

   char keyStr[PATH_MAX];
   char valueStr[PATH_MAX];
   unsigned int mtcount = model->getMetaDataCount();
   for ( unsigned int mt = 0; mt < mtcount; mt++ )
   {
      model->getMetaData( mt, keyStr, PATH_MAX, valueStr, PATH_MAX );
      if ( strncmp(keyStr, "animation_", 10 ) == 0 )
      {
         animFile = valueStr;
         std::string animName = removeExtension( valueStr );
         int a = findAnimation( animName );
         if ( a >= 0 && fileWritten.find(animFile) == fileWritten.end() )
         {
            writeAnimationFile( (base + animFile).c_str(), model, a );
            animWritten[a] = true;
            fileWritten[animFile] = true;
         }
         m_dst->writePrintf( "%s = \"%s\"\n", keyStr, animFile.c_str() );
      }
   }

   unsigned int acount = model->getAnimCount( MODE );
   for ( unsigned int a = 0; a < acount; a++ )
   {
      if ( animWritten[a] == 0 )
      {
         const char * animName = model->getAnimName( MODE, a );
         animFile = animName;
         animFile = addExtension( animFile, "caf" );
         m_dst->writePrintf( "animation_%s = \"%s\"\n", animName, animFile.c_str() );
         writeAnimationFile( (base + animFile).c_str(), model, a );
         animWritten[a] = 1;
      }
   }
   m_dst->writeString( "\n" );

   // Create meshes split on groups, with UVs and 
   // normals unique to each vertex
   MeshList ml;
   mesh_create_list( ml, model );

   m_dst->writeString( "# --- Meshes ---\n" );
   if ( (m_options == NULL) || m_options->m_singleMeshFile )
   {
      std::string meshFile = replaceExtension( m_modelBaseName.c_str(), "cmf" );
      m_dst->writePrintf( "mesh_file = \"%s\"\n", meshFile.c_str() );
      writeMeshListFile( (base + meshFile).c_str(), model, ml );
   }
   else
   {
      std::string meshName;
      std::string meshFile;

      unsigned int meshCount = ml.size();
      unsigned int meshNum;

      typedef std::map<string, MeshList> FileMeshMap;
      FileMeshMap fmm;

      string groupName;
      for ( meshNum = 0; meshNum < meshCount; meshNum++ )
      {
         groupName = m_model->getGroupName(ml[meshNum].group);
         _strtolower( groupName );
         fmm[ groupName ].push_back( ml[meshNum] );
      }

      // Write the meshes for each group
      for ( FileMeshMap::const_iterator fmm_it = fmm.begin();
            fmm_it != fmm.end(); fmm_it++ )
      {
         // Get a count of how many meshes make up this group
         // (probably 1, but you never know)
         string groupName = fmm_it->first;
         const MeshList & groupMeshList = fmm_it->second;

         // Create mesh name and filename strings
         string meshName  = groupName;
         string meshFile  = groupName + ".cmf";

         _escapeCal3dName(meshName);
         _escapeFileName(meshFile);

         // Write mesh file
         m_dst->writePrintf( "mesh_%s = \"%s\"\n", meshName.c_str(), meshFile.c_str() );
         writeMeshListFile( (base + meshFile).c_str(), model, groupMeshList );
      }
   }

   m_dst->writeString( "\n# --- Materials ---\n" );
   std::string matFile;
   unsigned int mcount = model->getTextureCount();
   for ( unsigned int m = 0; m < mcount; m++ )
   {
      const char * matName = model->getTextureName( m );
      matFile = matName;
      if ( m_options && !m_options->m_xmlMatFile )
      {
         matFile += ".crf";
         writeMaterialFile( (base + matFile).c_str(), model, m );
      }
      else 
      {
         matFile += ".xrf";
         writeXMaterialFile( (base + matFile).c_str(), model, m );
      }
      m_dst->writePrintf( "material_%s = \"%s\"\n", matName, matFile.c_str() );
   }
   m_dst->writeString( "\n" );

   m_options = NULL;

   return err;
}

Model::ModelErrorE Cal3dFilter::writeXMaterialFile( const char * filename, Model * model, unsigned int materialId )
{
   DataDest * oldDst = m_dst;

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   FunctionCaller<DataDest> fc( m_dst, &DataDest::close );

   if ( err != Model::ERROR_NONE )
      return err;

   // Header
   // FIXME test header-less XML file
   int version = CAL3D_MIN_XVERSION;
   char versionStr[32];
   if ( model->getMetaData( "cal3d_xrf_version", versionStr, sizeof(versionStr) ) )
   {
      version = atoi(versionStr);
   }

   if ( version < CAL3D_NO_XHEADER_VERSION )
      m_dst->writePrintf( "<HEADER MAGIC=\"XRF\" VERSION=\"%d\" />\n", version );

   Model::Material::MaterialTypeE type = model->getMaterialType( materialId );

   // Start material element
   m_dst->writePrintf( "<MATERIAL NUMMAPS=\"%d\"", 
         ( type == Model::Material::MATTYPE_TEXTURE ) ? 1 : 0 );
   if ( version >= CAL3D_NO_XHEADER_VERSION )
      m_dst->writePrintf( " VERSION=\"%d\"", version );
   m_dst->writePrintf( ">\n" );

   // Material lighting properties
   float fval[4];

   model->getTextureAmbient( materialId, fval );
   writeXColor( "AMBIENT", fval );
   model->getTextureDiffuse( materialId, fval );
   writeXColor( "DIFFUSE", fval );
   model->getTextureSpecular( materialId, fval );
   writeXColor( "SPECULAR", fval );

   model->getTextureShininess( materialId, fval[0] );
   m_dst->writePrintf( "    <SHININESS>%f</SHININESS>\n", fval[0] );

   // Texture map
   if ( type == Model::Material::MATTYPE_TEXTURE )
   {
      std::string textureFile = model->getTextureFilename( materialId );

      std::string fullName;
      std::string fullPath;
      std::string baseName;
      normalizePath( filename, fullName, fullPath, baseName );

      std::string relativeFile = getRelativePath( 
            fullPath.c_str(), textureFile.c_str() );

      m_dst->writePrintf( "    <MAP>%s</MAP>\n", relativeFile.c_str() );
   }

   // close material element
   m_dst->writePrintf( "</MATERIAL>\n" );

   m_dst = oldDst;
   return err;
}

Model::ModelErrorE Cal3dFilter::writeSkeletonFile( const char * filename, Model * model )
{
   DataDest * oldDst = m_dst;

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   FunctionCaller<DataDest> fc( m_dst, &DataDest::close );

   if ( err != Model::ERROR_NONE )
      return err;

   unsigned int bcount = model->getBoneJointCount();

   int32_t version = CAL3D_MIN_BVERSION;
   char versionStr[32];
   if ( model->getMetaData( "cal3d_binary_version", versionStr, sizeof(versionStr) ) )
   {
      version = atoi(versionStr);
   }

   // Header
   m_dst->writeBytes( (uint8_t *) CAL3D_MAGIC_SKELETON, CAL3D_MAGIC_SIZE );
   m_dst->write( version );
   m_dst->write( (int32_t) bcount );

   for ( unsigned int b = 0; b < bcount; b++ )
   {
      writeBBone( b );
   }

   m_dst = oldDst;
   return err;
}

Model::ModelErrorE Cal3dFilter::writeMeshFile( const char * filename, Model * model )
{
   // Create a list of all meshes
   MeshList ml;
   mesh_create_list( ml, model );

   return writeMeshListFile( filename, model, ml );
}

Model::ModelErrorE Cal3dFilter::writeMeshListFile( const char * filename, Model * model, const MeshList & meshList )
{
   DataDest * oldDst = m_dst;

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   FunctionCaller<DataDest> fc( m_dst, &DataDest::close );

   if ( err != Model::ERROR_NONE )
      return err;

   int32_t version = CAL3D_MIN_BVERSION;
   char versionStr[32];
   if ( model->getMetaData( "cal3d_binary_version", versionStr, sizeof(versionStr) ) )
   {
      version = atoi(versionStr);
   }

   // Header
   m_dst->writeBytes( (uint8_t *) CAL3D_MAGIC_MESH, CAL3D_MAGIC_SIZE );
   m_dst->write( version );
   m_dst->write( (int32_t) meshList.size() );

   // Save mesh to file
   log_debug( "writing mesh file %s\n", filename );
   MeshList::const_iterator mit;
   for ( mit = meshList.begin(); mit != meshList.end(); mit++ )
   {
      log_debug( " writing a mesh\n" );
      writeBMesh( *mit );
   }

   m_dst = oldDst;
   return err;
}

Model::ModelErrorE Cal3dFilter::writeMaterialFile( const char * filename, Model * model, unsigned int materialId )
{
   DataDest * oldDst = m_dst;

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   FunctionCaller<DataDest> fc( m_dst, &DataDest::close );

   if ( err != Model::ERROR_NONE )
      return err;

   // Header
   m_dst->writeBytes( (uint8_t *) CAL3D_MAGIC_MATERIAL, CAL3D_MAGIC_SIZE );

   int32_t version = CAL3D_MIN_BVERSION;
   char versionStr[32];
   if ( model->getMetaData( "cal3d_binary_version", versionStr, sizeof(versionStr) ) )
   {
      version = atoi(versionStr);
   }

   m_dst->write( version );

   // Material lighting properties
   float fval[4];

   model->getTextureAmbient( materialId, fval );
   writeBColor( fval );
   model->getTextureDiffuse( materialId, fval );
   writeBColor( fval );
   model->getTextureSpecular( materialId, fval );
   writeBColor( fval );
   model->getTextureShininess( materialId, fval[0] );
   m_dst->write( (float32_t) fval[0] );

   // Texture map
   if ( model->getMaterialType( materialId ) == Model::Material::MATTYPE_TEXTURE )
   {
      m_dst->write( (int32_t) 1 );

      std::string textureFile = model->getTextureFilename( materialId );

      std::string fullName;
      std::string fullPath;
      std::string baseName;
      normalizePath( filename, fullName, fullPath, baseName );

      std::string relativeFile = getRelativePath( 
            fullPath.c_str(), textureFile.c_str() );

      writeBString( relativeFile );
   }
   else
   {
      m_dst->write( (int32_t) 0 );
   }

   m_dst = oldDst;
   return err;
}

Model::ModelErrorE Cal3dFilter::writeAnimationFile( const char * filename, Model * model, unsigned int animationId )
{
   DataDest * oldDst = m_dst;

   Model::ModelErrorE err = Model::ERROR_NONE;
   m_dst = openOutput( filename, err );
   FunctionCaller<DataDest> fc( m_dst, &DataDest::close );

   if ( err != Model::ERROR_NONE )
      return err;

   // get anim header info
   float fps = model->getAnimFPS( MODE, animationId );
   unsigned int frameCount = model->getAnimFrameCount( MODE, animationId );
   float32_t duration = (double) frameCount / fps;
   if ( frameCount > 0 )
   {
      duration = ((double) frameCount - 1) / fps;
   }

   // build track list
   std::list<int> tracks;
   bool found = false;

   double kf[3];

   unsigned int bcount = model->getBoneJointCount();
   for ( unsigned int b = 0; b < bcount; b++ )
   {
      found = false;
      for ( unsigned int f = 0; !found && f < frameCount; f++ )
      {
         if ( model->getSkelAnimKeyframe( animationId, f, b, false, kf[0], kf[1], kf[2] )
               || model->getSkelAnimKeyframe( animationId, f, b, true, kf[0], kf[1], kf[2] ) )
         {
            found = true;
            tracks.push_back( b );
         }
      }
   }

   int32_t version = CAL3D_MIN_BVERSION;
   char versionStr[32];
   if ( model->getMetaData( "cal3d_binary_version", versionStr, sizeof(versionStr) ) )
   {
      version = atoi(versionStr);
   }

   // write header
   m_dst->writeBytes( (uint8_t *) CAL3D_MAGIC_ANIMATION, CAL3D_MAGIC_SIZE );
   m_dst->write( version );
   m_dst->write( duration );
   m_dst->write( (int32_t) tracks.size() );
   if ( version >= CAL3D_COMP_ANIM_VERSION )
   {
      // Flags, no compression
      m_dst->write( (int32_t) 0 );
   }

   // write tracks
   std::list<int>::iterator it;
   for ( it = tracks.begin(); it != tracks.end(); it++ )
   {
      writeBAnimTrack( animationId, *it );
   }

   m_dst = oldDst;
   return err;
}

void Cal3dFilter::writeBBone( unsigned int b )
{
   std::string name = m_model->getBoneJointName( b );
   writeBString( name );

   Matrix m;
   m_model->getBoneJointAbsoluteMatrix( b, m );

   Matrix pinv;
   int parent = m_model->getBoneJointParent( b );
   if ( parent >= 0 )
   {
      m_model->getBoneJointAbsoluteMatrix( parent, pinv );
      pinv = pinv.getInverse();
   }

   Matrix lm;
   lm = m * pinv;

   Vector trans;
   Quaternion rot;
   lm.getTranslation( trans );
   lm.getRotationQuaternion( rot );

   rot = rot.swapHandedness();

   writeBVector3( trans );  // relative to parent
   writeBQuaternion( rot );

   m = m.getInverse();
   m.getTranslation( trans );
   m.getRotationQuaternion( rot );

   rot = rot.swapHandedness();

   writeBVector3( trans );  // model space
   writeBQuaternion( rot );

   // write parent
   m_dst->write( (int32_t) parent );

   // find children
   std::list<int> children;
   unsigned bcount = m_model->getBoneJointCount();
   for ( unsigned int child = 0; child < bcount; child++ )
   {
      int cparent = m_model->getBoneJointParent( child );

      if ( (unsigned int) cparent == b )
      {
         children.push_back( child );
      }
   }

   // write children
   m_dst->write( (int32_t) children.size() );
   std::list<int>::iterator it;
   for ( it = children.begin(); it != children.end(); it++ )
   {
      m_dst->write( (int32_t) *it );
   }
}

void Cal3dFilter::writeBAnimTrack( unsigned int anim, unsigned int bone )
{
   float fps = m_model->getAnimFPS( MODE, anim );
   unsigned int frameCount = m_model->getAnimFrameCount( MODE, anim );

   std::list<int> frames;

   double kf[3];

   for ( unsigned int f = 0; f < frameCount; f++ )
   {
      if ( m_model->getSkelAnimKeyframe( anim, f, bone, false, kf[0], kf[1], kf[2] )
            || m_model->getSkelAnimKeyframe( anim, f, bone, true, kf[0], kf[1], kf[2] ) )
      {
         frames.push_back( f );
      }
   }

   // write track info
   m_dst->write( (int32_t) bone );
   m_dst->write( (int32_t) frames.size() );

   // write keyframe data
   std::list<int>::iterator it;
   for ( it = frames.begin(); it != frames.end(); it++ )
   {
      double frameTime = ((double) (*it)) / fps;
      m_dst->write( (float32_t) frameTime );

      // MM3D allows one type of keyframe without the other, Cal3D requires
      // both rotation and translation for each keyframe. If we have one
      // and the other is missing, we must do interpolation here.
      Matrix m;
      m_model->interpSkelAnimKeyframeTime( anim, frameTime, true, bone, m );

      Matrix rm;
      m_model->getBoneJointRelativeMatrix( bone, rm );

      m = m * rm;

      Vector trans;
      Quaternion rot;

      m.getRotationQuaternion( rot );
      m.getTranslation( trans );

      rot = rot.swapHandedness();

      writeBVector3( trans );
      writeBQuaternion( rot );
   }
}

void Cal3dFilter::writeBMesh( const Mesh & mesh )
{
   int materialId = -1;
   if ( mesh.group >= 0 )
   {
      materialId = m_model->getGroupTextureId( mesh.group );
   }

   m_dst->write( (int32_t) materialId );
   m_dst->write( (int32_t) mesh.vertices.size() );
   m_dst->write( (int32_t) mesh.faces.size() );
   m_dst->write( (int32_t) 0 );
   m_dst->write( (int32_t) 0 );
   if ( materialId >= 0 )
   {
      m_dst->write( (int32_t) 1 );
   }
   else
   {
      m_dst->write( (int32_t) 0 );
   }

   double coord[3];
   Mesh::VertexList::const_iterator vit;
   for ( vit = mesh.vertices.begin(); vit != mesh.vertices.end(); vit++ )
   {
      const Mesh::Vertex & mv = *vit;

      m_model->getVertexCoordsUnanimated( mv.v, coord );
      m_dst->write( (float32_t) coord[0] );
      m_dst->write( (float32_t) coord[1] );
      m_dst->write( (float32_t) coord[2] );

      m_dst->write( (float32_t) mv.norm[0] );
      m_dst->write( (float32_t) mv.norm[1] );
      m_dst->write( (float32_t) mv.norm[2] );

      m_dst->write( (int32_t) -1 );
      m_dst->write( (int32_t) 0 );

      if ( materialId >= 0 )
      {
         m_dst->write( (float32_t) mv.uv[0] );
         m_dst->write( (float32_t) mv.uv[1] );
      }

      Model::InfluenceList il;
      Model::InfluenceList::iterator it;
      m_model->getVertexInfluences( mv.v, il );

      // Our weights don't always equal 100%, get total weigth so we can normalize
      double total = 0.0;
      for ( it = il.begin(); it != il.end(); it++ )
      {
         total += it->m_weight;
      }

      // Don't allow negative weights, or divide by zero
      if ( total < 0.0005 )
      {
         total = 1.0;
      }

      // Write out influence list
      m_dst->write( (int32_t) il.size() ); // number of influences

      for ( it = il.begin(); it != il.end(); it++ )
      {
         m_dst->write( (int32_t) it->m_boneId );          // bone
         m_dst->write( (float32_t) (it->m_weight / total) );  // normalized weight
      }

      // No springs, don't need to write weight
   }

   // Write springs... we don't have any... don't write anything

   // Write faces
   Mesh::FaceList::const_iterator fit;
   for ( fit = mesh.faces.begin(); fit != mesh.faces.end(); fit++ )
   {
      const Mesh::Face & mf = *fit;

      m_dst->write( (int32_t) mf.v[0] );
      m_dst->write( (int32_t) mf.v[1] );
      m_dst->write( (int32_t) mf.v[2] );
   }
}

//------------------------------------------------------------------
// Binary write functions

void Cal3dFilter::writeBVector3( const Vector & vec )
{
   m_dst->write( (float32_t) vec[0] );
   m_dst->write( (float32_t) vec[1] );
   m_dst->write( (float32_t) vec[2] );
}

void Cal3dFilter::writeBQuaternion( const Quaternion & quat )
{
   m_dst->write( (float32_t) quat[0] );
   m_dst->write( (float32_t) quat[1] );
   m_dst->write( (float32_t) quat[2] );
   m_dst->write( (float32_t) quat[3] );
}

void Cal3dFilter::writeBString( const std::string & str )
{
   // Include NULL byte in write
   uint32_t len = str.size() + 1;
   m_dst->write( len );
   m_dst->writeBytes( (uint8_t *) str.c_str(), len );
}

void Cal3dFilter::writeBColor( const float * fval )
{
   uint32_t dval;
   for ( int i = 0; i < 4; i++ )
   {
      dval = (uint32_t) (fval[i] * 255.0);
      if ( dval > 255 )
      {
         dval = 255;
      }
      m_dst->write( (uint8_t) dval );
   }
}

void Cal3dFilter::writeXColor( const char * tag, const float * fval )
{
   m_dst->writePrintf( "    <%s>", tag );
   uint32_t dval;
   for ( int i = 0; i < 4; i++ )
   {
      dval = (uint32_t) (fval[i] * 255.0);
      if ( dval > 255 )
      {
         dval = 255;
      }
      m_dst->writePrintf( "%d%s", dval,
            (i < 3) ? " " : "" );
   }
   m_dst->writePrintf( "</%s>\n", tag );
}

int Cal3dFilter::timeToFrame( double tsec, double fps )
{
   int frame = 0;
   while ( (frame / fps) < (tsec - 0.0005f) )
   {
      frame++;
   }
   //log_debug( "  time %f is frame %d\n", tsec, frame );
   return frame;
}

int Cal3dFilter::findAnimation( const std::string& animName )
{
   int acount = m_model->getAnimCount( MODE );
   for ( int a = 0; a < acount; a++ )
   {
      if ( strcasecmp( animName.c_str(),
               m_model->getAnimName( MODE, a ) ) == 0 )
      {
         return a;
      }
   }
   return -1;
}

#ifdef PLUGIN

//------------------------------------------------------------------
// Plugin functions
//------------------------------------------------------------------

extern "C" bool plugin_init()
{
   if ( s_filter == NULL )
   {
      s_filter = new Cal3dFilter();
      FilterManager * texmgr = FilterManager::getInstance();
      texmgr->registerFilter( s_filter );
   }
   log_debug( "CAL3D model filter plugin initialized\n" );
   return true;
}

// The filter manager will delete our registered filter.
// We have no other cleanup to do
extern "C" bool plugin_uninit()
{
   s_filter = NULL; // FilterManager deletes filters
   log_debug( "CAL3D model filter plugin uninitialized\n" );
   return true;
}

extern "C" const char * plugin_version()
{
   return "0.1.0";
}

extern "C" const char * plugin_desc()
{
   return "CAL3D model filter";
}

#endif // PLUGIN
