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


#ifndef __FILTERMGR_H
#define __FILTERMGR_H

//------------------------------------------------------------------
// About the FilterManager class
//------------------------------------------------------------------
//
// The FilterManager class is a singleton that keeps a list of filters
// for importing and exporting models to various file formats.
//
// To create a new filter you must derive from the ModelFilter class
// and register your new filter with the FilterManager instance.  If
// your filter is called MyFilter, you would register your filter with
// the following function:
//
//    MyFilter * mf = new MyFilter();
//    FilterManager::getInstance()->registerFilter( mf );
//
// This would usually be done in the plugin_init function of a plugin.
// You only need one instance of your filter.
//
// See the ModelFilter class for more information on what a filter
// implemention is required to do.
//
#include "model.h"

#include "filefactory.h"

#include <list>
#include <string>

class ModelFilter;
class FileFactory;

typedef std::list< ModelFilter * > FilterList;

class FilterManager
{
   public:
      enum _WriteOptions_e
      {
         WO_FilterDefault,
         WO_ModelDefault,
         WO_ModelNoPrompt
      };
      typedef enum _WriteOptions_e WriteOptionsE;

      static FilterManager * getInstance();
      static void release();

      bool registerFilter( ModelFilter * filter );

      Model::ModelErrorE readFile( Model * model, const char * filename );
      Model::ModelErrorE writeFile( Model * model, const char * filename, bool exportModel = false, WriteOptionsE wo = WO_ModelDefault );

      std::list< std::string > getAllReadTypes();
      std::list< std::string > getAllWriteTypes( bool exportModel = false );

   protected:
      FilterManager();
      ~FilterManager();

      static FilterManager * s_instance;
      FileFactory m_factory;
      FilterList m_filters;
};

#endif // __FILTERMGR_H
