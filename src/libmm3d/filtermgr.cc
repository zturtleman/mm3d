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


#include <stdio.h> // For NULL
#include "filtermgr.h"
#include "modelfilter.h"
#include "log.h"

#include <ctype.h>

using std::list;
using std::string;

FilterManager * FilterManager::s_instance = NULL;

FilterManager::FilterManager()
{
}

FilterManager::~FilterManager()
{
   log_debug( "FilterManager releasing %d filters\n", m_filters.size() );
   FilterList::iterator it = m_filters.begin();
   while ( it != m_filters.end() )
   {
      if ( *it )
      {
         (*it)->release();
      }
      it++;
   }
}

FilterManager * FilterManager::getInstance()
{
   if ( !s_instance )
   {
      s_instance = new FilterManager();
   }

   return s_instance;
}

void FilterManager::release()
{
   if ( s_instance != NULL )
   {
      delete s_instance;
      s_instance = NULL;
   }
}

bool FilterManager::registerFilter( ModelFilter * filter )
{
   if ( filter )
   {
      m_filters.push_back( filter );
      return true;
   }
   else
   {
      return false;
   }
}

Model::ModelErrorE FilterManager::readFile( Model * model, const char * filename )
{
   FilterList::iterator it;
   for ( it = m_filters.begin(); it != m_filters.end(); it++ )
   {
      ModelFilter * filter = *it;

      if ( filter && filter->isSupported( filename ) && filter->canRead() )
      {
         model->forceAddOrDelete( true );
         Model::ModelErrorE rval = filter->readFile( model, filename );
         model->forceAddOrDelete( false );
#ifdef MM3D_EDIT
         model->setUndoEnabled( true );
#endif // MM3D_EDIT
         return rval;
      }

   }

   return Model::ERROR_UNKNOWN_TYPE;
}

Model::ModelErrorE FilterManager::writeFile( Model * model, const char * filename, bool exportModel, FilterManager::WriteOptionsE wo )
{
   bool canWrite = true;
   bool tryExport = false;
   FilterList::iterator it;

   for ( it = m_filters.begin(); it != m_filters.end(); it++ )
   {
      ModelFilter * filter = *it;

      if ( filter && filter->isSupported( filename ) )
      {
         if ( (exportModel && filter->canExport()) || filter->canWrite() )
         {
            bool mustFree = false;
            ModelFilter::OptionsFuncF f = filter->getOptionsPrompt();
            ModelFilter::Options * o = NULL;
            switch ( wo )
            {
               case WO_FilterDefault:
                  o = filter->getDefaultOptions();
                  mustFree = true;
                  break;
               case WO_ModelDefault:
               case WO_ModelNoPrompt:
                  //o = model->getWriteOptions( filename );
                  if ( o == NULL )
                  {
                     o = filter->getDefaultOptions();
                     mustFree = true;
                  }
                  break;
               default:
                  break;
            }

            bool doWrite = true;
            if ( f != NULL && o != NULL && wo != WO_ModelNoPrompt )
            {
               doWrite = f( model, o );
            }

            Model::ModelErrorE err = Model::ERROR_NONE;

            if ( doWrite )
            {
               err = filter->writeFile( model, filename, o );
            }
            else
            {
               err = Model::ERROR_CANCEL;
            }

            if ( mustFree )
            {
               if ( o )
               {
                  o->release();
               }
               o = NULL;
            }

            return err;
         }
         else
         {
            if ( !exportModel && filter->canExport() )
            {
               tryExport = true;
            }
            else
            {
               canWrite = false;
            }
         }
      }
   }

   if ( tryExport )
   {
      return Model::ERROR_EXPORT_ONLY;
   }
   if ( canWrite )
   {
      return Model::ERROR_UNKNOWN_TYPE;
   }
   else
   {
      return Model::ERROR_UNSUPPORTED_OPERATION;
   }
}

list<string> FilterManager::getAllReadTypes()
{
   list<string> rval;
   FilterList::iterator filterIt = m_filters.begin();

   while ( filterIt != m_filters.end() )
   {
      list<string> temp = (*filterIt)->getReadTypes();
      while ( temp.size() )
      {
         // Add to read list if we can read a dummy file of this type
         string tempFile = string( "file." ) + temp.front();
         if ( (*filterIt)->canRead( tempFile.c_str() ) )
         {
            string s = temp.front();
            rval.push_back( s );

#ifndef WIN32
            unsigned len = s.length();
            for ( unsigned n = 0; n < len; n++ )
            {
               if ( isupper( s[n] ) )
               {
                  s[n] = tolower( s[n] );
               }
               else
               {
                  s[n] = toupper( s[n] );
               }
            }

            rval.push_back( s );
#endif // ! WIN32
         }
         temp.pop_front();
      }
      
      filterIt++;
   }

   return rval;
}

list<string> FilterManager::getAllWriteTypes( bool exportModel )
{
   list<string> rval;
   FilterList::iterator filterIt = m_filters.begin();

   while ( filterIt != m_filters.end() )
   {
      list<string> temp = (*filterIt)->getWriteTypes();
      while ( temp.size() )
      {
         // Add to write list if we can write a dummy file of this type
         string tempFile = string( "file." ) + temp.front();
         if ( (exportModel && (*filterIt)->canExport( tempFile.c_str() ))
               || (*filterIt)->canWrite( tempFile.c_str() ) )
         {
            rval.push_back( temp.front() );
         }
         temp.pop_front();
      }
      
      filterIt++;
   }

   return rval;
}

