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

#ifndef __SMDFILTER_H
#define __SMDFILTER_H

#include "modelfilter.h"

#include <string>

class DataDest;

class SmdFilter : public ModelFilter
{
   public:

      class SmdOptions : public ModelFilter::Options
      {
         public:
            SmdOptions();

            virtual void release() { delete this; };

            bool m_saveMeshes;
            bool m_savePointsJoint;
            bool m_multipleVertexInfluences;
            std::vector<unsigned int> m_animations;

            void setOptionsFromModel( Model * m );

         protected:
            virtual ~SmdOptions(); // Use release() instead
      };

      SmdFilter();
      virtual ~SmdFilter();

      Model::ModelErrorE readFile( Model * model, const char * const filename );
      Model::ModelErrorE writeFile( Model * model, const char * const filename, ModelFilter::Options * o = NULL );

      bool canRead( const char * filename = NULL );
      bool canWrite( const char * filename = NULL );
      bool canExport( const char * filename = NULL );

      bool isSupported( const char * file );

      std::list< std::string > getReadTypes();
      std::list< std::string > getWriteTypes();

      ModelFilter::Options * getDefaultOptions() { return new SmdOptions; };

   protected:

      SmdOptions  * m_options;

      bool writeLine( DataDest *dst, const char * line, ... ) __attribute__ ((format (printf, 3, 4)));
};

#endif // __SMDFILTER_H
