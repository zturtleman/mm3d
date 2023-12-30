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

#ifndef __IQEFILTER_H
#define __IQEFILTER_H

#include "modelfilter.h"

#include <string>

class DataDest;

class IqeFilter : public ModelFilter
{
   public:

      class IqeOptions : public ModelFilter::Options
      {
         public:
            IqeOptions();

            virtual void release() { delete this; };

            bool m_playerSupported;

            bool m_saveAsPlayer;
            bool m_saveAnimationCfg;

            bool m_saveMeshes;
            bool m_savePointsJoint;
            bool m_saveSkeleton;
            bool m_saveAnimations;
            std::vector<unsigned int> m_animations;

            void setOptionsFromModel( Model * m, const char * const filename );

         protected:
            virtual ~IqeOptions(); // Use release() instead
      };

      IqeFilter();
      virtual ~IqeFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o = NULL );

      bool canRead( const char * filename = NULL );
      bool canWrite( const char * filename = NULL );
      bool canExport( const char * filename = NULL );

      bool isSupported( const char * file );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      ModelFilter::Options * getDefaultOptions() { return new IqeOptions; };

   protected:

      typedef enum _MeshSection_e
      {
         MS_Lower = 0,
         MS_Upper,
         MS_Head,
         MS_MAX
      } MeshSectionE;

      // the order is important, used for writing continue frames by type
      typedef enum _MeshAnimationType_e
      {
         MA_All,
         MA_Both,
         MA_Torso,
         MA_Legs,
         // NOTE: Team Arena has extra torso animations after legs
         MA_Head,
         MA_MAX
      } MeshAnimationTypeE;

      IqeOptions  * m_options;
      Model       * m_model;
      std::string   m_modelPath;

      bool writeLine( DataDest *dst, const char * line, ... ) __attribute__ ((format (printf, 3, 4)));

      Model::ModelErrorE writeSectionFile( Model *model, const char * filename );

      bool     writeAnimations( bool playerModel, const char * filename );
      std::string getSafeName( unsigned int anim );
      bool     animSyncWarning(std::string name);
      MeshAnimationTypeE getAnimationType( bool playerModel, const std::string & animName );
      bool     animInSection( std::string animName, MeshSectionE section );
      bool getExportAnimData( bool playerModel, int modelAnim,
            int & fileFrame, int & frameCount, int & fps );
};

#endif // __IQEFILTER_H
