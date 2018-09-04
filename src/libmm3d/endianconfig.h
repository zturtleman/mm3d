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


#ifndef __ENDIANCONFIG_H
#define __ENDIANCONFIG_H

// Can't use qconfig.h here because libmm3d cannot depend on Qt
#include "config.h"

#if BYTEORDER == 4321 // endian test (big endian)

inline uint16_t ltoh_u16( uint16_t littleval ) {
  union
  {
    uint16_t s;
    uint8_t b[2];
  } u1, u2;

  u1.s = littleval;
  u2.b[0] = u1.b[1];
  u2.b[1] = u1.b[0];
  return u2.s;
}

inline uint32_t ltoh_u32( uint32_t littleval ) {
  union
  {
    uint32_t l;
    uint8_t b[4];
  } u1, u2;

  u1.l = littleval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.l;
}

inline int16_t ltoh_16( int16_t littleval ) {
  union
  {
    int16_t s;
    uint8_t b[2];
  } u1, u2;

  u1.s = littleval;
  u2.b[0] = u1.b[1];
  u2.b[1] = u1.b[0];
  return u2.s;
}

inline int32_t ltoh_32( int32_t littleval ) {
  union
  {
    int32_t l;
    uint8_t b[4];
  } u1, u2;

  u1.l = littleval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.l;
}

inline float ltoh_float( float littleval ) {
  union
  {
    float f;
    uint8_t b[4];
  } u1, u2;

  u1.f = littleval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.f;
}

inline uint16_t htol_u16( uint16_t hostval ) {
  union
  {
    uint16_t s;
    uint8_t b[2];
  } u1, u2;

  u1.s = hostval;
  u2.b[0] = u1.b[1];
  u2.b[1] = u1.b[0];
  return u2.s;
}

inline uint32_t htol_u32( uint32_t hostval ) {
  union
  {
    uint32_t l;
    uint8_t b[4];
  } u1, u2;

  u1.l = hostval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.l;
}

inline int16_t htol_16( int16_t hostval ) {
  union
  {
    int16_t s;
    uint8_t b[2];
  } u1, u2;

  u1.s = hostval;
  u2.b[0] = u1.b[1];
  u2.b[1] = u1.b[0];
  return u2.s;
}
inline int32_t htol_32( int32_t hostval ) {
  union
  {
    int32_t l;
    uint8_t b[4];
  } u1, u2;

  u1.l = hostval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.l;
}

inline float htol_float( float hostval ) {
  union
  {
    float f;
    uint8_t b[4];
  } u1, u2;

  u1.f = hostval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.f;
}

inline uint16_t btoh_u16( uint16_t bigval ) {
   return bigval;
}

inline uint32_t btoh_u32( uint32_t bigval ) {
   return bigval;
}

inline int16_t btoh_16( int16_t bigval ) {
   return bigval;
}

inline int32_t btoh_32( int32_t bigval ) {
   return bigval;
}

inline float btoh_float( float bigval ) {
   return bigval;
}

inline uint16_t htob_u16( uint16_t hostval ) {
   return hostval;
}

inline uint32_t htob_u32( uint32_t hostval ) {
   return hostval;
}

inline int16_t htob_16( int16_t hostval ) {
   return hostval;
}

inline int32_t htob_32( int32_t hostval ) {
   return hostval;
}

inline float htob_float( float hostval ) {
   return hostval;
}

#else  // endian test (little endian)

inline uint16_t ltoh_u16( uint16_t littleval ) {
   return littleval;
}
inline uint32_t ltoh_u32( uint32_t littleval ) {
   return littleval;
}

inline int16_t ltoh_16( int16_t littleval ) {
   return littleval;
}
inline int32_t ltoh_32( int32_t littleval ) {
   return littleval;
}

inline float ltoh_float( float littleval ) {
   return littleval;
}

inline uint16_t htol_u16( uint16_t hostval ) {
   return hostval;
}
inline uint32_t htol_u32( uint32_t hostval ) {
   return hostval;
}

inline int16_t htol_16( int16_t hostval ) {
   return hostval;
}
inline int32_t htol_32( int32_t hostval ) {
   return hostval;
}

inline float htol_float( float hostval ) {
   return hostval;
}

inline uint16_t btoh_u16( uint16_t bigval ) {
  union
  {
    uint16_t s;
    uint8_t b[2];
  } u1, u2;

  u1.s = bigval;
  u2.b[0] = u1.b[1];
  u2.b[1] = u1.b[0];
  return u2.s;
}
inline uint32_t btoh_u32( uint32_t bigval ) {
  union
  {
    uint32_t l;
    uint8_t b[4];
  } u1, u2;

  u1.l = bigval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.l;
}

inline int16_t btoh_16( int16_t bigval ) {
  union
  {
    int16_t s;
    uint8_t b[2];
  } u1, u2;

  u1.s = bigval;
  u2.b[0] = u1.b[1];
  u2.b[1] = u1.b[0];
  return u2.s;
}
inline int32_t btoh_32( int32_t bigval ) {
  union
  {
    int32_t l;
    uint8_t b[4];
  } u1, u2;

  u1.l = bigval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.l;
}

inline float btoh_float( float bigval ) {
  union
  {
    float f;
    uint8_t b[4];
  } u1, u2;

  u1.f = bigval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.f;
}

inline uint16_t htob_u16( uint16_t hostval ) {
  union
  {
    uint16_t s;
    uint8_t b[2];
  } u1, u2;

  u1.s = hostval;
  u2.b[0] = u1.b[1];
  u2.b[1] = u1.b[0];
  return u2.s;
}
inline uint32_t htob_u32( uint32_t hostval ) {
  union
  {
    uint32_t l;
    uint8_t b[4];
  } u1, u2;

  u1.l = hostval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.l;
}

inline int16_t htob_16( int16_t hostval ) {
  union
  {
    int16_t s;
    uint8_t b[2];
  } u1, u2;

  u1.s = hostval;
  u2.b[0] = u1.b[1];
  u2.b[1] = u1.b[0];
  return u2.s;
}
inline int32_t htob_32( int32_t hostval ) {
  union
  {
    int32_t l;
    uint8_t b[4];
  } u1, u2;

  u1.l = hostval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.l;
}

inline float htob_float( float hostval ) {
  union
  {
    float f;
    uint8_t b[4];
  } u1, u2;

  u1.f = hostval;
  u2.b[0] = u1.b[3];
  u2.b[1] = u1.b[2];
  u2.b[2] = u1.b[1];
  u2.b[3] = u1.b[0];
  return u2.f;
}

#endif // endian test

#endif // __ENDIANCONFIG_H

