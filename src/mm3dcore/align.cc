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

#include "align.h"

#include "model.h"
#include "log.h"

void alignSelectedX( Model * m, AlignTypeE at, double xval )
{
   double xmin = 0.0;
   double xmax = 0.0;
   double ymin = 0.0;
   double ymax = 0.0;
   double zmin = 0.0;
   double zmax = 0.0;

   double diff = 0.0;

   m->getSelectedBoundingRegion( &xmin, &ymin, &zmin, &xmax, &ymax, &zmax );
   log_debug( "selected bounding region is (%f,%f,%f) - (%f, %f, %f)\n",
         xmin, ymin, zmin, xmax, ymax, zmax );

   switch ( at )
   {
      case AT_Center:
         diff = xval - ((xmax + xmin) / 2.0);
         break;
      case AT_Min:
         diff = xval - xmin; 
         break;
      case AT_Max:
         diff = xval - xmax; 
         break;
      default:  // Bzzt, thanks for playing
         log_error( "bad align argument: %d\n", (int) at );
         return;
   }

   log_debug( "align difference is %f\n", diff );

   unsigned count;
   unsigned p;
   double coords[3] = { 0.0, 0.0, 0.0 };

   count = m->getVertexCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isVertexSelected( p ) )
      {
         m->getVertexCoords( p, coords );
         coords[0] += diff;
         m->moveVertex( p, coords[0], coords[1], coords[2] );
      }
   }
   count = m->getBoneJointCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isBoneJointSelected( p ) )
      {
         m->getBoneJointCoords( p, coords );
         coords[0] += diff;
         m->moveBoneJoint( p, coords[0], coords[1], coords[2] );
      }
   }
   count = m->getPointCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isPointSelected( p ) )
      {
         m->getPointCoords( p, coords );
         coords[0] += diff;
         m->movePoint( p, coords[0], coords[1], coords[2] );
      }
   }
}

void alignSelectedY( Model * m, AlignTypeE at, double yval )
{
   double xmin = 0.0;
   double xmax = 0.0;
   double ymin = 0.0;
   double ymax = 0.0;
   double zmin = 0.0;
   double zmax = 0.0;

   double diff = 0.0;

   m->getSelectedBoundingRegion( &xmin, &ymin, &zmin, &xmax, &ymax, &zmax );
   log_debug( "selected bounding region is (%f,%f,%f) - (%f, %f, %f)\n",
         xmin, ymin, zmin, xmax, ymax, zmax );

   switch ( at )
   {
      case AT_Center:
         diff = yval - ((ymax + ymin) / 2.0);
         break;
      case AT_Min:
         diff = yval - ymin; 
         break;
      case AT_Max:
         diff = yval - ymax; 
         break;
      default:  // Bzzt, thanks for playing
         log_error( "bad align argument: %d\n", (int) at );
         return;
   }

   log_debug( "align difference is %f\n", diff );

   unsigned count;
   unsigned p;
   double coords[3] = { 0.0, 0.0, 0.0 };

   count = m->getVertexCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isVertexSelected( p ) )
      {
         m->getVertexCoords( p, coords );
         coords[1] += diff;
         m->moveVertex( p, coords[0], coords[1], coords[2] );
      }
   }
   count = m->getBoneJointCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isBoneJointSelected( p ) )
      {
         m->getBoneJointCoords( p, coords );
         coords[1] += diff;
         m->moveBoneJoint( p, coords[0], coords[1], coords[2] );
      }
   }
   count = m->getPointCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isPointSelected( p ) )
      {
         m->getPointCoords( p, coords );
         coords[1] += diff;
         m->movePoint( p, coords[0], coords[1], coords[2] );
      }
   }
}

void alignSelectedZ( Model * m, AlignTypeE at, double zval )
{
   double xmin = 0.0;
   double xmax = 0.0;
   double ymin = 0.0;
   double ymax = 0.0;
   double zmin = 0.0;
   double zmax = 0.0;

   double diff = 0.0;

   m->getSelectedBoundingRegion( &xmin, &ymin, &zmin, &xmax, &ymax, &zmax );
   log_debug( "selected bounding region is (%f,%f,%f) - (%f, %f, %f)\n",
         xmin, ymin, zmin, xmax, ymax, zmax );

   switch ( at )
   {
      case AT_Center:
         diff = zval - ((zmax + zmin) / 2.0);
         break;
      case AT_Min:
         diff = zval - zmin; 
         break;
      case AT_Max:
         diff = zval - zmax; 
         break;
      default:  // Bzzt, thanks for playing
         log_error( "bad align argument: %d\n", (int) at );
         return;
   }

   log_debug( "align difference is %f\n", diff );

   unsigned count;
   unsigned p;
   double coords[3] = { 0.0, 0.0, 0.0 };

   count = m->getVertexCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isVertexSelected( p ) )
      {
         m->getVertexCoords( p, coords );
         coords[2] += diff;
         m->moveVertex( p, coords[0], coords[1], coords[2] );
      }
   }
   count = m->getBoneJointCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isBoneJointSelected( p ) )
      {
         m->getBoneJointCoords( p, coords );
         coords[2] += diff;
         m->moveBoneJoint( p, coords[0], coords[1], coords[2] );
      }
   }
   count = m->getPointCount();
   for ( p = 0; p < count; p++ )
   {
      if ( m->isPointSelected( p ) )
      {
         m->getPointCoords( p, coords );
         coords[2] += diff;
         m->movePoint( p, coords[0], coords[1], coords[2] );
      }
   }
}

