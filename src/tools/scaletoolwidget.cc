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


#include "scaletoolwidget.h"
#include "3dmprefs.h"

#include "mq3macro.h"
#include "mq3compat.h"

#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>

ScaleToolWidget::ScaleToolWidget( Observer * observer, QWidget * parent )
   : QDockWindow ( QDockWindow::InDock, parent, "", WDestructiveClose ),
     m_observer( observer )
{
   const int  DEFAULT_PROPORTION = ST_ScaleFree;
   const int  DEFAULT_POINT      = ST_ScalePointCenter;

   m_layout = boxLayout();

   m_proportionLabel = new QLabel( tr("Proportion"), this, "" );
   m_layout->addWidget( m_proportionLabel );

   m_proportionValue = new QComboBox( this, "" );
   m_layout->addWidget( m_proportionValue );

   m_proportionValue->insertItem( tr("Free", "Free scaling option"), ST_ScaleFree );
   m_proportionValue->insertItem( tr("Keep Aspect 2D", "2D scaling aspect option"), ST_ScaleProportion2D );
   m_proportionValue->insertItem( tr("Keep Aspect 3D", "3D scaling aspect option"), ST_ScaleProportion3D );

   int aspectIndex = DEFAULT_PROPORTION;
   if ( g_prefs.exists("ui_scaletool_aspect_index") )
   {
      int temp = g_prefs( "ui_scaletool_aspect_index" ).intValue();
      if ( temp >= 0 && temp < 3 )
      {
         aspectIndex = temp;
      }
   }
   m_proportionValue->setCurrentItem( aspectIndex );

   connect( m_proportionValue, SIGNAL(activated(int)), this, SLOT(proportionValueChanged(int)) );

   m_pointLabel = new QLabel( tr("Point"), this, "" );
   m_layout->addWidget( m_pointLabel );

   m_pointValue = new QComboBox( this, "" );
   m_layout->addWidget( m_pointValue );

   m_pointValue->insertItem( tr("Center", "Scale from center"), ST_ScalePointCenter );
   m_pointValue->insertItem( tr("Far Corner", "Scale from far corner"), ST_ScalePointFar );

   int pointIndex = DEFAULT_POINT;
   if ( g_prefs.exists("ui_scaletool_point_index") )
   {
      int temp = g_prefs( "ui_scaletool_point_index" ).intValue();
      if ( temp >= ST_ScalePointCenter && temp <= ST_ScalePointFar )
      {
         pointIndex = temp;
      }
   }
   m_pointValue->setCurrentItem( pointIndex );

   connect( m_pointValue, SIGNAL(activated(int)), this, SLOT(pointValueChanged(int)) );

   m_proportionLabel->show();
   m_proportionValue->show();
   m_pointLabel->show();
   m_pointValue->show();

   proportionValueChanged( aspectIndex );
   pointValueChanged( pointIndex );
}

ScaleToolWidget::~ScaleToolWidget()
{
}

void ScaleToolWidget::proportionValueChanged( int newValue )
{
   g_prefs( "ui_scaletool_aspect_index" ) = newValue;
   m_observer->setProportionValue( newValue );
}

void ScaleToolWidget::pointValueChanged( int newValue )
{
   g_prefs( "ui_scaletool_point_index" ) = newValue;
   m_observer->setPointValue( newValue );
}

