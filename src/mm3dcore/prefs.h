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


#ifndef __PREFERENCES_H
#define __PREFERENCES_H

#include <stdio.h>
#include <string>
#include <map>
#include <vector>

#include "datadest.h"

class PrefItem;

typedef std::map< std::string, PrefItem > PrefItemHash;
typedef std::vector< PrefItem > PrefItemList;

class PrefItem
{
   public:
      PrefItem();
      virtual ~PrefItem();

      enum  PI_Type
      {
         PI_String,
         PI_Int,
         PI_Double,
         PI_List,
         PI_Hash
      };
      
      bool     isList();
      bool     isHash();
      unsigned count();
      unsigned keys();
      void     clear();
      void     insert( unsigned index, const PrefItem & item );
      void     remove( unsigned index );
      void     remove( const std::string & key );
      bool     exists( const std::string & key );
      void     write( int indent, DataDest & dst );

      int    operator=(int rhs);
      double operator=(double rhs);
      const std::string & operator=(const std::string& rhs);

      int     intValue();
      double  doubleValue();
      std::string stringValue();

      operator int();
      operator double();
      operator std::string();

      PrefItem & operator[](unsigned);
      PrefItem & operator()(const std::string &);

   protected:
      void convertAll(void);

      bool         m_allTypes;
      PI_Type      m_type;
      std::string      m_string;
      int          m_int;
      double       m_double;
      PrefItemList m_list;
      PrefItemHash m_hash;
};

class Preferences
{
   public:
      Preferences();
      virtual ~Preferences();

      PrefItem & operator[](unsigned);
      PrefItem & operator()(const std::string &);

      bool setDefault( const std::string & key, int value );
      bool setDefault( const std::string & key, double value );
      bool setDefault( const std::string & key, const std::string & value );

      void setRootItem( PrefItem * i );
      void write( DataDest & dst );

      bool exists( const std::string & );

   protected:
      PrefItem * m_rootItem;
};

// Overloaded type-specific operators for convenience
std::string operator+( PrefItem &, std::string );
std::string operator+( std::string, PrefItem & );
int     operator+( PrefItem &, int );
int     operator+( int, PrefItem & );
double  operator+( PrefItem &, double );
double  operator+( double, PrefItem & );
int     operator*( PrefItem &, int );
int     operator*( int, PrefItem & );
double  operator*( PrefItem &, double );
double  operator*( double, PrefItem & );
int     operator/( PrefItem &, int );
int     operator/( int, PrefItem & );
double  operator/( PrefItem &, double );
double  operator/( double, PrefItem & );
int     operator-( PrefItem &, int );
int     operator-( int, PrefItem & );
double  operator-( PrefItem &, double );
double  operator-( double, PrefItem & );

#endif /* __PREFERENCES_H */

