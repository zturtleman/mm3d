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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glheaders.h"
#include "bsptree.h"
#include "glmath.h"
#include "log.h"
#include "model.h" // Yes, it's hackish

std::list< BspTree::Poly * > BspTree::Poly::s_recycle;
std::list< BspTree::Node * > BspTree::Node::s_recycle;

int BspTree::Poly::s_allocated = 0;
int BspTree::Node::s_allocated = 0;

void normalize( float * val )
{
   float mag = 0.0f;
   int t;
   for ( t = 0; t < 3; t++ )
   {
      mag += val[t] * val[t];
   }

   mag = sqrt( mag );

   for ( t = 0; t < 3; t++ )
   {
      val[t] = val[t] / mag;
   }
}

float dot_product( float * val1, float * val2 )
{
   return ( val1[0] * val2[0] ) 
      +  ( val1[1] * val2[1] )  
      +  ( val1[2] * val2[2] );
}

bool float_equiv( float rhs, float lhs )
{
   if ( fabs( rhs - lhs ) < 0.0001f )
   {
      return true;
   }
   else
   {
      return false;
   }
}

static void _setMaterial( DrawingContext * context, int texture, Model::Material * material )
{
   glMaterialfv( GL_FRONT, GL_AMBIENT,
         material->m_ambient );
   glMaterialfv( GL_FRONT, GL_DIFFUSE,
         material->m_diffuse );
   glMaterialfv( GL_FRONT, GL_SPECULAR,
         material->m_specular );
   glMaterialfv( GL_FRONT, GL_EMISSION,
         material->m_emissive );
   glMaterialf( GL_FRONT, GL_SHININESS,
         material->m_shininess );

   context->m_currentTexture = texture;

   if ( material->m_type == Model::Material::MATTYPE_TEXTURE )
   {
      glBindTexture( GL_TEXTURE_2D,
            context->m_matTextures[ context->m_currentTexture ] );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
            material->m_sClamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 
            material->m_tClamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
   }
}

int BspTree::Poly::s_nextId = 0;

void BspTree::Poly::calculateNormal()
{
   float A = coord[0][1] * (coord[1][2] - coord[2][2]) + coord[1][1] * (coord[2][2] - coord[0][2]) + coord[2][1] * (coord[0][2] - coord[1][2]);
   float B = coord[0][2] * (coord[1][0] - coord[2][0]) + coord[1][2] * (coord[2][0] - coord[0][0]) + coord[2][2] * (coord[0][0] - coord[1][0]);
   float C = coord[0][0] * (coord[1][1] - coord[2][1]) + coord[1][0] * (coord[2][1] - coord[0][1]) + coord[2][0] * (coord[0][1] - coord[1][1]);

   float len = sqrt((A * A) + (B * B) + (C * C));

   norm[0] = A / len;
   norm[1] = B / len;
   norm[2] = C / len;

   calculateD();

   //printf( "normal: %f %f %f\n", norm[0], norm[1], norm[2] );
   //printf( "d:      %f\n", d );
}

void BspTree::Poly::calculateD()
{
   d = dot_product( coord[0], norm );
}

void BspTree::Poly::intersection( float * p1, float * p2, float * po, float & place )
{
   float interval = 0.25;
   float dtemp    = 0.0f;

   place    = 0.5f;

   bool greater = false;

   if ( dot_product( p2, norm ) > d )
   {
      greater = true;
   }

   if ( dot_product( p1, norm ) > d )
   {
      if ( greater )
      {
         //printf( "no intersection!\n" );
         return;
      }
   }
   else
   {
      if ( !greater )
      {
         //printf( "no intersection!\n" );
         return;
      }
   }

   float diff[3];
   float oldPlace = place;

   diff[0] = p2[0] - p1[0];
   diff[1] = p2[1] - p1[1];
   diff[2] = p2[2] - p1[2];

   do 
   {
      po[0] = diff[0] * place + p1[0];
      po[1] = diff[1] * place + p1[1];
      po[2] = diff[2] * place + p1[2];

      dtemp = dot_product( po, norm );

      oldPlace = place;

      if ( greater )
      {
         place += ( dtemp > d ) ? -interval : interval;
      }
      else
      {
         place += ( dtemp > d ) ? interval : -interval;
      }

      interval = interval * 0.5f;
   } while ( !float_equiv( d, dtemp ) );

   place = oldPlace;
}

void BspTree::Poly::render( DrawingContext * context )
{
   //printf( "render triangle %d\n", id );

   if ( context->m_currentTexture != texture )
   {
      glEnd();

      _setMaterial( context, texture, static_cast< Model::Material *>( material ) );

      glBegin( GL_TRIANGLES );
   }

   Model::Triangle * tri = static_cast< Model::Triangle *>( triangle );
   if ( tri->m_visible )
   {
      for ( int i = 0; i < 3; i++ )
      {
         glTexCoord2f( s[ i ], t[ i ] );
         glNormal3fv( drawNormals[i] );
         glVertex3fv( coord[i] );
      }
   }
}

void BspTree::Poly::print()
{
   //printf( "Poly %d\n", id );
   //printf( "  coord[0] = %f %f %f\n", coord[0][0], coord[0][1], coord[0][2] );
   //printf( "  coord[1] = %f %f %f\n", coord[1][0], coord[1][1], coord[1][2] );
   //printf( "  coord[2] = %f %f %f\n", coord[2][0], coord[2][1], coord[2][2] );
   //printf( "  n  = %f %f %f\n", norm[0], norm[1], norm[2] );
}

void BspTree::addPoly( BspTree::Poly * p )
{
   Node * n = Node::get();
   n->self = p;

   if ( m_root == NULL )
   {
      m_root = n;
   }
   else
   {
      m_root->addChild( n );
   }
}

void BspTree::render( float * point, DrawingContext * context )
{
   if ( m_root )
   {
      _setMaterial( context, m_root->self->texture, static_cast< Model::Material *>( m_root->self->material ) );
      glEnable( GL_TEXTURE_2D );

      glBegin( GL_TRIANGLES );
      m_root->render( point, context );
      glEnd();
   }
}

void BspTree::clear()
{
   if ( m_root )
   {
      m_root->release();
      m_root = NULL;
   }
}

BspTree::Poly * BspTree::Poly::get()
{
   if ( !s_recycle.empty() )
   {
      Poly * n = s_recycle.front();
      s_recycle.pop_front();
      return n;
   }
   else
   {
      return new Poly;
   }
}

void BspTree::Poly::release()
{
   s_recycle.push_front( this );
}

BspTree::Node * BspTree::Node::get()
{
   if ( !s_recycle.empty() )
   {
      Node * n = s_recycle.front();
      s_recycle.pop_front();
      n->self = NULL;
      n->left = NULL;
      n->right = NULL;
      return n;
   }
   else
   {
      return new Node;
   }
}

void BspTree::Node::release()
{
   if ( left )
   {
      left->release();
   }
   if ( self )
   {
      self->release();
   }
   if ( right )
   {
      right->release();
   }
   s_recycle.push_front( this );
}

void BspTree::Node::splitNodes( int idx1, int idx2, int idx3, 
      float * p1, float * p2, 
      BspTree::Node * n1, BspTree::Node * n2,
      const float & place1, 
      const float & place2 )
{
   n1->self = Poly::get();
   //printf( "split node poly: %d\n", n1->self->id );
   for ( int i = 0; i < 3; i++ )
   {
      n1->self->coord[0][i] = p1[i];
      n1->self->coord[1][i] = self->coord[idx2][i];
      n1->self->coord[2][i] = self->coord[idx3][i];

      n1->self->drawNormals[0][i] = self->norm[i];
      n1->self->drawNormals[1][i] = self->drawNormals[idx2][i];
      n1->self->drawNormals[2][i] = self->drawNormals[idx3][i];
   }
   n1->self->s[0] = (self->s[ idx2 ] - self->s[ idx1 ]) * place1 + self->s[ idx1 ];
   n1->self->s[1] = self->s[ idx2 ];
   n1->self->s[2] = self->s[ idx3 ];

   n1->self->t[0] = (self->t[ idx2 ] - self->t[ idx1 ]) * place1 + self->t[ idx1 ];
   n1->self->t[1] = self->t[ idx2 ];
   n1->self->t[2] = self->t[ idx3 ];

   n1->self->calculateNormal();
   n1->self->texture = self->texture;
   n1->self->triangle = self->triangle;
   n1->self->material = self->material;

   n2->self = Poly::get();
   //printf( "split node poly: %d\n", n2->self->id );
   for ( int i = 0; i < 3; i++ )
   {
      n2->self->coord[0][i] = p1[i];
      n2->self->coord[1][i] = self->coord[idx3][i];
      n2->self->coord[2][i] = p2[i];

      n2->self->drawNormals[0][i] = self->norm[i];
      n2->self->drawNormals[1][i] = self->drawNormals[idx3][i];
      n2->self->drawNormals[2][i] = self->norm[i];
   }
   n2->self->s[0] = (self->s[ idx2 ] - self->s[ idx1 ]) * place1 + self->s[ idx1 ];
   n2->self->s[1] = self->s[ idx3 ];
   n2->self->s[2] = (self->s[ idx3 ] - self->s[ idx1 ]) * place2 + self->s[ idx1 ];

   n2->self->t[0] = (self->t[ idx2 ] - self->t[ idx1 ]) * place1 + self->t[ idx1 ];
   n2->self->t[1] = self->t[ idx3 ];
   n2->self->t[2] = (self->t[ idx3 ] - self->t[ idx1 ]) * place2 + self->t[ idx1 ];

   n2->self->calculateNormal();
   n2->self->texture = self->texture;
   n2->self->triangle = self->triangle;
   n2->self->material = self->material;

   for ( int i = 0; i < 3; i++ )
   {
      self->coord[idx2][i] = p1[i];
      self->coord[idx3][i] = p2[i];

      self->drawNormals[idx2][i] = self->norm[i];
      self->drawNormals[idx3][i] = self->norm[i];
   }
   self->s[idx2] = (self->s[ idx2 ] - self->s[ idx1 ]) * place1 + self->s[ idx1 ];
   self->s[idx3] = (self->s[ idx3 ] - self->s[ idx1 ]) * place2 + self->s[ idx1 ];

   self->t[idx2] = (self->t[ idx2 ] - self->t[ idx1 ]) * place1 + self->t[ idx1 ];
   self->t[idx3] = (self->t[ idx3 ] - self->t[ idx1 ]) * place2 + self->t[ idx1 ];

   self->calculateD();
}

void BspTree::Node::splitNode( int idx1, int idx2, int idx3, 
      float * p1, BspTree::Node * n1,
      const float & place )
{
   n1->self = Poly::get();
   //printf( "split node poly: %d\n", n1->self->id );
   for ( int i = 0; i < 3; i++ )
   {
      n1->self->coord[0][i] = self->coord[idx1][i];
      n1->self->coord[1][i] = self->coord[idx2][i];
      n1->self->coord[2][i] = p1[i];

      n1->self->drawNormals[0][i] = self->drawNormals[idx1][i];
      n1->self->drawNormals[1][i] = self->drawNormals[idx2][i];
      n1->self->drawNormals[2][i] = self->norm[i];
   }

   n1->self->s[0] = self->s[ idx1 ];
   n1->self->s[1] = self->s[ idx2 ];
   n1->self->s[2] = (self->s[ idx3 ] - self->s[ idx2 ]) * place + self->s[ idx2 ];

   n1->self->t[0] = self->t[ idx1 ];
   n1->self->t[1] = self->t[ idx2 ];
   n1->self->t[2] = (self->t[ idx3 ] - self->t[ idx2 ]) * place + self->t[ idx2 ];

   n1->self->calculateNormal();
   n1->self->texture = self->texture;
   n1->self->triangle = self->triangle;
   n1->self->material = self->material;

   for ( int i = 0; i < 3; i++ )
   {
      self->coord[idx2][i] = p1[i];

      self->drawNormals[idx2][i] = self->norm[i];
   }
   self->s[idx2] = (self->s[ idx3 ] - self->s[ idx2 ]) * place + self->s[ idx2 ];
   self->t[idx2] = (self->t[ idx3 ] - self->t[ idx2 ]) * place + self->t[ idx2 ];

   self->calculateD();
}

void BspTree::Node::addChild( Node * n )
{
   float d1 = dot_product( self->norm, n->self->coord[0] );
   float d2 = dot_product( self->norm, n->self->coord[1] );
   float d3 = dot_product( self->norm, n->self->coord[2] );

   int i1 = 0;
   int i2 = 0;
   int i3 = 0;

   if ( !float_equiv( d1, self->d ) )
      i1 = ( d1 < self->d ) ? -1 : 1;
   if ( !float_equiv( d2, self->d ) )
      i2 = ( d2 < self->d ) ? -1 : 1;
   if ( !float_equiv( d3, self->d ) )
      i3 = ( d3 < self->d ) ? -1 : 1;

   //printf( "self d = %f\n", self->d );
   //printf( "addChild d = %f %f %f\n", d1, d2, d3 );
   //printf( "addChild i = %d %d %d\n", i1, i2, i3 );

   // This will catch co-plane also... which should be fine
   if ( i1 <= 0 && i2 <= 0 && i3 <= 0 )
   {
      //printf( "add right\n" );
      if ( left )
      {
         left->addChild( n );
      }
      else
      {
         left = n;
      }
      return;
   }

   if ( i1 >= 0 && i2 >= 0 && i3 >= 0 )
   {
      //printf( "add left\n" );
      if ( right )
      {
         right->addChild( n );
      }
      else
      {
         right = n;
      }
      return;
   }

   //printf( "split\n" );

   float p1[3];
   float p2[3];
   float place1 = 0.0f;
   float place2 = 0.0f;

   if ( i1 == 0 || i2 == 0 || i3 == 0 )
   {
      // one of the vertices is on the plane
      //printf( "split on vertex\n" );

      Node * n1 = Node::get();
      if ( i1 == 0 )
      {
         self->intersection( n->self->coord[1], n->self->coord[2], p1, place1 );

         n->splitNode( 0, 1, 2, p1, n1, place1 );

         if ( i2 < 0 )
         {
            if ( right )
               addChild( n );
            else
               right = n;

            if ( left )
               left->addChild( n1 );
            else
               left = n1;
         }
         else
         {
            if ( right )
               addChild( n1 );
            else
               right = n1;

            if ( left )
               left->addChild( n );
            else
               left = n;
         }
      }
      else if ( i2 == 0 )
      {
         self->intersection( n->self->coord[2], n->self->coord[0], p1, place1 );

         n->splitNode( 1, 2, 0, p1, n1, place1 );

         if ( i1 < 0 )
         {
            if ( right )
               addChild( n1 );
            else
               right = n1;

            if ( left )
               left->addChild( n );
            else
               left = n;
         }
         else
         {
            if ( right )
               addChild( n );
            else
               right = n;

            if ( left )
               left->addChild( n1 );
            else
               left = n1;
         }
      }
      else if ( i3 == 0 )
      {
         self->intersection( n->self->coord[0], n->self->coord[1], p1, place1 );

         n->splitNode( 2, 0, 1, p1, n1, place1 );

         if ( i1 < 0 )
         {
            if ( right )
               addChild( n );
            else
               right = n;

            if ( left )
               left->addChild( n1 );
            else
               left = n1;
         }
         else
         {
            if ( right )
               addChild( n1 );
            else
               right = n1;

            if ( left )
               left->addChild( n );
            else
               left = n;
         }
      }
   }
   else
   {
      Node * n1 = Node::get();
      Node * n2 = Node::get();
      if ( i1 == i2 )
      {
         //printf( "split 1/2\n" );
         self->intersection( n->self->coord[2], n->self->coord[0], p1, place1 );
         self->intersection( n->self->coord[2], n->self->coord[1], p2, place2 );
         //printf( "split at %f %f %f\n", p1[0], p1[1], p1[2] );
         //printf( "split at %f %f %f\n", p2[0], p2[1], p2[2]  );

         n->splitNodes( 2, 0, 1, p1, p2, n1, n2, place1, place2 );

         if ( i3 < 0 )
         {
            n1->left = n2;

            if ( right )
               right->addChild( n1 );
            else
               right = n1;

            if ( left )
               left->addChild( n );
            else
               left = n;
         }
         else
         {
            n1->right = n2;

            if ( left )
               left->addChild( n1 );
            else
               left = n1;

            if ( right )
               right->addChild( n );
            else
               right = n;
         }
      }
      else if ( i1 == i3 )
      {
         //printf( "split 1/3\n" );
         self->intersection( n->self->coord[1], n->self->coord[2], p1, place1 );
         self->intersection( n->self->coord[1], n->self->coord[0], p2, place2 );
         //printf( "split at %f %f %f\n", p1[0], p1[1], p1[2]  );
         //printf( "split at %f %f %f\n", p2[0], p2[1], p2[2]  );

         n->splitNodes( 1, 2, 0, p1, p2, n1, n2, place1, place2 );

         if ( i2 < 0 )
         {
            n1->left = n2;

            if ( right )
               right->addChild( n1 );
            else
               right = n1;

            if ( left )
               left->addChild( n );
            else
               left = n;
         }
         else
         {
            n1->right = n2;

            if ( left )
               left->addChild( n1 );
            else
               left = n1;

            if ( right )
               right->addChild( n );
            else
               right = n;
         }
      }
      else if ( i2 == i3 )
      {
         self->intersection( n->self->coord[0], n->self->coord[1], p1, place1 );
         self->intersection( n->self->coord[0], n->self->coord[2], p2, place2 );

         n->splitNodes( 0, 1, 2, p1, p2, n1, n2, place1, place2 );

         if ( i1 < 0 )
         {
            n1->left = n2;

            if ( right )
               right->addChild( n1 );
            else
               right = n1;

            if ( left )
               left->addChild( n );
            else
               left = n;
         }
         else
         {
            n1->right = n2;

            if ( left )
               left->addChild( n1 );
            else
               left = n1;

            if ( right )
               right->addChild( n );
            else
               right = n;
         }
      }
   }
}

void BspTree::Node::render( float * point, DrawingContext * context )
{
   float d = dot_product( self->norm, point );

   if ( d < self->d )
   {
      if ( right )
      {
         right->render( point, context );
      }
      self->render( context );
      if ( left )
      {
         left->render( point, context );
      }
   }
   else
   {
      if ( left )
      {
         left->render( point, context );
      }
      self->render( context );
      if ( right )
      {
         right->render( point, context );
      }
   }
}

void BspTree::Poly::stats()
{
   log_debug( "BspTree::Poly: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
}

void BspTree::Node::stats()
{
   log_debug( "BspTree::Node: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
}

int BspTree::Poly::flush()
{
   int c = 0;
   std::list<Poly *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

int BspTree::Node::flush()
{
   int c = 0;
   std::list<Node *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

#if 0

int main( int argc, char * argv[] )
{
   char input[128];

   BspTree tree;

   while ( fgets( input, sizeof(input), stdin ) )
   {
      float coord[0][3];
      float coord[1][3];
      float coord[2][3];

      if ( input[0] == 'c' )
      {
         printf( "camera\n" );
         if ( sscanf(&input[1], "%f %f %f", 
                  &coord[0][0], &coord[0][1], &coord[0][2] ) == 3 )
         {
            printf( "got camera point\n" );
            tree.render( coord[0] );
         }
      }
      else if ( sscanf( input, "%f %f %f   %f %f %f   %f %f %f", 
               &coord[0][0], &coord[0][1], &coord[0][2],
               &coord[1][0], &coord[1][1], &coord[1][2],
               &coord[2][0], &coord[2][1], &coord[2][2] ) == 9 )
      {
         Poly * p = Poly::get();
         printf( "read poly: %d\n", p->id );
         for ( int i = 0; i < 3; i++ )
         {
            p->coord[0][i] = coord[0][i];
            p->coord[1][i] = coord[1][i];
            p->coord[2][i] = coord[2][i];
         }
         p->calculateNormal();

         tree.addPoly( p );
      }
   }
   return 0;
}

#endif // 0
