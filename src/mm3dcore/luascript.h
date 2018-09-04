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

#include "config.h"

#ifdef HAVE_LUALIB

#ifndef __LUASCRIPT_H
#define __LUASCRIPT_H

extern "C" 
{
#include <lauxlib.h>
#include <lua.h>
};

#include <string>

class LuaScript
{
   public:
      LuaScript();
      virtual ~LuaScript();

      int runFile( const char * filename );

      const char * error() const { return m_errstr.c_str(); };
      void registerFunction( const char * name, lua_CFunction func );
      void registerClosure( void * ptr, const char * name, lua_CFunction func );
      void registerHook( lua_Hook func, int mask, int count = 0 );

   protected:
      
      lua_State * m_luaState;
      std::string m_errstr;
};

#endif // HAVE_LUALIB
#endif // __LUASCRIPT_H
