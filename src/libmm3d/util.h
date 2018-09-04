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


#ifndef __UTIL_H
#define __UTIL_H

// If less than minValue, set to minValue
// If greater than maxValue, set to maxValue
template<typename T>
T util_clamp( const T & val, const T & minValue, const T & maxValue )
{
   if ( val < minValue )
      return minValue;
   if ( val > maxValue )
      return maxValue;
   return val;
}

// If less than minValue, set to minValue
// If greater than maxValue, set to minValue
template<typename T>
T util_wrap_up( const T & val, const T & minValue, const T & maxValue )
{
   if ( val < minValue )
      return minValue;
   if ( val > maxValue )
      return minValue;
   return val;
}

// If less than minValue, set to maxValue
// If greater than maxValue, set to maxValue
template<typename T>
T util_wrap_down( const T & val, const T & minValue, const T & maxValue )
{
   if ( val < minValue )
      return maxValue;
   if ( val > maxValue )
      return maxValue;
   return val;
}

#endif // __UTIL_H
