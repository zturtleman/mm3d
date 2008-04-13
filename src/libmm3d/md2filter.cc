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


#include "md2filter.h"

#include "texture.h"
#include "texmgr.h"
#include "misc.h"
#include "log.h"
#include "triprim.h"
#include "translate.h"
#include "modelstatus.h"

#include "mm3dport.h"
#include "datasource.h"
#include "datadest.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

// FIXME this should be centralized
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

const unsigned MAX_QUAKE_NORMALS = 162;
static float s_quakeNormals[ MAX_QUAKE_NORMALS ][3] = {
#include "md2filter-anorms.h"
};

#include <string>

using std::list;
using std::string;

static void _invertModelNormals( Model * model )
{
   size_t tcount = model->getTriangleCount();
   for ( size_t t = 0; t < tcount; t++ )
   {
      model->invertNormals( t );
   }
}

static unsigned bestNormal( double * n )
{
   float bestDistance = 10000.0f;
   unsigned bestIndex = 0;

   float norm[3];

   norm[0] = n[0];
   norm[1] = n[2];
   norm[2] = n[1];

   for ( unsigned t = 0; t < MAX_QUAKE_NORMALS; t++ )
   {
      float x = norm[0] - s_quakeNormals[t][0];
      float y = norm[1] - s_quakeNormals[t][1];
      float z = norm[2] - s_quakeNormals[t][2];

      float distance = sqrt(x*x + y*y + z*z);

      if ( distance < bestDistance )
      {
         bestIndex = t;
         bestDistance = distance;
      }
   }

   return bestIndex;
}

typedef struct _TexCoord_t
{
   float s;
   float t;
} TexCoordT;

Md2Filter::Md2Filter()
   : m_lastAnimFrame( "" ),
     m_lastAnimIndex( -1 )
{
}

Md2Filter::~Md2Filter()
{
}

Model::ModelErrorE Md2Filter::readFile( Model * model, const char * const filename )
{
   if ( model && filename && filename[0] )
   {
      Model::ModelErrorE err = Model::ERROR_NONE;
      DataSource* src = openInput( filename, err );
      FunctionCaller<DataSource> fc( src, &DataSource::close );

      if ( err != Model::ERROR_NONE )
         return err;

      string modelPath = "";
      string modelBaseName = "";
      string modelFullName = "";

      normalizePath( filename, modelFullName, modelPath, modelBaseName );
      modelPath = modelPath + string("/");
      
      model->setFilename( modelFullName.c_str() );

      unsigned fileLength = src->getFileSize();

      int32_t magic = 0;
      int32_t version = 0;
      int32_t skinWidth = 0;
      int32_t skinHeight = 0;
      int32_t frameSize = 0;
      int32_t numSkins = 0;
      int32_t numVertices = 0;
      int32_t numTexCoords = 0;
      int32_t numTriangles = 0;
      int32_t numGlCommands = 0;
      int32_t numFrames = 0;
      int32_t offsetSkins = 0;
      int32_t offsetTexCoords = 0;
      int32_t offsetTriangles = 0;
      int32_t offsetFrames = 0;
      int32_t offsetGlCommands = 0;
      int32_t offsetEnd = 0;

      src->read( magic );
      src->read( version );
      src->read( skinWidth );
      src->read( skinHeight );
      src->read( frameSize );
      src->read( numSkins );
      src->read( numVertices );
      src->read( numTexCoords );
      src->read( numTriangles );
      src->read( numGlCommands );
      src->read( numFrames );
      src->read( offsetSkins );
      src->read( offsetTexCoords );
      src->read( offsetTriangles );
      src->read( offsetFrames );
      src->read( offsetGlCommands );
      src->read( offsetEnd );

      Matrix loadMatrix;
      loadMatrix.setRotationInDegrees( -90, -90, 0 );

      if ( magic != 0x32504449 )
      {
         return Model::ERROR_BAD_MAGIC;
      }

      if ( version != 8 )
      {
         return Model::ERROR_UNSUPPORTED_VERSION;
      }

      log_debug( "Magic: %08X\n",     (unsigned) magic );
      log_debug( "Version: %d\n",     version );
      log_debug( "Vertices: %d\n",    numVertices );
      log_debug( "Faces: %d\n",       numTriangles );
      log_debug( "Skins: %d\n",       numSkins );
      log_debug( "Frames: %d\n",      numFrames );
      log_debug( "GL Commands: %d\n", numGlCommands );
      log_debug( "Frame Size: %d\n",  frameSize );
      log_debug( "TexCoords: %d\n",   numTexCoords );
      log_debug( "Skin Width: %d\n",  skinWidth );
      log_debug( "Skin Height: %d\n", skinHeight );

      log_debug( "Offset Skins: %d\n",      offsetSkins );
      log_debug( "Offset TexCoords: %d\n",  offsetTexCoords );
      log_debug( "Offset Triangles: %d\n",  offsetTriangles );
      log_debug( "Offset Frames: %d\n",     offsetFrames );
      log_debug( "Offset GlCommands: %d\n", offsetGlCommands );
      log_debug( "Offset End: %d\n",        offsetEnd );

      log_debug( "File Length: %d\n",       fileLength );

      int32_t i;
      unsigned t;
      unsigned vertex;

      int animIndex = -1;
      m_lastAnimIndex = -1;
      m_animFrame = -1;
      m_lastAnimFrame = "";
      
      // Read first frame to get vertices
      src->seek( offsetFrames );
      
      float scale[3];
      float translate[3];
      char name[64];
      for ( t = 0; t < 3; t++ )
         src->read( scale[t] );
      for ( t = 0; t < 3; t++ )
         src->read( translate[t] );

      //loadMatrix.apply3( scale );
      //loadMatrix.apply3( translate );

      src->readBytes( (uint8_t *) name, 16 );

      for ( i = 0; i < numVertices; i++ )
      {
         uint8_t coord[3];
         uint8_t normal;
         for ( t = 0; t < 3; t++ )
         {
            src->read( coord[t] );
         }
         src->read( normal );

         double vec[3];
         vec[0] = coord[0] * scale[0] + translate[0];
         vec[1] = coord[1] * scale[1] + translate[1];
         vec[2] = coord[2] * scale[2] + translate[2];
         loadMatrix.apply3( vec );

         vertex = model->addVertex( 
               vec[0],
               vec[1],
               vec[2] );
      }

      // Now read all frames to get animation vertices
      src->seek( offsetFrames );
      
      if ( numFrames > 1 )
      {
         for ( int n = 0; n < numFrames; n++ )
         {
            for ( t = 0; t < 3; t++ )
               src->read( scale[t] );
            for ( t = 0; t < 3; t++ )
               src->read( translate[t] );

            src->readBytes( (uint8_t *) name, 16 );

            animIndex = addNeededAnimFrame( model, name );

            for ( i = 0; i < numVertices; i++ )
            {
               uint8_t coord[3];
               uint8_t normal;
               for ( t = 0; t < 3; t++ )
                  src->read( coord[t] );
               src->read( normal );

               double vec[3];
               vec[0] = coord[0] * scale[0] + translate[0];
               vec[1] = coord[1] * scale[1] + translate[1];
               vec[2] = coord[2] * scale[2] + translate[2];
               loadMatrix.apply3( vec );

               model->setFrameAnimVertexCoords( animIndex, m_animFrame, i,
                     vec[0],
                     vec[1],
                     vec[2] );
            }
         }
      }

      // Read texture coords so that we have them for our triangles
      TexCoordT * texCoordsList = new TexCoordT[ numTexCoords ];
      src->seek( offsetTexCoords );

      for ( i = 0; i < numTexCoords; i++ )
      {
         int16_t s;
         int16_t t;
         src->read( s );
         src->read( t );
         texCoordsList[i].s = (float) s / skinWidth;
         texCoordsList[i].t = 1.0 - (float) t / skinHeight;
      }

      // Create group for all triangles
      model->addGroup( "Group" );

      // Now read triangles
      src->seek( offsetTriangles );

      for ( i = 0; i < numTriangles; i++ )
      {
         uint16_t vertexIndices[3];
         uint16_t textureIndices[3];

         for ( t = 0; t < 3; t++ )
            src->read( vertexIndices[t] );

         unsigned tri = model->addTriangle( vertexIndices[0], vertexIndices[1], vertexIndices[2] );
         model->addTriangleToGroup( 0, tri );

         for ( t = 0; t < 3; t++ )
         {
            src->read( textureIndices[t] );
            model->setTextureCoords( i, t, 
                  texCoordsList[ textureIndices[t] ].s, 
                  texCoordsList[ textureIndices[t] ].t );
         }
      }

      // Now read skins
      src->seek( offsetSkins );

      TextureManager * texmgr = TextureManager::getInstance();
      bool haveSkin = false;

      vector<Model::Material *> & modelMaterials = getMaterialList( model );

      log_debug( "Skin data:\n" );
      for ( i = 0; i < numSkins; i++ )
      {
         src->readBytes( (uint8_t *) name, 64 );
         log_debug( "Skin %d: %s\n", i, name );

         string tempStr = name;
         replaceBackslash( tempStr );

         char tempPath[ 64 ];
         if ( !model->getMetaData( "MD2_PATH", tempPath, sizeof(tempPath) ) )
         {
            string md2path = tempStr;
            size_t slashChar = md2path.rfind( '/' );
            log_debug( "The '/' char is at %d\n", slashChar );
            if ( slashChar < md2path.size() )
            {
               md2path.resize( slashChar + 1 );
               log_debug( "setting MD2 path to %s\n", md2path.c_str() );
               model->addMetaData( "MD2_PATH", md2path.c_str() );
            }
         }

         string fullName;
         string fullPath;
         string baseName;

         normalizePath( tempStr.c_str(), fullName, fullPath, baseName );

         log_debug( "base model skin name is %s\n", baseName.c_str() );

         string skin = modelPath + baseName;
         log_debug( "full model skin path is %s\n", skin.c_str() );

         Model::Material * mat = Model::Material::get();

         mat->m_name = "skin";

         for ( int m = 0; m < 3; m++ )
         {
            mat->m_ambient[m] = 0.2;
            mat->m_diffuse[m] = 0.8;
            mat->m_specular[m] = 0.0;
            mat->m_emissive[m] = 0.0;
         }
         mat->m_ambient[3]  = 1.0;
         mat->m_diffuse[3]  = 1.0;
         mat->m_specular[3] = 1.0;
         mat->m_emissive[3] = 1.0;

         mat->m_shininess = 0.0;

         //replaceBackslash( material->m_texture );
         //replaceBackslash( material->m_alphamap );

         // Get absolute path for texture
         string texturePath = skin;

         texturePath = fixAbsolutePath( modelPath.c_str(), texturePath.c_str() );
         texturePath = getAbsolutePath( modelPath.c_str(), texturePath.c_str() );

         mat->m_filename  = texturePath;

         mat->m_alphaFilename = "";

         if ( texmgr->getTexture( texturePath.c_str() ) )
         {
            log_debug( "skin %d: '%s'\n", i, texturePath.c_str());
            haveSkin = true;
            modelMaterials.push_back( mat );
         }
         else
         {
            mat->release();
         }
      }

      // If we don't have any skins, lets try to find some
      if ( modelMaterials.size() == 0 )
      {
         char * noext = strdup( modelBaseName.c_str() );

         char * ext = strrchr( noext, '.' );
         if ( ext )
         {
            ext[0] = '\0';
         }

         log_debug( "no skins defined, looking for some....\n" );
         list<string> files;
         getFileList( files, modelPath.c_str(), noext );
         list<string>::iterator it;

         free( noext );

         for ( it = files.begin(); it != files.end(); it++ )
         {
            string texturePath = modelPath + (*it);

            texturePath = getAbsolutePath( modelPath.c_str(), texturePath.c_str() );

            log_debug( "checking %s\n", texturePath.c_str() );
            if ( texmgr->getTexture( texturePath.c_str() ) )
            {
               log_debug( "  %s is a skin\n", texturePath.c_str() );
               haveSkin = true;
               Model::Material * mat = Model::Material::get();

               replaceBackslash( texturePath );

               mat->m_name = getFileNameFromPath( texturePath.c_str() );

               for ( int m = 0; m < 3; m++ )
               {
                  mat->m_ambient[m] = 0.2;
                  mat->m_diffuse[m] = 0.8;
                  mat->m_specular[m] = 0.0;
                  mat->m_emissive[m] = 0.0;
               }
               mat->m_ambient[3]  = 1.0;
               mat->m_diffuse[3]  = 1.0;
               mat->m_specular[3] = 1.0;
               mat->m_emissive[3] = 1.0;

               mat->m_shininess = 0.0;

               //replaceBackslash( material->m_texture );
               //replaceBackslash( material->m_alphamap );

               // Get absolute path for texture

               mat->m_filename  = texturePath;

               mat->m_alphaFilename = "";

               modelMaterials.push_back( mat );
            }
         }
      }

      if ( haveSkin )
      {
         model->setGroupTextureId( 0, 0 );
      }

      // Now read strips/fans
      src->seek( offsetGlCommands );

      unsigned numStrips = 0;
      unsigned numFans   = 0;
      unsigned tcount    = 0;

      for ( i = 0; i < numGlCommands; i++ )
      {
         int32_t  numVertices = 0;
         bool     isStrip = true;

         uint32_t  vertexIndex = 0;
         float u = 0.0;
         float v = 0.0;

         src->read( numVertices );

         if ( numVertices != 0 )
         {
            if ( numVertices < 0 )
            {
               numVertices = -numVertices;
               isStrip = false;

               numFans++;
            }
            else
            {
               numStrips++;
            }

            tcount += numVertices - 2;

            for ( t = 0; t < (unsigned) numVertices; t++ )
            {
               src->read( u );
               src->read( v );
               src->read( vertexIndex );
            }
         }
         else
         {
            break;
         }
      }

      log_debug( "strip count: %d\n", numStrips );
      log_debug( "fan count:   %d\n", numFans   );
      log_debug( "tri count:   %d\n", tcount    );

      delete[] texCoordsList;

      _invertModelNormals( model );

      return Model::ERROR_NONE;
   }
   else
   {
      log_error( "no filename supplied for model filter" );
      return Model::ERROR_BAD_ARGUMENT;
   }
}

// FIXME test this!
Model::ModelErrorE Md2Filter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o )
{
   if ( model && filename && filename[0] )
   {
      {
         int groupTexture = -1;
         int gcount = model->getGroupCount();
         for ( int g = 0; g < gcount; g++ )
         {
            int tex = model->getGroupTextureId( g );
            if ( tex >= 0 )
            {
               if ( groupTexture >= 0 && groupTexture != tex ) {
                  model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "MD2 requires all groups to have the same material." ) ).c_str() );
                  return Model::ERROR_FILTER_SPECIFIC;
               }
               groupTexture = tex;
            }
         }
      }

      {
         unsigned tcount = model->getTriangleCount();
         for ( unsigned t = 0; t < tcount; ++t )
         {
            if ( model->getTriangleGroup( t ) < 0 )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "MD2 export requires all faces to be grouped." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }
         }
      }

      _invertModelNormals( model );

      Model::ModelErrorE err = Model::ERROR_NONE;
      DataDest * dst = openOutput( filename, err );
      FunctionCaller<DataDest> fc( dst, &DataDest::close );

      if ( err != Model::ERROR_NONE )
         return err;

      string modelPath = "";
      string modelBaseName = "";
      string modelFullName = "";

      normalizePath( filename, modelFullName, modelPath, modelBaseName );
      
      vector<Model::Vertex *>   & modelVertices  = getVertexList( model );
      vector<Model::Triangle *> & modelTriangles = getTriangleList( model );
      vector<Model::Material *> & modelMaterials = getMaterialList( model );

      unsigned numStrips = 0;
      unsigned numFans   = 0;
      unsigned tcount    = 0;

      TriPrim * tp = TriPrim::newTriPrim();

      std::vector<TriPrim::TriangleVertexList> stripList;
      std::vector<TriPrim::TriangleVertexList> fanList;
      log_debug( "finding triangle primitives\n" );
      if ( tp->findPrimitives( model, true ) )
      {
         TriPrim::TriangleVertexList tvl;

         TriPrim::TriPrimTypeE tpt;
         while ( (tpt = tp->getNextPrimitive( tvl ) ) != TriPrim::TRI_PRIM_NONE )
         {
            //log_debug( "got triangle %s with %d triangles\n", tpt == TriPrim::TRI_PRIM_STRIP ? "strip" : "fan", tvl.size() );
            tcount += tvl.size() - 2;

            if ( tpt == TriPrim::TRI_PRIM_STRIP )
            {
               numStrips++;
               stripList.push_back( tvl );
            }
            else
            {
               numFans++;
               fanList.push_back( tvl );
            }
         }

         numStrips = stripList.size();
         numFans   = fanList.size();
      }
      delete tp;
      tp = NULL;

      log_debug( "strip count: %d\n", numStrips );
      log_debug( "fan count:   %d\n", numFans   );
      log_debug( "tri count:   %d\n", tcount    );

      int32_t magic = 0x32504449;
      int32_t version = 8;
      int32_t skinWidth = 256;
      int32_t skinHeight = 256;
      int32_t frameSize = 0;
      int32_t numSkins = modelMaterials.size();
      int32_t numVertices = modelVertices.size();
      int32_t numTexCoords = modelTriangles.size() * 3;
      int32_t numTriangles = modelTriangles.size();
      int32_t numGlCommands = (numTriangles + numStrips * 2 + numFans * 2) * 3 
         + numStrips + numFans + 1;
      int32_t numFrames = 0;
      int32_t offsetSkins = 68;
      int32_t offsetTexCoords = 68;
      int32_t offsetTriangles = 0;
      int32_t offsetFrames = 0;
      int32_t offsetGlCommands = 0;
      int32_t offsetEnd = 0;

      // Use the load matrix and then invert it
      Matrix saveMatrix;
      saveMatrix.setRotationInDegrees( -90, -90, 0 );
      saveMatrix = saveMatrix.getInverse();

      bool noAnim = false;

      frameSize = (4 * numVertices) + 40;

      unsigned i;

      unsigned animCount = model->getAnimCount( Model::ANIMMODE_FRAME );
      for ( i = 0; i < animCount; i++ )
      {
         numFrames += model->getAnimFrameCount( Model::ANIMMODE_FRAME, i );
      }
      if ( numFrames == 0 )
      {
         numFrames = 1;
         noAnim = true;
      }

      if ( ! modelMaterials.empty() && modelMaterials[0]->m_textureData )
      {
         skinWidth  = modelMaterials[0]->m_textureData->m_width;
         skinHeight = modelMaterials[0]->m_textureData->m_height;
      }

      // Write header
      dst->write(magic);
      dst->write(version);
      dst->write(skinWidth);
      dst->write(skinHeight);
      dst->write(frameSize);
      dst->write(numSkins);
      dst->write(numVertices);
      dst->write(numTexCoords);
      dst->write(numTriangles);
      dst->write(numGlCommands);
      dst->write(numFrames);
      dst->write(offsetSkins);
      dst->write(offsetTexCoords);
      dst->write(offsetTriangles);
      dst->write(offsetFrames);
      dst->write(offsetGlCommands);
      dst->write(offsetEnd);


      // Write skins
      offsetSkins = dst->offset();
      for ( i = 0; i < (unsigned) numSkins; i++ )
      {
         int8_t skinpath[64]; // must be 64 bytes for skin writing to work

         string fullName, fullPath, baseName;
         normalizePath( modelMaterials[i]->m_filename.c_str(), fullName, fullPath, baseName );

         char * noext = strdup( modelBaseName.c_str() );

         char * ext = strrchr( noext, '.' );
         if ( ext )
         {
            ext[0] = '\0';
         }

         char md2path[64];
         if ( model->getMetaData( "MD2_PATH", md2path, sizeof(md2path) ) )
         {
            PORT_snprintf( (char *) skinpath, sizeof(skinpath), "%s%s", md2path, baseName.c_str() );
         }
         else
         {
            // Assume player model
            PORT_snprintf( (char *) skinpath, sizeof(skinpath), "players/%s/%s", noext, baseName.c_str() );
         }
         log_debug( "writing skin %s as %s\n", baseName.c_str(), skinpath );

         dst->writeBytes( (uint8_t *) skinpath, sizeof(skinpath) );

         free( noext );
      }

      // Write texture coordinates
      // TODO might be able to optimize by finding shared texture coordinates
      offsetTexCoords = dst->offset();

      for ( i = 0; i < (unsigned) numTriangles; i++ )
      {
         for ( unsigned n = 0; n < 3; n++ )
         {
            float   sd;
            float   td;
            uint16_t si;
            uint16_t ti;
            model->getTextureCoords( i, n, sd, td );

            td = 1.0 - td;

            si = (int16_t) (sd * (double) skinWidth);
            dst->write(si);
            ti = (int16_t) (td * (double) skinHeight);
            dst->write(ti);
         }
      }

      // Write Triangles
      offsetTriangles = dst->offset();
      for ( i = 0; i < (unsigned) numTriangles; i++ )
      {
         for ( unsigned v = 0; v < 3; v++ )
         {
            uint16_t vertex = (uint16_t) model->getTriangleVertex( i, v );
            dst->write(vertex);
         }
         for ( unsigned v = 0; v < 3; v++ )
         {
            uint16_t texindex = (uint16_t) i * 3 + v;
            dst->write(texindex);
         }
      }

      // Write Frames
      offsetFrames = dst->offset();

      for ( unsigned anim = 0; 
            (noAnim && anim == 0) || anim < model->getAnimCount( Model::ANIMMODE_FRAME );
            anim++ )
      {
         // Heh... oops
         model->calculateFrameNormals( anim );

         std::string animname = noAnim ? "Frame" : model->getAnimName( Model::ANIMMODE_FRAME, anim );

         // Check for animation name ending with a number
         size_t len = strlen( animname.c_str() );
         log_debug( "last char of animname is '%c'\n", animname[len-1] );
         if ( len > 0 && isdigit( animname[len-1] ) )
         {
            log_debug( "appending underscore\n" );
            // last char is number, append an underscore as a separator
            animname += '_';
         }

         for ( unsigned i = 0; 
               (noAnim && i == 0) || i < model->getAnimFrameCount( Model::ANIMMODE_FRAME, anim ); 
               i++ )
         {
            double x0 = 0, x1 = 0;
            double y0 = 0, y1 = 0;
            double z0 = 0, z1 = 0;

            model->getFrameAnimVertexCoords( anim, i, 0, x0, y0, z0 );

            x1 = x0;
            y1 = y0;
            z1 = z0;

            unsigned v;

            for ( v = 0; v < (unsigned) numVertices; v++ )
            {
               double x = 0;
               double y = 0;
               double z = 0;

               model->getFrameAnimVertexCoords( anim, i, v, x, y, z );

               if ( x < x0 )
                  x0 = x;
               if ( y < y0 )
                  y0 = y;
               if ( z < z0 )
                  z0 = z;
               if ( x > x1 )
                  x1 = x;
               if ( y > y1 )
                  y1 = y;
               if ( z > z1 )
                  z1 = z;
            }

            float scale[3];
            float translate[3];

            scale[0] = (x1 - x0) / 255.0;
            scale[1] = (y1 - y0) / 255.0;
            scale[2] = (z1 - z0) / 255.0;

            translate[0] = (float) (0 - (-x0));
            translate[1] = (float) (0 - (-y0));
            translate[2] = (float) (0 - (-z0));

            saveMatrix.apply3( scale );
            saveMatrix.apply3( translate );

            dst->write(scale[0]);
            dst->write(scale[1]);
            dst->write(scale[2]);
            dst->write(translate[0]);
            dst->write(translate[1]);
            dst->write(translate[2]);


            char namestr[16];
            snprintf( namestr, 16, "%s%03d", animname.c_str(), i );
            namestr[15] = '\0';
            dst->writeBytes( (uint8_t *) namestr, sizeof(namestr) );

            double vertNormal[3] = {0,0,0};
            for ( v = 0; v < (unsigned) numVertices; v++ )
            {
               double vec[3] = {0, 0, 0};
               model->getFrameAnimVertexCoords( anim, i, v, vec[0], vec[1], vec[2] );
               saveMatrix.apply3( vec );
               uint8_t xi = (uint8_t) ((vec[0] - translate[0]) / scale[0] + 0.5);
               uint8_t yi = (uint8_t) ((vec[1] - translate[1]) / scale[1] + 0.5);
               uint8_t zi = (uint8_t) ((vec[2] - translate[2]) / scale[2] + 0.5);
               uint8_t normal = 0;

               model->getFrameAnimVertexNormal( anim, i, v, 
                     vertNormal[0], vertNormal[1], vertNormal[2] );
               saveMatrix.apply3( vertNormal );

               // Have to invert normal
               vertNormal[0] = -vertNormal[0];
               vertNormal[1] = -vertNormal[1];
               vertNormal[2] = -vertNormal[2];

               normal = bestNormal( vertNormal );

               dst->write( xi );
               dst->write( yi );
               dst->write( zi );
               dst->write( normal );
            }
         }
      }

      // Write GL Commands
      offsetGlCommands = dst->offset();

      if ( numTriangles > 0 )
      {
         if ( numStrips > 0 || numFans > 0 )
         {
            unsigned t = 0;
            for ( t = 0; t < numStrips; t++ )
            {
               int32_t  count = stripList[t].size();
               dst->write(count);

               for ( unsigned i = 0; i < (unsigned) count; i++ )
               {
                  uint32_t v1 = stripList[t][i].vert;
                  float    s1 = stripList[t][i].s;
                  float    t1 = 1.0f - stripList[t][i].t;

                  // FIXME this is reversed from below. Is it correct?
                  dst->write(t1);
                  dst->write(s1);
                  dst->write(v1);
               }
            }

            for ( t = 0; t < numFans; t++ )
            {
               int32_t  count = fanList[t].size();

               dst->write((int32_t) -count);

               for ( unsigned i = 0; i < (unsigned) count; i++ )
               {
                  uint32_t v1 = fanList[t][i].vert;
                  float    s1 = fanList[t][i].s;
                  float    t1 = 1.0f - fanList[t][i].t;

                  // FIXME this is reversed from above. Is it correct?
                  dst->write(v1);
                  dst->write(s1);
                  dst->write(t1);
               }
            }
         }
      }

      // Write GL command end marker
      int32_t end = 0;
      dst->write( end );

      // Get file length
      offsetEnd = dst->offset();

      // Re-write header
      dst->seek( 0 );
      dst->write(magic);
      dst->write(version);
      dst->write(skinWidth);
      dst->write(skinHeight);
      dst->write(frameSize);
      dst->write(numSkins);
      dst->write(numVertices);
      dst->write(numTexCoords);
      dst->write(numTriangles);
      dst->write(numGlCommands);
      dst->write(numFrames);
      dst->write(offsetSkins);
      dst->write(offsetTexCoords);
      dst->write(offsetTriangles);
      dst->write(offsetFrames);
      dst->write(offsetGlCommands);
      dst->write(offsetEnd);

      _invertModelNormals( model );
      model->operationComplete( "Invert normals for save" );

      return Model::ERROR_NONE;
   }
   else
   {
      log_error( "no filename supplied for model filter" );
      return Model::ERROR_BAD_ARGUMENT;
   }
}

bool Md2Filter::isSupported( const char * filename )
{
   if ( filename )
   {
      unsigned len = strlen( filename );

      if ( len >= 4 && strcasecmp( &filename[len-4], ".md2" ) == 0 )
      {
         return true;
      }
   }

   return false;
}

list<string> Md2Filter::getReadTypes()
{
   list<string> rval;

   rval.push_back( "*.md2" );

   return rval;
}

list<string> Md2Filter::getWriteTypes()
{
   list<string> rval;

   rval.push_back( "*.md2" );

   return rval;
}

int Md2Filter::addNeededAnimFrame( Model * model, const char * name )
{
   char * temp = strdup( name );

   int t = strlen(temp) - 1;
   for ( ; t > 0 && isdigit(temp[t]); t-- )
   {
      temp[t] = '\0';
   }
   if ( t > 0 && temp[t] == '_' ) {
      temp[t] = '\0';
   }

   if ( m_lastAnimIndex == -1 || strcasecmp( temp, m_lastAnimFrame.c_str() ) != 0 )
   {
      m_lastAnimFrame = temp;
      m_lastAnimIndex = model->addAnimation( Model::ANIMMODE_FRAME, temp );
      model->setAnimFPS( Model::ANIMMODE_FRAME, m_lastAnimIndex, 10.0 );
      m_animFrame = 0;
   }
   else
   {
      m_animFrame++;
   }
   model->setAnimFrameCount( Model::ANIMMODE_FRAME, m_lastAnimIndex, m_animFrame+1 );

   free( temp );
   return m_lastAnimIndex;
}

