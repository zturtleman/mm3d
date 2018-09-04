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


#include "glheaders.h"
#include "bounding.h"

BoundingBox::BoundingBox()
   : m_x1( 0.0 ),
     m_y1( 0.0 ),
     m_z1( 0.0 ),
     m_x2( 0.0 ),
     m_y2( 0.0 ),
     m_z2( 0.0 ),
     m_isMat( false )
{
   m_mat.loadIdentity();
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::setMatrixBounds( const Matrix & viewMat, double x1, double y1, double x2, double y2 )
{
   m_isMat = true;
   m_mat = viewMat;
   m_inv = m_mat.getInverse();
   m_x1 = x1;
   m_y1 = y1;
   m_z1 = 0;
   m_x2 = x2;
   m_y2 = y2;
   m_z2 = 0;
}

void BoundingBox::setBounds( double x1, double y1, double z1, double x2, double y2, double z2 )
{
   m_isMat = false;
   m_x1 = x1;
   m_y1 = y1;
   m_z1 = z1;
   m_x2 = x2;
   m_y2 = y2;
   m_z2 = z2;
}

void BoundingBox::draw()
{
   glEnable( GL_COLOR_LOGIC_OP );
   glColor3f( 1.0, 1.0, 1.0 );
   glLogicOp( GL_XOR );
   glBegin( GL_LINES );

   if ( m_isMat )
   {
      Vector p1;
      p1[0] = m_x1;
      p1[1] = m_y1;

      Vector p2;
      p2[0] = m_x2;
      p2[1] = m_y1;

      Vector p3;
      p3[0] = m_x2;
      p3[1] = m_y2;

      Vector p4;
      p4[0] = m_x1;
      p4[1] = m_y2;

      m_inv.apply( p1 );
      m_inv.apply( p2 );
      m_inv.apply( p3 );
      m_inv.apply( p4 );

      glVertex3dv( p1.getVector() );
      glVertex3dv( p2.getVector() );

      glVertex3dv( p2.getVector() );
      glVertex3dv( p3.getVector() );

      glVertex3dv( p3.getVector() );
      glVertex3dv( p4.getVector() );

      glVertex3dv( p4.getVector() );
      glVertex3dv( p1.getVector() );
   }
   else
   {
      {
         glVertex3f( m_x1, m_y1, m_z1 );
         glVertex3f( m_x1, m_y1, m_z2 );
         glVertex3f( m_x1, m_y1, m_z1 );
         glVertex3f( m_x1, m_y2, m_z1 );
         glVertex3f( m_x1, m_y2, m_z1 );
         glVertex3f( m_x1, m_y2, m_z2 );
         glVertex3f( m_x1, m_y1, m_z2 );
         glVertex3f( m_x1, m_y2, m_z2 );
      }
      {
         glVertex3f( m_x1, m_y1, m_z1 );
         glVertex3f( m_x1, m_y1, m_z2 );
         glVertex3f( m_x1, m_y1, m_z1 );
         glVertex3f( m_x2, m_y1, m_z1 );
         glVertex3f( m_x2, m_y1, m_z1 );
         glVertex3f( m_x2, m_y1, m_z2 );
         glVertex3f( m_x1, m_y1, m_z2 );
         glVertex3f( m_x2, m_y1, m_z2 );
      }
      {
         glVertex3f( m_x1, m_y1, m_z1 );
         glVertex3f( m_x2, m_y1, m_z1 );
         glVertex3f( m_x1, m_y1, m_z1 );
         glVertex3f( m_x1, m_y2, m_z1 );
         glVertex3f( m_x1, m_y2, m_z1 );
         glVertex3f( m_x2, m_y2, m_z1 );
         glVertex3f( m_x2, m_y1, m_z1 );
         glVertex3f( m_x2, m_y2, m_z1 );
      }
   }
   glEnd();
   glLogicOp( GL_COPY );
   glDisable( GL_COLOR_LOGIC_OP );
}
