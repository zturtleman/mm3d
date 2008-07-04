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


#include "undomgr.h"

#include "log.h"
#include "undo.h"

UndoManager::UndoManager()
   : m_currentUndo( NULL ),
     m_currentList( NULL ),
     m_listCombine( true ),
     m_sizeLimit( 0 ),
     m_countLimit( 0 ),
     m_saveLevel( 0 )
{
}

UndoManager::~UndoManager()
{
   clear();
}

void UndoManager::setSaved()
{
   m_saveLevel = 0;
}

bool UndoManager::isSaved() const
{
   return ( m_saveLevel == 0 ) ? true : false;
}

void UndoManager::clear()
{
   operationComplete( "Doomed operation" );
   clearRedo();

   while ( ! m_atomic.empty() )
   {
      UndoList * l = m_atomic.front();
      while ( ! l->empty() )
      {
         l->front()->undoRelease();
         l->front()->release();
         l->pop_front();
      }
      delete l;
      m_atomic.pop_front();
   }
}

void UndoManager::addUndo( Undo * u, bool listCombine )
{
   clearRedo();

   if ( !listCombine )
   {
      m_listCombine = false;
   }

   if ( m_currentUndo )
   {
      // Try to combine these undo items
      if ( combineWithList( u ) )
      {
         // Combined, release new one
         u->release();
      }
      else
      {
         // Push the current undo onto the current list and make this undo current
         pushUndoToList( m_currentUndo );
         m_currentUndo = u;
      }
   }
   else
   {
      // No undo yet, this is our first
      m_currentUndo = u;
   }
}

void UndoManager::operationComplete( const char * opname )
{
   // if we have anything to undo
   if ( m_currentUndo )
   {
      log_debug( "operation complete: %s\n", opname );
      m_saveLevel++;
      pushUndoToList( m_currentUndo );
      m_currentList->setOpName( opname );
      m_atomic.push_front( m_currentList );
   }
   else
   {
      log_debug( "nothing to undo\n" );
   }
   m_currentList = NULL;
   m_currentUndo = NULL;
   m_listCombine = true;

   checkSize();

   showStatistics();
}

UndoList * UndoManager::undo()
{
   LOG_PROFILE();

   operationComplete();

   if ( ! m_atomic.empty() )
   {
      m_atomicRedo.push_front( m_atomic.front() );
      m_atomic.pop_front();

      showStatistics();

      log_debug( "Undo: %s\n", m_atomicRedo.front()->getOpName() );

      m_saveLevel--;

      return m_atomicRedo.front();
   }
   else
   {
      return NULL;
   }
}

UndoList * UndoManager::redo()
{
   LOG_PROFILE();

   operationComplete();

   if ( ! m_atomicRedo.empty() )
   {
      m_atomic.push_front( m_atomicRedo.front() );
      m_atomicRedo.pop_front();

      showStatistics();

      log_debug( "Redo: %s\n", m_atomic.front()->getOpName() );

      m_saveLevel++;

      return m_atomic.front();
   }
   else
   {
      return NULL;
   }
}

const char * UndoManager::getUndoOpName() const
{
   if ( m_currentUndo )
   {
      return "";
   }
   else if ( ! m_atomic.empty() )
   {
      return m_atomic.front()->getOpName();
   }
   else
   {
      return "";
   }
}

const char * UndoManager::getRedoOpName() const
{
   if ( ! m_atomicRedo.empty() )
   {
      return m_atomicRedo.front()->getOpName();
   }
   else
   {
      return "";
   }
}

UndoList * UndoManager::undoCurrent()
{
   LOG_PROFILE();

   if ( m_currentUndo )
   {
      operationComplete( "Partial operation" );

      if ( ! m_atomic.empty() )
      {
         m_atomicRedo.push_front( m_atomic.front() );
         m_atomic.pop_front();

         log_debug( "Undo: %s\n", m_atomicRedo.front()->getOpName() );
         return m_atomicRedo.front();
      }
   }
   return NULL;
}

bool UndoManager::combineWithList( Undo * u )
{
   if ( m_currentUndo->combine( u ) )
   {
      return true;
   }
   else if ( m_listCombine )
   {
      if ( m_currentList )
      {
         UndoList::iterator it;
         for ( it = m_currentList->begin(); it != m_currentList->end(); it++ )
         {
            if ( (*it)->combine( u ) )
            {
               return true;
            }
         }
      }
   }
   return false;
}

void UndoManager::pushUndoToList( Undo * u )
{
   if ( !m_currentList )
   {
      m_currentList = new UndoList;
   }
   m_currentList->push_back( u );
}

void UndoManager::clearRedo()
{
   while ( ! m_atomicRedo.empty() )
   {
      UndoList * list = m_atomicRedo.front();
      while ( ! list->empty() )
      {
         list->front()->redoRelease();
         list->front()->release();
         list->pop_front();
      }
      delete list;
      m_atomicRedo.pop_front();
   }
}

void UndoManager::showStatistics() const
{
   int undoItems = 0;
   int redoItems = 0;
   unsigned undoSize = 0;
   unsigned redoSize = 0;

   AtomicList::const_iterator it;
   UndoList::const_iterator uit;

   log_debug( "Undo:\n" );
   for ( it = m_atomic.begin(); it != m_atomic.end(); it++ )
   {
      log_debug( "  %s\n", (*it)->getOpName() );
      undoItems += (*it)->size();
      for ( uit = (*it)->begin(); uit != (*it)->end(); uit++ )
      {
         undoSize += (*uit)->size();
      }
   }
   log_debug( "\n" );

   log_debug( "Redo:\n" );
   for ( it = m_atomicRedo.begin(); it != m_atomicRedo.end(); it++ )
   {
      log_debug( "  %s\n", (*it)->getOpName() );
      redoItems += (*it)->size();
      for ( uit = (*it)->begin(); uit != (*it)->end(); uit++ )
      {
         redoSize += (*uit)->size();
      }
   }
   log_debug( "\n" );

   log_debug( "--------------- Undo statistics ---------------\n" );
   log_debug( " undo:  %7d size, %5d items, %5d lists\n", undoSize, undoItems, m_atomic.size() );
   log_debug( " redo:  %7d size, %5d items, %5d lists\n", redoSize, redoItems, m_atomicRedo.size() );
   log_debug( " total: %7d size, %5d items, %5d lists\n", undoSize + redoSize, undoItems + redoItems, m_atomic.size() +m_atomicRedo.size() );
   log_debug( "-----------------------------------------------\n" );
}

void UndoManager::checkSize()
{
   if ( m_countLimit )
   {
      unsigned count = m_atomic.size();
      while ( count > 1 && count > m_countLimit )
      {
         UndoList * l = m_atomic.back();
         while ( ! l->empty() )
         {
            l->front()->undoRelease();
            l->front()->release();
            l->pop_front();
         }
         delete l;
         m_atomic.pop_back();
         count--;
      }
   }

   int undoItems = 0;
   unsigned undoSize = 0;

   AtomicList::iterator it;
   UndoList::iterator uit;

   bool getNewSize = true;

   while ( getNewSize ) 
   {
      getNewSize = false;
      undoSize = 0;

      for ( it = m_atomic.begin(); it != m_atomic.end(); it++ )
      {
         undoItems += (*it)->size();
         for ( uit = (*it)->begin(); uit != (*it)->end(); uit++ )
         {
            undoSize += (*uit)->size();
         }
      }

      if ( ( m_sizeLimit && undoSize > m_sizeLimit && m_atomic.size() > 1 )
            || (undoSize > MAX_UNDO_LIST_SIZE && m_atomic.size() > 1 )
            )
      {
         log_debug( "Undo list size is %d, freeing a list\n", undoSize );
         UndoList * l = m_atomic.back();
         m_atomic.pop_back();

         while ( ! l->empty() )
         {
            l->front()->undoRelease();
            l->front()->release();
            l->pop_front();
         }
         delete l;
         getNewSize = true;
      }
   }
}

