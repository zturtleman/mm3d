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


#ifndef __BOUNDING_H
#define __BOUNDING_H

#include "decal.h"
#include "glmath.h"

class BoundingBox : public Decal
{
   public:
      BoundingBox();
      virtual ~BoundingBox();

      void draw( float devicePixelRatio );

      void setMatrixBounds( const Matrix & viewMat, double x1, double y1, double x2, double y2 );
      void setBounds( double x1, double y1, double z1, double x2, double y2, double z2 );

   protected:
      double m_x1;
      double m_y1;
      double m_z1;
      double m_x2;
      double m_y2;
      double m_z2;
      Matrix m_mat;
      Matrix m_inv;
      bool   m_isMat;
};

#endif // __BOUNDING_H
