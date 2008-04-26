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

class MiscTest : public QObject
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
      // FIXME setAll array
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
            if ( c != 3 )
            {
               QVERIFY_FALSE(lhs.equiv(rhs));
            }
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

   // FIXME add tests
   //
   // Vector:
   //  ?translate
   //
   // Operations:
   //   Matrix * Matrix
   //  xVector * Matrix
   //  xVector * double
   //  xVector - Vector
   //  xVector + Vector
   //   Quat * Quat
   //
   // Matrix:
   //  xinitialization (identity)
   //  xloadIdentity
   //  xequiv
   //  xset
   //  xget
   //  -show
   //   setTranslation
   //   getTranslation
   //  -setRotation
   //  -setRotationInDegrees
   //  -setInverseRotation
   //  -setInverseRotationInDegrees
   //   getRotation
   //   setRotationQuaternion
   //   getRotationQuaternion
   //   inverseRotateVector
   //   inverseTranslateVector
   //   normalizeRotation
   //  -apply, apply3, apply3x
   //  -postMultiply
   //   getDeterminant
   //   getSubMatrix
   //   getInverse
   //   operator Matrix*
   //  -operator Vector*
   //
   // Quaternion:
   //   Quat * Quat
   //  -initialization
   //   set
   //   get
   //   getVector
   //  -show
   //   setEulerAngles
   //  -setRotationOnAxis
   //  -setRotationToPoint
   //   getRotationOnAxis
   //   normalize
   //   swapHandedness
   //
   // Misc:
   //   distance
   //
   // Template:
   //   distance
   //   mag
   //   normalize
   //   dot3
   //   equiv3
   //   calculate_normal
};

QTEST_MAIN(MiscTest)
#include "glmath_test.moc"
