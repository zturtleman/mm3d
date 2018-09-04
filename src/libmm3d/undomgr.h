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


#ifndef __UNDOMGR_H
#define __UNDOMGR_H

#include <list>
#include <string.h>
#include <stdlib.h>

using std::list;

class Undo;
//typedef list< Undo * > UndoList;

class UndoList : public std::list<Undo *>
{
   public:
      UndoList() : m_name(NULL) {};
      virtual ~UndoList() { if ( m_name ) { free(m_name); } };

      void setOpName( const char * name ) { 
         if ( m_name ) { free(m_name); }
         if ( name )   { m_name = strdup(name); } };
      const char * getOpName() const { return m_name; };

   protected:
      char * m_name;
};

typedef list< UndoList * > AtomicList;

class UndoManager
{
   public:
      UndoManager();
      virtual ~UndoManager();

      void clear();

      void setSaved();
      bool isSaved() const;

      void addUndo( Undo * u, bool listCombine = false );
      void operationComplete( const char * opname = NULL );

      // Items should be applied in reverse order (back to front)
      UndoList * undo();

      // Items should be applied in forward order (front to back)
      UndoList * redo();

      // True if there is an undo list available
      bool canUndo() const { return m_atomic.empty() ? false : true; };

      // True if there is a redo list available
      bool canRedo() const { return m_atomicRedo.empty() ? false : true ; }; 

      const char * getUndoOpName() const;
      const char * getRedoOpName() const;

      // Only returns undo list if there is one being built
      // Items should be applied in reverse order (back to front)
      UndoList * undoCurrent();

      void showStatistics() const;

      void setSizeLimit( unsigned sizeLimit )   { m_sizeLimit  = sizeLimit; };
      void setCountLimit( unsigned countLimit ) { m_countLimit = countLimit; };

   protected:
      enum
      {
         MAX_UNDO_LIST_SIZE = 20000000  // 20mb
      };

      bool combineWithList( Undo * u );
      void pushUndoToList( Undo * u );
      void clearRedo();
      void checkSize();

      Undo       * m_currentUndo;
      UndoList   * m_currentList;
      AtomicList   m_atomic;
      AtomicList   m_atomicRedo;
      bool         m_listCombine;

      unsigned     m_sizeLimit;
      unsigned     m_countLimit;
      int          m_saveLevel;
};

#endif // __UNDOMGR_H
