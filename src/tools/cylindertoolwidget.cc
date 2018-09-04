/*  Maverick Model 3D
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


#include "cylindertoolwidget.h"

#include "3dmprefs.h"

#include <QtWidgets/QLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>

CylinderToolWidget::CylinderToolWidget( Observer * observer, QMainWindow * parent )
   : ToolWidget ( parent ),
     m_observer( observer )
{
   const int DEFAULT_SEGMENTS = 4;
   const int DEFAULT_SIDES  = 8;
   const int DEFAULT_WIDTH  = 100;
   const int DEFAULT_SCALE  = 100;
   //const int DEFAULT_SHAPE  = 10;

   m_layout = boxLayout();

   m_segmentsLabel = new QLabel( tr("Segments"), mainWidget() );
   m_layout->addWidget( m_segmentsLabel );

   m_segmentsValue = new QSpinBox( mainWidget() );
   m_layout->addWidget( m_segmentsValue );

   m_segmentsValue->setMinimum( 1 );
   m_segmentsValue->setMaximum( 100 );
   
   int segmentsVal = DEFAULT_SEGMENTS;
   if ( g_prefs.exists( "ui_cylindertool_segments" ) )
   {
      int val = g_prefs( "ui_cylindertool_segments" ).intValue();
      if ( val >= 1 && val <= 100 )
      {
         segmentsVal = val;
      }
   }
   m_segmentsValue->setValue( segmentsVal );

   m_sidesLabel = new QLabel( tr("Sides"), mainWidget() );
   m_layout->addWidget( m_sidesLabel );

   m_sidesValue = new QSpinBox( mainWidget() );
   m_layout->addWidget( m_sidesValue );

   m_sidesValue->setMinimum( 3 );
   m_sidesValue->setMaximum( 100 );
   
   int sidesVal = DEFAULT_SIDES;
   if ( g_prefs.exists( "ui_cylindertool_sides" ) )
   {
      int val = g_prefs( "ui_cylindertool_sides" ).intValue();
      if ( val >= 3 && val <= 100 )
      {
         sidesVal = val;
      }
   }
   m_sidesValue->setValue( sidesVal );
 

   m_widthLabel = new QLabel( tr("Width"), mainWidget() );
   m_layout->addWidget( m_widthLabel );

   m_widthValue = new QSpinBox( mainWidget() );
   m_layout->addWidget( m_widthValue );

   m_widthValue->setMinimum( 0 );
   m_widthValue->setMaximum( 100 );
   int widthVal = DEFAULT_WIDTH;
   if ( g_prefs.exists( "ui_cylindertool_width" ) )
   {
      int val = g_prefs( "ui_cylindertool_width" ).intValue();
      if ( val >= 0 && val <= 100 )
      {
         widthVal = val;
      }
   }
   m_widthValue->setValue( widthVal );

   m_scaleLabel = new QLabel( tr("Scale"), mainWidget() );
   m_layout->addWidget( m_scaleLabel );

   m_scaleValue = new QSpinBox( mainWidget() );
   m_layout->addWidget( m_scaleValue );

   m_scaleValue->setMinimum( 0 );
   m_scaleValue->setMaximum( 100 );
   int scaleVal = DEFAULT_SCALE;
   if ( g_prefs.exists( "ui_cylindertool_scale" ) )
   {
      int val = g_prefs( "ui_cylindertool_scale" ).intValue();
      if ( val >= 0 && val <= 100 )
      {
         scaleVal = val;
      }
   }
   m_scaleValue->setValue( scaleVal );

   /*
   m_shapeLabel = new QLabel( "Shape", mainWidget() );
   m_layout->addWidget( m_shapeLabel );

   m_shapeValue = new QSlider( Qt::Horizontal, mainWidget() );
   m_layout->addWidget( m_shapeValue );

   m_shapeValue->setMinimumWidth( 64 );
   m_shapeValue->setMinimum( 0 );
   m_shapeValue->setMaximum( DEFAULT_SHAPE * 2 );
   m_shapeValue->setValue( DEFAULT_SHAPE );
   m_shapeValue->setTickInterval( DEFAULT_SHAPE / 2);
   m_shapeValue->setTickmarks( QSlider::Below );
   */

   m_layout->addStretch();

   connect( m_segmentsValue, SIGNAL(valueChanged(int)), this, SLOT(segmentsValueChanged(int)) );
   connect( m_sidesValue, SIGNAL(valueChanged(int)), this, SLOT(sidesValueChanged(int)) );
   connect( m_widthValue,  SIGNAL(valueChanged(int)), this, SLOT(widthValueChanged(int))  );
   connect( m_scaleValue,  SIGNAL(valueChanged(int)), this, SLOT(scaleValueChanged(int))  );
   //connect( m_shapeValue,  SIGNAL(valueChanged(int)), this, SLOT(shapeValueChanged(int))  );

   m_segmentsLabel->show();
   m_segmentsValue->show();
   m_sidesLabel->show();
   m_sidesValue->show();
   m_widthLabel->show();
   m_widthValue->show();
   m_scaleLabel->show();
   m_scaleValue->show();
   //m_shapeLabel->hide();
   //m_shapeValue->hide();

   segmentsValueChanged( segmentsVal );
   sidesValueChanged( sidesVal );
   widthValueChanged( widthVal );
   scaleValueChanged( scaleVal );
   //shapeValueChanged( DEFAULT_SHAPE );
}

CylinderToolWidget::~CylinderToolWidget()
{
}

void CylinderToolWidget::segmentsValueChanged( int newValue )
{
   g_prefs( "ui_cylindertool_segments" ) = newValue;
   m_observer->setSegmentsValue( newValue );
}

void CylinderToolWidget::sidesValueChanged( int newValue )
{
   g_prefs( "ui_cylindertool_sides" ) = newValue;
   m_observer->setSidesValue( newValue );
}

void CylinderToolWidget::widthValueChanged( int newValue )
{
   g_prefs( "ui_cylindertool_width" ) = newValue;
   m_observer->setWidthValue( newValue );
}

void CylinderToolWidget::scaleValueChanged( int newValue )
{
   g_prefs( "ui_cylindertool_scale" ) = newValue;
   m_observer->setScaleValue( newValue );
}

void CylinderToolWidget::shapeValueChanged( int newValue )
{
   m_observer->setShapeValue( newValue );
}

