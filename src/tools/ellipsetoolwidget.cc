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


#include "ellipsetoolwidget.h"

#include "mq3macro.h"
#include "mq3compat.h"

#include <qlayout.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <math.h>

#include "3dmprefs.h"

EllipsoidToolWidget::EllipsoidToolWidget( Observer * observer, QWidget * parent )
   : Q3DockWindow ( Q3DockWindow::InDock, parent, "", Qt::WDestructiveClose ),
     m_observer( observer )
{
   const int  DEFAULT_SMOOTHNESS = 2;
   const bool DEFAULT_SPHERE     = false;
   const bool DEFAULT_CENTER     = false;

   m_layout = boxLayout();

   m_smoothLabel = new QLabel( tr("Smoothness:"), this, "" );
   m_layout->addWidget( m_smoothLabel );
   m_smoothValue = new QSpinBox( this, "" );
   m_layout->addWidget( m_smoothValue );

   m_smoothValue->setMinValue( 0 );
   m_smoothValue->setMaxValue( 5 );

   m_facesLabel = new QLabel( tr("Faces: ") + QString("320"), this, "" );
   m_layout->addWidget( m_facesLabel );

   m_sphereCheckBox = new QCheckBox( tr("Sphere"), this, "" );
   m_layout->addWidget( m_sphereCheckBox );

   m_centerCheckBox = new QCheckBox( tr("From Center", "Checkbox that indicates if ellipsoid is created from center or far corner"), this, "" );
   m_layout->addWidget( m_centerCheckBox );

   int smoothVal = DEFAULT_SMOOTHNESS;
   g_prefs.setDefault( "ui_ellipsetool_smoothness", DEFAULT_SMOOTHNESS );
   int val = g_prefs( "ui_ellipsetool_smoothness" ).intValue();
   if ( val >= 0 && val <= 5 )
   {
      smoothVal = val;
   }

   m_smoothValue->setValue( smoothVal );

   g_prefs.setDefault( "ui_ellipsetool_issphere", DEFAULT_SPHERE ? 1 : 0 );
   bool isSphere = DEFAULT_SPHERE;
   isSphere = (g_prefs( "ui_ellipsetool_issphere" ).intValue() != 0) ? true : false;

   m_sphereCheckBox->setChecked( isSphere );

   g_prefs.setDefault( "ui_ellipsetool_fromcenter", DEFAULT_CENTER ? 1 : 0 );
   bool fromCenter = DEFAULT_CENTER;
   fromCenter = (g_prefs( "ui_ellipsetool_fromcenter" ).intValue() != 0) ? true : false;

   m_centerCheckBox->setChecked( fromCenter );

   connect( m_smoothValue,    SIGNAL(valueChanged(int)), this, SLOT(smoothnessValueChanged(int))      );
   connect( m_sphereCheckBox, SIGNAL(toggled(bool)),     this, SLOT(sphereCheckBoxValueChanged(bool)) );
   connect( m_centerCheckBox, SIGNAL(toggled(bool)),     this, SLOT(centerCheckBoxValueChanged(bool)) );

   m_smoothLabel->show();
   m_smoothValue->show();
   m_facesLabel->show();
   m_sphereCheckBox->show();
   m_centerCheckBox->show();

   smoothnessValueChanged( smoothVal );
   sphereCheckBoxValueChanged( isSphere );
   centerCheckBoxValueChanged( fromCenter );
}

EllipsoidToolWidget::~EllipsoidToolWidget()
{
}

void EllipsoidToolWidget::smoothnessValueChanged( int newValue )
{
   QString str = tr("Faces: ");
   str += QString::number( 20 * (unsigned) pow(4, newValue) );
   m_facesLabel->setText( str );

   g_prefs( "ui_ellipsetool_smoothness" ) = newValue;
   m_observer->setSmoothnessValue( newValue );
}

void EllipsoidToolWidget::sphereCheckBoxValueChanged( bool o )
{
   g_prefs( "ui_ellipsetool_issphere" ) = o ? 1 : 0;
   m_observer->setSphere( o );
}

void EllipsoidToolWidget::centerCheckBoxValueChanged( bool o )
{
   g_prefs( "ui_ellipsetool_fromcenter" ) = o ? 1 : 0;
   m_observer->setCenter( o );
}

