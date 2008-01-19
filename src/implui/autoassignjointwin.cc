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

#include <QSlider>
#include <QCheckBox>
#include <QShortcut>

using std::list;
using std::map;

AutoAssignJointWin::AutoAssignJointWin( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_model( model )
{
   setModal( true );
   setupUi( this );
   if ( m_model->getSelectedBoneJointCount() == 0 )
   {
      m_selected->setChecked( false );
      m_selected->setEnabled( false );
   }

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

AutoAssignJointWin::~AutoAssignJointWin()
{
}

void AutoAssignJointWin::helpNowEvent()
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

