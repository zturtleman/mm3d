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


#include "spherifywin.h"
#include "helpwin.h"
#include "model.h"
#include "glmath.h"
#include "decalmgr.h"
#include "log.h"

#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <q3accel.h>
#include <math.h>
#include <stdlib.h>


SpherifyWin::SpherifyWin( Model * model, QWidget * parent )
   : ValueWin( parent, Qt::WDestructiveClose ),
     m_model( model )
{
   setLabel( "Spherify" );
   m_valueSlider->setMinValue( -100 );
   m_valueEdit->setText( QString("0") );

   double min[3] = { 0.0, 0.0, 0.0 };
   double max[3] = { 0.0, 0.0, 0.0 };
   bool haveCenter = false;

   SpherifyPosition sv;
   std::list<Model::Position> posList;
   m_model->getSelectedPositions( posList );
   std::list<Model::Position>::iterator it;
   for ( it = posList.begin(); it != posList.end(); it++ )
   {
       sv.pos = (*it);
       m_model->getPositionCoords( (*it), sv.coords );

       if ( haveCenter )
       {
           min[0] = sv.coords[0] < min[0] ? sv.coords[0] : min[0];
           min[1] = sv.coords[1] < min[1] ? sv.coords[1] : min[1];
           min[2] = sv.coords[2] < min[2] ? sv.coords[2] : min[2];

           max[0] = sv.coords[0] > max[0] ? sv.coords[0] : max[0];
           max[1] = sv.coords[1] > max[1] ? sv.coords[1] : max[1];
           max[2] = sv.coords[2] > max[2] ? sv.coords[2] : max[2];
       }
       else
       {
           min[0] = sv.coords[0];
           min[1] = sv.coords[1];
           min[2] = sv.coords[2];

           max[0] = sv.coords[0];
           max[1] = sv.coords[1];
           max[2] = sv.coords[2];

           haveCenter = true;
       }

       m_positions.push_back( sv );
   }

   m_center[0] = (min[0] + max[0]) / 2.0;
   m_center[1] = (min[1] + max[1]) / 2.0;
   m_center[2] = (min[2] + max[2]) / 2.0;

   m_radius = 0.0;
   {
       SpherifyPositionList::iterator it;
       for ( it = m_positions.begin(); it != m_positions.end(); it++ )
       {
           double dist = distance( m_center[0], m_center[1], m_center[2],
                   (*it).coords[0], (*it).coords[1], (*it).coords[2] );
           m_radius = dist > m_radius ? dist : m_radius;
       }
   }

   log_debug( "center is %f,%f,%f\n", m_center[0], m_center[1], m_center[2] );
   log_debug( "radius is %f\n", m_radius );
}

SpherifyWin::~SpherifyWin()
{
}

void SpherifyWin::showHelp()
{
   HelpWin * win = new HelpWin( "olh_spherifywin.html", true );
   win->show();
}

void SpherifyWin::valueSliderChanged( int v )
{
   log_debug( "changed\n" );

   if ( v > 100 )
   {
      v = 100;
   }
   if ( v < -100 ) 
   {
      v = -100;
   }

   double percent = (double) v / 100.0;

   double diff[4] = { 0.0, 0.0, 0.0, 0.0 };
   SpherifyPositionList::iterator it;
   for ( it = m_positions.begin(); it != m_positions.end(); it++ )
   {
      diff[0] = (*it).coords[0] - m_center[0];
      diff[1] = (*it).coords[1] - m_center[1];
      diff[2] = (*it).coords[2] - m_center[2];

      Vector vec( diff );
      vec.normalize3();

      diff[0] = vec.get( 0 ) * m_radius + m_center[0];
      diff[1] = vec.get( 1 ) * m_radius + m_center[1];
      diff[2] = vec.get( 2 ) * m_radius + m_center[2];

      diff[0] = (diff[0] - (*it).coords[0]) * percent + (*it).coords[0];
      diff[1] = (diff[1] - (*it).coords[1]) * percent + (*it).coords[1];
      diff[2] = (diff[2] - (*it).coords[2]) * percent + (*it).coords[2];

      m_model->movePosition( (*it).pos, diff[0], diff[1], diff[2] );
   }

   DecalManager::getInstance()->modelUpdated( m_model );
   ValueWin::valueSliderChanged( v );
}

void SpherifyWin::valueEditChanged( const QString & str )
{
   ValueWin::valueEditChanged( str );
}

void SpherifyWin::accept()
{
   m_model->operationComplete( tr( "Spherify", "operation complete" ).utf8() );
   DecalManager::getInstance()->modelUpdated( m_model );
   ValueWin::accept();
}

void SpherifyWin::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   ValueWin::reject();
}


