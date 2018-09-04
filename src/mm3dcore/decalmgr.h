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


#ifndef __DECALMGR_H
#define __DECALMGR_H

#include "tool.h"

#include <map>
#include <list>

class Decal;
class Model;
class Tool;

class DecalManager
{
   public:
      
      static DecalManager * getInstance();
      static void release();

      void registerModel( Model * model );
      void registerToolParent( Tool::Parent * parent );

      void unregisterModel( Model * model );
      void unregisterToolParent( Tool::Parent * parent );

      void addDecalToModel( Decal * decal, Model * model );
      void addDecalToParent( Decal * decal, Tool::Parent * parent );

      void removeDecal( Decal * decal );

      void modelUpdated( Model * model );
      void modelAnimate( Model * model );

   protected:

      DecalManager();
      virtual ~DecalManager();

      static DecalManager * s_instance;

      typedef std::list<Tool::Parent *> ParentList;
      typedef std::map< Model *, ParentList * > ModelParentMap;

      typedef std::map< Decal *, Model *> DecalModelMap;
      typedef std::map< Decal *, Tool::Parent *> DecalParentMap;

      // Do not delete the items that this points to!
      ModelParentMap m_parents;
      DecalModelMap  m_decalModels;
      DecalParentMap m_decalParents;
};

#endif // __DECALMGR_H
