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

#ifndef __BSPTREE_H
#define __BSPTREE_H

#include "drawcontext.h"

#include <list>

class BspTree
{
   public:
      class Poly
      {
         public:
            static Poly * get();
            void release();

            static int flush();
            static void stats();

            int   id;

            float coord[3][3];
            float drawNormals[3][3];

            int   texture;
            void * material; // Yeah, yeah, I know... it's hackish
            void * triangle; // Yeah, yeah, I know... it's hackish

            float s[3];    // texture coordinates
            float t[3];

            float norm[3];
            float d;       // dot product

            void calculateNormal();
            void calculateD();
            void intersection( float * p1, float * p2, float * po, float & place );

            void render( DrawingContext * context );

            void print();

         protected:
            Poly() : id( ++s_nextId ) { s_allocated++; }; 
            virtual ~Poly() { s_allocated--; };

            static int s_nextId;
            static std::list< Poly * > s_recycle;

            static int s_allocated;

      };

      class Node
      {
         public:
            static Node * get();
            void release();

            static int flush();
            static void stats();

            void addChild( Node * n );
            void render( float * point, DrawingContext * context );

            void splitNodes( int idx1, int idx2, int idx3, 
                  float * p1, float * p2, Node * n1, Node * n2,
                  const float & place1, const float & place2 );

            void splitNode( int idx1, int idx2, int idx3, 
                  float * p1, Node * n1,
                  const float & place );

            Poly * self;

            Node * left;
            Node * right;

         protected:
            Node() : self( NULL ), left( NULL ), right( NULL ) { s_allocated++;};
            virtual ~Node() { s_allocated--; };

            static std::list< Node * > s_recycle;

            static int s_allocated;

      };

      BspTree() : m_root( NULL ) {};
      virtual ~BspTree() { clear(); };

      void addPoly( Poly * p );
      void render( float * point, DrawingContext * context );

      void clear();

   protected:
      Node * m_root;
};

#endif // __BSPTREE_H

