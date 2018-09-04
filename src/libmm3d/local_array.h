/*  Maverick Model 3D
 * 
 *  Copyright (c) 2007 Kevin Worcester
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


#ifndef __LOCAL_ARRAY_H
#define __LOCAL_ARRAY_H

#include <stdlib.h>

template<typename T> class local_array
{
   public:
      local_array( T * pval = NULL )
         : m_pval( pval ) {}
      ~local_array() { free_ptr(); }

      T * get() { return m_pval; }
      T * reset(T * newPval) { free_ptr(); return m_pval = newPval; }
      const T * get() const { return m_pval; }

      T & operator*() { return *m_pval; }
      T * operator->() { return m_pval; }
      const T & operator*() const { return *m_pval; }
      const T * operator->() const { return m_pval; }

      T * operator=(T* newPval) { return reset(newPval); }

      bool operator!() const { return m_pval == NULL; }
      bool isnull() const { return m_pval == NULL; }

   protected:
      void free_ptr() { delete[] m_pval; m_pval = NULL; }

      T * m_pval;
};

#endif // __LOCAL_ARRAY_H
