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


#include "decalmgr.h"

#include "decal.h"
#include "log.h"

using std::map;
using std::list;

DecalManager * DecalManager::s_instance = NULL;

DecalManager::DecalManager()
{
}

DecalManager::~DecalManager()
{
   // We only need to delete the lists that we're pointing to.
   // We do not need to delete the Models or the Tool::Parents in the map/lists

   log_debug( "DecalManager releasing %d model parent lists\n", m_parents.size() );
   ModelParentMap::iterator it;

   for ( it = m_parents.begin(); it != m_parents.end(); it++ )
   {
      delete (*it).second;
   }

   m_parents.clear();
}

DecalManager * DecalManager::getInstance()
{
   if ( s_instance == NULL )
   {
      s_instance = new DecalManager();
   }

   return s_instance;
}

void DecalManager::release()
{
   if ( s_instance != NULL )
   {
      delete s_instance;
      s_instance = NULL;
   }
}

void DecalManager::registerModel( Model * model )
{
   if ( model )
   {
      if ( m_parents.find( model ) == m_parents.end() )
      {
         m_parents[ model ] = new list<Tool::Parent *>;
      }
   }
}

void DecalManager::registerToolParent( Tool::Parent * parent )
{
   if ( parent )
   {
      Model * model = parent->getModel();

      registerModel( model );

      m_parents[ model ]->push_back( parent );
   }
}

void DecalManager::unregisterModel( Model * model )
{
   ModelParentMap::iterator it = m_parents.find( model );
   if ( it != m_parents.end() )
   {
      delete (*it).second;            // delete Tool::Parent list
      m_parents.erase( model ); // remove Model key
   }
}

void DecalManager::unregisterToolParent( Tool::Parent * parent )
{
   Model * model = parent->getModel();
   ModelParentMap::iterator it = m_parents.find( model );
   if ( it != m_parents.end() )
   {
      m_parents[model]->remove( parent );
   }
}

void DecalManager::addDecalToModel( Decal * decal, Model * model )
{
   if ( m_parents.find( model ) != m_parents.end() )
   {
      ParentList::iterator it;

      for ( it = m_parents[model]->begin(); it != m_parents[model]->end(); it++ )
      {
         (*it)->addDecal( decal );
      }

      m_decalModels[decal] = model;
   }
}

void DecalManager::addDecalToParent( Decal * decal, Tool::Parent * parent )
{
   Model * model = parent->getModel();
   if ( m_parents.find( model ) != m_parents.end() )
   {
      ParentList::iterator it;

      for ( it = m_parents[model]->begin(); it != m_parents[model]->end(); it++ )
      {
         if ( (*it) == parent )
         {
            (*it)->addDecal( decal );
            break;
         }
      }

      m_decalParents[decal] = parent;
   }
}

void DecalManager::removeDecal( Decal * decal )
{
   DecalModelMap::iterator mit = m_decalModels.find( decal );
   if ( mit != m_decalModels.end() && m_parents.find( (*mit).second ) != m_parents.end() )
   {
      Model * model = m_decalModels[decal];

      ParentList::iterator it;
      for ( it = m_parents[model]->begin(); it != m_parents[model]->end(); it++ )
      {
         (*it)->removeDecal( decal );
      }
   }

   ModelParentMap::iterator mpit;
   DecalParentMap::iterator pit = m_decalParents.find( decal );

   if ( pit != m_decalParents.end() )
   {
      Tool::Parent * parent = m_decalParents[decal];
      for ( mpit = m_parents.begin(); mpit != m_parents.end(); mpit++ )
      {
         Model * model = (*mpit).first;

         ParentList::iterator plit;

         for ( plit = m_parents[model]->begin(); plit != m_parents[model]->end(); plit++ )
         {
            if ( (*plit) == parent )
            {
               (*plit)->removeDecal( decal );
               break;
            }
         }
      }
   }
}

void DecalManager::modelUpdated( Model * model )
{
   ModelParentMap::iterator mit = m_parents.find( model );
   if ( mit != m_parents.end() )
   {
      for ( ParentList::iterator it = mit->second->begin();
            it != mit->second->end(); it++ )
      {
         (*it)->updateView();
      }
   }
}

void DecalManager::modelAnimate( Model * model )
{
   ModelParentMap::iterator mit = m_parents.find( model );
   if ( mit != m_parents.end() )
   {
      for ( ParentList::iterator it = mit->second->begin(); it != mit->second->end(); it++ )
      {
         (*it)->update3dView();
      }
   }
}
