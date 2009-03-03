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

#include <cstdio>

#include "texturetest.h"

#include "texmgr.h"

void texture_test_compare( const char * f1, const char * f2, unsigned fuzzyValue )
{
   TextureManager * texmgr = TextureManager::getInstance();

   Texture * t1 = texmgr->getTexture( f1 );
   if ( t1 == NULL )
   {
      printf( "could not open %s\n", f1 );
      return;
   }
   Texture * t2 = texmgr->getTexture( f2 );
   if ( t2 == NULL )
   {
      printf( "could not open %s\n", f2 );
      return;
   }

   Texture::CompareResultT res;
   bool match = false;

   float fuzzyImage = 0.90f;
   bool pixelPerfect  = false;
   float fuzzyMatch   = 0.0f;
   int steps = 256 * 3;
   for ( int t = 0; t <= steps; t++ )
   {
      float fuzzyValue = 1.0f - ((float) t / (float) steps);
      match = Texture::compare( t1, t2, &res, t );
      if ( res.comparable )
      {
         if ( res.pixelCount == res.matchCount )
         {
            pixelPerfect = true;
         }

         if ( ((float) res.fuzzyCount / (float) res.pixelCount ) >= fuzzyImage )
         {
            fuzzyMatch = fuzzyValue;
            break;
         }
      }
      else
      {
         break;
      }
   }

   printf( "%s and %s\n", t1->m_name, t2->m_name );
   printf( "  %c %.2f %.2f\n", 
         match ? 'Y' : 'N',
         (float) res.matchCount / (float) res.pixelCount, fuzzyMatch );
}

