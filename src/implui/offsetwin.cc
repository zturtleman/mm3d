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


#include "offsetwin.h"
#include "helpwin.h"
#include "model.h"
#include "glmath.h"
#include "decalmgr.h"
#include "log.h"

#include <QtWidgets/QSlider>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>

#include <math.h>
#include <stdlib.h>


OffsetWin::OffsetWin( Model * model, QWidget * parent )
   : QDialog( parent, Qt::WindowFlags() ),
     m_model( model ),
     m_editing( false )
{
   setupUi( this );
   setModal( true );

   setAttribute( Qt::WA_DeleteOnClose );

   m_valueSlider->setMinimum( -100 );
   m_valueSlider->setMaximum( 100 );

   m_valueEdit->setText( QString("0") );
   m_rangeEdit->setText( QString("10.00") );

   OffsetPosition ov;
   std::list<int> vertList;
   m_model->getSelectedVertices( vertList );
   std::list<int>::iterator it;
   for ( it = vertList.begin(); it != vertList.end(); it++ )
   {
       ov.vert = (*it);
       m_model->getVertexCoords( ov.vert, ov.coords );
       m_model->getAverageNormal( ov.vert, ov.normal );

       // vertex not connected to a face has no normal
       if ( ov.normal[0] == 0.0f && ov.normal[1] == 0.0f && ov.normal[2] == 0.0f ) {
          continue;
       }

       m_positions.push_back( ov );
   }
}

OffsetWin::~OffsetWin()
{
}

void OffsetWin::showHelp()
{
   HelpWin * win = new HelpWin( "olh_offsetwin.html", true );
   win->show();
}

void OffsetWin::valueSliderChanged( int v )
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
   double dist = m_range * percent;

   double coords[3] = { 0.0, 0.0, 0.0 };
   OffsetPositionList::iterator it;
   for ( it = m_positions.begin(); it != m_positions.end(); it++ )
   {
      coords[0] = (*it).coords[0] + (*it).normal[0] * dist;
      coords[1] = (*it).coords[1] + (*it).normal[1] * dist;
      coords[2] = (*it).coords[2] + (*it).normal[2] * dist;

      m_model->moveVertex( (*it).vert, coords[0], coords[1], coords[2] );
   }

   DecalManager::getInstance()->modelUpdated( m_model );

   if ( ! m_editing )
   {
      QString str = QString::asprintf( "%d", v );
      m_valueEdit->setText( str );
   }
}

void OffsetWin::valueEditChanged( const QString & str )
{
   m_editing = true;
   float v = str.toDouble();
   m_valueSlider->setValue( (int) v );
   m_editing = false;
}

void OffsetWin::rangeEditChanged( const QString & str )
{
   m_range = str.toDouble();

   valueSliderChanged( m_valueSlider->value() );
}

void OffsetWin::accept()
{
   m_model->operationComplete( tr( "Offset by Normal", "operation complete" ).toUtf8() );
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::accept();
}

void OffsetWin::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}
