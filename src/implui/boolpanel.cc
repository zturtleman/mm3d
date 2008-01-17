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

#include "boolpanel.h"

#include "viewpanel.h"
#include "boolwin.h"

#include "log.h"

#include <qlayout.h>

#include <QContextMenuEvent>

BoolPanel::BoolPanel( Model * model, QWidget * parent, ViewPanel * panel )
   : QDockWidget( tr("Boolean Operation"), parent ),
     m_panel( panel ),
     m_boolWidget( new BoolWin( model, panel, this ) )
{
   setWidget( m_boolWidget );
}

BoolPanel::~BoolPanel()
{
}

void BoolPanel::setModel( Model * model )
{
   m_boolWidget->setModel( model );
}

void BoolPanel::modelChanged( int changeBits )
{
   if ( this->isVisible() )
   {
   }
}

void BoolPanel::show()
{
   QDockWidget::show();

   // this is causing a segfault on dock/undock because the widgets
   // are being destroyed and re-created. The solution is to call setModel
   // explicitly whenever the window is shown (this only happens in viewwin.cc)
   //setModel( m_model ) 
}

void BoolPanel::close()
{
   hide(); // Do hide instead
}

void BoolPanel::hide()
{
   log_debug( "BoolPanel::hide()\n" );
   QDockWidget::hide();
}

void BoolPanel::contextMenuEvent( QContextMenuEvent * e )
{
   e->ignore();
}
