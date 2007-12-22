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


#ifndef __UTIL_H
#define __UTIL_H

// If less than minValue, set to minValue
// If greater than maxValue, set to maxValue
template<typename T>
void util_clamp( T & val, const T & minValue, const T & maxValue )
{
   if ( val < minValue )
      val = minValue;
   if ( val > maxValue )
      val = maxValue;
}

// If less than minValue, set to minValue
// If greater than maxValue, set to minValue
template<typename T>
void util_wrap_up( T & val, const T & minValue, const T & maxValue )
{
   if ( val < minValue )
      val = minValue;
   if ( val > maxValue )
      val = minValue;
}

// If less than minValue, set to maxValue
// If greater than maxValue, set to maxValue
template<typename T>
void util_wrap_down( T & val, const T & minValue, const T & maxValue )
{
   if ( val < minValue )
      val = maxValue;
   if ( val > maxValue )
      val = maxValue;
}

#endif // __UTIL_H
