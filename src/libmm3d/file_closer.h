/*  Misfit Model 3D
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


#ifndef __FILECLOSER_H
#define __FILECLOSER_H

#include <stdio.h>

class file_closer
{
   public:
      file_closer( FILE * fp = NULL )
         : m_fp( fp ) {}
      ~file_closer() { free_ptr(); }

      FILE * get() { return m_fp; }
      FILE * reset(FILE * fp) { free_ptr(); return m_fp = fp; }
      const FILE * get() const { return m_fp; }

      FILE & operator*() { return *m_fp; }
      FILE * operator->() { return m_fp; }
      const FILE & operator*() const { return *m_fp; }
      const FILE * operator->() const { return m_fp; }

      FILE * operator=(FILE* fp) { return reset(fp); }

      bool operator!() const { return m_fp == NULL; }
      bool isnull() const { return m_fp == NULL; }

   protected:
      void free_ptr() { if (m_fp) fclose(m_fp); m_fp = NULL; }

      FILE * m_fp;
};


#endif // __FILECLOSER_H

