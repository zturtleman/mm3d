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


#ifndef __SORTED_LIST_H
#define __SORTED_LIST_H

#include <vector>

template <typename T> class sorted_list : public std::vector<T>
{
   public:
      typedef int (*CompareFunction)( const T &, const T & );

      sorted_list();
      virtual ~sorted_list();

      void insert_sorted( const T & val );
      bool find_sorted( const T & val, unsigned & index ) const;

   protected:
};

template <typename T> sorted_list<T>::sorted_list()
{
}

template <typename T> sorted_list<T>::~sorted_list()
{
}

template <typename T> void sorted_list<T>::insert_sorted( const T & val )
{
   typename std::vector< T >::iterator it;

   unsigned len = this->size();
   if ( len == 0 || (*this)[len-1] < val )
   {
      this->push_back( val );
   }
   else
   {
      for ( it = this->begin(); it != this->end(); it ++)
      {
         if ( val < *it )
         {
            break;
         }
      }
      this->insert( it, val );
   }
}

template <typename T> bool sorted_list<T>::find_sorted( const T & val, unsigned & index ) const
{
   int top = this->size() - 1;
   int bot = 0;
   int mid = top / 2;

   while ( bot <= top )
   {
      if ( (*this)[mid] == val )
      {
         index = mid;
         return true;
      }

      if ( (*this)[mid] < val )
      {
         bot = mid + 1;
         mid = (top + bot) / 2;
      }
      else
      {
         top = mid - 1;
         mid = (top + bot) / 2;
      }
   }

   return false;
}

template <typename T> class sorted_ptr_list : public std::vector<T>
{
   public:
      typedef int (*CompareFunction)( const T &, const T & );

      sorted_ptr_list();
      virtual ~sorted_ptr_list();

      void insert_sorted( const T & val );
      bool find_sorted( const T & val, unsigned & index ) const;

   protected:
};

template <typename T> sorted_ptr_list<T>::sorted_ptr_list()
{
}

template <typename T> sorted_ptr_list<T>::~sorted_ptr_list()
{
}

template <typename T> void sorted_ptr_list<T>::insert_sorted( const T & val )
{
   typename std::vector< T >::iterator it;

   unsigned len = this->size();
   if ( len == 0 || *((*this)[len-1]) < *val )
   {
      this->push_back( val );
   }
   else
   {
      for ( it = this->begin(); it != this->end(); it ++)
      {
         if ( *val < *(*it) )
         {
            break;
         }
      }
      this->insert( it, val );
   }
}

template <typename T> bool sorted_ptr_list<T>::find_sorted( const T & val, unsigned & index ) const
{
   int top = this->size() - 1;
   int bot = 0;
   int mid = top / 2;

   while ( bot <= top )
   {
      if ( *(*this)[mid] == *val )
      {
         index = mid;
         return true;
      }

      if ( *(*this)[mid] < *val )
      {
         bot = mid + 1;
         mid = (top + bot) / 2;
      }
      else
      {
         top = mid - 1;
         mid = (top + bot) / 2;
      }
   }

   return false;
}

#endif // __SORTED_LIST_H
