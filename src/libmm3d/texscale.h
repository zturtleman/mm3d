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

#ifndef __TEXSCALE_H
#define __TEXSCALE_H

#include <stdint.h>

#include "texture.h"

// Call this to determine if the texture must be scaled
bool texture_scale_need_scale( unsigned x, unsigned y );

// These take a data ptr which is the byte array for the RGBA image data.
// They return a new data ptr (new uint8_t[n]).  The caller is responsible
// for freeing both pointers.
uint8_t * texture_scale_auto( uint8_t * data, Texture::FormatE format, int & oldx, int & oldy );
uint8_t * texture_scale_size( uint8_t * data, Texture::FormatE format, int oldx, int oldy, int newx, int newy );

#endif // __TEXSCALE_H
