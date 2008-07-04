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

#include "model.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>

ProjToolWidget::ProjToolWidget( Observer * observer, QMainWindow * parent )
   : ToolWidget ( parent ),
     m_observer( observer )
{
   const int DEFAULT_TYPE = Model::TPT_Sphere;

   m_layout = boxLayout();

   m_typeLabel = new QLabel( tr("Type"), mainWidget() );
   m_layout->addWidget( m_typeLabel );

   m_typeValue = new QComboBox( mainWidget() );
   m_layout->addWidget( m_typeValue );

   m_typeValue->insertItem( Model::TPT_Cylinder, tr("Cylinder", "Cylinder projection type") );
   m_typeValue->insertItem( Model::TPT_Sphere, tr("Sphere", "Sphere projection type") );
   m_typeValue->insertItem( Model::TPT_Plane, tr("Plane", "Plane projection type") );

   int typeIndex = DEFAULT_TYPE;
   g_prefs.setDefault( "ui_projtool_type_index", DEFAULT_TYPE );
   int temp = g_prefs( "ui_projtool_type_index" ).intValue();
   if ( temp >= Model::TPT_Cylinder  && temp <= Model::TPT_Plane )
   {
      typeIndex = temp;
   }
   m_typeValue->setCurrentIndex( typeIndex );

   m_layout->addStretch();

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

