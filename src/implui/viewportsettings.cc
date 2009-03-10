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

#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QShortcut>


ViewportSettings::ViewportSettings( QWidget * parent )
   : QDialog( parent )
{
   setAttribute( Qt::WA_DeleteOnClose );
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   QString temp;

   m_gridUnit->setText( QString( g_prefs( "ui_grid_inc" ).stringValue().c_str() ) );
   m_3dGridUnit->setText( QString( g_prefs( "ui_3dgrid_inc" ).stringValue().c_str() ) );
   m_3dGridLines->setText( QString( g_prefs( "ui_3dgrid_count" ).stringValue().c_str() ) );

   switch ( g_prefs( "ui_grid_mode" ).intValue() )
   {
      default:
      case 0:
         m_binaryGrid->setChecked( true );
         break;
      case 1:
         m_decimalGrid->setChecked( true );
         break;
      case 2:
         m_fixedGrid->setChecked( true );
         break;
   }

   m_3dXY->setChecked( g_prefs( "ui_3dgrid_xy" ).intValue() != 0 );
   m_3dXZ->setChecked( g_prefs( "ui_3dgrid_xz" ).intValue() != 0 );
   m_3dYZ->setChecked( g_prefs( "ui_3dgrid_yz" ).intValue() != 0 );
}

ViewportSettings::~ViewportSettings()
{
}

void ViewportSettings::accept()
{
   g_prefs( "ui_grid_inc" )     = (const char *) m_gridUnit->text().toLatin1();
   g_prefs( "ui_3dgrid_inc" )   = (const char *) m_3dGridUnit->text().toLatin1();
   g_prefs( "ui_3dgrid_count" ) = (const char *) m_3dGridLines->text().toLatin1();

   int grid_mode = 0;

   if ( m_decimalGrid->isChecked() )
      grid_mode = 1;
   else if (m_fixedGrid->isChecked() )
      grid_mode = 2;

   g_prefs( "ui_grid_mode" ) = grid_mode;
   g_prefs( "ui_3dgrid_xy" ) = m_3dXY->isChecked() ? 1 : 0;
   g_prefs( "ui_3dgrid_xz" ) = m_3dXZ->isChecked() ? 1 : 0;
   g_prefs( "ui_3dgrid_yz" ) = m_3dYZ->isChecked() ? 1 : 0;

   QDialog::accept();
}

void ViewportSettings::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_viewportsettings.html", true );
   win->show();
}

