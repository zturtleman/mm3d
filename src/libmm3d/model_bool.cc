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


#include "model.h"

#include "glmath.h"
#include "log.h"
#include "weld.h"

#include <vector>

const double TOLERANCE = 0.00005;
const double ATOLERANCE = 0.00000001;

static bool _coord_equiv( double * ac, double * bc )
{
   double d = distance( ac[0], ac[1], ac[2],
         bc[0], bc[1], bc[2] );

   if ( d < TOLERANCE )
   {
      return true;
   }
   else
   {
      return false;
   }
}

typedef struct _UnionTriangle_t
{
   int tri;
   double norm[3];
   unsigned int v[3];
   double coord[3][3];
   double center[3];
   double tpoint[3];
   double tnorm[3][3];
} UnionTriangleT;
typedef std::list< UnionTriangleT > UnionTriangleList;

typedef enum _IntersectionCheck_e
{
   IC_TryAgain,  // Swap A and B and try again
   IC_No,        // No intersection found
   IC_Yes,       // Intersection found
   IC_MAX
} IntersectionCheckE;

// This function returns the angle on pivot from point p1 to p2
static double _angleToPoint( double * pivot, double * p1, double * p2 )
{
   double vec1[3];
   double vec2[3];

   vec1[0] = p1[0] - pivot[0];
   vec1[1] = p1[1] - pivot[1];
   vec1[2] = p1[2] - pivot[2];

   vec2[0] = p2[0] - pivot[0];
   vec2[1] = p2[1] - pivot[1];
   vec2[2] = p2[2] - pivot[2];

   normalize3( vec1 );
   normalize3( vec2 );

   double d = dot3( vec1, vec2 );
   if ( d > (1.0 - ATOLERANCE) )
   {
      return 0.0;
   }
   else
   {
      return acos( d );
   }
}

// returns:
//    1 = in front
//    0 = on plane
//   -1 = in back
static int _pointInPlane( double * coord, double * triCoord, double * triNorm )
{
   double btoa[3];

   for ( int j = 0; j < 3; j++ )
   {
      btoa[j] = coord[j] - triCoord[j];
   }
   normalize3( btoa );

   double c = dot3( btoa, triNorm );

   if ( c < -ATOLERANCE )
      return -1;
   else if ( c > ATOLERANCE )
      return 1;
   else
      return 0;
}

// returns:
//    1 = inside triangle
//    0 = on triangle edge
//   -1 = outside triangle
static int _pointInTriangle( double * coord, UnionTriangleT & tri )
{
   int vside[3];

   int i;
   for ( i = 0; i < 3; i++ )
   {
      vside[i] = _pointInPlane( coord, tri.tpoint, tri.tnorm[i] );
   }

   if ( vside[0] < 0 && vside[1] < 0 && vside[2] < 0 )
   {
      return 1;
   }
   if ( vside[0] > 0 && vside[1] > 0 && vside[2] > 0 )
   {
      return 1;
   }
   if ( vside[0] == 0 && vside[1] == vside[2] )
   {
      return 0;
   }
   if ( vside[1] == 0 && vside[0] == vside[2] )
   {
      return 0;
   }
   if ( vside[2] == 0 && vside[0] == vside[1] )
   {
      return 0;
   }
   if ( vside[0] == 0 && vside[1] == 0 )
   {
      return 0;
   }
   if ( vside[0] == 0 && vside[2] == 0 )
   {
      return 0;
   }
   if ( vside[1] == 0 && vside[2] == 0 )
   {
      return 0;
   }
   return -1;
}

// returns:
//    ipoint = coordinates of edge line and plane
//    bool   = whether or not ipoint is between p1 and p2
static bool _findEdgePlaneIntersection( double * ipoint, double * p1, double * p2, 
      double * triCoord, double * triNorm, bool checkRange = true )
{
   double edgeVec[3];
   edgeVec[0] = p2[0] - p1[0];
   edgeVec[1] = p2[1] - p1[1];
   edgeVec[2] = p2[2] - p1[2];

   /*
   log_debug( "finding intersection with plane with line from %f,%f,%f to %f,%f,%f\n",
         p1[0], p1[1], p1[2], 
         p2[0], p2[1], p2[2] );
   */

   double a =   dot3( edgeVec, triNorm );
   double d = - dot3( triCoord, triNorm );

   // prevent divide by zero
   if ( fabs(a) < TOLERANCE )
   {
      // edge is parallel to plane, the value of ipoint is undefined
      return false;
   }

   // distance along edgeVec p1 to plane
   double dist = -(d + dot3( p1, triNorm )) / a;

   // dist is % of velocity vector, scale to get impact point
   edgeVec[0] *= dist;
   edgeVec[1] *= dist;
   edgeVec[2] *= dist;

   ipoint[0] = p1[0] + edgeVec[0];
   ipoint[1] = p1[1] + edgeVec[1];
   ipoint[2] = p1[2] + edgeVec[2];

   if ( checkRange )
   {
      // Make sure intersection point is between p1 and p2
      if ( dist >= (0.0 - TOLERANCE)
            && dist <= (1.0 + TOLERANCE) )
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      // Distance is irrelevant, we just want to know where 
      // the intersection is (as long as it's not behind the triangle)
      if ( dist >= (0.0 - TOLERANCE) )
      {
         return true;
      }
      else
      {
         return false;
      }
   }
}

static void _initUnionTriangle( Model * model, UnionTriangleT & ut, int triIndex )
{
   ut.tri = triIndex;
   model->getTriangleVertices( triIndex, ut.v[0], ut.v[1], ut.v[2] );

   int i;
   for ( i = 0; i < 3; i++ )
   {
      model->getVertexCoords( ut.v[i], ut.coord[i] );
   }
   calculate_normal( ut.norm,
         ut.coord[0], ut.coord[1], ut.coord[2] );

   // Find center point and tent point (for inside-triangle test)
   for ( i = 0; i < 3; i++ )
   {
      ut.center[i] = 
         (  ut.coord[0][i] 
            + ut.coord[1][i] 
            + ut.coord[2][i] ) / 3.0;

      ut.tpoint[i] = ut.center[i] + ut.norm[i];
   }

   // Find normals for tent triangles (for inside-triangle test)
   calculate_normal( ut.tnorm[0], 
         ut.tpoint, ut.coord[0], ut.coord[1] );
   calculate_normal( ut.tnorm[1], 
         ut.tpoint, ut.coord[1], ut.coord[2] );
   calculate_normal( ut.tnorm[2], 
         ut.tpoint, ut.coord[2], ut.coord[0] );
}

static bool _isValidTriangle( Model * model, int v1, int v2, int v3 )
{
   double coord[3][3];

   model->getVertexCoords( v1, coord[0] );
   model->getVertexCoords( v2, coord[1] );
   model->getVertexCoords( v3, coord[2] );

   if ( _coord_equiv( coord[0], coord[1] ) )
   {
      return false;
   }
   if ( _coord_equiv( coord[0], coord[2] ) )
   {
      return false;
   }
   if ( _coord_equiv( coord[1], coord[2] ) )
   {
      return false;
   }

   double aval1;
   double aval2;
   double aval3;
   
   aval1 = fabs(_angleToPoint( coord[0], coord[1], coord[2] ));
   aval2 = fabs(_angleToPoint( coord[1], coord[0], coord[2] ));
   aval3 = fabs(_angleToPoint( coord[2], coord[0], coord[1] ));

   if ( aval1 < TOLERANCE && aval2 < TOLERANCE )
   {
      return false;
   }

   if ( aval1 < TOLERANCE && aval3 < TOLERANCE)
   {
      return false;
   }

   if ( aval2 < TOLERANCE && aval3 < TOLERANCE)
   {
      return false;
   }

   return true;
}

typedef struct _NewTriangle_t
{
   int v[3];
} NewTriangleT;
typedef std::list< NewTriangleT > NewTriangleList;

static void _cutTriangle( Model * model, UnionTriangleList & utl,
      UnionTriangleT & ut, UnionTriangleT & utCut, double * p1, double * p2 )
{
   log_debug( "cutting triangle %d\n", ut.tri );

   int onVertex = 0;
   for ( int n = 0; n < 3; n++ )
   {
      if ( _coord_equiv( ut.coord[n], p1 ) )
      {
         onVertex++;
      }
      else if ( _coord_equiv( ut.coord[n], p2 ) )
      {
         onVertex++;
      }
   }

   //log_debug( "onVertex = %d\n", onVertex );

   double a1 = fabs( _angleToPoint( ut.coord[0], p1, ut.coord[1] ) );
   double a2 = fabs( _angleToPoint( ut.coord[0], p2, ut.coord[1] ) );

   if ( (a1 == 0 && a2 == 0) )
   {
      // If we're a perfectly straight line from v0 to v1 then
      // our intersection line is on our edge. No need to cut.
      a1 = a2;
      return;
   }

   if ( _coord_equiv( ut.coord[0], p1 ) )
   {
      a1 = 0.0;
   }
   if ( _coord_equiv( ut.coord[0], p2 ) )
   {
      a2 = 0.0;
   }

   double nc1[3];
   double nc2[3];

   if ( a1 < a2 )
   {
      nc1[0] = p1[0];
      nc1[1] = p1[1];
      nc1[2] = p1[2];
      nc2[0] = p2[0];
      nc2[1] = p2[1];
      nc2[2] = p2[2];
   }
   else
   {
      nc1[0] = p2[0];
      nc1[1] = p2[1];
      nc1[2] = p2[2];
      nc2[0] = p1[0];
      nc2[1] = p1[1];
      nc2[2] = p1[2];
   }

   double norm[3];
   norm[0] = ut.norm[0];
   norm[1] = ut.norm[1];
   norm[2] = ut.norm[2];

   int v1 = model->addVertex( nc1[0], nc1[1], nc1[2] );
   int v2 = model->addVertex( nc2[0], nc2[1], nc2[2] );

   NewTriangleT nt;
   NewTriangleList ntl;

   nt.v[0] = ut.v[0];
   nt.v[1] = v1;
   nt.v[2] = v2;
   ntl.push_back( nt );
   
   nt.v[0] = ut.v[0];
   nt.v[1] = ut.v[1];
   nt.v[2] = v1;
   ntl.push_back( nt );
   
   nt.v[0] = ut.v[0];
   nt.v[1] = v2;
   nt.v[2] = ut.v[2];
   ntl.push_back( nt );

   double newnorm[3] = {1,0,0};
   calculate_normal( newnorm, ut.coord[2], nc2, nc1 );
   
   if ( dot3( norm, newnorm ) > 0.0 )
   {
      nt.v[0] = ut.v[2];
      nt.v[1] = v2;
      nt.v[2] = v1;
      ntl.push_back( nt );

      nt.v[0] = ut.v[2];
      nt.v[1] = v1;
      nt.v[2] = ut.v[1];
      ntl.push_back( nt );
   }
   else
   {
      nt.v[0] = ut.v[1];
      nt.v[1] = v2;
      nt.v[2] = v1;
      ntl.push_back( nt );

      // reversed v[1] and v[2] (appears to behave correctly now)
      nt.v[0] = ut.v[2];
      nt.v[1] = v2;
      nt.v[2] = ut.v[1];
      ntl.push_back( nt );
   }

   bool setUt = true;

   int group = model->getTriangleGroup( ut.tri );

   // now add the remaining triangles to the list
   NewTriangleList::iterator it;
   for ( it = ntl.begin(); it != ntl.end(); it++ )
   {
      if ( _isValidTriangle( model, 
               (*it).v[0], (*it).v[1], (*it).v[2] ) )
      {
         if ( setUt )
         {
            setUt = false;
            model->setTriangleVertices( ut.tri,
               (*it).v[0], (*it).v[1], (*it).v[2] );
            _initUnionTriangle( model, ut, ut.tri );
         }
         else
         {
            int tri = model->addTriangle( (*it).v[0], (*it).v[1], (*it).v[2] );
            if ( group >= 0 )
            {
               model->addTriangleToGroup( group, tri );
            }

            UnionTriangleT nut;
            _initUnionTriangle( model, nut, tri );
            utl.push_back( nut );
         }
      }
   }

   if ( setUt )
   {
      log_error( "No valid triangles in any splits\n" );
   }
}

static IntersectionCheckE _findIntersection( Model * model, 
      UnionTriangleList & la, UnionTriangleList & lb,
      UnionTriangleT & a, UnionTriangleT & b )
{
   //log_debug( "checking %d against %d\n", a.tri, b.tri );
   int i;
   int j;
   int sharedVertices = 0;
   int coPlanar = 0;
   int vside[3]; // on which side of B is each A vertex?

   /*
   for ( i = 0; i < 3; i++ )
   {
      log_debug( "  point %d: %f,%f,%f\n",
            a.tri, a.coord[i][0], a.coord[i][1], a.coord[i][2] );
   }
   for ( i = 0; i < 3; i++ )
   {
      log_debug( "  point %d: %f,%f,%f\n",
            b.tri, b.coord[i][0], b.coord[i][1], b.coord[i][2] );
   }
   */

   bool inFront = false;
   bool inBack  = false;

   for ( i = 0; i < 3; i++ )
   {
      for ( j = 0; j < 3; j++ )
      {
         // If the triangles share a vertex or if the vertices
         // are at the same point in space, they are considered "shared"
         if ( a.v[i] == b.v[j] 
               || _coord_equiv( a.coord[i], b.coord[j] ) )
         {
            sharedVertices++;
         }
      }
   }

   if ( sharedVertices == 3 )
   {
      // triangles completely overlap each other
      // no splits required
      return IC_No;
   }

   // Calculate which side of B each A vertex is on
   for ( i = 0; i < 3; i++ )
   {
      double btoa[3];

      for ( j = 0; j < 3; j++ )
      {
         btoa[j] = a.coord[i][j] - b.coord[i][j];
      }
      normalize3( btoa );

      double c = dot3( btoa, b.norm );

      if ( c < -TOLERANCE )
      {
         inBack = true;
         vside[i] = -1;
      }
      else if ( c > TOLERANCE )
      {
         inFront = true;
         vside[i] = 1;
      }
      else
      {
         coPlanar++;
         vside[i] = 0;
      }
   }

   /*
   log_debug( "  shared=%d coPlanar=%d front=%s back=%s\n",
         sharedVertices, coPlanar,
         (inFront ? "yes" : "no"),
         (inBack ? "yes" : "no") );
   */

   if ( sharedVertices == 2 && (inFront || inBack) )
   {
      // triangles share an edge but are not co-planar
      // no splits required
      return IC_No;
   }

   // Are A and B co-planar?
   if ( coPlanar == 3 )
   {
      // co-planar faces should only matter
      // if other non-coplanar faces intersect along the same edge (for face
      // removal purposes)

      // NOTE: My testing seems to confirm this, but there may be
      // cases where this is not true. If co-planar faces are not
      // getting split correctly this case may be the cause.

      // They are on the same plane but do not overlap
      // (but B may be inside A, swap and try again)
      //return IC_TryAgain;

      return IC_No;
   }

   // Does A go through B's plane?
   // (consider co-planar to go "through")
   if ( coPlanar == 2 || (inFront && inBack) )
   {
      // Yes, does A go through B itself?

      int oddSide = 0; // default to 0, correct below
      int far1 = 1;
      int far2 = 2;

      if ( vside[1] != 0 && vside[1] != vside[0] && vside[1] != vside[2] )
      {
         oddSide = 1;
         far1 = 0;
         far2 = 2;
      }
      else if ( vside[2] != 0 && vside[2] != vside[0] && vside[2] != vside[1] )
      {
         oddSide = 2;
         far1 = 0;
         far2 = 1;
      }

      double ipoint1[3];
      double ipoint2[3];

      bool   hit1 = _findEdgePlaneIntersection( ipoint1, 
            a.coord[oddSide], a.coord[far1], b.coord[0], b.norm );
      bool   hit2 = _findEdgePlaneIntersection( ipoint2, 
            a.coord[oddSide], a.coord[far2], b.coord[0], b.norm );

      // hit1 and hit2 indicate that the edge crosses the plane
      // the follow up is to see if the intersection is within the triangle
      // On the triangle's edge counts as "inside"
      if ( hit1 )
      {
         hit1 = (_pointInTriangle( ipoint1, b ) >= 0);
         //log_debug( "  intersects plane at %f,%f,%f (%s)\n", ipoint1[0], ipoint1[1], ipoint1[2], (hit1 ? "yes" : "no") );
      }
      if ( hit2 )
      {
         hit2 = (_pointInTriangle( ipoint2, b ) >= 0 );
         //log_debug( "  intersects plane at %f,%f,%f (%s)\n", ipoint2[0], ipoint2[1], ipoint2[2], (hit2 ? "yes" : "no") );
      }

      if ( hit1 || hit2 )
      {
         // If the only intersection was on the plane's edge, 
         // don't count it as an intersection
         if ( hit1 != hit2 )
         {
            //log_debug( "  one hit\n" );
            if ( hit1 )
            {
               //int i = _pointInTriangle( ipoint1, b );
               //log_debug( "  in triangle: %d\n", i );
               if ( _pointInTriangle( ipoint1, b ) == 0 )
               {
                  // One intersection, on edge, bail
                  return IC_TryAgain;
               }
            }
            if ( hit2 )
            {
               //int i = _pointInTriangle( ipoint2, b );
               //log_debug( "  in triangle: %d\n", i );
               if ( _pointInTriangle( ipoint2, b ) == 0 )
               {
                  // One intersection, on edge, bail
                  return IC_TryAgain;
               }
            }
         }

         if ( hit1 && hit2 )
         {
            _cutTriangle( model, la, a, b, ipoint1, ipoint2 );
            _cutTriangle( model, lb, b, a, ipoint1, ipoint2 );
         }
         else
         {
            // find point where edge leaves B and change ipoint to that point
            double epoint[3];
            if ( !hit1 )
            {
               for ( int i = 0; !hit1 && i < 3; i++ )
               {
                  hit1 = _findEdgePlaneIntersection( epoint, 
                        ipoint1, ipoint2, b.tpoint, b.tnorm[i] );
               }
               memcpy( ipoint1, epoint, sizeof(epoint) );
            }
            if ( !hit2 )
            {
               for ( int i = 0; !hit2 && i < 3; i++ )
               {
                  hit2 = _findEdgePlaneIntersection( epoint, 
                        ipoint1, ipoint2, b.tpoint, b.tnorm[i] );
               }
               memcpy( ipoint2, epoint, sizeof(epoint) );
            }

            _cutTriangle( model, la, a, b, ipoint1, ipoint2 );
            _cutTriangle( model, lb, b, a, ipoint1, ipoint2 );
         }
         return IC_Yes;
      }

      // No, A edges go through B's plane, but not B itself,
      // but B edges may go through A, swap and try again
      return IC_TryAgain;
   }

   // No, A is entirely on one side of B's plane
   return IC_No;
}

static void _buildUnionTriangleList( Model * model,
      UnionTriangleList & buildList,
      std::list<int> & sourceList )
{
   buildList.clear();

   // Creation union triangle lists for intersection tests
   std::list<int>::iterator it;
   for ( it = sourceList.begin(); it != sourceList.end(); it++ )
   {
      UnionTriangleT ut;
      _initUnionTriangle( model, ut, *it );
      buildList.push_back( ut );
   }
}

static void _findNearTriangles( Model * model, UnionTriangleT & ut, UnionTriangleList & lb, int & coplanar, int & front )
{
   coplanar = -1;
   front    = -1;

   double dist = 0.0;

   double ipoint[3];

   UnionTriangleList::iterator it;
   UnionTriangleList::iterator save_it;

   for ( it = lb.begin(); it != lb.end(); it++ )
   {
      if ( _findEdgePlaneIntersection( ipoint, 
            ut.center, ut.tpoint, (*it).center, (*it).norm, false ) )
      {
         //log_debug( "found line/plane intersection\n" );
         if ( _pointInTriangle( ipoint, (*it) ) >= 0 )
         {
            //log_debug( "  intersection is in far triangle\n" );
            if ( _pointInPlane( ipoint, ut.center, ut.norm ) == 0 )
            {
               //log_debug( "    intersection is in self (co planar)\n" );
               coplanar = (*it).tri;
            }
            else
            {
               double d = mag3( ipoint );
               if ( front < 0 || d < dist )
               {
                  //log_debug( "    found new nearest front triangle\n" );
                  front = (*it).tri;
                  dist = d;
                  save_it = it;
               }
            }
         }
      }
   }

   if ( front >= 0 )
   {
      if ( _pointInPlane( ut.center, (*save_it).center, (*save_it).norm ) > 0 )
      {
         //log_debug( "    front triangle faces us, we're outside\n" );

         // our triangle is in front of an outward facing triangle,
         // so it is not inside another shape
         front = -1;
      }
   }
}

static void _removeInternalTriangles( Model * model, UnionTriangleList & la, UnionTriangleList & lb )
{
   std::list<int> removeList;
   std::list<int> removeList2;

   int coplanar;
   int front;

   UnionTriangleList::iterator it;
   for ( it = la.begin(); it != la.end(); it++ )
   {
      _findNearTriangles( model, *it, lb, coplanar, front );

      if ( front >= 0 )
      {
         //log_debug( "A triangle %d is on the inside\n", (*it).tri );
         removeList.push_back( (*it).tri );
      }
   }

   for ( it = lb.begin(); it != lb.end(); it++ )
   {
      _findNearTriangles( model, *it, la, coplanar, front );

      if ( front >= 0 || coplanar >= 0 )
      {
         //log_debug( "B triangle %d is on the inside\n", (*it).tri );
         removeList2.push_back( (*it).tri );
      }
   }

   // Must delete in reverse order because deleting a triangle invalidates
   // any index that comes after it.
   int a = -1;
   int b = -1;
   while ( !removeList.empty() || !removeList2.empty() )
   {
      a = -1;
      b = -1;

      if ( !removeList.empty() )
      {
         a = removeList.back();
      }
      if ( !removeList2.empty() )
      {
         b = removeList2.back();
      }
      
      if ( a > b )
      {
         model->deleteTriangle( a );
         removeList.pop_back();
      }
      else
      {
         model->deleteTriangle( b );
         removeList2.pop_back();
      }
   }

   model->deleteOrphanedVertices();
}

static void _removeExternalTriangles( Model * model, UnionTriangleList & la, UnionTriangleList & lb )
{
   std::list<int> removeList;
   std::list<int> removeList2;

   int coplanar;
   int front;

   UnionTriangleList::iterator it;
   for ( it = la.begin(); it != la.end(); it++ )
   {
      _findNearTriangles( model, *it, lb, coplanar, front );

      if ( front < 0 )
      {
         //log_debug( "A triangle %d is on the inside\n", (*it).tri );
         removeList.push_back( (*it).tri );
      }
   }

   for ( it = lb.begin(); it != lb.end(); it++ )
   {
      _findNearTriangles( model, *it, la, coplanar, front );

      if ( front < 0 || coplanar >= 0 )
      {
         //log_debug( "B triangle %d is on the inside\n", (*it).tri );
         removeList2.push_back( (*it).tri );
      }
   }

   // Must delete in reverse order because deleting a triangle invalidates
   // any index that comes after it.
   int a = -1;
   int b = -1;
   while ( !removeList.empty() || !removeList2.empty() )
   {
      a = -1;
      b = -1;

      if ( !removeList.empty() )
      {
         a = removeList.back();
      }
      if ( !removeList2.empty() )
      {
         b = removeList2.back();
      }
      
      if ( a > b )
      {
         model->deleteTriangle( a );
         removeList.pop_back();
      }
      else
      {
         model->deleteTriangle( b );
         removeList2.pop_back();
      }
   }

   model->deleteOrphanedVertices();
}

static void _removeSubtractionTriangles( Model * model, UnionTriangleList & la, UnionTriangleList & lb )
{
   std::list<int> invertList;
   std::list<int> removeList;
   std::list<int> removeList2;

   int coplanar;
   int front;

   UnionTriangleList::iterator it;
   for ( it = lb.begin(); it != lb.end(); it++ )
   {
      _findNearTriangles( model, *it, la, coplanar, front );

      if ( coplanar >= 0 )
      {
         removeList.push_back( (*it).tri );
      }
      else
      {
         if ( front >= 0 )
         {
            //log_debug( "B triangle %d is on the inside\n", (*it).tri );
            invertList.push_back( (*it).tri );
         }
         else
         {
            //log_debug( "B triangle %d is on the outside\n", (*it).tri );
            removeList.push_back( (*it).tri );
         }
      }
   }

   for ( it = la.begin(); it != la.end(); it++ )
   {
      _findNearTriangles( model, *it, lb, coplanar, front );

      if ( coplanar >= 0 && front < 0
            || coplanar < 0 && front >= 0 )
      {
         //log_debug( "A triangle %d is on the inside\n", (*it).tri );
         removeList2.push_back( (*it).tri );
      }
   }

   std::list<int>::iterator iit;
   for ( iit = invertList.begin(); iit != invertList.end(); iit++ )
   {
      model->invertNormals( *iit );
   }

   // Must delete in reverse order because deleting a triangle invalidates
   // any index that comes after it.
   int a = -1;
   int b = -1;
   while ( !removeList.empty() || !removeList2.empty() )
   {
      a = -1;
      b = -1;

      if ( !removeList.empty() )
      {
         a = removeList.back();
      }
      if ( !removeList2.empty() )
      {
         b = removeList2.back();
      }
      
      if ( a > b )
      {
         model->deleteTriangle( a );
         removeList.pop_back();
      }
      else
      {
         model->deleteTriangle( b );
         removeList2.pop_back();
      }
   }

   model->deleteOrphanedVertices();
}

void Model::booleanOperation( Model::BooleanOpE op,
      std::list<int> & listA, std::list<int> & listB )
{
   UnionTriangleList la;
   UnionTriangleList lb;

   _buildUnionTriangleList( this, la, listA );
   _buildUnionTriangleList( this, lb, listB );

   UnionTriangleList::iterator a;
   UnionTriangleList::iterator b;

   for ( a = la.begin(); a != la.end(); a++ )
   {
      for ( b = lb.begin(); b != lb.end(); b++ )
      {
         if ( IC_TryAgain == _findIntersection( this, la, lb, *a, *b ) )
         {
            _findIntersection( this, lb, la, *b, *a );
         }
      }
   }

   for ( a = la.begin(); a != la.end(); a++ )
   {
      selectTriangle( (*a).tri );
   }
   for ( b = lb.begin(); b != lb.end(); b++ )
   {
      selectTriangle( (*b).tri );
   }

   weldSelectedVertices( this );

   switch ( op )
   {
      case BO_UnionRemove:
         _removeInternalTriangles( this, la, lb );
         break;
      case BO_Subtraction:
         _removeSubtractionTriangles( this, la, lb );
         break;
      case BO_Intersection:
         _removeExternalTriangles( this, la, lb );
         break;
      default:
         log_error( "Uknown boolean op: %d\n", op );
      case BO_Union:
         break;
   }
}

