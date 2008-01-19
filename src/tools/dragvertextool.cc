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


#include "dragvertextool.h"
#include "pixmap/dragvertextool.xpm"
#include "glmath.h"
#include "model.h"
#include "modelstatus.h"

#include <math.h>
#include <QObject>
#include <QApplication>

#include "log.h"

DragVertexTool::DragVertexTool()
{
}

DragVertexTool::~DragVertexTool()
{
}

void DragVertexTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();

   m_vertVectors.clear();

   std::list<int> vertices;
   model->getSelectedVertices( vertices );

   if ( !vertices.empty() )
   {
      m_vertId = vertices.front();

      m_view = parent->getParentViewMatrix();        // model to viewport space
      m_inv  = parent->getParentViewInverseMatrix(); // viewport to model space

      m_vertCoords[0] = 0;
      m_vertCoords[1] = 0;
      m_vertCoords[2] = 0;
      model->getVertexCoords( m_vertId, m_vertCoords );

      log_debug( "vertex %d is at ( %f, %f, %f )\n", m_vertId,
            m_vertCoords[0], m_vertCoords[1], m_vertCoords[2] );

      m_view.apply3x( m_vertCoords );

      size_t tcount = model->getTriangleCount();
      for ( size_t t = 0; t < tcount; t++ )
      {
         int tv[3];
         int i;

         bool match = false;
         for ( i = 0; i < 3; i++ )
         {
            tv[i] = model->getTriangleVertex( t, i );
            if ( tv[i] == m_vertId )
            {
               // TODO: could find the same vertex multiple times if
               // edge belongs to more than one triangle, may want
               // to prevent that in the future
               match = true;
            }
         }

         if ( match )
         {
            for ( i = 0; i < 3; i++ )
            {
               if ( tv[i] != m_vertId )
               {
                  double c[3]  = { 0, 0, 0 };
                  model->getVertexCoords( tv[i], c );
                  m_view.apply3x( c );

                  Vector v;
                  v[0] = c[0] - m_vertCoords[0];
                  v[1] = c[1] - m_vertCoords[1];
                  v[2] = c[2] - m_vertCoords[2];

                  log_debug( "adding vector ( %f, %f, %f )\n",
                        v[0], v[1], v[2] );

                  m_vertVectors.push_back( v );
               }
            }
         }
      }

      model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Dragging selected vertex" ).toUtf8() );
   }
   else
   {
      m_vertId = -1;
      model_status( parent->getModel(), StatusError, STATUSTIME_LONG, qApp->translate( "Tool", "Must a vertex selected" ).toUtf8() );
   }
}

void DragVertexTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   parent->updateAllViews();

   model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Drag complete" ).toUtf8() );
}

void DragVertexTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   Matrix m;

   Vector newPos;
   parent->getParentXYValue( x, y, newPos[0], newPos[1] );
   Model * model = parent->getModel();

   log_debug( "pos is ( %f, %f, %f )\n",
         newPos[0], newPos[1], newPos[2] );

   newPos[0] -= m_vertCoords[0];
   newPos[1] -= m_vertCoords[1];

   log_debug( "pos vector is ( %f, %f, %f )\n",
         newPos[0], newPos[1], newPos[2] );

   double pscale = newPos.mag3();
   newPos.normalize3();

   log_debug( "mouse has moved %f units\n",
         pscale );

   if ( !m_vertVectors.empty() && m_vertId >= 0 )
   {
      Vector best = m_vertVectors.front();
      double bestDist = 0.0;
      double ratio = 1.0;

      std::list< Vector >::iterator it;

      for ( it = m_vertVectors.begin(); it != m_vertVectors.end(); it++ )
      {
         Vector vtry = (*it);
         vtry[2] = 0;

         double trylen = vtry.mag3();

         vtry.normalize3();
         double d = newPos.dot3( vtry );

         log_debug( "  dot3 is %f (%f, %f, %f)\n", d,
               vtry[0], vtry[1], vtry[2] );

         if ( fabs( d ) > fabs( bestDist ) )
         {
            bestDist = d;
            best = (*it);
            ratio = pscale / trylen;
         }
      }

      log_debug( "best vector is (%f, %f, %f)\n",
            best[0], best[1], best[2] );

      best.scale3( bestDist * ratio );

      log_debug( "best scaled is (%f, %f, %f)\n",
            best[0], best[1], best[2] );

      best[0] += m_vertCoords[0];
      best[1] += m_vertCoords[1];
      best[2] += m_vertCoords[2];

      log_debug( "best sum is (%f, %f, %f)\n",
            best[0], best[1], best[2] );

      m_inv.apply3x( best );

      log_debug( "best applied is (%f, %f, %f)\n",
            best[0], best[1], best[2] );

      model->moveVertex( m_vertId, best[0], best[1], best[2] );
   }

   parent->updateAllViews();
}

const char ** DragVertexTool::getPixmap()
{
   return (const char **) dragvertextool_xpm;
}

void DragVertexTool::activated( int argc, Model * model, QMainWindow * mainwin )
{
   //model_status( model, StatusNormal, STATUSTIME_NONE, qApp->translate( "Tool", "Tip: Hold shift to restrict movement to one dimension" ).toUtf8() );
}

const char * DragVertexTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Drag Vertex on Edge" );
}

