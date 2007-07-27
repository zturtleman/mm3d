/*  Md3Filter plugin for Misfit Model 3D
 *
 *  Copyright (c) 2005-2007 Russell Valentine and Kevin Worcester
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

/*
 * Original version by Russell Valentine (russ@coldstonelabs.org)
 * Plugin based off lwofilter.cc and md2filter.cc by Kevin Worcester.
 * Spec: http://linux.ucla.edu/~phaethon/q3/formats/md3format.html
 *
 * Support for player models and animation.cfg added by Kevin Worcester.
 */

#include "md3filter.h"

#include "model.h"
#include "texture.h"
#include "texmgr.h"
#include "log.h"
#include "binutil.h"
#include "misc.h"
#include "filtermgr.h"

#include "mm3dport.h"
#include "endianconfig.h"
#include "msg.h"

#include "translate.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <vector>

#ifdef PLUGIN
#include "pluginapi.h"
#include "version.h"

static Md3Filter * s_filter = NULL;
#endif // PLUGIN

const int MD3_ANIMATIONS = 25;

const int HEADER_SIZE = (11 * 4) + MAX_QPATH;
const int FRAME_SIZE = ( 3 * ( 3 * 4 ) + 4 + 16 );
const int TAG_SIZE = ( MAX_QPATH + ( 4 * 3 ) + ( 3 * 4 * 3 ) );


const char s_animNames[ MD3_ANIMATIONS ][16] = 
{
   "both_death1",
   "both_dead1",
   "both_death2",
   "both_dead2",
   "both_death3",
   "both_dead3",
   "torso_gesture",
   "torso_attack",
   "torso_attack2",
   "torso_drop",
   "torso_raise",
   "torso_stand",
   "torso_stand2",
   "legs_walkcr",
   "legs_walk",
   "legs_run",
   "legs_back",
   "legs_swim",
   "legs_jump",
   "legs_land",
   "legs_jumpb",
   "legs_landb",
   "legs_idle",
   "legs_idlecr",
   "legs_turn",
};

int s_animLoop[ MD3_ANIMATIONS ] = 
{
   0, // both_death1
   0, // both_dead1
   0, // both_death2
   0, // both_dead2
   0, // both_death3
   0, // both_dead3
   0, // torso_gesture
   0, // torso_attack
   0, // torso_attack2
   0, // torso_drop
   0, // torso_raise
   1, // torso_stand
   1, // torso_stand2
   1, // legs_walkcr
   1, // legs_walk
   1, // legs_run
   1, // legs_back
   1, // legs_swim
   0, // legs_jump
   0, // legs_land
   0, // legs_jumpb
   0, // legs_landb
   1, // legs_idle
   1, // legs_idlecr
   1, // legs_turn
};

int s_animSyncWarning[ MD3_ANIMATIONS ] = 
{
   0, // both_death1
   0, // both_dead1
   0, // both_death2
   0, // both_dead2
   0, // both_death3
   0, // both_dead3
   0, // torso_gesture
   1, // torso_attack
   1, // torso_attack2
   1, // torso_drop
   1, // torso_raise
   0, // torso_stand
   0, // torso_stand2
   0, // legs_walkcr
   0, // legs_walk
   0, // legs_run
   0, // legs_back
   0, // legs_swim
   0, // legs_jump
   0, // legs_land
   0, // legs_jumpb
   0, // legs_landb
   0, // legs_idle
   0, // legs_idlecr
   0, // legs_turn
};

Md3Filter::Md3Filter()
{
}

Md3Filter::~Md3Filter()
{
}

Model::ModelErrorE Md3Filter::readFile( Model * model, const char * const filename )
{
   if ( model && filename && filename[0] )
   {
      // New logic:
      //
      // 1) Load each file into memory
      // 2) Create model structure with vertices, triangles, groups, and skins
      //    for all meshes in all files
      //      a) Set up points
      //      b) Set meshes
      //      c) Make sure vertices are relative to appropriate tag
      // 3) Load animations
      //      a) Set up points
      //      b) Set meshes
      //      c) Make sure vertices are relative to appropriate tag
      //
      // Notes:
      //
      // 1) don't add tags multiple times
      // 2) get correct anim number from anim frame counts
      // 3) set default vertex coords for meshes that aren't in an animation

      Md3FileDataList fileList;

      bool loadAll = false;

      m_modelPath = "";
      m_modelBaseName = "";
      string modelFullName = "";

      m_pathList.clear();

      normalizePath( filename, modelFullName, m_modelPath, m_modelBaseName );
      m_modelPath = m_modelPath + string( "/" );

      string lowerFile = m_modelPath + fixFileCase( m_modelPath.c_str(), "lower.md3" );
      string upperFile = m_modelPath + fixFileCase( m_modelPath.c_str(), "upper.md3" );
      string headFile  = m_modelPath + fixFileCase( m_modelPath.c_str(), "head.md3"  );

      if (     strncasecmp( m_modelBaseName.c_str(), "lower.", 6 ) == 0 
            || strncasecmp( m_modelBaseName.c_str(), "upper.", 6 ) == 0 
            || strncasecmp( m_modelBaseName.c_str(), "head.",  5 ) == 0 )
      {
         if ( file_exists( lowerFile.c_str() )
               && file_exists( upperFile.c_str() )
               && file_exists( headFile.c_str() ) )
         {
            log_debug( "have all files for %s\n", m_modelPath.c_str() );

            char answer = msg_info_prompt( transll( QT_TRANSLATE_NOOP( "LowLevel", "This looks like a player model.\nDo you want to load all sections?")).c_str(), "Ync" );
            if ( answer == 'Y' )
            {
               loadAll = true;
               model->addMetaData( "MD3_composite", "1" );
            }
            else if ( answer == 'N' )
            {
               model->addMetaData( "MD3_composite", "0" );
            }
            else
            {
               return Model::ERROR_CANCEL;
            }
         }
      }

      if ( loadAll )
      {
         Md3FileDataT fd;

         fd.section = MS_Lower;
         fd.modelBaseName = "lower.md3";
         fd.modelFile = lowerFile;
         fd.tag = "";
         fd.tagPoint = -1;
         fd.fileBuf = NULL;
         fd.offsetMeshes = 0;
         fd.numMeshes = 0;
         fd.offsetTags = 0;
         fd.numTags = 0;
         fd.numFrames = 0;
         fileList.push_back( fd );

         fd.section = MS_Upper;
         fd.modelBaseName = "upper.md3";
         fd.modelFile = upperFile;
         fd.tag = "tag_torso";
         fileList.push_back( fd );

         fd.section = MS_Head;
         fd.modelBaseName = "head.md3";
         fd.modelFile = headFile;
         fd.tag = "tag_head";
         fileList.push_back( fd );
      }
      else
      {
         Md3FileDataT fd;

         fd.section = MS_None;
         fd.modelBaseName = m_modelBaseName;
         fd.modelFile = filename;
         fd.tag = "";
         fd.tagPoint = -1;
         fd.fileBuf = NULL;
         fd.offsetMeshes = 0;
         fd.numMeshes = 0;
         fd.offsetTags = 0;
         fd.numTags = 0;
         fd.numFrames = 0;

         fileList.push_back( fd );
      }

      Md3FileDataList::iterator it = fileList.begin();

      if ( loadAll )
      {
         readAnimations( false );
      }

      for ( it = fileList.begin(); it != fileList.end(); it++ )
      {
         m_modelBaseName = (*it).modelBaseName;
         FILE * fp = fopen( (*it).modelFile.c_str(), "rb" );
         if ( fp == NULL )
         {
            if ( errno == ENOENT )
            {
               log_error( "%s: file does not exist\n", filename );
               return Model::ERROR_NO_FILE;
            }
            if ( errno == EPERM )
            {
               log_error( "%s: access denied\n", filename );
               return Model::ERROR_NO_ACCESS;
            }

            log_error( "%s: could not open file\n", filename );
            return Model::ERROR_FILE_OPEN;
         }

         log_debug( "loading model file %s\n", (*it).modelFile.c_str() );

         model->setFilename( modelFullName.c_str() );

         m_model = model;

         fseek( fp, 0, SEEK_END );
         unsigned fileLength = ftell( fp );
         fseek( fp, 0, SEEK_SET );

         // read whole file into memory
         m_fileBuf = new uint8_t[fileLength];
         m_bufPos = m_fileBuf;

         if ( fread( m_fileBuf, fileLength, 1, fp ) != 1 )
         {
            delete[] m_fileBuf;
            fclose( fp );
            log_error( "%s: could not read file\n", filename );
            return Model::ERROR_FILE_READ;
         }

         fclose( fp );

         Matrix loadMatrix;
         loadMatrix.setRotationInDegrees( -90, -90, 0 );

         int8_t magic[4];
         for ( int t = 0; t < 4; t++ )
         {
            magic[t]=readI1();
         }
         int32_t version = readI4();
         char pk3Name[MAX_QPATH];
         readString( pk3Name, sizeof( pk3Name ) );
         replaceBackslash( pk3Name );

         int32_t flags = readI4();
         int32_t numFrames = readI4();
         int32_t numTags = readI4();
         int32_t numMeshes = readI4();
         int32_t numSkins = readI4();
         int32_t offsetFrames = readI4();
         int32_t offsetTags = readI4();
         int32_t offsetMeshes = readI4();
         int32_t offsetEnd = readI4();

         log_debug( "Magic: %c%c%c%c\n", magic[0], magic[1], magic[2], magic[3] );
         log_debug( "Version: %d\n",     version );
         log_debug( "PK3 Name: %s\n",    pk3Name );
         log_debug( "Flags: %d\n",    flags );
         log_debug( "Frames: %d\n",      numFrames );
         log_debug( "Tags: %d\n",        numTags );
         log_debug( "Meshes: %d\n",    numMeshes );
         log_debug( "Skins: %d\n",       numSkins );
         log_debug( "Offset Frames: %d\n",      offsetFrames );
         log_debug( "Offset Tags: %d\n",  offsetTags );
         log_debug( "Offset Meshes: %d\n",  offsetMeshes );
         log_debug( "Offset End: %d\n",        offsetEnd );
         log_debug( "File Length: %d\n",       fileLength );

         if ( magic[0] != 'I' && magic[1] != 'D' && magic[2] != 'P' && magic[3] != '3' )
         {
            log_debug( "Bad Magic: %c%c%c%c\n", magic[0], magic[1], magic[2], magic[3] );
            return Model::ERROR_BAD_MAGIC;
         }

         if ( version != MD3_VERSION )
         {
            return Model::ERROR_UNSUPPORTED_VERSION;
         }

         
         Md3PathT mpath;
         mpath.section  = (*it).section;
         mpath.material = -1;
         mpath.path = extractPath(pk3Name);;

         m_pathList.push_back( mpath );
         m_lastMd3Path = mpath.path;

         // frames
         // mm3d doesn't need this, but nice to have if you ever need to debug

#if 0
         m_bufPos     = &m_fileBuf[ offsetFrames ];
         for ( int i = 0; i < numFrames; i++ )
         {
            float minBound[3];
            for ( int t = 0; t < 3; t++ )
            {
               minBound[t] = readF4();
            }

            float maxBound[3];
            for ( int t = 0; t < 3; t++ )
            {
               maxBound[t] = readF4();
            }

            float localOrigin[3];
            for ( int t = 0; t < 3; t++ )
            {
               localOrigin[t] = readF4();
            }

            //float radius = readF4();

            char frameName[16];
            readString( frameName, sizeof(frameName) );

            //log_debug( "Frame %d minBound: %f, %f, %f\n", i, minBound[0], minBound[1], minBound[2] );
            //log_debug( "Frame %d maxBound: %f, %f, %f\n", i, maxBound[0], maxBound[1], maxBound[2] );
            //log_debug( "Frame %d localOrigin: %f, %f, %f\n", i, localOrigin[0], localOrigin[1], localOrigin[2] );
            //log_debug( "Frame %d radius: %f\n", i, radius );
            log_debug( "Frame %d name: %s\n", i, frameName );
         }
#endif // 1

         if ( (*it).tag.size() > 0 )
         {
            (*it).tagPoint = m_model->getPointByName( (*it).tag.c_str() );
            log_debug( "tag point for %s is %d\n", (*it).tag.c_str(), (*it).tagPoint );
         }

         m_meshVecInfos = new MeshVectorInfoT*[numMeshes];
         setPoints( (*it).section, offsetTags, numTags, numFrames, (*it).tagPoint, -1 );
         setMeshes( (*it).section, offsetMeshes, numMeshes, (*it).tagPoint, -1 );

         (*it).meshVecInfos = m_meshVecInfos;
         (*it).fileBuf = m_fileBuf;
         (*it).offsetMeshes = offsetMeshes;
         (*it).numMeshes = numMeshes;
         (*it).offsetTags = offsetTags;
         (*it).numTags = numTags;
         (*it).numFrames = numFrames;
      }

      if ( fileList.front().numFrames > 1 )
      {
         log_debug( "Model has animation, setting up animation mode.\n" );
         if ( m_model->getAnimCount( Model::ANIMMODE_FRAME ) == 0 )
         {
            if ( !loadAll || !readAnimations( true ) )
            {
               int animIndex = m_model->addAnimation( Model::ANIMMODE_FRAME, "AnimFrames" );
               m_model->setAnimFPS( Model::ANIMMODE_FRAME, animIndex, 15.0);
               m_model->setAnimFrameCount( Model::ANIMMODE_FRAME, 0, fileList.front().numFrames );
            }
         }
      }

      for ( it = fileList.begin(); it != fileList.end(); it++ )
      {
         m_meshVecInfos = (*it).meshVecInfos;
         m_fileBuf = (*it).fileBuf;

         //Animations
         int32_t animIndex = 0;
         if ( (*it).numFrames > 0 )
         {
            setPoints( (*it).section, (*it).offsetTags, (*it).numTags, (*it).numFrames, (*it).tagPoint, animIndex );
            setMeshes( (*it).section, (*it).offsetMeshes, (*it).numMeshes, (*it).tagPoint, animIndex );
         }
      }

      // Set MD3_PATH
      size_t len = m_pathList.size();

      std::string mainStr = "";
      size_t i = 0;
      for ( i = 0; mainStr.empty() && i < len; i++ )
      {
         if ( m_pathList[i].material < 0 )
         {
            mainStr = m_pathList[i].path;
            break;
         }
      }

      model->addMetaData( "MD3_PATH", mainStr.c_str() );

      for ( i = 0; i < len; i++ )
      {
         if ( !m_pathList[i].path.empty()
               && strcasecmp( m_pathList[i].path.c_str(), mainStr.c_str() ) != 0 )
         {
            std::string key = "MD3_PATH_";
            if ( m_pathList[i].material >= 0 )
            {
               const char * name = model->getTextureName(
                     m_pathList[i].material );
               key += name ? name : "";
            }
            else
            {
               switch ( m_pathList[i].section )
               {
                  case MS_Head:
                     key += "head";
                     break;
                  case MS_Upper:
                     key += "upper";
                     break;
                  case MS_Lower:
                     key += "lower";
                     break;
                  default:
                     key += "main";
                     break;
               }
            }

            model->addMetaData( key.c_str(), m_pathList[i].path.c_str() );
         }
      }

      // Clean-up
      for ( it = fileList.begin(); it != fileList.end(); it++ )
      {
         for ( int i = 0; i < (*it).numMeshes; i++ )
         {
            delete[] (*it).meshVecInfos[i];
         }
         delete[] (*it).meshVecInfos;

         delete[] (*it).fileBuf;
      }

      return Model::ERROR_NONE;
   }
   else
   {
      log_error( "no filename supplied for model filter" );
      return Model::ERROR_NO_FILE;
   }
}

uint8_t Md3Filter::readU1()
{
   uint8_t val = 0;
   memcpy( &val, m_bufPos, sizeof(val) );
   m_bufPos += sizeof( val );

   return val;
}

int32_t Md3Filter::readI4()
{
   int32_t val = 0;
   memcpy( &val, m_bufPos, sizeof( val ) );
   m_bufPos += sizeof( val );

   val = ltoh_32( val );

   return val;
}

int16_t Md3Filter::readI2()
{
   int16_t val = 0;
   memcpy( &val, m_bufPos, sizeof( val ) );
   m_bufPos += sizeof( val );

   val = ltoh_16( val );

   return val;
}

int8_t Md3Filter::readI1()
{
   int8_t val = 0;
   memcpy( &val, m_bufPos, sizeof( val ) );
   m_bufPos += sizeof( val );

   return val;
}

float Md3Filter::readF4()
{
   float val = 0;
   memcpy( &val, m_bufPos, sizeof( val ) );
   m_bufPos += sizeof( val );

   val = ltoh_float( val );

   return val;
}

unsigned Md3Filter::readString( char * dest, size_t len )
{
   strncpy( dest, (char *) m_bufPos, len );
   dest[ len - 1 ]= '\0';

   m_bufPos += len;
   return len;
}

bool Md3Filter::readAnimations( bool create )
{
   string animFile = m_modelPath + "animation.cfg";
   FILE * fp = fopen( animFile.c_str(), "r" );
   int animCount = 0;
   int animFrames = 0;
   int animOffset = 0;

   //m_animMap.clear();
   m_animStartFrame.clear();
   m_torsoStart = 0;
   m_legsStart = 0;
   m_standFrame = 0;
   m_idleFrame = 0;

   if ( fp != NULL )
   {
      log_debug( "reading animation.cfg\n" );
      char line[256];
      while ( fgets(line, sizeof(line), fp ) != NULL )
      {
         if ( strncmp( line, "//", 2 ) == 0 )
         {
            // Comment
         }
         else if ( isspace( line[0] ) )
         {
            // Blank
         }
         else
         {
            int first  = 0;
            int fcount = 0;
            int loop   = 0;
            int fps    = 0;
            if ( sscanf( line, "%d %d %d %d",
                     &first, &fcount, &loop, &fps ) == 4 )
            {
               log_debug( "got anim frame details\n" );
               if ( animCount == 6 )
               {
                  m_torsoStart = first;
               }
               else if ( animCount == 11 )
               {
                  m_standFrame = first;
               }
               else if ( animCount == 13 )
               {
                  // Some animation files have the leg frames continuously numbered 
                  // after the torso frames, others number the legs following the
                  // "both" frames. Here we are adjusting the second case to make
                  // it match the first case by using "animOffset"
                  if ( first == m_torsoStart )
                  {
                     animOffset = animFrames - m_torsoStart;
                  }
                  m_legsStart = first + animOffset;
               }
               else if ( animCount == 22 )
               {
                  m_idleFrame = first - (m_legsStart - m_torsoStart) + animOffset;
               }

               first += animOffset;

               if ( first + fcount > animFrames )
               {
                  if ( create )
                  {
                     //m_animMap.push_back( m_model->getAnimCount( Model::ANIMMODE_FRAME ) );
                     m_animStartFrame.push_back( first );

                     char * name = NULL;
                     if ( animCount < MD3_ANIMATIONS )
                     {
                        // I won't change it, I promise
                        name = (char *) s_animNames[ animCount ];
                     }
                     else
                     {
                        name = strrchr( line, '/' );
                        if ( name )
                        {
                           name++;
                           while ( isspace(name[0]) )
                           {
                              name++;
                           }
                           int end = 0;
                           while ( name[end] && !isspace(name[end]) )
                           {
                              end++;
                           }
                           name[end] = '\0';

                           for ( end = 0; line[end]; end++ )
                           {
                              name[end] = tolower( line[end] );
                           }
                        }
                        else
                        {
                           name = "Unknown";
                        }
                     }

                     log_debug( "adding animation '%s'\n", name );
                     int animIndex = m_model->addAnimation( Model::ANIMMODE_FRAME, s_animNames[ animCount ] );
                     m_model->setAnimFPS( Model::ANIMMODE_FRAME, animIndex, (double) fps );
                     m_model->setAnimFrameCount( Model::ANIMMODE_FRAME, animIndex, fcount );
                  }

                  animFrames += fcount;
               }
               else
               {
                  log_debug( "did not add animation for '%s'\n", s_animNames[ animCount ] );
               }
               animCount++;
            }
            else
            {
               if ( create )
               {
                  char * value = NULL;
                  if ( strncasecmp( line, "sex", 3 ) == 0 )
                  {
                     // sex (m,f,n)
                     value = &line[3];
                     while( isspace(value[0]) )
                     {
                        value++;
                     }
                     int end = 0;
                     while ( value[end] && !isspace( value[end] ) )
                     {
                        end++;
                     }
                     value[end] = '\0';

                     m_model->addMetaData( "MD3_sex", value );
                  }
                  else if ( strncasecmp( line, "footsteps", 9 ) == 0 )
                  {
                     // footsteps (normal,boot,flesh,mech)
                     value = &line[9];
                     while( isspace(value[0]) )
                     {
                        value++;
                     }
                     int end = 0;
                     while ( value[end] && !isspace( value[end] ) )
                     {
                        end++;
                     }
                     value[end] = '\0';

                     m_model->addMetaData( "MD3_footsteps", value );
                  }
                  else if ( strncasecmp( line, "headoffset", 10 ) == 0 )
                  {
                     // headoffset %d %d %d
                     value = &line[10];
                     while( isspace(value[0]) )
                     {
                        value++;
                     }
                     int end = 0;
                     while ( value[end] && value[end] != '\r' && value[end] != '\n' )
                     {
                        end++;
                     }
                     value[end] = '\0';

                     m_model->addMetaData( "MD3_headoffset", value );
                  }
                  else if ( strncasecmp( line, "fixedtorso", 10 ) == 0 )
                  {
                     m_model->addMetaData( "MD3_fixedtorso", "" );
                  }
                  else if ( strncasecmp( line, "fixedlegs", 9 ) == 0 )
                  {
                     m_model->addMetaData( "MD3_fixedlegs", "" );
                  }
                  else
                  {
                     log_warning( "Unknown meta data: %s", line );
                  }
               }
            }
         }
      }
      fclose( fp );
      return true;
   }
   return false;
}

void Md3Filter::setMeshes( MeshSectionE section, int32_t offsetMeshes, int32_t numMeshes, int32_t parentTag, int32_t animIndex )
{
   Matrix loadMatrix;

   loadMatrix.setRotationInDegrees( -90, -90, 0 );
   double pos[3] = { 0,0,0 };
   double rot[3] = { 0,0,0 };

   if ( parentTag >= 0 )
   {
      m_model->getPointCoords( parentTag, pos );
      m_model->getPointOrientation( parentTag, rot );

      loadMatrix.loadIdentity();
      loadMatrix.setRotation( rot );
      loadMatrix.setTranslation( pos[0], pos[1], pos[2] );
   }

   // Meshes
   m_bufPos = &m_fileBuf [ offsetMeshes ];
   int32_t meshPos = offsetMeshes;
   for ( int mesh = 0; mesh < numMeshes; mesh++ )
   {
      //Mesh header
      int8_t meshMagic[4];
      for ( int t = 0; t < 4; t++ )
      {
         meshMagic[t]=readI1();
      }

      char meshName[MAX_QPATH];
      readString( meshName, sizeof( meshName ) );

      int32_t meshFlags = readI4();
      int32_t meshFrameCount = readI4();
      int32_t meshShaderCount = readI4();
      int32_t meshVertexCount = readI4();
      int32_t meshTriangleCount = readI4();
      int32_t meshTriangleOffset = readI4();
      int32_t meshShaderOffset = readI4();
      int32_t meshSTOffset = readI4();
      int32_t meshXYZOffset = readI4();
      int32_t meshEndOffset = readI4();

      log_debug( "Mesh %d magic: %c%c%c%c\n", mesh, meshMagic[0], meshMagic[1], meshMagic[2], meshMagic[3] );
      log_debug( "Mesh %d name: %s\n", mesh, meshName );
      log_debug( "Mesh %d flags: %d\n", mesh, meshFlags );
      log_debug( "Mesh %d num_frames: %d\n", mesh, meshFrameCount );
      log_debug( "Mesh %d num_shaders: %d\n", mesh, meshShaderCount );
      log_debug( "Mesh %d num_vertex: %d\n", mesh, meshVertexCount );
      log_debug( "Mesh %d num_triangles: %d\n", mesh, meshTriangleCount );
      log_debug( "Mesh %d triangle offset: %d\n", mesh, meshTriangleOffset );
      log_debug( "Mesh %d shader offset: %d\n", mesh, meshShaderOffset );
      log_debug( "Mesh %d st offset: %d\n", mesh, meshSTOffset );
      log_debug( "Mesh %d xyz offset: %d\n", mesh, meshXYZOffset );
      log_debug( "Mesh %d end offset: %d\n", mesh, meshEndOffset );

      int frameSize = meshVertexCount * 4 * 2;

      //Load first frame
      // Vertex
      m_bufPos = &m_fileBuf [ meshPos + meshXYZOffset ];
      float meshVec[4] = { 0, 0, 0, 1 };
      if ( animIndex < 0 )
      {
         int fileFrame = animToFrame( section, -1, 0 );
         log_debug( "Using frame %d as default for section %d\n", fileFrame, section );
         if ( fileFrame >= meshFrameCount )
         {
            log_error( "mesh %s appears to be missing frame %d for anim %d, using frame %d instead\n", meshName, fileFrame, animIndex, 0 );
            fileFrame = 0;
         }
         m_bufPos = &m_fileBuf [ meshPos + meshXYZOffset + fileFrame * frameSize ];

         m_meshVecInfos[mesh] = new MeshVectorInfoT[meshVertexCount];

         for ( int vert = 0; vert < meshVertexCount; vert++ )
         {
            for ( int n = 0; n < 3; n ++ )
            {
               meshVec[n] = readI2() * MD3_XYZ_SCALE;
            }
            meshVec[3] = 1;
            m_meshVecInfos[mesh][vert].lng = readI1();
            m_meshVecInfos[mesh][vert].lat = readI1();
            //log_debug("normals lat, lng: %d, %d\n", m_meshVecInfos[mesh][vert].lat, m_meshVecInfos[mesh][vert].lng);

            loadMatrix.apply( meshVec );
            m_meshVecInfos[mesh][vert].id = m_model->addVertex( meshVec[0], meshVec[1], meshVec[2] );
         }
      }
      else
      {
         int acount = m_model->getAnimCount( Model::ANIMMODE_FRAME );
         for ( animIndex = 0; animIndex < acount; animIndex++ )
         {
            bool inAnim = animInSection( getSafeName( animIndex), section );
            int fcount = m_model->getAnimFrameCount( Model::ANIMMODE_FRAME, animIndex );
            for ( int frame = 0; frame < fcount; frame++ )
            {
               int fileFrame = animToFrame( section, animIndex, frame );
               if ( fileFrame >= meshFrameCount )
               {
                  log_error( "mesh %s appears to be missing frame %d for anim %d, using frame %d instead\n", meshName, fileFrame, animIndex, meshFrameCount - 1 );
                  fileFrame = meshFrameCount - 1;
               }
               m_bufPos = &m_fileBuf [ meshPos + meshXYZOffset + fileFrame * frameSize ];
               if ( parentTag >= 0 )
               {
                  m_model->getFrameAnimPointCoords( animIndex, frame, parentTag, pos[0], pos[1], pos[2] );
                  m_model->getFrameAnimPointRotation( animIndex, frame, parentTag, rot[0], rot[1], rot[2] );
                  loadMatrix.loadIdentity();
                  loadMatrix.setRotation( rot );
                  loadMatrix.setTranslation( pos[0], pos[1], pos[2] );
               }

               for ( int vert = 0; vert < meshVertexCount; vert++ )
               {
                  if ( inAnim )
                  {
                     for ( int n = 0; n < 3; n ++ )
                     {
                        meshVec[n] = readI2() * MD3_XYZ_SCALE;
                     }
                     m_meshVecInfos[mesh][vert].lng = readI1();
                     m_meshVecInfos[mesh][vert].lat = readI1();
                     //log_debug("normals lat, lng: %d, %d\n", m_meshVecInfos[mesh][vert].lat, m_meshVecInfos[mesh][vert].lng);
                  }
                  else
                  {
                     double coord[3];
                     m_model->getVertexCoords( m_meshVecInfos[mesh][vert].id, coord );
                     meshVec[0] = coord[0];
                     meshVec[1] = coord[1];
                     meshVec[2] = coord[2];
                     meshVec[3] = 1;

                     Matrix invMatrix;
                     invMatrix.loadIdentity();
                     if ( parentTag >= 0 )
                     {
                        m_model->getPointCoords( parentTag, pos );
                        m_model->getPointOrientation( parentTag, rot );

                        invMatrix.setRotation( rot );
                        invMatrix.setTranslation( pos[0], pos[1], pos[2] );
                     }
                     else
                     {
                        invMatrix.setRotationInDegrees( -90, -90, 0 );
                     }

                     invMatrix = invMatrix.getInverse();
                     invMatrix.apply( meshVec );
                  }

                  meshVec[3] = 1;
                  loadMatrix.apply( meshVec );
                  m_model->setFrameAnimVertexCoords( animIndex, frame, m_meshVecInfos[mesh][vert].id, meshVec[0], meshVec[1], meshVec[2] );
               }
            }
         }
      }

      if ( animIndex < 0 )
      {
         // Triangle
         m_bufPos = &m_fileBuf [ meshPos + meshTriangleOffset ];
         int32_t triang[meshTriangleCount][3];
         unsigned tri[meshTriangleCount];
         int32_t groupId = m_model->addGroup( meshName );
         for ( int t = 0; t < meshTriangleCount; t++ )
         {
            for ( int n = 0; n < 3; n++ )
            {
               triang[t][n] = readI4();
            }
            tri[t]= m_model->addTriangle( m_meshVecInfos[mesh][triang[t][2]].id, m_meshVecInfos[mesh][triang[t][1]].id, m_meshVecInfos[mesh][triang[t][0]].id );
            m_model->addTriangleToGroup( groupId, tri[t] );
         }

         //Vertex Texture Coords
         m_bufPos = &m_fileBuf [ meshPos + meshSTOffset ];
         for (int v = 0; v < meshVertexCount; v++ )
         {
            m_meshVecInfos[mesh][v].s = readF4();
            m_meshVecInfos[mesh][v].t = readF4();
         }

         //Textures/Shaders
         vector<Model::Material *> & modelMaterials = getMaterialList( m_model );

         m_bufPos = &m_fileBuf [ meshPos + meshShaderOffset ];
         char shaderName[MAX_QPATH];
         readString(shaderName, sizeof( shaderName ));
         int32_t shaderIndex = readI4();

         replaceBackslash( shaderName );
         string shaderFileName = string( shaderName );
         shaderFileName = getFileNameFromPath( shaderName );
         string skin = m_modelPath+shaderFileName;
         string shaderFullName;
         string shaderFullPath;
         string shaderBaseName;

         Md3PathT mpath;
         mpath.section = section;
         mpath.material = modelMaterials.size();
         mpath.path = extractPath(shaderName);
         m_pathList.push_back( mpath );
         m_lastMd3Path = mpath.path;

         normalizePath( skin.c_str(), shaderFullName, shaderFullPath, shaderBaseName );
         log_debug( "Shader Name: %s\n", shaderName );
         log_debug( "Shader Index: %d\n", shaderIndex );
         log_debug( "Shader FileName: %s\n", shaderFileName.c_str() );
         log_debug( "Shader full name: %s\n", shaderFullName.c_str() );
         log_debug( "Shader full path: %s\n", shaderFullPath.c_str() );
         log_debug( "Shader base name: %s\n", shaderBaseName.c_str() );

         string textureFile = m_modelPath+shaderBaseName;
         textureFile = fixAbsolutePath( m_modelPath.c_str(), textureFile.c_str() );
         textureFile = getAbsolutePath( m_modelPath.c_str(), textureFile.c_str() );
         log_debug( "textureFile = %s\n", textureFile.c_str() );
         bool textureFound = false;
         int matId = -1;
         //First check for skins
         matId = setSkins( meshName );
         if (matId >= 0)
         {
            textureFound = true;
         }

         if ( ! textureFound )
         {
            textureFile = findTexture( shaderBaseName, shaderName );
            if ( textureFile.size() > 0)
            {
               //If we already loaded this texture before
               int checkId = materialsCheck( textureFile );
               if (checkId >= 0)
               {
                  matId=checkId;
                  textureFound = true;
               }
               else
               {
                  textureFound = true;
                  Model::Material * mat = Model::Material::get();
                  mat->m_name = shaderBaseName;
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
                  mat->m_alphaFilename = "";
                  mat->m_name = getFileNameFromPath( textureFile.c_str() );
                  mat->m_filename = textureFile;
                  modelMaterials.push_back( mat );
                  matId=modelMaterials.size()-1;
               }
            }
         }
         if (textureFound)
         {
            log_debug( "skin : '%s'\n", textureFile.c_str() );
            m_model->setGroupTextureId( groupId, matId );
         }

         //Texture Mapping
         //
         // Add texture coordinates even if we didn't find the texture
         // (user may load it manually)
         for ( int t = 0; t < meshTriangleCount; t++ )
         {
            m_model->setTextureCoords( tri[t], 0, m_meshVecInfos[mesh][triang[t][2]].s, 1.0-m_meshVecInfos[mesh][triang[t][2]].t );
            m_model->setTextureCoords( tri[t], 1, m_meshVecInfos[mesh][triang[t][1]].s, 1.0-m_meshVecInfos[mesh][triang[t][1]].t );
            m_model->setTextureCoords( tri[t], 2, m_meshVecInfos[mesh][triang[t][0]].s, 1.0-m_meshVecInfos[mesh][triang[t][0]].t );
         }
      }

      //Goto end
      meshPos = meshPos + meshEndOffset;
      log_debug( "mesh position is %d\n", meshPos );
      m_bufPos = &m_fileBuf [ meshPos ];
   }
   return;
}

//Give it file basename and it will find a texture like it.
//Returns full path
std::string Md3Filter::findTexture( std::string baseName, std::string shaderFullName )
{
   //Lets go through everything see if we could find a texture
   if ( baseName.size() <= 0 || strcmp( baseName.c_str(), "." ) == 0 )
   {
      return "";
   }
   TextureManager * texmgr = TextureManager::getInstance();
   //log_debug( "looking for texture files with name like: %s\n", baseName.c_str() );
   char * noext = strdup( baseName.c_str() );

   char * ext = strrchr( noext, '.' );
   if ( ext )
   {
      ext[0] = '\0';
   }
   list<string> files;
   getFileList( files, m_modelPath.c_str(), noext );
   list<string>::iterator it;

   free( noext );

   // try current directory
   for ( it = files.begin(); it != files.end(); it++ )
   {
      string texturePath = m_modelPath + (*it);
      texturePath = getAbsolutePath( m_modelPath.c_str(), texturePath.c_str() );

      //log_debug( "checking %s\n", texturePath.c_str() );
      if ( texmgr->getTexture( texturePath.c_str() ) )
      {
         //log_debug( "  %s is a skin\n", texturePath.c_str() );
         return texturePath;
      }
   }

   // try shader path
   if ( !m_lastMd3Path.empty() )
   {
      size_t len = m_lastMd3Path.size();
      if ( m_modelPath.size() > len )
      {
         std::string path = m_modelPath;
         path.resize( path.size() - len );
         path += shaderFullName;

         return path;
      }
   }

   return shaderFullName;
}

//If a material with filename already exists reutrn id, otherwise -1
int32_t Md3Filter::materialsCheck( std::string textureFullName )
{
   vector<Model::Material *> & modelMaterials = getMaterialList( m_model );
   for ( unsigned i = 0; i < modelMaterials.size(); i++ )
   {
      if ( strcmp( modelMaterials[i]->m_filename.c_str(), textureFullName.c_str() ) == 0 )
      {
         return i;
      }
   }
   return -1;
}

//Will load all the textures in a skin file for a meshName
//it will return the default material.
int32_t Md3Filter::setSkins( char *meshName )
{
   if ( strlen(meshName) < 0 )
   {
      log_debug( "setSkins() no meshName.\n" );
      return -1;
   }

   int matId = -1;

   vector<Model::Material *> & modelMaterials = getMaterialList( m_model );

   //Find all the skin files
   list<string> files;
   list<string>::iterator it;

   char * noext = strdup( m_modelBaseName.c_str() );
   char * ext = strrchr( noext, '.' );
   if ( ext )
   {
      ext[0] = '\0';
   }
   char *base = (char *) malloc( sizeof( char ) * ( strlen(noext)+2 ) );
   strcpy( base, noext );
   strcat( base, "_" );

   getFileList( files, m_modelPath.c_str(), base );
   bool defaultSet = false;
   for ( it = files.begin(); it != files.end(); it++ )
   {
      bool isDefault = false;
      string fileName=(*it);
      string fullName=m_modelPath + fileName;
      fullName = getAbsolutePath( m_modelPath.c_str(), fullName.c_str() );

      //Only take the ones with .skin extension
      noext = strdup( fileName.c_str() );
      ext = strrchr( noext, '.' );
      if ( PORT_strcasestr( fileName.c_str(), "_default.skin" ) != NULL )
      {
         log_debug( "is default\n" );
         isDefault = true;
      }
      if ( strcmp( ext, ".skin" ) == 0 )
      {
         FILE * fp = fopen( fullName.c_str(), "rb" );
         if ( fp == NULL )
         {
            log_error( "%s: file does not exist\n", fileName.c_str() );
            continue;
         }
         if ( errno == EPERM )
         {
            log_error( "%s: access denied\n", fileName.c_str() );
            continue;
         }
         fseek( fp, 0, SEEK_END );
         unsigned fileLength = ftell( fp );
         fseek( fp, 0, SEEK_SET );
         if ( fileLength <= 0 )
         {
            log_error( "%s: empty file\n", fileName.c_str() );
            fclose( fp );
            continue;
         }
         char *skinBuf = (char *)malloc( fileLength );

         if ( fread( skinBuf, fileLength, 1, fp ) != 1 )
         {
            delete[] skinBuf;
            log_error( "%s: could not read file\n", fileName.c_str() );
            fclose( fp );
            free( skinBuf );
            continue;
         }
         fclose( fp );
         //XXX: There has got to be a better way to do this, maybe ifstream?
         char *sep = (char *) malloc( sizeof( char ) * ( strlen( meshName ) + 2 ) );
         strcpy( sep, meshName );
         strcat( sep, "," );
         char * line = (char *) PORT_strcasestr( skinBuf, (char *) meshName);
         char * file;
         if ( line )
         {
            char *nl = strchr( line, '\n' );
            if ( nl )
            {
               nl[0] = '\0';
               nl = strchr( line, '\r' );
               if ( nl )
               {
                  nl[0] = '\0';
               }
            }
            file = strchr( line, ',' );
            if ( file && strlen( file ) > 1 )
            {
               file++;
               log_debug( "texture file is: %s\n", file );
            }
            else
            {
               continue;
            }
         }
         else
         {
            continue;
         }
         //Whew! we have the file now lets load it up.
         replaceBackslash( file );
         string textureBaseName = getFileNameFromPath( file );
         string textureFullName = findTexture( textureBaseName, file );
         if ( textureFullName.size() > 0 )
         {
            int checkId = materialsCheck(textureFullName);
            //If already loaded
            if ( checkId >= 0 )
            {
               if ( ! defaultSet )
               {
                  matId = checkId;
                  if ( isDefault )
                  {
                     defaultSet=true;
                  }
               }
            }
            else
            {
               Model::Material * mat = Model::Material::get();
               mat->m_name = textureBaseName;
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
               mat->m_filename = textureFullName;
               mat->m_alphaFilename = "";
               mat->m_name = getFileNameFromPath( textureFullName.c_str() );
               modelMaterials.push_back( mat );
               if ( ! defaultSet )
               {
                  matId = modelMaterials.size()-1;
                  if ( isDefault )
                  {
                     defaultSet=true;
                  }
               }
            }
         }
         free( skinBuf );
      }
   }

   free( base );
   return matId;
}

//If animIndex < 0 it will setup the initial points
//otherwise it will set the position.
void Md3Filter::setPoints( MeshSectionE section, int32_t offsetTags, int32_t numTags, int32_t numFrames, int32_t parentTag, int32_t animIndex )
{
   Matrix loadMatrix;

   loadMatrix.setRotationInDegrees( -90, -90, 0 );
   double pos[3] = { 0,0,0 };
   double rot[3] = { 0,0,0 };

   if ( parentTag >= 0 )
   {
      m_model->getPointCoords( parentTag, pos );
      m_model->getPointOrientation( parentTag, rot );

      loadMatrix.loadIdentity();
      loadMatrix.setRotation( rot );
      loadMatrix.setTranslation( pos[0], pos[1], pos[2] );
   }

   const int TAG_SIZE = (64 + 3*4 + 9*4 );
   int frameSize = numTags * TAG_SIZE;

   // Tags
   int animCount = m_model->getAnimCount( Model::ANIMMODE_FRAME );
   if ( animIndex < 0 )
   {
      animCount = 0;
   }

   for ( animIndex = animIndex; animIndex < animCount; animIndex++ )
   {
      int frameCount = m_model->getAnimFrameCount( Model::ANIMMODE_FRAME, animIndex );
      if ( animIndex < 0 )
      {
         frameCount = 1;
      }

      for ( int f = 0; f < frameCount; f++ )
      {
         int fileFrame = animToFrame( section, animIndex, f );
         if ( animIndex < 0 )
         {
            log_debug( "Using frame %d as default for tag section %d\n", fileFrame, section );
         }
         if ( fileFrame >= numFrames )
         {
            log_error( "tag section appears to be missing frame %d for anim %d, using frame %d instead\n", fileFrame, animIndex, numFrames - 1 );
            fileFrame = numFrames - 1;
         }
         /*
         log_debug( "section %d anim %d frame %d is file frame %d\n",
               section, animIndex, f, fileFrame );
         */
         m_bufPos = &m_fileBuf [ offsetTags + (fileFrame * frameSize )];
         if ( animIndex >= 0 && parentTag >= 0 )
         {
            m_model->getFrameAnimPointCoords( animIndex, f, parentTag, pos[0], pos[1], pos[2] );
            m_model->getFrameAnimPointRotation( animIndex, f, parentTag, rot[0], rot[1], rot[2] );
            loadMatrix.loadIdentity();
            loadMatrix.setRotation( rot );
            loadMatrix.setTranslation( pos[0], pos[1], pos[2] );
         }

         for ( int i = 0; i < numTags; i++ )
         {
            char tagName[MAX_QPATH];
            readString( tagName, sizeof( tagName ) );
            double posVector[3];
            for ( int t = 0; t < 3; t++ )
            {
               posVector[t] = readF4();
            }

            Matrix curMatrix;
            double rotVector[3];
            curMatrix.setTranslation( posVector );
            for ( int t = 0; t < 3; t++ )
            {
               for ( int s = 0; s < 3; s++ )
               {
                  curMatrix.set( t, s, readF4() );
               }
            }
            curMatrix = curMatrix * loadMatrix;

            curMatrix.getRotation( rotVector );
            curMatrix.getTranslation( posVector );

            if ( animIndex < 0 )
            {
               // Only add the point if we don't already have one of the same name
               int p = m_model->getPointByName( tagName );
               if ( p < 0 )
               {
                  m_model->addPoint( tagName, posVector[0], posVector[1], posVector[2], rotVector[0], rotVector[1], rotVector[2] );
               }
            }
            else
            {
               int p = m_model->getPointByName( tagName );
               if ( p != parentTag )
               {
                  m_model->setFrameAnimPointCoords( animIndex, f, p, posVector[0], posVector[1], posVector[2] );
                  m_model->setFrameAnimPointRotation( animIndex, f, p, rotVector[0], rotVector[1], rotVector[2] );
               }
            }
         }
      }
   }

   return;
}

int Md3Filter::animToFrame( MeshSectionE section, int anim, int frame )
{
   if ( anim < 0 )
   {
      // Not an animation, use first frame
      switch ( section )
      {
         case MS_Lower:
            return m_idleFrame;
            break;
         case MS_Upper:
            return m_standFrame;
            break;
         default:
            break;
      }
      return 0;
   }

   if ( !animInSection( getSafeName( anim ), section ) )
   {
      // Not valid for this section, use first frame
      switch ( section )
      {
         case MS_Lower:
            return m_idleFrame;
            break;
         case MS_Upper:
            return m_standFrame;
            break;
         default:
            break;
      }
      return 0;
   }

   if ( anim >= (int) m_animStartFrame.size() )
   {
      // Not a valid animation
      return frame;
   }

   int fileFrame = m_animStartFrame[anim] + frame;
   switch ( section )
   {
      case MS_None:
         // Not a multi-MD3 model, use specified frame
         return frame;

      case MS_Lower:
         if ( fileFrame > m_torsoStart )
         {
            fileFrame -= (m_legsStart - m_torsoStart);
         }
         return fileFrame;

      case MS_Upper:
         return fileFrame;

      case MS_Head:
      default:
         // No head animations, use first frame
         return 0;
   }
   return frame;
}

bool Md3Filter::animInSection( std::string animName, MeshSectionE section )
{
   if ( section == MS_Head )
   {
      return false;
   }

   if ( section == MS_None )
   {
      // Uh... sure...
      return true;
   }

   if ( strncasecmp( animName.c_str(), "torso_", 6 ) == 0 )
   {
      if ( section == MS_Upper )
         return true;
      else
         return false;
   }
   if ( strncasecmp( animName.c_str(), "legs_", 5 ) == 0 )
   {
      if ( section == MS_Lower )
         return true;
      else
         return false;
   }

   // It's a "both_" animation, or something weird
   if ( strncasecmp( animName.c_str(), "both_", 5 ) == 0 )
      return true;
   else
      return false;
}

bool Md3Filter::groupInSection( std::string groupName, MeshSectionE section )
{
   if ( section == MS_None )
   {
      // not a player model, anything goes
      return true;
   }

   if ( groupName[1] == '_' )
   {
      switch ( toupper( groupName[0] ) )
      {
         case 'U':
            if ( section == MS_Upper )
            {
               return true;
            }
            break;
         case 'L':
            if ( section == MS_Lower )
            {
               return true;
            }
            break;
         case 'H':
            if ( section == MS_Head )
            {
               return true;
            }
            break;
         default:
            break;
      }
   }

   return false;
}

bool Md3Filter::tagInSection( std::string tagName, MeshSectionE section )
{
   if ( section == MS_None )
   {
      // not a player model, anything goes
      return true;
   }

   if ( strcasecmp( tagName.c_str(), "tag_torso" ) == 0 )
   {
      if ( section != MS_Head )
      {
         return true;
      }
   }
   else if ( strcasecmp( tagName.c_str(), "tag_head" ) == 0 )
   {
      if ( section != MS_Lower )
      {
         return true;
      }
   }
   else if ( strcasecmp( tagName.c_str(), "tag_weapon" ) == 0 )
   {
      if ( section == MS_Upper )
      {
         return true;
      }
   }

   return false;
}

bool Md3Filter::tagIsSectionRoot( std::string tagName, MeshSectionE section )
{
   switch ( section )
   {
      case MS_Upper:
         if ( strcasecmp( tagName.c_str(), "tag_torso" ) == 0 )
         {
            return true;
         }
         break;

      case MS_Head:
         if ( strcasecmp( tagName.c_str(), "tag_head" ) == 0 )
         {
            return true;
         }
         break;

      case MS_Lower:
      case MS_None:
         // No root tag
         break;

      default:
         break;
   }

   return false;
}

std::string Md3Filter::getSafeName( unsigned int anim )
{
   std::string animName = "none";

   if ( anim < m_model->getAnimCount( Model::ANIMMODE_FRAME ) )
   {
      animName = m_model->getAnimName( Model::ANIMMODE_FRAME, anim );
   }

   return animName;
}
 
Model::ModelErrorE Md3Filter::writeFile( Model * model, const char * const filename, ModelFilter::Options *  )
{
   if ( model && filename && filename[0] )
   {
      if ( model->getGroupCount() == 0 )
      {
         model->setFilterSpecificError( "MD3 export requires faces to be grouped." );
         return Model::ERROR_FILTER_SPECIFIC;
      }

      m_model = model;

      MeshList meshes;

      // MD3 does not allow a single vertex to have more than one texture
      // coordinate or normal. MM3D does. The mesh_create_list function will
      // break the model up into meshes where vertices meet the MD3 criteria.
      // See mesh.h for details.
      mesh_create_list( meshes, model );

      string modelPath = "";
      string modelBaseName = "";
      string modelFullName = "";

      normalizePath( filename, modelFullName, modelPath, modelBaseName );

      m_modelPath = modelPath;

      bool saveAsPlayer = true;

      bool haveUpper = false;
      bool haveLower = false;

      unsigned gcount = m_model->getGroupCount();
      for ( unsigned g = 0; g < gcount; g++ )
      {
         std::string name = m_model->getGroupName(g);
         if ( name[1] == '_' )
         {
            switch ( toupper(name[0]) )
            {
               case 'U':
                  haveUpper = true;
                  break;
               case 'L':
                  haveLower = true;
                  break;
               default:
                  break;
            }
         }
      }

      if ( haveUpper && haveLower )
      {
         if (  m_model->getPointByName( "tag_torso" )  < 0
            || m_model->getPointByName( "tag_weapon" ) < 0
            || m_model->getPointByName( "tag_head" )   < 0 )
         {
            // missing required tags for player model
            saveAsPlayer = false;
         }
      }
      else
      {
         // don't have the proper groups to save as a player model
         saveAsPlayer = false;
      }

      if ( saveAsPlayer )
      {
         if (     strncasecmp( modelBaseName.c_str(), "lower.", 6 ) == 0 
               || strncasecmp( modelBaseName.c_str(), "upper.", 6 ) == 0 
               || strncasecmp( modelBaseName.c_str(), "head.",  5 ) == 0 )
         {
            log_debug( "filename %s looks like a player model\n", modelBaseName.c_str() );
            char value[20];
            if ( model->getMetaData( "MD3_composite", value, sizeof(value)) )
            {
               if ( atoi(value) == 0 )
               {
                  log_debug( "model is explicitly not a composite\n" );
                  saveAsPlayer = false;
               }
            }
            else
            {
               // TODO: may want to prompt instead of assuming "yes"
               log_debug( "model is implicitly a composite (no composite meta tag)\n" );
            }
         }
         else
         {
            log_debug( "filename %s does not look like a player model\n", modelBaseName.c_str() );
            saveAsPlayer = false;
         }
      }

      if ( saveAsPlayer )
      {
         log_debug( "saving as a player model\n" );

         std::string playerFile;
         std::string path = modelPath + "/";

         playerFile = path + fixFileCase( m_modelPath.c_str(), "lower.md3" );
         writeSectionFile( playerFile.c_str(), MS_Lower, meshes );

         playerFile = path + fixFileCase( m_modelPath.c_str(), "upper.md3" );
         writeSectionFile( playerFile.c_str(), MS_Upper, meshes );

         playerFile = path + fixFileCase( m_modelPath.c_str(), "head.md3" );
         writeSectionFile( playerFile.c_str(), MS_Head,  meshes );

         writeAnimations();

         return Model::ERROR_NONE;
      }
      else
      {
         log_debug( "saving as a single model\n" );
         return writeSectionFile( filename, MS_None, meshes );
      }
   }
   else
   {
      log_error( "no filename supplied for model filter\n" );
      return Model::ERROR_NO_FILE;
   }
}

Model::ModelErrorE Md3Filter::writeSectionFile( const char * filename, Md3Filter::MeshSectionE section, MeshList & meshes )
{
   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   log_debug( "writing section file %s\n", filename );
   switch ( section )
   {
      case MS_None:
         log_debug( "  writing all data as one section\n" );
         break;
      case MS_Lower:
         log_debug( "  writing lower section\n" );
         break;
      case MS_Upper:
         log_debug( "  writing upper section\n" );
         break;
      case MS_Head:
         log_debug( "  writing head section\n" );
         break;
      default:
         log_debug( "  writing unknown section\n" );
         break;
   }

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   //MD3 HEADER
   int8_t magic[4];
   magic[0]='I';
   magic[1]='D';
   magic[2]='P';
   magic[3]='3';
   int32_t version = MD3_VERSION;
   char pk3Name[MAX_QPATH];
   std::string pk3Path = "";
   memset( pk3Name, 0, MAX_QPATH );

   pk3Path = sectionToPath( section );
   if ( !pk3Path.empty() 
         && pk3Path[pk3Path.size() - 1] != '/' 
         && pk3Path.size() < ( MAX_QPATH+1 ) )
   {
      pk3Path += "/";
   }
   if ( PORT_snprintf( pk3Name, sizeof( pk3Name ), "%s%s",
            pk3Path.c_str(), modelBaseName.c_str() ) >= MAX_QPATH )
   {
      log_error( "MD3_PATH+filename is too large\n" );
      m_model->setFilterSpecificError( "MD3_PATH+filename is to long." );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   int32_t flags=0;
   int32_t numFrames = 0;
   //We are making all the anims be one anim.
   unsigned animCount = m_model->getAnimCount( Model::ANIMMODE_FRAME );
   for ( unsigned i = 0; i < animCount; i++ )
   {
      // Skip animations that don't belong in this section
      std::string name = getSafeName( i );
      if ( animInSection( name, section ) )
      {
         numFrames += m_model->getAnimFrameCount( Model::ANIMMODE_FRAME, i );
      }
   }

   if ( numFrames > MD3_MAX_FRAMES )
   {
      log_error( "Number of frames(%d) is larger than %d\n.\n", numFrames, MD3_MAX_FRAMES );
      m_model->setFilterSpecificError( "Too many animation frames for MD3 export." );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   if ( animCount == 0 )
   {
      animCount = 1;
   }
   if ( numFrames == 0 )
   {
      numFrames = 1;
   }

   unsigned pcount = m_model->getPointCount();
   int32_t numTags = (int32_t) pcount;
   switch ( section )
   {
      case MS_Head:
      case MS_Lower:
         numTags = 1;
         break;
      case MS_Upper:
         numTags = 3;
         break;
      default:
         break;
   }

   MeshList::iterator mlit;

   int32_t numMeshes = 0;
   int numTris = 0;
   int numVerts = 0;

   std::string groupName;

   for ( mlit = meshes.begin(); mlit != meshes.end(); mlit++ )
   {
      int g = (*mlit).group;

      // only grouped meshes are counted
      if ( g >= 0 )
      {
         if ( groupInSection( m_model->getGroupName( (*mlit).group ), section ) )
         {
            numMeshes++;
            numTris += (*mlit).faces.size();
            numVerts += (*mlit).vertices.size();
         }
      }
   }

   int32_t numSkins = 0;
   if ( numTags > MD3_MAX_TAGS )
   {
      log_error( "Number of tags(%d) is larger than %d\n.\n", numTags, MD3_MAX_TAGS );
      m_model->setFilterSpecificError( "Too many points for MD3 export." );
      return Model::ERROR_FILTER_SPECIFIC;
   }
   if ( numMeshes > MD3_MAX_SURFACES )
   {
      log_error( "Number of groups(%d) is larger than %d\n.\n", numMeshes, MD3_MAX_SURFACES );
      m_model->setFilterSpecificError( "Too many groups for MD3 export." );
      return Model::ERROR_FILTER_SPECIFIC;
   }
   // numSkins is usually zero for MD3 header, there can be skins for each mesh though later
   int32_t offsetFrames = HEADER_SIZE;
   int32_t offsetTags = offsetFrames + numFrames * FRAME_SIZE;
   int32_t offsetMeshes = offsetTags + numFrames * numTags * TAG_SIZE;
   int32_t offsetEnd = offsetMeshes;

   // MD3 limit tests
   if ( numTris > MD3_MAX_TRIANGLES )
   {
      log_error( "Number of triangles(%d) is larger than %d\n.\n", numTris, MD3_MAX_TRIANGLES );
      m_model->setFilterSpecificError( "MD3 export can't support that many faces per group." );
      return Model::ERROR_FILTER_SPECIFIC;
   }
   if ( numVerts > MD3_MAX_VERTS )
   {
      log_error( "Number of verticies(%d) is larger than %d\n.\n", numVerts, MD3_MAX_VERTS );
      m_model->setFilterSpecificError( "MD3 export can't support that many verticies per group." );
      return Model::ERROR_FILTER_SPECIFIC;
   }

   // Open file for writing
   m_fpOut = fopen ( filename, "wb" );
   if ( m_fpOut == NULL )
   {
      if ( errno == ENOENT )
      {
         log_error( "%s: file could not be created\n", filename );
         return Model::ERROR_NO_FILE;
      }
      if ( errno == EPERM )
      {
         log_error( "%s: access denied\n", filename );
         return Model::ERROR_NO_ACCESS;
      }

      log_error( "%s: could not open file\n", filename );
      return Model::ERROR_FILE_OPEN;
   }

   // write file header
   write( magic[0] );
   write( magic[1] );
   write( magic[2] );
   write( magic[3] );
   write( version );
   writeS( pk3Name, MAX_QPATH );
   write( flags );
   write( numFrames );
   write( numTags );
   write( numMeshes );
   write( numSkins );
   write( offsetFrames );
   write( offsetTags );
   write( offsetMeshes );

   size_t endPos = ftell( m_fpOut );
   write( offsetEnd );

   int rootTag = -1;
   // Change save matrix if needed
   log_debug( "finding root tag for section %s\n", modelBaseName.c_str() );
   for ( unsigned p = 0; p < pcount; p++ )
   {
      if ( tagIsSectionRoot( m_model->getPointName(p), section ) )
      {
         log_debug( "  root tag is %s\n", m_model->getPointName(p) );
         rootTag = p;
      }
   }

   // FRAMES
   log_debug( "writing frames at %d/%d\n", offsetFrames, ftell(m_fpOut) );
   unsigned a;

   for ( a = 0; a < animCount; a++ )
   {
      if ( animInSection( getSafeName( a ), section ) 
            || (section == MS_Head && a == 0) )
      {
         unsigned aFrameCount = m_model->getAnimFrameCount( Model::ANIMMODE_FRAME, a );
         if ( (aFrameCount == 0 && animCount == 1)
               || (section == MS_Head) )
         {
            aFrameCount = 1;
         }
         for ( unsigned t = 0; t < aFrameCount; t++ )
         {
            Matrix saveMatrix = getMatrixFromPoint( a, t, rootTag ).getInverse();
            list<int>::iterator vit;
            double max[4] = { DBL_MIN, DBL_MIN, DBL_MIN, 1 };
            double min[4] = { DBL_MAX, DBL_MAX, DBL_MAX, 1 };
            for ( mlit = meshes.begin(); mlit != meshes.end(); mlit++ )
            {
               int i = (*mlit).group;
               if ( i >= 0 && groupInSection( m_model->getGroupName( i ), section ) )
               {
                  list<int> tris = m_model->getGroupTriangles( i );
                  list<int>::iterator it;
                  for ( it = tris.begin(); it != tris.end(); it++ )
                  {
                     for ( int n = 0; n < 3; n++ )
                     {
                        int vertex = m_model->getTriangleVertex( *it, n );
                        double cords[3];
                        m_model->getFrameAnimVertexCoords( a, t, vertex, cords[0], cords[1], cords[2] );
                        max[0] = greater( max[0], cords[0] );
                        max[1] = greater( max[1], cords[1] );
                        max[2] = greater( max[2], cords[2] );
                        min[0] = smaller( min[0], cords[0] );
                        min[1] = smaller( min[1], cords[1] );
                        min[2] = smaller( min[2], cords[2] );
                     }
                  }
               }
            }
            saveMatrix.apply( min );
            saveMatrix.apply( max );

            //min_bounds
            for ( int v = 0; v < 3; v++ )
            {
               write( (float) min[v] );
            }
            //max_bounds
            for ( int v = 0; v < 3; v++ )
            {
               write( (float) max[v] );
            }
            //local_origin
            float temp = 0;
            for ( int v = 0; v < 3; v++ )
            {
               write( temp );
            }
            //radius
            double radiusm = sqrt( min[0] * min[0] + min[1] * min[1] + min[2] * min[2] );
            double radius = sqrt( max[0] * max[0] + max[1] * max[1] + max[2] * max[2] );
            if ( radiusm > radius )
            {
               radius=radiusm;
            }
            //log_debug( "Frame radius: %f\n", ( (float) radius ) );
            write( (float) radius );
            char name[16] = "Misfit Model 3D"; // this is what other exporters do
            writeS( name, 16 );
         }
      }
   }

   //TAGS
   log_debug( "writing tags at %d/%d\n", offsetTags, ftell(m_fpOut) );

   for ( a = 0; a < animCount; a++ )
   {
      if ( animInSection( getSafeName( a ), section ) 
            || (section == MS_Head && a == 0) )
      {
         unsigned aFrameCount = m_model->getAnimFrameCount( Model::ANIMMODE_FRAME, a );
         if ( (aFrameCount == 0 && animCount == 1)
               || (section == MS_Head) )
         {
            aFrameCount = 1;
         }
         for ( unsigned t = 0; t < aFrameCount; t++ )
         {
            Matrix saveMatrix = getMatrixFromPoint( a, t, rootTag ).getInverse();
            for ( unsigned j = 0; j < pcount; j++ )
            {
               if ( tagInSection( m_model->getPointName(j), section ) )
               {
                  char tName[MAX_QPATH];
                  memset( tName, 0, MAX_QPATH );
                  if ( PORT_snprintf( tName, sizeof( tName ), "%s", m_model->getPointName( j ) ) >= MAX_QPATH )
                  {
                     fclose( m_fpOut );
                     log_error( "Point name is too large\n" );
                     m_model->setFilterSpecificError( "Point name is too large for MD3 export." );
                     return Model::ERROR_FILTER_SPECIFIC;
                  }
                  writeS( tName, MAX_QPATH );

                  // origin
                  double origin[4] = { 0, 0, 0, 1 };
                  m_model->getFrameAnimPointCoords( a, t, j, origin[0], origin[1], origin[2] );

                  saveMatrix.apply( origin );

                  write( (float) origin[0] );
                  write( (float) origin[1] );
                  write( (float) origin[2] );

                  Matrix rotMatrix;
                  double rotVector[3];
                  m_model->getFrameAnimPointRotation( a, t, j, rotVector[0], rotVector[1], rotVector[2] );

                  //TODO: Seems whenver we have a nan its from a identity matrix
                  if ( rotVector[0] != rotVector[0] || rotVector[1] != rotVector[1] || rotVector[2] != rotVector[2] )
                  {
                     writeIdentity();
                     continue;
                  }
                  rotMatrix.setRotation( rotVector );
                  rotMatrix=rotMatrix*saveMatrix;

                  // orientation
                  for ( int m = 0; m < 3; m++ )
                  {
                     for ( int n = 0; n < 3; n++ )
                     {
                        write( (float) rotMatrix.get( m, n ) );
                     }
                  }
               }
            }
         }
      }
   }

   vector<Model::Material *> & modelMaterials = getMaterialList( m_model );

   // MESHES
   log_debug( "writing meshes at %d/%d\n", offsetMeshes, ftell(m_fpOut) );

   for ( mlit = meshes.begin(); mlit != meshes.end(); mlit++ )
   {
      if ( (*mlit).group >= 0 && groupInSection( m_model->getGroupName( (*mlit).group ), section ) )
      {
         // MESH HEADER
         int8_t mMagic[4];
         mMagic[0]='I';
         mMagic[1]='D';
         mMagic[2]='P';
         mMagic[3]='3';
         char mName[MAX_QPATH];
         memset( mName, 0, MAX_QPATH );
         if ( PORT_snprintf( mName, sizeof( mName ), "%s", m_model->getGroupName( (*mlit).group ) ) > MAX_QPATH )
         {
            fclose( m_fpOut );
            log_error( "group name is too large\n" );
            m_model->setFilterSpecificError( "Group name is too large for MD3 export." );
            return Model::ERROR_FILTER_SPECIFIC;
         }

         const int TRI_SIZE = 3 * 4;
         const int SHADER_SIZE = (MAX_QPATH + 4);
         const int TEXCOORD_SIZE = 2 * 4;
         const int VERT_SIZE = 4 * 2;

         int32_t mFlags       = 0;
         int32_t mNumFrames   = numFrames;
         int32_t mNumShaders  = 1;
         int32_t mNumVerts    = (*mlit).vertices.size();
         int32_t mNumTris     = (*mlit).faces.size();
         int32_t mOffTris     = HEADER_SIZE;
         int32_t mOffShaders  = mOffTris + mNumTris * TRI_SIZE;
         int32_t mOffST       = mOffShaders + mNumShaders * SHADER_SIZE;
         int32_t mOffVerts    = mOffST + mNumVerts * TEXCOORD_SIZE;
         int32_t mOffEnd      = mOffVerts + mNumFrames * mNumVerts * VERT_SIZE;

         // write header
         write( mMagic[0] );
         write( mMagic[1] );
         write( mMagic[2] );
         write( mMagic[3] );
         writeS( mName, MAX_QPATH );
         write( mFlags );
         write( mNumFrames );
         write( mNumShaders );
         write( mNumVerts );
         write( mNumTris );
         write( mOffTris );
         write( mOffShaders );
         write( mOffST );
         write( mOffVerts );
         write( mOffEnd );

         // TRIANGLES
         Mesh::FaceList::iterator fit;
         for ( fit = (*mlit).faces.begin(); fit != (*mlit).faces.end(); fit++ )
         {
            for ( int j = 2; j >= 0; j-- )
            {
               write( (*fit).v[j] );
            }
         }

         // SHADERS
         for ( int32_t t = 0; t < mNumShaders; t++ )
         {
            int matId = m_model->getGroupTextureId( (*mlit).group );
            string matFileName;
            string matFullName;
            string matPath;
            string matBaseName;
            if ( matId != -1 )
            {
               Model::Material * mat = modelMaterials[matId];
               matFileName = mat->m_filename;
            }
            else
            {
               //Texture isn't set
               matFileName = mName;
               matFileName += ".tga";
            }

            char sName[MAX_QPATH];
            std::string spk3Path;
            memset( sName, 0, MAX_QPATH );

            if ( matId >= 0 )
            {
               spk3Path = materialToPath( matId );
               if ( !spk3Path.empty() 
                     && spk3Path[spk3Path.size() - 1] != '/' 
                     && spk3Path.size() < ( MAX_QPATH+1 ) )
               {
                  spk3Path += "/";
               }
            }

            normalizePath( matFileName.c_str(), matFullName, matPath, matBaseName );

            log_debug( "comparing %s and %s\n", matFullName.c_str(), m_modelPath.c_str() );
            if ( strncmp( matFullName.c_str(), m_modelPath.c_str(), m_modelPath.size() ) == 0)
            {
               log_debug( "path is common, using MD3_PATH\n" );
               // model path is the same as texture file path, remove model
               // path and prepend MD3_PATH
               if ( PORT_snprintf( sName, sizeof( sName ), "%s%s",
                        spk3Path.c_str(), matBaseName.c_str() ) >= MAX_QPATH )
               {
                  fclose( m_fpOut );
                  log_error( "MD3_PATH+texture_filename is to long.\n" );
                  m_model->setFilterSpecificError( "Texture filename is too long." );
                  return Model::ERROR_FILTER_SPECIFIC;
               }
            }
            else if ( pathIsAbsolute( matFileName.c_str() ) )
            {
               log_debug( "path is not common, but is absolute\n" );
               // model path is not the same as texture file path, try to
               // remove pk3 path from model and try again
               std::string common;
               common = m_modelPath;

               // default to PK3 Path
               PORT_snprintf( sName, sizeof( sName ), "%s%s",
                     spk3Path.c_str(), matBaseName.c_str() );
            }
            else
            {
               log_debug( "path is relative, using as-is\n" );
               // relative path... sounds like a fallback, just use 
               // matFileName as is
               PORT_snprintf( sName, sizeof(sName), "%s", matFileName.c_str() );
            }
            log_debug( "writing texture path: %s\n", sName );

            writeS( sName, MAX_QPATH );
            write( t );
         }

         // TEXT COORDS
         Mesh::VertexList::iterator vit;

         for ( vit = (*mlit).vertices.begin(); vit != (*mlit).vertices.end(); vit++ )
         {
            write( (*vit).uv[0] );
            write( (float) (1.0f - (*vit).uv[1]) );
         }

         // VERTEX
         m_model->calculateNormals();
         for ( unsigned a = 0; a < animCount; a++ )
         {
            if ( animInSection( getSafeName( a ), section ) 
                  || (section == MS_Head && a == 0) )
            {
               // If there are no anims calculateFrameNormals will segfault
               // (in earlier versions of MM3D)
               if ( m_model->getAnimCount( Model::ANIMMODE_FRAME ) > 0 )
               {
                  m_model->calculateFrameNormals( a );
               }
               unsigned aFrameCount = m_model->getAnimFrameCount( Model::ANIMMODE_FRAME, a );
               if ( (aFrameCount == 0 && animCount == 1) 
                     || (section == MS_Head) )
               {
                  aFrameCount = 1;
               }
               for ( unsigned t = 0; t < aFrameCount; t++ )
               {
                  Matrix saveMatrix = getMatrixFromPoint( a, t, rootTag ).getInverse();

                  if ( section == MS_Head )
                  {
                     saveMatrix = getMatrixFromPoint( -1, -1, rootTag ).getInverse();
                  }

                  for ( vit = (*mlit).vertices.begin(); vit != (*mlit).vertices.end(); vit++ )
                  {
                     double meshVec[4] = {0,0,0,1};
                     double meshNor[4] = {0,0,0,1};

                     if ( section == MS_Head )
                     {
                        // force unanimated coordinates for head
                        m_model->getVertexCoords( (*vit).v, meshVec );
                     }
                     else
                     {
                        //getFrameAnimVertexCoords will get non frame coords if no anim
                        m_model->getFrameAnimVertexCoords( a, t, (*vit).v, meshVec[0], meshVec[1], meshVec[2] );
                     }

                     if ( aFrameCount == 1 && animCount == 1 )
                     {
                        float meshNorF[3];
                        if ( getVertexNormal( m_model, (*mlit).group, (*vit).v, meshNorF ) )
                        {
                           meshNor[0] = meshNorF[0];
                           meshNor[1] = meshNorF[1];
                           meshNor[2] = meshNorF[2];
                        }
                     }
                     else
                     {
                        m_model->getFrameAnimVertexNormal( a, t, (*vit).v, meshNor[0], meshNor[1], meshNor[2] );
                     }
                     saveMatrix.apply( meshVec );
                     saveMatrix.apply( meshNor );
                     write( (int16_t) ( meshVec[0] / MD3_XYZ_SCALE ) );
                     write( (int16_t) ( meshVec[1] / MD3_XYZ_SCALE ) );
                     write( (int16_t) ( meshVec[2] / MD3_XYZ_SCALE ) );
                     int16_t lng;
                     int16_t lat;
                     if ( meshNor[0] == 0 && meshNor[1] == 0 )
                     {
                        if ( meshNor[2] > 0 )
                        {
                           lng = 0;
                           lat = 0;
                        }
                        else
                        {
                           lat = 128;
                           lng = 0;
                        }
                     }
                     else
                     {
                        lng = (int16_t) ( acos( meshNor[2] ) * 255 / ( 2 * PI ) );
                        lat = (int16_t) ( atan2( meshNor[1], meshNor[0] ) * 255 / ( 2 * PI ) );
                     }
                     // log_debug("%f, %f, %f lat %d lng %d\n", meshNor[0], meshNor[1], meshNor[2], lat, lng);
                     uint16_t normal = ( ( lat & 255 ) * 256 ) | ( lng & 255 );
                     normal = htol_u16( normal );
                     write( (int16_t) normal );
                  }
               }
            }
         }
      }
   }

   offsetEnd = ftell( m_fpOut );
   fseek( m_fpOut, endPos, SEEK_SET );
   write( offsetEnd );

   fclose( m_fpOut );
   return Model::ERROR_NONE;
}

bool Md3Filter::writeAnimations()
{
   string animFile = m_modelPath + "/animation.cfg";
   FILE * fp = fopen( animFile.c_str(), "w" );

   if ( fp != NULL )
   {
      log_debug( "writing animation.cfg\n" );

      fprintf( fp, "// animation config file\r\n\r\n" );

      char value[30];
      if ( m_model->getMetaData( "MD3_sex", value, sizeof(value) ) )
      {
         fprintf( fp, "sex %s\r\n", value );
      }
      if ( m_model->getMetaData( "MD3_footsteps", value, sizeof(value) ) )
      {
         fprintf( fp, "footsteps %s\r\n", value );
      }
      if ( m_model->getMetaData( "MD3_headoffset", value, sizeof(value) ) )
      {
         fprintf( fp, "headoffset %s\r\n", value );
      }
      if ( m_model->getMetaData( "MD3_fixedtorso", value, sizeof(value) ) )
      {
         fprintf( fp, "fixedtorso %s\r\n", value );
      }
      if ( m_model->getMetaData( "MD3_fixedlegs", value, sizeof(value) ) )
      {
         fprintf( fp, "fixedlegs %s\r\n", value );
      }

      fprintf( fp, "\r\n" );

      fprintf( fp, "// frame data:\r\n" );
      fprintf( fp, "//    first   count   looping   fps\r\n\r\n" );

      char warning[] = " (MUST NOT CHANGE -- hand animation is synced to this)";
      for ( int anim = 0; anim < MD3_ANIMATIONS; anim++ )
      {
         int animIndex = 0;
         int animFrame = 0;
         int count = 1;
         int fps   = 15;
         getExportAnimData( anim, animIndex, animFrame, count, fps );

         int loop = count; // loop by default

         // disable looping on non-looping anims
         if ( count <= 1 || s_animLoop[anim] == 0 )
         {
            loop = 0;
         }

         char name[30];
         strcpy( name, s_animNames[anim] );
         size_t len = strlen( name );
         for ( size_t n = 0; n < len; n++ )
         {
            name[n] = toupper( name[n] );
         }

         fprintf( fp, "%d\t%d\t%d\t%d\t\t// %s%s\r\n", 
               animFrame, count, loop, fps, name,
               (s_animSyncWarning[anim] ? warning : "") );
      }
      fclose( fp );
      return true;
   }
   return false;
}

Matrix Md3Filter::getMatrixFromPoint( int anim, int frame, int point )
{
   Matrix m;
   m.loadIdentity();

   if ( point < 0 )
   {
      m.setRotationInDegrees( -90, -90, 0 );
      return m;
   }

   double rot[3];
   double pos[3];

   if ( anim >= 0 && frame >= 0 )
   {
      m_model->getFrameAnimPointRotation( anim, frame, point, rot[0], rot[1], rot[2] );
      m_model->getFrameAnimPointCoords( anim, frame, point, pos[0], pos[1], pos[2] );
   }
   else
   {
      m_model->getPointRotation( point, rot );
      m_model->getPointTranslation( point, pos );
   }

   m.loadIdentity();
   m.setRotation( rot );
   m.setTranslation( pos );

   return m;
}

void Md3Filter::getExportAnimData( int fileAnim, int & modelAnim, 
      int & fileFrame, int & frameCount, int & fps )
{
   fileFrame  = 0;
   frameCount = 0;

   size_t animCount = m_model->getAnimCount( Model::ANIMMODE_FRAME );

   for ( size_t a = 0; a < animCount; a++ )
   {
      std::string name = getSafeName( a );
      if ( animInSection( name, MS_Upper )
            || animInSection( name, MS_Lower ) )
      {
         if ( strcasecmp( s_animNames[ fileAnim ], name.c_str() ) == 0 )
         {
            frameCount = m_model->getAnimFrameCount( Model::ANIMMODE_FRAME, a );
            fps   = (int) m_model->getAnimFPS( Model::ANIMMODE_FRAME, a );

            if ( fps <= 0 ) // just being paranoid
            {
               fps = 15;
            }

            return;
         }
         else
         {
            fileFrame += m_model->getAnimFrameCount( Model::ANIMMODE_FRAME, a );
         }
      }
   }

   // if this is a "dead" animation and there isn't a "dead" animation, try the
   // last frame of the "death" animation.
   if ( fileAnim < 6 && (fileAnim % 2) == 1 )
   {
      fileAnim--;
      getExportAnimData( fileAnim, modelAnim, fileFrame, frameCount, fps );
      if ( modelAnim >= 0 )
      {
         fileFrame += frameCount - 1;
         frameCount = 1;
         return;
      }
   }

   modelAnim = -1;
}

size_t Md3Filter::write( int8_t val )
{
   int8_t temp8=val;
   return fwrite( &temp8, sizeof( int8_t ), 1, m_fpOut );
}

size_t Md3Filter::write( int16_t val )
{
   int16_t temp16=htol_16( val );
   return fwrite( &temp16, sizeof( int16_t ), 1, m_fpOut );
}

size_t Md3Filter::write( int32_t val )
{
   int32_t temp32=htol_32( val );
   return fwrite( &temp32, sizeof( int32_t ), 1, m_fpOut );
}

size_t Md3Filter::write( float val )
{
   float temp32=htol_float( val );
   return fwrite( &temp32, sizeof( float ), 1, m_fpOut );
}

size_t Md3Filter::writeS( char *val, size_t len )
{
   return fwrite( val, len, 1, m_fpOut );
}

size_t Md3Filter::writeIdentity()
{
   size_t count = 0;
   float z = 0.0;
   float o = 1.0;
   for ( int i = 0; i < 3; i++ )
   {
      for ( int j = 0; j < 3; j++ )
      {
         if ( i == j )
         {
            count += write( o );
         }
         else
         {
            count += write( z );
         }
      }
   }
   return count;
}

std::string Md3Filter::extractPath( const char * md3DataPath )
{
   std::string path = md3DataPath;
   replaceBackslash( path );
   size_t i = path.rfind( '/' );
   if ( i > 0 && i < path.size() )
   {
      path.resize(i+1);
      return path;
   }
   else
   {
      return "";
   }
}

std::string Md3Filter::sectionToPath( int materialIndex )
{
   char pk3Path[MAX_QPATH] = "";
   if ( m_model->getMetaData( "MD3_PATH", pk3Path, sizeof(pk3Path) ) )
   {
      return pk3Path;
   }
   // NOTE: I removed the "model/" stuff because it's causing problems with
   // working models
   return "";
}

std::string Md3Filter::materialToPath( int materialIndex )
{
   char pk3Path[MAX_QPATH] = "";

   const char * name = m_model->getTextureName( materialIndex );
   if ( !name )
   {
      name = "";
   }
   std::string keyStr = std::string("MD3_PATH_") + name;
   if ( m_model->getMetaData( keyStr.c_str(), pk3Path, sizeof(pk3Path) ) )
   {
      return pk3Path;
   }

   keyStr = "MD3_PATH";
   if ( m_model->getMetaData( keyStr.c_str(), pk3Path, sizeof(pk3Path) ) )
   {
      return pk3Path;
   }
   return "";
}

std::string Md3Filter::defaultPath()
{
   std::string path = "models/players/";
   std::string modelName = "noname/";
   if ( m_modelPath.size() > 1 )
   {
      size_t i = m_modelPath.rfind( '/', m_modelPath.size()-2 );
      if ( i > 0 && i < m_modelPath.size() )
      {
         modelName = m_modelPath.substr( i+1, m_modelPath.size() );
      }
   }
   path += modelName;

   return path;
}


//returns greater of a or b
double Md3Filter::greater( double a, double b )
{
   if ( a > b )
   {
      return a;
   }
   return b;
}

//returns smaller of a or b
double Md3Filter::smaller( double a, double b )
{
   if ( a < b )
   {
      return a;
   }
   return b;
}

bool Md3Filter::getVertexNormal( Model * model, int groupId, int vertexId, float *normal )
{
   list<int> tris = model->getGroupTriangles( groupId );
   list<int>::iterator tri;
   for ( tri = tris.begin(); tri != tris.end(); tri++ )
   {
      for ( int n = 0; n < 3; n++ )
      {
         int vert = model->getTriangleVertex( *tri, n );
         if ( vert == vertexId )
         {
            model->getNormal( *tri, n, normal );
            return true;
         }
      }
   }

   return false;
}

//MM3D internal stuff

bool Md3Filter::canRead( const char * filename )
{
   log_debug( "canRead( %s )\n", filename );
   log_debug( "  true\n" );
   return true;
}

bool Md3Filter::canWrite( const char * filename )
{
   log_debug( "canWrite( %s )\n", filename );
   log_debug( "  true\n" );
   return true;
}

bool Md3Filter::canExport( const char * filename )
{
   log_debug( "canExport( %s )\n", filename );
   log_debug( "  true\n" );
   return true;
}

bool Md3Filter::isSupported( const char * filename )
{
   log_debug( "isSupported( %s )\n", filename );
   unsigned len = strlen( filename );

   if ( len >= 4 && strcasecmp( &filename[len-4], ".md3" ) == 0 )
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

list< string > Md3Filter::getReadTypes()
{
   list<string> rval;
   rval.push_back( "*.md3" );
   return rval;
}

list< string > Md3Filter::getWriteTypes()
{
   list<string> rval;
   rval.push_back( "*.md3" );
   return rval;
}

#ifdef PLUGIN

//------------------------------------------------------------------
// Plugin functions
//------------------------------------------------------------------

PLUGIN_API bool plugin_init()
{
   if ( s_filter == NULL )
   {
      s_filter = new Md3Filter();
      FilterManager * texmgr = FilterManager::getInstance();
      texmgr->registerFilter( s_filter );
   }
   log_debug( "MD3 model filter plugin initialized\n" );
   return true;
}

// The filter manager will delete our registered filter.
// We have no other cleanup to do
PLUGIN_API bool plugin_uninit()
{
   s_filter = NULL;                                         // FilterManager deletes filters
   log_debug( "MD3 model filter plugin uninitialized\n" );
   return true;
}

PLUGIN_API const char * plugin_version()
{
   return "0.1.0";
}

PLUGIN_API const char * plugin_mm3d_version()
{
   return VERSION_STRING;
}

PLUGIN_API const char * plugin_desc()
{
   return "MD3 model filter";
}
#endif                                                      // PLUGIN
