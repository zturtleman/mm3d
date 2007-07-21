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


#include "alignwin.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "decalmgr.h"
#include "helpwin.h"

#include <qlineedit.h>
#include <qradiobutton.h>
#include <stdlib.h>

using std::list;
using std::map;

AlignWin::AlignWin( Model * model, QWidget * parent, const char * name )
   : AlignWinBase( parent, name, true, Qt::WDestructiveClose ),
     m_accel( new QAccel(this) ),
     m_model( model ),
     m_atX( AT_Center ),
     m_atY( AT_Center ),
     m_atZ( AT_Center )
{
   m_xCenter->setChecked( true );
   m_yCenter->setChecked( true );
   m_zCenter->setChecked( true );

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );
}

AlignWin::~AlignWin()
{
}

void AlignWin::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_alignwin.html", true );
   win->show();
}

void AlignWin::alignX()
{
   double val = atof( m_xValue->text().latin1() );
   log_debug( "aligning x on %f\n", val );
   alignSelectedX( m_model, m_atX, val );
   DecalManager::getInstance()->modelUpdated( m_model );
   model_status( m_model, StatusNormal, STATUSTIME_SHORT, tr("Align X").utf8() );
}

void AlignWin::alignY()
{
   double val = atof( m_yValue->text().latin1() );
   log_debug( "aligning y on %f\n", val );
   alignSelectedY( m_model, m_atY, val );
   DecalManager::getInstance()->modelUpdated( m_model );
   model_status( m_model, StatusNormal, STATUSTIME_SHORT, tr("Align Y").utf8() );
}

void AlignWin::alignZ()
{
   double val = atof( m_zValue->text().latin1() );
   log_debug( "aligning z on %f\n", val );
   alignSelectedZ( m_model, m_atZ, val );
   DecalManager::getInstance()->modelUpdated( m_model );
   model_status( m_model, StatusNormal, STATUSTIME_SHORT, tr("Align Z").utf8() );
}

void AlignWin::selectedXCenter()
{
   m_atX = AT_Center;
}

void AlignWin::selectedXMin()
{
   m_atX = AT_Min;
}

void AlignWin::selectedXMax()
{
   m_atX = AT_Max;
}

void AlignWin::selectedYCenter()
{
   m_atY = AT_Center;
}

void AlignWin::selectedYMin()
{
   m_atY = AT_Min;
}

void AlignWin::selectedYMax()
{
   m_atY = AT_Max;
}

void AlignWin::selectedZCenter()
{
   m_atZ = AT_Center;
}

void AlignWin::selectedZMin()
{
   m_atZ = AT_Min;
}

void AlignWin::selectedZMax()
{
   m_atZ = AT_Max;
}

void AlignWin::accept()
{
   log_debug( "Alignment complete" );
   m_model->operationComplete( tr( "Align Selected", "operation complete" ).utf8() );
   AlignWinBase::accept();
}

void AlignWin::reject()
{
   log_debug( "Alignment canceled" );
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   AlignWinBase::reject();
}
