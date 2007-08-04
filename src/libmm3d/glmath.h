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


#ifndef __GLMATH_H
#define __GLMATH_H

#include <stdio.h>
#include <math.h>

const double PI = 3.1415926;
const double PIOVER180 = PI / 180.0;

class Vector;
class Quaternion;

class Matrix
{
   public:
      Matrix();
      ~Matrix();

      void loadIdentity();
      void show() const;
      void set( int r, int c, double val );
      double get( int r, int c ) const;

      void setTranslation( const Vector & vector );
      void setTranslation( const double * vector );
      void setTranslation( const double & x, const double & y, const double & z );

      void setRotation( const Vector & radians );
      void setRotation( const double * radians );
      void setRotationInDegrees( const double * degrees );
      void setRotationInDegrees( const Vector & degrees );
      void setRotationInDegrees( double x, double y, double z );
      void getRotation( double & x, double & y, double & z ) const;
      void getRotation( Vector & radians ) const;
      void getRotation( double * radians ) const;
      void getTranslation( double & x, double & y, double & z ) const;
      void getTranslation( Vector & vector ) const;
      void getTranslation( double * vector ) const;

      void setInverseRotation( const double * radians );
      void setInverseRotationInDegrees( const double * radians );

      void setRotationOnAxis( const double * pVect, double radians );
      void setRotationQuaternion( const Quaternion & quat );
      void getRotationQuaternion( Quaternion & quat );

      void inverseTranslateVector( double * pVect ) const;
      void inverseRotateVector( double * pVect ) const;

      void normalizeRotation();

      void apply( float * pVec ) const;
      void apply( double * pVec ) const;
      void apply( Vector & pVec ) const;
      void apply3( float * pVec ) const;
      void apply3( double * pVec ) const;
      void apply3( Vector & pVec ) const;
      void apply3x( float * pVec ) const;  // Apply 4x4 matrix to 3 element vec
      void apply3x( double * pVec ) const;
      void apply3x( Vector & pVec ) const;

      void postMultiply( const Matrix & lhs );

      float getDeterminant() const;
      float getDeterminant3() const;
      Matrix getInverse() const;
      Matrix getSubMatrix( int i, int j ) const;

      friend Matrix operator*( const Matrix & lhs, const Matrix & rhs );
      friend Vector operator*( const Vector & lhs, const Matrix & rhs );
      friend class Vector;

   protected:

      const double * getMatrix() const { return m_val; };

      double m_val[16];
};

class Vector
{
   public:
      Vector( const double * val = NULL);
      Vector( const double & x, const double & y, const double & z, const double & w = 1.0 );
      ~Vector();

      void show() const;
      void translate( const Matrix & rhs );
      void transform( const Matrix & rhs );
      void transform3( const Matrix & rhs );
      void set( int c, double val );
      void setAll( double x, double y, double z, double w = 1.0 );
      void setAll( const double * vec, int count = 3 );
      double get( int c ) const { return m_val[c]; };

      void scale( double val );
      void scale3( double val );

      double mag();
      double mag3();

      void normalize();
      void normalize3();

      double dot3( const Vector & rhs );
      Vector cross3( const Vector & rhs ) const;

      const double * getVector() const { return m_val; };
      double * getVector() { return m_val; };

      double & operator[]( int index );
      const double & operator[]( int index ) const;
      Vector operator+=( const Vector & rhs );
      Vector operator-=( const Vector & rhs );

      friend Vector operator*( const Vector & lhs, const Matrix & rhs );
      friend Vector operator*( const Vector & lhs, const double & rhs );
      friend Vector operator-( const Vector & lhs, const Vector & rhs );
      friend Vector operator+( const Vector & lhs, const Vector & rhs );

   protected:
      double m_val[4];
};

class Quaternion : public Vector
{
   public:
      Quaternion( const double * val = NULL );
      Quaternion( const Vector & val );
      ~Quaternion();

      void show() const;
      void setEulerAngles( const double * radians );
      void setRotationOnAxis( double x, double y, double z, double radians );
      void setRotationOnAxis( const double * axis, double radians );
      void setRotationToPoint( const double & faceX, const double & faceY, const double & faceZ,
            const double & pointX, const double & pointY, const double & pointZ );
      void setRotationToPoint( const Vector & face, const Vector & point );
      void getRotationOnAxis( double * axis, double & radians );
      void set( int c, double val );
      double get( int c ) const { return m_val[c]; };

      void normalize();

      Quaternion swapHandedness();

      const double * getVector() const { return m_val; };

      friend Quaternion operator*( const Quaternion & lhs, const Quaternion & rhs );

   protected:
      //double m_val[4];
};

template<typename T> T distance( 
      const T & x1, const T & y1, const T z1, 
      const T & x2, const T & y2, const T z2 )
{
   T xDiff = x1 - x2;
   T yDiff = y1 - y2;
   T zDiff = z1 - z2;

   return sqrt( xDiff*xDiff + yDiff*yDiff + zDiff*zDiff );
}

template<typename T> T distance( 
      const T & x1, const T & y1, 
      const T & x2, const T & y2 )
{
   T xDiff = x1 - x2;
   T yDiff = y1 - y2;

   return sqrt( xDiff*xDiff + yDiff*yDiff );
}

template<typename T> T mag3( T * vec )
{
   return sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2] );
}

template<typename T> void normalize3( T * vec )
{
   if ( vec != NULL )
   {
      T length = mag3( vec );
      vec[0] = vec[0] / length;
      vec[1] = vec[1] / length;
      vec[2] = vec[2] / length;
   }
}

extern double distance ( const Vector & v1, const Vector & v2 );

extern double distance ( const double * v1, const double * v2 );

template<typename T> T dot3( T * lhs, T * rhs )
{
   return(   lhs[0] * rhs[0]
           + lhs[1] * rhs[1]
           + lhs[2] * rhs[2] );
}

template<typename T> bool equiv3( T * lhs, T * rhs )
{
   return(   fabs(lhs[0] - rhs[0]) < 0.0001
         &&  fabs(lhs[1] - rhs[1]) < 0.0001
         &&  fabs(lhs[2] - rhs[2]) < 0.0001 );
}

#endif // __GLMATH_H
