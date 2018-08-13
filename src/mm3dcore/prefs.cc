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


#include "prefs.h"

#include "mm3dconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using std::map;
using std::vector;

static void _print_escaped( DataDest & dst, const std::string & str )
{
   dst.writeString( "\"" );
   size_t len = str.size();
   for ( size_t i = 0; i < len; i++ )
   {
      if ( str[i] == '"' || str[i] == '\\' )
      {
         dst.writeString( "\\" );
      }
      dst.writePrintf( "%c", str[i] );
   }
   dst.writeString( "\"" );
}

PrefItem::PrefItem()
{
   m_type     = PI_String;
   m_int      = 0;
   m_double   = 0;
   m_string   = "";
   m_allTypes = true;
}

PrefItem::~PrefItem()
{
   m_list.clear();
   m_hash.clear();
}

void PrefItem::convertAll(void)
{
   // Convert to string, then set all others from the string
   switch( m_type )
   {
      case PI_Hash:
         return;
         break;
      case PI_List:
         return;
         break;
      case PI_String:
         break;
      case PI_Int:
         {
            char tmp[32];
            sprintf( tmp, "%d", m_int );
            m_string = tmp;
         }
         break;
      case PI_Double:
         {
            char tmp[32];
            sprintf( tmp, "%f", m_double );
            m_string = tmp;
         }
         break;
      default:
         fprintf(stderr, "Unhandled PrefItem type\n");
         return;
         break;
   }

   m_int    = atoi( m_string.c_str() );
   m_double = atof( m_string.c_str() );

   m_allTypes = true;
   return;
}

bool PrefItem::isList(void)
{
   return ( m_type == PI_List ? true : false );
}

bool PrefItem::isHash(void)
{
   return ( m_type == PI_Hash ? true : false );
}

unsigned PrefItem::count(void)
{
   return ( m_list.size() );
}

unsigned PrefItem::keys(void)
{
   return ( m_hash.size() );
}

void PrefItem::clear()
{
   if ( m_type == PI_List )
      m_list.clear();
   if ( m_type == PI_Hash )
      m_hash.clear();
}

void PrefItem::insert( unsigned index, const PrefItem & item )
{
   if ( index < m_list.size() )
   {
      unsigned c = 0;
      PrefItemList::iterator it = m_list.begin();

      while ( c < index )
      {
         it++;
         c++;
      }

      m_list.insert( it, 1, item );
   }
   else
   {
      (*this)[index] = item;
   }
}

void PrefItem::remove( unsigned index )
{
   if ( isList() && index < m_list.size() )
   {
      unsigned c = 0;
      PrefItemList::iterator it = m_list.begin();

      while ( c < index )
      {
         it++;
         c++;
      }

      m_list.erase( it );
   }
}

void PrefItem::remove( const std::string & key )
{
   if ( isHash() )
   {
      m_hash.erase( key );
   }
}

bool PrefItem::exists( const std::string & key )
{
   if ( isHash() )
   {
      return m_hash.find( key ) != m_hash.end();
   }
   else
   {
      return false;
   }
}

void PrefItem::write( int indent, DataDest & dst )
{
   if ( m_type == PI_Hash )
   {
      dst.writeString( "{" FILE_NEWLINE );
      indent += 3;
      PrefItemHash::iterator it;
      for ( it = m_hash.begin(); it != m_hash.end(); it++ )
      {
         for ( int t = 0; t < indent; t++ )
         {
            dst.writeString( " " );
         }
         dst.writePrintf( "\"%s\" => ", (*it).first.c_str() );
         (*it).second.write( indent + 3, dst );
         if ( ++it != m_hash.end() )
         {
            dst.writeString( "," FILE_NEWLINE );
         }
         else
         {
            dst.writeString( FILE_NEWLINE );
         }
         it--;
      }
      indent -= 3;
      for ( int i = 0; i < indent; i++ )
      {
         dst.writeString( " " );
      }
      dst.writeString( "}" );
   }
   else if ( m_type == PI_List )
   {
      dst.writeString( "(" FILE_NEWLINE );
      indent += 3;
      for ( unsigned t = 0; t < m_list.size(); t++ )
      {
         for ( int i = 0; i < indent; i++ )
         {
            dst.writeString( " " );
         }
         m_list[t].write( indent +3, dst );
         if ( t+1 != m_list.size() )
         {
            dst.writeString( "," FILE_NEWLINE );
         }
         else
         {
            dst.writeString( FILE_NEWLINE );
         }
      }
      indent -= 3;
      for ( int i = 0; i < indent; i++ )
      {
         dst.writeString( " " );
      }
      dst.writeString( ")" );
   }
   else
   {
      convertAll();
      if ( m_string.find('"') < m_string.size()
            || m_string.find('/') < m_string.size() )
      {
         _print_escaped( dst, m_string );
      }
      else
      {
         dst.writePrintf( "\"%s\"",  m_string.c_str() );
      }
   }
}

int PrefItem::operator=( int rhs )
{
   m_int = rhs;
   m_type = PI_Int;
   m_allTypes = false;

   return m_int;
}

double PrefItem::operator=( double rhs )
{
   m_double = rhs;
   m_type = PI_Double;
   m_allTypes = false;

   return m_double;
}

const std::string & PrefItem::operator=( const std::string & rhs )
{
   m_string = rhs;
   m_type = PI_String;
   m_allTypes = false;

   return m_string;
}

int PrefItem::intValue()
{
   if ( m_allTypes == false && m_type != PI_Int )
   {
      convertAll();
   }

   return m_int;
}

double PrefItem::doubleValue()
{
   if ( m_allTypes == false && m_type != PI_Double )
   {
      convertAll();
   }

   return m_double;
}

std::string PrefItem::stringValue()
{
   if ( m_allTypes == false && m_type != PI_String )
   {
      convertAll();
   }

   return m_string;
}

PrefItem::operator int()
{
   return intValue();
}

PrefItem::operator double()
{
   return doubleValue();
}

PrefItem::operator std::string()
{
   return stringValue();
}

PrefItem & PrefItem::operator[](unsigned index)
{
   m_type = PI_List;
   while ( m_list.size() <= index )
   {
      PrefItem i;
      i = "";
      m_list.push_back( i );
   }
   return( m_list[ index ] );
}

PrefItem & PrefItem::operator()(const std::string & key)
{
   m_type = PI_Hash;
   return( m_hash[ key ] );
}

Preferences::Preferences()
   : m_rootItem( new PrefItem() )
{
}

Preferences::~Preferences()
{
   delete m_rootItem;
}

void Preferences::setRootItem( PrefItem * i )
{
   delete m_rootItem;
   m_rootItem = i;
}

bool Preferences::setDefault( const std::string & key, int value )
{
   if ( !m_rootItem->exists( key ) )
   {
      (*m_rootItem)( key ) = value;
      return true;
   }
   return false;
}

bool Preferences::setDefault( const std::string & key, double value )
{
   if ( !m_rootItem->exists( key ) )
   {
      (*m_rootItem)( key ) = value;
      return true;
   }
   return false;
}

bool Preferences::setDefault( const std::string & key, const std::string & value )
{
   if ( !m_rootItem->exists( key ) )
   {
      (*m_rootItem)( key ) = value;
      return true;
   }
   return false;
}

PrefItem & Preferences::operator[](unsigned index)
{
   return (*m_rootItem)[ index ];
}

PrefItem & Preferences::operator()(const std::string & key)
{
   return (*m_rootItem)( key );
}

void Preferences::write( DataDest & dst )
{
   m_rootItem->write( 0, dst );
   dst.writeString( FILE_NEWLINE );
}

bool Preferences::exists( const std::string & key )
{
   return m_rootItem->exists( key );
}

std::string operator+( std::string str, PrefItem & item )
{
   return str + item.stringValue();
}

std::string operator+( PrefItem & item, std::string str )
{
   return item.stringValue() + str;
}

int operator+( PrefItem & item, int i )
{
   return item.intValue() + i;
}

int operator+( int i, PrefItem & item )
{
   return i + item.intValue();
}

double operator+( double d, PrefItem & item )
{
   return d + item.doubleValue();
}

double operator+( PrefItem & item, double d )
{
   return item.doubleValue() + d;
}

int operator*( PrefItem & item, int i )
{
   return item.intValue() * i;
}

int operator*( int i, PrefItem & item )
{
   return i * item.intValue();
}

double operator*( double d, PrefItem & item )
{
   return d * item.doubleValue();
}

double operator*( PrefItem & item, double d )
{
   return item.doubleValue() * d;
}

int operator/( PrefItem & item, int i )
{
   return item.intValue() / i;
}

int operator/( int i, PrefItem & item )
{
   return i / item.intValue();
}

double operator/( double d, PrefItem & item )
{
   return d / item.doubleValue();
}

double operator/( PrefItem & item, double d )
{
   return item.doubleValue() / d;
}

int operator-( PrefItem & item, int i )
{
   return item.intValue() - i;
}

int operator-( int i, PrefItem & item )
{
   return i - item.intValue();
}

double operator-( double d, PrefItem & item )
{
   return d - item.doubleValue();
}

double operator-( PrefItem & item, double d )
{
   return item.doubleValue() - d;
}

/*
int main( int argc, char *argv[] )
{
   Preferences prefs;

   prefs("foo") = "bar";
   prefs("ksw")[0] = 7;
   prefs("ksw")[2] = 42;
   prefs("ksw")[1] = 21;
   PrefItem item = prefs("ksw")[1];
   prefs("ksw").remove(1);
   prefs("ksw").insert( 0, item );

   printf("prefs(foo) = %s\n", ((std::string )prefs("foo")).c_str());
   for ( int t = 0; t < 3; t++ )
   {
      printf("prefs(ksw)[%d] = %d\n", t, (int) prefs("ksw")[t]);
   }

   printf( "%d\n", prefs("ksw")[1] * 3 );
}
*/
