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


#include "model.h"

#include "log.h"

#include <set>

#ifdef MM3D_EDIT
#include "modelundo.h"
#endif // MM3D_EDIT

#ifdef MM3D_EDIT

int Model::addGroup( const char * name )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return -1;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return -1;
   }

   m_changeBits |= AddOther;

   if ( name )
   {
      int num = m_groups.size();

      Group * group = Group::get();
      group->m_name = name;
      m_groups.push_back( group );

      MU_AddGroup * undo = new MU_AddGroup();
      undo->addGroup( num, group );
      sendUndo( undo );

      return num;
   }
   else
   {
      return -1;
   }
}

void Model::deleteGroup( unsigned groupNum )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return;
   }
   if ( m_frameAnims.size() > 0 && !m_forceAddOrDelete)
   {
      displayFrameAnimPrimitiveError();
      return;
   }

   MU_DeleteGroup * undo = new MU_DeleteGroup();
   undo->deleteGroup( groupNum, m_groups[ groupNum ] );
   sendUndo( undo );

   removeGroup( groupNum );
}

bool Model::setGroupSmooth( unsigned groupNum, uint8_t smooth )
{
   if ( m_animationMode )
   {
      return false;
   }

   if ( groupNum < m_groups.size()  )
   {
      MU_SetGroupSmooth * undo = new MU_SetGroupSmooth();
      undo->setGroupSmooth( groupNum, smooth, m_groups[groupNum]->m_smooth );
      sendUndo( undo );

      m_groups[ groupNum ]->m_smooth = smooth;
      m_validNormals = false;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setGroupAngle( unsigned groupNum, uint8_t angle )
{
   if ( m_animationMode )
   {
      return false;
   }

   if ( groupNum < m_groups.size()  )
   {
      MU_SetGroupAngle * undo = new MU_SetGroupAngle();
      undo->setGroupAngle( groupNum, angle, m_groups[groupNum]->m_angle );
      sendUndo( undo );

      m_groups[ groupNum ]->m_angle = angle;
      m_validNormals = false;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setGroupName( unsigned groupNum, const char * name )
{
   if ( m_animationMode )
   {
      return false;
   }

   if ( groupNum >= 0 && groupNum < m_groups.size() && name )
   {
      MU_SetGroupName * undo = new MU_SetGroupName();
      undo->setGroupName( groupNum, name, m_groups[groupNum]->m_name.c_str() );
      sendUndo( undo );

      m_groups[ groupNum ]->m_name = name;
      return true;
   }
   else
   {
      return false;
   }
}

void Model::setSelectedAsGroup( unsigned groupNum )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;
   invalidateNormals();

   m_validBspTree = false;

   if ( groupNum < m_groups.size() )
   {
      Group * grp = m_groups[groupNum];
      unsigned t = 0;

      while ( !grp->m_triangleIndices.empty() )
      {
         removeTriangleFromGroup( groupNum, *grp->m_triangleIndices.begin() );
      }

      // Put selected triangles into group groupNum
      for ( t = 0; t < m_triangles.size(); t++ )
      {
         if ( m_triangles[t]->m_selected )
         {
            addTriangleToGroup( groupNum, t );
         }
      }

      // Remove selected triangles from other groups
      for ( t = 0; t < m_triangles.size(); t++ )
      {
         if ( m_triangles[t]->m_selected )
         {
            for ( unsigned g = 0; g < m_groups.size(); g++ )
            {
               if ( g != groupNum )
               {
                  removeTriangleFromGroup( g, t );
               }
            }
         }
      }
   }
}

void Model::addSelectedToGroup( unsigned groupNum )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;
   invalidateNormals();

   m_validBspTree = false;

   if ( groupNum < m_groups.size() )
   {
      for ( unsigned t = 0; t < m_triangles.size(); t++ )
      {
         if ( m_triangles[t]->m_selected )
         {
            int g = getTriangleGroup( t );
            if ( g != (int) groupNum )
            {
               if ( g >= 0 )
                  removeTriangleFromGroup( g, t );
               addTriangleToGroup( groupNum, t );
            }
         }
      }
   }
}

void Model::addTriangleToGroup( unsigned groupNum, unsigned triangleNum )
{
   LOG_PROFILE();
   if ( m_animationMode )
   {
      return;
   }

   m_changeBits |= AddOther;

   if ( groupNum < m_groups.size() && triangleNum < m_triangles.size() )
   {
      m_validBspTree = false;

      m_groups[groupNum]->m_triangleIndices.insert( triangleNum );

      MU_AddToGroup * undo = new MU_AddToGroup();
      undo->addToGroup( groupNum, triangleNum );
      sendUndo( undo );
   }
   else
   {
      log_error( "addTriangleToGroup(%d, %d) argument out of range\n", 
            groupNum, triangleNum );
   }
}

void Model::removeTriangleFromGroup( unsigned groupNum, unsigned triangleNum )
{
   if ( m_animationMode )
   {
      return;
   }

   m_validBspTree = false;

   if ( groupNum < m_groups.size() && triangleNum < m_triangles.size() )
   {
      Group * grp = m_groups[ groupNum ];
      std::set<int>::iterator it = grp->m_triangleIndices.find( triangleNum );
      if ( it != grp->m_triangleIndices.end() )
      {
         grp->m_triangleIndices.erase( it );

         MU_RemoveFromGroup * undo = new MU_RemoveFromGroup();
         undo->removeFromGroup( groupNum, triangleNum );
         sendUndo( undo );
      }
   }
   else
   {
      log_error( "addTriangleToGroup(%d, %d) argument out of range\n", 
            groupNum, triangleNum );
   }
}

#endif // MM3D_EDIT

list<int> Model::getUngroupedTriangles() const
{
   list<int> triangles;

   unsigned t = 0;
   unsigned tcount = m_triangles.size();

   for ( t = 0; t < tcount; t++ )
   {
      m_triangles[t]->m_marked = false;
   }

   unsigned g = 0;
   unsigned gcount = m_groups.size();

   for ( g = 0; g < gcount; g++ )
   {
      Group * grp = m_groups[g];
      for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
            it != grp->m_triangleIndices.end();
            ++it )
      {
         m_triangles[ *it ]->m_marked = true;
      }
   }

   for ( t = 0; t < tcount; t++ )
   {
      if ( ! m_triangles[t]->m_marked )
      {
         triangles.push_back( t );
      }
   }

   return triangles;
}

list<int> Model::getGroupTriangles( unsigned groupNumber ) const
{
   list<int> triangles;
   if ( groupNumber < m_groups.size() )
   {
      Group * grp = m_groups[ groupNumber ];
      for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
            it != grp->m_triangleIndices.end();
            ++it )
      {
         triangles.push_back( *it );
      }
   }

   return triangles;
}

int Model::getTriangleGroup( unsigned triangleNumber ) const
{
   for ( unsigned g = 0; g < m_groups.size(); g++ )
   {
      Group * grp = m_groups[g];
      if ( grp->m_triangleIndices.end()
            != grp->m_triangleIndices.find( triangleNumber ) )
      {
         return g;
      }
   }
   
   // Triangle is not in a group
   return -1;
}

const char * Model::getGroupName( unsigned groupNum ) const
{
   if ( groupNum >= 0 && groupNum < m_groups.size() )
   {
      return m_groups[ groupNum ]->m_name.c_str();
   }
   else
   {
      return NULL;
   }
}

int Model::getGroupByName( const char * const groupName, bool ignoreCase ) const
{
   int (*compare)(const char *, const char *);
   compare = ignoreCase ? strcasecmp : strcmp;

   int groupNumber = -1;

   for ( unsigned g = 0; g < m_groups.size(); g++ )
   {
      if ( compare( groupName, m_groups[g]->m_name.c_str() ) == 0 )
      {
         groupNumber = g;
         break;
      }
   }

   return groupNumber;
}

uint8_t Model::getGroupSmooth( unsigned groupNum  ) const
{
   if ( groupNum < m_groups.size()  )
   {
      return m_groups[ groupNum ]->m_smooth;
   }
   else
   {
      return 0;
   }
}

uint8_t Model::getGroupAngle( unsigned groupNum  ) const
{
   if ( groupNum < m_groups.size()  )
   {
      return m_groups[ groupNum ]->m_angle;
   }
   else
   {
      return 180;
   }
}

int Model::removeUnusedGroups()
{
   int removed = 0;
   for ( int g = m_groups.size() - 1; g >=0; --g )
   {
      if ( m_groups[g]->m_triangleIndices.empty() )
      {
         ++removed;
         deleteGroup( g );
      }
   }
   return removed;
}

int Model::mergeIdenticalGroups()
{
   int merged = 0;
   std::set<int> toRemove;
   int groupCount = m_groups.size();
   for ( int g = 0; g < groupCount; ++g )
   {
      if ( toRemove.find(g) == toRemove.end() )
      {
         for ( int g2 = g + 1; g2 < groupCount; ++g2 )
         {
            Group * grp = m_groups[g2];
            if ( m_groups[g]->propEqual( *grp, ~Model::PropTriangles ) )
            {
               for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
                     it != grp->m_triangleIndices.end(); ++it ) {
                  addTriangleToGroup( g, *it );
               }
               toRemove.insert( g2 );
               ++merged;
            }
         }
      }
   }

   for ( int g = groupCount - 1; g >= 0; --g )
   {
      if ( toRemove.find(g) != toRemove.end() )
      {
         deleteGroup( g );
      }
   }
   return merged;
}

int Model::removeUnusedMaterials()
{
   int removed = 0;
   std::set<int> inUse;
   for ( int g = m_groups.size() - 1; g >=0; --g )
   {
      int mat = m_groups[g]->m_materialIndex;
      if ( mat >= 0 )
      {
         inUse.insert( mat );
      }
   }
   for ( int m = m_materials.size() - 1; m >= 0; --m )
   {
      if ( inUse.find( m ) == inUse.end() )
      {
         ++removed;
         deleteTexture( m );
      }
   }
   return removed;
}

int Model::mergeIdenticalMaterials()
{
   int merged = 0;
   std::set<int> toRemove;
   int matCount = m_materials.size();
   for ( int m = 0; m < matCount; ++m )
   {
      if ( toRemove.find(m) == toRemove.end() )
      {
         for ( int m2 = m + 1; m2 < matCount; ++m2 )
         {
            Material * mat = m_materials[m2];
            if ( m_materials[m]->propEqual( *mat ) )
            {
               for ( unsigned g = 0; g < m_groups.size(); ++g )
               {
                  if ( m_groups[g]->m_materialIndex == m2 )
                  {
                     setGroupTextureId( g, m );
                  }
               }
               toRemove.insert( m2 );
               ++merged;
            }
         }
      }
   }

   for ( int m = matCount - 1; m >= 0; --m )
   {
      if ( toRemove.find(m) != toRemove.end() )
      {
         deleteTexture( m );
      }
   }
   return merged;
}

