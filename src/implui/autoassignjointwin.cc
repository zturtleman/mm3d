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


#include "autoassignjointwin.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "helpwin.h"

#include <qslider.h>
#include <qcheckbox.h>

using std::list;
using std::map;

AutoAssignJointWin::AutoAssignJointWin( Model * model, QWidget * parent, const char * name )
   : AutoAssignJointWinBase( parent, name, true ),
     m_accel( new QAccel(this) ),
     m_model( model )
{
   if ( m_model->getSelectedBoneJointCount() == 0 )
   {
      m_selected->setChecked( false );
      m_selected->setEnabled( false );
   }

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );
}

AutoAssignJointWin::~AutoAssignJointWin()
{
}

void AutoAssignJointWin::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_autoassignjointwin.html", true );
   win->show();
}

int AutoAssignJointWin::getSensitivity()
{
   return m_sensitivity->value();
}

bool AutoAssignJointWin::getSelected()
{
   return m_selected->isChecked();
}

