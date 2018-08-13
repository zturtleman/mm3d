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


#include "lwofilter.h"

#include "model.h"
#include "texture.h"
#include "log.h"
#include "endianconfig.h"
#include "binutil.h"
#include "misc.h"
#include "filtermgr.h"
#include "mm3dport.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <vector>

using std::list;
using std::string;

#ifdef PLUGIN
static LwoFilter * s_filter = NULL;
#endif // PLUGIN

LwoFilter::LwoFilter()
{
}

LwoFilter::~LwoFilter()
{
}

Model::ModelErrorE LwoFilter::readFile( Model * model, const char * const filename )
{
   Model::ModelErrorE err = Model::ERROR_NONE;
   m_src = openInput( filename, err );
   SourceCloser fc( m_src );

   if ( err != Model::ERROR_NONE )
      return err;

   m_src->setEndianness( DataSource::BigEndian );

   m_modelPath = "";
   m_modelBaseName = "";
   m_modelFullName = "";

   normalizePath( filename, m_modelFullName, m_modelPath, m_modelBaseName );

   model->setFilename( m_modelFullName.c_str() );

   m_model = model;

   uint32_t chunkLen = 0;

   char id[5];
   id[4] = '\0';

   m_vertices =  0;
   m_faces    =  0;
   m_groups   =  0;
   m_curGroup = -1;

   m_surfacePolys.clear();
   m_surfaceTags.clear();
   m_tags.clear();
   m_polyMaps.clear();
   m_clips.clear();
   m_smoothAngles.clear();
   m_vertexMaps.clear();
   m_discVertexMaps.clear();

   m_lastVertexBase = 0;
   m_lastPolyBase = 0;
   m_isLWO2 = false;

   readID( id );
   if ( strncmp( id, "FORM", 4) == 0 )
   {
      chunkLen = readU4();
      if ( chunkLen == (m_src->getFileSize() - 8) )
      {
         readID( id );
         if ( strncmp( id, "LWO2", 4 ) == 0 )
         {
            m_isLWO2 = true;
         }

         if ( m_isLWO2 || strncmp( id, "LWOB", 4 ) == 0 )
         {

            while ( (m_src->offset() + 8) <= (unsigned)m_src->getFileSize() && !m_src->unexpectedEof() )
            {
               readID( id );
               chunkLen = readU4();

               if ( strncmp( id, "PNTS", 4 ) == 0 )
               {
                  log_debug( "vertex chunk is %d bytes\n", chunkLen );
                  readVertexChunk( chunkLen );
               }
               else if ( strncmp( id, "POLS", 4 ) == 0 )
               {
                  if ( m_isLWO2 )
                  {
                     readID( id );
                     chunkLen -= 4;
                  }

                  if ( !m_isLWO2 || (m_isLWO2 && strncmp( id, "FACE", 4 ) == 0) )
                  {
                     log_debug( "polygon chunk is %d bytes\n", chunkLen );
                     readPolygonChunk( chunkLen );
                  }
                  else
                  {
                     log_debug( "ignoring POLS chunk %s of %d bytes\n", id, chunkLen );
                     m_src->seek( m_src->offset() + chunkLen );
                  }
               }
               else if ( strncmp( id, "SRFS", 4 ) == 0 )
               {
                  log_debug( "surface list chunk is %d bytes\n", chunkLen );
                  readSurfaceListChunk( chunkLen );
               }
               else if ( strncmp( id, "SURF", 4 ) == 0 )
               {
                  log_debug( "surface definition chunk is %d bytes\n", chunkLen );
                  readSurfaceDefinitionChunk( chunkLen );
               }
               else if ( m_isLWO2 && strncmp( id, "VMAP", 4 ) == 0 )
               {
                  log_debug( "vertex map chunk is %d bytes\n", chunkLen );
                  readVertexMapChunk( chunkLen );
               }
               else if ( m_isLWO2 && strncmp( id, "TAGS", 4 ) == 0 )
               {
                  log_debug( "tag chunk is %d bytes\n", chunkLen );
                  readTagChunk( chunkLen );
               }
               else if ( m_isLWO2 && strncmp( id, "PTAG", 4 ) == 0 )
               {
                  log_debug( "poly tag chunk is %d bytes\n", chunkLen );
                  readPolyTagChunk( chunkLen );
               }
               else if ( m_isLWO2 && strncmp( id, "CLIP", 4 ) == 0 )
               {
                  log_debug( "clip chunk is %d bytes\n", chunkLen );
                  readClipChunk( chunkLen );
               }
               else
               {
                  log_debug( "ignoring chunk %s of %d bytes\n", id, chunkLen );
                  m_src->seek( m_src->offset() + chunkLen );
               }
            }
         }
         else
         {
            err = Model::ERROR_UNSUPPORTED_VERSION;
         }
      }
      else
      {
         err = Model::ERROR_BAD_DATA;
      }
   }
   else
   {
      err = Model::ERROR_BAD_MAGIC;
   }

   log_debug( "read %d vertices, %d faces, %d groups\n", m_vertices, m_faces, m_groups );

   unsigned surf = 0;
   unsigned poly = 0;
   unsigned materialId = 0;
   for ( surf = 0; surf < m_surfacePolys.size(); surf++ )
   {
      bool addedGroup = true;
      PolyList * l = NULL;

      if ( m_isLWO2 )
      {
         if ( m_surfaceTags[surf] < (unsigned) m_model->getTextureCount() )
         {
            log_debug( "adding group %s for tag %d, surface %d\n", 
                  m_tags[surf].c_str(), surf, materialId );
            m_model->addGroup( m_tags[surf].c_str() );
            m_model->setGroupTextureId( materialId, materialId );
            l = &m_surfacePolys[ surf ];
         }
         else
         {
            addedGroup = false;
         }

         log_debug( "setting smoothness on group %d to %f(%d)\n",
               materialId, m_smoothAngles[surf], surf );
         m_model->setGroupAngle( materialId, (uint8_t) m_smoothAngles[surf] );
      }
      else
      {
         m_model->addGroup( m_model->getTextureName( materialId ) );
         m_model->setGroupTextureId( materialId, materialId );
         l = &m_surfacePolys[ materialId ];

         log_debug( "setting smoothness on group %d to %f(%d)\n",
               materialId, m_smoothAngles[materialId], materialId );
         m_model->setGroupAngle( materialId, (uint8_t) m_smoothAngles[materialId] );
      }

      if ( addedGroup )
      {
         for ( poly = 0; poly < l->size(); poly++ )
         {
            if ( m_isLWO2 )
            {
               unsigned index = m_polyMaps[ (*l)[poly] ].polyIndex;
               unsigned count = m_polyMaps[ (*l)[poly] ].polyCount;

               for ( unsigned p = 0; p < count; p++ )
               {
                  m_model->addTriangleToGroup( materialId, index + p );
               }
            }
            else
            {
               m_model->addTriangleToGroup( materialId, (*l)[poly] );
            }
         }

         materialId++;
      }
   }

   // apply vmap texture coordinates
   {
      unsigned tcount = m_model->getTriangleCount();
      unsigned count = m_vertexMaps.size();
      for ( unsigned t = 0; t < count; t++ )
      {
         for ( unsigned tri = 0; tri < tcount; tri++ )
         {
            for ( unsigned i = 0; i < 3; i++ )
            {
               int vert = m_model->getTriangleVertex( tri, i );
               if ( vert == m_vertexMaps[t].vertex )
               {
                  m_model->setTextureCoords( tri, i,
                        m_vertexMaps[t].s,
                        m_vertexMaps[t].t );
               }
            }
         }
      }
   }

   // apply vmad texture coordinates
   // TODO get sample
#if 0
   {
      unsigned count = m_discVertexMaps.size();
      for ( unsigned t = 0; t < count; t++ )
      {
         DiscVertexMapDataT * dvmd = &m_discVertexMaps[t];
         unsigned polyBase = m_polyMaps[ dvmd->polyIndex ].polyIndex;
         unsigned count    = m_polyMaps[ dvmd->polyIndex ].polyCount;
         // TODO confirm vertex is triangle vertex index, not model vertex index
         if ( dvmd->vertexNumber == 0 )
         {
            for ( unsigned n = 0; n < count; n++ )
            {
               m_model->setTextureCoords( polyBase + n, 0,
                     m_vertexMaps[t].s,
                     m_vertexMaps[t].t );
            }
         }
         else
         {
            // TODO handle vertex 2 & 3
         }
      }
   }
#endif // 0

   // Invert normals
   {
      unsigned count = m_model->getTriangleCount();
      for ( unsigned t = 0; t < count; t++ )
      {
         m_model->invertNormals( t );
      }
   }

   return err;
}

Model::ModelErrorE LwoFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o  )
{
   return Model::ERROR_UNSUPPORTED_OPERATION;
}

bool LwoFilter::canRead( const char * filename )
{
   log_debug( "canRead( %s )\n", filename );
   log_debug( "  true\n" );
   return true;
}

bool LwoFilter::canWrite( const char * filename )
{
   log_debug( "canWrite( %s )\n", filename );
   log_debug( "  false\n" );
   return false;
}

bool LwoFilter::canExport( const char * filename )
{
   log_debug( "canExport( %s )\n", filename );
   log_debug( "  false\n" );
   return false;
}

bool LwoFilter::isSupported( const char * filename )
{
   log_debug( "isSupported( %s )\n", filename );
   unsigned len = strlen( filename );

   if ( len >= 4 && strcasecmp( &filename[len-4], ".lwo" ) == 0 )
   {
      log_debug( "  true\n" );
      return true;
   }
   else
   {
      log_debug( "  false\n" );
      return false;
   }
}

list< string > LwoFilter::getReadTypes()
{
   list<string> rval;
   rval.push_back( "*.lwo" );
   return rval;
}

list< string > LwoFilter::getWriteTypes()
{
   list<string> rval;
   //rval.push_back( "*.lwo" );
   return rval;
}

//------------------------------------------------------------------
// Protected Methods
//------------------------------------------------------------------

bool LwoFilter::readVertexChunk( size_t chunkLen )
{
   m_lastVertexBase = m_model->getVertexCount();

   // TODO Check for invalid chunk size (not divisible by 12)
   unsigned vcount = chunkLen / 12;
   for ( unsigned v = 0; v < vcount; v++ )
   {
      float x = readF4();
      float y = readF4();
      float z = readF4();

      // Invert z coordinate
      z = -z;

      m_model->addVertex( x, y, z );
   }
   m_vertices += vcount;
   return true;
}

bool LwoFilter::readPolygonChunk( size_t chunkLen )
{
   m_lastPolyBase = m_polyMaps.size();

   int remaining = chunkLen;
   while ( remaining > 0 )
   {
      uint16_t vcount = readU2();
      if ( m_isLWO2 )
      {
         vcount = vcount & 0x03FF;
      }
      remaining -= 2;

      uint16_t v = 0;
      std::vector<int> vlist;
      for ( v = 0; v < vcount; v++ )
      {
         unsigned vert = 0;
         if ( m_isLWO2 )
         {
            remaining -= readVX( vert );
         }
         else
         { 
            vert = readU2();
            remaining -= 2;
         }
         vlist.push_back( vert + m_lastVertexBase );
      }
      int16_t surf = 0;
      if ( !m_isLWO2 )
      {
         surf = readI2();
         if ( surf > 0 )
         {
            surf--;
         }
         else
         {
            surf = -surf;
            surf--;
            // TODO deal with detail polygons
         }
         remaining -= 2;
      }

      PolyMapT pm;
      pm.polyIndex = m_model->getTriangleCount();
      pm.polyCount = (vcount >= 3) ? vcount - 2 : 0;
      m_polyMaps.push_back( pm );

      if ( vcount < 3 )
      {
         log_warning( "less than 3 vertices in face (%d)\n", vcount );
      }
      else
      {
         for ( unsigned n = 0; n + 2 < vcount; n++ )
         {
            unsigned tri = m_model->addTriangle( vlist[0], vlist[n+1], vlist[n+2] );
            if ( !m_isLWO2 && surf >= 0 && surf < (int) m_surfacePolys.size() )
            {
               m_surfacePolys[surf].push_back( tri );
            }
         }
      }
   }
   return true;
}

bool LwoFilter::readSurfaceListChunk( size_t chunkLen )
{
   int remaining = chunkLen;
   while ( remaining > 0 )
   {
      char surfaceName[256];
      unsigned len = readString( surfaceName, sizeof(surfaceName) );
      log_debug( "found surface '%s'\n", surfaceName );
      m_surfacePolys.push_back( PolyList() );
      remaining -= len;
   }
   return true;
}

bool LwoFilter::readSurfaceDefinitionChunk( size_t chunkLen )
{
   int remaining = chunkLen;

   float   diffuse   =  1.0f;
   float   specular  =  0.0f;
   float   ambient   =  0.0f;
   float   shininess = 40.0f;
   float   color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
   float   smoothAngle = 0.0f;
   char surfaceName[256] = "";
   char sourceName[256] = "";
   char filename[256] = "";

   unsigned nameLen   = readString( surfaceName, sizeof(surfaceName) );
   remaining -= nameLen;

   unsigned sourceLen = 0;
   if ( m_isLWO2 )
   {
      sourceLen = readString( sourceName, sizeof(sourceName) );
      remaining -= sourceLen;
   }

   log_debug( "  index is %d\n", m_model->getTextureCount() );
   log_debug( "  name is '%s'\n", surfaceName );

   while ( remaining > 0 )
   {
      char id[5];
      id[4] = '\0';

      readID( id );
      unsigned subChunkLen = readU2();
      remaining -= 6;

      if ( strncmp( id, "COLR", 4 ) == 0 )
      {
         remaining -= readColor( color[0], color[1], color[2] );
         log_debug( "  color is %f %f %f\n", color[0], color[1], color[2] );

         if ( m_isLWO2 )
         {
            unsigned vx = 0;
            remaining -= readVX( vx );
         }
      }
      else if ( m_isLWO2 && strncmp( id, "DIFF", 4 ) == 0 )
      {
         diffuse = readF4();
         remaining -= 4;
         unsigned vx = 0;
         remaining -= readVX( vx );

         log_debug( "  diffuse is %f\n", diffuse );
      }
      else if ( strncmp( id, "CTEX", 4 ) == 0 
             || strncmp( id, "DTEX", 4 ) == 0 
             || strncmp( id, "STEX", 4 ) == 0 
             || strncmp( id, "RTEX", 4 ) == 0 
             || strncmp( id, "TTEX", 4 ) == 0 
             || strncmp( id, "BTEX", 4 ) == 0 )
      {
         char typeStr[64];
         unsigned typeLen = readString( typeStr, sizeof(typeStr) );
         log_debug( "  %c texture type is %s\n", id[0], typeStr );
         remaining -= typeLen;
      }
      else if ( strncmp( id, "TIMG", 4 ) == 0 )
      {
         if ( !m_isLWO2 )
         {
            unsigned filenameLen = readString( filename, sizeof(filename) );
            log_debug( "  got texture filename '%s'\n", filename );
            remaining -= filenameLen;
         }
      }
      else if ( strncmp( id, "SPEC", 4 ) == 0 )
      {
         if ( m_isLWO2 )
         {
            specular = readF4();
            remaining -= 4;
            unsigned vx = 0;
            remaining -= readVX( vx );
         }
         else
         {
            int spec = readU2();
            specular = (spec / 256.0f);
            remaining -= 2;
         }
         log_debug( "  specular value is %f\n", specular );
      }
      else if ( strncmp( id, "GLOS", 4 ) == 0 )
      {
         if ( m_isLWO2 )
         {
            float glos = readF4();
            remaining -= 4;

            shininess = glos;

            unsigned vx = 0;
            remaining -= readVX( vx );

            log_debug( "  shininess value is %f\n", shininess );
         }
         else
         {
            int glos = readU2();
            shininess = (glos / 1024.0f) * 100.0f;
            log_debug( "  shininess value is %f\n", shininess );
            remaining -= 2;
         }
      }
      else if ( strncmp( id, "SMAN", 4 ) == 0 )
      {
         smoothAngle = readF4();
         remaining -= 4;

         // In radians
         smoothAngle /= PIOVER180;

         if ( smoothAngle < 0.0f )
         {
            smoothAngle = 0.0f;
         }
         if ( smoothAngle > 180.0f )
         {
            smoothAngle = 180.0f;
         }

         log_debug( "  max smoothing angle is %f\n", smoothAngle );
      }
      else if ( m_isLWO2 && strncmp( id, "BLOK", 4 ) == 0 )
      {
         SurfaceBlockDataT sbd;
         readSurfaceBlockChunk( subChunkLen, sbd );
         remaining -= subChunkLen;

         if ( sbd.textureIndex >= 0 )
         {
            strncpy( filename, m_clips[sbd.textureIndex].filename.c_str(), sizeof(filename) );
            filename[ sizeof(filename) - 1 ] = '\0';
            log_debug( "  texture file is %s\n", filename );
         }
      }
      else
      {
         log_debug( "  *** ignoring sub chunk %s of %d bytes\n", id, subChunkLen );
         m_src->seek( m_src->offset() + subChunkLen );
         remaining -= subChunkLen;
      }
   }

   Model::Material * mat = Model::Material::get();
   mat->m_type = Model::Material::MATTYPE_BLANK;
   memset( mat->m_color, 255, sizeof( mat->m_color ) );
   mat->m_name = surfaceName;
   mat->m_diffuse[0]  = color[0] * diffuse;
   mat->m_diffuse[1]  = color[1] * diffuse;
   mat->m_diffuse[2]  = color[2] * diffuse;
   mat->m_diffuse[3]  = 1.0f;
   mat->m_ambient[0]  = color[0] * ambient;
   mat->m_ambient[1]  = color[1] * ambient;
   mat->m_ambient[2]  = color[2] * ambient;
   mat->m_ambient[3]  = 1.0f;
   mat->m_specular[0] = color[0] * specular;
   mat->m_specular[1] = color[1] * specular;
   mat->m_specular[2] = color[2] * specular;
   mat->m_specular[3] = 1.0f;
   mat->m_shininess   = 0.0f;
   if ( filename[0] != '\0' )
   {
      mat->m_type = Model::Material::MATTYPE_TEXTURE;
      mat->m_filename = filename;
   }
   else
   {
      mat->m_type = Model::Material::MATTYPE_BLANK;
   }
   unsigned surfId = m_model->getTextureCount();
   getMaterialList( m_model ).push_back( mat );

   m_smoothAngles.push_back( smoothAngle );

   for ( unsigned s = 0; s < m_tags.size(); s++ )
   {
      if ( strcmp( surfaceName, m_tags[s].c_str() ) == 0 )
      {
         m_surfaceTags[ s ] = surfId;
         break;
      }
   }
   return true;
}

bool LwoFilter::readSurfaceBlockChunk( size_t chunkLen, SurfaceBlockDataT & sbd )
{
   int remaining = chunkLen;

   char id[5];
   id[4] = '\0';

   readID( id );
   unsigned headerLen = readU2();

   remaining -= 6;

   // Skip header, it's boring anyway
   remaining -= headerLen;
   m_src->seek( m_src->offset() + headerLen );

   sbd.textureIndex = -1;

   while ( remaining > 0 )
   {
      readID( id );
      unsigned subChunkLen = readU2();
      remaining -= 6;

      if ( strncmp( id, "IMAG", 4 ) == 0 )
      {
         unsigned index = 0;
         remaining -= readVX( index );

         sbd.textureIndex = index - 1;
         log_debug( "  surface is using texture file index %d\n", index );
      }
      else
      {
         log_debug( "  *** ignoring block sub chunk %s of %d bytes\n", id, subChunkLen );
         m_src->seek( m_src->offset() + subChunkLen );
         remaining -= subChunkLen;
      }
      log_debug( "  %d bytes left in block chunk\n", remaining );
   }

   return true;
}

bool LwoFilter::readVertexMapChunk( size_t chunkLen )
{
   uint32_t remaining = chunkLen;
   char id[5];
   id[4] = '\0';

   readID( id );
   unsigned dimension = readU2();
   remaining -= 6;

   char name[256];
   remaining -= readString( name, sizeof(name) );

   if ( strcmp( id, "TXUV" ) == 0)
   {
      while ( remaining > 0 )
      {
         float val[4];

         unsigned vert = 0;
         remaining -= readVX( vert );
         for ( unsigned t = 0; t < dimension; t++ )
         {
            if ( t < 4 )
            {
               val[t] = readF4();
            }
            else
            {
               readF4();
            }
            remaining -= 4;
         }

         VertexMapDataT vmd;
         vmd.vertex = vert;
         vmd.s = val[0];
         vmd.t = val[1];
         m_vertexMaps.push_back( vmd );
      }
   }
   else
   {
      readNothing( remaining );
   }

   return true;
}

bool LwoFilter::readDiscVertexMapChunk( size_t chunkLen )
{
   return false;
}

bool LwoFilter::readTagChunk( size_t chunkLen )
{
   int remaining = chunkLen;
   while ( remaining > 0 )
   {
      char tagName[256];
      unsigned len = readString( tagName, sizeof(tagName) );
      log_debug( "found tag %d '%s'\n", m_tags.size(), tagName );
      m_tags.push_back( tagName );
      m_surfacePolys.push_back( PolyList() );
      m_surfaceTags.push_back( 0xffffffff );
      remaining -= len;
   }
   return true;
}

bool LwoFilter::readPolyTagChunk( size_t chunkLen )
{
   int remaining = chunkLen;
   char id[5];
   id[4] = '\0';

   readID( id );
   remaining -= 4;

   bool isSurface = (strcmp( "SURF", id ) == 0 );

   log_debug( "found %s ptag \n", id );

   while ( remaining > 0 )
   {
      unsigned poly = 0;
      remaining -= readVX( poly );

      poly += m_lastPolyBase;

      unsigned tag = readU2();
      remaining -= 2;

      if ( isSurface )
      {
         m_surfacePolys[ tag ].push_back( poly );
      }
   }
   return true;
}

bool LwoFilter::readClipChunk( size_t chunkLen )
{
   int remaining = chunkLen;
   char id[5];
   id[4] = '\0';

   unsigned clipIndex = readU4();
   remaining -= 4;

   ClipDataT cd;
   cd.filename = "";

   while ( m_clips.size() < clipIndex )
   {
      m_clips.push_back( cd );
   }
   clipIndex--;

   while ( remaining > 0 )
   {
      readID( id );
      remaining -= 4;

      unsigned chunkRemaining = readU2();
      remaining -= 2;

      if ( strcmp( "STIL", id ) == 0 )
      {
         char texFilename[256];

         remaining -= chunkRemaining;
         chunkRemaining -= readString( texFilename, sizeof(texFilename) );
         chunkRemaining -= readNothing( chunkRemaining );

         log_debug( "found texture image '%s'\n", texFilename );

         replaceBackslash( texFilename );

         // Get absolute path for texture
         string texturePath = texFilename;

         texturePath = fixAbsolutePath( m_modelPath.c_str(), texturePath.c_str() );
         texturePath = getAbsolutePath( m_modelPath.c_str(), texturePath.c_str() );

         cd.filename = texturePath;
      }
      else
      {
         log_debug( "ignoring clip type '%s'\n", id );
         remaining -= readNothing( chunkRemaining );
      }
   }

   m_clips[clipIndex] =  cd;

   return true;
}

uint32_t LwoFilter::readU4()
{
   uint32_t val;
   m_src->read( val );
   return val;
}

uint16_t LwoFilter::readU2()
{
   uint16_t val;
   m_src->read( val );
   return val;
}

uint8_t LwoFilter::readU1()
{
   uint8_t val;
   m_src->read( val );
   return val;
}

int32_t LwoFilter::readI4()
{
   int32_t val;
   m_src->read( val );
   return val;
}

int16_t LwoFilter::readI2()
{
   int16_t val;
   m_src->read( val );
   return val;
}

int8_t LwoFilter::readI1()
{
   int8_t val;
   m_src->read( val );
   return val;
}

float LwoFilter::readF4()
{
   float val;
   m_src->read( val );
   return val;
}

unsigned LwoFilter::readColor( float & r, float & g, float & b )
{
   if ( m_isLWO2 )
   {
      r = readF4();
      g = readF4();
      b = readF4();
      return 12;
   }
   else
   {
      r = readU1() / 255.0f;
      g = readU1() / 255.0f;
      b = readU1() / 255.0f;
      readU1();
      return 4;
   }
}

unsigned LwoFilter::readVX( unsigned & vx )
{
   off_t pos = m_src->offset();
   uint8_t firstByte = m_src->readU8();
   m_src->seek( pos );

   if ( m_isLWO2 && firstByte == 0xFF )
   {
      vx = readU4();
      vx = vx & 0x00FFFFFF; // FIXME: this is probably wrong on big endian?
      return 4;
   }
   else
   { 
      vx = readU2();
      return 2;
   }
}

void LwoFilter::readID( char * id )
{
   m_src->readBytes( (uint8_t *) id, 4 );
}

unsigned LwoFilter::readString( char * dest, size_t len )
{
   m_src->readBytes( (uint8_t *) dest, len );
   dest[ len - 1 ]= '\0';
   unsigned readLen = strlen( dest );
   readLen++; // account for null
   if ( (readLen % 2) == 1 )
   {
      // Read length is odd... there's an extra padding null
      m_src->readU8();
      readLen++;
   }

   return readLen;
}

unsigned LwoFilter::readNothing( size_t len )
{
   m_src->seek( m_src->offset() + len );
   return len;
}

#ifdef PLUGIN

//------------------------------------------------------------------
// Plugin functions
//------------------------------------------------------------------

PLUGIN_API bool plugin_init()
{
   if ( s_filter == NULL )
   {
      s_filter = new LwoFilter();
      FilterManager * texmgr = FilterManager::getInstance();
      texmgr->registerFilter( s_filter );
   }
   log_debug( "LWO model filter plugin initialized\n" );
   return true;
}

// The filter manager will delete our registered filter.
// We have no other cleanup to do
PLUGIN_API bool plugin_uninit()
{
   s_filter = NULL; // FilterManager deletes filters
   log_debug( "LWO model filter plugin uninitialized\n" );
   return true;
}

PLUGIN_API const char * plugin_mm3d_version()
{
   return VERSION_STRING;
}

PLUGIN_API const char * plugin_version()
{
   return "0.1.0";
}

PLUGIN_API const char * plugin_desc()
{
   return "LWO model filter";
}

#endif // PLUGIN
