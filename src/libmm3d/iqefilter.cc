/*  IqeFilter plugin for Misfit Model 3D
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

static const char *IQE_HEADER = "# Inter-Quake Export";

IqeFilter::IqeOptions::IqeOptions()
   : m_saveMeshes( true ),
     m_savePointsJoint( true ),
     m_savePointsAnim( true ),
     m_saveSkeleton( true ),
     m_saveAnimations( true ),
     m_animations()
{
}

IqeFilter::IqeOptions::~IqeOptions()
{
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

      if ( !m_options->m_saveMeshes && !m_options->m_savePointsJoint && !m_options->m_saveSkeleton && !m_options->m_saveAnimations )
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

      if ( m_options->m_savePointsJoint || m_options->m_savePointsAnim )
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
      writeLine( dst, IQE_HEADER );
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

            writeLine( dst, "\tpq %.8f %.8f %.8f %.8f %.8f %.8f %.8f", trans[0], trans[1], trans[2], rot[0], rot[1], rot[2], rot[3] );
            //writeLine( dst, "\tpm %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f", trans[0], trans[1], trans[2],
            //      lm.get( 0, 0 ), lm.get( 0, 1 ), lm.get( 0, 2 ),
            //      lm.get( 1, 0 ), lm.get( 1, 1 ), lm.get( 1, 2 ),
            //      lm.get( 2, 0 ), lm.get( 2, 1 ), lm.get( 2, 2 ) );
         }

         writeLine( dst, "" );
      }

      //
      // Write Points as Joints
      //
      int pointCount = model->getPointCount();
      if ( m_options->m_savePointsJoint && pointCount > 0 )
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

            writeLine( dst, "\tpq %.8f %.8f %.8f %.8f %.8f %.8f %.8f", trans[0], trans[1], trans[2], rot[0], rot[1], rot[2], rot[3] );
            //writeLine( dst, "\tpm %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f", trans[0], trans[1], trans[2],
            //      lm.get( 0, 0 ), lm.get( 0, 1 ), lm.get( 0, 2 ),
            //      lm.get( 1, 0 ), lm.get( 1, 1 ), lm.get( 1, 2 ),
            //      lm.get( 2, 0 ), lm.get( 2, 1 ), lm.get( 2, 2 ) );
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
          mesh_create_list( meshes, model );

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

                writeLine( dst, "vp %.8lf %.8lf %.8lf", meshVec[0], meshVec[1], meshVec[2] );
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

                      dst->writePrintf( " %d %.8lf", it->m_boneId, weight );
                   }
#ifdef WIN32
                   dst->writePrintf( "\n" );
#else // WIN32
                   dst->writePrintf( "\r\n" );
#endif // WIN32
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

                  writeLine( dst, "pq %.8f %.8f %.8f %.8f %.8f %.8f %.8f", trans[0], trans[1], trans[2], rot[0], rot[1], rot[2], rot[3] );
                  //writeLine( dst, "pm %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f", trans[0], trans[1], trans[2],
                  //      lm.get( 0, 0 ), lm.get( 0, 1 ), lm.get( 0, 2 ),
                  //      lm.get( 1, 0 ), lm.get( 1, 1 ), lm.get( 1, 2 ),
                  //      lm.get( 2, 0 ), lm.get( 2, 1 ), lm.get( 2, 2 ) );
               }

               // This is the same as point joint matricies. There is
               // no animation. It's only useful for compatibility with
               // programs that require all joints to be animated.
               if ( m_options->m_savePointsAnim && pointCount > 0 )
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

                     writeLine( dst, "pq %.8f %.8f %.8f %.8f %.8f %.8f %.8f", trans[0], trans[1], trans[2], rot[0], rot[1], rot[2], rot[3] );
                     //writeLine( dst, "pm %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f", trans[0], trans[1], trans[2],
                     //      lm.get( 0, 0 ), lm.get( 0, 1 ), lm.get( 0, 2 ),
                     //      lm.get( 1, 0 ), lm.get( 1, 1 ), lm.get( 1, 2 ),
                     //      lm.get( 2, 0 ), lm.get( 2, 1 ), lm.get( 2, 2 ) );
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
#ifdef WIN32
   dst->writePrintf( "\n" );
#else // WIN32
   dst->writePrintf( "\r\n" );
#endif // WIN32
   return true;
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
