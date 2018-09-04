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

#include "texscale.h"

#include "log.h"

#include <stdlib.h>

static unsigned MAX_SCALE_SIZE = 2048;

static bool _is_power_of_two( unsigned a )
{
   while ( a > 0 )
   {
      if ( (a & 1) )
      {
         if ( ((a & ~1) > 0) )
         {
            return false;
         }
         else
         {
            return true;
         }
      }
      a = a >> 1;
   }
   return true;
}

static int _get_best_scale_size( unsigned a )
{
   unsigned newa = a;
   int best = 2;
   int diff = abs( (int) best - (int) a );

   bool scale = _is_power_of_two( a ) ? false : true;

   if ( scale )
   {
      newa = 2;
      best = newa;
      while ( newa < MAX_SCALE_SIZE )
      {
         int newdiff = abs( (int) newa - (int) a );

         if( newdiff < diff )
         {
            best = newa;
            diff = newdiff;
         }

         newa = newa << 1;
      }
      newa = best;
   }

   return newa;
}

bool texture_scale_need_scale( unsigned x, unsigned y )
{
   if ( ! _is_power_of_two( x ) || ! _is_power_of_two( y ) )
   {
      return true;
   }
   else
   {
      return false;
   }
}

uint8_t * texture_scale_auto( uint8_t * data, Texture::FormatE format, int & oldx, int & oldy )
{
   int newx = _get_best_scale_size( oldx );
   int newy = _get_best_scale_size( oldy );

   uint8_t * d = texture_scale_size( data, format, oldx, oldy, newx, newy );

   oldx = newx;
   oldy = newy;

   return d;
}

uint8_t * texture_scale_size( uint8_t * data, Texture::FormatE format, int oldx, int oldy, int newx, int newy )
{
   log_debug( "scaling %s image from %d,%d to %d,%d\n", 
         ((format == Texture::FORMAT_RGB) ? "RGB" : "RGBA"), 
         oldx, oldy, newx, newy );

   unsigned bpp = (format == Texture::FORMAT_RGB) ? 3 : 4;
   unsigned imageSize = newx * newy * bpp;

   uint8_t * dest = new uint8_t[imageSize];

   unsigned samplex   = 0;
   unsigned sampley   = 0;
   unsigned sampleoff = 0;
   unsigned destoff   = 0;
   unsigned b = 0;

   for ( int y = 0; y < newy; y++ )
   {
      for ( int x = 0; x < newx; x++ )
      {
         samplex   = (unsigned) (((double) x / (double) newx) * (double) oldx + 0.5);
         sampley   = (unsigned) (((double) y / (double) newy) * (double) oldy + 0.5);
         sampleoff = (samplex + (sampley * oldx)) * bpp;
         destoff   = (x + (y * newx)) * bpp;

         for ( b = 0; b < bpp; b++ )
         {
            dest[ destoff + b ] = data[ sampleoff + b ];
         }
      }
   }
   log_debug( "max sample = %d,%d\n", samplex, sampley );

   return dest;
}

