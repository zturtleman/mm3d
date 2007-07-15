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


#include "polytoolwidget.h"

#include "3dmprefs.h"

#include "mq3macro.h"
#include "mq3compat.h"

#include <qlayout.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>

PolyToolWidget::PolyToolWidget( Observer * observer, QWidget * parent )
   : QDockWindow ( QDockWindow::InDock, parent, "", WDestructiveClose ),
     m_observer( observer )
{
   const bool DEFAULT_FAN  = false;

   m_layout = boxLayout();

   m_fanLabel = new QLabel( tr("Fan"), this, "" );
   m_layout->addWidget( m_fanLabel );

   m_fanValue = new QCheckBox( this, "" );
   m_layout->addWidget( m_fanValue );

   bool isPoly = DEFAULT_FAN;
   if ( g_prefs.exists( "ui_polytool_isfan" ) )
   {
      isPoly = (g_prefs( "ui_polytool_isfan" ).intValue() != 0) ? true : false;
   }
   m_fanValue->setChecked( isPoly );

   connect( m_fanValue,  SIGNAL(toggled(bool)), this, SLOT(fanValueChanged(bool))  );

   m_fanLabel->show();
   m_fanValue->show();

   fanValueChanged( isPoly );
}

PolyToolWidget::~PolyToolWidget()
{
}

void PolyToolWidget::fanValueChanged( bool newValue )
{
   g_prefs( "ui_polytool_isfan" ) = newValue ? 1 : 0;
   m_observer->setFanValue( newValue );
}

