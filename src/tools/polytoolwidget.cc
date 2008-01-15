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
#include <qcombobox.h>

PolyToolWidget::PolyToolWidget( Observer * observer, QWidget * parent )
   : Q3DockWindow ( Q3DockWindow::InDock, parent, "", Qt::WDestructiveClose ),
     m_observer( observer )
{
   const int DEFAULT_FAN  = 0;

   m_layout = boxLayout();

   m_typeLabel = new QLabel( tr("Poly Type"), this, "" );
   m_layout->addWidget( m_typeLabel );

   m_typeValue = new QComboBox( this, "" );
   m_typeValue->insertItem( tr("Strip", "Triangle strip option"), 0 );
   m_typeValue->insertItem( tr("Fan", "Triangle fan option"), 1 );
   m_layout->addWidget( m_typeValue );

   g_prefs.setDefault( "ui_polytool_is_fan", DEFAULT_FAN );
   int index = g_prefs( "ui_polytool_isfan" ).intValue();
   m_typeValue->setCurrentItem( (index == 0) ? 0 : 1 );

   connect( m_typeValue,  SIGNAL(activated(int)), this, SLOT(typeValueChanged(int))  );

   m_typeLabel->show();
   m_typeValue->show();

   typeValueChanged( index );
}

PolyToolWidget::~PolyToolWidget()
{
}

void PolyToolWidget::typeValueChanged( int newValue )
{
   g_prefs( "ui_polytool_isfan" ) = newValue;
   m_observer->setTypeValue( newValue );
}

