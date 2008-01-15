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

#include "viewportsettings.h"

#include "helpwin.h"
#include "3dmprefs.h"

#include <QLineEdit>
#include <QCheckBox>

#include <q3accel.h>


ViewportSettings::ViewportSettings( QWidget * parent )
   : QDialog( parent, Qt::WDestructiveClose ),
     m_accel( new Q3Accel(this) )
{
   setupUi( this );
   setModal( true );

   m_accel->insertItem( Qt::Key_F1, 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );

   QString temp;

   m_gridUnit->setText( QString( g_prefs( "ui_grid_inc" ).stringValue().c_str() ) );
   m_3dGridUnit->setText( QString( g_prefs( "ui_3dgrid_inc" ).stringValue().c_str() ) );
   m_3dGridLines->setText( QString( g_prefs( "ui_3dgrid_count" ).stringValue().c_str() ) );

   m_3dXY->setChecked( g_prefs( "ui_3dgrid_xy" ).intValue() != 0 );
   m_3dXZ->setChecked( g_prefs( "ui_3dgrid_xz" ).intValue() != 0 );
   m_3dYZ->setChecked( g_prefs( "ui_3dgrid_yz" ).intValue() != 0 );
}

ViewportSettings::~ViewportSettings()
{
}

void ViewportSettings::accept()
{
   g_prefs( "ui_grid_inc" )     = m_gridUnit->text().latin1();
   g_prefs( "ui_3dgrid_inc" )   = m_3dGridUnit->text().latin1();
   g_prefs( "ui_3dgrid_count" ) = m_3dGridLines->text().latin1();

   g_prefs( "ui_3dgrid_xy" ) = m_3dXY->isChecked() ? 1 : 0;
   g_prefs( "ui_3dgrid_xz" ) = m_3dXZ->isChecked() ? 1 : 0;
   g_prefs( "ui_3dgrid_yz" ) = m_3dYZ->isChecked() ? 1 : 0;

   QDialog::accept();
}

void ViewportSettings::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_viewportsettings.html", true );
   win->show();
}

