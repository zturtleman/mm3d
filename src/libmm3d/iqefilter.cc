/*  IqeFilter plugin for Maverick Model 3D
 *
 *  Copyright (c) 2018 Zack Middleton
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

#include "iqefilter.h"

#include "texture.h"
#include "texmgr.h"
#include "misc.h"
#include "log.h"
#include "translate.h"
#include "modelstatus.h"
#include "mesh.h"

#include "mm3dport.h"
//#include "datasource.h"
#include "datadest.h"
#include "release_ptr.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include <string>

using std::list;
using std::string;

static const char *s_animSyncWarning[] =
{
   "torso_attack",
   "torso_attack2",
   "torso_drop",
   "torso_raise",
   NULL
};

static const char *IQE_HEADER = "# Inter-Quake Export";

IqeFilter::IqeOptions::IqeOptions()
   : m_playerSupported( false ),
     m_saveAsPlayer( false ),
     m_saveAnimationCfg( false ),
     m_saveMeshes( true ),
     m_savePointsJoint( true ),
     m_saveSkeleton( true ),
     m_saveAnimations( true ),
     m_animations()
{
}

IqeFilter::IqeOptions::~IqeOptions()
{
}

void IqeFilter::IqeOptions::setOptionsFromModel( Model * model, const char * const filename )
{
   m_playerSupported = false;

   string modelPath = "";
   string modelBaseName = "";
   string modelFullName = "";

   normalizePath( filename, modelFullName, modelPath, modelBaseName );

   if (     strncasecmp( modelBaseName.c_str(), "lower.", 6 ) == 0
         || strncasecmp( modelBaseName.c_str(), "upper.", 6 ) == 0
         || strncasecmp( modelBaseName.c_str(), "head.",  5 ) == 0 )
   {
      bool haveUpper = false;
      bool haveLower = false;
      bool haveUnknown = false;

      unsigned gcount = model->getGroupCount();
      for ( unsigned g = 0; g < gcount; g++ )
      {
         std::string name = model->getGroupName( g );
         if ( name[0] != '\0' && name[1] == '_' )
         {
            switch ( toupper( name[0] ) )
            {
               case 'U':
                  haveUpper = true;
                  break;
               case 'L':
                  haveLower = true;
                  break;
               case 'H':
                  break;
               default:
                  haveUnknown = true;
                  break;
            }
         } else {
            haveUnknown = true;
         }
      }

      if ( !haveUnknown && haveUpper && haveLower
            && model->getPointByName( "tag_torso" ) >= 0
            && model->getPointByName( "tag_head" )  >= 0 )
      {
         // have filename, groups, and tags required for player model
         m_playerSupported = true;
      }
   }

   m_saveAsPlayer = m_playerSupported;
   m_saveAnimationCfg = m_playerSupported;

   char value[20];
   if ( model->getMetaData( "MD3_composite", value, sizeof( value ) ) )
   {
      if ( atoi( value ) == 0 )
      {
         m_saveAsPlayer = false;
         m_saveAnimationCfg = false;
      }
   }

   if ( model->getMetaData( "MD3_animationcfg", value, sizeof( value ) ) )
   {
      m_saveAnimationCfg = !!atoi( value );
   }
}

IqeFilter::IqeFilter()
{
}

IqeFilter::~IqeFilter()
{
}

Model::ModelErrorE IqeFilter::readFile( Model * model, const char * const filename )
{
   return Model::ERROR_UNSUPPORTED_OPERATION;
}

// TODO: Support using a file for bone order list like the Blender IQE exporter?
Model::ModelErrorE IqeFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o )
{
   if ( model && filename && filename[0] )
   {
      release_ptr<IqeOptions> freeOptions = NULL;
      m_options = dynamic_cast<IqeOptions *>( o );
      if ( !m_options )
      {
         freeOptions = static_cast< IqeOptions * >( getDefaultOptions() );
         m_options = freeOptions.get();
      }

      if ( !m_options->m_saveMeshes && !m_options->m_saveSkeleton && !m_options->m_saveAnimations )
      {
         model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "No data marked for saving as IQE." ) ).c_str() );
         return Model::ERROR_FILTER_SPECIFIC;
      }

      if ( m_options->m_saveMeshes )
      {
         unsigned tcount = model->getTriangleCount();
         for ( unsigned t = 0; t < tcount; ++t )
         {
            if ( model->getTriangleGroup( t ) < 0 )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "IQE requires all faces to be grouped." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }
         }
      }

      if ( m_options->m_savePointsJoint && ( m_options->m_saveSkeleton || m_options->m_saveAnimations ) )
      {
         unsigned pcount = model->getPointCount();
         for ( unsigned p = 0; p < pcount; ++p )
         {
            Model::InfluenceList il;
            model->getPointInfluences( p, il );

            if ( il.size() > 1 )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "IQE requires points to only have one bone influence." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }
         }
      }

      m_model = model;

      string modelPath = "";
      string modelBaseName = "";
      string modelFullName = "";

      normalizePath( filename, modelFullName, modelPath, modelBaseName );

      m_modelPath = modelPath;

      if ( m_options->m_saveAsPlayer )
      {
         log_debug( "saving as a player model\n" );

         Model * section;
         std::string playerFile;
         std::string path = modelPath + "/";

         section = m_model->copyQuake3PlayerSection( Model::MS_Lower, Model::ANIMMODE_SKELETAL );
         if ( section )
         {
            playerFile = path + fixFileCase( m_modelPath.c_str(), "lower.iqe" );
            writeSectionFile( section, playerFile.c_str() );
            delete section;
         }

         section = m_model->copyQuake3PlayerSection( Model::MS_Upper, Model::ANIMMODE_SKELETAL );
         if ( section )
         {
            playerFile = path + fixFileCase( m_modelPath.c_str(), "upper.iqe" );
            writeSectionFile( section, playerFile.c_str() );
            delete section;
         }

         section = m_model->copyQuake3PlayerSection( Model::MS_Head, Model::ANIMMODE_SKELETAL );
         if ( section )
         {
            playerFile = path + fixFileCase( m_modelPath.c_str(), "head.iqe" );
            writeSectionFile( section, playerFile.c_str() );
            delete section;
         }

         if ( m_options->m_saveAnimationCfg )
         {
            writeAnimations( true, NULL );
         }
         else
         {
            model->addMetaData( "MD3_animationcfg", "0" );
         }

         model->addMetaData( "MD3_composite", "1" );
         model->operationComplete( transll( QT_TRANSLATE_NOOP( "LowLevel", "Set meta data for IQE export" ) ).c_str() );

         return Model::ERROR_NONE;
      }
      else
      {
         log_debug( "saving as a single model\n" );

         writeSectionFile( model, filename );

         if ( m_options->m_saveAnimationCfg )
         {
            writeAnimations( false, filename );
         }

         if ( m_options->m_saveAnimationCfg || m_options->m_playerSupported )
         {
            if ( m_options->m_playerSupported )
            {
               model->addMetaData( "MD3_composite", "0" );
            }
            if ( m_options->m_saveAnimationCfg )
            {
               model->addMetaData( "MD3_animationcfg", "1" );
            }

            model->operationComplete( transll( QT_TRANSLATE_NOOP( "LowLevel", "Set meta data for IQE export" ) ).c_str() );
         }

         return Model::ERROR_NONE;
      }
   }
   else
   {
      log_error( "no filename supplied for model filter\n" );
      return Model::ERROR_NO_FILE;
   }
}

Model::ModelErrorE IqeFilter::writeSectionFile( Model *model, const char * filename )
{
   if ( model && filename && filename[0] )
   {
      Model::ModelErrorE err = Model::ERROR_NONE;
      DataDest * dst = openOutput( filename, err );
      DestCloser fc( dst );

      if ( err != Model::ERROR_NONE )
         return err;

      // Use the load matrix and then invert it
      Matrix saveMatrix;
      saveMatrix.setRotationInDegrees( -90, -90, 0 );
      saveMatrix = saveMatrix.getInverse();

      //
      // Write Header
      //
      writeLine( dst, "%s", IQE_HEADER );
      writeLine( dst, "" );

      //
      // Write Joints
      //
      int boneCount = model->getBoneJointCount();
      if ( m_options->m_saveSkeleton && boneCount > 0 )
      {
         for ( int bone = 0; bone < boneCount; bone++ )
         {
            int parent = model->getBoneJointParent( bone );

            writeLine( dst, "joint \"%s\" %d", model->getBoneJointName( bone ), parent );

            Matrix m;
            model->getBoneJointAbsoluteMatrix( bone, m );
            m = m * saveMatrix;

            Matrix pinv;
            if ( parent >= 0 )
            {
               model->getBoneJointAbsoluteMatrix( parent, pinv );
               pinv = pinv * saveMatrix;
               pinv = pinv.getInverse();
            }

            Matrix lm;
            lm = m * pinv;

            Vector trans;
            Quaternion rot;
            lm.getTranslation( trans );
            lm.getRotationQuaternion( rot );

            // make quat w be negative
            rot[0] = -rot[0];
            rot[1] = -rot[1];
            rot[2] = -rot[2];
            rot[3] = -rot[3];

            writeLine( dst, "\tpq %.8f %.8f %.8f %.8f %.8f %.8f %.8f",
                       (float)trans[0], (float)trans[1], (float)trans[2],
                       (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3] );
            //writeLine( dst, "\tpm %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f",
            //           (float)trans[0], (float)trans[1], (float)trans[2],
            //           (float)lm.get( 0, 0 ), (float)lm.get( 0, 1 ), (float)lm.get( 0, 2 ),
            //           (float)lm.get( 1, 0 ), (float)lm.get( 1, 1 ), (float)lm.get( 1, 2 ),
            //           (float)lm.get( 2, 0 ), (float)lm.get( 2, 1 ), (float)lm.get( 2, 2 ) );
         }

         writeLine( dst, "" );
      }

      //
      // Write Points as Joints
      //
      int pointCount = model->getPointCount();
      if ( m_options->m_saveSkeleton && m_options->m_savePointsJoint && pointCount > 0 )
      {
         writeLine( dst, "# Points" );

         for ( int point = 0; point < pointCount; point++ )
         {
            int parent = m_options->m_saveSkeleton ? model->getPrimaryPointInfluence( point ) : -1;

            writeLine( dst, "joint \"%s\" %d", model->getPointName( point ), parent );

            Matrix m;
            model->getPointAbsoluteMatrix( point, m );
            m = m * saveMatrix;

            Matrix pinv;
            if ( parent >= 0 )
            {
               model->getBoneJointAbsoluteMatrix( parent, pinv );
               pinv = pinv * saveMatrix;
               pinv = pinv.getInverse();
            }

            Matrix lm;
            lm = m * pinv;

            Vector trans;
            Quaternion rot;
            lm.getTranslation( trans );
            lm.getRotationQuaternion( rot );

            // make quat w be negative
            rot[0] = -rot[0];
            rot[1] = -rot[1];
            rot[2] = -rot[2];
            rot[3] = -rot[3];

            writeLine( dst, "\tpq %.8f %.8f %.8f %.8f %.8f %.8f %.8f",
                       (float)trans[0], (float)trans[1], (float)trans[2],
                       (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3] );
            //writeLine( dst, "\tpm %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f",
            //           (float)trans[0], (float)trans[1], (float)trans[2],
            //           (float)lm.get( 0, 0 ), (float)lm.get( 0, 1 ), (float)lm.get( 0, 2 ),
            //           (float)lm.get( 1, 0 ), (float)lm.get( 1, 1 ), (float)lm.get( 1, 2 ),
            //           (float)lm.get( 2, 0 ), (float)lm.get( 2, 1 ), (float)lm.get( 2, 2 ) );
         }

         writeLine( dst, "" );
      }

      //
      // Write Meshes
      //
      if ( m_options->m_saveMeshes )
      {
          MeshList meshes;

          // MD3 does not allow a single vertex to have more than one texture
          // coordinate or normal. MM3D does. The mesh_create_list function will
          // break the model up into meshes where vertices meet the MD3 criteria.
          // See mesh.h for details.
#if 0
          // TODO: Split up meshes to meet Quake 3 per-mesh triangle and vertex limits. Need an export option dialog and/or meta data.
          mesh_create_list( meshes, model, Mesh::MO_All, 2000 - 1, 1000 - 1 );
#else
          mesh_create_list( meshes, model );
#endif

          vector<Model::Material *> & modelMaterials = getMaterialList( model );

          MeshList::iterator mlit;

          for ( mlit = meshes.begin(); mlit != meshes.end(); mlit++ )
          {
             int g = (*mlit).group;

             if ( g < 0 )
             {
                // Ungrouped triangles
                continue;
             }

             //
             // Mesh data
             //
             std::string groupName = model->getGroupName( g );
             int matId = model->getGroupTextureId( g );

             writeLine( dst, "mesh \"%s\"", groupName.c_str() );
             if ( matId >= 0 )
             {
                writeLine( dst, "\tmaterial \"%s\"", modelMaterials[ matId ]->m_name.c_str() );
             }
             writeLine( dst, "" );

             //
             // Mesh vertex data
             //
             Mesh::VertexList::iterator vit;

             for ( vit = (*mlit).vertices.begin(); vit != (*mlit).vertices.end(); vit++ )
             {
                double meshVec[4] = { 0,0,0,1 };
                float meshNor[4] = { 0,0,0,1 };

                model->getVertexCoordsUnanimated( (*vit).v, meshVec );

                meshNor[0] = (*vit).norm[0];
                meshNor[1] = (*vit).norm[1];
                meshNor[2] = (*vit).norm[2];

                saveMatrix.apply( meshVec );
                saveMatrix.apply( meshNor );

                writeLine( dst, "vp %.8f %.8f %.8f", (float)meshVec[0], (float)meshVec[1], (float)meshVec[2] );
                writeLine( dst, "\tvt %.8f %.8f", (*vit).uv[0], (float) (1.0f - (*vit).uv[1]) );
                writeLine( dst, "\tvn %.8f %.8f %.8f", meshNor[0], meshNor[1], meshNor[2] );

                if ( m_options->m_saveSkeleton && boneCount > 0 )
                {
                   Model::InfluenceList il;
                   Model::InfluenceList::iterator it;
                   model->getVertexInfluences( (*vit).v, il );

                   // Sort highest weight first
                   il.sort(std::greater<Model::InfluenceT>());

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
                   dst->writePrintf( "\tvb" );
                   for ( it = il.begin(); it != il.end(); it++ )
                   {
                      double weight = (it->m_weight / total);

                      if ( weight < 1e-8 )
                      {
                         break;
                      }

                      dst->writePrintf( " %d %.8f", it->m_boneId, (float)weight );
                   }
                   dst->writePrintf( "\r\n" );
                }
             }

             writeLine( dst, "" );

             //
             // Mesh face data
             //
             Mesh::FaceList::iterator fit;

             for ( fit = (*mlit).faces.begin(); fit != (*mlit).faces.end(); fit++ )
             {
                // Quake-like engines use reverse triangle winding order (glCullFace GL_FRONT)
                writeLine( dst, "fm %d %d %d", (*fit).v[2], (*fit).v[1], (*fit).v[0] );
             }

             writeLine( dst, "" );
          }
      }

      //
      // Write Animations
      //
      if ( m_options->m_saveAnimations && boneCount > 0 )
      {
         vector<Matrix> poseFinal;

         poseFinal.reserve( boneCount );

         // Animation numbers to export are set by iqeprompt_show()
         std::vector<unsigned>::iterator it;
         for (it = m_options->m_animations.begin(); it != m_options->m_animations.end(); it++)
         {
            unsigned anim = *it;
            const char *animName = model->getAnimName( Model::ANIMMODE_SKELETAL, anim );
            float fps = model->getAnimFPS( Model::ANIMMODE_SKELETAL, anim );
            unsigned frameCount = model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, anim );
            bool loop = model->getAnimLooping( Model::ANIMMODE_SKELETAL, anim );

            if ( frameCount == 0 )
            {
               continue;
            }

            writeLine( dst, "animation \"%s\"", animName );
            writeLine( dst, "\tframerate %.8f", fps );
            if ( loop )
            {
               writeLine( dst, "\tloop" );
            }
            writeLine( dst, "" );

            for ( unsigned frame = 0; frame < frameCount; frame++ )
            {
               double frameTime = frame / (double)fps;

               writeLine( dst, "frame" );

               for ( int bone = 0; bone < boneCount; bone++ )
               {
                  int parent = model->getBoneJointParent( bone );

                  Matrix transform;
                  model->interpSkelAnimKeyframeTime( anim, frameTime, loop, bone, transform );

                  Matrix rm;
                  model->getBoneJointRelativeMatrix( bone, rm );

                  Matrix relativeFinal = transform * rm;

                  if ( parent == -1 )
                  {
                     poseFinal[ bone ] = relativeFinal * saveMatrix;
                  }
                  else
                  {
                     poseFinal[ bone ] = relativeFinal * poseFinal[ parent ];
                  }

                  Matrix m = poseFinal[ bone ];

                  Matrix pinv;
                  if ( parent >= 0 )
                  {
                     pinv = poseFinal[ parent ];
                     pinv = pinv.getInverse();
                  }

                  Matrix lm;
                  lm = m * pinv;

                  Vector trans;
                  Quaternion rot;
                  lm.getTranslation( trans );
                  lm.getRotationQuaternion( rot );

                  // make quat w be negative
                  rot[0] = -rot[0];
                  rot[1] = -rot[1];
                  rot[2] = -rot[2];
                  rot[3] = -rot[3];

                  writeLine( dst, "pq %.8f %.8f %.8f %.8f %.8f %.8f %.8f",
                                  (float)trans[0], (float)trans[1], (float)trans[2],
                                  (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3] );
                  //writeLine( dst, "pm %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f",
                  //           (float)trans[0], (float)trans[1], (float)trans[2],
                  //           (float)lm.get( 0, 0 ), (float)lm.get( 0, 1 ), (float)lm.get( 0, 2 ),
                  //           (float)lm.get( 1, 0 ), (float)lm.get( 1, 1 ), (float)lm.get( 1, 2 ),
                  //           (float)lm.get( 2, 0 ), (float)lm.get( 2, 1 ), (float)lm.get( 2, 2 ) );
               }

               // This is the same as point joint matricies. There is
               // no animation.
               if ( m_options->m_savePointsJoint && pointCount > 0 )
               {
                  for ( int point = 0; point < pointCount; point++ )
                  {
                     int parent = model->getPrimaryPointInfluence( point );

                     Matrix m;
                     model->getPointAbsoluteMatrix( point, m );
                     m = m * saveMatrix;

                     Matrix pinv;
                     if ( parent >= 0 )
                     {
                        model->getBoneJointAbsoluteMatrix( parent, pinv );
                        pinv = pinv * saveMatrix;
                        pinv = pinv.getInverse();
                     }

                     Matrix lm;
                     lm = m * pinv;

                     Vector trans;
                     Quaternion rot;
                     lm.getTranslation( trans );
                     lm.getRotationQuaternion( rot );

                     // make quat w be negative
                     rot[0] = -rot[0];
                     rot[1] = -rot[1];
                     rot[2] = -rot[2];
                     rot[3] = -rot[3];

                     writeLine( dst, "pq %.8f %.8f %.8f %.8f %.8f %.8f %.8f",
                                (float)trans[0], (float)trans[1], (float)trans[2],
                                (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3] );
                     //writeLine( dst, "pm %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f",
                     //           (float)trans[0], (float)trans[1], (float)trans[2],
                     //           (float)lm.get( 0, 0 ), (float)lm.get( 0, 1 ), (float)lm.get( 0, 2 ),
                     //           (float)lm.get( 1, 0 ), (float)lm.get( 1, 1 ), (float)lm.get( 1, 2 ),
                     //           (float)lm.get( 2, 0 ), (float)lm.get( 2, 1 ), (float)lm.get( 2, 2 ) );
                  }
               }

               writeLine( dst, "" );
            }
         }
      }

      return Model::ERROR_NONE;
   }
   else
   {
      log_error( "no filename supplied for model filter" );
      return Model::ERROR_BAD_ARGUMENT;
   }
}

bool IqeFilter::writeLine( DataDest *dst, const char * line, ... )
{
   va_list ap;
   va_start( ap, line );
   dst->writeVPrintf( line, ap );
   va_end( ap );
   dst->writePrintf( "\r\n" );
   return true;
}

// writeAnimations() and it's util functions are copied from Md3Filter.
// Changed to use ANIMMODE_SKELETAL and m_animations.
bool IqeFilter::writeAnimations( bool playerModel, const char * modelName )
{
   string animFile = m_modelPath + "/animation.cfg";
   bool eliteLoop = false;
   bool animKeyword = false;

   Model::ModelErrorE err = Model::ERROR_NONE;
   DataDest *dst = openOutput( animFile.c_str(), err );
   DestCloser fc( dst );

   if ( err != Model::ERROR_NONE )
   {
      return false;
   }
   else
   {
      log_debug( "writing animation.cfg\n" );

      if ( modelName )
      {
         dst->writePrintf( "// animation config file for %s\r\n", PORT_basename( modelName ) );
      }
      else
      {
         dst->writeString( "// animation config file\r\n" );
      }

      bool hadKeyword = false;
      char keyword[1024], value[1024];

      for (unsigned int i = 0; i < m_model->getMetaDataCount(); i++)
      {
         if (!m_model->getMetaData(i, keyword, sizeof (keyword), value, sizeof (value)))
            continue;

         if (strncasecmp(keyword, "MD3_CFG_", 8) == 0)
         {
            if (!hadKeyword)
            {
               hadKeyword = true;
               dst->writeString( "\r\n" );
            }
            if (strlen(value) > 0)
               dst->writePrintf( "%s %s\r\n", &keyword[8], value );
            else
               dst->writePrintf( "%s\r\n", &keyword[8] );
         }
         // Support old keywords
         else if (strncasecmp(keyword, "MD3_sex", 7) == 0
           || strncasecmp(keyword, "MD3_footsteps", 13) == 0
           || strncasecmp(keyword, "MD3_headoffset", 14) == 0
           || strncasecmp(keyword, "MD3_fixedtorso", 14) == 0
           || strncasecmp(keyword, "MD3_fixedlegs", 13) == 0)
         {
            if (!hadKeyword)
            {
               hadKeyword = true;
               dst->writeString( "\r\n" );
            }
            if (strlen(value) > 0)
               dst->writePrintf( "%s %s\r\n", &keyword[4], value );
            else
               dst->writePrintf( "%s\r\n", &keyword[4] );
         }
         // animations.cfg format settings
         else if (strncasecmp(keyword, "MD3_EliteLoop", 13) == 0)
         {
            eliteLoop = (atoi(value) > 0);
         }
         else if (strncasecmp(keyword, "MD3_AnimKeyword", 15) == 0)
         {
            animKeyword = (atoi(value) > 0);
         }
      }

      dst->writeString( "\r\n" );

      dst->writeString( "// frame data:\r\n" );
      dst->writeString( "//    first   count   looping   fps\r\n\r\n" );

      char warning[] = " (MUST NOT CHANGE -- hand animation is synced to this)";

      size_t longestName = 16; // minimum name length for spacing
      std::vector<unsigned>::iterator it;
      for (it = m_options->m_animations.begin(); it != m_options->m_animations.end(); it++)
      {
         unsigned anim = *it;
         std::string name = getSafeName( anim );
         size_t len = name.length();
         if (len > longestName) {
            longestName = len;
         }
      }

      for (it = m_options->m_animations.begin(); it != m_options->m_animations.end(); it++)
      {
         unsigned anim = *it;
         int animFrame = 0;
         int count = 1;
         int fps   = 15;
         if (!getExportAnimData( playerModel, (int)anim, animFrame, count, fps ))
         {
            continue;
         }

         int loop = count; // loop by default

         std::string name = getSafeName( anim );
         size_t len = name.length();
         for ( size_t n = 0; n < len; n++ )
         {
            name[n] = std::toupper(name[n]);
         }

         // disable looping on non-looping anims
         if ( !m_model->getAnimLooping( Model::ANIMMODE_SKELETAL, anim ) )
         {
            loop = 0;
         }

         // Convert to Elite Force Single Player Style
         if (eliteLoop)
         {
            if (loop == 0)
               loop = -1; // No loop
            else
               loop = 0; // Loop
         }

         if (animKeyword)
         {
            // Align animFrame
            const size_t MAX_SPACES = 40;
            char spaces[MAX_SPACES+2] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                 ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};
            size_t maxSpaces = (longestName+6 > MAX_SPACES) ? MAX_SPACES : longestName+6;

            spaces[((name.length() < maxSpaces) ? (maxSpaces - name.length()) : 0)] = '\0';

            if (animSyncWarning(name))
               dst->writePrintf( "%s%s\t%d\t%d\t%d\t%d\t\t// %s\r\n", 
                     name.c_str(), spaces, animFrame, count, loop, fps, warning );
            else
               dst->writePrintf( "%s%s\t%d\t%d\t%d\t%d\r\n", 
                     name.c_str(), spaces, animFrame, count, loop, fps );
         }
         else
         {
            dst->writePrintf( "%d\t%d\t%d\t%d\t\t// %s%s\r\n", 
                  animFrame, count, loop, fps, name.c_str(),
                  (animSyncWarning(name) ? warning : "") );
         }
      }
      return true;
   }
}

std::string IqeFilter::getSafeName( unsigned int anim )
{
   std::string animName = "none";

   if ( anim < m_model->getAnimCount( Model::ANIMMODE_SKELETAL ) )
   {
      animName = m_model->getAnimName( Model::ANIMMODE_SKELETAL, anim );
   }

   return animName;
}

bool IqeFilter::animSyncWarning(std::string name)
{
   char value[20];

   if ( m_model->getMetaData( "MD3_NoSyncWarning", value, sizeof(value) ) )
   {
      if ( atoi( value ) == 1 )
         return false;
   }

   for (unsigned i = 0; s_animSyncWarning[i] != NULL; i++)
   {
      if (strncasecmp(s_animSyncWarning[i], name.c_str(), name.length()) == 0)
      {
         return true;
      }
   }
   return false;
}

IqeFilter::MeshAnimationTypeE IqeFilter::getAnimationType( bool playerModel, const std::string & animName )
{
   MeshAnimationTypeE animType = MA_All;

   if ( !playerModel )
   {
      return MA_All;
   }

   if (strncasecmp(animName.c_str(), "both_", 5) == 0)
   {
      animType = MA_Both;
   }
   else if (strncasecmp(animName.c_str(), "torso_", 6) == 0)
   {
      animType = MA_Torso;
   }
   else if (strncasecmp(animName.c_str(), "legs_", 5) == 0)
   {
      animType = MA_Legs;
   }
   else if (strncasecmp(animName.c_str(), "head_", 5) == 0)
   {
      animType = MA_Head;
   }

   return animType;
}

bool IqeFilter::animInSection( std::string animName, MeshSectionE section )
{
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

   if ( strncasecmp( animName.c_str(), "head_", 5 ) == 0 )
   {
      if ( section == MS_Head )
         return true;
      else
         return false;
   }

   // It's a "both_" animation, or something weird
   if ( strncasecmp( animName.c_str(), "both_", 5 ) == 0 )
   {
      if ( section == MS_Lower || section == MS_Upper )
         return true;
      else
         return false;
   }

   if ( strncasecmp( animName.c_str(), "all_", 4 ) == 0 )
   {
      // Animation for torso, legs, and head!
      return true;
   }

   return false;
}

bool IqeFilter::getExportAnimData( bool playerModel, int modelAnim,
      int & fileFrame, int & frameCount, int & fps )
{
   fileFrame  = 0;
   frameCount = 0;

   std::string animName = getSafeName( modelAnim );
   MeshAnimationTypeE animType = getAnimationType( playerModel, animName );

   // If this is a "dead" animation and its after a "death" animation
   //   and it has 0 frames, use the last frame of the death animation.
   if (modelAnim > 0 && ((strncasecmp(animName.c_str(), "all_dead", 8) == 0
         && strncasecmp(getSafeName( modelAnim - 1 ).c_str(), "all_death", 9) == 0)
      || (strncasecmp(animName.c_str(), "both_dead", 9) == 0
         && strncasecmp(getSafeName( modelAnim - 1 ).c_str(), "both_death", 10) == 0))
      && m_model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, modelAnim ) == 0 )
   {
      if ( getExportAnimData( playerModel, modelAnim - 1, fileFrame, frameCount, fps ) )
      {
         fileFrame += frameCount - 1;
         frameCount = 1;
         return true;
      }
   }

   std::vector<unsigned>::iterator it;
   for (it = m_options->m_animations.begin(); it != m_options->m_animations.end(); it++)
   {
      size_t a = *it;
      std::string name = getSafeName( a );
      if ( !playerModel
            || animInSection( name, MS_Upper )
            || animInSection( name, MS_Lower )
            || animInSection( name, MS_Head ) )
      {
         MeshAnimationTypeE type = getAnimationType( playerModel, name );
         if ( (int)a == modelAnim )
         {
            frameCount = m_model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, a );
            fps   = (int) m_model->getAnimFPS( Model::ANIMMODE_SKELETAL, a );

            if ( fps <= 0 ) // just being paranoid
            {
               fps = 15;
            }

            if (animType > MA_Torso)
            {
               // Must still count torso animations after this for fileFrame
               //   and in the case of 'head' we must also count legs animations
            }
            else
            {
               return true;
            }
         }
         // All torso frames go before leg frames, all legs go after torso
         else if (((int)a < modelAnim && animType >= type) || ((int)a > modelAnim && animType > type))
         {
               fileFrame += m_model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, a );
         }
      }
   }

   if (animType > MA_Torso && frameCount)
   {
      // Finished adding up fileFrame for legs or head animations
      return true;
   }

   return false;
}

bool IqeFilter::canRead( const char * filename )
{
   return false;
}

bool IqeFilter::canWrite( const char * filename )
{
   return true;
}

bool IqeFilter::canExport( const char * filename )
{
   return true;
}

bool IqeFilter::isSupported( const char * filename )
{
   if ( filename )
   {
      unsigned len = strlen( filename );

      if ( len >= 4 && strcasecmp( &filename[len-4], ".iqe" ) == 0 )
      {
         return true;
      }
   }

   return false;
}

list<string> IqeFilter::getReadTypes()
{
   list<string> rval;

   //rval.push_back( "*.iqe" );

   return rval;
}

list<string> IqeFilter::getWriteTypes()
{
   list<string> rval;

   rval.push_back( "*.iqe" );

   return rval;
}
