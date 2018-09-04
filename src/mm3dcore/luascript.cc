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

#include "luascript.h"

#ifdef HAVE_LUALIB

#include "log.h"
#include "msg.h"
#include "mm3dport.h"
#include "filedatasource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern "C" {
#include <lualib.h>
};

typedef struct _ReadChunkData_t
{
   const char * filename;
   FileDataSource * src;
} ReadChunkDataT;

static const char * _luascript_readchunk( lua_State * L, void * data, size_t * size )
{
   ReadChunkDataT * rcd = (ReadChunkDataT *) data;
   static const int MAX_DATA = 1024;
   int readBytes = 0;

   const char * filename = rcd->filename;
   data = malloc( MAX_DATA );
   
   // Open file if it isn't already open
   if ( rcd->src == NULL )
   {
      rcd->src = new FileDataSource( filename );
      if ( rcd->src->getErrno() != 0 )
      {
         char msg[1024];
         PORT_snprintf( msg, sizeof(msg), "%s: %s", filename, strerror( rcd->src->getErrno() ) );
         msg_error( msg );
         return NULL;
      }
   }

   readBytes = rcd->src->getRemaining();
   if ( readBytes > MAX_DATA )
   {
      readBytes = MAX_DATA;
   }

   if ( rcd->src->readBytes( (uint8_t*)data, readBytes ) == false )
   {
      // End of file, clean up

      delete rcd->src;
      rcd->src = NULL;

      free( data );
      data = NULL;
   }

   *size = readBytes;

   return (const char *) data;
}

LuaScript::LuaScript()
{
   m_luaState = lua_open();
   luaopen_math( m_luaState );
}

LuaScript::~LuaScript()
{
   lua_close( m_luaState );
}

int LuaScript::runFile( const char * filename )
{
   log_debug( "running script: %s\n", filename );

   int rval = -1;
   m_errstr = "No error";
   if ( filename )
   {
      ReadChunkDataT rcd;
      rcd.src = NULL;
      rcd.filename = filename;
      rval = lua_load( m_luaState, _luascript_readchunk, (void *) &rcd, filename );

      if ( rval == 0 )
      {
         rval = lua_pcall( m_luaState, 0, 0, 0 );

         if ( rval != 0 )
         {
            log_error( "lua_pcall failed: %d (stack is %d)\n", rval, lua_gettop( m_luaState ) );
            unsigned top = lua_gettop( m_luaState );

            if ( top > 0 && lua_isstring( m_luaState, top ) )
            {
               m_errstr = lua_tostring( m_luaState, top );
            }
            else
            {
               m_errstr = "Unknown error";
            }
         }
      }
      else
      {
         log_error( "lua_load failed: %d (stack is %d) \n", rval, lua_gettop( m_luaState ) );
         unsigned top = lua_gettop( m_luaState );
         if ( top > 0 && lua_isstring( m_luaState, top ) )
         {
            m_errstr = lua_tostring( m_luaState, top );
         }
         else
         {
            m_errstr = "Parse error";
         }
      }

      // strip bracket from error string
      const char * bracket = strchr( m_errstr.c_str(), ']' );
      if ( bracket )
      {
         bracket++;
         m_errstr = bracket;
      }
   }

   return rval;
}

void LuaScript::registerFunction( const char * name, lua_CFunction func )
{
   lua_register( m_luaState, name, func );
}

void LuaScript::registerClosure( void * ptr, const char * name, lua_CFunction func )
{
   lua_pushstring( m_luaState, name );
   lua_pushlightuserdata( m_luaState, ptr );
   lua_pushcclosure( m_luaState, func, 1 );
   lua_settable( m_luaState, LUA_GLOBALSINDEX );
}

/*
void LuaScript::registerClosure( void * ptr1, void * ptr2, const char * name, lua_CFunction func )
{
   lua_pushstring( m_luaState, name );
   lua_pushlightuserdata( m_luaState, ptr1 );
   lua_pushlightuserdata( m_luaState, ptr2 );
   lua_pushcclosure( m_luaState, func, 2 );
   lua_settable( m_luaState, LUA_GLOBALSINDEX );
}
*/

void LuaScript::registerHook( lua_Hook func, int mask, int count )
{
   lua_sethook( m_luaState, func, mask, count );
}

#endif // HAVE_LUALIB
