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


#include "projtoolwidget.h"
#include "3dmprefs.h"

#include "mq3macro.h"
#include "mq3compat.h"
#include "model.h"

#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>

ProjToolWidget::ProjToolWidget( Observer * observer, QWidget * parent )
   : Q3DockWindow ( Q3DockWindow::InDock, parent, "", Qt::WDestructiveClose ),
     m_observer( observer )
{
   const int DEFAULT_TYPE = Model::TPT_Sphere;

   m_layout = boxLayout();

   m_typeLabel = new QLabel( tr("Type"), this, "" );
   m_layout->addWidget( m_typeLabel );

   m_typeValue = new QComboBox( this, "" );
   m_layout->addWidget( m_typeValue );

   m_typeValue->insertItem( tr("Cylinder", "Cylinder projection type"), Model::TPT_Cylinder );
   m_typeValue->insertItem( tr("Sphere", "Sphere projection type"), Model::TPT_Sphere );
   m_typeValue->insertItem( tr("Plane", "Plane projection type"), Model::TPT_Plane );

   int typeIndex = DEFAULT_TYPE;
   g_prefs.setDefault( "ui_projtool_type_index", DEFAULT_TYPE );
   int temp = g_prefs( "ui_projtool_type_index" ).intValue();
   if ( temp >= Model::TPT_Cylinder  && temp <= Model::TPT_Plane )
   {
      typeIndex = temp;
   }
   m_typeValue->setCurrentItem( typeIndex );

   connect( m_typeValue, SIGNAL(activated(int)), this, SLOT(typeValueChanged(int)) );

   m_typeLabel->show();
   m_typeValue->show();

   typeValueChanged( typeIndex );
}

ProjToolWidget::~ProjToolWidget()
{
}

void ProjToolWidget::typeValueChanged( int newValue )
{
   g_prefs( "ui_projtool_type_index" ) = newValue;
   m_observer->setTypeValue( newValue );
}

