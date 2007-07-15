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


#include "rotatepoint.h"
#include "glheaders.h"

RotatePoint::RotatePoint()
{
}

RotatePoint::~RotatePoint()
{
}

void RotatePoint::draw()
{
   glColor3f( 0.0, 1.0, 0.0 );
   glBegin( GL_LINES );

   // XY plane
   glVertex3f( m_x - 0.25, m_y + 0.0,  m_z + 0.0 );
   glVertex3f( m_x + 0.0,  m_y - 0.25, m_z + 0.0 );

   glVertex3f( m_x + 0.0,  m_y - 0.25, m_z + 0.0 );
   glVertex3f( m_x + 0.25, m_y + 0.0,  m_z + 0.0 );

   glVertex3f( m_x + 0.25, m_y + 0.0,  m_z + 0.0 );
   glVertex3f( m_x +  0.0, m_y + 0.25,  m_z + 0.0 );

   glVertex3f( m_x +  0.0, m_y + 0.25,  m_z + 0.0 );
   glVertex3f( m_x - 0.25, m_y + 0.0,  m_z + 0.0 );

   // YZ plane
   glVertex3f( m_x + 0.0, m_y - 0.25, m_z + 0.0  );
   glVertex3f( m_x + 0.0, m_y + 0.0,  m_z - 0.25 );

   glVertex3f( m_x + 0.0, m_y + 0.0,  m_z - 0.25 );
   glVertex3f( m_x + 0.0, m_y + 0.25, m_z + 0.0  );

   glVertex3f( m_x + 0.0, m_y + 0.25, m_z + 0.0  );
   glVertex3f( m_x + 0.0, m_y + 0.0,  m_z + 0.25 );

   glVertex3f( m_x + 0.0, m_y +  0.0, m_z + 0.25 );
   glVertex3f( m_x + 0.0, m_y - 0.25, m_z + 0.0  );

   // XZ plane
   glVertex3f( m_x - 0.25, m_y + 0.0, m_z + 0.0  );
   glVertex3f( m_x + 0.0,  m_y + 0.0, m_z - 0.25 );

   glVertex3f( m_x + 0.0,  m_y + 0.0, m_z - 0.25 );
   glVertex3f( m_x + 0.25, m_y + 0.0, m_z + 0.0  );

   glVertex3f( m_x + 0.25, m_y + 0.0, m_z + 0.0  );
   glVertex3f( m_x +  0.0, m_y + 0.0, m_z + 0.25 );

   glVertex3f( m_x + 0.0,  m_y + 0.0, m_z + 0.25 );
   glVertex3f( m_x - 0.25, m_y + 0.0, m_z + 0.0  );

   glEnd();
}

void RotatePoint::setPoint( double x, double y, double z )
{
   m_x = x;
   m_y = y;
   m_z = z;
}

void RotatePoint::getPoint( double & x, double & y, double & z )
{
   x = m_x;
   y = m_y;
   z = m_z;
}

