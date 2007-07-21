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


#include "glmath.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

Matrix::Matrix()
{
   loadIdentity();
}

Matrix::~Matrix()
{
}

void Matrix::loadIdentity()
{
   for ( int r = 0; r < 4; r++ )
   {
      for ( int c = 0; c < 4; c++ )
      {
         m_val[ (r<<2) + c ] = (c==r) ? 1.0 : 0.0;
      }
   }
}

void Matrix::show() const
{
   for ( int r = 0; r < 4; r++ )
   {
      for ( int c = 0; c < 4; c++ )
      {
         printf( "%0.2f ", m_val[ (r<<2) + c ] );
      }
      printf( "\n" );
   }
}

void Matrix::set( int r, int c, double val )
{
   m_val[ (r<<2) + c ] = val;
}

double Matrix::get( int r, int c ) const
{
   return m_val[ (r<<2) + c ];
}

void Matrix::setTranslation( const double * vector )
{
   m_val[ 12 ] = vector[ 0 ];
   m_val[ 13 ] = vector[ 1 ];
   m_val[ 14 ] = vector[ 2 ];
}

void Matrix::setTranslation( const Vector & vector )
{
   m_val[ 12 ] = vector[ 0 ];
   m_val[ 13 ] = vector[ 1 ];
   m_val[ 14 ] = vector[ 2 ];
}

void Matrix::setTranslation( const double & x, const double & y, const double & z )
{
   m_val[ 12 ] = x;
   m_val[ 13 ] = y;
   m_val[ 14 ] = z;
}

// takes array of three angles in radians
void Matrix::setRotation( const double * radians )
{
   if ( radians )
   {
      double cr = cos( radians[0] );
      double sr = sin( radians[0] );
      double cp = cos( radians[1] );
      double sp = sin( radians[1] );
      double cy = cos( radians[2] );
      double sy = sin( radians[2] );
      double srsp = sr * sp;
      double crsp = cr * sp;

      m_val[  0 ] = cp * cy;
      m_val[  1 ] = cp * sy;
      m_val[  2 ] = -sp;

      m_val[  4 ] = (srsp * cy) - (cr * sy);
      m_val[  5 ] = (srsp * sy) + (cr * cy);
      m_val[  6 ] = (sr * cp);

      m_val[  8 ] = (crsp * cy) + (sr * sy);
      m_val[  9 ] = (crsp * sy) - (sr * cy);
      m_val[ 10 ] = (cr * cp);
   }
}

void Matrix::setRotation( const Vector & radians )
{
   setRotation( radians.getVector() );
}

void Matrix::getRotation( double * radians ) const
{
   if ( radians )
   {
      double sinYaw;
      double cosYaw;
      double sinPitch = -m_val[2];
      double cosPitch = sqrt( 1 - sinPitch*sinPitch );
      double sinRoll;
      double cosRoll;

      // if cosPitch is NaN, it's because sinPitch^2 was slightly greater than 1.0
      // in that case, cosPitch should be 0.0, so detect and correct it here
      if ( cosPitch != cosPitch )
      {
         cosPitch = 0.0;
      }

      if ( fabs( cosPitch ) > 0.0001 )
      {
         sinRoll = m_val[6]  / cosPitch;
         cosRoll = m_val[10] / cosPitch;
         sinYaw  = m_val[1]  / cosPitch;
         cosYaw  = m_val[0]  / cosPitch;
      }
      else
      {
         sinRoll = -m_val[9];
         cosRoll = m_val[5];
         sinYaw  = 0;
         cosYaw  = 1;
      }

      radians[2] = atan2( sinYaw, cosYaw );
      radians[1] = atan2( sinPitch, cosPitch );
      radians[0] = atan2( sinRoll, cosRoll );
   }
}

void Matrix::getRotation( Vector & radians ) const
{
   getRotation( radians.getVector() );
}

void Matrix::getTranslation( double * vector ) const
{
   if ( vector )
   {
      vector[ 0 ] = m_val[ 12 ];
      vector[ 1 ] = m_val[ 13 ];
      vector[ 2 ] = m_val[ 14 ];
   }
}

void Matrix::getTranslation( Vector & vector ) const
{
   vector[ 0 ] = m_val[ 12 ];
   vector[ 1 ] = m_val[ 13 ];
   vector[ 2 ] = m_val[ 14 ];
}

void Matrix::setRotationInDegrees( const double * degrees )
{
   double radVector[3];
   radVector[ 0 ] = degrees[ 0 ] * PIOVER180;
   radVector[ 1 ] = degrees[ 1 ] * PIOVER180;
   radVector[ 2 ] = degrees[ 2 ] * PIOVER180;
   setRotation( radVector );
}

void Matrix::setRotationInDegrees( const Vector & degrees )
{
   setRotationInDegrees( degrees.getVector() );
}

void Matrix::setRotationInDegrees( double x, double y, double z )
{
   double radVector[3];
   radVector[ 0 ] = x * PIOVER180;
   radVector[ 1 ] = y * PIOVER180;
   radVector[ 2 ] = z * PIOVER180;
   setRotation( radVector );
}

void Matrix::setInverseRotation( const double * radians )
{
   if ( radians )
   {
      double cr = cos( -radians[0] );
      double sr = sin( -radians[0] );
      double cp = cos( -radians[1] );
      double sp = sin( -radians[1] );
      double cy = cos( -radians[2] );
      double sy = sin( -radians[2] );
      double srsp = sr * sp;
      double crsp = cr * sp;

      m_val[  0 ] = cp * cy;
      m_val[  1 ] = cp * sy;
      m_val[  2 ] = -sp;

      m_val[  4 ] = (srsp * cy) - (cr * sy);
      m_val[  5 ] = (srsp * sy) + (cr * cy);
      m_val[  6 ] = (sr * cp);

      m_val[  8 ] = (crsp * cy) + (sr * sy);
      m_val[  9 ] = (crsp * sy) - (sr * cy);
      m_val[ 10 ] = (cr * cp);
   }
}

void Matrix::setInverseRotationInDegrees( const double * degrees )
{
   double radVector[3];
   radVector[ 0 ] = degrees[ 0 ] * PIOVER180;
   radVector[ 1 ] = degrees[ 1 ] * PIOVER180;
   radVector[ 2 ] = degrees[ 2 ] * PIOVER180;
   setInverseRotation( radVector );
}

void Matrix::setRotationOnAxis( const double * pVect, double radians )
{
   if ( pVect )
   {
      Quaternion quat;
      quat.setRotationOnAxis( pVect, radians );
      setRotationQuaternion( quat );
   }
}

void Matrix::setRotationQuaternion( const Quaternion & quat )
{
   float xx      = quat.get(0) * quat.get(0);
   float xy      = quat.get(0) * quat.get(1);
   float xz      = quat.get(0) * quat.get(2);
   float xw      = quat.get(0) * quat.get(3);

   float yy      = quat.get(1) * quat.get(1);
   float yz      = quat.get(1) * quat.get(2);
   float yw      = quat.get(1) * quat.get(3);

   float zz      = quat.get(2) * quat.get(2);
   float zw      = quat.get(2) * quat.get(3);

   m_val[0]  = 1 - 2 * ( yy + zz );
   m_val[4]  =     2 * ( xy - zw );
   m_val[8]  =     2 * ( xz + yw );

   m_val[1]  =     2 * ( xy + zw );
   m_val[5]  = 1 - 2 * ( xx + zz );
   m_val[9]  =     2 * ( yz - xw );

   m_val[2]  =     2 * ( xz - yw );
   m_val[6]  =     2 * ( yz + xw );
   m_val[10] = 1 - 2 * ( xx + yy );

   m_val[3]  = m_val[7] = m_val[11] = m_val[12] = m_val[13] = m_val[14] = 0;
   m_val[15] = 1;
}

void Matrix::getRotationQuaternion( Quaternion & quat )
{
   // FIXME implement and test
   double s;

   double t = 1 + m_val[0] + m_val[5] + m_val[10];
   if ( t > 0.00001 )
   {
      s = sqrt(t) * 2;
      quat.set( 0, ( m_val[6] - m_val[9] ) / s );
      quat.set( 1, ( m_val[8] - m_val[2] ) / s );
      quat.set( 2, ( m_val[1] - m_val[4] ) / s );
      quat.set( 3, 0.25 * s );
   }
   else
   {
      if ( m_val[0] > m_val[5] && m_val[0] > m_val[10] ) // Column 0
      {
         s  = sqrt( 1.0 + m_val[0] - m_val[5] - m_val[10] ) * 2;
         quat.set( 0, 0.25 * s );
         quat.set( 1, (m_val[1] + m_val[4] ) / s );
         quat.set( 2, (m_val[8] + m_val[2] ) / s );
         quat.set( 3, (m_val[6] - m_val[9] ) / s );
      }
      else if ( m_val[5] > m_val[10] ) // Column 1
      {
         s  = sqrt( 1.0 + m_val[5] - m_val[0] - m_val[10] ) * 2;
         quat.set( 0, (m_val[1] + m_val[4] ) / s );
         quat.set( 1, 0.25 * s );
         quat.set( 2, (m_val[6] + m_val[9] ) / s );
         quat.set( 3, (m_val[8] - m_val[2] ) / s );
      }
      else // Column 2
      {
         s  = sqrt( 1.0 + m_val[10] - m_val[0] - m_val[5] ) * 2;
         quat.set( 0, (m_val[8] + m_val[2] ) / s );
         quat.set( 1, (m_val[6] + m_val[9] ) / s );
         quat.set( 2, 0.25 * s );
         quat.set( 3, (m_val[1] - m_val[4] ) / s );
      }
   }
}

void Matrix::inverseRotateVector( double * pVect ) const
{
    double vec[3];

    vec[0] = pVect[0] * m_val[0] + pVect[1] * m_val[1] + pVect[2] * m_val[2];
    vec[1] = pVect[0] * m_val[4] + pVect[1] * m_val[5] + pVect[2] * m_val[6];
    vec[2] = pVect[0] * m_val[8] + pVect[1] * m_val[9] + pVect[2] * m_val[10];

    memcpy( pVect, vec, sizeof( double )*3 );
}

void Matrix::inverseTranslateVector( double *pVect ) const
{
    pVect[0] = pVect[0] - m_val[12];
    pVect[1] = pVect[1] - m_val[13];
    pVect[2] = pVect[2] - m_val[14];
}

void Matrix::normalizeRotation()
{
   normalize3( &m_val[0] );
   normalize3( &m_val[4] );
   normalize3( &m_val[8] );
}

void Matrix::apply( float * pVec ) const
{
   if ( pVec )
   {
      float vec[4];
      for ( int c = 0; c < 4; c++ )
      {
         vec[ c ] 
            = pVec[ 0 ] * m_val[ ( 0<<2 ) + c ]
            + pVec[ 1 ] * m_val[ ( 1<<2 ) + c ]
            + pVec[ 2 ] * m_val[ ( 2<<2 ) + c ]
            + pVec[ 3 ] * m_val[ ( 3<<2 ) + c ] ;
      }

      pVec[0] = vec[0];
      pVec[1] = vec[1];
      pVec[2] = vec[2];
      pVec[3] = vec[3];
   }
}

void Matrix::apply( double * pVec ) const
{
   if ( pVec )
   {
      double vec[4];
      for ( int c = 0; c < 4; c++ )
      {
         vec[ c ] 
            = pVec[ 0 ] * m_val[ ( 0<<2 ) + c ]
            + pVec[ 1 ] * m_val[ ( 1<<2 ) + c ]
            + pVec[ 2 ] * m_val[ ( 2<<2 ) + c ]
            + pVec[ 3 ] * m_val[ ( 3<<2 ) + c ] ;
      }

      pVec[0] = vec[0];
      pVec[1] = vec[1];
      pVec[2] = vec[2];
      pVec[3] = vec[3];
   }
}

void Matrix::apply( Vector & pVec ) const
{
   apply( pVec.getVector() );
}

void Matrix::apply3( float * pVec ) const
{
   if ( pVec )
   {
      float vec[4];
      for ( int c = 0; c < 3; c++ )
      {
         vec[ c ] 
            = pVec[ 0 ] * m_val[ ( 0<<2 ) + c ]
            + pVec[ 1 ] * m_val[ ( 1<<2 ) + c ]
            + pVec[ 2 ] * m_val[ ( 2<<2 ) + c ];
      }

      pVec[0] = vec[0];
      pVec[1] = vec[1];
      pVec[2] = vec[2];
   }
}

void Matrix::apply3( double * pVec ) const
{
   if ( pVec )
   {
      double vec[3];
      for ( int c = 0; c < 3; c++ )
      {
         vec[ c ] 
            = pVec[ 0 ] * m_val[ ( 0<<2 ) + c ]
            + pVec[ 1 ] * m_val[ ( 1<<2 ) + c ]
            + pVec[ 2 ] * m_val[ ( 2<<2 ) + c ];
      }

      pVec[0] = vec[0];
      pVec[1] = vec[1];
      pVec[2] = vec[2];
   }
}

void Matrix::apply3( Vector & pVec ) const
{
   apply3( pVec.getVector() );
}

void Matrix::apply3x( float * pVec ) const
{
   if ( pVec )
   {
      float vec[4];
      for ( int c = 0; c < 3; c++ )
      {
         vec[ c ] 
            = pVec[ 0 ] * m_val[ ( 0<<2 ) + c ]
            + pVec[ 1 ] * m_val[ ( 1<<2 ) + c ]
            + pVec[ 2 ] * m_val[ ( 2<<2 ) + c ]
            + 1.0f      * m_val[ ( 3<<2 ) + c ];
      }

      pVec[0] = vec[0];
      pVec[1] = vec[1];
      pVec[2] = vec[2];
   }
}

void Matrix::apply3x( double * pVec ) const
{
   if ( pVec )
   {
      double vec[3];
      for ( int c = 0; c < 3; c++ )
      {
         vec[ c ] 
            = pVec[ 0 ] * m_val[ ( 0<<2 ) + c ]
            + pVec[ 1 ] * m_val[ ( 1<<2 ) + c ]
            + pVec[ 2 ] * m_val[ ( 2<<2 ) + c ]
            + 1.0       * m_val[ ( 3<<2 ) + c ];
      }

      pVec[0] = vec[0];
      pVec[1] = vec[1];
      pVec[2] = vec[2];
   }
}

void Matrix::apply3x( Vector & pVec ) const
{
   apply3x( pVec.getVector() );
}

void Matrix::postMultiply( const Matrix & lhs )
{
   double val[16];
   int r;
   int c;

   for ( r = 0; r < 4; r++ )
   {
      for ( c = 0; c < 4; c++ )
      {
         val[ (r<<2) + c ] 
            = lhs.m_val[ (r<<2) + 0 ] * m_val[ ( 0<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 1 ] * m_val[ ( 1<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 2 ] * m_val[ ( 2<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 3 ] * m_val[ ( 3<<2 ) + c ]
            ;
      }
   }

   for ( r = 0; r < 16; r++ )
   {
      m_val[r] = val[r];
   }
}

float Matrix::getDeterminant3() const
{
   float det = 
        m_val[0] * ( m_val[5]*m_val[10] - m_val[6]*m_val[9] )
      - m_val[4] * ( m_val[1]*m_val[10] - m_val[2]*m_val[9] )
      + m_val[8] * ( m_val[1]*m_val[6]  - m_val[2]*m_val[5] );

   return det;
}

float Matrix::getDeterminant() const
{
   float result = 0;

   for ( int n = 0, i = 1; n < 4; n++, i *= -1 )
   {
      Matrix msub3 = getSubMatrix( 0, n );

      float det = msub3.getDeterminant3();
      result += m_val[n] * det * i;
   }

   return( result );
}

Matrix Matrix::getSubMatrix( int i, int j ) const
{
   Matrix ret;
   for( int di = 0; di < 3; di ++ ) 
   {
      for( int dj = 0; dj < 3; dj ++ ) 
      {
         int si = di + ( ( di >= i ) ? 1 : 0 );
         int sj = dj + ( ( dj >= j ) ? 1 : 0 );

         ret.set( di, dj, m_val[si * 4 + sj] );
      }
   }

   return ret;
}

Matrix Matrix::getInverse() const
{
   Matrix mr;

   float mdet = getDeterminant();

   if ( fabs( mdet ) < 0.0005 )
   {
      return mr;
   }

   for ( int i = 0; i < 4; i++ )
   {
      for ( int j = 0; j < 4; j++ )
      {
         int sign = 1 - ( (i +j) % 2 ) * 2;
         Matrix mtemp = getSubMatrix( i, j );
         mr.set( j, i, ( mtemp.getDeterminant3() * sign ) / mdet);
      }
   }

   return mr;
}


Matrix operator*( const Matrix &lhs, const Matrix & rhs )
{
   Matrix m;
   for ( int r = 0; r < 4; r++ )
   {
      for ( int c = 0; c < 4; c++ )
      {
         m.m_val[ (r<<2) + c ] 
            = lhs.m_val[ (r<<2) + 0 ] * rhs.m_val[ ( 0<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 1 ] * rhs.m_val[ ( 1<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 2 ] * rhs.m_val[ ( 2<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 3 ] * rhs.m_val[ ( 3<<2 ) + c ]
            ;
      }
   }

   return m;
}

Vector operator*( const Vector &lhs, const Matrix & rhs )
{
   Vector m;
   for ( int r = 0; r < 1; r++ )
   {
      for ( int c = 0; c < 4; c++ )
      {
         m.m_val[ (r<<2) + c ] 
            = lhs.m_val[ (r<<2) + 0 ] * rhs.m_val[ ( 0<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 1 ] * rhs.m_val[ ( 1<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 2 ] * rhs.m_val[ ( 2<<2 ) + c ]
            + lhs.m_val[ (r<<2) + 3 ] * rhs.m_val[ ( 3<<2 ) + c ]
            ;
      }
   }

   return m;
}

Vector operator*( const Vector &lhs, const double & rhs )
{
   Vector v;
   v.m_val[0] = lhs.m_val[0] * rhs;
   v.m_val[1] = lhs.m_val[1] * rhs;
   v.m_val[2] = lhs.m_val[2] * rhs;
   v.m_val[3] = lhs.m_val[3] * rhs;
   return v;
}

Vector operator-( const Vector &lhs, const Vector & rhs )
{
   Vector v;
   v.m_val[0] = lhs.m_val[0] - rhs.m_val[0];
   v.m_val[1] = lhs.m_val[1] - rhs.m_val[1];
   v.m_val[2] = lhs.m_val[2] - rhs.m_val[2];
   v.m_val[3] = lhs.m_val[3] - rhs.m_val[3];
   return v;
}

Vector operator+( const Vector &lhs, const Vector & rhs )
{
   Vector v;
   v.m_val[0] = lhs.m_val[0] + rhs.m_val[0];
   v.m_val[1] = lhs.m_val[1] + rhs.m_val[1];
   v.m_val[2] = lhs.m_val[2] + rhs.m_val[2];
   v.m_val[3] = lhs.m_val[3] + rhs.m_val[3];
   return v;
}

Quaternion operator*( const Quaternion &lhs, const Quaternion &rhs )
{
   Quaternion res;

	res.m_val[0] = lhs.m_val[3]*rhs.m_val[0] + lhs.m_val[0]*rhs.m_val[3] + lhs.m_val[1]*rhs.m_val[2] - lhs.m_val[2]*rhs.m_val[1];
	res.m_val[1] = lhs.m_val[3]*rhs.m_val[1] + lhs.m_val[1]*rhs.m_val[3] + lhs.m_val[2]*rhs.m_val[0] - lhs.m_val[0]*rhs.m_val[2];
	res.m_val[2] = lhs.m_val[3]*rhs.m_val[2] + lhs.m_val[2]*rhs.m_val[3] + lhs.m_val[0]*rhs.m_val[1] - lhs.m_val[1]*rhs.m_val[0];
   res.m_val[3] = lhs.m_val[3]*rhs.m_val[3] - lhs.m_val[0]*rhs.m_val[0] - lhs.m_val[1]*rhs.m_val[1] - lhs.m_val[2]*rhs.m_val[2];

   return res;
}

Vector::Vector( const double * val )
{
   if ( val )
   {
      m_val[0] = val[0];
      m_val[1] = val[1];
      m_val[2] = val[2];
      m_val[3] = 1.0;
   }
   else
   {
      m_val[0] = 0.0;
      m_val[1] = 0.0;
      m_val[2] = 0.0;
      m_val[3] = 1.0;
   }
}

Vector::Vector( const double & x, const double & y, const double & z, const double & w )
{
   m_val[0] = x;
   m_val[1] = y;
   m_val[2] = z;
   m_val[3] = w;
}

Vector::~Vector()
{
}

void Vector::translate( const Matrix & rhs )
{
   Vector v;
   for ( int r = 0; r < 4; r++ )
   {
      v.m_val[ r ] 
         = m_val[ 0 ] * rhs.m_val[ (0<<2) + r ]
         + m_val[ 1 ] * rhs.m_val[ (1<<2) + r ]
         + m_val[ 2 ] * rhs.m_val[ (2<<2) + r ]
         + m_val[ 3 ] * rhs.m_val[ (3<<2) + r ]
         ;
   }

   *this = v;
}

void Vector::set( int c, double val )
{
   m_val[ c ] = val;
}

void Vector::setAll( double x, double y, double z, double w )
{
   m_val[ 0 ] = x;
   m_val[ 1 ] = y;
   m_val[ 2 ] = z;
   m_val[ 3 ] = w;
}

void Vector::setAll( const double * vec, int count )
{
   for ( int i = 0; i < count && i < 4; i++ )
   {
      m_val[ i ] = vec[i];
   }
}

void Vector::show() const
{
   for ( int c = 0; c < 4; c++ )
   {
      printf( "%.2f ", m_val[c] );
   }
   printf( "\n" );
}

void Vector::transform( const Matrix& rhs )
{
    double vector[ 4 ];
    const double *matrix = rhs.getMatrix();

    vector[ 0 ] = m_val[ 0 ] * matrix[ 0 ] + m_val[ 1 ] * matrix[ 4 ] + m_val[ 2 ] * matrix[ 8 ] + matrix[ 12 ];
    vector[ 1 ] = m_val[ 0 ] * matrix[ 1 ] + m_val[ 1 ] * matrix[ 5 ] + m_val[ 2 ] * matrix[ 9 ] + matrix[ 13 ];
    vector[ 2 ] = m_val[ 0 ] * matrix[ 2 ] + m_val[ 1 ] * matrix[ 6 ] + m_val[ 2 ] * matrix[ 10 ] + matrix[ 14 ];
    vector[ 3 ] = m_val[ 0 ] * matrix[ 3 ] + m_val[ 1 ] * matrix[ 7 ] + m_val[ 2 ] * matrix[ 11 ] + matrix[ 15 ];

    m_val[ 0 ] = ( double ) ( vector[ 0 ] );
    m_val[ 1 ] = ( double ) ( vector[ 1 ] );
    m_val[ 2 ] = ( double ) ( vector[ 2 ] );
    m_val[ 3 ] = ( double ) ( vector[ 3 ] );
}

void Vector::transform3( const Matrix& rhs )
{
    double vector[ 4 ];
    const double *matrix = rhs.getMatrix();

    vector[ 0 ] = m_val[ 0 ] * matrix[ 0 ] + m_val[ 1 ] * matrix[ 4 ] + m_val[ 2 ] * matrix[ 8 ];
    vector[ 1 ] = m_val[ 0 ] * matrix[ 1 ] + m_val[ 1 ] * matrix[ 5 ] + m_val[ 2 ] * matrix[ 9 ];
    vector[ 2 ] = m_val[ 0 ] * matrix[ 2 ] + m_val[ 1 ] * matrix[ 6 ] + m_val[ 2 ] * matrix[ 10 ];

    m_val[ 0 ] = ( double ) ( vector[ 0 ] );
    m_val[ 1 ] = ( double ) ( vector[ 1 ] );
    m_val[ 2 ] = ( double ) ( vector[ 2 ] );
    m_val[ 3 ] = 1;
}

void Vector::scale( double scale )
{
   for ( int t = 0; t < 4; t++ )
   {
      m_val[t] *= scale;
   }
}

void Vector::scale3( double scale )
{
   for ( int t = 0; t < 3; t++ )
   {
      m_val[t] *= scale;
   }
}

double Vector::mag()
{
   return sqrt(   m_val[0]*m_val[0]
                + m_val[1]*m_val[1]
                + m_val[2]*m_val[2]
                + m_val[3]*m_val[3] );
}

double Vector::mag3()
{
   return sqrt(   m_val[0]*m_val[0]
                + m_val[1]*m_val[1]
                + m_val[2]*m_val[2] );
}

void Vector::normalize()
{
   double m = mag();
   for ( int t = 0; t < 4; t++ )
   {
      m_val[t] = m_val[t] / m;
   }
}

void Vector::normalize3()
{
   double m = mag3();
   for ( int t = 0; t < 3; t++ )
   {
      m_val[t] = m_val[t] / m;
   }
}

double Vector::dot3( const Vector & rhs )
{
   return(   m_val[0] * rhs.m_val[0]
           + m_val[1] * rhs.m_val[1]
           + m_val[2] * rhs.m_val[2] );
}

Vector Vector::cross3( const Vector & rhs ) const
{
   Vector rval;

   rval.m_val[0] = m_val[1]*rhs.m_val[2] - rhs.m_val[1]*m_val[2];
   rval.m_val[1] = m_val[2]*rhs.m_val[0] - rhs.m_val[2]*m_val[0];
   rval.m_val[2] = m_val[0]*rhs.m_val[1] - rhs.m_val[0]*m_val[1];

   return rval;
}

double & Vector::operator[]( int index )
{
   return m_val[index];
}

const double & Vector::operator[]( int index ) const
{
   return m_val[index];
}

Vector Vector::operator+=( const Vector & rhs )
{
   this->m_val[0] += rhs.m_val[0];
   this->m_val[1] += rhs.m_val[1];
   this->m_val[2] += rhs.m_val[2];
   this->m_val[3] += rhs.m_val[3];
   return *this;
}

Vector Vector::operator-=( const Vector & rhs )
{
   this->m_val[0] -= rhs.m_val[0];
   this->m_val[1] -= rhs.m_val[1];
   this->m_val[2] -= rhs.m_val[2];
   this->m_val[3] -= rhs.m_val[3];
   return *this;
}

Quaternion::Quaternion( const double * val )
{
   if ( val )
   {
      m_val[0] = val[0];
      m_val[1] = val[1];
      m_val[2] = val[2];
      m_val[3] = val[3];
   }
   else
   {
      m_val[0] = 0.0;
      m_val[1] = 0.0;
      m_val[2] = 0.0;
      m_val[3] = 1.0;
   }
}

Quaternion::Quaternion( const Vector & val )
{
   m_val[0] = val[0];
   m_val[1] = val[1];
   m_val[2] = val[2];
   m_val[3] = val[3];
}

Quaternion::~Quaternion()
{
}

void Quaternion::set( int c, double val )
{
   m_val[ c ] = val;
}

void Quaternion::show() const
{
   for ( int c = 0; c < 4; c++ )
   {
      printf( "%.2f ", m_val[c] );
   }
   printf( "\n" );
}

void Quaternion::setEulerAngles( const double * radians )
{
   double axis[4];

   Quaternion x;
   axis[0] = 1.0;
   axis[1] = 0.0;
   axis[2] = 0.0;
   x.setRotationOnAxis( axis, radians[0] );

   Quaternion y;
   axis[0] = 0.0;
   axis[1] = 1.0;
   axis[2] = 0.0;
   y.setRotationOnAxis( axis, radians[1] );

   Quaternion z;
   axis[0] = 0.0;
   axis[1] = 0.0;
   axis[2] = 1.0;
   z.setRotationOnAxis( axis, radians[2] );

   z = z * y;
   z = z * x;
   *this = z;
}

void Quaternion::setRotationOnAxis( const double * axis, double radians )
{
   if ( axis && ::mag3( axis ) > 0.00001 )
   {
      for ( int t = 0; t < 3; t++ )
      {
         m_val[t] = axis[t];
      }
      m_val[3] = 0;
      normalize();

      double a = sin( radians / 2 );
      double b = cos( radians / 2 );

      m_val[0] *= a;
      m_val[1] *= a;
      m_val[2] *= a;
      m_val[3]  = b;
   }
}

void Quaternion::setRotationToPoint( const Vector & face, const Vector & point )
{
   setRotationToPoint( face.get(0), face.get(1), face.get(2), 
         point.get(0), point.get(1), point.get(2) );
}

void Quaternion::setRotationToPoint( const double & faceX, const double & faceY, const double & faceZ,
      const double & pointX, const double & pointY, const double & pointZ )
{
   Vector v1;
   Vector v2;

   v1.set( 0, faceX );   // Facing
   v1.set( 1, faceY );   // Facing
   v1.set( 2, faceZ );   // Facing

   v2.set( 0, pointX );   // Want to face
   v2.set( 1, pointY );   // Want to face
   v2.set( 2, pointZ );   // Want to face

   v1.normalize3();
   v2.normalize3();

   float anglec = v1.get(0) * v2.get(0) + v1.get(1) * v2.get(1) + v1.get(2) * v2.get(2);
   double angle  = acos( anglec );

   // Plane equation: (given three points)
   //
   //     A = y1 (z2 - z3) + y2 (z3 - z1) + y3 (z1 - z2)
   //     B = z1 (x2 - x3) + z2 (x3 - x1) + z3 (x1 - x2)
   //     C = x1 (y2 - y3) + x2 (y3 - y1) + x3 (y1 - y2)
   //   - D = x1 (y2 z3 - y3 z2) + x2 (y3 z1 - y1 z3) + x3 (y1 z2 - y2 z1)

   float A = v1.get(1) * v2.get(2) - v2.get(1) * v1.get(2);
   float B = v1.get(2) * v2.get(0) - v2.get(3) * v1.get(0);
   float C = v1.get(0) * v2.get(1) - v2.get(0) * v1.get(1);

   Vector normal;
   normal.set( 0, A );
   normal.set( 1, B );
   normal.set( 2, C );
   normal.normalize3();

   setRotationOnAxis( normal.getVector(), angle );
}

void Quaternion::getRotationOnAxis( double * axis, double & radians )
{
   normalize();

   double cos_a = m_val[3];
   radians = acos( cos_a ) * 2;

   double sin_a = sqrt( 1.0 - cos_a * cos_a );
   if ( fabs( sin_a ) < 0.0005 )
   {
      sin_a = 1.0;
   }

   axis[0] = m_val[0] / sin_a;
   axis[1] = m_val[1] / sin_a;
   axis[2] = m_val[2] / sin_a;
}

void Quaternion::normalize()
{
   double mag = 0;
   int t;
   for ( t = 0; t < 4; t++ )
   {
      mag += m_val[t] * m_val[t];
   }

   mag = sqrt( mag );

   for ( t = 0; t < 4; t++ )
   {
      m_val[t] = m_val[t] / mag;
   }
}

double distance ( const Vector & v1, const Vector & v2 )
{
   double xDiff = v2.get(0) - v1.get(0);
   double yDiff = v2.get(1) - v1.get(1);
   double zDiff = v2.get(2) - v1.get(2);

   return sqrt( xDiff*xDiff + yDiff*yDiff + zDiff*zDiff );
}

double distance ( const double * v1, const double * v2 )
{
   double xDiff = v2[0] - v1[0];
   double yDiff = v2[1] - v1[1];
   double zDiff = v2[2] - v1[2];

   return sqrt( xDiff*xDiff + yDiff*yDiff + zDiff*zDiff );
}

