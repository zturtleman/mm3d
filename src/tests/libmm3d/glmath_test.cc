/*  Misfit Model 3D
 * 
 *  Copyright (c) 2007-2008 Kevin Worcester
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


// This file tests glmath.h

#include <QtTest/QtTest>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string>

#include "glmath.h"
#include "log.h"

#include "test_common.h"

double TOLERANCE = 0.001;

class GlmathTest : public QObject
{
   Q_OBJECT
private slots:
   void initTestCase()
   {
      log_enable_debug( false );
   }

   void testVectorInit()
   {
      {
         Vector v;
         QVERIFY_TRUE( fabs(v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(1.0 - v.get(3)) < TOLERANCE );
      }
      {
         // 4th element is ignored
         double array[4] = { 3, 4, 5, 6 };
         Vector v(array);
         QVERIFY_TRUE( fabs(3 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(1.0 - v.get(3)) < TOLERANCE );
      }
      {
         // 4th element defaults to 1
         Vector v(3, 4, 5);
         QVERIFY_TRUE( fabs(3 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(1.0 - v.get(3)) < TOLERANCE );
      }
      {
         Vector v(3, 4, 5, 6);
         QVERIFY_TRUE( fabs(3 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v.get(3)) < TOLERANCE );
      }
   }

   void testVectorSetGet()
   {
      {
         Vector v;
         QVERIFY_TRUE( fabs(v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(1 - v.get(3)) < TOLERANCE );
      }
      {
         Vector v;
         v.set(0, 3);
         v.set(1, 4);
         v.set(2, 5);
         v.set(3, 6);
         QVERIFY_TRUE( fabs(3 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v.get(3)) < TOLERANCE );
      }
      {
         Vector v;
         v[0] = 3;
         v[1] = 4;
         v[2] = 5;
         v[3] = 6;
         QVERIFY_TRUE( fabs(3 - v[0]) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v[1]) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v[2]) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v[3]) < TOLERANCE );
      }
      {
         const Vector v(3, 4, 5, 6);
         QVERIFY_TRUE( fabs(3 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v.get(3)) < TOLERANCE );
      }
      {
         const Vector v(3, 4, 5, 6);
         QVERIFY_TRUE( fabs(3 - v[0]) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v[1]) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v[2]) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v[3]) < TOLERANCE );
      }
   }

   void testVectorGetVector()
   {
      {
         const Vector vec(3, 4, 5, 6);
         const double * v = vec.getVector();
         QVERIFY_TRUE( fabs(3 - v[0]) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v[1]) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v[2]) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v[3]) < TOLERANCE );
      }
      {
         Vector vec(3, 4, 5, 6);
         double * v = vec.getVector();
         QVERIFY_TRUE( fabs(3 - v[0]) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v[1]) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v[2]) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v[3]) < TOLERANCE );

         v[1] = 7.0;

         QVERIFY_TRUE( fabs(3 - vec[0]) < TOLERANCE );
         QVERIFY_TRUE( fabs(7 - vec[1]) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - vec[2]) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - vec[3]) < TOLERANCE );
      }
   }

   void testVectorSetAll()
   {
      {
         // 4th element defaults to 1
         Vector v(3, 4, 5, 6);
         QVERIFY_TRUE( fabs(3 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v.get(3)) < TOLERANCE );

         v.setAll( 7, 8, 9 );
         QVERIFY_TRUE( fabs(7 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(8 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(9 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(1 - v.get(3)) < TOLERANCE );
      }
      {
         Vector v(3, 4, 5, 6);
         QVERIFY_TRUE( fabs(3 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v.get(3)) < TOLERANCE );

         v.setAll( 7, 8, 9, 10 );
         QVERIFY_TRUE( fabs(7  - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(8  - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(9  - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(10 - v.get(3)) < TOLERANCE );
      }
      {
         // 4th element defaults to 1
         Vector v(3, 4, 5, 6);
         QVERIFY_TRUE( fabs(3 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(5 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v.get(3)) < TOLERANCE );

         double v1[4] = { 7, 8, 9, 10 };
         v.setAll( v1, 4 );
         QVERIFY_TRUE( fabs(7 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(8 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(9 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(10 - v.get(3)) < TOLERANCE );

         // Don't set v[3]
         double v2[4] = { 2, 4, 6, 8 };
         v.setAll( v2, 3 );
         QVERIFY_TRUE( fabs(2 - v.get(0)) < TOLERANCE );
         QVERIFY_TRUE( fabs(4 - v.get(1)) < TOLERANCE );
         QVERIFY_TRUE( fabs(6 - v.get(2)) < TOLERANCE );
         QVERIFY_TRUE( fabs(10 - v.get(3)) < TOLERANCE );
      }
   }

   void testVectorEqual()
   {
      {
         const Vector vec( 1, 2, 3, 4 );
         const Vector exp( 1, 2, 3, 4 );
         QVERIFY_TRUE(exp == vec);
      }
      {
         const Vector vec( 1, 2, 3, 4 );
         const Vector exp( 1.001, 2, 3, 4 );
         QVERIFY_FALSE(exp == vec);
      }
      {
         const Vector vec( 1, 2, 3, 4 );
         const Vector exp( 1, 2.001, 3, 4 );
         QVERIFY_FALSE(exp == vec);
      }
      {
         const Vector vec( 1, 2, 3, 4 );
         const Vector exp( 1, 2, 3.001, 4 );
         QVERIFY_FALSE(exp == vec);
      }
      {
         const Vector vec( 1, 2, 3, 4 );
         const Vector exp( 1, 2, 3, 4.001 );
         QVERIFY_FALSE(exp == vec);
      }
   }

   void testVectorScale()
   {
      {
         Vector v(3, 4, 5, 6);
         const Vector exp(-6, -8, -10, -12);

         v.scale( -2 );
         QVERIFY_TRUE( v == exp );
      }
      {
         Vector v(3, 4, 5, 6);
         const Vector exp(-6, -8, -10, 6);

         v.scale3( -2 );
         QVERIFY_TRUE( v == exp );
      }
      {
         const Vector v(3, 4, 5, 6);
         const Vector exp(-6, -8, -10, -12);

         const Vector res = v * -2.0;
         QVERIFY_TRUE( res == exp );
      }
   }

   void testVectorPlusEquals()
   {
      {
         Vector lhs(1, 2, 3, 4);
         const Vector rhs(3, 4, 5, 6);
         const Vector exp(4, 6, 8, 10);

         lhs += rhs;

         QVERIFY_TRUE( lhs == exp );
      }
      {
         Vector lhs(1, 2, 3, 4);
         const Vector exp(2, 4, 6, 8);

         lhs += lhs;

         QVERIFY_TRUE( lhs == exp );
      }
      {
         const Vector lhs(1, 2, 3, 4);
         const Vector rhs(3, 4, 5, 6);
         const Vector exp(4, 6, 8, 10);

         const Vector res = lhs + rhs;

         QVERIFY_TRUE( res == exp );
      }
   }

   void testVectorMinusEquals()
   {
      {
         Vector lhs(1, 2, 3, 4);
         const Vector rhs(2, 4, 6, 8);
         const Vector exp(-1, -2, -3, -4);

         lhs -= rhs;

         QVERIFY_TRUE( lhs == exp );
      }
      {
         Vector lhs(1, 2, 3, 4);
         const Vector exp(0, 0, 0, 0);

         lhs -= lhs;

         QVERIFY_TRUE( lhs == exp );
      }
      {
         const Vector lhs(1, 2, 3, 4);
         const Vector rhs(2, 4, 6, 8);
         const Vector exp(-1, -2, -3, -4);

         const Vector res = lhs - rhs;

         QVERIFY_TRUE( res == exp );
      }
   }

   void testVectorMag()
   {
      {
         const Vector x(1, 0, 0, 1);
         const Vector y(0, 2, 0, 1);
         const Vector z(0, 0, 3, 1);

         QVERIFY_TRUE( fabs(x.mag3() - 1) < TOLERANCE );
         QVERIFY_TRUE( fabs(y.mag3() - 2) < TOLERANCE );
         QVERIFY_TRUE( fabs(z.mag3() - 3) < TOLERANCE );
      }
      {
         const Vector x(1, 0, 0, 0);
         const Vector y(0, 2, 0, 0);
         const Vector z(0, 0, 3, 0);
         const Vector w(0, 0, 0, 4);

         QVERIFY_TRUE( fabs(x.mag() - 1) < TOLERANCE );
         QVERIFY_TRUE( fabs(y.mag() - 2) < TOLERANCE );
         QVERIFY_TRUE( fabs(z.mag() - 3) < TOLERANCE );
         QVERIFY_TRUE( fabs(w.mag() - 4) < TOLERANCE );
      }
      {
         const Vector v(1, -2, 3, -4);
         QVERIFY_TRUE( fabs(v.mag3() - 3.741657) < TOLERANCE );
         QVERIFY_TRUE( fabs(v.mag() - 5.477226) < TOLERANCE );
      }
   }

   void testVectorDot()
   {
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(1, 0, 0, 1);

         QVERIFY_TRUE( fabs(lhs.dot3(rhs) - 1 ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(0, 1, 0, 1);

         QVERIFY_TRUE( fabs(lhs.dot3(rhs) ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(-1, 0, 0, 1);

         QVERIFY_TRUE( fabs(lhs.dot3(rhs) + 1 ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(0, -1, 0, 1);

         QVERIFY_TRUE( fabs(lhs.dot3(rhs) ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         Vector rhs(1, -1, 0, 1);
         rhs.normalize3();

         QVERIFY_TRUE( fabs(lhs.dot3(rhs) - cos(PI / 4)) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         Vector rhs(-1, -1, 0, 1);
         rhs.normalize3();

         QVERIFY_TRUE( fabs(lhs.dot3(rhs) - cos(PI * 3 / 4)) < TOLERANCE );
      }
   }

   void testVectorNormalize()
   {
      {
         Vector vec(1, 2, 3, 4);
         Vector exp(0.267261, 0.534522, 0.801784, 4);

         vec.normalize3();

         QVERIFY_TRUE( vec == exp );
      }
      {
         Vector vec(1, 2, 3, 4);
         Vector exp(0.182574, 0.365148, 0.547723, 0.730297);

         vec.normalize();

         QVERIFY_TRUE( vec == exp );
      }
   }

   void testVectorCross()
   {
      // FIXME could be more thorough
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(0, 1, 0, 1);
         const Vector exp(0, 0, 1, 1);

         const Vector res = lhs.cross3(rhs);

         QVERIFY_TRUE( res == exp );
      }
      {
         const Vector lhs(0, 1, 0, 1);
         const Vector rhs(1, 0, 0, 1);
         const Vector exp(0, 0, -1, 1);

         const Vector res = lhs.cross3(rhs);

         QVERIFY_TRUE( res == exp );
      }
   }

   void testVectorTranslate()
   {
      // FIXME this is not translation, this is transformation.
      // 1) Make this actually a translation.
      // 2) Change Vector::translate calls that should be transform
      // call Vector::transform instead.
      {
         Matrix mat;
         Vector vec(1, 1, 1);
         const Vector exp(5, 6, 7, 1);

         mat.setTranslation( 4, 5, 6 );

         vec.translate( mat );

         QVERIFY_TRUE( vec == exp );
      }
   }

   void testVectorTransform()
   {
      // Translate
      {
         Matrix mat;
         Vector vec(1, 1, 1);
         Vector exp(5, 6, 7);

         mat.setTranslation( 4, 5, 6 );

         vec.transform( mat );

         QVERIFY_TRUE( vec == exp );
      }
      // Translate (ignored, transform3)
      {
         Matrix mat;
         Vector vec(1, 1, 1);
         Vector exp(1, 1, 1);

         mat.setTranslation( 4, 5, 6 );

         vec.transform3( mat );

         QVERIFY_TRUE( vec == exp );
      }
      // X Rotation
      {
         Matrix mat;
         Vector vec(1, 2, 3);
         Vector exp(1, -3, 2);

         mat.setRotation( Vector(PI / 2, 0, 0) );

         vec.transform( mat );

         QVERIFY_TRUE( vec == exp );
      }
      // Y Rotation
      {
         Matrix mat;
         Vector vec(1, 2, 3);
         Vector exp(-3, 2, 1);

         mat.setRotation( Vector(0, -PI / 2, 0) );

         vec.transform( mat );

         QVERIFY_TRUE( vec == exp );
      }
      // Z Rotation
      {
         Matrix mat;
         Vector vec(1, 2, 3);
         Vector exp(2, -1, 3);

         mat.setRotation( Vector(0, 0, PI*3 / 2) );

         vec.transform( mat );

         QVERIFY_TRUE( vec == exp );
      }
      // Scale
      {
         Matrix mat;
         Vector vec(1, 2, 3);
         Vector exp(2, 6, 12, 5);

         mat.set( 0, 0, 2.0 );
         mat.set( 1, 1, 3.0 );
         mat.set( 2, 2, 4.0 );
         mat.set( 3, 3, 5.0 );

         vec.transform( mat );

         QVERIFY_TRUE( vec == exp );
      }
      // Scale3
      {
         Matrix mat;
         Vector vec(1, 2, 3);
         Vector exp(2, 6, 12);

         mat.set( 0, 0, 2.0 );
         mat.set( 1, 1, 3.0 );
         mat.set( 2, 2, 4.0 );
         mat.set( 3, 3, 5.0 );

         vec.transform3( mat );

         QVERIFY_TRUE( vec == exp );
      }
      // Scale
      {
         Matrix mat;
         const Vector vec(1, 2, 3);
         const Vector exp(2, 6, 12, 5);

         mat.set( 0, 0, 2.0 );
         mat.set( 1, 1, 3.0 );
         mat.set( 2, 2, 4.0 );
         mat.set( 3, 3, 5.0 );

         const Vector res = vec * mat;

         QVERIFY_TRUE( res == exp );
      }
   }

   void testMatrixInit()
   {
      const Matrix m;

      for ( int c = 0; c < 4; ++c )
      {
         for ( int r = 0; r < 4; ++r )
         {
            if ( r == c )
            {
               QVERIFY_TRUE(float_equiv(m.get(r, c), 1.0));
            }
            else
            {
               QVERIFY_TRUE(float_equiv(m.get(r, c), 0.0));
            }
         }
      }
   }

   void testMatrixLoadIdentity()
   {
      Matrix m;

      QVERIFY_TRUE( m.isIdentity() );

      for ( int c = 0; c < 4; ++c )
      {
         for ( int r = 0; r < 4; ++r )
         {
            m.set(r, c, 4.0);
            QVERIFY_TRUE(float_equiv(m.get(r, c), 4.0));
         }
      }

      QVERIFY_FALSE( m.isIdentity() );

      m.loadIdentity();

      QVERIFY_TRUE( m.isIdentity() );

      for ( int c = 0; c < 4; ++c )
      {
         for ( int r = 0; r < 4; ++r )
         {
            if ( r == c )
            {
               QVERIFY_TRUE(float_equiv(m.get(r, c), 1.0));
            }
            else
            {
               QVERIFY_TRUE(float_equiv(m.get(r, c), 0.0));
            }
         }
      }
   }

   void testMatrixSetGet()
   {
      Matrix m;

      QVERIFY_TRUE( m.isIdentity() );

      for ( int c = 0; c < 4; ++c )
      {
         for ( int r = 0; r < 4; ++r )
         {
            m.set(r, c, r * 8.0 + c);
         }
      }

      for ( int c = 0; c < 4; ++c )
      {
         for ( int r = 0; r < 4; ++r )
         {
            QVERIFY_TRUE(float_equiv(m.get(r, c), r * 8.0 + c));
         }
      }
   }

   void testMatrixEqual()
   {
      for ( int c = 0; c < 4; ++c )
      {
         for ( int r = 0; r < 4; ++r )
         {
            Matrix lhs;
            Matrix rhs;

            QVERIFY_TRUE(lhs == rhs);
            QVERIFY_TRUE(lhs.equiv(rhs));
            lhs.set(r, c, lhs.get(r, c) + 0.001);
            QVERIFY_FALSE(lhs == rhs);
            QVERIFY_FALSE(lhs.equiv(rhs));
         }
      }
   }

   void testMatrixEquiv()
   {
      {
         Matrix lhs;
         Matrix rhs;
         lhs.setRotation( Vector(-PI, 0, 0) );
         rhs.setRotation( Vector(PI, 0, 0) );
         QVERIFY_TRUE(lhs.equiv(rhs));
      }
      {
         Matrix lhs;
         Matrix rhs;
         lhs.setRotation( Vector(-PI / 2.0, 0, 0) );
         rhs.setRotation( Vector(PI / 2.0, 0, 0) );
         QVERIFY_FALSE(lhs.equiv(rhs));
      }
      {
         Matrix lhs;
         Matrix rhs;
         lhs.setRotation( Vector(0, -PI, 0) );
         rhs.setRotation( Vector(0, PI, 0) );
         QVERIFY_TRUE(lhs.equiv(rhs));
      }
      {
         Matrix lhs;
         Matrix rhs;
         lhs.setRotation( Vector(0, -PI / 2.0, 0) );
         rhs.setRotation( Vector(0, PI / 2.0, 0) );
         QVERIFY_FALSE(lhs.equiv(rhs));
      }
      {
         Matrix lhs;
         Matrix rhs;
         lhs.setRotation( Vector(0, 0, -PI) );
         rhs.setRotation( Vector(0, 0, PI) );
         QVERIFY_TRUE(lhs.equiv(rhs));
      }
      {
         Matrix lhs;
         Matrix rhs;
         lhs.setRotation( Vector(0, 0, -PI / 2.0) );
         rhs.setRotation( Vector(0, 0, PI / 2.0) );
         QVERIFY_FALSE(lhs.equiv(rhs));
      }
   }

   void testMatrixSetGetTranslation()
   {
      {
         Matrix m;
         m.setTranslation( 1, 2, 3 );
         double x = 0, y = 0, z = 0;
         m.getTranslation( x, y, z );
         QVERIFY_TRUE(float_equiv(x, 1.0));
         QVERIFY_TRUE(float_equiv(y, 2.0));
         QVERIFY_TRUE(float_equiv(z, 3.0));
      }
      {
         Matrix m;
         m.setTranslation( Vector(4, 6, 8) );
         Vector v;
         m.getTranslation( v );
         QVERIFY_TRUE(float_equiv(v[0], 4.0));
         QVERIFY_TRUE(float_equiv(v[1], 6.0));
         QVERIFY_TRUE(float_equiv(v[2], 8.0));
      }
      {
         Matrix m;
         double array[3] = { 1, 4, 9 };
         m.setTranslation( array );
         double v[3] = { 0, 0, 0 };
         m.getTranslation( v );
         QVERIFY_TRUE(float_equiv(v[0], 1.0));
         QVERIFY_TRUE(float_equiv(v[1], 4.0));
         QVERIFY_TRUE(float_equiv(v[2], 9.0));
      }
   }

   void testMatrixSetGetRotation()
   {
      {
         Matrix m;
         m.setRotation( Vector(PI/2, PI/3, PI/4) );
         Vector v;
         m.getRotation( v );
         QVERIFY_TRUE(float_equiv(v[0], PI/2));
         QVERIFY_TRUE(float_equiv(v[1], PI/3));
         QVERIFY_TRUE(float_equiv(v[2], PI/4));
         double x = 0, y = 0, z = 0;
         m.getRotation( x, y, z );
         QVERIFY_TRUE(float_equiv(x, PI/2));
         QVERIFY_TRUE(float_equiv(y, PI/3));
         QVERIFY_TRUE(float_equiv(z, PI/4));
      }
      {
         Matrix m;
         double vec[3] = { PI/3, PI/4, PI/5 };
         m.setRotation( vec );
         double v[3] = { 0, 0, 0 };
         m.getRotation( v );
         QVERIFY_TRUE(float_equiv(v[0], PI/3));
         QVERIFY_TRUE(float_equiv(v[1], PI/4));
         QVERIFY_TRUE(float_equiv(v[2], PI/5));
      }
   }

   void testMatrixApplyRotation()
   {
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         Vector vec(0, 2, 0);
         const Vector exp(0, 0, 2);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         double vec[4] = { 0, 2, 0, 1 };
         const Vector exp(0, 0, 2);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         float vec[4] = { 0, 2, 0, 1 };
         const Vector exp(0, 0, 2);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv((float)exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv((float)exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv((float)exp[2], vec[2]));
      }

      // Apply with a 0 'w' element in the vector
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         Vector vec(0, 2, 0, 0);
         const Vector exp(0, 0, 2, 0);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         double vec[4] = { 0, 2, 0, 0 };
         const Vector exp(0, 0, 2, 0);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         float vec[4] = { 0, 2, 0, 0 };
         const Vector exp(0, 0, 2, 0);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv((float)exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv((float)exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv((float)exp[2], vec[2]));
      }

      // Apply3
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         Vector vec(0, 2, 0, 0);
         const Vector exp(0, 0, 2, 0);
         m.apply3( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         double vec[4] = { 0, 2, 0, 0 };
         const Vector exp(0, 0, 2, 0);
         m.apply3( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         float vec[4] = { 0, 2, 0, 0 };
         const Vector exp(0, 0, 2, 0);
         m.apply3( vec );
         QVERIFY_TRUE(float_equiv((float)exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv((float)exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv((float)exp[2], vec[2]));
      }

      // Apply3x
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         Vector vec(0, 2, 0, 0);
         const Vector exp(0, 0, 2, 0);
         m.apply3x( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         double vec[4] = { 0, 2, 0, 0 };
         const Vector exp(0, 0, 2);
         m.apply3x( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         Matrix m;
         m.setRotation( Vector(PI/2, 0, 0) );
         float vec[4] = { 0, 2, 0, 0 };
         const Vector exp(0, 0, 2);
         m.apply3x( vec );
         QVERIFY_TRUE(float_equiv((float)exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv((float)exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv((float)exp[2], vec[2]));
      }
   }

   void testMatrixApplyTranslation()
   {
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         Vector vec(0, 2, 0);
         const Vector exp(1, 4, 3);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         double vec[4] = { 0, 2, 0, 1 };
         const Vector exp(1, 4, 3);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         float vec[4] = { 0, 2, 0, 1 };
         const Vector exp(1, 4, 3);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv((float)exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv((float)exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv((float)exp[2], vec[2]));
      }

      // Apply with a 0 'w' element in the vector
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         Vector vec(0, 2, 0, 0);
         const Vector exp(0, 2, 0, 0);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         double vec[4] = { 0, 2, 0, 0 };
         const Vector exp(0, 2, 0, 0);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         float vec[4] = { 0, 2, 0, 0 };
         const Vector exp(0, 2, 0, 0);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv((float)exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv((float)exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv((float)exp[2], vec[2]));
      }

      // Apply3
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         Vector vec(0, 2, 0);
         const Vector exp(0, 2, 0);
         m.apply3( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         double vec[4] = { 0, 2, 0, 1 };
         const Vector exp(0, 2, 0);
         m.apply3( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         float vec[4] = { 0, 2, 0, 1 };
         const Vector exp(0, 2, 0);
         m.apply3( vec );
         QVERIFY_TRUE(float_equiv((float)exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv((float)exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv((float)exp[2], vec[2]));
      }

      // Apply3x
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3, 0) );
         Vector vec(0, 2, 0, 0);
         const Vector exp(1, 4, 3, 0);
         m.apply3x( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         double vec[4] = { 0, 2, 0, 0 };
         const Vector exp(1, 4, 3);
         m.apply3x( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         Matrix m;
         m.setTranslation( Vector(1, 2, 3) );
         float vec[4] = { 0, 2, 0, 0 };
         const Vector exp(1, 4, 3);
         m.apply3x( vec );
         QVERIFY_TRUE(float_equiv((float)exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv((float)exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv((float)exp[2], vec[2]));
      }
   }

   void testMatrixApplyScale()
   {
      {
         Matrix m;
         m.set(0, 0, 2);
         m.set(1, 1, -4);
         m.set(2, 2, 6);
         Vector vec(2, 2, 2);
         const Vector exp(4, -8, 12);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }

      // Apply with a 0 'w' element in the vector
      {
         Matrix m;
         m.set(0, 0, 2);
         m.set(1, 1, -4);
         m.set(2, 2, 6);
         Vector vec(2, 2, 2, 0);
         const Vector exp(4, -8, 12, 0);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }

      // Apply3
      {
         Matrix m;
         m.set(0, 0, 2);
         m.set(1, 1, -4);
         m.set(2, 2, 6);
         Vector vec(2, 2, 2, 0);
         const Vector exp(4, -8, 12, 0);
         m.apply3( vec );
         QVERIFY_TRUE(exp == vec);
      }

      // Apply3x
      {
         Matrix m;
         m.set(0, 0, 2);
         m.set(1, 1, -4);
         m.set(2, 2, 6);
         Vector vec(2, 2, 2, 0);
         const Vector exp(4, -8, 12, 0);
         m.apply3( vec );
         QVERIFY_TRUE(exp == vec);
      }
   }

   void testMatrixApplyRotationTranslation()
   {
      Matrix m;
      m.setTranslation( Vector(1, 2, 3) );
      m.setRotation( Vector(0, -PI/2, 0) );
      {
         double vec[4] = { 2, 0, 0, 1 };
         const Vector exp(1, 2, 5);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         double vec[4] = { 2, 0, 0, 0 };
         const Vector exp(0, 0, 2, 0);
         m.apply( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         double vec[4] = { 2, 0, 0, 0 };
         const Vector exp(1, 2, 5, 0);
         m.apply3x( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         double vec[4] = { 2, 0, 0, 1 };
         const Vector exp(0, 0, 2);
         m.apply3( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
   }

   void testMatrixApplyInverse()
   {
      Matrix m;
      m.setTranslation( Vector(1, 2, 3) );
      m.setRotation( Vector(0, -PI/2, 0) );
      {
         // Inverse rotate doesn't apply translation
         double vec[4] = { 2, 0, 0, 1 };
         const Vector exp(0, 0, -2);
         m.inverseRotateVector( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
      {
         // Inverse translate doesn't apply rotation
         double vec[4] = { 2, 0, 0, 1 };
         const Vector exp(1, -2, -3);
         m.inverseTranslateVector( vec );
         QVERIFY_TRUE(float_equiv(exp[0], vec[0]));
         QVERIFY_TRUE(float_equiv(exp[1], vec[1]));
         QVERIFY_TRUE(float_equiv(exp[2], vec[2]));
      }
   }

   void testMatrixSetInverseRotation()
   {
      Matrix m;
      m.setInverseRotation( Vector(0, -PI/2, 0).getVector() );
      Vector vec(2, 0, 0);
      Vector exp(0, 0, -2);
      m.apply( vec );
      QVERIFY_TRUE(exp == vec);
   }

   void testMatrixSetInverseInDegrees()
   {
      Matrix m;
      m.setInverseRotationInDegrees( Vector(0, -90, 0).getVector() );
      Vector vec(2, 0, 0);
      Vector exp(0, 0, -2);
      m.apply( vec );
      QVERIFY_TRUE(exp == vec);
   }

   void testMatrixSetRotationInDegrees()
   {
      {
         Matrix m;
         m.setRotationInDegrees( Vector(0, 90, 0) );
         Vector vec(2, 0, 0);
         Vector exp(0, 0, -2);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         double rot[3] = { 0, 90, 0 };
         m.setRotationInDegrees( rot );
         Vector vec(2, 0, 0);
         Vector exp(0, 0, -2);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }
      {
         Matrix m;
         double rot[3] = { 0, 90, 0 };
         m.setRotationInDegrees( rot[0], rot[1], rot[2] );
         Vector vec(2, 0, 0);
         Vector exp(0, 0, -2);
         m.apply( vec );
         QVERIFY_TRUE(exp == vec);
      }
   }

   void testMatrixSetGetQuaternion()
   {
      {
         Matrix m;
         m.setRotationOnAxis( Vector(1, 0, 0).getVector(), PI/2 );

         Vector exp(PI/2, 0, 0);
         Vector rot(0, 0, 0);
         m.getRotation(rot.getVector());
         QVERIFY_TRUE(exp == rot);

         Quaternion qexp;
         qexp.setRotationOnAxis(1, 0, 0, PI/2);
         Quaternion quat;
         m.getRotationQuaternion(quat);
         QVERIFY_TRUE(qexp == quat);
      }
      {
         Matrix m;
         Quaternion q;
         q.setRotationOnAxis( 0, 1, 0, PI/2 );
         m.setRotationQuaternion( q );

         Vector exp(0, PI/2, 0);
         Vector rot(0, 0, 0);
         m.getRotation(rot.getVector());
         QVERIFY_TRUE(exp == rot);

         Quaternion qexp;
         qexp.setRotationOnAxis(0, 1, 0, PI/2);
         Quaternion quat;
         m.getRotationQuaternion(quat);
         QVERIFY_TRUE(qexp == quat);
      }
   }

   void testMatrixQuaternionRotation()
   {
      Vector v(1, 0, 1);
      v.normalize3();

      Matrix m;
      m.setRotationOnAxis( v.getVector(), PI/2 );

      Vector v1(2, 0, 0);
      Vector v2(0, 0, 2);

      Vector exp1(1, sqrt(2), 1);
      Vector exp2(1, -sqrt(2), 1);

      m.apply(v1);
      m.apply(v2);

      QVERIFY_TRUE( exp1 == v1 );
      QVERIFY_TRUE( exp2 == v2 );
   }

   void testMatrixMultiply()
   {
      Matrix lhs;
      Matrix rhs;
      lhs.setRotation( Vector(PI/2, 0.0, 0.0) );
      rhs.setRotation( Vector(0.0, PI/2, 0.0) );

      {
         //  0  0 -1  0
         //  1  0  0  0
         //  0 -1  0  0
         //  0  0  0  1
         Matrix exp;
         exp.set(0, 0, 0);
         exp.set(1, 1, 0);
         exp.set(2, 2, 0);
         exp.set(0, 2, -1);
         exp.set(1, 0,  1);
         exp.set(2, 1, -1);

         Matrix res = lhs * rhs;
         QVERIFY_TRUE(exp == res);
      }
      {
         //  0  1  0  0
         //  0  0  1  0
         //  1  0  0  0
         //  0  0  0  1
         Matrix exp;
         exp.set(0, 0, 0);
         exp.set(1, 1, 0);
         exp.set(2, 2, 0);
         exp.set(0, 1, 1);
         exp.set(1, 2, 1);
         exp.set(2, 0, 1);

         Matrix res = rhs * lhs;
         QVERIFY_TRUE(exp == res);
      }
      {
         //  post multiply: lhs = rhs * lhs
         //  0  1  0  0
         //  0  0  1  0
         //  1  0  0  0
         //  0  0  0  1
         Matrix exp;
         exp.set(0, 0, 0);
         exp.set(1, 1, 0);
         exp.set(2, 2, 0);
         exp.set(0, 1, 1);
         exp.set(1, 2, 1);
         exp.set(2, 0, 1);

         Matrix res = lhs;
         res.postMultiply(rhs);
         QVERIFY_TRUE(exp == res);
      }
   }

   void testMatrixNormalizeRotation()
   {
      Matrix lhs;
      Matrix rhs;
      lhs.setRotation( Vector(PI/2, PI, -PI/2) );

      rhs.set( 0, 0, 2 );
      rhs.set( 1, 1, 3 );
      rhs.set( 2, 2, 4 );

      Matrix m = rhs * lhs;

      {
         Vector vec( 1, 2, 3 );
         m.apply( vec );

         Vector exp(-12, 2, -6);
         QVERIFY_TRUE( exp == vec );
      }
      
      m.normalizeRotation();

      {
         Vector vec( 1, 2, 3 );
         m.apply( vec );

         Vector exp( -3, 1, -2 );
         QVERIFY_TRUE( exp == vec );
      }
   }

   void testMatrixGetInverse()
   {
      {
         Matrix m;
         m.setRotation( Vector( PI/2, 0, 0 ) );
         Matrix inv = m.getInverse();
         Matrix id = m * inv;
         QVERIFY_TRUE( id.isIdentity() );
         QVERIFY_TRUE( fabs(m.getDeterminant()) > TOLERANCE );
      }
      {
         Matrix m;
         m.setTranslation( 1, 2, 3 );
         Matrix inv = m.getInverse();
         Matrix id = m * inv;
         QVERIFY_TRUE( id.isIdentity() );
         QVERIFY_TRUE( fabs(m.getDeterminant()) > TOLERANCE );
      }
      {
         Matrix m;
         m.set( 0, 0, 2 );
         m.set( 1, 1, 3 );
         m.set( 2, 2, 4 );
         Matrix inv = m.getInverse();
         Matrix id = m * inv;
         QVERIFY_TRUE( id.isIdentity() );
         QVERIFY_TRUE( fabs(m.getDeterminant()) > TOLERANCE );
      }
      {
         // Can't invert
         Matrix m;
         m.set( 0, 0, 2 );
         m.set( 1, 1, 0 );
         m.set( 2, 2, 4 );
         Matrix inv = m.getInverse();
         Matrix id = m * inv;
         QVERIFY_FALSE( id.isIdentity() );
         QVERIFY_FALSE( fabs(m.getDeterminant()) > TOLERANCE );
      }
   }

   void testQuaternionInit()
   {
      {
         const Quaternion q;
         const Vector exp( 0, 0, 0, 1 );
         QVERIFY_TRUE( exp == q );
      }
      {
         const Quaternion q( Vector(1, 2, 3, 4).getVector() );
         const Vector exp( 1, 2, 3, 4 );
         QVERIFY_TRUE( exp == q );
      }
      {
         const Quaternion q( Vector(1, 2, 3, 4) );
         const Vector exp( 1, 2, 3, 4 );
         QVERIFY_TRUE( exp == q );
      }
   }

   void testQuaternionSet()
   {
      Quaternion q;
      {
         const Vector exp( 0, 0, 0, 1 );
         QVERIFY_TRUE( exp == q );
      }
      q.set(0, 1);
      q.set(1, 2);
      q.set(2, 3);
      q.set(3, 4);
      {
         const Vector exp( 1, 2, 3, 4 );
         QVERIFY_TRUE( exp == q );
      }

      QVERIFY_TRUE( float_equiv( 1.0, q.get(0) ) );
      QVERIFY_TRUE( float_equiv( 2.0, q.get(1) ) );
      QVERIFY_TRUE( float_equiv( 3.0, q.get(2) ) );
      QVERIFY_TRUE( float_equiv( 4.0, q.get(3) ) );

      const double * vec = q.getVector();
      QVERIFY_TRUE( float_equiv( 1.0, vec[0] ) );
      QVERIFY_TRUE( float_equiv( 2.0, vec[1] ) );
      QVERIFY_TRUE( float_equiv( 3.0, vec[2] ) );
      QVERIFY_TRUE( float_equiv( 4.0, vec[3] ) );
   }

   // Note this function also tests Quaternion::getRotationOnAxis
   // Quaternion::setRotationOnAxis is tested in the Matrix code
   void testQuaternionEulerAngles()
   {
      {
         Quaternion q;
         q.setEulerAngles( Vector(PI/2, 0, 0).getVector() );
         double axis[3] = { 0, 0, 0 };
         double rad = 0;
         q.getRotationOnAxis( axis, rad );

         double exp[3] = { 1, 0, 0 };
         QVERIFY_TRUE( floatCompareVector(axis, exp, 3) );
         QVERIFY_TRUE( float_equiv(rad, PI/2) );
      }
      {
         Quaternion q;
         q.setEulerAngles( Vector(0, PI/2, 0).getVector() );
         double axis[3] = { 0, 0, 0 };
         double rad = 0;
         q.getRotationOnAxis( axis, rad );

         double exp[3] = { 0, 1, 0 };
         QVERIFY_TRUE( floatCompareVector(axis, exp, 3) );
         QVERIFY_TRUE( float_equiv(rad, PI/2) );
      }
      {
         Quaternion q;
         q.setEulerAngles( Vector(0, 0, PI/2).getVector() );
         double axis[3] = { 0, 0, 0 };
         double rad = 0;
         q.getRotationOnAxis( axis, rad );

         double exp[3] = { 0, 0, 1 };
         QVERIFY_TRUE( floatCompareVector(axis, exp, 3) );
         QVERIFY_TRUE( float_equiv(rad, PI/2) );
      }
   }

   void testQuaternionSwapHandedness()
   {
      Quaternion q;
      q.setEulerAngles( Vector(PI/2, 0, 0).getVector() );

      {
         double axis[3] = { 0, 0, 0 };
         double rad = 0;
         q.getRotationOnAxis( axis, rad );
         double exp[3] = { 1, 0, 0 };
         QVERIFY_TRUE( floatCompareVector(axis, exp, 3) );
         QVERIFY_TRUE( float_equiv(rad, PI/2) );
      }

      q.swapHandedness();

      {
         double axis[3] = { 0, 0, 0 };
         double rad = 0;
         q.getRotationOnAxis( axis, rad );
         double exp[3] = { 1, 0, 0 };
         QVERIFY_TRUE( floatCompareVector(axis, exp, 3) );
         QVERIFY_TRUE( float_equiv(rad, PI/2) );
      }
   }

   void testQuaternionNormalize()
   {
      Quaternion q(Vector(1, 2, 3, 4).getVector());
      q.normalize();

      Vector exp(1, 2, 3, 4);
      exp.normalize();

      QVERIFY_TRUE( exp == q );
   }

   void testQuaternionSetRotationToPoint()
   {
      Vector face(0, 0, 2);
      Vector point(3, 0, 0);

      {
         Quaternion q;
         q.setRotationToPoint( face, point );
         double axis[3] = { 0, 0, 0 };
         double rad = 0;
         q.getRotationOnAxis( axis, rad );
         double exp[3] = { 0, 1, 0 };
         QVERIFY_TRUE( floatCompareVector(axis, exp, 3) );
         QVERIFY_TRUE( float_equiv(rad, PI/2) );
      }
      {
         Quaternion q;
         q.setRotationToPoint( face[0], face[1], face[2],
               point[0], point[1], point[2] );
         double axis[3] = { 0, 0, 0 };
         double rad = 0;
         q.getRotationOnAxis( axis, rad );
         double exp[3] = { 0, 1, 0 };
         QVERIFY_TRUE( floatCompareVector(axis, exp, 3) );
         QVERIFY_TRUE( float_equiv(rad, PI/2) );
      }
   }

   void testDistance()
   {
      // vector functions
      {
         Vector p1(2, 3, 4);
         Vector p2(4, 6, 8);

         QVERIFY_TRUE( float_equiv(5.385165, distance(p1, p2)) );
      }
      {
         Vector p1(2, 3, 4);
         Vector p2(4, 6, 8);

         QVERIFY_TRUE( float_equiv(5.385165, distance(p1.getVector(), p2.getVector())) );
      }
      // template functions
      {
         Vector p1(2, 3, 4);
         Vector p2(4, 6, 8);

         QVERIFY_TRUE( float_equiv(5.385165, distance(
                     p1[0], p1[1], p1[2],
                     p2[0], p2[1], p2[2] )) );
      }
      {
         Vector p1(2, 3, 4);
         Vector p2(4, 6, 8);

         QVERIFY_TRUE( float_equiv(3.605551, distance(
                     p1[0], p1[1],
                     p2[0], p2[1] )) );
      }
   }

   void testMag()
   {
      Vector vec(2, 3, 4);
      QVERIFY_TRUE( float_equiv(5.385165, mag3(vec.getVector())) );
   }

   void testNormalize()
   {
      Vector vec(2, 3, 4, 5);
      normalize3( vec.getVector() );
      Vector exp(0.371391, 0.557086, 0.742781, 5);
      QVERIFY_TRUE(exp == vec);
   }

   void testDot()
   {
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(1, 0, 0, 1);

         QVERIFY_TRUE( fabs(dot3(lhs.getVector(), rhs.getVector()) - 1 ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(0, 1, 0, 1);

         QVERIFY_TRUE( fabs(lhs.dot3(rhs) ) < TOLERANCE );
         QVERIFY_TRUE( fabs(dot3(lhs.getVector(), rhs.getVector()) ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(-1, 0, 0, 1);

         QVERIFY_TRUE( fabs(dot3(lhs.getVector(), rhs.getVector()) + 1 ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         const Vector rhs(0, -1, 0, 1);

         QVERIFY_TRUE( fabs(dot3(lhs.getVector(), rhs.getVector()) ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         Vector rhs(1, -1, 0, 1);
         rhs.normalize3();

         QVERIFY_TRUE( fabs(dot3(lhs.getVector(), rhs.getVector()) - cos(PI / 4) ) < TOLERANCE );
      }
      {
         const Vector lhs(1, 0, 0, 1);
         Vector rhs(-1, -1, 0, 1);
         rhs.normalize3();

         QVERIFY_TRUE( fabs(dot3(lhs.getVector(), rhs.getVector()) - cos(PI * 3 / 4) ) < TOLERANCE );
      }
   }

   void testEquiv()
   {
      {
         const Vector v1( 1, 1, 1 );
         const Vector v2( 1, 1, 1 );

         QVERIFY_TRUE( equiv3( v1.getVector(), v2.getVector() ) );
      }
      {
         const Vector v1( 1, 1, 1 );
         const Vector v2( 1.001, 1, 1 );

         QVERIFY_FALSE( equiv3( v1.getVector(), v2.getVector() ) );
      }
      {
         const Vector v1( 1, 1, 1 );
         const Vector v2( 1, 1.001, 1 );

         QVERIFY_FALSE( equiv3( v1.getVector(), v2.getVector() ) );
      }
      {
         const Vector v1( 1, 1, 1 );
         const Vector v2( 1, 1, 1.001 );

         QVERIFY_FALSE( equiv3( v1.getVector(), v2.getVector() ) );
      }
   }

   void testCalculateNormal()
   {
      // FIXME could be more thorough
      {
         const Vector p1(1, 1, 1, 1);
         const Vector p2(3, 1, 1, 1);
         const Vector p3(1, 4, 1, 1);

         const Vector exp(0, 0, 1, 1);

         Vector normal;

         calculate_normal( normal.getVector(),
               p1.getVector(), p2.getVector(), p3.getVector() );

         QVERIFY_TRUE( normal == exp );
      }
      {
         const Vector p1(1, 1, 1, 1);
         const Vector p2(1, 4, 1, 1);
         const Vector p3(3, 1, 1, 1);

         const Vector exp(0, 0, -1, 1);

         Vector normal;

         calculate_normal( normal.getVector(),
               p1.getVector(), p2.getVector(), p3.getVector() );

         QVERIFY_TRUE( normal == exp );
      }
      {
         const Vector lhs(0, 1, 0, 1);
         const Vector rhs(1, 0, 0, 1);
         const Vector exp(0, 0, -1, 1);

         const Vector res = lhs.cross3(rhs);

         QVERIFY_TRUE( res == exp );
      }
   }

   // Doesn't actually test anything, but it's just for debugging anyway.
   void testShow()
   {
      Matrix m;
      m.show();

      Vector v;
      v.show();

      Quaternion q;
      q.show();
   }

};

QTEST_MAIN(GlmathTest)
#include "glmath_test.moc"
