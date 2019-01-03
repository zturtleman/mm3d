/*  SmdFilter plugin for Maverick Model 3D
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

#include "smdfilter.h"

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
#include <limits.h>

#include <string>

using std::list;
using std::string;

SmdFilter::SmdOptions::SmdOptions()
   : m_saveMeshes( true ),
     m_savePointsJoint( false ),
     m_multipleVertexInfluences( false ),
     m_animations()
{
}

SmdFilter::SmdOptions::~SmdOptions()
{
}

void SmdFilter::SmdOptions::setOptionsFromModel( Model * m )
{
   char value[128];
   if ( m->getMetaData( "smd_vertex_format", value, sizeof(value) ) )
   {
      if ( strcasecmp( value, "goldsrc" ) == 0 || strcasecmp( value, "goldsource" ) == 0 )
      {
         m_multipleVertexInfluences = false;
      }
      else //if ( strcasecmp( value, "source" ) == 0 )
      {
         m_multipleVertexInfluences = true;
      }
   }
   else
   {
      // No vertex format defined. Use goldsrc unless we have multiple bone joint
      // influences for any vertex.
      m_multipleVertexInfluences = false;
      unsigned vcount = m->getVertexCount();
      Model::InfluenceList il;
      for ( unsigned v = 0; m_multipleVertexInfluences == false && v < vcount; v++ )
      {
         m->getVertexInfluences( v, il );
         if ( il.size() > 1 )
         {
            m_multipleVertexInfluences = true;
         }
      }
   }

   if ( m->getMetaData( "smd_points_as_joints", value, sizeof(value) ) && atoi( value ) != 0 )
   {
      m_savePointsJoint = true;
   }
   else
   {
      m_savePointsJoint = false;
   }
}

SmdFilter::SmdFilter()
{
}

SmdFilter::~SmdFilter()
{
}

Model::ModelErrorE SmdFilter::readFile( Model * model, const char * const filename )
{
   return Model::ERROR_UNSUPPORTED_OPERATION;
}

Model::ModelErrorE SmdFilter::writeFile( Model * model, const char * const filename, ModelFilter::Options * o )
{
   if ( model && filename && filename[0] )
   {
      release_ptr<SmdOptions> freeOptions = NULL;
      m_options = dynamic_cast<SmdOptions *>( o );
      if ( !m_options )
      {
         freeOptions = static_cast< SmdOptions * >( getDefaultOptions() );
         m_options = freeOptions.get();
      }

      // Check for identical bone joint names
      {
         unsigned c = model->getBoneJointCount();
         for ( unsigned i = 0; i < c; i++ )
         {
            for ( unsigned j = i+1; j < c; j++ )
            {
               if ( strcmp( model->getBoneJointName( i ), model->getBoneJointName( j ) ) == 0 )
               {
                  model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "Bone joints must have unique names for SMD export." ) ).c_str() );
                  return Model::ERROR_FILTER_SPECIFIC;
               }
            }
         }
      }

      if ( m_options->m_savePointsJoint )
      {
         unsigned pcount = model->getPointCount();
         for ( unsigned p = 0; p < pcount; ++p )
         {
            Model::InfluenceList il;
            model->getPointInfluences( p, il );

            if ( il.size() > 1 )
            {
               model->setFilterSpecificError( transll( QT_TRANSLATE_NOOP( "LowLevel", "SMD export requires points to only have one bone influence." ) ).c_str() );
               return Model::ERROR_FILTER_SPECIFIC;
            }
         }
      }

      Model::ModelErrorE err = Model::ERROR_NONE;
      DataDest * dst = openOutput( filename, err );
      DestCloser fc( dst );

      if ( err != Model::ERROR_NONE )
         return err;

      // Use the load matrix and then invert it
      Matrix saveMatrix;
      saveMatrix.setRotationInDegrees( -90, 0, 0 );
      saveMatrix = saveMatrix.getInverse();

      int boneCount = model->getBoneJointCount();
      int pointCount = model->getPointCount();
      bool defaultBoneJoint = false;

      if ( boneCount == 0 && m_options->m_saveMeshes && model->getTriangleCount() > 0 )
      {
         defaultBoneJoint = true;
      }

      //
      // Write Header
      //
      writeLine( dst, "version 1" );

      //
      // Write Joints list
      //
      writeLine( dst, "nodes" );

      if ( defaultBoneJoint )
      {
         writeLine( dst, "0 \"root\" -1" );
      }
      else
      {
         for ( int bone = 0; bone < boneCount; bone++ )
         {
            int parent = model->getBoneJointParent( bone );

            writeLine( dst, "%d \"%s\" %d", bone, model->getBoneJointName( bone ), parent );
         }
      }

      if ( m_options->m_savePointsJoint )
      {
         for ( int point = 0; point < pointCount; point++ )
         {
            int parent = model->getPrimaryPointInfluence( point );

            writeLine( dst, "%d \"%s\" %d", (int)defaultBoneJoint + boneCount + point, model->getPointName( point ), parent );
         }
      }

      writeLine( dst, "end" );

      //
      // Write Bind Pose
      //
      if ( m_options->m_animations.size() > 0 && m_options->m_animations[0] == UINT_MAX )
      {
         writeLine( dst, "skeleton" );

         // write base pose as frame 0
         writeLine( dst, "time 0" );

         if ( defaultBoneJoint )
         {
            writeLine( dst, "%d %.6f %.6f %.6f %.6f %.6f %.6f",
                       0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
         }
         else
         {
            for ( int bone = 0; bone < boneCount; bone++ )
            {
               int parent = model->getBoneJointParent( bone );

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

               double trans[3];
               double rot[3];
               lm.getTranslation( trans );
               lm.getRotation( rot );

               writeLine( dst, "%d %.6f %.6f %.6f %.6f %.6f %.6f", bone,
                          (float)trans[0], (float)trans[1], (float)trans[2],
                          (float)rot[0], (float)rot[1], (float)rot[2] );
            }
         }

         if ( m_options->m_savePointsJoint )
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

               double trans[3];
               double rot[3];
               lm.getTranslation( trans );
               lm.getRotation( rot );

               writeLine( dst, "%d %.6f %.6f %.6f %.6f %.6f %.6f", boneCount + point,
                          (float)trans[0], (float)trans[1], (float)trans[2],
                          (float)rot[0], (float)rot[1], (float)rot[2] );
            }
         }

         writeLine( dst, "end" );
      }

      //
      // Write Animations
      //
      if ( m_options->m_animations.size() > 0 )
      {
         vector<Matrix> poseFinal;

         poseFinal.reserve( boneCount );

         // Animation numbers to export are set by smdprompt_show()
         // Note: Prompt is limited to only selecting one animation as SMD doesn't officially support multiple animations in one SMD.
         std::vector<unsigned>::iterator it;
         for (it = m_options->m_animations.begin(); it != m_options->m_animations.end(); it++)
         {
            if ( *it == UINT_MAX )
            {
               // Bind pose was already written
               continue;
            }

            unsigned anim = *it;
            //const char *animName = model->getAnimName( Model::ANIMMODE_SKELETAL, anim );
            float fps = model->getAnimFPS( Model::ANIMMODE_SKELETAL, anim );
            unsigned frameCount = model->getAnimFrameCount( Model::ANIMMODE_SKELETAL, anim );
            bool loop = model->getAnimLooping( Model::ANIMMODE_SKELETAL, anim );

            if ( frameCount == 0 )
            {
               continue;
            }

            //writeLine( dst, "// animation: %s, framerate %.6f%s", animName, fps, loop ? ", looping" : "" );
            writeLine( dst, "skeleton" );

            for ( unsigned frame = 0; frame < frameCount; frame++ )
            {
               double frameTime = frame / (double)fps;

               // "time N" must be written even if there are no keyframes for this frame.
               writeLine( dst, "time %d", frame );

               if ( defaultBoneJoint )
               {
                  writeLine( dst, "%d %.6f %.6f %.6f %.6f %.6f %.6f",
                             0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
               }
               else
               {
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

                     double trans[3];
                     double rot[3];
                     lm.getTranslation( trans );
                     lm.getRotation( rot );

                     // Studiomdl copies previous keyframe when a joint keyframes is not specified
                     // instead of lerping between specified keyframes. So just write all joint
                     // keyframes for all frames.

                     // FIXME?: Printf %.6f can round "rot" to 0.000001 greater than M_PI.
                     writeLine( dst, "%d %.6f %.6f %.6f %.6f %.6f %.6f", bone,
                                (float)trans[0], (float)trans[1], (float)trans[2],
                                (float)rot[0], (float)rot[1], (float)rot[2] );
                  }
               }

               // This is the same as point joint matricies. There is
               // no animation. It's only useful for compatibility with
               // programs that require all joints to be animated.
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

                     double trans[3];
                     double rot[3];
                     lm.getTranslation( trans );
                     lm.getRotation( rot );

                     // FIXME?: Printf %.6f can round "rot" to 0.000001 greater than M_PI.
                     writeLine( dst, "%d %.6f %.6f %.6f %.6f %.6f %.6f", boneCount + point,
                                (float)trans[0], (float)trans[1], (float)trans[2],
                                (float)rot[0], (float)rot[1], (float)rot[2] );
                  }
               }
            }

            writeLine( dst, "end" );
         }
      }

      //
      // Write Meshes
      //
      if ( m_options->m_saveMeshes && model->getTriangleCount() > 0 )
      {
         writeLine( dst, "triangles" );

         vector<Model::Material *> & modelMaterials = getMaterialList( model );

         int groupCount = model->getGroupCount();
         for ( int g = 0; g < groupCount+1; g++ )
         {
            int matId;
            list<int> faces;
            list<int>::iterator fit;
            std::string materialName;

            if ( g == groupCount )
            {
               matId = -1;
               faces = model->getUngroupedTriangles();
            }
            else
            {
               matId = model->getGroupTextureId( g );
               faces = model->getGroupTriangles( g );
            }

            if ( matId != -1 )
            {
               // TODO: Add option to save material name instead of texture name?
               //materialName = modelMaterials[ matId ]->m_name;
               materialName = PORT_basename( modelMaterials[ matId ]->m_filename.c_str() );
            }

            if ( materialName.length() == 0 )
            {
               materialName = "default.bmp";
            }

            for ( fit = faces.begin(); fit != faces.end(); fit++ )
            {
               writeLine( dst, "%s", materialName.c_str() );

               for ( int vindex = 0; vindex < 3; vindex++ )
               {
                  int vert = model->getTriangleVertex( *fit, vindex );
                  double meshVec[4] = { 0,0,0,1 };
                  float meshNor[4] = { 0,0,0,1 };
                  float uv[4] = { 0,0 };

                  model->getVertexCoordsUnanimated( vert, meshVec );
                  model->getNormal( *fit, vindex, meshNor );
                  model->getTextureCoords( *fit, vindex, uv[0], uv[1] );

                  saveMatrix.apply( meshVec );
                  saveMatrix.apply( meshNor );

                  // TODO: Move to Model::getVertexInfluencesNormalizedNonZero( vertex, list, epsilon ) or something.
                  //       model->getVertexBoneJoint( vert ) returns a bone even if the weight is zero or would be zero in printf.
                  Model::InfluenceList influences;
                  Model::InfluenceList::iterator it;
                  model->getVertexInfluences( vert, influences );

                  // Sort highest weight first
                  influences.sort(std::greater<Model::InfluenceT>());

                  // Our weights don't always equal 100%, get total weigth so we can normalize
                  double total = 0.0;
                  for ( it = influences.begin(); it != influences.end(); it++ )
                  {
                     total += it->m_weight;
                  }

                  // Don't allow negative weights, or divide by zero
                  if ( total < 0.0005 )
                  {
                     total = 1.0;
                  }

                  size_t weightCount = 0;
                  for ( it = influences.begin(); it != influences.end(); it++, weightCount++ )
                  {
                     it->m_weight = (it->m_weight / total);

                     // Lower than 0.0000005 round to 0 in prinf %.6f
                     if ( it->m_weight < 5e-7 )
                     {
                        // TODO: For each previous weight add it->m_weight / weightCount ?
                        influences.resize( weightCount );
                        break;
                     }
                  }

                  int vertBone = 0;
                  if ( !m_options->m_multipleVertexInfluences )
                  {
                     if ( influences.size() > 0 )
                     {
                        vertBone = influences.begin()->m_boneId;
                     }
                  }

                  dst->writePrintf( "%d %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f",
                     vertBone,
                     (float)meshVec[0], (float)meshVec[1], (float)meshVec[2],
                     meshNor[0], meshNor[1], meshNor[2],
                     uv[0], uv[1] );

                  if ( m_options->m_multipleVertexInfluences )
                  {
                     dst->writePrintf( " %d", (int) influences.size() );
                     for ( it = influences.begin(); it != influences.end(); it++ )
                     {
                        dst->writePrintf( " %d %.6f", it->m_boneId, (float)it->m_weight );
                     }
                  }

                  dst->writePrintf( "\r\n" );
               }
            }
         }

         writeLine( dst, "end" );
      }

      if ( m_options->m_multipleVertexInfluences )
      {
         model->updateMetaData( "smd_vertex_format", "source" );
      }
      else
      {
         model->updateMetaData( "smd_vertex_format", "goldsrc" );
      }

      if ( m_options->m_savePointsJoint )
      {
         model->updateMetaData( "smd_points_as_joints", "1" );
      }
      else
      {
         model->updateMetaData( "smd_points_as_joints", "0" );
      }

      model->operationComplete( transll( QT_TRANSLATE_NOOP( "LowLevel", "Set meta data for SMD export" ) ).c_str() );

      return Model::ERROR_NONE;
   }
   else
   {
      log_error( "no filename supplied for model filter" );
      return Model::ERROR_BAD_ARGUMENT;
   }
}

bool SmdFilter::writeLine( DataDest *dst, const char * line, ... )
{
   va_list ap;
   va_start( ap, line );
   dst->writeVPrintf( line, ap );
   va_end( ap );
   dst->writePrintf( "\r\n" );
   return true;
}

bool SmdFilter::canRead( const char * filename )
{
   return false;
}

bool SmdFilter::canWrite( const char * filename )
{
   return true;
}

bool SmdFilter::canExport( const char * filename )
{
   return true;
}

bool SmdFilter::isSupported( const char * filename )
{
   if ( filename )
   {
      unsigned len = strlen( filename );

      if ( len >= 4 && strcasecmp( &filename[len-4], ".smd" ) == 0 )
      {
         return true;
      }
   }

   return false;
}

list<string> SmdFilter::getReadTypes()
{
   list<string> rval;

   //rval.push_back( "*.smd" );

   return rval;
}

list<string> SmdFilter::getWriteTypes()
{
   list<string> rval;

   rval.push_back( "*.smd" );

   return rval;
}
