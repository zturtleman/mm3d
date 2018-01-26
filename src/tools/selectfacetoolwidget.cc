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


#include "selectfacetoolwidget.h"

#include "3dmprefs.h"

#include <QtWidgets/QLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>

SelectFaceToolWidget::SelectFaceToolWidget( Observer * observer, QMainWindow * parent )
   : ToolWidget ( parent ),
     m_observer( observer )
{
   const bool DEFAULT_BACKFACING  = true;

   m_layout = boxLayout();

   m_backfacingLabel = new QLabel( tr("Include Back-facing"), mainWidget() );
   m_layout->addWidget( m_backfacingLabel );

   m_backfacingValue = new QCheckBox( mainWidget() );
   m_layout->addWidget( m_backfacingValue );

   bool includeBackfacing = DEFAULT_BACKFACING;
   if ( g_prefs.exists( "ui_selectfacetool_backfacing" ) )
   {
      includeBackfacing = (g_prefs( "ui_selectfacetool_backfacing" ).intValue() != 0) ? true : false;
   }
   m_backfacingValue->setChecked( includeBackfacing );

   m_layout->addStretch();

   connect( m_backfacingValue,  SIGNAL(toggled(bool)), this, SLOT(backfacingValueChanged(bool))  );

   m_backfacingLabel->show();
   m_backfacingValue->show();

   backfacingValueChanged( includeBackfacing );
}

SelectFaceToolWidget::~SelectFaceToolWidget()
{
}

void SelectFaceToolWidget::backfacingValueChanged( bool newValue )
{
   g_prefs( "ui_selectfacetool_backfacing" ) = newValue ? 1 : 0;
   m_observer->setBackfacingValue( newValue );
}

