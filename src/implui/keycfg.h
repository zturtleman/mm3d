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

#ifndef __KEYCFG_H
#define __KEYCFG_H

#include <list>
#include <string>

#include <QtGui/QKeySequence>

class KeyConfig
{
   public:
      KeyConfig();
      virtual ~KeyConfig();

      QKeySequence getKey( const char * operation );
      void setKey( const char * operation, const QKeySequence & key );
      void setDefaultKey( const char * operation, const QKeySequence & key );

      bool saveFile( const char * filename );
      bool loadFile( const char * filename );

      //static int getSpecialKey( const char * keyName );
      //static std::string getSpecialKeyName( int key );

      struct _KeyData_t
      {
         std::string operation;
         QKeySequence key;
      };
      typedef struct _KeyData_t KeyDataT;
      typedef std::list< KeyDataT > KeyDataList;

   protected:

      KeyDataList m_list;
};

extern KeyConfig g_keyConfig;

bool keycfg_load_file( const char * filename );
bool keycfg_save_file( const char * filename );
void keycfg_set_defaults();

#endif // __KEYCFG_H
