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


#include "rotatetool.h"

#include "model.h"
#include "pixmap/rotatetool.xpm"
#include "glmath.h"
#include "decalmgr.h"
#include "rotatepoint.h"
#include "log.h"
#include "modelstatus.h"

#include <math.h>
#include <qobject.h>
#include <qapplication.h>

static void _add_coords( double * dest, double * rhs, double * lhs )
{
   dest[0] = rhs[0] + lhs[0];
   dest[1] = rhs[1] + lhs[1];
   dest[2] = rhs[2] + lhs[2];
}


RotateTool::RotateTool()
   : m_model( NULL ),
     m_tracking( false ),
     m_widget( NULL )
{
}

RotateTool::~RotateTool()
{
}

void RotateTool::setModel( Model * model )
{
   m_model = model;
}

void RotateTool::activated( int arg, Model * model, QMainWindow * mainwin )
{
   m_model = model;

   model_status( model, StatusNormal, STATUSTIME_NONE, qApp->translate( "Tool", "Tip: Hold shift to rotate in 15 degree increments" ).utf8() );

   m_coords[0] = 0;
   m_coords[1] = 0;
   m_coords[2] = 0;
   m_coords[3] = 1;

   if ( model->getAnimationMode() == Model::ANIMMODE_SKELETAL )
   {
      list<int> joints;
      model->getSelectedBoneJoints( joints );
      if ( joints.size() > 0 )
      {
         model->getBoneJointCoords( joints.front(), m_coords );
      }
   }
   else
   {
      double coords[3];

      unsigned count = 0;

      unsigned vcount = model->getVertexCount();

      for ( unsigned int v = 0; v < vcount; v++ )
      {
         if ( model->isVertexSelected( v ) )
         {
            model->getVertexCoords( v, coords );
            _add_coords( m_coords, m_coords, coords );
            count++;
         }
      }

      unsigned int bcount = model->getBoneJointCount();
      for ( unsigned int b = 0; b < bcount; b++ )
      {
         if ( model->isBoneJointSelected( b ) )
         {
            model->getBoneJointCoords( b, coords );
            _add_coords( m_coords, m_coords, coords );
            count++;
         }
      }

      unsigned int pcount = model->getPointCount();
      for ( unsigned int p = 0; p < pcount; p++ )
      {
         if ( model->isPointSelected( p ) )
         {
            model->getPointTranslation( p, coords );
            _add_coords( m_coords, m_coords, coords );
            count++;
         }
      }

      unsigned int rcount = model->getProjectionCount();
      for ( unsigned int r = 0; r < rcount; r++ )
      {
         if ( model->isProjectionSelected( r ) )
         {
            model->getProjectionCoords( r, coords );
            _add_coords( m_coords, m_coords, coords );
            count++;
         }
      }

      m_coords[0] = m_coords[0] / (double) count;
      m_coords[1] = m_coords[1] / (double) count;
      m_coords[2] = m_coords[2] / (double) count;
   }

   m_rotatePoint = new RotatePoint();
   m_rotatePoint->setPoint( m_coords[0], m_coords[1], m_coords[2] );
   DecalManager::getInstance()->addDecalToModel( m_rotatePoint, model );

   m_widget = new RotateToolWidget( this, mainwin,
         m_coords[0], m_coords[1], m_coords[2] );
#ifdef HAVE_QT4
   //mainwin->addDockWindow( m_widget, DockBottom );
#endif
   m_widget->show();
}

void RotateTool::deactivated()
{
   DecalManager::getInstance()->removeDecal( m_rotatePoint );
   delete m_rotatePoint;
   m_rotatePoint = NULL;
   m_widget->close();
   m_widget = NULL;
}

void RotateTool::mouseButtonDown( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();
   m_model = model;

   if ( model->getAnimationMode() == Model::ANIMMODE_SKELETAL )
   {
      list<int> joints;
      model->getSelectedBoneJoints( joints );
      if ( joints.size() > 0 )
      {
         model->getBoneJointCoords( joints.front(), m_coords );
         m_widget->setCoords( m_coords[0], m_coords[1], m_coords[2] );
         m_rotatePoint->setPoint( m_coords[0], m_coords[1], m_coords[2] );
      }
   }

   double newCoords[3] = {0,0,0};

   parent->getParentXYValue( x, y, newCoords[0], newCoords[1], true );

   m_viewMatrix  = parent->getParentViewMatrix();
   m_viewInverse = parent->getParentViewInverseMatrix();

   if ( buttonState & BS_Left )
   {
      m_tracking = true;

      getRotateCoords( parent );

      double xDiff = newCoords[0] - m_coords[0];
      double yDiff = newCoords[1] - m_coords[1];

      double angle = diffToAngle( yDiff, xDiff );

      if ( buttonState & BS_Shift )
      {
         angle = adjustToNearest( angle );
      }

      m_startAngle = angle;
      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Rotating selected primitives" ).utf8() );
   }
   else if ( buttonState & BS_Right && model->getAnimationMode() != Model::ANIMMODE_SKELETAL )
   {
      for ( int t = 0; t < 3; t++ )
      {
         m_coords[t] = newCoords[t];
      }
      m_coords[3] = 1;

      m_viewInverse.apply( m_coords );

      m_widget->setCoords( m_coords[0], m_coords[1], m_coords[2] );
      m_rotatePoint->setPoint( m_coords[0], m_coords[1], m_coords[2] );
      model_status( model, StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Setting rotation point" ).utf8() );
   }
   parent->updateAllViews();
}

void RotateTool::mouseButtonMove( Parent * parent, int buttonState, int x, int y )
{
   Model * model = parent->getModel();
   m_model = model;

   double newCoords[3] = {0,0,0};

   parent->getParentXYValue( x, y, newCoords[0], newCoords[1] );

   if ( buttonState & BS_Left )
   {
      getRotateCoords( parent );

      double xDiff = newCoords[0] - m_coords[0];
      double yDiff = newCoords[1] - m_coords[1];

      double angles[3] = { 0, 0, 0 };
      Matrix m;

      double angle = diffToAngle( yDiff, xDiff );
      if ( buttonState & BS_Shift )
      {
         angle = adjustToNearest( angle );
      }
      angles[2] = (angle - m_startAngle);

      double vec[4] = { 0, 0, 1, 1 };

      m_viewInverse.apply3( vec );
      m.setRotationOnAxis( vec, angle - m_startAngle );

      m_coords[3] = 1;
      m_viewInverse.apply( m_coords );

      model->rotateSelected( m, m_coords );

      m_startAngle = angle;
   }
   else if ( buttonState & BS_Right )
   {
      getRotateCoords( parent );

      for ( int t = 0; t < 3; t++ )
      {
         m_coords[t] = newCoords[t];
      }
      m_coords[3] = 1;

      m_viewInverse.apply( m_coords );

      m_widget->setCoords( m_coords[0], m_coords[1], m_coords[2] );
      m_rotatePoint->setPoint( m_coords[0], m_coords[1], m_coords[2] );
   }
   parent->updateAllViews();
}

void RotateTool::mouseButtonUp( Parent * parent, int buttonState, int x, int y )
{
   parent->updateAllViews();

   m_tracking = false;

   if ( buttonState & BS_Left )
   {
      model_status( parent->getModel(), StatusNormal, STATUSTIME_SHORT, qApp->translate( "Tool", "Rotate complete" ).utf8() );
   }
}

const char ** RotateTool::getPixmap()
{
   return (const char **) rotatetool_xpm;
}

double RotateTool::diffToAngle( double opposite, double adjacent )
{
   if ( adjacent < 0.0001 && adjacent > -0.0001 )
   {
      adjacent = (adjacent >= 0 ) ? 0.0001 : -0.0001;
   }

   double angle = atan(  opposite / adjacent );

   float quad = PIOVER180 * 90;

   if ( adjacent < 0 )
   {
      if ( opposite < 0 )
      {
         angle = -(quad) - ( (quad) - angle );
      }
      else
      {
         angle = (quad) + ( (quad) + angle );
      }
   }

   return angle;
}

void RotateTool::getRotateCoords( Tool::Parent * parent )
{
   m_rotatePoint->getPoint( m_coords[0], m_coords[1], m_coords[2] );
   m_coords[3] = 1;
   m_viewMatrix.apply( m_coords );
}

double RotateTool::adjustToNearest( double angle )
{
   double f = angle / PIOVER180; // Change to degrees
   if ( f < 0.0 )
   {
      int n = (int) (f / 15.0 - 0.5);
      f = n * 15.0;
   }
   else
   {
      int n = (int) (f / 15.0 + 0.5);
      f = n * 15.0;
   }
   log_debug( "nearest angle is %f\n", f );
   return f * PIOVER180;
}

const char * RotateTool::getName( int arg )
{
   return QT_TRANSLATE_NOOP( "Tool", "Rotate" );
}

void RotateTool::setXValue( double newValue )
{
   double x = 0; double y = 0; double z = 0;
   m_rotatePoint->getPoint( x, y, z );
   x = newValue;
   m_rotatePoint->setPoint( x, y, z );

   DecalManager::getInstance()->modelUpdated( m_model );
}

void RotateTool::setYValue( double newValue )
{
   double x = 0; double y = 0; double z = 0;
   m_rotatePoint->getPoint( x, y, z );
   y = newValue;
   m_rotatePoint->setPoint( x, y, z );

   DecalManager::getInstance()->modelUpdated( m_model );
}

void RotateTool::setZValue( double newValue )
{
   double x = 0; double y = 0; double z = 0;
   m_rotatePoint->getPoint( x, y, z );
   z = newValue;
   m_rotatePoint->setPoint( x, y, z );

   DecalManager::getInstance()->modelUpdated( m_model );
}

