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


#include "torustoolwidget.h"

#include "3dmprefs.h"

#include "mq3macro.h"
#include "mq3compat.h"

#include <qlayout.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qslider.h>

TorusToolWidget::TorusToolWidget( Observer * observer, QWidget * parent )
   : Q3DockWindow ( Q3DockWindow::InDock, parent, "", Qt::WDestructiveClose ),
     m_observer( observer )
{
   const int  DEFAULT_SEGMENTS = 8;
   const int  DEFAULT_SIDES  = 8;
   const int  DEFAULT_WIDTH  = 50;
   const bool DEFAULT_CIRCLE = false;

   m_layout = boxLayout();

   m_segmentsLabel = new QLabel( tr("Segments"), this, "" );
   m_layout->addWidget( m_segmentsLabel );

   m_segmentsValue = new QSpinBox( this, "" );
   m_layout->addWidget( m_segmentsValue );

   m_segmentsValue->setMinValue( 3 );
   m_segmentsValue->setMaxValue( 100 );
   
   int segmentsVal = DEFAULT_SEGMENTS;
   if ( g_prefs.exists( "ui_torustool_segments" ) )
   {
      int val = g_prefs( "ui_torustool_segments" ).intValue();
      if ( val >= 3 && val <= 100 )
      {
         segmentsVal = val;
      }
   }
   m_segmentsValue->setValue( segmentsVal );

   m_sidesLabel = new QLabel( tr("Sides"), this, "" );
   m_layout->addWidget( m_sidesLabel );

   m_sidesValue = new QSpinBox( this, "" );
   m_layout->addWidget( m_sidesValue );

   m_sidesValue->setMinValue( 3 );
   m_sidesValue->setMaxValue( 100 );
   
   int sidesVal = DEFAULT_SIDES;
   if ( g_prefs.exists( "ui_torustool_sides" ) )
   {
      int val = g_prefs( "ui_torustool_sides" ).intValue();
      if ( val >= 3 && val <= 100 )
      {
         sidesVal = val;
      }
   }
   m_sidesValue->setValue( sidesVal );

   m_widthLabel = new QLabel( tr("Width"), this, "" );
   m_layout->addWidget( m_widthLabel );

   m_widthValue = new QSpinBox( this, "" );
   m_layout->addWidget( m_widthValue );

   m_widthValue->setMinValue( 1 );
   m_widthValue->setMaxValue( 199 );
   int widthVal = DEFAULT_WIDTH;
   if ( g_prefs.exists( "ui_torustool_width" ) )
   {
      int val = g_prefs( "ui_torustool_width" ).intValue();
      if ( val >= 0 && val <= 100 )
      {
         widthVal = val;
      }
   }
   m_widthValue->setValue( widthVal );

   m_circleValue = new QCheckBox( tr("Circle"), this, "" );
   m_layout->addWidget( m_circleValue );

   bool circleVal = DEFAULT_CIRCLE;
   g_prefs.setDefault( "ui_torustool_circle", DEFAULT_CIRCLE ? 1 : 0 );
   circleVal = g_prefs( "ui_torustool_circle" ).intValue() ? true : false;

   m_circleValue->setChecked( circleVal );

   m_centerValue = new QCheckBox( tr("From Center", "Checkbox that indicates if torus is created from center or from far corner"), this, "" );
   m_layout->addWidget( m_centerValue );

   bool centerVal = DEFAULT_CIRCLE;
   g_prefs.setDefault( "ui_torustool_center", DEFAULT_CIRCLE ? 1 : 0 );
   centerVal = g_prefs( "ui_torustool_center" ).intValue() ? true : false;

   m_centerValue->setChecked( centerVal );

   connect( m_segmentsValue, SIGNAL(valueChanged(int)), this, SLOT(segmentsValueChanged(int)) );
   connect( m_sidesValue, SIGNAL(valueChanged(int)), this, SLOT(sidesValueChanged(int)) );
   connect( m_widthValue,  SIGNAL(valueChanged(int)), this, SLOT(widthValueChanged(int))   );
   connect( m_circleValue, SIGNAL(toggled(bool)),     this, SLOT(circleValueChanged(bool)) );
   connect( m_centerValue, SIGNAL(toggled(bool)),     this, SLOT(centerValueChanged(bool)) );

   m_segmentsLabel->show();
   m_segmentsValue->show();
   m_sidesLabel->show();
   m_sidesValue->show();
   m_widthLabel->show();
   m_widthValue->show();
   m_circleValue->show();
   m_centerValue->show();

   segmentsValueChanged( segmentsVal );
   sidesValueChanged( sidesVal );
   widthValueChanged( widthVal );
   circleValueChanged( circleVal );
   centerValueChanged( centerVal );
}

TorusToolWidget::~TorusToolWidget()
{
}

void TorusToolWidget::segmentsValueChanged( int newValue )
{
   g_prefs( "ui_torustool_segments" ) = newValue;
   m_observer->setSegmentsValue( newValue );
}

void TorusToolWidget::sidesValueChanged( int newValue )
{
   g_prefs( "ui_torustool_sides" ) = newValue;
   m_observer->setSidesValue( newValue );
}

void TorusToolWidget::widthValueChanged( int newValue )
{
   g_prefs( "ui_torustool_width" ) = newValue;
   m_observer->setWidthValue( newValue );
}

void TorusToolWidget::circleValueChanged( bool newValue )
{
   g_prefs( "ui_torustool_circle" ) = newValue ? 1 : 0;
   m_observer->setCircleValue( newValue );
}

void TorusToolWidget::centerValueChanged( bool newValue )
{
   g_prefs( "ui_torustool_center" ) = newValue ? 1 : 0;
   m_observer->setCenterValue( newValue );
}

